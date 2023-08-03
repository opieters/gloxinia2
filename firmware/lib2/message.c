#include <message.h>
#include <utilities.h>
#include <can.h>
#include <uart.h>
#include <address.h>
#include <event_controller.h>
#include <i2c.h>
#include <sensor.h>
#include <libpic30.h>

void address_get_task(void *data);

static message_t m_error;
static uint8_t m_error_data[2];

extern bool uart_connection_active;

void message_init(message_t *m,
        uint16_t identifier,
        bool request_message_bit,
        message_cmd_t command,
        uint8_t interface_id,
        uint8_t sensor_id,
        uint8_t *data,
        uint8_t length) {
    m->command = command;
    m->identifier = identifier;
    m->interface_id = interface_id;
    m->sensor_id = sensor_id;
    length = MIN(MESSAGE_DATA_LGENTH, length);
    m->length = length;
    for(int i = 0; i < length; i++){
        m->data[i] = data[i];
    }
    m->status = M_TX_INIT_DONE;
    m->request_message_bit = request_message_bit;
}

void message_reset(message_t *m) {
    if ((m->status != M_TX_QUEUED)) {
        m->status = M_TX_INIT_DONE;
    }
}

void send_message_uart(const message_t *m) {
    message_t m_uart;

    if (!uart_connection_active) {
        return;
    }

    m_uart = *m;
    message_reset(&m_uart);
    uart_queue_message(&m_uart);
}

void send_message_can(const message_t *m) {
    can_message_t mc;

    // cap length
    mc.data_length = MIN(8, m->length);
    can_init_message(&mc, m->identifier, m->request_message_bit, CAN_EXTENDED_FRAME, CAN_HEADER(m->command, m->interface_id, m->sensor_id), m->data, mc.data_length);
    
    uint16_t n_attempts = 0;
    can_status_t sent_status = CAN_NO_ERROR;
    do {
        sent_status = can_send_message(&mc, m->identifier % 8);
        n_attempts++;
    } while ((sent_status != CAN_NO_ERROR) && (n_attempts < 8U));
    
    __delay_ms(100);
    
    if(sent_status == CAN_NO_ERROR)
    {
        // log message when sent
        send_message_uart(m);
    }
    
    //can_send_message_any_ch(&mc);
}

void message_send(message_t *m) {
    switch (m->status) {
        case M_TX_INIT_DONE:
            if (uart_connection_active) {
                send_message_can(m);
                send_message_uart(m);
            } else {
                send_message_can(m);
            }
            break;
        case M_RX_FROM_CAN:
        case M_RX_FROM_UART:
        case M_TX_QUEUED:
        case M_TX_SENT:
        case M_ERROR:
        case M_EMPTY:
        default:
            break;
    }
}

void message_process(const message_t *m) {
    // do not process message if there was an error
    if (m->status >= M_ERROR) {
        m_error_data[0] = m->command;
        m_error_data[1] = m->status;
        message_init(&m_error, controller_address,
                MESSAGE_NO_REQUEST,
                M_TX_ERROR,
                NO_INTERFACE_ID,
                NO_SENSOR_ID,
                m_error_data,
                ARRAY_LENGTH(m_error_data));

        send_message_uart(&m_error);
        return;
    }

    switch (m->command) {
        case M_REQUEST_ADDRESS_AVAILABLE:
            UART_DEBUG_PRINT("CAN_REQUEST_ADDRESS_AVAILABLE");
            if(m->identifier == controller_address)
                cmd_request_address_available(m);
            break;
        case M_ADDRESS_TAKEN:
            UART_DEBUG_PRINT("CAN_ADDRESS_TAKEN");
            if(m->identifier == controller_address)
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
            UART_DEBUG_PRINT("MSG_TEXT");
            send_message_uart(m);
            break;
        case M_NODE_INFO:
            UART_DEBUG_PRINT("NODE_INFO");
            cmd_node_info(m);
            break;
        case M_SENSOR_STATUS:
            UART_DEBUG_PRINT("SENSOR_STATUS");
            cmd_sensor_status(m);
            break;
        case M_SENSOR_ERROR:
            UART_DEBUG_PRINT("SENSOR_ERROR");
            cmd_sensor_error(m);
            break;
        case M_SENSOR_START:
            UART_DEBUG_PRINT("SENSOR_START");
            cmd_sensor_start(m);
            break;
        case M_SENSOR_STOP:
            UART_DEBUG_PRINT("SENSOR_STOP");
            cmd_sensor_stop(m);
            break;
        case M_SENSOR_CONFIG:
            UART_DEBUG_PRINT("SENSOR_CONFIG");
            cmd_sensor_config(m);
            break;
        case M_NODE_RESET:
        {
            UART_DEBUG_PRINT("NODE_RESET");
            can_disable();
            i2c1_disable();
            i2c2_disable();
            asm("RESET");

            break;
        }
        // these messages do not trigger an action
        case M_DATA_BURST_START:
            UART_DEBUG_PRINT("DATA_BURST_STOP");
            break;
        case M_DATA_BURST:
            UART_DEBUG_PRINT("DATA_BURST");
            break;
        case M_DATA_BURST_STOP:
            UART_DEBUG_PRINT("DATA_BURST_STOP");
            break;
        case M_HELLO:
            UART_DEBUG_PRINT("CAN_HELLO");
            break;
        case M_CONFIG_SAVED:
            UART_DEBUG_PRINT("M_CONFIG_SAVED");
            break;
        default:
            break;
    }
}

