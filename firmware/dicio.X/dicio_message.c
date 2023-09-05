#include "dicio.h"
#include "sdcard.h"
#include <sensor.h>
#include <sensor_common.h>
#include <utilities.h>
#include <rtcc.h>
#include <address.h>

extern uint8_t n_nodes;
extern node_config_t node_configs[DICIO_MAX_N_NODES];
task_schedule_t dicio_broadcast_start_schedule;
task_schedule_t dicio_broadcast_stop_schedule;
uint8_t dicio_boardcast_start_node_counter = 0;
uint8_t dicio_boardcast_stop_node_counter = 0;


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
            //UART_DEBUG_PRINT("CAN_ADDRESS_TAKEN");
            if(m->identifier == controller_address)
                cmd_address_taken(m);
            break;
        case M_SENSOR_START:
            //UART_DEBUG_PRINT("M_SENSOR_START");
            if(m->identifier == controller_address)
            {
                if(m->request_message_bit)
                    dicio_broadcast_measurement_start(m);
                
                // store variable in memory to save that we started
                uint8_t buffer[SDCARD_SECTOR_SIZE];
                SD_SPI_SectorRead(DICIO_MEASUREMENT_RUNNING_ADDRESS, buffer, 1);
                buffer[0] = 1;
                SD_SPI_SectorWrite(DICIO_MEASUREMENT_RUNNING_ADDRESS, buffer, 1);
                
                cmd_sensor_start(m);
            }
            break;
         case M_CONFIG_SAVED:
            //UART_DEBUG_PRINT("M_CONFIG_SAVED");
            break;
        case M_DATA_BURST:
            //UART_DEBUG_PRINT("DATA_BURST");
            break;
        case M_DATA_BURST_START:
            //UART_DEBUG_PRINT("DATA_BURST_STOP");
            break;
        case M_DATA_BURST_STOP:
            //UART_DEBUG_PRINT("DATA_BURST_STOP");
            break;
        case M_DATA_CLEAR:
            //UART_DEBUG_PRINT("DATA_CLEAR");
            if(m->identifier == controller_address)
                cmd_data_clear(m);
            break;
        case M_DATA_READ:
            //UART_DEBUG_PRINT("DATA_READ");
            if(m->identifier == controller_address)
                cmd_data_read(m);
            break;
        case M_DATA_WRITE:
            //UART_DEBUG_PRINT("DATA_WRITE");
            if(m->identifier == controller_address)
                cmd_data_write(m);
            break;
        case M_DISCOVERY:
            //UART_DEBUG_PRINT("CAN_DISCOVERY");
            if((m->identifier == controller_address) || (m->identifier == ADDRESS_GATEWAY))
                cmd_discovery(m);
            break;
        case M_HELLO:
            //UART_DEBUG_PRINT("CAN_HELLO");
            break;
        case M_MSG_TEXT:
            //UART_DEBUG_PRINT("MSG_TEXT");
            send_message_uart(m);
            break;
        case M_NODE_INFO:
            //UART_DEBUG_PRINT("NODE_INFO");
            if(m->identifier == controller_address)
                cmd_node_info(m);
            else
                dicio_register_node(m);
            break;
        case M_NODE_RESET:
        {
            //UART_DEBUG_PRINT("NODE_RESET");
            can_disable();
            i2c1_disable();
            i2c2_disable();
            asm("RESET");

            break;
        }
        case M_REQUEST_ADDRESS_AVAILABLE:
            //UART_DEBUG_PRINT("CAN_REQUEST_ADDRESS_AVAILABLE");
            if(m->identifier == controller_address)
                cmd_request_address_available(m);
            break;
        case M_UPDATE_ADDRESS:
            //UART_DEBUG_PRINT("CAN_UPDATE_ADDRESS");
            if(m->identifier == controller_address)
                cmd_update_address(m);
            break;
        case M_SENSOR_CONFIG:
            //UART_DEBUG_PRINT("SENSOR_CONFIG");
            if(m->identifier == controller_address)
                cmd_sensor_config(m);
            else
                dicio_process_node_config(m);
            break;
        case M_SENSOR_ERROR:
            //UART_DEBUG_PRINT("SENSOR_ERROR");
            cmd_sensor_error(m);
            break;
        case M_SENSOR_STATUS:
            //UART_DEBUG_PRINT("SENSOR_STATUS");
            if(m->identifier == controller_address)
                cmd_sensor_status(m);
            break;
        case M_SENSOR_STOP:
            //UART_DEBUG_PRINT("SENSOR_STOP");
            if(m->identifier == controller_address)
            {
                if(m->request_message_bit)
                    dicio_broadcast_measurement_stop(m);
                
                cmd_sensor_stop(m);
                
                // store variable in memory to save that we started
                uint8_t buffer[SDCARD_SECTOR_SIZE];
                SD_SPI_SectorRead(DICIO_MEASUREMENT_RUNNING_ADDRESS, buffer, 1);
                buffer[0] = 0;
                SD_SPI_SectorWrite(DICIO_MEASUREMENT_RUNNING_ADDRESS, buffer, 1);
            }
            break;
        case M_SENSOR_CONFIG_END:
            //UART_DEBUG_PRINT("SENSOR_CONFIG_END");
            if(m->identifier == controller_address)
                cmd_sensor_config_end(m);
            break;
        case M_SENSOR_DATA:
            //UART_DEBUG_PRINT("SENSOR_DATA");
            cmd_sensor_data(m);
            break;
        case M_DICIO_LOAD_CONFIGURATION_FROM_SDCARD:
            cmd_dicio_load_configuration_from_sdcard(m);
            break;
        case M_DICIO_CLEAR_CONFIGURATION_ON_SDCARD:  
            cmd_dicio_clear_configuration_on_sdcard(m);
            break;
        case M_DICIO_TIME:
            cmd_dicio_time(m);
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


