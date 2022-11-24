#include <message.h>
#include <utilities.h>
#include <can.h>
#include <uart.h>
#include <address.h>
#include <event_controller.h>
#include <i2c.h>
#include <sensor.h>

// internal functions
static void cmd_request_address_available(message_t *m);
static void cmd_address_taken(message_t *m);
static void cmd_update_address(message_t *m);
static void cmd_discovery(message_t *m);
static void cmd_node_info(message_t *m);
static void cmd_sensor_config(message_t *m);
static void cmd_sensor_status(message_t *m);
static void cmd_sensor_error(message_t *m);
static void cmd_sensor_data(message_t *m);

static void address_get_task(void *data);

extern bool uart_connection_active;

void message_init(message_t *m,
                  uint16_t identifier,
                  bool request_message_bit,
                  message_cmd_t command,
                  uint16_t sensor_identifier,
                  uint8_t *data,
                  uint8_t length)
{
    m->command = command;
    m->identifier = identifier;
    m->sensor_identifier = sensor_identifier;
    m->length = length;
    m->data = data;
    m->status = M_TX_INIT_DONE;
    m->request_message_bit = request_message_bit;
}

void message_reset(message_t *m)
{
    if ((m->status != M_TX_QUEUED))
    {
        m->status = M_TX_INIT_DONE;
    }
}

void send_message_uart(message_t *m)
{
    message_t m_uart;

    if (!uart_connection_active)
    {
        return;
    }

    m_uart = *m;
    message_reset(&m_uart);
    uart_queue_message(&m_uart);
}

void send_message_can(message_t *m)
{
    can_message_t mc;

    // cap length
    m->length = MIN(8, m->length);
    can_init_message(&mc, m->identifier, m->request_message_bit, CAN_EXTENDED_FRAME, CAN_HEADER(m->command, m->sensor_identifier), m->data, m->length);
    can_send_message_any_ch(&mc);
}

void message_send(message_t *m)
{
    switch (m->status)
    {
    case M_RX_FROM_CAN:
    case M_RX_FROM_UART:
    case M_TX_QUEUED:
    case M_TX_SENT:
    case M_ERROR:
        break;
    case M_TX_INIT_DONE:
        if (uart_connection_active)
        {
            send_message_can(m);
            send_message_uart(m);
        }
        else
        {
            send_message_can(m);
        }
        break;
    }
}

void message_process(message_t *m)
{
    switch (m->command)
    {
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
    case M_NODE_INFO:
        cmd_node_info(m);
        break;
    case M_SENSOR_STATUS:
        cmd_sensor_status(m);
        break;
    case M_SENSOR_ERROR:
        cmd_sensor_error(m);
        break;
    case M_SENSOR_DATA:
        cmd_sensor_data(m);
        break;
    case M_SENSOR_CONFIG:
        cmd_sensor_config(m);
        break;
    case M_NODE_RESET:
    {
        can_disable();
        i2c1_disable();
        i2c2_disable();
        asm("RESET");

        break;
    }
    default:
        break;
    }
}

static void cmd_request_address_available(message_t *m)
{
    if (m->identifier == controller_address)
    {
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
    }
    else
    {
        UART_DEBUG_PRINT("CAN address not taken by this node.");
    }
}

static void cmd_address_taken(message_t *m)
{
    if (controller_address == m->identifier)
    {
        controller_address = ADDRESS_UNSET;

        UART_DEBUG_PRINT("Cleared address.");

        // if an unsuccessful attempt was made to set the gateway node address,
        // we must report this such that an error can be displayed by the GUI
        // to inform the user that there is a connectivity issue
        if (uart_connection_active)
        {
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
        task_t task_get_address = {address_get_task, NULL};

        push_queued_task(task_get_address);
    }
}

static void address_get_task(void *data)
{
    address_get();
}

static void cmd_update_address(message_t *m)
{
    if ((m->identifier == controller_address) && (m->length == 2))
    {
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

static void cmd_discovery(message_t *m)
{
    can_reset();

    if (m->request_message_bit && (m->identifier == ADDRESS_GATEWAY))
    {
        if (controller_address == ADDRESS_UNSET)
        {
            address_get();
            return;
        }
        else
        {
            // forward the message to the other nodes
            if (m->status == M_RX_FROM_UART)
            {
                can_send_fmessage_any_ch(m);
            }
        }
    }
    if (m->request_message_bit && (m->identifier == ADDRESS_GATEWAY) && (controller_address != ADDRESS_UNSET))
    {
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

    if (!m->request_message_bit)
    {
        send_message_uart(m);
    }
}

static void cmd_node_info(message_t *m)
{
    if ((m->identifier != controller_address) && (!m->request_message_bit))
    {
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

static void cmd_sensor_config(message_t *m)
{
    sensor_set_config_from_buffer((uint8_t)m->sensor_identifier, m->data, m->length);
}

static void cmd_sensor_status(message_t *m)
{
    if (m->identifier == ADDRESS_GATEWAY)
    {
    }
    else
    {
    }
}

static void cmd_sensor_error(message_t *m)
{
}

static void cmd_sensor_data(message_t *m)
{
}