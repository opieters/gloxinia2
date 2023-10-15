#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <xc.h>
#include <stdbool.h>

#define NO_INTERFACE_ID (0xf)
#define NO_SENSOR_ID (0xf)
#define MESSAGE_REQUEST 1
#define MESSAGE_NO_REQUEST 0
#define MESSAGE_DATA_LGENTH 8

typedef enum
{
    M_NOP = 0x00,
    M_REQUEST_ADDRESS_AVAILABLE = 0x01, // check if CAN identifier is already taken. If not, use this identifier as identifier
    M_ADDRESS_TAKEN = 0x02,             // response message when identifier is already taken
    M_UPDATE_ADDRESS = 0x03,            // change the CAN identifier to a new value. Requester must guarantee that the value is not yet taken
    M_READY = 0x04,
    M_DISCOVERY = 0x05, // discovery message that is used to identify all nodes in the system
    M_HELLO = 0x06,
    M_MSG_TEXT = 0x07,
    M_NODE_INFO = 0x08, // get node info ([node type (1), number of connected sensors (1), software version (2)])
    M_NODE_RESET = 0x09,
    M_SENSOR_STATUS = 0x10,
    M_SENSOR_ERROR = 0x11,
    M_SENSOR_DATA = 0x12,
    M_SENSOR_CONFIG = 0x13, // configure sensors or read configuration
    M_SENSOR_START = 0x17,
    M_CONFIG_SAVED = 0x18,
    M_SENSOR_STOP = 0x15,
    M_SENSOR_CONFIG_END = 0x16, // ends burst of configuration data
    M_ACTUATOR_STATUS = 0x20,
    M_INTERFACE_SUPPLY = 0x21,
    M_TX_ERROR = 0x30,
    M_DATA_CLEAR = 0x40,
    M_DATA_READ = 0x41,
    M_DATA_WRITE = 0x42,    // write data to sd card (first four bytes are address is little endian format, next bytes are data)
    M_DATA_BURST_START = 0x43,
    M_DATA_BURST = 0x44,
    M_DATA_BURST_STOP = 0x45,
    M_DICIO_LOAD_CONFIGURATION_FROM_SDCARD = 0x50,
    M_DICIO_CLEAR_CONFIGURATION_ON_SDCARD = 0x51,        
    M_DICIO_TIME = 0x52,
    M_BOOT_READ_HW_VERSION = 0x80,
    M_BOOT_READ_SW_VERSION = 0x81,
    M_BOOT_READ_BOOT_VERSION = 0x82,
    M_BOOT_READ_FLASH = 0x83,
    M_BOOT_WRITE_FLASH = 0x84,
    M_BOOT_ERASE_FLASH = 0x85,
    M_BOOT_WRITE_FLASH_INIT = 0x86,
    M_BOOT_WRITE_FLASH_DONE = 0x87,
    M_BOOT_SET_ADDRESS_OFFSET = 0x88,
    M_BOOT_RESET_DEVICE = 0x89,
    M_BOOT_SELF_VERIFY = 0x8A,
    M_BOOT_GET_MEMORY_ADDRESS_RANGE_COMMAND = 0x8B,
    M_BOOT_READY = 0x8F,
} message_cmd_t;

/*typedef enum {
    UART_MSG_NONE,
    UART_MSG_TRANSFERRED,
    UART_MSG_QUEUED,
    UART_MSG_SENT,
    UART_MSG_ERROR,
    UART_MSG_INIT_DONE,
    UART_MSG_RECEIVED,
    N_UART_MSG_STATUS_T
} uart_message_status_t;*/

typedef enum
{
    M_EMPTY,
    M_RX_FROM_CAN,
    M_RX_FROM_UART,
    M_TX_INIT_DONE,
    M_TX_QUEUED,
    M_TX_SENT,
    M_ERROR,
    M_ERROR_STOP_DETECT,
    M_ERROR_OVERFLOW,
    M_ERROR_HW_OVERFLOW
} message_status_t;

typedef enum
{
    M_NODE_UNKNOWN = 0x00,
    M_NODE_DICIO = 0x01,
    M_NODE_PLANALTA = 0x02,
    M_NODE_SYLVATICA = 0x03,
} message_node_t;

typedef struct
{
    uint16_t identifier;
    message_cmd_t command;
    uint8_t interface_id;
    uint8_t sensor_id;
    bool request_message_bit;
    uint8_t length;
    uint8_t data[MESSAGE_DATA_LGENTH];
    message_status_t status;
} __attribute__((packed)) message_t;

#ifdef __cplusplus
extern "C"
{
#endif

    void message_init(message_t *m,
                      uint16_t identifier,
                      bool request_message_bit,
                      message_cmd_t command,
                      uint8_t interface_id,
                      uint8_t sensor_id,
                      uint8_t *data,
                      uint8_t length);
    void message_reset(message_t *m);

    void message_send(message_t *m);
    void message_process(const message_t *m);

    void send_message_uart(const message_t *m);
    void send_message_can(const message_t *m);
    
    // internal functions
    void cmd_request_address_available(const message_t *m);
    void cmd_address_taken(const message_t *m);
    void cmd_update_address(const message_t *m);
    void cmd_discovery(const message_t *m);
    void cmd_node_info(const message_t *m);
    void cmd_sensor_config(const message_t *m);
    void cmd_sensor_status(const message_t *m);
    void cmd_sensor_error(const message_t *m);
    void cmd_sensor_data(const message_t *m);
    void cmd_sensor_start(const message_t* m);
    void cmd_sensor_stop(const message_t* m);
    void cmd_sensor_config_end(const message_t *m);
    void cmd_data_clear(const message_t* m);
    void cmd_data_read(const message_t* m);
    void cmd_data_write(const message_t* m);
    void cmd_dicio_load_configuration_from_sdcard(const message_t* m);
    void cmd_dicio_clear_configuration_on_sdcard(const message_t* m);
    void cmd_dicio_time(const message_t* m);
    void cmd_interface_supply(const message_t* m);

#ifdef __cplusplus
}
#endif

#endif
