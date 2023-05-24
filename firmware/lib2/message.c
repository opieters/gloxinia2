#include <message.h>
#include <utilities.h>
#include <can.h>
#include <uart.h>
#include <address.h>
#include <event_controller.h>
#include <i2c.h>
#include <sensor.h>
#ifdef __DICIO__
#include "../dicio.X/dicio.h"
#include "../dicio.X/sdcard.h"
#endif

// internal functions
static void cmd_request_address_available(const message_t *m);
static void cmd_address_taken(const message_t *m);
static void cmd_update_address(const message_t *m);
static void cmd_discovery(const message_t *m);
static void cmd_node_info(const message_t *m);
static void cmd_sensor_config(const message_t *m);
static void cmd_sensor_status(const message_t *m);
static void cmd_sensor_error(const message_t *m);
static void cmd_sensor_data(const message_t *m);
static void cmd_sensor_start(const message_t* m);
static void cmd_sensor_stop(const message_t* m);
static void cmd_sensor_config_end(const message_t *m);
static void cmd_data_clear(const message_t* m);
static void cmd_data_read(const message_t* m);
static void cmd_data_write(const message_t* m);

static void address_get_task(void *data);

static message_t m_error;
static uint8_t m_error_data[2];

extern bool uart_connection_active;

void message_init(message_t *m,
        uint16_t identifier,
        bool request_message_bit,
        message_cmd_t command,
        uint16_t sensor_identifier,
        uint8_t *data,
        uint8_t length) {
    m->command = command;
    m->identifier = identifier;
    m->sensor_identifier = sensor_identifier;
    m->length = length;
    m->data = data;
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
    can_init_message(&mc, m->identifier, m->request_message_bit, CAN_EXTENDED_FRAME, CAN_HEADER(m->command, m->sensor_identifier), m->data, mc.data_length);
    can_send_message_any_ch(&mc);
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
                NO_SENSOR_ID,
                m_error_data,
                ARRAY_LENGTH(m_error_data));

        send_message_uart(&m_error);
        return;
    }

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
        case M_SENSOR_DATA:
            UART_DEBUG_PRINT("SENSOR_DATA");
            cmd_sensor_data(m);
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
        case M_SENSOR_CONFIG_END:
            UART_DEBUG_PRINT("SENSOR_CONFIG_END");
            cmd_sensor_config_end(m);
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
        case M_DATA_CLEAR:
            UART_DEBUG_PRINT("DATA_CLEAR");
            cmd_data_clear(m);
            break;
        case M_DATA_READ:
            UART_DEBUG_PRINT("DATA_READ");
            cmd_data_read(m);
            break;
        case M_DATA_WRITE:
            UART_DEBUG_PRINT("DATA_WRITE");
            cmd_data_write(m);
            break;
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
        default:
            break;
    }
}

static void cmd_request_address_available(const message_t *m) {
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

static void cmd_address_taken(const message_t *m) {
    if (controller_address == m->identifier) {
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

static void address_get_task(void *data) {
    address_find_non_reserved();
}

static void cmd_update_address(const message_t *m) {
    if ((m->identifier == controller_address) && (m->length == 2)) {
        address_set_and_check_available((m->data[0] << 8) | m->data[1]);
    }

    // report back
    message_t m_address;
    message_init(&m_address, controller_address,
            MESSAGE_NO_REQUEST,
            M_DISCOVERY,
            NO_SENSOR_ID,
            NULL,
            0);
    message_send(&m_address);
}

static void cmd_discovery(const message_t *m) {
    if (m->request_message_bit && (m->identifier == ADDRESS_GATEWAY)) {
        if (controller_address == ADDRESS_UNSET) {
            address_find_non_reserved();
            return;
        } else {
            // forward the message to the other nodes
            if (m->status == M_RX_FROM_UART) {
                can_send_fmessage_any_ch(m);
            }
        }
    }
    
    //TODO: check this if!!
    if (m->request_message_bit && (m->identifier == ADDRESS_GATEWAY) && (controller_address != ADDRESS_UNSET)) {
        // send message back to gateway
        message_t m_discovery;
        message_init(&m_discovery,
                controller_address,
                MESSAGE_NO_REQUEST,
                M_DISCOVERY,
                NO_SENSOR_ID,
                NULL,
                0);

        message_send(&m_discovery);
    }

    if (!m->request_message_bit) {
        send_message_uart(m);
    }
}

static void cmd_node_info(const message_t *m) {
    if ((m->identifier != controller_address) && (!m->request_message_bit)) {
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
            NO_SENSOR_ID,
            data,
            ARRAY_LENGTH(data));
    message_send(&i);
}

static void cmd_sensor_config(const message_t *m) {
    if (m->request_message_bit) {
        // send config
        uint8_t reg = 0, length;
        uint8_t buffer[8];
        message_t m_sensor;
        do {
            sensor_get_config(m->sensor_identifier, reg, buffer, &length);
            reg++;
            if (length > 0) {
                message_init(&m_sensor, controller_address,
                        MESSAGE_NO_REQUEST,
                        m->command,
                        m->sensor_identifier,
                        buffer,
                        length);
                message_send(&m_sensor);
            }
        } while (length > 0);
    } else {
        sensor_set_config_from_buffer((uint8_t) m->sensor_identifier, m->data, m->length);
    }
    
#ifdef __DICIO__
    if (!m->request_message_bit) {
        dicio_process_node_config(m);
    }
#endif
    
}

static void cmd_sensor_config_end(const message_t* m)
{
    #ifdef __DICIO__
    if (!m->request_message_bit) {
        dicio_process_node_config(m);
    }
    #endif
}

static void cmd_sensor_status(const message_t *m) {
    if (m->request_message_bit) {
        sensor_send_status(m->sensor_identifier);
    } else {
        sensor_set_status(m->sensor_identifier, m->data[0]);
    }
}

static void cmd_sensor_error(const message_t *m) {
}

static void cmd_sensor_data(const message_t *m) {
}

static void cmd_sensor_start(const message_t* m) {
    sensor_start();
}

static void cmd_sensor_stop(const message_t* m) {
    sensor_stop();
}

static void cmd_data_clear(const message_t* m) {
#ifdef __DICIO__
    if (m->request_message_bit) {
        dicio_clear_data();
    }
#endif
}

static void cmd_data_read(const message_t* m) {
#ifdef __DICIO__
    if (m->request_message_bit) {
        dicio_dump_sdcard_data(DICIO_DATA_START_ADDRESS, SD_SPI_GetSectorCount());
    } else {
        if(m->length == 8)
        {
            uint32_t sector_start = 0, sector_stop = 0;
            for(int i = 0; i < 4; i++)
            {
                sector_start = (sector_start << 8) | m->data[i];
                sector_stop = (sector_stop << 8) | m->data[i+4];
            }
            dicio_dump_sdcard_data(sector_start, sector_stop);
        }
    }
#endif
}

static void cmd_data_write(const message_t* m) {
#ifdef __DICIO__
    // TODO
#endif
}
