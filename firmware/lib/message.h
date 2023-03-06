#ifndef __MESSAGE_H__
#define	__MESSAGE_H__

#include <xc.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        M_REQUEST_ADDRESS_AVAILABLE = 0x01, // check if CAN identifier is already taken. If not, use this identifier as identifier
        M_ADDRESS_TAKEN = 0x02, // response message when identifier is already taken
        M_UPDATE_ADDRESS = 0x03, // change the CAN identifier to a new value. Requester must guarantee that the value is not yet taken
        M_DISCOVERY = 0x05, // discovery message that is used to identify all nodes in the system
        M_HELL0 = 0x06,
        M_SENSOR_STATUS,
        M_SENSOR_ERROR,
        M_MSG_TEXT = 0x07,
        M_SENSOR_DATA,
        M_ACTUATOR_STATUS,
    } message_cmd_t;
    
    /*
         typedef enum {
        NO_CAN_CMD = 0x00, // dummy code, not used for now
        CAN_REQUEST_ADDRESS_AVAILABLE = 0x01, // check if CAN identifier is already taken. If not, use this identifier as identifier
        CAN_ADDRESS_TAKEN = 0x02, // response message when identifier is already taken
        CAN_UPDATE_ADDRESS = 0x03, // change the CAN identifier to a new value. Requester must guarantee that the value is not yet taken
        CAN_DISCOVERY = 0x05, // discovery message that is used to identify all nodes in the system
        CAN_DATA_CMD_APDS9301 = 0x41,
        CAN_DATA_CMD_OPT3001Q1 = 0x42,
        CAN_DATA_CMD_BH1721FVC = 0x43,
        CAN_DATA_CMD_APDS9306 = 0x44,
        CAN_DATA_CMD_SHT35 = 0x45,
        CAN_DATA_CMD_SYLVATICA = 0x06,
        CAN_DATA_CMD_PLANALTA = 0x07,
        CAN_DATA_CMD_LICOR = 0x08,
        CAN_DATA_CMD_GROWTH_CHAMBER = 0x09,
        CAN_DATA_CMD_RELAY_BOARD = 0x0A,
        CAN_DATA_CMD_PLANALTA_FS = 0x0B,
        CAN_DATA_CMD_NAU7802 = 0x0C,
        CAN_DATA_CMD_DS18B20 = 0x0D,
        CAN_DATA_CMD_SYLVATICA2 = 0x0E,
        CAN_CONFIG_CMD_APDS9301 = 0x31,
        CAN_CONFIG_CMD_OPT3001Q1 = 0x32,
        CAN_CONFIG_CMD_BH1721FVC = 0x33,
        CAN_CONFIG_CMD_APDS9306 = 0x34,
        CAN_CONFIG_CMD_SHT35 = 0x35,
        CAN_CONFIG_CMD_SYLVATICA = 0x36,
        CAN_CONFIG_CMD_PLANALTA = 0x37,
        CAN_CONFIG_CMD_LICOR = 0x38,
        CAN_CONFIG_CMD_GROWTH_CHAMBER = 0x39,
        CAN_CONFIG_CMD_RELAY_BOARD = 0x3A,
        CAN_CONFIG_CMD_PLANALTA_FS = 0x3B,
        CAN_CONFIG_CMD_NAU7802 = 0x3C,
        CAN_CONFIG_CMD_DS18B20 = 0x3D,
        CAN_CONFIG_CMD_SYLVATICA2 = 0x3E,
        CAN_MSG_SENSOR_STATUS = 0x50,
        CAN_MSG_SENSOR_ERROR = 0x51,
        CAN_MSG_ACTUATOR_ERROR = 0x52,
        CAN_MSG_ACTUATOR_STATUS = 0x53,
        CAN_INFO_MSG_TEXT,
        CAN_INFO_MSG_START_MEAS,
        CAN_INFO_MSG_STOP_MEAS,
        CAN_INFO_MSG_RESET,
        CAN_INFO_MSG_ACTUATOR_STATUS,
        CAN_INFO_MSG_INIT_SENSORS,
        CAN_INFO_MSG_SAMPLE,
        CAN_INFO_MSG_HELLO,
        CAN_INFO_MSG_INT_ADC,
        CAN_INFO_MSG_INIT_DAC,
        CAN_INFO_MSG_SCHEDULE_I2C,
        CAN_INFO_LIA_GAIN_SET,
        CAN_INFO_CMD_MEASUREMENT_START,
        CAN_INFO_CMD_ACTUATOR_START,
        CAN_CONFIG_CMD,
        N_CAN_MSG
    } can_cmd_t;
     */
    
    typedef enum {
        NO_INTERFACE,
        CAN_INTERFACE,
        UART_INTERFACE,
    } rx_interface_t;
    
    typedef struct {
        uint16_t identifier;
        bool request_message_bit;
        message_cmd_t command;
        uint16_t sensor_identifier;
        uint8_t* data;
        uint8_t length;
        rx_interface_t interface;
    } message_t;
    
    void init_message(message_t* m, uint16_t identifier,
        bool request_message_bit,
        message_cmd_t command,
        uint16_t sensor_identifier,
        
        uint8_t* data,
            uint8_t length,
        rx_interface_t interface);
    
    
    void process_message(message_t* m);
    void send_message(message_t* m);

#ifdef	__cplusplus
}
#endif

#endif

