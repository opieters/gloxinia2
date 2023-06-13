#include <sensor_lia.h>
#include <sensor.h>
#include <utilities.h>

void sensor_lia_get_config(sensor_gconfig_t* intf, uint8_t reg, uint8_t* buffer, uint8_t* length){
    //sensor_lia_config_t *config = &intf->sensor_config.lia;
    
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

sensor_status_t sensor_lia_config(sensor_gconfig_t *intf, const uint8_t *buffer, const uint8_t length)
{
    if (length < 1)
    {
        return SENSOR_STATUS_ERROR;
    }

    UART_DEBUG_PRINT("Configuring LIA sensor");

    sensor_lia_config_t *config = &intf->sensor_config.lia;
    
    switch(buffer[0]){
        case sensor_lia_gloxinia_register_general:
            if (length != 5){ return SENSOR_STATUS_ERROR; }

            config->mode = buffer[1];
            config->fs_high = buffer[2];
            config->fs_low = buffer[3];
            config->ouput_enable = buffer[4];
            
            // validate configuration
            if(!validate_lia_config(config)){
                UART_DEBUG_PRINT("Configuring LIA INVALID CONFIG");
                return SENSOR_STATUS_ERROR;
            }
            break;
            
            return SENSOR_STATUS_IDLE;
            
            break;
        case sensor_lia_gloxinia_register_pga_config:
            if((length != 2) || (config->pga_config == NULL)) { return SENSOR_STATUS_ERROR; }
            
            config->pga_config->gain = buffer[1];
           
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
    //struct sensor_gconfig_s *intf = (struct sensor_gconfig_s *)data;
    
}

void lia_init_sensor(sensor_gconfig_t *intf)
{
    // initialise PGA
    sensor_lia_config_t *config = (sensor_lia_config_t *)&intf->sensor_config.lia;
    if(config->pga_config != NULL){
        config->pga_config->status = PGA_STATUS_ON;
        pga_init(config->pga_config);
    }
}


void sensor_lia_activate(sensor_gconfig_t* intf){
    // TODO: init ADC
    // TODO: start ADC
}

bool validate_lia_config(sensor_lia_config_t *config)
{
    bool status = true;      

    if(config->pga_config != NULL){
        switch(config->pga_config->gain)
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
        if(config->fs_low > config->fs_high)
        {
            status = false;
        }
        if(config->fs_low > PLANALTA_FS_FREQ_50KHZ)
        {
            status = false;
        }
        if(config->fs_high > PLANALTA_FS_FREQ_50KHZ)
        {
            status = false;
        }
    }

    return status;
}
