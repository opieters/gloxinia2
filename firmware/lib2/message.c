#include <message.h>
#include <utilities.h>
#include <can.h>
#include <uart.h>
#include <address.h>
#include <event_controller.h>

// internal functions
static void cmd_request_address_available(message_t* m);
static void cmd_address_taken(message_t* m);
static void cmd_update_address(message_t* m);
static void cmd_discovery(message_t* m);

static void send_message_uart(message_t* m);

extern bool uart_connection_active;

void message_init(message_t* m,
        uint16_t identifier,
        bool request_message_bit,
        message_cmd_t command,
        uint16_t sensor_identifier,
        uint8_t* data,
        uint8_t length) {
    m->command = command;
    m->identifier = identifier;
    m->sensor_identifier = sensor_identifier;
    m->length = length;
    m->data = data;
    m->status = M_TX_INIT_DONE;
    m->request_message_bit = request_message_bit;
}

void message_reset(message_t* m) {
    if ((m->status != M_TX_QUEUED)) {
        m->status = M_TX_INIT_DONE;
    }
}

static void send_message_uart(message_t* m) {
    message_t m_uart;
    
    if(!uart_connection_active){
        return;
    }

    m_uart = *m;
    message_reset(&m_uart);
    uart_queue_message(&m_uart);
}

static void send_message_can(message_t* m) {
    can_message_t mc;

    // cap length
    m->length = MIN(8, m->length);
    can_init_message(&mc, m->identifier, m->request_message_bit, CAN_EXTENDED_FRAME, CAN_HEADER(m->command, m->sensor_identifier), m->data, m->length);
    can_send_message_any_ch(&mc);
}

void message_send(message_t* m) {
    switch (m->status) {
        case M_RX_FROM_CAN:
        case M_RX_FROM_UART:
        case M_TX_QUEUED:
        case M_TX_SENT:
        case M_ERROR:
            break;
        case M_TX_INIT_DONE:
            if (uart_connection_active) {
                send_message_can(m);
                send_message_uart(m);
            } else {
                send_message_can(m);
            }
            break;
    }
}

inline void message_process(message_t* m) {
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
        message_t rm;
        message_init(&rm,
                m->identifier,
                0,
                M_ADDRESS_TAKEN,
                NO_SENSOR_ID,
                NULL,
                0);
        message_send(&rm);
        UART_DEBUG_PRINT("CAN address collision detected, sending reply");
    } else {
        UART_DEBUG_PRINT("CAN address not taken by this node.");
    }
}

static void cmd_address_taken(message_t* m) {
    if (controller_address == m->identifier) {
        controller_address = ADDRESS_UNSET;

        UART_DEBUG_PRINT("Cleared address.");
        
        // if an unsuccessful attempt was made to set the gateway node address,
        // we must report this such that an error can be displayed by the GUI
        // to inform the user that there is a connectivity issue
        if(uart_connection_active){
            message_init(m,
                    m->identifier,
                    0,
                    M_ADDRESS_TAKEN,
                    NO_SENSOR_ID,
                    NULL,
                    0);
            message_reset(m);
            uart_queue_message(m);
        }
        
        // new attempt to get address
        task_schedule_t task_get_address = {address_get, 0, 0, 0};
        
        schedule_specific_event(task_get_address, ID_GET_ADDRESS);
    }
}

static void cmd_update_address(message_t* m) {
    if ((m->identifier == controller_address) && (m->length == 2)) {
        address_set((m->data[0] << 8) | m->data[1]);
    }
    
    // report back
    message_init(m, controller_address,
        MESSAGE_NO_REQUEST,
        M_DISCOVERY,
        NO_SENSOR_ID,
        NULL,
        0);
    message_send(m);
}

static void cmd_discovery(message_t* m) {
    can_reset();
    
    if(m->request_message_bit && (m->identifier == ADDRESS_GATEWAY)){
        if(controller_address == ADDRESS_UNSET){
            address_get();
            return;
        } else {
            // forward the message to the other nodes
            if(m->status == M_RX_FROM_UART){
                can_send_fmessage_any_ch(m);
            }
        }
    } 
    if(m->request_message_bit && (m->identifier == ADDRESS_GATEWAY) && (controller_address != ADDRESS_UNSET)){
        // send message back to gateway
        message_init(m,
            controller_address,
            MESSAGE_NO_REQUEST,
            M_DISCOVERY,
            NO_SENSOR_ID,
            NULL,
            0);
        
        message_send(m);
    }
    
    if(!m->request_message_bit){
        send_message_uart(m);
    }
}