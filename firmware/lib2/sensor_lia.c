#include <sensor_lia.h>
#include <sensor.h>
#include <utilities.h>
#include <address.h>
#include <adc16.h>
#include <dac.h>
#include "../planalta2.X/planalta.h"

extern adc16_config_t adc16_config;
extern dac_config_t dac_config;

void sensor_lia_get_config(sensor_gconfig_t* intf, uint8_t reg, uint8_t* buffer, uint8_t* length){
    sensor_lia_config_t *config = &intf->sensor_config.lia;
    
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
        case sensor_lia_gloxinia_register_config:
            buffer[2] = config->mode;
            buffer[3] = config->fs_high;
            buffer[4] = config->fs_low;
            buffer[5] = config->ouput_enable;
            *length = 6;
            break;
        case sensor_lia_gloxinia_register_pga_config:
            buffer[2] = config->pga_config->gain;
            buffer[3] = config->auto_gain;
            
            *length = 4;
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
            if(length != 3) { return SENSOR_STATUS_ERROR; }
            
            intf->measure.task.cb = sensor_lia_measure;
            intf->measure.task.data = (void*) config;
                    
            schedule_init(&intf->measure, intf->measure.task, (((uint16_t) buffer[1]) << 8) | buffer[2]);
            break;
        case sensor_lia_gloxinia_register_config:
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
        case sensor_lia_gloxinia_register_pga_config:
            if((length != 3) || (config->pga_config == NULL)) { return SENSOR_STATUS_ERROR; }
            
            config->pga_config->gain = buffer[1];
            config->auto_gain = buffer[2];
           
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
    sensor_gconfig_t* gsc = (sensor_gconfig_t*) data;
    
    sensor_lia_config_t *config = &gsc->sensor_config.lia;
    uint8_t m_data[SENSOR_LIA_CAN_DATA_LENGTH];

    // send measurement data to data sink
    m_data[0] = (uint8_t) (config->sample_i >> 8);
    m_data[1] = (uint8_t) (config->sample_i);
    m_data[2] = (uint8_t) (config->sample_q >> 8);
    m_data[3] = (uint8_t) (config->sample_q);
    
    if(config->ouput_enable){
        m_data[4] = (uint8_t) (config->source_i >> 8);
        m_data[5] = (uint8_t) (config->source_i);
        m_data[6] = (uint8_t) (config->source_q >> 8);
        m_data[7] = (uint8_t) (config->source_q);
    } else {
        m_data[4] = 0;
        m_data[5] = 0;
        m_data[6] = 0;
        m_data[7] = 0;
    }

    message_init(&gsc->log,
                 controller_address,
                 MESSAGE_NO_REQUEST,
                 M_SENSOR_DATA,
                 gsc->interface->interface_id,
                 gsc->sensor_id,
                 m_data,
                 SENSOR_LIA_CAN_DATA_LENGTH);
    message_send(&gsc->log);
    
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


void lia_init_output_driver(sensor_gconfig_t* config)
{    
    // update actual hardware
    pga_update_status(config->sensor_config.lia.pga_config, PGA_STATUS_ON);

    if(config->sensor_config.lia.ouput_enable)
    {
        switch(config->interface->interface_id)
        {
            case 0:
                _RP98R = _RPOUT_OC2;   // DRV1 signal
                break;
            case 1:
                _RP99R = _RPOUT_OC2;   // DRV2 signal
                break;
            case 2:
                _RP82R = _RPOUT_OC2;  // DRV3 signal
                break;
            case 3:
                _RP84R = _RPOUT_OC2;  // DRV4 signal
                break;
            default:
                break;
        }
    } else {
        switch(config->interface->interface_id)
        {
            case 0:
                _RP98R = 0;   // DRV1 signal
                _RF2 = 0;
                break;
            case 1:
                _RP99R = 0;   // DRV2 signal
                _RF3 = 0;
                break;
            case 2:
                _RP82R = 0;  // DRV3 signal
                _RF4 = 0;
                break;
            case 3:
                _RP84R = 0;  // DRV4 signal
                _RF5 = 0;
                break;
            default:
                break;
        }
    }
}


void sensor_lia_activate(sensor_gconfig_t* config)
{
    // updates PGA and configures output pin
    lia_init_output_driver(config);
    
    // configure DAC only once
    planalta_init_dac_config(
            &dac_config, 
            config->sensor_config.lia.mode,
            config->sensor_config.lia.fs_low,
            false); // TODO: check if false is the right choice here
    
    // start ADC only once
    if(adc16_config.status != ADC16_STATUS_ON)
    {
        adc16_init(&adc16_config);
        adc16_start(&adc16_config);
    }   
    
    config->status = SENSOR_STATUS_RUNNING;
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
