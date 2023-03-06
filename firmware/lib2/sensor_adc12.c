#include <adc12.h>
#include <sensor.h>
#include <address.h>
#include <utilities.h>


void sensor_adc12_get_config(struct sensor_interface_s* intf, uint8_t reg, uint8_t* buffer, uint8_t* length){
    sensor_adc12_config_t *config = &intf->config.sht35;
    
    buffer[0] = SENSOR_TYPE_ADC12;
    buffer[1] = reg;
    
    switch(reg){
        case sensor_adc12_gloxinia_register_general:
            intf->measure.task.cb = sensor_adc12_measure;
            intf->measure.task.data = (void *)intf;

            buffer[2] = intf->measure.period >> 8;
            buffer[3] = intf->measure.period & 0x0ff;
            *length = 4;
            break;
        case sensor_adc12_gloxinia_register_config:
            buffer[2] = config->address;
            buffer[3] = config->repeatability;
            buffer[4] = config->clock;
            buffer[5] = config->rate;
            buffer[6] = config->periodicity;
            *length = 7;
            break;
        default:
            *length = 0;
    }
}

sensor_status_t sensor_adc12_config(struct sensor_interface_s *intf, uint8_t *buffer, uint8_t length)
{
    if (length < 1)
    {
        return SENSOR_STATUS_ERROR;
    }

    UART_DEBUG_PRINT("Configuring ADC12");

    sensor_adc12_config_t *config = &intf->config.sht35;
    
    config->data_ready = false;
    
    switch(buffer[0]){
        case sensor_adc12_gloxinia_register_general:
            if (length != 3){ return SENSOR_STATUS_ERROR; }
            
            intf->measure.task.cb = sensor_adc12_measure;
            intf->measure.task.data = (void *)intf;

            schedule_init(&intf->measure, intf->measure.task, (((uint16_t)buffer[1]) << 8) | buffer[2]);

            return SENSOR_STATUS_IDLE;
            
            break;
        case sensor_adc12_gloxinia_register_config:
            if(length != 6) { return SENSOR_STATUS_ERROR; }
            
            // load configuration from buffer into data structure
            // TODO

            // validate configuration
            if(!validate_adc12_config(config)){
                UART_DEBUG_PRINT("Configuring ADC12 INVALID CONFIG");
                return SENSOR_STATUS_ERROR;
            } else {
                // start sensor initialisation (async) only when configuration is OK
                adc12_init_sensor(intf);
            }
            break;
        default:
            break;
    }

    return SENSOR_STATUS_IDLE;
}

void sensor_adc12_measure(void *data)
{
    struct sensor_interface_s *intf = (struct sensor_interface_s *)data;

    if (intf->config.sht35.periodicity == S_SHT35_SINGLE_SHOT)
    {
        if (i2c_check_message_sent(&intf->config.sht35.m_read) &&
            i2c_check_message_sent(&intf->config.sht35.m_config))
        {
            i2c_reset_message(&intf->config.sht35.m_read, 1);
            i2c_queue_message(&intf->config.sht35.m_read);
            i2c_reset_message(&intf->config.sht35.m_config, 1);
            i2c_queue_message(&intf->config.sht35.m_config);
        }
        else
        {
            UART_DEBUG_PRINT("SHT35 on %x not fully processed.", intf->sensor_id);
            intf->config.sht35.m_read.status = I2C_MESSAGE_CANCELED;
            intf->config.sht35.m_config.status = I2C_MESSAGE_CANCELED;

            intf->status = SENSOR_STATUS_ERROR;
            sensor_error_handle(intf);
        }
    }
    else
    {
        if (i2c_check_message_sent(&intf->config.sht35.m_fetch))
        {
            i2c_reset_message(&intf->config.sht35.m_fetch, 1);
            i2c_queue_message(&intf->config.sht35.m_fetch);
        }
        else
        {
            UART_DEBUG_PRINT("SHT35 %x not fully processed.", intf->sensor_id);
            intf->status = SENSOR_STATUS_ERROR;

            sensor_error_handle(intf);
        }
    }
}

void adc12_init_sensor(struct sensor_interface_s *intf)
{
    AD1CON1bits.ADON = 0;
    AD1CON1bits.ADDMABM = 0; // ???
    AD12B = 1; // 12-bit mode
    FORM = 0b10; // use fractional format (data in MSB, LSB 0)
    SSRC = 0b000; // PWM Generator 1 primary trigger compare ends sampling and starts conversion
}


void sensor_adc12_activate(sensor_interface_t* intf){
    intf->status = SENSOR_STATUS_RUNNING;
    
    // Start ADC
    AD1CON1bits.ADON = 1;
}


bool validate_adc12_config(sensor_adc12_config_t *config)
{
    sensor_adc12_config_t original = *config;
    
    return true;
}
