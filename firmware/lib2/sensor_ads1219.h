#ifndef __SENSOR_ADS1219_H__
#define	__SENSOR_ADS1219_H__

#include <xc.h>
#include <i2c.h>
#include <dsp.h>
#include <sensor_common.h>

#define SENSOR_ADS1219_CAN_DATA_LENGTH 4
#define SENSOR_ADS1219_DATA_LENGTH 4
#define SENSOR_ADS1219_GET_CONFIG_REGISTER(M, G, D, C, V) ( ((M) << 5) | ( (G) << 4) | ( (D) << 2) | ( (C) << 1) | (V) )

/// @brief Commands sent over I2C to ADS1219 (don't care is implemented as 0)
typedef enum {
    sensor_ads1219_command_reset       = 0b00000110,
    sensor_ads1219_command_start_sync  = 0b00001000,
    sensor_ads1219_command_powerdown   = 0b00000010,
    sensor_ads1219_command_rdata       = 0b00010000,
    sensor_ads1219_command_rreg        = 0b00100000,
    sensor_ads1219_command_wreg        = 0b01000000,
} sensor_ads1219_command_t;

/// @brief ADS1219 mux configuration
typedef enum {
    sensor_ads1219_input_mux_ain0_ain1 = 0b000,
    sensor_ads1219_input_mux_ain2_ain1 = 0b000,
    sensor_ads1219_input_mux_ain1_ain3 = 0b001,
    sensor_ads1219_input_mux_ain0_ain2 = 0b010,
    sensor_ads1219_input_mux_ain0_agnd = 0b011,
    sensor_ads1219_input_mux_ain1_agnd = 0b100,
    sensor_ads1219_input_mux_ain2_agnd = 0b101,
    sensor_ads1219_input_mux_ain3_agnd = 0b110,
    sensor_ads1219_input_mux_avdd_2    = 0b111,
} sensor_ads1219_input_mux_configuration_t;

/// @brief ADS1219 gain configuration
typedef enum {
    sensor_ads1219_gain_1 = 0b0,
    sensor_ads1219_gain_4 = 0b1,
} sensor_ads1219_gain_configuration_t;

/// @brief ADS1219 data readout rate in samples per second (SPS)
typedef enum {
    sensor_ads1219_20_sps   = 0b00,
    sensor_ads1219_90_sps   = 0b01,
    sensor_ads1219_330_sps  = 0b10,
    sensor_ads1219_1000_sps = 0b11,
} sensor_ads1219_fs_configuration_t;

/// @brief ADS1219 conversion mode
typedef enum {
    sensor_ads1219_conversion_mode_single_shot = 0b0,
    sensor_ads1219_conversion_mode_continuous = 0b1
} sensor_ads1219_conversion_mode_t;

/// @brief ADS1219 voltage reference selection
typedef enum {
    sensor_ads1219_vref_internal = 0b0,
    sensor_ads1219_vref_external = 0b1,
} sensor_ads1219_vref_t;

/// @brief ADS1219 register selection, has to be used in combination with
/// sensor_ads1219_command_rreg and sensor_ads1219_command_wreg using the
/// bitwise or operator (|)
typedef enum {
    sensor_ads1219_register_0 = 0b00000000,
    sensor_ads1219_register_1 = 0b00000100,
} sensor_ads1219_register_t;

/// @brief Gloxinia configuration registers (with computer)
typedef enum {
    sensor_ads1219_gloxinia_register_general = 0x00,
    sensor_ads1219_gloxinia_register_config = 0x01,
    sensor_ads1219_gloxinia_register_channels = 0x02,
} sensor_ads1219_gloxinia_register_t;

/// @brief ADS1219 error codes during initialisation or readout
typedef enum
{
    S_ADS1219_ERROR_CONFIG,
    S_ADS1219_ERROR_STATUS_A,
    S_ADS1219_ERROR_STATUS_B,
    S_ADS1219_ERROR_READOUT
} sensor_ads1219_error_t;

