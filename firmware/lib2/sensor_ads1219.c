#include <sensor_ads1219.h>
#include <sensor.h>
#include <address.h>
#ifdef __DICIO__
#include <../dicio.X/dicio.h>
#endif
#ifdef __SYLVATICA__
#include <../sylvatica.X/sylvatica.h>
#endif 

void sensor_ads1219_result_cb(i2c_message_t* m);
void sensor_ads1219_start_cb(i2c_message_t* m);
void sensor_ads1219_interrupt_cb(void* data);
sensor_ads1219_input_mux_configuration_t sensor_ads1219_channel_to_mux(uint8_t channel);


sensor_status_t sensor_ads1219_config(struct sensor_gconfig_s *gconfig, const uint8_t *buffer, const uint8_t length)
{
    sensor_ads1219_config_t* config = &gconfig->sensor_config.ads1219;
    task_t interrupt_task;
    
    if(length < 1){
        return SENSOR_STATUS_ERROR;
    }
    
    UART_DEBUG_PRINT("Configuring ADS1219 sensor");
    
    switch(buffer[0])
    {
        case sensor_ads1219_gloxinia_register_general:
            if (length != 3){ return SENSOR_STATUS_ERROR; }

            gconfig->measure.task.cb = sensor_ads1219_measure;
            gconfig->measure.task.data = (void*) gconfig;
            
            schedule_init(&gconfig->measure, gconfig->measure.task, (((uint16_t)buffer[1]) << 8) | buffer[2]);
            
            task_init(&interrupt_task, sensor_ads1219_interrupt_cb, (void*) gconfig);
            
#ifdef __DICIO__
            dicio_configure_interrupt_routine(gconfig->interface, interrupt_task);
#elif defined(__SYLVATICA__)
            sylvatica_configure_interrupt_routine(gconfig->interface, interrupt_task);
#else
#error "Board not supported for this sensor."
#endif
            
            break;
        case sensor_ads1219_gloxinia_register_config:
            if(length != 7) { return SENSOR_STATUS_ERROR; }
            
            config->address = buffer[1];
            config->i2c_bus = sensor_get_i2c_bus(gconfig->interface->interface_id);
            config->enabled_channels = buffer[2];
            config->gain = buffer[3];
            config->fs = buffer[4];
            config->conversion = buffer[5];
            config->vref = buffer[6];
            
            sensor_ads1219_init(gconfig);
            
            break;
        default:
            break;
    }
    
    return SENSOR_STATUS_IDLE;
}

void sensor_ads1219_get_config(struct sensor_gconfig_s *gconfig, uint8_t reg, uint8_t *buffer, uint8_t *length)
{
    sensor_ads1219_config_t* config = &gconfig->sensor_config.ads1219;
    
    buffer[0] = SENSOR_TYPE_ADS1219;
    buffer[1] = reg;
    
    switch(reg)
    {
        case sensor_ads1219_gloxinia_register_general:
            gconfig->measure.task.cb = sensor_ads1219_measure;
            gconfig->measure.task.data = (void*) gconfig;
            
            buffer[2] = (uint8_t) (gconfig->measure.period >> 8);
            buffer[3] = (uint8_t) gconfig->measure.period;
            
            *length = 4;
            
            break;
        case sensor_ads1219_gloxinia_register_config:
            buffer[2] = config->address;
            buffer[3] = config->enabled_channels;
            buffer[4] = config->gain;
            buffer[5] = config->fs;
            buffer[6] = config->conversion;
            buffer[7] = config->vref;
            
            *length = 8;
            
            break; 
        default:
            *length = 0;
            break;
    }
}


void sensor_ads1219_init(struct sensor_gconfig_s *gconfig)
{
    sensor_ads1219_config_t* config = &gconfig->sensor_config.ads1219;
   
    config->m_reset_data[0] = sensor_ads1219_command_reset;
    
    // send reset
    i2c_init_message(&config->m_reset,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_reset_data,
            ARRAY_LENGTH(config->m_reset_data),
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            sensor_ads1219_reset_cb,
            gconfig,
            0);

    i2c_queue_message(&config->m_reset);
}

