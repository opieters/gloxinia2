#ifndef __UART_COMMON_H__
#define	__UART_COMMON_H__

#include <xc.h> // include processor files - each processor file is guarded.  
#include <utilities.h>
#include <stddef.h>

#define UART_CMD_START ('\x01')
#define UART_CMD_STOP  ('\x04')

#define UART_MESSAGE_BUFFER_LENGTH 64U
#define UART_HEADER_SIZE 7U

#define UART_RX_BUFFER_SIZE 4U

typedef enum {
    UART_MSG_NONE,
    UART_MSG_TRANSFERRED,
    UART_MSG_QUEUED,
    UART_MSG_SENT,
    UART_MSG_ERROR,
    UART_MSG_INIT_DONE,
    UART_MSG_RECEIVED,
    N_UART_MSG_STATUS_T
} uart_message_status_t;

typedef enum {
    // CAN COMMANDS, see CAN documentation
    
    SERIAL_CAN_REQUEST_ADDRESS_AVAILABLE   = 0x0001,
    SERIAL_CAN_ADDRESS_TAKEN               = 0x0002,
    SERIAL_CAN_UPDATE_ADDRESS              = 0x0003,
    SERIAL_CAN_DISCOVERY                   = 0x0004,
    
    // SERIAL COMMANDS
    SERIAL_START_MEAS_CMD         = 0x8000,
    SERIAL_STOP_MEAS_CMD          = 0x8001,
    SERIAL_SENSOR_ACTIVATE_CMD    = 0x8002,
    SERIAL_SENSOR_DEACTIVATE_CMD  = 0x8003,
    SERIAL_RESET_NODE_CMD         = 0x8004,
    SERIAL_RESET_SYSTEM_CMD       = 0x8005,
    SERIAL_TEXT_MESSAGE_CMD       = 0x8006,
    SERIAL_SENSOR_DATA_CMD        = 0x8007,
    SERIAL_SENSOR_STATUS_CMD      = 0x8008,
    SERIAL_MEAS_PERIOD_CMD        = 0x8009,
    SERIAL_ERR_MESSAGE_CMD        = 0x800A,
    SERIAL_LOOP_MESSAGE_CMD       = 0x800B,
    SERIAL_ACTUATOR_STATUS        = 0x800C,
    SERIAL_HELLO_CMD              = 0x800D,
    SERIAL_INIT_SAMPLING_CMD      = 0x800E,
    SERIAL_INIT_SENSORS_CMD       = 0x800F,
    SERIAL_SENSOR_ERROR_CMD       = 0x8010,
    SERIAL_LIA_GAIN_SET_CMD       = 0x8011,
    SERIAL_UNKNOWN_CMD            = 0x8012,
    SERIAL_MEAS_TRIGGER_CMD       = 0x8013,
    SERIAL_SENSOR_CONFIG_CMD      = 0x8014,
    SERIAL_ACTUATOR_DATA_CMD      = 0x8015,
    SERIAL_ACTUATOR_ERROR_CMD     = 0x8016,
    SERIAL_ACTUATOR_TRIGGER_CMD   = 0x8017,
    SERIAL_ACTUATOR_GC_TEMP       = 0x8018,
    SERIAL_ACTUATOR_GC_RH         = 0x8019,
    SERIAL_START_INIT             = 0x801A,
    SERIAL_ACTUATOR_RELAY         = 0x801B,
    SERIAL_SENSOR_ACTUATOR_ENABLE = 0x801C,
    SERIAL_ACTUATOR_RELAY_NOW     = 0x801D,
    SERIAL_ACTUATOR_STATUS_CMD    = 0x801E,
    N_SERIAL_CMD                  = 0x801F,
} serial_cmd_t;

typedef struct {
    serial_cmd_t command;
    uint8_t id;
    uint16_t extended_id;            
    uint16_t length;
    uint8_t* data;
    volatile uart_message_status_t status;
} uart_message_t;

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    void uart_init_message(uart_message_t* m, 
            serial_cmd_t command,
            uint8_t id,
            uint16_t extended_id,            
            uint8_t* data,
            size_t length);
    
    void uart_reset_message(uart_message_t* m);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __UART_H__ */

