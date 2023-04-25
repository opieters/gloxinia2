#include <sensor_lia.h>
#include <sensor.h>
#include <utilities.h>

void sensor_lia_get_config(struct sensor_interface_s* intf, uint8_t reg, uint8_t* buffer, uint8_t* length){
    sensor_lia_config_t *config = &intf->config.lia;
    
    buffer[0] = SENSOR_TYPE_LIA;
    buffer[1] = reg;
    
    switch(reg){
        case sensor_lia_gloxinia_register_general:
            intf->measure.task.cb = sensor_lia_measure;
            intf->measure.task.data = (void *)intf;

            buffer[2] = intf->measure.period >> 8;
            buffer[3] = intf->measure.period & 0x0ff;
            *length = 4;
            break;
        case sensor_lia_gloxinia_register_pga_config:
            *length = 7;
            break;
        default:
            *length = 0;
    }
}

sensor_status_t sensor_lia_config(struct sensor_interface_s *intf, uint8_t *buffer, uint8_t length)
{
    if (length < 1)
    {
        return SENSOR_STATUS_ERROR;
    }

    UART_DEBUG_PRINT("Configuring LIA sensor");

    sensor_lia_config_t *config = &intf->config.lia;
    
    switch(buffer[0]){
        case sensor_lia_gloxinia_register_general:
            if (length != 4){ return SENSOR_STATUS_ERROR; }

            config->mode = buffer[1];
            config->fs = (((uint16_t) buffer[2]) << 8) | buffer[3];
            
            // validate configuration
            if(!validate_lia_config(config)){
                UART_DEBUG_PRINT("Configuring LIA INVALID CONFIG");
                return SENSOR_STATUS_ERROR;
            }
            break;
            
            return SENSOR_STATUS_IDLE;
            
            break;
        case sensor_lia_gloxinia_register_pga_config:
            if(length != 4) { return SENSOR_STATUS_ERROR; }
            
            config->pga_config.channel = buffer[1];
            config->pga_config.gain = buffer[2];
            config->pga_config.status = buffer[3];
           
            // validate configuration
            if(!validate_lia_config(config)){
                UART_DEBUG_PRINT("Configuring LIA INVALID CONFIG");
                return SENSOR_STATUS_ERROR;
            } else {
                // start sensor initialisation (async) only when configuration is OK
                lia_init_sensor(intf);
            }
            break;
        default:
            break;
    }

    return SENSOR_STATUS_IDLE;
}

void sensor_lia_measure(void *data)
{
    struct sensor_interface_s *intf = (struct sensor_interface_s *)data;
    
}

void lia_init_sensor(struct sensor_interface_s *intf)
{
    // initialise PGA
    sensor_lia_config_t *config = (sensor_lia_config_t *)&intf->config.lia;
    config->pga_config.status = PGA_STATUS_ON;
    pga_init(&config->pga_config);
}


void sensor_lia_activate(sensor_interface_t* intf){
    // TODO: init ADC
    // TODO: start ADC
}

bool validate_lia_config(sensor_lia_config_t *config)
{
    bool status = true;      

    switch(config->pga_config.gain)
    {
        case PGA_GAIN_1:
        case PGA_GAIN_2:
        case PGA_GAIN_5:
        case PGA_GAIN_10:
        case PGA_GAIN_20:
        case PGA_GAIN_50:
        case PGA_GAIN_100:
        case PGA_GAIN_200:
            break;
        default:
            status = false;
    }
    
    switch(config->pga_config.channel)
    {
        case PGA_MUX_VCAL_CH0:
        case PGA_MUX_CH1:
        case PGA_CAL1:
        case PGA_CAL2:
        case PGA_CAL3:
        case PGA_CAL4:
            break;
        default:
            status = false;
    }

    switch(config->pga_config.status)
    {
        case PGA_STATUS_OFF:
        case PGA_STATUS_ON:
        case PGA_STATUS_ERROR:
            break;
        default:
            status = false;
    }
    
    switch(config->mode)
    {
        case PLANALTA_LIA_F_50KHZ:
        case PLANALTA_LIA_F_25KHZ:
        case PLANALTA_LIA_F_10KHZ:
        case PLANALTA_LIA_F_5KHZ:
        case PLANALTA_FS:
            break;
        default:
            status = false;
    }
    
    if(config->mode == PLANALTA_FS)
    {
        if(config->fs == 0)
        {
            status = false;
        }
        if(config->fs > 0b11111111111)
        {
            status = false;
        }
    }

    return status;
}