void sensor_ads1219_reset_cb(i2c_message_t* m)
{
    sensor_gconfig_t* gconfig = (sensor_gconfig_t*) m->callback_data;
    
    //UART_DEBUG_PRINT("ADS1219 RCB");
    
    if(m->error != I2C_NO_ERROR){
        //UART_DEBUG_PRINT("ADS1219 ERROR");
        
        gconfig->status = SENSOR_STATUS_ERROR;
        
        sensor_i2c_error_handle(gconfig, m, S_ADS1219_ERROR_RESET_CB);
    }
}

void sensor_ads1219_activate(struct sensor_gconfig_s *gconfig)
{
    sensor_ads1219_config_t* config = &gconfig->sensor_config.ads1219;
    
    config->m_config_data[0] = sensor_ads1219_command_wreg;

    i2c_init_message(&config->m_config,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_config_data,
            ARRAY_LENGTH(config->m_config_data),
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            NULL,
            I2C_NO_DATA,
            0);
    
    config->m_start_data[0] = sensor_ads1219_command_start_sync;

    i2c_init_message(&config->m_start,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_start_data,
            ARRAY_LENGTH(config->m_start_data),
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            sensor_ads1219_start_cb,
            gconfig,
            0);
    
    config->m_result_write[0] = sensor_ads1219_command_rdata;

    i2c_init_message(&config->m_result,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_result_write,
            ARRAY_LENGTH(config->m_result_write),
            config->m_result_read,
            ARRAY_LENGTH(config->m_result_read),
            i2c_get_write_read_controller(config->i2c_bus),
            sensor_ads1219_result_cb,
            gconfig,
            0);
    
    UART_DEBUG_PRINT("Activating sensor ADS1219");
    
    gconfig->status = SENSOR_STATUS_RUNNING;
}

void sensor_ads1219_deactivate(struct sensor_gconfig_s *gconfig)
{
    sensor_ads1219_config_t* config = &gconfig->sensor_config.ads1219;
   
    config->m_reset_data[0] = sensor_ads1219_command_powerdown;
    
    // send reset
    i2c_init_message(&config->m_reset,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_reset_data,
            ARRAY_LENGTH(config->m_reset_data),
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            I2C_NO_CALLBACK,
            I2C_NO_DATA,
            0);

    i2c_queue_message(&config->m_reset);
}

void sensor_ads1219_measure(void *data)
{
    
    sensor_gconfig_t* gconfig = (sensor_gconfig_t*) data;
    sensor_ads1219_config_t* config = &gconfig->sensor_config.ads1219;
    
    config->selected_channel = 1;
    
    while (((config->selected_channel & config->enabled_channels) == 0) && (config->selected_channel != 0))
    {
        config->selected_channel <<= 1;
    }
    
    if(config->selected_channel == 0)
    {
        gconfig->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(gconfig);
        return;
    }
    
    config->m_config_data[1] = SENSOR_ADS1219_GET_CONFIG_REGISTER(
            sensor_ads1219_channel_to_mux(config->selected_channel), 
            config->gain, config->fs, config->conversion, config->vref);

    i2c_reset_message(&config->m_config);
    i2c_queue_message(&config->m_config);
    
    i2c_reset_message(&config->m_start);
    i2c_queue_message(&config->m_start);
}

void sensor_ads1219_start_cb(i2c_message_t* m)
{
    sensor_gconfig_t* gconfig = (sensor_gconfig_t*) m->callback_data;
    //sensor_ads1219_config_t* config = &gconfig->sensor_config.ads1219;
    
    if(m->error != I2C_NO_ERROR)
    {        
        gconfig->status = SENSOR_STATUS_ERROR;
        sensor_i2c_error_handle(gconfig, m, S_ADS1219_ERROR_START_CB);
    }
}


