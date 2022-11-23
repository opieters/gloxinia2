#ifndef __I2C_H__
#define	__I2C_H__

#include <xc.h>
#include <stdbool.h>
#include <utilities.h>
#include <stdint.h>

#define I2C_MESSAGE_BUFFER_LENGTH 256
#define I2C_W_MESSAGE_BUFFER_LENGTH 32
#define FREQUENCY_SCL 100000
#define DELAY_I2C     0
#define MAX_N_I2C_RST_READS 100
#define I2C_TIMER_PERIOD 0xffff
#define N_I2C_DEVICE_RESET_CALLBACKS 5
#define I2C_WRITE_ADDRESS(x) ( ((x) << 1) & 0xfe )
#define I2C_READ_ADDRESS(x) ( ((x) << 1) | 0x01 )
#define I2C_ERROR_TH 50

#define I2C_NO_DATA NULL
#define I2C_NO_CALLBACK NULL


#ifdef	__cplusplus
extern "C" {
#endif

    /**
     * @brief State of I2C module interaction
     * 
     * @details TODO
     */
    typedef enum {
        I2C_NO_ERROR = 0,
        I2C_NO_ACK,
        I2C_NO_ADDRESS_ACK,
        I2C_NO_REPEATED_ADDRESS_ACK,
        I2C_NO_FINAL_NACK,
        I2C_INCORRECT_DATA,
        I2C_QUEUE_FULL,
        I2C_BUFFER_FULL,
        I2C_MISSED_DATA,
        I2C_ZERO_ATTEMPTS,
        I2C_TOO_MANY_READS,
        I2C_MSG_NOT_INITIALISED,
        I2C_MESSAGE_BUS_INACTIVE,
    } i2c_error_t;

    typedef enum {
        I2C_MESSAGE_HANDLED,
        I2C_MESSAGE_QUEUED,
        I2C_MESSAGE_PROCESSING,
        I2C_MESSAGE_TRANSFERRING,
        I2C_MESSAGE_CANCELED,
        I2C_MESSAGE_NO_BUS,
        I2C_MESSAGE_READ_READY,
        n_i2c_status
    } i2c_mstatus_t;

    typedef enum {
        I2C1_BUS,
        I2C2_BUS,
        N_I2C_BUS,
    } i2c_bus_t;

    typedef enum {
        I2C_BUS_DISABLED,
        I2C_BUS_ENABLED,
        I2C_BUS_ERROR,
    } i2c_bus_status_t;

    /*
     * Forward declaration if I2C struct and message type without 
     * initialisation. This is needed to allow the data_processor attribute to 
     * use i2c_message_t as argument. 
     */
    typedef struct i2c_message_s i2c_message_t;
    typedef void (*i2c_controller_t)(i2c_message_t*);

    /**
     * Struct to store data fields associated with an I2C message.
     * 
     * @attribute address: I2C data address (read/write)
     * @attribute data: array of data elements to send/receive
     * @attribute data_length: length of data exchange in bytes
     * @attribute controller: controller that implements FSM to exchange data
     * @attribute data_processor: function that is called after data exchange
     */

    struct i2c_message_s {
        uint8_t address;
        i2c_bus_t i2c_bus;
        uint8_t* write_data;
        uint8_t write_length;
        uint8_t* read_data;
        uint8_t read_length;
        void (*controller)(i2c_message_t* m);
        int8_t n_attempts;
        volatile i2c_mstatus_t status;
        volatile i2c_error_t error;
        void (*callback)(i2c_message_t* m);
        void* callback_data;
        uint8_t callback_data_length;
        void (*cancelled_callback)(i2c_message_t* m);
    };

    typedef struct i2c_reset_sensor_s {
        i2c_bus_t i2c_bus;
        void (*reset)(void);
        bool (*init)(void);
    } i2c_reset_sensor_t;

    typedef enum {
        I2C_STATUS_OFF,
        I2C_STATUS_SECONDARY_ON,
        I2C_STATUS_PRIMARY_ON
    } i2c_status_t;

    typedef struct {
        uint16_t i2c_address;
        i2c_status_t status;
        void (*pw_sr_cb) (i2c_message_t*);
        void (*pr_sw_cb) (i2c_message_t*);
        pin_t scl_pin;
        pin_t sda_pin;
    } i2c_config_t;

    typedef enum {
        I2C_SLAVE_STATE_IDLE,
        I2C_SLAVE_STATE_NO_MESSAGE,
        I2C_SLAVE_STATE_WRITE,
        I2C_SLAVE_STATE_READ,
        I2C_SLAVE_STATE_ADDRESS,
        I2C_SLAVE_STATE_REPEAT_START,
        I2C_SLAVE_STATE_STOP,
    } i2c_slave_state_t;

    typedef enum {
        I2C_CALLBACK_NONE,
        I2C_CALLBACK_M_READ_S_WRITE,
        I2C_CALLBACK_M_WRITE_S_READ,
    } i2c_callback_status_t;

    void i2c_init_message(
            i2c_message_t* m,
            uint8_t address,
            i2c_bus_t i2c_bus,
            uint8_t* write_data,
            size_t write_length,
            uint8_t* read_data,
            uint8_t read_length,
            void (*controller)(i2c_message_t* m),
            int8_t n_attempts,
            void (*callback)(i2c_message_t* m),
            void* callback_data,
            uint8_t callback_data_length);

    /**
     * @brief Handles pending messages in the I2C buffer queue. 
     * 
     * @details Handles calling the FSM to transfer the data to/from the I2C
     * slave device and executes the `data_processor` function if the
     * data transfer is successful. 
     */
    void i2c_process_queue(void);

    void i2c_reset_message(i2c_message_t* m, uint8_t n_attempts);

    /**
     * @brief Initialises the I2C1 module.
     */
    void i2c1_init(i2c_config_t* config);

    /**
     * @brief Initialises the I2C2 module.
     */
    void i2c2_init(i2c_config_t* config);

    void i2c1_disable(void);
    void i2c2_disable(void);

    i2c_controller_t i2c_get_write_controller(i2c_bus_t bus);
    i2c_controller_t i2c_get_read_controller(i2c_bus_t bus);
    i2c_controller_t i2c_get_write_read_controller(i2c_bus_t bus);


    /**
     * 
     * @brief FSM implementing an I2C read data transfer. 
     * 
     * @details This function returns after every action and needs to be called 
     * until I2C_TRANSFER_COMPLETE or an error is returned.
     * @return Status of the I2C transfer I2C_NO_ERROR if no error and operation
     * is ongoing, I2C_TRANSFER_COMPLETE at transfer completion or error.
     */
    void i2c1_read_controller(i2c_message_t* m);

    /**
     * @brief Same as default_i2c_read_controller, but for writing a message to 
     * an I2C slave.
     * @return Status of the I2C transfer I2C_NO_ERROR if no error and operation
     * is ongoing, I2C_TRANSFER_COMPLETE at transfer completion or error.
     */
    void i2c1_write_controller(i2c_message_t* m);

    /**
     * @brief Same as `default_i2c_read_controller` but with second I2C module.
     */
    void i2c2_read_controller(i2c_message_t* m);

    void i2c1_write_read_controller(i2c_message_t* m);
    void i2c2_write_read_controller(i2c_message_t* m);

    /**
     * @brief Same as `default_i2c_write_controller` but with second I2C module.
     */
    void i2c2_write_controller(i2c_message_t* m);

    /**
     * @brief Same as `default_i2c_read_controller` but blocking. 
     * 
     * @details Functions returns only if the transfer is complete or an error 
     * occurred.
     * @return see `default_i2c_read_controller`.
     */
    void i2c_blocking_read_controller(i2c_message_t* m);

    /**
     * @brief Same as `default_i2c_write_controller` but blocking. 
     * 
     * @details Functions returns only if the transfer is complete or an error 
     * occurred.
     * @return See `default_i2c_write_controller`.
     */
    void i2c_blocking_write_controller(i2c_message_t* m);


    /**
     * @brief Same as `queue_simple_i2c_message` but the controller must be 
     * set correctly.
     * 
     * @param message: I2C message to transmit/receive
     * @return I2C_NO_ERROR
     */
    void i2c_queue_message(i2c_message_t* message);
    
    bool i2c_check_message_sent(i2c_message_t* m);

#ifdef	__cplusplus
}
#endif

#endif