void dicio_broadcast_measurement_start(const message_t* m)
{
    // send a message to each of the connected nodes to request their configuration data
    // to do this, we use a schedule since this allows for easy async operation
    
    dicio_boardcast_start_node_counter = 0;
    
    task_t task;
    task_init(&task, dicio_config_node_config_readout, NULL);
    schedule_init(&dicio_broadcast_start_schedule, task, 1);
    schedule_specific_event(&dicio_broadcast_start_schedule, ID_DICIO_BORADCAST_START);
}

void dicio_config_node_config_readout(void *data)
{
    message_t m;
    
    if(dicio_boardcast_start_node_counter != 0)
    {
        // start sampling data
        message_init(&m,
            node_configs[dicio_boardcast_start_node_counter - 1].node_id,
            true,
            M_SENSOR_START,
            NO_INTERFACE_ID,
            NO_SENSOR_ID,
            NULL,
            0);
        message_send(&m);
    }
    
    if(dicio_boardcast_start_node_counter == n_nodes)
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
        
        schedule_remove_event(ID_DICIO_BORADCAST_START);
    } else {
        // clear configuration
        dicio_process_node_config(NULL);
        
        // readout each of the sensor interfaces
        for(int i = 0; i < node_configs[dicio_boardcast_start_node_counter].n_interfaces; i++)
        {
            for(int j = 0; j < 4; j++){
                message_init(&m,
                    node_configs[dicio_boardcast_start_node_counter].node_id,
                    true,
                    M_SENSOR_CONFIG,
                    i,
                    j,
                    NULL,
                    0);
                message_send(&m);
            }
        }
        
        dicio_boardcast_start_node_counter++;
    }
}

void dicio_broadcast_measurement_stop(const message_t* m)
{    
    dicio_boardcast_stop_node_counter = 0;

    task_t task;
    task_init(&task, dicio_broadcast_stop_task, NULL);
    schedule_init(&dicio_broadcast_stop_schedule, task, 1);
    schedule_specific_event(&dicio_broadcast_stop_schedule, ID_DICIO_BORADCAST_STOP);
}

void dicio_broadcast_stop_task(void *data)
{
    message_t m;
    
    if(n_nodes > 0)
    {
        if(dicio_boardcast_stop_node_counter == (n_nodes - 1))
        {
            schedule_remove_event(ID_DICIO_BORADCAST_STOP);
        }
        
        // send stop message
        message_init(&m,
            node_configs[dicio_boardcast_stop_node_counter].node_id,
            true,
            M_SENSOR_STOP,
            NO_INTERFACE_ID,
            NO_SENSOR_ID,
            NULL,
            0);
        message_send(&m);
    } else {
        schedule_remove_event(ID_DICIO_BORADCAST_STOP);
    }
    
    dicio_boardcast_stop_node_counter++;
}


void cmd_sensor_data(const message_t *m) {
    sensor_save_data(m->identifier, m->interface_id, m->sensor_id, m->data, m->length);
}

void cmd_dicio_load_configuration_from_sdcard(const message_t* m)
{
    if((!m->request_message_bit) || (m->identifier != ADDRESS_GATEWAY))
        return;
        
    dicio_load_node_configs();
}
void cmd_dicio_clear_configuration_on_sdcard(const message_t* m)
{
    uint8_t dicio_sector_buffer[SDCARD_SECTOR_SIZE];
    uint32_t i, j;
    
    
    
    for (i = 0; i < DICIO_DATA_START_ADDRESS; i++)
    {
        for(j = 0; j < ARRAY_LENGTH(dicio_sector_buffer); j++){
            dicio_sector_buffer[j] = 0;
        }
        
        SD_SPI_SectorWrite(i, dicio_sector_buffer, 1);
    }
}

void cmd_dicio_time(const message_t* m)
{
    message_t reply;
    uint8_t data[CAN_MAX_N_BYTES];
    clock_time_t ct;
    
    if((m->identifier == controller_address) && (m->request_message_bit))
    {
        // send current time config

        clock_get_time(&ct);
        data[0] = ct.year;
        data[1] = ct.month;
        data[2] = ct.day;
        data[3] = ct.wday;
        data[4] = ct.hour;
        data[5] = ct.minute;
        data[6] = ct.second;

        message_init(&reply,
            controller_address,
            false,
            M_DICIO_TIME,
            NO_INTERFACE_ID,
            NO_SENSOR_ID,
            data,
            7); // better to use sizeof()?
        message_send(&reply);
    }
    
    // received time from computer, update
    if((m->length == 7) && (m->identifier == ADDRESS_GATEWAY))
    {
        ct.year = m->data[0];
        ct.month = m->data[1];
        ct.day = m->data[2];
        ct.wday = m->data[3];
        ct.hour = m->data[4];
        ct.minute = m->data[5];
        ct.second = m->data[6];

        UART_DEBUG_PRINT("Setting time");
        
        clock_set_time(&ct);
    }
}