#include "dicio.h"
#include "sdcard.h"
#include <sensor.h>
#include <sensor_common.h>
#include <utilities.h>
#include <rtcc.h>
#include <address.h>

extern uint8_t n_nodes;
extern node_config_t node_configs[DICIO_MAX_N_NODES];
task_schedule_t dicio_node_config_readout;
uint8_t dicio_node_config_readout_counter = 0;

static message_t m_error;
static uint8_t m_error_data[2];

void dicio_message_process(const message_t* m)
{
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
        case M_ADDRESS_TAKEN:
            UART_DEBUG_PRINT("CAN_ADDRESS_TAKEN");
            if(m->identifier == controller_address)
                cmd_address_taken(m);
            break;
        case M_CONFIG_DONE_START_READOUT:
            UART_DEBUG_PRINT("M_CONFIG_DONE_START_READOUT");
            if(m->identifier == controller_address)
                cmd_config_done_start_readout(m);
            break;
         case M_CONFIG_SAVED:
            UART_DEBUG_PRINT("M_CONFIG_SAVED");
            break;
        case M_DATA_BURST:
            UART_DEBUG_PRINT("DATA_BURST");
            break;
        case M_DATA_BURST_START:
            UART_DEBUG_PRINT("DATA_BURST_STOP");
            break;
        case M_DATA_BURST_STOP:
            UART_DEBUG_PRINT("DATA_BURST_STOP");
            break;
        case M_DATA_CLEAR:
            UART_DEBUG_PRINT("DATA_CLEAR");
            if(m->identifier == controller_address)
                cmd_data_clear(m);
            break;
        case M_DATA_READ:
            UART_DEBUG_PRINT("DATA_READ");
            if(m->identifier == controller_address)
                cmd_data_read(m);
            break;
        case M_DATA_WRITE:
            UART_DEBUG_PRINT("DATA_WRITE");
            if(m->identifier == controller_address)
                cmd_data_write(m);
            break;
        case M_DISCOVERY:
            UART_DEBUG_PRINT("CAN_DISCOVERY");
            if((m->identifier == controller_address) || (m->identifier == ADDRESS_GATEWAY))
                cmd_discovery(m);
            break;
        case M_HELLO:
            UART_DEBUG_PRINT("CAN_HELLO");
            break;
        case M_MSG_TEXT:
            UART_DEBUG_PRINT("MSG_TEXT");
            send_message_uart(m);
            break;
        case M_NODE_INFO:
            UART_DEBUG_PRINT("NODE_INFO");
            if(m->identifier == controller_address)
                cmd_node_info(m);
            dicio_register_node(m);
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
        case M_REQUEST_ADDRESS_AVAILABLE:
            UART_DEBUG_PRINT("CAN_REQUEST_ADDRESS_AVAILABLE");
            if(m->identifier == controller_address)
                cmd_request_address_available(m);
            break;
        case M_UPDATE_ADDRESS:
            UART_DEBUG_PRINT("CAN_UPDATE_ADDRESS");
            if(m->identifier == controller_address)
                cmd_update_address(m);
            break;
        case M_SENSOR_CONFIG:
            UART_DEBUG_PRINT("SENSOR_CONFIG");
            if(m->identifier == controller_address)
                cmd_sensor_config(m);
            // TODO: check if we need to handle this differently, since data
            // from the current node (dicio with UART) is also stored as
            // foreign node
            dicio_process_node_config(m);
            break;
        case M_SENSOR_ERROR:
            UART_DEBUG_PRINT("SENSOR_ERROR");
            cmd_sensor_error(m);
            break;
        case M_SENSOR_START:
            UART_DEBUG_PRINT("SENSOR_START");
            if(m->identifier == controller_address)
                cmd_sensor_start(m);
            break;
        case M_SENSOR_STATUS:
            UART_DEBUG_PRINT("SENSOR_STATUS");
            if(m->identifier == controller_address)
                cmd_sensor_status(m);
            break;
        case M_SENSOR_STOP:
            UART_DEBUG_PRINT("SENSOR_STOP");
            if(m->identifier == controller_address)
                cmd_sensor_stop(m);
            break;
        case M_SENSOR_CONFIG_END:
            UART_DEBUG_PRINT("SENSOR_CONFIG_END");
            if(m->identifier == controller_address)
                cmd_sensor_config_end(m);
            break;
        case M_SENSOR_DATA:
            UART_DEBUG_PRINT("SENSOR_DATA");
            cmd_sensor_data(m);
            break;
        default:
            break;
    }
}


void cmd_sensor_config_end(const message_t* m)
{
    if (!m->request_message_bit) {
        dicio_process_node_config(m);
    }
}


void cmd_config_done_start_readout(const message_t* m)
{
    // send a message to each of the connected nodes to request their configuration data
    // to do this, we use a schedule since this allows for easy async operation
    
    dicio_node_config_readout_counter = 0;
    
    task_t task = {dicio_config_node_config_readout, NULL};
    schedule_init(&dicio_node_config_readout, task, 1);
    schedule_specific_event(&dicio_node_config_readout, ID_DICIO_NODE_CONFIG_READOUT);
}

void dicio_config_node_config_readout(void *data)
{
    message_t m;
    
    if(dicio_node_config_readout_counter != 0)
    {
        // start sampling data
        message_init(&m,
            node_configs[dicio_node_config_readout_counter - 1].node_id,
            true,
            M_SENSOR_START,
            NO_INTERFACE_ID,
            NO_SENSOR_ID,
            NULL,
            0);
        message_send(&m);
    }
    
    if(dicio_node_config_readout_counter == n_nodes)
    {
        // stop schedule, send message to GUI that readout is complete
        message_init(&m,
            controller_address,
            false,
            M_CONFIG_SAVED,
            NO_INTERFACE_ID,
            NO_SENSOR_ID,
            NULL,
            0);
        message_send(&m);
        
        schedule_remove_event(ID_DICIO_NODE_CONFIG_READOUT);
    } else {
        // clear configuration
        dicio_process_node_config(NULL);
        
        // readout each of the sensor interfaces
        for(int i = 0; i < node_configs[dicio_node_config_readout_counter].n_interfaces; i++)
        {
            message_init(&m,
                node_configs[dicio_node_config_readout_counter].node_id,
                true,
                M_SENSOR_CONFIG,
                NO_INTERFACE_ID,
                NO_SENSOR_ID,
                NULL,
                0);
            message_send(&m);
        }
        
        dicio_node_config_readout_counter++;
    }
}


void cmd_sensor_data(const message_t *m) {
    sensor_save_data(m->identifier, m->interface_id, m->sensor_id, m->data, m->length);
}