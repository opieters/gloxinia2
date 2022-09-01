#ifndef __SENSOR_SYLVATICA2_H__
#define	__SENSOR_SYLVATICA2_H__

#include <xc.h>
#include <dsp.h>
#include <utilities.h>
#include <i2c.h>
#include "sensor_common.h"
#include "dicio.h"

/**
 * Write addresses of I2C sensors and modules
 */
#define I2C_ADDRESS_0_SENSOR_SYLVATICA2 (SYLVATICA2_I2C_BASE_ADDRESS | 0b000)
#define I2C_ADDRESS_1_SENSOR_SYLVATICA2 (SYLVATICA2_I2C_BASE_ADDRESS | 0b001)
#define I2C_ADDRESS_2_SENSOR_SYLVATICA2 (SYLVATICA2_I2C_BASE_ADDRESS | 0b010)
#define I2C_ADDRESS_3_SENSOR_SYLVATICA2 (SYLVATICA2_I2C_BASE_ADDRESS | 0b011)
#define I2C_ADDRESS_4_SENSOR_SYLVATICA2 (SYLVATICA2_I2C_BASE_ADDRESS | 0b100)
#define SENSOR_SYLVATICA2_CAN_DATA_LENGTH 5

/**
 * @brief Indicate which ports are input and output ports.
 * 
 * @details `0` indicates an output port and `1` indicates an input port.
 */


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sensor_sylvatica2_config_s sensor_sylvatica2_config_t;
    
typedef struct sensor_sylvatica2_channel_config_s {
    sensor_status_t status;
    
    pga_config_t gain;
    
    i2c_message_t m_ch_config;
    uint8_t m_ch_config_data[2];
} sensor_sylvatica2_channel_config_t;

typedef struct sensor_sylvatica2_raw_channel_config_s {    
    i2c_message_t m_write, m_read;
    
    sensor_status_t status;
    
    uint8_t m_write_data[1];
    uint8_t* m_read_data;
} sensor_sylvatica2_raw_channel_config_t;


    
struct sensor_sylvatica2_config_s {
    sensor_general_config_t general;

    i2c_message_t m_status_config;
    uint8_t m_status_config_data[2];
    
    i2c_message_t m_adc_config;
    uint8_t m_adc_config_data[2];
    
    i2c_message_t m;
    uint8_t m_data[1];
    
    i2c_message_t m2;
    uint8_t m2_data[36];
    
    sensor_sylvatica2_channel_config_t channels[8];
    sensor_sylvatica2_raw_channel_config_t raw_channels[8];
    
    uint16_t sample_time;
};


/**
 * @brief Initialisation of BH1721FVC sensor.
 * 
 * @details Resets the BH1721FVC sensor and initialises it.
 * 
 * @return i2c_status_t indicating status of the I2C transfer.
 */
void sylvatica2_sensor_init_channel(sensor_sylvatica2_channel_config_t* config, 
        sensor_sylvatica2_config_t* sensor_config,
        const uint8_t channel_n);
void sylvatica2_sensor_init_raw_channel(sensor_sylvatica2_raw_channel_config_t* config, 
        sensor_sylvatica2_config_t* sensor_config, 
        const uint8_t channel_n);
i2c_error_t sylvatica2_sensor_init(sensor_sylvatica2_config_t* config);

void sylvatica2_measurement_read_cb(i2c_message_t* m);
void sylvatica2_measurement_raw_read_cb(i2c_message_t* m);
void sylvatica2_sensor_read(sensor_sylvatica2_config_t* config);

void sylvatica2_i2c1_write_read_controller(i2c_message_t* m);
void sylvatica2_i2c2_write_read_controller(i2c_message_t* m);

void sylvatica2_sensor_start(sensor_sylvatica2_config_t* config);

void sylvatica2_read_channel_config(sensor_sylvatica2_channel_config_t* config, 
        sensor_sylvatica2_config_t* sensor_config,
        const uint8_t channel_n);

#ifdef	__cplusplus
}
#endif


#endif

