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

static uint8_t dicio_sector_buffer[SDCARD_SECTOR_SIZE];


void dicio_storage_message_process(const message_t* m)
{
    switch(m->command)
    {
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
        case M_CONFIG_DONE_FINISHED_READOUT:
            UART_DEBUG_PRINT("M_CONFIG_DONE_FINISHED_READOUT");
            break;
        default:
            break;
    }
}


void cmd_config_done_start_readout(const message_t* m)
{
    // send a message to each of the connected nodes to request their configuration data
    // to do this, we use a schedule since this allows for easy async operation and also to check if a node became unresponsive
    
    dicio_node_config_readout_counter = 0;
    
    task_t task = {dicio_config_node_config_readout, NULL};
    schedule_init(&dicio_node_config_readout, task, 1);
    schedule_specific_event(&dicio_node_config_readout, ID_DICIO_NODE_CONFIG_READOUT);
}

void dicio_config_node_config_readout(void *data)
{
    message_t m;
    if(dicio_node_config_readout_counter == n_nodes)
    {
        // stop schedule, send message to GUI that readout is complete
        message_init(&m,
            controller_address,
            false,
            M_CONFIG_DONE_FINISHED_READOUT,
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
        for(int i = 0; i < node_configs[dicio_node_config_readout_counter].n_interfaces; i++){
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

void dicio_read_sdconfig_data(void)
{
    uint8_t dicio_sector_buffer[SDCARD_SECTOR_SIZE];

    if (!SD_SPI_SectorRead(DICIO_CONFIG_ADDRESS, dicio_sector_buffer, 1))
    {
        UART_DEBUG_PRINT("Unable to read sector 0!");
    }
    // TODO
}

void dicio_write_sdconfig_data(void)
{
    uint8_t dicio_sector_buffer[SDCARD_SECTOR_SIZE];
    uint16_t ctime[4];
    size_t write_counter = 0;
    uint32_t address = DICIO_CONFIG_ADDRESS;
    int i;

    // clear sector buffer
    clear_buffer(dicio_sector_buffer, ARRAY_LENGTH(dicio_sector_buffer));

    // write data into the buffer
    // write software version
    dicio_sector_buffer[write_counter++] = SOFTWARE_VERSION_MAJOR;
    dicio_sector_buffer[write_counter++] = SOFTWARE_VERSION_MINOR;
    // write hardware version
    dicio_sector_buffer[write_counter++] = __HARDWARE_VERSION__;
    // write current time
    clock_get_raw_time(ctime);
    for (int i = 0; i < ARRAY_LENGTH(ctime); i++)
    {
        dicio_sector_buffer[write_counter++] = (uint8_t)((ctime[i] >> 8) & 0xff);
        dicio_sector_buffer[write_counter++] = (uint8_t)(ctime[i] & 0xff);
    }

    // write number of connected nodes
    dicio_sector_buffer[write_counter++] = n_nodes;

    SD_SPI_SectorWrite(address++, dicio_sector_buffer, 1);

    clear_buffer(dicio_sector_buffer, ARRAY_LENGTH(dicio_sector_buffer));

    // write sensor configuration to buffer
    for (i = 0; i < N_SENSOR_INTERFACES; i++)
    {
        if (sizeof(sensor_interfaces[i]) >= SDCARD_SECTOR_SIZE)
        {
            UART_DEBUG_PRINT("Sensor configuration does not fit in a single sector!");
            break;
        }

        // load data into buffer and size
        memcpy(&dicio_sector_buffer[0], &sensor_interfaces[i], sizeof(sensor_interfaces[i]));
        write_counter += sizeof(sensor_interfaces[i]);
        SD_SPI_SectorWrite(address++, dicio_sector_buffer, 1);
        clear_buffer(dicio_sector_buffer, ARRAY_LENGTH(dicio_sector_buffer));
    }
}

void dicio_write_node_configs_sd(void)
{
    // loop over nodes
    for (int i = 0; i < n_nodes; i++)
    {
        node_config_t *config = &node_configs[i];

        config->stored_config = false;

        if (i == 0)
        {
            message_t m;

            message_init(&m, config->node_id,
                         MESSAGE_REQUEST,
                         M_SENSOR_CONFIG,
                         NO_INTERFACE_ID,
                         NO_SENSOR_ID,
                         NULL,
                         0);

            message_send(&m);
        }
    }
}

void dicio_register_node(message_t *m)
{
    // check if we can still register a new node
    if (n_nodes >= ARRAY_LENGTH(node_configs))
        return;

    // search if m exists already
    for (int i = 0; i < n_nodes; i++)
    {
        if (m->identifier == node_configs[i].node_id)
        {
            return;
        }
    }

    // node not found -> register new node
    node_config_t *config = &node_configs[n_nodes];
    n_nodes++;

    if (n_nodes >= 2)
        config->sector_address = node_configs[n_nodes - 2].sector_address + node_configs[n_nodes - 2].n_interfaces + 1;
    else
        config->sector_address = DICIO_NODE_CONFIG_START_ADDRESS;

    switch (m->data[0])
    {
    case M_NODE_DICIO:
        config->n_interfaces = DICIO_N_SENSOR_IFS;
        break;
    case M_NODE_SYLVATICA:
        config->n_interfaces = SYLVATICA_N_SENSOR_IFS;
        break;
    case M_NODE_PLANALTA:
        config->n_interfaces = PLANALTA_N_SENSOR_IFS;
        break;
    default:
        config->n_interfaces = 0;
        break;
    }

    config->v_hw = m->data[1];
    config->v_sw_u = m->data[2];
    config->v_sw_l = m->data[3];

    config->node_id = m->identifier;
    config->stored_config = false;
}

void dicio_process_node_config(const message_t *m)
{
    static uint16_t write_counter = 0;
    message_t m_rqst;

    UART_DEBUG_PRINT("STORING SENSOR CONFIG OF OTHER NODE");
    
    // this clears all buffers
    if (m == NULL)
    {
        clear_buffer(dicio_sector_buffer, ARRAY_LENGTH(dicio_sector_buffer));
        write_counter = 0;
    }

    // this command triggers the write operation
    if (m->command == M_SENSOR_CONFIG_END)
    {
        uint32_t address;
        bool match_found = false;
        int i;

        // find node with matching id
        for (i = 0; i < n_nodes; i++)
        {
            if (node_configs[i].node_id == m->identifier)
            {
                match_found = true;
                break;
            }
        }

        if (!match_found)
            return;

        // construct address
        address = node_configs[i].sector_address + 1 + ((m->interface_id << 4) | m->sensor_id);

        // write data to SD card
        SD_SPI_SectorWrite(address, dicio_sector_buffer, 1);
        clear_buffer(dicio_sector_buffer, ARRAY_LENGTH(dicio_sector_buffer));

        // request next interface if it exists
        if ((m->interface_id + 1) < (node_configs[i].n_interfaces))
        {
            message_init(&m_rqst, node_configs[i].node_id,
                         MESSAGE_REQUEST,
                         M_SENSOR_CONFIG,
                         m->interface_id,
                         m->sensor_id + 1,
                         NULL,
                         0);

            message_send(&m_rqst);
        }
        else
        {
            // config readout complete -> set bool
            node_configs[i].stored_config = true;

            // if more nodes exist, request that data also
            if ((++i) < n_nodes)
            {
                message_init(&m_rqst, node_configs[i].node_id,
                             MESSAGE_REQUEST,
                             M_SENSOR_CONFIG,
                             NO_INTERFACE_ID,
                             NO_SENSOR_ID,
                             NULL,
                             0);

                message_send(&m_rqst);
            }
        }

        return;
    }

    // write message meta data
    dicio_sector_buffer[write_counter++] = SDCARD_START_BYTE;
    dicio_sector_buffer[write_counter++] = m->command;
    dicio_sector_buffer[write_counter++] = m->interface_id;
    dicio_sector_buffer[write_counter++] = m->sensor_id;
    dicio_sector_buffer[write_counter++] = m->length;

    // write message data
    for (int i = 0; i < m->length; i++)
    {
        dicio_sector_buffer[write_counter++] = m->data[i];
    }
    dicio_sector_buffer[write_counter++] = SDCARD_STOP_BYTE;
}

bool dicio_clear_data(void)
{
    uint32_t n_sectors = SD_SPI_GetSectorCount();
    uint8_t dicio_sector_buffer[SDCARD_SECTOR_SIZE];
    for (int i = 0; i < SDCARD_SECTOR_SIZE; i++)
        dicio_sector_buffer[i] = 0;

    for (uint32_t i = DICIO_DATA_START_ADDRESS; i < n_sectors; i++)
    {
        if (!SD_SPI_SectorWrite(i, dicio_sector_buffer, 1))

            return false;
    }

    return true;
}

void dicio_dump_sdcard_data(uint32_t sector_start, uint32_t sector_stop)
{
    uint8_t dicio_sector_buffer[SDCARD_SECTOR_SIZE];
    message_t m;
    uint8_t m_data[CAN_MAX_N_BYTES];
    uint32_t i;

    if (sector_stop < sector_start)
    {
        return;
    }

    // send start address of first sector
    for (int i = 0; i < 4; i++)
    {
        m_data[3 - i] = (sector_start >> (8 * i));
    }
    message_init(&m,
                 controller_address,
                 MESSAGE_NO_REQUEST,
                 M_DATA_BURST_START,
            NO_INTERFACE_ID,
                 NO_SENSOR_ID,
                 m_data,
                 4);
    message_send(&m);

    message_init(&m,
                 controller_address,
                 MESSAGE_NO_REQUEST,
                 M_DATA_BURST,
            NO_INTERFACE_ID,
                 NO_SENSOR_ID,
                 m_data,
                 8);

    // loop over all sectors
    for (i = sector_start; i < sector_stop; i++)
    {
        if (SD_SPI_SectorRead(i, dicio_sector_buffer, 1))
        {
            for (uint16_t j = 0; j < SDCARD_SECTOR_SIZE; j++)
            {
                m_data[j % CAN_MAX_N_BYTES] = dicio_sector_buffer[j];

                // send 8 bytes at a time
                if ((j % CAN_MAX_N_BYTES) == (CAN_MAX_N_BYTES - 1))
                {
                    message_send(&m);
                }
            }
        }
        else
        {
            // send address of the last sector (failed readout)
            for (int j = 0; j < 4; i++)
            {
                m_data[3 - j] = (i >> (8 * j));
            }

            message_init(&m,
                         controller_address,
                         MESSAGE_NO_REQUEST,
                         M_DATA_BURST_STOP,
                    NO_INTERFACE_ID,
                         NO_SENSOR_ID,
                         m_data,
                         4);
            message_send(&m);

            return;
        }
    }

    // send address of the last sector (not incl.)
    for (int i = 0; i < 4; i++)
    {

        m_data[3 - i] = (sector_stop >> (8 * i));
    }

    message_init(&m,
                 controller_address,
                 MESSAGE_NO_REQUEST,
                 M_DATA_BURST_STOP,
            NO_INTERFACE_ID,
                 NO_SENSOR_ID,
                 m_data,
                 4);
    message_send(&m);
}

void cmd_data_clear(const message_t* m) {
#ifdef __DICIO__
    if (m->request_message_bit) {
        dicio_clear_data();
    }
#endif
}

void cmd_data_read(const message_t* m) {
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

void cmd_data_write(const message_t* m) {
#ifdef __DICIO__
    // TODO
#endif
}
