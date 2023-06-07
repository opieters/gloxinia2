#ifndef __SENSOR_SHT35_H__
#define __SENSOR_SHT35_H__

#include <xc.h>
#include <utilities.h>
#include <i2c.h>
#include <sensor_common.h>

/// @brief SHT35 sensor I2C configuration data length
#define SENSOR_SHT35_CONFIG_DATA_LENGTH 2

/// @brief SHT35 sensor sample data length
#define SENSOR_SHT35_DATA_LENGTH 6

/// @brief SHT35 sensor CAN data length
#define SENSOR_SHT35_CAN_DATA_LENGTH SENSOR_SHT35_DATA_LENGTH

/// @brief SHT35 sensor fetch data message length
#define SENSOR_SHT35_FETCH_DATA_LENGTH 2

/// @brief SHT35 CRC polynomial
#define SHT35_CRC_POLY 0x31

/// @brief SHT35 I2C address 1
#define I2C_ADDRESS_0_SENSOR_SHT35 0x44

/// @brief SHT35 I2C address 2
#define I2C_ADDRESS_1_SENSOR_SHT35 0x45

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /// @brief SHT35 sensor configuration structure
    typedef enum
    {
        sensor_sht35_gloxinia_register_general = 0x00,
        sensor_sht35_gloxinia_register_config = 0x01,
    } sensor_sht35_gloxinia_register_t;

    /// @brief SHT35 periodicity enumeration
    typedef enum
    {
        S_SHT35_SINGLE_SHOT, ///< Single shot measurement
        S_SHT35_PERIODIC,    ///< Periodic measurement
    } sensor_sht35_periodicity_t;

    /// @brief SHT35 repeatability enumeration
    typedef enum
    {
        S_SHT35_HIGH_REPEATABILIBTY,  ///< High repeatability
        S_SHT35_MEDIUM_REPEATABILITY, ///< Medium repeatability
        S_SHT35_LOW_REPEATABILITY,    ///< Low repeatability
    } sensor_sht35_repeatability_t;

    /// @brief SHT35 clock stretching enumeration
    typedef enum
    {
        S_SHT35_ENABLE_CLOCK_STRETCHING,  ///< Enable clock stretching
        S_SHT35_DISABLE_CLOCK_STRETCHING, ///< Disable clock stretching
    } sensor_sht35_clock_stretching_t;

    /// @brief SHT35 sample rate enumeration
    typedef enum
    {
        S_SHT35_0_5_MPS, ///< 0.5 measurements per second
        S_SHT35_1_MPS,   ///< 1 measurement per second
        S_SHT35_2_MPS,   ///< 2 measurements per second
        S_SHT35_4_MPS,   ///< 4 measurements per second
        S_SHT35_10_MPS   ///< 10 measurements per second
    } sensor_sht35_sample_rate_t;

    /// @brief SHT35 error enumeration
    typedef enum
    {
        S_SHT35_ERROR_FETCH_PERIODIC,      ///< Error fetching periodic data
        S_SHT35_ERROR_READOUT_PERIODIC,    ///< Error reading out periodic data
        S_SHT35_ERROR_READOUT_SINGLE_SHOT, ///< Error reading out single shot data
        S_SHT35_ERROR_CONFIG_SINGLE_SHOT,  ///< Error configuring single shot data
        S_SHT35_ERROR_PHASE1_CB,           ///< Error in phase 1 callback
        S_SHT35_ERROR_PHASE2_CB            ///< Error in phase 2 callback
    } sensor_sht35_error_t;

    /**
     * @brief SHT35 sensor configuration structure
     *
     * @param address: I2C address of sensor
     * @param i2c_bus: I2C bus to use
     * @param repeatability: Repeatability of sensor
     * @param clock: Clock stretching of sensor
     * @param rate: Sample rate of sensor
     * @param periodicity: Periodicity of sensor
     * @param m_config: I2C message for configuration
     * @param m_config2: I2C message for configuration
     * @param m_read: I2C message for reading data
     * @param m_fetch: I2C message for fetching data
     * @param data_ready: Data ready flag
     * @param m_config_data: I2C configuration data
     * @param m_read_data: I2C read data
     * @param m_fetch_data: I2C fetch data
     */
    typedef struct sensor_sht35_config_s
    {
        uint8_t address;
        i2c_bus_t i2c_bus;

        sensor_sht35_repeatability_t repeatability;
        sensor_sht35_clock_stretching_t clock;
        sensor_sht35_sample_rate_t rate;
        sensor_sht35_periodicity_t periodicity;

        i2c_message_t m_config;
        i2c_message_t m_config2;
        i2c_message_t m_read;
        i2c_message_t m_fetch;

        bool data_ready;

        uint8_t m_config_data[SENSOR_SHT35_CONFIG_DATA_LENGTH];
        uint8_t m_read_data[SENSOR_SHT35_DATA_LENGTH];
        uint8_t m_fetch_data[SENSOR_SHT35_FETCH_DATA_LENGTH];
    } sensor_sht35_config_t;

    // forward declaration of sensor_interface_s since cyclic dependency is not
    // allowed.
    struct sensor_gconfig_s;

    /**
     * @brief Handle SHT35 sensor configuration data.
     *
     * @details Configures SHT35 sensor with given configuration parameters stored in buffer. This is internally used to handle data from messages received over CAN or USB.
     *
     * @param intf: Sensor interface
     * @param buffer: Buffer containing configuration data
     * @param length: Length of buffer
     *
     * @return sensor_status_t: Status of operation
     */
    sensor_status_t sensor_sht35_config(struct sensor_gconfig_s *config, const uint8_t *buffer, const uint8_t length);

    /**
     * @brief Write SHT35 sensor configuration to buffer.
     *
     * @details The buffer should be allocated with sufficient length (min. 7 bytes). The length is not checked.
     *
     * @param intf: Sensor interface
     * @param reg: Register to read
     * @param buffer: Buffer to write configuration data to, should have sufficient length (min. 7 bytes)
     * @param length: Length of buffer
     */
    void sensor_sht35_get_config(struct sensor_gconfig_s *config, uint8_t reg, uint8_t *buffer, uint8_t *length);

    /**
     * @brief Initialize SHT35 sensor over I2C bus.
     *
     * @details This function initializes the SHT35 sensor over I2C bus. It
     * starts the initialisation process and queues the first I2C message.
     * However, it is a non-blocking function. So it will return before the full
     * configuration is done. If an error is detected, the configuration stops
     * and the status is automatically updated to reflect this error. A message
     * will also be transmitted over the bus to notify the host application.
     *
     * @param intf: Sensor interface
     */
    void sht35_init_sensor(struct sensor_gconfig_s *intf);

    /**
     * @brief Activate SHT35 sensor readout.
     *
     * @details This schedules the SHT35 sensor for readout using the scheduler.
     * It will only do this if the sensors is not yet scheduled and if the
     * configuration was successful.
     */
    void sensor_sht35_activate(struct sensor_gconfig_s *intf);

    /**
     * @brief Prepare SHT35 measurement.
     *
     * @details Prepares I2C messages for SHT35 measurement. This will
     * first check the status of the previous readout and then start the next
     * readout over I2C. This function must match the signature of a tast
     * callback since it is automatically scheduled.
     *
     * @param data: pointer to the initerface where this sensor is connected
     */
    void sensor_sht35_measure(void *data);

    /**
     * @brief Callbacks for I2C messages.
     *
     * @details Processes I2C data produced by the SHT35 sensor and forwards it 
     * over the corresponding bus. This function will also check if there are 
     * any errors and generates messages accordingly.
     *
     * @param m: I2C message containing data received from I2C exchange
     */
    void sht35_i2c_cb_periodic_m_read(i2c_message_t *m);
    void sht35_i2c_cb_periodic_m_fetch(i2c_message_t *m);
    void sht35_i2c_cb_single_shot_m_config(i2c_message_t *m);
    void sht35_i2c_cb_single_shot_m_read(i2c_message_t *m);

    /**
     * @brief Validate SHT35 sensor configuration.
     * 
     * @param config: SHT35 sensor configuration
     * 
     * @return true if configuration is valid, false otherwise
    */
    bool validate_sht35_config(sensor_sht35_config_t *config);

    /**
     * @brief Calculate CRC for SHT35 sensor.
     * 
     * @param b: Byte to calculate CRC for
     * @param crc: Current CRC value
     * @param poly: Polynomial to use for CRC calculation (SHT35_CRC_POLY)
     * 
     * @return uint8_t: Calculated CRC
    */
    uint8_t sht35_calculate_crc(uint8_t b, uint8_t crc, uint8_t poly);

#ifdef __cplusplus
}
#endif

#endif
