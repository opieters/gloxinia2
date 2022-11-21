#ifndef __MESSAGE_H__
#define	__MESSAGE_H__

#include <xc.h>
#include <stdbool.h>

#define NO_SENSOR_ID 0
#define MESSAGE_REQUEST 1
#define MESSAGE_NO_REQUEST 0

typedef enum {
    M_REQUEST_ADDRESS_AVAILABLE  = 0x01, // check if CAN identifier is already taken. If not, use this identifier as identifier
    M_ADDRESS_TAKEN              = 0x02, // response message when identifier is already taken
    M_UPDATE_ADDRESS             = 0x03, // change the CAN identifier to a new value. Requester must guarantee that the value is not yet taken
    M_READY                      = 0x04,
    M_DISCOVERY                  = 0x05, // discovery message that is used to identify all nodes in the system
    M_HELLO                      = 0x06,
    M_MSG_TEXT                   = 0x07,
    M_SENSOR_STATUS,
    M_SENSOR_ERROR,
    M_SENSOR_DATA,
    M_ACTUATOR_STATUS,
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

typedef enum {
    M_RX_FROM_CAN,
    M_RX_FROM_UART,
    M_TX_INIT_DONE,
    M_TX_QUEUED,
    M_TX_SENT,
    M_ERROR,
} message_status_t;

typedef struct {
    uint16_t identifier;
    bool request_message_bit;
    message_cmd_t command;
    uint16_t sensor_identifier;
    uint8_t* data;
    uint8_t length;
    message_status_t status;
} message_t;

#ifdef	__cplusplus
extern "C" {
#endif
    
    void message_init(message_t* m, 
            uint16_t identifier,
            bool request_message_bit,
            message_cmd_t command,
            uint16_t sensor_identifier,
            uint8_t* data,
            uint8_t length);
    void message_reset(message_t* m);
    
    void message_send(message_t* m);
    void message_process(message_t* m);

#ifdef	__cplusplus
}
#endif

#endif

