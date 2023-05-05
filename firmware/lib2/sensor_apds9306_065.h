#ifndef __SENSOR_APDS9306_H__
#define __SENSOR_APDS9306_H__

#include <xc.h>
#include <i2c.h>
#include <sensor_common.h>

/// @brief I2C address of the APDS9306 sensor.
#define I2C_ADDRESS_SENSOR_APDS9306_065 0x52

/// @brief APDS9306_065 sample data length.
#define SENSOR_APDS3906_065_CAN_DATA_LENGTH 3

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /// @brief APDS9306 065 Gloxinia register addresses for external interface.
    typedef enum
    {
        sensor_apds9306_065_gloxinia_register_general = 0x00,
        sensor_apds9306_065_gloxinia_register_config = 0x01,
        sensor_apds9306_065_gloxinia_register_als_th_high = 0x02,
        sensor_apds9306_065_gloxinia_register_als_th_low = 0x03,
    } sensor_apds9306_065_gloxinia_register_t;

    /// @brief APDS9306 065 (raw) register addresses.
    typedef enum
    {
        SENSOR_APDS9306_065_R_MAIN_CTRL = 0x00,
        SENSOR_APDS9306_065_R_ALS_MEAS_RATE = 0x04,
        SENSOR_APDS9306_065_R_ALS_GAIN = 0x05,
        SENSOR_APDS9306_065_R_PART_ID = 0x06,
        SENSOR_APDS9306_065_R_MAIN_STATUS = 0x07,
        SENSOR_APDS9306_065_R_CLEAR_DATA_0 = 0x0A,
        SENSOR_APDS9306_065_R_CLEAR_DATA_1 = 0x0B,
        SENSOR_APDS9306_065_R_CLEAR_DATA_2 = 0x0C,
        SENSOR_APDS9306_065_R_ALS_DATA_0 = 0x0D,
        SENSOR_APDS9306_065_R_ALS_DATA_1 = 0x0E,
        SENSOR_APDS9306_065_R_ALS_DATA_2 = 0x0F,
        SENSOR_APDS9306_065_R_INT_CFG = 0x19,
        SENSOR_APDS9306_065_R_INT_PERSISTENCE = 0x1A,
        SENSOR_APDS9306_065_R_ALS_THRES_UP_0 = 0x21,
        SENSOR_APDS9306_065_R_ALS_THRES_UP_1 = 0x22,
        SENSOR_APDS9306_065_R_ALS_THRES_UP_2 = 0x23,
        SENSOR_APDS9306_065_R_ALS_THRES_LOW_0 = 0x24,
        SENSOR_APDS9306_065_R_ALS_THRES_LOW_1 = 0x25,
        SENSOR_APDS9306_065_R_ALS_THRES_LOW_2 = 0x26,
        SENSOR_APDS9306_065_R_ALS_THRES_VAR = 0x27,
    } sensor_apds9306_065_register_t;

    /// @brief APDS9306 065 ALS measurement rate values
    typedef enum
    {
        SENSOR_APDS9306_065_ALS_MEAS_RATE_25MS = 0b000,
        SENSOR_APDS9306_065_ALS_MEAS_RATE_50MS = 0b001,
        SENSOR_APDS9306_065_ALS_MEAS_RATE_100MS = 0b010,
        SENSOR_APDS9306_065_ALS_MEAS_RATE_200MS = 0b011,
        SENSOR_APDS9306_065_ALS_MEAS_RATE_500MS = 0b100,
        SENSOR_APDS9306_065_ALS_MEAS_RATE_1000MS = 0b101,
        SENSOR_APDS9306_065_ALS_MEAS_RATE_2000MS = 0b110,
    } sensor_apds9306_065_als_meas_rate_t;

    /// @brief APDS9306 065 ALS resolution values
    typedef enum
    {
        SENSOR_APDS9306_065_ALS_RESOLUTION_13BIT = 0b101,
        SENSOR_APDS9306_065_ALS_RESOLUTION_16BIT = 0b100,
        SENSOR_APDS9306_065_ALS_RESOLUTION_17BIT = 0b011,
        SENSOR_APDS9306_065_ALS_RESOLUTION_18BIT = 0b010,
        SENSOR_APDS9306_065_ALS_RESOLUTION_19BIT = 0b001,
        SENSOR_APDS9306_065_ALS_RESOLUTION_20BIT = 0b000,
    } sensor_apds9306_065_als_resolution_t;

    /// @brief APDS9306 065 ALS gain values
    typedef enum
    {
        SENSOR_APDS9306_065_ALS_GAIN_1 = 0b000,
        SENSOR_APDS9306_065_ALS_GAIN_3 = 0b001,
        SENSOR_APDS9306_065_ALS_GAIN_6 = 0b010,
        SENSOR_APDS9306_065_ALS_GAIN_9 = 0b011,
        SENSOR_APDS9306_065_ALS_GAIN_18 = 0b100,
    } sensor_apds9306_065_als_gain_t;

    /// @brief APDS9306 065 ALS status
    typedef enum
    {
        SENSOR_APDS9306_065_ALS_STATUS_ACTIVE,
        SENSOR_APDS9306_065_ALS_STATUS_STANDBY,
    } sensor_apds9306_065_als_status_t;

    /// @brief APDS9306 065 ALS interrupt variance values
    typedef enum
    {
        SENSOR_APDS9306_065_ALS_IV_8_COUNTS = 0b000,
        SENSOR_APDS9306_065_ALS_IV_16_COUNTS = 0b001,
        SENSOR_APDS9306_065_ALS_IV_32_COUNTS = 0b010,
        SENSOR_APDS9306_065_ALS_IV_64_COUNTS = 0b011,
        SENSOR_APDS9306_065_ALS_IV_128_COUNTS = 0b100,
        SENSOR_APDS9306_065_ALS_IV_256_COUNTS = 0b101,
        SENSOR_APDS9306_065_ALS_IV_512_COUNTS = 0b110,
        SENSOR_APDS9306_065_ALS_IV_1024_COUNTS = 0b111,
    } sensor_apds9306_065_als_interrupt_variance_t;

    /// @brief APDS9306 065 error codes during initialisation or readout
    typedef enum
    {
        S_APDS9306_065_ERROR_READOUT,
        S_APDS9306_065_ERROR_PHASE1_CB,
        S_APDS9306_065_ERROR_PHASE2_CB,
        S_APDS9306_065_ERROR_PHASE3_CB,
        S_APDS9306_065_ERROR_PHASE4_CB,
        S_APDS9306_065_ERROR_PHASE5_CB,
        S_APDS9306_065_ERROR_PHASE6_CB,
        S_APDS9306_065_ERROR_PHASE7_CB
    } sensor_apds9306_065_error_t;

    /** 
     * @brief APDS9306 065 sensor configuration
     * 
     * @param address I2C address of the sensor
     * @param i2c_bus I2C bus to use
     * @param meas_rate Measurement rate
     * @param meas_resolution Measurement resolution
     * @param gain Gain
     * @param als_threshold_high ALS threshold high
     * @param als_threshold_low ALS threshold low
     * @param m_config1 I2C message for configuration phase 1
     * @param m_config2 I2C message for configuration phase 2
     * @param m_read_setup I2C message for read setup
     * @param m_read I2C message for reading sensor data
     * @param m_config1_data Data for configuration phase 1
     * @param m_config2_data Data for configuration phase 2
     * @param m_read_address Data for read setup
     * @param m_read_data Data for reading sensor data
     */
    typedef struct sensor_sensor_apds9306_065_config_s
    {
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
    
    struct sensor_gconfig_s;

    /**
     * @brief Initialisation of APDS9306-065 sensor.
     *
     * @details Resets the APDS9306-065 sensor and initialises it.
     *
     * @return i2c_status_t indicating status of the I2C transfer.
     */
    sensor_status_t sensor_apds9306_065_config(struct sensor_gconfig_s *intf, uint8_t *buffer, uint8_t length);
    void sensor_apds9306_065_get_config(struct sensor_gconfig_s *intf, uint8_t reg, uint8_t *buffer, uint8_t *length);

    void sensor_apds9306_065_init_sensor(struct sensor_gconfig_s *intf);
    void sensor_apds9306_065_activate(struct sensor_gconfig_s *intf);

    /**
     * @brief Send APDS9306-065 I2C data over CAN bus.
     *
     * @details Processes I2C data received by APDS9306-065 sensor and prepares CAN
     * message for transmission.
     *
     * @param m: I2C message containing data received from I2C exchange
     */
    void sensor_apds9306_065_i2c_cb(i2c_message_t *m);

    /**
     * @brief Validate APDS9306-065 sensor configuration.
     * 
     * @param config: APDS9306-065 sensor configuration
     * 
     * @return true if configuration is valid, false otherwise
    */
    bool validate_sensor_apds9306_065_config(sensor_apds9306_065_config_t *config);

    /**
     * @brief Prepare APDS9306-065 measurement.
     * 
     * @details Prepares I2C messages for APDS9306-065 measurement. This will 
     * first check the status of the previous readout and then start the next 
     * readout over I2C. This function must match the signature of a tast 
     * callback since it is automatically scheduled.
     * 
     * @param data: pointer to the initerface where this sensor is connected
    */
    void sensor_apds9306_065_measure(void *data);

#ifdef __cplusplus
}
#endif

#endif
