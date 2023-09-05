#include <xc.h>
#include <utilities.h>
#include <dsp.h>
#include <fir_common.h>
#include <spi.h>
#include <uart.h>
#include <stdio.h>
#include <string.h>
#include <libpic30.h>
#include <address.h>
#include <sensor.h>
#include <sensor_adc16.h>

void sensor_adc16_get_config(struct sensor_gconfig_s* gsc, uint8_t reg, uint8_t* buffer, uint8_t* length){   
    buffer[0] = SENSOR_TYPE_ADC16;
    buffer[1] = reg;
    
    sensor_adc16_config_t *config = &gsc->sensor_config.adc16;
    
    switch(reg){
        case sensor_adc16_gloxinia_register_general:
            gsc->measure.task.cb = sensor_adc16_measure;
            gsc->measure.task.data = (void *)gsc;

            buffer[2] = gsc->measure.period >> 8;
            buffer[3] = gsc->measure.period & 0x0ff;
            *length = 4;
            break;
        case sensor_adc16_gloxinia_register_config:
            buffer[2] = config->average;
            *length = 3;
            break;
        case sensor_adc16_gloxinia_register_pga:
            if(config->pga != NULL){
                buffer[2] = config->pga->gain;
                *length = 3;
            } else {
                *length = 2;
            }
        default:
            *length = 0;
    }
}

sensor_status_t sensor_adc16_config(struct sensor_gconfig_s *gsc, const uint8_t *buffer, uint8_t length)
{
    if (length < 1)
    {
        return SENSOR_STATUS_ERROR;
    }

    UART_DEBUG_PRINT("Configuring ADC16 sensor");

    sensor_adc16_config_t *config = &gsc->sensor_config.adc16;
    
    switch(buffer[0]){
        case sensor_adc16_gloxinia_register_general:
            if (length != 3){ return SENSOR_STATUS_ERROR; }
            
            gsc->measure.task.cb = sensor_adc16_measure;
            gsc->measure.task.data = (void *)gsc;

            schedule_init(&gsc->measure, gsc->measure.task, (((uint16_t)buffer[1]) << 8) | buffer[2]);

            return SENSOR_STATUS_IDLE;
            
            break;
        case sensor_adc16_gloxinia_register_config:
            if(length != 2) { return SENSOR_STATUS_ERROR; }
            
            // load configuration from buffer into data structure
            config->average = buffer[1];
            // validate configuration
            if(!validate_adc16_config(config)){
                UART_DEBUG_PRINT("Configuring ADC16 INVALID CONFIG");
                return SENSOR_STATUS_ERROR;
            } else {
                // sensor init is delayed until start is called because other channels can still be configured in the meantime
            }
            break;
        case sensor_adc16_gloxinia_register_pga:
            if((config->pga == NULL) || (length != 3)){ return SENSOR_STATUS_ERROR; }
            
            config->pga->gain = buffer[1];
            config->auto_gain_setting = buffer[2];
            
            if(!validate_adc16_config(config)){
                return SENSOR_STATUS_ERROR;
            }
        default:
            break;
    }

    return SENSOR_STATUS_IDLE;
}

void sensor_adc16_measure(void *data)
{
    sensor_gconfig_t* gsc = (sensor_gconfig_t*) data;
    
    sensor_adc16_config_t *config = &gsc->sensor_config.adc16;
    uint8_t m_data[SENSOR_ADC16_CAN_DATA_LENGTH];

    // send measurement data to data sink
    m_data[0] = 0;

    if(config->average && (config->count > 0))
    {
        config->result = config->result / config->count;
        config->count = 0;
    }

    m_data[0] = (uint8_t) (config->result >> 8);
    m_data[1] = (uint8_t) (config->result);

    message_init(&gsc->log,
                 controller_address,
                 MESSAGE_NO_REQUEST,
                 M_SENSOR_DATA,
                 gsc->interface->interface_id,
                 gsc->sensor_id,
                 m_data,
                 SENSOR_ADC16_CAN_DATA_LENGTH);
    message_send(&gsc->log);
}

bool validate_adc16_config(sensor_adc16_config_t *config)
{    
    if(config->pga == NULL)
        return false;
    
    return true;
}

void sensor_adc16_activate(sensor_gconfig_t* intf)
{
    intf->status = SENSOR_STATUS_RUNNING;
}

