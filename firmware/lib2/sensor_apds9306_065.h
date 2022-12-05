#ifndef __SENSOR_APDS9306_H__
#define	__SENSOR_APDS9306_H__

#include <xc.h>
#include <i2c.h>
#include <sensor_common.h>

#define I2C_ADDRESS_SENSOR_APDS9306_065     0x52
#define SENSOR_APDS3906_098_CAN_DATA_LENGTH 3

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
    
    typedef enum {
        sensor_apds9306_065_gloxinia_register_general = 0x00,
        sensor_apds9306_065_gloxinia_register_config = 0x01,
        sensor_apds9306_065_gloxinia_register_als_th_high = 0x02,
        sensor_apds9306_065_gloxinia_register_als_th_low = 0x03,
    } sensor_apds9306_065_gloxinia_register_t;

typedef enum {
    SENSOR_APDS9306_065_R_MAIN_CTRL        = 0x00,
    SENSOR_APDS9306_065_R_ALS_MEAS_RATE    = 0x04,
    SENSOR_APDS9306_065_R_ALS_GAIN         = 0x05,
    SENSOR_APDS9306_065_R_PART_ID          = 0x06,
    SENSOR_APDS9306_065_R_MAIN_STATUS      = 0x07,
    SENSOR_APDS9306_065_R_CLEAR_DATA_0     = 0x0A,
    SENSOR_APDS9306_065_R_CLEAR_DATA_1     = 0x0B,
    SENSOR_APDS9306_065_R_CLEAR_DATA_2     = 0x0C,
    SENSOR_APDS9306_065_R_ALS_DATA_0       = 0x0D,
    SENSOR_APDS9306_065_R_ALS_DATA_1       = 0x0E,
    SENSOR_APDS9306_065_R_ALS_DATA_2       = 0x0F,
    SENSOR_APDS9306_065_R_INT_CFG          = 0x19,
    SENSOR_APDS9306_065_R_INT_PERSISTENCE  = 0x1A,
    SENSOR_APDS9306_065_R_ALS_THRES_UP_0   = 0x21,
    SENSOR_APDS9306_065_R_ALS_THRES_UP_1   = 0x22,
    SENSOR_APDS9306_065_R_ALS_THRES_UP_2   = 0x23,
    SENSOR_APDS9306_065_R_ALS_THRES_LOW_0  = 0x24,
    SENSOR_APDS9306_065_R_ALS_THRES_LOW_1  = 0x25,
    SENSOR_APDS9306_065_R_ALS_THRES_LOW_2  = 0x26,
    SENSOR_APDS9306_065_R_ALS_THRES_VAR    = 0x27,
} sensor_apds9306_065_register_t;

typedef enum {
    SENSOR_APDS9306_065_ALS_MEAS_RATE_25MS = 0b000,
    SENSOR_APDS9306_065_ALS_MEAS_RATE_50MS = 0b001,
    SENSOR_APDS9306_065_ALS_MEAS_RATE_100MS = 0b010,
    SENSOR_APDS9306_065_ALS_MEAS_RATE_200MS = 0b011,
    SENSOR_APDS9306_065_ALS_MEAS_RATE_500MS = 0b100,
    SENSOR_APDS9306_065_ALS_MEAS_RATE_1000MS = 0b101,
    SENSOR_APDS9306_065_ALS_MEAS_RATE_2000MS = 0b110,   
} sensor_apds9306_065_als_meas_rate_t;

typedef enum {
    SENSOR_APDS9306_065_ALS_RESOLUTION_13BIT = 0b101,
    SENSOR_APDS9306_065_ALS_RESOLUTION_16BIT = 0b100,
    SENSOR_APDS9306_065_ALS_RESOLUTION_17BIT = 0b011,
    SENSOR_APDS9306_065_ALS_RESOLUTION_18BIT = 0b010,
    SENSOR_APDS9306_065_ALS_RESOLUTION_19BIT = 0b001,
    SENSOR_APDS9306_065_ALS_RESOLUTION_20BIT = 0b000,
} sensor_apds9306_065_als_resolution_t;

typedef enum {
    SENSOR_APDS9306_065_ALS_GAIN_1 = 0b000,
    SENSOR_APDS9306_065_ALS_GAIN_3 = 0b001,
    SENSOR_APDS9306_065_ALS_GAIN_6 = 0b010,
    SENSOR_APDS9306_065_ALS_GAIN_9 = 0b011,
    SENSOR_APDS9306_065_ALS_GAIN_18 = 0b100,
} sensor_apds9306_065_als_gain_t;

typedef enum {
    SENSOR_APDS9306_065_ALS_STATUS_ACTIVE,
    SENSOR_APDS9306_065_ALS_STATUS_STANDBY,
} sensor_apds9306_065_als_status_t;

typedef enum {
    SENSOR_APDS9306_065_ALS_IV_8_COUNTS = 0b000,
    SENSOR_APDS9306_065_ALS_IV_16_COUNTS = 0b001,    
    SENSOR_APDS9306_065_ALS_IV_32_COUNTS = 0b010,  
    SENSOR_APDS9306_065_ALS_IV_64_COUNTS = 0b011,  
    SENSOR_APDS9306_065_ALS_IV_128_COUNTS = 0b100,  
    SENSOR_APDS9306_065_ALS_IV_256_COUNTS = 0b101,  
    SENSOR_APDS9306_065_ALS_IV_512_COUNTS = 0b110,  
    SENSOR_APDS9306_065_ALS_IV_1024_COUNTS = 0b111,  
} sensor_apds9306_065_als_interrupt_variance_t;

typedef struct sensor_sensor_apds9306_065_config_s {
    uint8_t address;
    i2c_bus_t i2c_bus;
    
    sensor_apds9306_065_als_meas_rate_t meas_rate;
    sensor_apds9306_065_als_resolution_t meas_resolution;
    sensor_apds9306_065_als_gain_t gain;
    
    uint32_t als_threshold_high;
    uint32_t als_threshold_low;

    i2c_message_t m_config1;
    i2c_message_t m_config2;
    i2c_message_t m_read_setup;
    i2c_message_t m_read;
    
    uint8_t m_config1_data[4];
    uint8_t m_config2_data[4];
    uint8_t m_read_address[1];
    uint8_t m_read_data[3];
} sensor_apds9306_065_config_t;

struct sensor_interface_s;

/**
 * @brief Initialisation of APDS9306-065 sensor.
 * 
 * @details Resets the APDS9306-065 sensor and initialises it.
 * 
 * @return i2c_status_t indicating status of the I2C transfer.
 */
sensor_status_t sensor_apds9306_065_config(struct sensor_interface_s* intf, uint8_t* buffer, uint8_t length);
void sensor_apds9306_065_get_config(struct sensor_interface_s* intf, uint8_t reg, uint8_t* buffer, uint8_t* length);

void sensor_apds9306_065_init_sensor(struct sensor_interface_s* intf);
void sensor_apds9306_065_activate(struct sensor_interface_s* intf);

/**
 * @brief Send APDS9306-065 I2C data over CAN bus.
 * 
 * @details Processes I2C data received by APDS9306-065 sensor and prepares CAN 
 * message for transmission.
 * 
 * @param m: I2C message containing data received from I2C exchange
 */
void sensor_apds9306_065_i2c_cb(i2c_message_t* m);

bool validate_sensor_apds9306_065_config(sensor_apds9306_065_config_t* config);

void sensor_apds9306_065_measure(void* data);

#ifdef	__cplusplus
}
#endif 

#endif