void sensor_ads1219_result_cb(i2c_message_t* m)
{
    sensor_gconfig_t* gconfig = (sensor_gconfig_t*) m->callback_data;
    sensor_ads1219_config_t* config = &gconfig->sensor_config.ads1219;
    
    if(m->error != I2C_NO_ERROR)
    {        
        gconfig->status = SENSOR_STATUS_ERROR;
        sensor_i2c_error_handle(gconfig, m, S_ADS1219_ERROR_RESULT_CB);
    } else {
        config->m_data[0] = config->selected_channel;
        config->m_data[1] = m->read_data[0];
        config->m_data[2] = m->read_data[1];
        config->m_data[3] = m->read_data[2];
        
        message_init(&gconfig->log,
                controller_address,
                MESSAGE_NO_REQUEST,
                M_SENSOR_DATA,
                gconfig->interface->interface_id,
                gconfig->sensor_id,
                config->m_data,
                SENSOR_ADS1219_CAN_DATA_LENGTH);
        message_send(&gconfig->log);
    }
    
    do {
        config->selected_channel <<= 1;
    } while (((config->selected_channel & config->enabled_channels) == 0) && (config->selected_channel != 0));
    
    if(config->selected_channel != 0)
    {
        config->m_config_data[1] = SENSOR_ADS1219_GET_CONFIG_REGISTER(sensor_ads1219_channel_to_mux(config->selected_channel), config->gain, config->fs, config->conversion, config->vref);

        i2c_reset_message(&config->m_config);
        i2c_queue_message(&config->m_config);
        
        i2c_reset_message(&config->m_start);
        i2c_queue_message(&config->m_start);
    }
}

bool validate_sensor_ads1219_config(sensor_ads1219_config_t* config) {
    sensor_ads1219_config_t original = *config;
    
    SENSOR_CONFIG_CHECK_MAX_SET(config->gain, sensor_ads1219_gain_4, sensor_ads1219_gain_4);
    SENSOR_CONFIG_CHECK_MAX_SET(config->fs, sensor_ads1219_1000_sps, sensor_ads1219_1000_sps);
    SENSOR_CONFIG_CHECK_MAX_SET(config->conversion, sensor_ads1219_conversion_mode_continuous, sensor_ads1219_conversion_mode_continuous);
    SENSOR_CONFIG_CHECK_MAX_SET(config->vref, sensor_ads1219_vref_external, sensor_ads1219_vref_external);
    SENSOR_CONFIG_CHECK_MAX_SET(config->enabled_channels, 0b11111111, 0b11111111);
    
    if(config->gain != original.gain){
        return false;
    }
    if(config->fs != original.fs){
        return false;
    }
    if(config->conversion != original.conversion){
        return false;
    }
    if(config->vref != original.vref){
        return false;
    }
    if(config->enabled_channels != original.enabled_channels){
        return false;
    }
    if((config->address & 0b1110000) != 0b1000000){
        return false;
    }
    
    return true;
}

sensor_ads1219_input_mux_configuration_t sensor_ads1219_channel_to_mux(uint8_t channel)
{
    switch(channel)
    {
        case 0b00000001:
        case 0b00000100:
        case 0b00010000:
        case 0b01000000:
            return sensor_ads1219_input_mux_avdd_2;
        case 0b00000010:
            return sensor_ads1219_input_mux_ain0_agnd;
        case 0b00001000:
            return sensor_ads1219_input_mux_ain1_agnd;
        case 0b00100000:
            return sensor_ads1219_input_mux_ain2_agnd;
        case 0b10000000:
            return sensor_ads1219_input_mux_ain3_agnd;
        default:
            break;
    }
    return sensor_ads1219_input_mux_avdd_2;
}

void sensor_ads1219_interrupt_cb(void* data)
{
    sensor_gconfig_t* gconfig = (sensor_gconfig_t*) data;
    sensor_ads1219_config_t* config = &gconfig->sensor_config.ads1219;
    
    i2c_reset_message(&config->m_result);
    i2c_queue_message(&config->m_result);
}
