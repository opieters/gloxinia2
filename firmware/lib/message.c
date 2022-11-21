#include "message.h"
#include <uart.h>
#include <can.h>
#include <address.h>


// internal functions
static void cmd_request_address_available(message_t* m);
static void cmd_address_taken(message_t* m);
static void cmd_update_address(message_t* m);
static void cmd_discovery(message_t* m);

static void send_message_uart(message_t* m);

void init_message(message_t* m, uint16_t identifier,
     bool request_message_bit,
     message_cmd_t command,
     uint16_t sensor_identifier,
        uint8_t* data,
     uint8_t length,
     rx_interface_t interface){
    m->command = command;
    m->identifier = identifier;
    m->sensor_identifier = sensor_identifier;
    m->length = length;
    m->data = data;
    m->interface = interface;
}

inline void process_message(message_t* m) {
    switch (m->command) {
        case M_REQUEST_ADDRESS_AVAILABLE:
            UART_DEBUG_PRINT("CAN_REQUEST_ADDRESS_AVAILABLE");
            cmd_request_address_available(m);
            break;
        case M_ADDRESS_TAKEN:
            UART_DEBUG_PRINT("CAN_ADDRESS_TAKEN");
            cmd_address_taken(m);
            break;
        case M_UPDATE_ADDRESS:
            UART_DEBUG_PRINT("CAN_UPDATE_ADDRESS");
            cmd_update_address(m);
            break;
        case M_DISCOVERY:
            UART_DEBUG_PRINT("CAN_DISCOVERY");
            cmd_discovery(m);
            break;
        case M_MSG_TEXT:
            send_message_uart(m);
            break;
        /*case CAN_INFO_MSG_START_MEAS:
            break;
        case CAN_INFO_MSG_STOP_MEAS:
            // TODO go into sleep mode?
            break;
        case CAN_INFO_MSG_RESET:
        {
            uint8_t reset_device = 0;
            if (m->data_length == 2U) {
                if (m->data[1] == controller_address) {
                    reset_device = 1;
                }
            } else {
                reset_device = 1;
            }
            if (reset_device) {
                delay_us(10);
                asm("RESET");
            }
            break;
        }
        case CAN_MSG_SENSOR_STATUS:

            break;
        case CAN_INFO_MSG_ACTUATOR_STATUS:
            // TODO
            break;
        case CAN_INFO_MSG_INIT_SENSORS:
            break;
        case CAN_INFO_MSG_SAMPLE:
            //sample_sensors = 1;
            break;
        case CAN_INFO_MSG_INT_ADC:
            break;
        case CAN_INFO_MSG_INIT_DAC:
            break;
        case CAN_INFO_MSG_SCHEDULE_I2C:
            break;
        case CAN_INFO_MSG_HELLO:
            UART_DEBUG_PRINT("CAN_INFO_MSG_HELLO");
            break;
        case CAN_INFO_LIA_GAIN_SET:
            if (m->data_length == 4) {
                if (m->data[1] == (controller_address >> 3)) {
                    // TODO
                    //select_gain_pga113(m->data[3]);
                }
            }
            break;*/
        default:
            break;
    }
}

static void cmd_request_address_available(message_t* m) {
    if (m->identifier == controller_address) {
        //can_message_t rm;
        
        UART_DEBUG_PRINT("CAN address collision detected, sending reply");
        
        
    } else {
        UART_DEBUG_PRINT("CAN address not taken by this node.");
    }
}

static void cmd_address_taken(message_t* m){
    if (controller_address == m->identifier) {
        controller_address = 0;

        UART_DEBUG_PRINT("Cleared address.");
    }
}

static void send_message_uart(message_t* m){
    //can_message_t mc;
    
    if(controller_address == 0){
        // message also sent over UART TODO
    } else {
        // cap length
    }       
    //m->length = MAX(8, m->length);
    //can_init_message(&mc, m->identifier, m->request_message_bit, CAN_EXTENDED_FRAME, CAN_HEADER(m->command, m->sensor_identifier), m->data, m->length);
    //can_send_message_any_ch(&mc);
}

void send_message(message_t* m){
    can_message_t mc;
    
    if(controller_address == 0){
        // message also sent over UART TODO
    } else {
        // cap length
    }       
    if(m->interface != UART_INTERFACE){
    m->length = MAX(8, m->length);
    can_init_message(&mc, m->identifier, m->request_message_bit, CAN_EXTENDED_FRAME, CAN_HEADER(m->command, m->sensor_identifier), m->data, m->length);
    can_send_raw_message_any_ch(&mc);
    }
}

static void cmd_update_address(message_t* m){
    if ((m->identifier == controller_address) && (m->length == 2)) {
        set_device_address((m->data[0] << 8) | m->data[1]);
    }
}

static void cmd_discovery(message_t* m){
    // TODO: UART
    if (m->identifier == 0) {
        can_send_message_any_ch(m);
    } else {
        uart_send_message(m);
    }
}