void cmd_request_address_available(const message_t *m) {
    message_t rm;
    message_init(&rm,
            m->identifier,
            0,
            M_ADDRESS_TAKEN,
            NO_INTERFACE_ID,
            NO_SENSOR_ID,
            NULL,
            0);
    message_send(&rm);
    UART_DEBUG_PRINT("CAN address collision detected, sending reply");
}

void cmd_address_taken(const message_t *m) {
    if (!m->request_message_bit) {
        controller_address = ADDRESS_UNSET;

        UART_DEBUG_PRINT("Cleared address.");

        // if an unsuccessful attempt was made to set the gateway node address,
        // we must report this such that an error can be displayed by the GUI
        // to inform the user that there is a connectivity issue
        if (uart_connection_active) {
            message_t m_address;
            message_init(&m_address,
                    m->identifier,
                    0,
                    M_ADDRESS_TAKEN,
                    NO_INTERFACE_ID,
                    NO_SENSOR_ID,
                    NULL,
                    0);
            message_reset(&m_address);
            uart_queue_message(&m_address);
        }

        // new attempt to get address
        task_t task_get_address = {address_get_task, NULL};

        push_queued_task(task_get_address);
    }
}

void address_get_task(void *data) {
    address_find_non_reserved();
}

void cmd_update_address(const message_t *m) {
    if (m->length == 2) {
        address_set_and_check_available((m->data[0] << 8) | m->data[1]);
    }

    // report back
    message_t m_address;
    message_init(&m_address, controller_address,
            MESSAGE_NO_REQUEST,
            M_DISCOVERY,
            NO_INTERFACE_ID,
            NO_SENSOR_ID,
            NULL,
            0);
    message_send(&m_address);
}

void cmd_discovery(const message_t *m) {
    if (m->request_message_bit) {
        if (controller_address == ADDRESS_UNSET) {
            address_find_non_reserved();
            return;
        } else {
            // send message back to gateway
            message_t m_discovery;
            message_init(&m_discovery,
                    controller_address,
                    MESSAGE_NO_REQUEST,
                    M_DISCOVERY,
                    NO_INTERFACE_ID,
                    NO_SENSOR_ID,
                    NULL,
                    0);

            message_send(&m_discovery);
        }
    }
}

void cmd_node_info(const message_t *m) {
    if (!m->request_message_bit) {
        return;
    }
    uint8_t data[4];
    message_t i;

    // node type
#ifdef __DICIO__
    data[0] = M_NODE_DICIO;
#elif __SYLVATICA__
    data[0] = M_NODE_SYLVATICA;
#elif __PLANALTA__
    data[0] = M_NODE_PLANALTA;
#else
    data[0] = M_NODE_UNKNOWN;
#endif

    data[1] = __HARDWARE_VERSION__;
    data[2] = __SOFWARE_MAJOR_VERSION__;
    data[3] = __SOFWARE_MINOR_VERSION__;

    message_init(&i, controller_address,
            MESSAGE_NO_REQUEST,
            m->command,
            NO_INTERFACE_ID,
            NO_SENSOR_ID,
            data,
            ARRAY_LENGTH(data));
    message_send(&i);
}

void cmd_sensor_config(const message_t *m) {
    if (m->request_message_bit) {
        // send config
        uint8_t reg = 0, length = 0;
        uint8_t buffer[8];
        message_t m_sensor;
        
        do {
            sensor_get_config(m->interface_id, m->sensor_id, reg, buffer, &length);
            reg++;
            if (length > 0) {
                message_init(&m_sensor, controller_address,
                        MESSAGE_NO_REQUEST,
                        m->command,
                        m->interface_id,
                        m->sensor_id,
                        buffer,
                        length);
                message_send(&m_sensor);
            }
        } while (length > 0);
        
        message_init(&m_sensor, controller_address,
                MESSAGE_NO_REQUEST,
                M_SENSOR_CONFIG_END,
                m->interface_id,
                m->sensor_id,
                buffer,
                0);
        message_send(&m_sensor);
        
    } else {
        sensor_set_config_from_buffer(m->interface_id, m->sensor_id, &m->data[0], m->length);
    }
}



void cmd_sensor_status(const message_t *m) {
    if (m->request_message_bit) {
        sensor_send_status(m->interface_id, m->sensor_id);
    } else {
        sensor_set_status(m->interface_id, m->sensor_id, m->data[0]);
    }
}

void cmd_sensor_error(const message_t *m) {
}



void cmd_sensor_start(const message_t* m) {
    sensor_start();
}

void cmd_sensor_stop(const message_t* m) {
    sensor_stop();
}


