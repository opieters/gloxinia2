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

void dicio_message_process(const message_t* m)
{
    switch(m->command)
    {
        case M_NODE_INFO:
            UART_DEBUG_PRINT("NODE_INFO_DICIO");
            dicio_register_node(m);
            break;
        case M_SENSOR_DATA:
            UART_DEBUG_PRINT("SENSOR_DATA");
            cmd_sensor_data(m);
            break;
        case M_SENSOR_CONFIG:
            UART_DEBUG_PRINT("SENSOR_CONFIG");
            cmd_sensor_config(m);
            break;
        case M_SENSOR_CONFIG_END:
            UART_DEBUG_PRINT("SENSOR_CONFIG_END");
            cmd_sensor_config_end(m);
            break;
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
        case M_CONFIG_SAVED:
            UART_DEBUG_PRINT("M_CONFIG_DONE_FINISHED_READOUT");
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