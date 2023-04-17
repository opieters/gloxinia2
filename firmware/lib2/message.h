#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <xc.h>
#include <stdbool.h>

#define NO_SENSOR_ID (0xff)
#define MESSAGE_REQUEST 1
#define MESSAGE_NO_REQUEST 0

typedef enum
{
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
    M_SENSOR_CONFIG_END = 0x16, // ends burst of configuration data
    M_SENSOR_START = 0x14,
    M_SENSOR_STOP = 0x15,
    M_ACTUATOR_STATUS = 0x20,
    M_TX_ERROR = 0x30,
    M_DATA_CLEAR = 0x40,
    M_DATA_READ = 0x41,
    M_DATA_WRITE = 0x42,
    M_DATA_BURST_START = 0x43,
    M_DATA_BURST = 0x44,
    M_DATA_BURST_STOP = 0x45,
    M_BOOT_READ_VERSION = 0x80,
    M_BOOT_READ_FLASH = 0x81,
    M_BOOT_WRITE_FLASH_INIT = 0x84,
    M_BOOT_WRITE_FLASH = 0x82,
    M_BOOT_WRITE_FLASH_DONE = 0x85,
    M_BOOT_ERASE_FLASH = 0x83,
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
    bool request_message_bit;
    message_cmd_t command;
    uint16_t sensor_identifier;
    uint8_t *data;
    uint8_t length;
    message_status_t status;
} message_t;

#ifdef __cplusplus
extern "C"
{
#endif

    void message_init(message_t *m,
                      uint16_t identifier,
                      bool request_message_bit,
                      message_cmd_t command,
                      uint16_t sensor_identifier,
                      uint8_t *data,
                      uint8_t length);
    void message_reset(message_t *m);

    void message_send(message_t *m);
    void message_process(const message_t *m);

    void send_message_uart(const message_t *m);
    void send_message_can(const message_t *m);

#ifdef __cplusplus
}
#endif

#endif