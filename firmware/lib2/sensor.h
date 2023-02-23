#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <xc.h>
#include <can.h>
#include <i2c.h>
#include <sensor_sht35.h>
#include <sensor_apds9306_065.h>
#include <sensor_analog.h>
#include <sensor_common.h>
#include <event_controller.h>

/// @brief Number of sensor interfaces of dicio board
#define DICIO_N_SENSOR_IFS 4

/// @brief Number of sensor interfaces of planalta board
#define PLANALTA_N_SENSOR_IFS 4

/// @brief Number of sensor interfaces of sylvatica board
#define SYLVATICA_N_SENSOR_IFS 8

/// @brief Number of sensor interfaces (general definition)
#ifdef __SYLVATICA__
#define N_SENSOR_INTERFACES SYLVATICA_N_SENSOR_IFS
#elif defined __PLANALTA__
#define N_SENSOR_INTERFACES PLANALTA_N_SENSOR_IFS
#else
#define N_SENSOR_INTERFACES DICIO_N_SENSOR_IFS
#endif

/// @brief Sensor configuration structure union
typedef union sensor_config_s
{
    sensor_sht35_config_t sht35;               ///< SHT35 sensor configuration
    sensor_analogue_config_t analogue;         ///< Analogue sensor configuration
    sensor_apds9306_065_config_t apds9306_065; ///< APDS9306-065 sensor configuration
} sensor_config_t;

/**
 * @brief Sensor interface structure
 *
 * @param sensor_type Sensor type
 * @param sensor_id Sensor ID
 * @param config Sensor configuration
 * @param status Sensor status
 * @param measure Task schedule for measurement
 * @param log message for logging
 * @param log_data message data for logging
 */
typedef struct sensor_interface_s
{
    sensor_type_t sensor_type;
    uint8_t sensor_id;
    sensor_config_t config;

    sensor_status_t status;

    task_schedule_t measure;

    message_t log;
    uint8_t log_data[CAN_MAX_N_BYTES];
} sensor_interface_t;

/// @brief // Sensor interface array
extern sensor_interface_t sensor_interfaces[N_SENSOR_INTERFACES];

#ifdef __cplusplus
extern "C"
{
#endif

    /// @brief Initialise sensor interfaces to default values
    void sensors_init(void);

    /**
     * @brief Set sensor configuration from buffer
     *
     * @param interface_id Sensor interface ID
     * @param buffer Buffer containing configuration data
     * @param length Length of buffer
     */
    void sensor_set_config_from_buffer(uint8_t interface_id, uint8_t *buffer, uint8_t length);

    /**
     * @brief Write sensor configuration to buffer.
     *
     * @details The buffer should be allocated with sufficient length (min. 8 bytes). The length is not checked. 8 bytes is the maximum length of the data in an ECAN message, so this is sufficient.
     *
     * @param intf: Sensor interface
     * @param reg: Register to read
     * @param buffer: Buffer to write configuration data to, should have sufficient length (min. 7 bytes)
     * @param length: Length of buffer
     */
    void sensor_get_config(uint8_t interface_id, uint8_t reg, uint8_t *buffer, uint8_t *length);

    /**
     * @brief Get I2C bus of this sensor interface
     *
     * @param interface_id Sensor interface ID
     * @return i2c_bus_t I2C bus
     */
    i2c_bus_t sensor_get_i2c_bus(uint8_t interface_id);

    /**
     * @brief Sends error message
     *
     * @details Logs an error that occurred on this sensor interface.
     *
     * @param intf Sensor interface
     * @param data Error data
     * @param length Length of error data
     */
    void sensor_error_log(sensor_interface_t *intf, uint8_t *data, uint8_t length);

    /**
     * @brief Handle sensor error (general method)
     *
     * @param intf Sensor interface
     *
     * @details This method is called when an error occurs on a sensor interface. It logs the error by transmitting the current sensor status. It also removes the sensor from the measurement schedule.
     */
    void sensor_error_handle(sensor_interface_t *intf);

    /**
     * @brief Handle sensor error (I2C)
     *
     * @details More specific version of sensor_error_handle() for I2C errors. The error message also includes the error status of the I2C message and location where the error occurred.
     *
     * @param intf Sensor interface
     * @param m I2C message
     * @param location Location where error occurred
     */
    void sensor_i2c_error_handle(sensor_interface_t *intf, i2c_message_t *m, uint8_t location);

    /**
     * @brief Updates sensor status to SENSOR_STATUS_ACTIVE
     */
    void sensor_start(void);

    /**
     * @brief Updates sensor status to SENSOR_STATUS_STOPPED
     *
     * @details This method cancelles all sennsors that have their status set to SENSOR_STATUS_RUNNING.
     */
    void sensor_stop(void);

    /**
     * @brief Sensors sensor status of sensor on interface
     *
     * @param interface_id Sensor interface ID
     */
    void sensor_send_status(uint8_t interface_id);

    /**
     * @brief Set sensor status
     *
     * @details Sets the sensor status to s and performs necessary actions to
     * act upon this change. For example, if the sensor status is set to
     * SENSOR_STATUS_ACTIVE, the sensor is added to the measurement schedule.
     * It will then be updated automatically to SENSOR_STATUS_RUNNING when the
     * measurement task is started.
     *
     * @param interface_id Sensor interface ID
     * @param s Sensor status
     */
    void sensor_set_status(uint8_t interface_id, sensor_status_t s);

#ifdef __DICIO__
    /**
     * @brief Save sensor data to SD card
     *
     * @param sensor_type Sensor type
     * @param buffer Buffer containing sensor data
     * @param length Length of buffer
     */
    void sdcard_save_sensor_data(sensor_type_t sensor_type, uint8_t *buffer, size_t length);
#endif

#ifdef __cplusplus
}
#endif

#endif