/**
 * @brief ADS1219 sensor configuration
 * 
 * @param address I2C address of the sensor
 * @param i2c_bus I2C bus of the sensor
 * @param enabled_channels Select which channels to scan, even bits are offset calibration bits
 * @param selected_channel Internal variable to keep track of selected channel
 * @param gain Selected gain
 * @param fs Conversion frequency
 * @param conversion Conversion mode
 * @param vref Voltage reference setting
 * @param m_config I2C message for mux configuration
 * @param m_status_check_a I2C message to check conversion status (A)
 * @param m_status_check_b I2C message to check conversion status (B)
 * @param m_result I2C message to read conversion result
 * @param m_config_data Data for configuration
 * @param m_status_data_write Write data for status check
 * @param m_status_data_read Read data for status check
 * @param m_result_write Write data for conversion result
 * @param m_result_read Read data for conversion result
 * @param m_data
 */
typedef struct {
    uint8_t address;
    i2c_bus_t i2c_bus;
    
    uint8_t enabled_channels;
    uint8_t selected_channel;
    
    sensor_ads1219_gain_configuration_t gain;
    sensor_ads1219_fs_configuration_t fs;
    sensor_ads1219_conversion_mode_t conversion;
    sensor_ads1219_vref_t vref;
    
    i2c_message_t m_config;
    i2c_message_t m_status_check_a;
    i2c_message_t m_status_check_b;
    i2c_message_t m_result;
    
    uint8_t m_config_data[2];
    uint8_t m_status_data_write[1];
    uint8_t m_status_data_read[1];
    uint8_t m_result_write[1];
    uint8_t m_result_read[3];
    
    uint8_t m_data[SENSOR_ADS1219_CAN_DATA_LENGTH];
} sensor_ads1219_config_t;


/// @brief Forward-declaration of general configuration structure.
struct sensor_gconfig_s;

#ifdef	__cplusplus
extern "C" {
#endif
    
    /**
     * @brief Handle ADS1219 sensor configuration data.
     *
     * @details Configures ADS1219 sensor with given configuration parameters stored in buffer. This is internally used to handle data from messages received over CAN or USB.
     *
     * @param intf: Sensor interface
     * @param buffer: Buffer containing configuration data
     * @param length: Length of buffer
     *
     * @return sensor_status_t: Status of operation
     */
    sensor_status_t sensor_ads1219_config(struct sensor_gconfig_s *config, const uint8_t *buffer, const uint8_t length);

    /**
     * @brief Write ADS1219 sensor configuration to buffer.
     *
     * @details The buffer should be allocated with sufficient length (min. 7 bytes). The length is not checked.
     *
     * @param intf: Sensor interface
     * @param reg: Register to read
     * @param buffer: Buffer to write configuration data to, should have sufficient length (min. 7 bytes)
     * @param length: Length of buffer
     */
    void sensor_ads1219_get_config(struct sensor_gconfig_s *config, uint8_t reg, uint8_t *buffer, uint8_t *length);

    /**
     * @brief Initialize ADS1219 sensor over I2C bus.
     *
     * @details This function initializes the ADS1219 sensor over I2C bus. It
     * starts the initialisation process and queues the first I2C message.
     * However, it is a non-blocking function. So it will return before the full
     * configuration is done. If an error is detected, the configuration stops
     * and the status is automatically updated to reflect this error. A message
     * will also be transmitted over the bus to notify the host application.
     *
     * @param intf: Sensor interface
     */
    void sensor_ads1219_init(struct sensor_gconfig_s *intf);

    /**
     * @brief Activate ADS1219 sensor readout.
     *
     * @details This schedules the ADS1219 sensor for readout using the scheduler.
     * It will only do this if the sensors is not yet scheduled and if the
     * configuration was successful.
     */
    void sensor_ads1219_activate(struct sensor_gconfig_s *intf);
    
    /**
     * @brief Deactivate ADS1219 sensor readout.
     *
     * @details This powers down the ADS1219.
     */
    void sensor_ads1219_deactivate(struct sensor_gconfig_s *intf);

    /**
     * @brief Prepare ADS1219 measurement.
     *
     * @details Prepares I2C messages for ADS1219 measurement. This will
     * first check the status of the previous readout and then start the next
     * readout over I2C. This function must match the signature of a tast
     * callback since it is automatically scheduled.
     *
     * @param data: pointer to the initerface where this sensor is connected
     */
    void sensor_ads1219_measure(void *data);

#ifdef	__cplusplus
}
#endif

#endif

