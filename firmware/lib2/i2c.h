#ifndef __I2C_H__
#define __I2C_H__

#include <xc.h>
#include <stdbool.h>
#include <utilities.h>
#include <stdint.h>

/// @brief Maximum number of I2C messages in the buffer
#define I2C_MESSAGE_BUFFER_LENGTH 256

/// @brief I2C/SCL target frequency in Hz
#define FREQUENCY_SCL 100000

/// @brief I2C bus delay correction factor
#define DELAY_I2C 0

/// @brief I2C bus timeout in ms
#define I2C_TIMER_PERIOD 0xffff

/// @brief Get 8-bit write address from 7-bit I2C address
#define I2C_WRITE_ADDRESS(x) (((x) << 1) & 0xfe)

/// @brief Get 8-bit read address from 7-bit I2C address
#define I2C_READ_ADDRESS(x) (((x) << 1) | 0x01)

/// @brief I2C message data pointer for no data
#define I2C_NO_DATA NULL

/// @brief I2C message callback pointer for no callback
#define I2C_NO_CALLBACK NULL

#ifdef __cplusplus
extern "C"
{
#endif

    /// @brief I2C module or message error status
    typedef enum
    {
        I2C_NO_ERROR = 0,            ///< No error
        I2C_NO_ACK,                  ///< No data acknowledge from slave
        I2C_NO_ADDRESS_ACK,          ///< No address acknowledge from slave
        I2C_NO_REPEATED_ADDRESS_ACK, ///< No repeated address acknowledge from slave
        I2C_NO_FINAL_NACK,           ///< No final NACK from slave
        I2C_INCORRECT_DATA,          ///< Incorrect data received from slave
        I2C_QUEUE_FULL,              ///< I2C message queue is full
        I2C_BUFFER_FULL,             ///< I2C message buffer is full
        I2C_MISSED_DATA,             ///< I2C message data was missed
        I2C_ZERO_ATTEMPTS,           ///< I2C message has zero attempts left
        I2C_TOO_MANY_READS,          ///< I2C message has more read operations than expected
        I2C_MSG_NOT_INITIALISED,     ///< I2C message is not initialised correctly
        I2C_MESSAGE_BUS_INACTIVE,    ///< I2C bus is inactive
    } i2c_error_t;

    /// @brief I2C message status
    typedef enum
    {
        I2C_MESSAGE_HANDLED,
        I2C_MESSAGE_QUEUED,
        I2C_MESSAGE_PROCESSING,
        I2C_MESSAGE_TRANSFERRING,
        I2C_MESSAGE_CANCELED,
        I2C_MESSAGE_NO_BUS,
        I2C_MESSAGE_READ_READY,
        n_i2c_status
    } i2c_mstatus_t;

    /// @brief I2C bus
    typedef enum
    {
        I2C1_BUS, ///< I2C1 bus
        I2C2_BUS, ///< I2C2 bus
    } i2c_bus_t;

    /// @brief I2C bus status
    typedef enum
    {
        I2C_BUS_DISABLED, ///< I2C bus is disabled
        I2C_BUS_ENABLED,  ///< I2C bus is enabled
        I2C_BUS_ERROR,    ///< I2C bus error
    } i2c_bus_status_t;

    /*
     * @details Forward declaration if I2C struct and message type without
     * initialisation. This is needed to allow the data_processor attribute to
     * use i2c_message_t as argument.
     */
    typedef struct i2c_message_s i2c_message_t;
    typedef void (*i2c_controller_t)(i2c_message_t *);

    /**
     * @brief Struct to store data fields associated with an I2C message.
     *
     * @param address: I2C data address (read/write)
     * @param i2c_bus: I2C bus to use
     * @param write_data: array of data elements to write to slave
     * @param write_length: number of data elements to write to slave
     * @param read_data: array of data elements to read from slave
     * @param read_length: number of data elements to read from slave
     * @param controller: controller that implements FSM to exchange data
     * @param n_attempts: number of attempts to send message
     * @param status: I2C message status
     * @param error: I2C message error status
     * @param callback: function to call when message is handled
     * @param callback_data: data to pass to callback function
     * @param callback_data_length: length of callback data
     * @param cancelled_callback: function to call when message is cancelled
     */

    struct i2c_message_s
    {
        uint8_t address;
        i2c_bus_t i2c_bus;
        uint8_t *write_data;
        uint8_t write_length;
        uint8_t *read_data;
        uint8_t read_length;
        void (*controller)(i2c_message_t *m);
        int8_t n_attempts;
        volatile i2c_mstatus_t status;
        volatile i2c_error_t error;
        void (*callback)(i2c_message_t *m);
        void *callback_data;
        uint8_t callback_data_length;
        void (*cancelled_callback)(i2c_message_t *m);
    };

    /// @brief I2C module status
    typedef enum {
        I2C_STATUS_SECONDARY_OFF,  ///< I2C module not configured as slave
        I2C_STATUS_SECONDARY_ON,   ///< I2C module configured as slave
        I2C_STATUS_PRIMARY_OFF, ///< I2C module not configured as master
        I2C_STATUS_PRIMARY_ON   ///< I2C module configured as master
    } i2c_status_t;
    
    /**
     * @brief Struct to store data fields associated with an I2C bus.
     *
     * @param i2c_address: I2C address of slave
     * @param status: I2C bus status
     * @param pw_sr_cb: callback function for write slave request
     * @param pr_sw_cb: callback function for read slave request
     * @param scl_pin: SCL pin
     * @param sda_pin: SDA pin
     */
    typedef struct
    {
        uint16_t i2c_address;
        i2c_status_t status;
        void (*pw_sr_cb)(i2c_message_t *);
        void (*pr_sw_cb)(i2c_message_t *);
        pin_t scl_pin;
        pin_t sda_pin;
    } i2c_config_t;

    /// @brief I2C slave status
    typedef enum
    {
        I2C_SLAVE_STATE_IDLE,         ///< I2C slave is idle
        I2C_SLAVE_STATE_NO_MESSAGE,   ///< I2C slave has no message, only address
        I2C_SLAVE_STATE_WRITE,        ///< I2C slave is writing: send data to master
        I2C_SLAVE_STATE_READ,         ///< I2C slave is reading: receive data from master
        I2C_SLAVE_STATE_ADDRESS,      ///< I2C slave is receiving address
        I2C_SLAVE_STATE_REPEAT_START, ///< I2C slave is receiving repeated start condition
        I2C_SLAVE_STATE_STOP,         ///< I2C slave is receiving stop condition
    } i2c_slave_state_t;

    /**
     * @brief Initialises an I2C message.
     *
     * @param m: I2C message to initialise
     * @param address: I2C data address (read/write)
     * @param i2c_bus: I2C bus to use
     * @param write_data: array of data elements to write to slave
     * @param write_length: number of data elements to write to slave
     * @param read_data: array of data elements to read from slave
     * @param read_length: number of data elements to read from slave
     * @param controller: controller that implements FSM to exchange data
     * @param n_attempts: number of attempts to send message
     * @param callback: function to call when message is handled
     * @param callback_data: data to pass to callback function
     * @param callback_data_length: length of callback data
     */
    void i2c_init_message(
        i2c_message_t *m,
        uint8_t address,
        i2c_bus_t i2c_bus,
        uint8_t *write_data,
        size_t write_length,
        uint8_t *read_data,
        uint8_t read_length,
        void (*controller)(i2c_message_t *m),
        int8_t n_attempts,
        void (*callback)(i2c_message_t *m),
        void *callback_data,
        uint8_t callback_data_length);

    /**
     * @brief Handles pending messages in the I2C buffer queue.
     *
     * @details Handles calling the FSM to transfer the data to/from the I2C
     * slave device and executes the `data_processor` function if the
     * data transfer is successful.
     */
    void i2c_process_queue(void);

    /**
     * @brief Re-initialises an I2C message.
     *
     * @param m: I2C message to initialise
     * @param n_attempts: number of attempts to send message
     */
    void i2c_reset_message(i2c_message_t *m, uint8_t n_attempts);

    /**
     * @brief Initialises I2C1 module.
     *
     * @param config: I2C configuration
     */
    void i2c1_init(i2c_config_t *config);

    /**
     * @brief Initialises I2C2 module.
     *
     * @param config: I2C configuration
     */
    void i2c2_init(i2c_config_t *config);

    /// @brief Disables I2C1 module.
    void i2c1_disable(void);

    /// @brief Disables I2C2 module.
    void i2c2_disable(void);

    /**
     * @brief Automatically select the correct write controller for the I2C bus.
     *
     * @details Use this function if the default controller is suitable and data has to be written to the slave device.
     *
     * @param bus: I2C bus to use
     * @return Controller function to use
     */
    i2c_controller_t i2c_get_write_controller(i2c_bus_t bus);

    /**
     * @brief Automatically select the correct read controller for the I2C bus.
     *
     * @details Use this function if the default controller is suitable and data has to read data from the slave device.
     *
     * @param bus: I2C bus to use
     * @return Controller function to use
     */
    i2c_controller_t i2c_get_read_controller(i2c_bus_t bus);

    /**
     * @brief Automatically select the correct write and read controller for the I2C bus.
     *
     * @details Use this function if the default controller is suitable and data has to write and subequently read data from the slave device.
     *
     * @param bus: I2C bus to use
     * @return Controller function to use
     */
    i2c_controller_t i2c_get_write_read_controller(i2c_bus_t bus);

    void i2c_queue_message(i2c_message_t *message);

    /**
     * @brief Checks if the I2C message has been sent.
     * 
     * @param m: I2C message to check
     * @return true if message has been sent, false otherwise
    */
    bool i2c_check_message_sent(i2c_message_t *m);

    /**
     * @brief I2C controllers
     *
     * @details These functions implement the Finite State Machine (FSM) to send
     * or receive I2C messages. They should not be used directly, but in 
     * conjuncton with `i2c_process_queue` to handle I2C messages.
     */
    void i2c1_read_controller(i2c_message_t *m);
    void i2c1_write_controller(i2c_message_t *m);
    void i2c2_read_controller(i2c_message_t *m);
    void i2c1_write_read_controller(i2c_message_t *m);
    void i2c2_write_read_controller(i2c_message_t *m);
    void i2c2_write_controller(i2c_message_t *m);
    void i2c_blocking_read_controller(i2c_message_t *m);
    void i2c_blocking_write_controller(i2c_message_t *m);

#ifdef __cplusplus
}
#endif

#endif
