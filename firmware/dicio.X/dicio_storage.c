#include "dicio.h"
#include "sdcard.h"
#include <sensor.h>
#include <sensor_common.h>
#include <utilities.h>
#include <rtcc.h>
#include <address.h>

extern uint8_t n_nodes;
extern node_config_t node_configs[DICIO_MAX_N_NODES];
static uint8_t dicio_sector_buffer[SDCARD_SECTOR_SIZE];

void dicio_load_node_configs(void)
{
    uint8_t dicio_sector_buffer[SDCARD_SECTOR_SIZE];
    message_t m_tx;
    uint32_t i;
    uint16_t j = 0;
    message_cmd_t cmd;
    uint8_t interface_id, sensor_id, length;
    
    int state = 0;

    // start from here
    i = DICIO_NODE_CONFIG_START_ADDRESS; 

    n_nodes = 0;

    // loop over all sectors
    while(i < DICIO_DATA_START_ADDRESS)
    {
        if (SD_SPI_SectorRead(i, dicio_sector_buffer, 1))
        {
            // check if sector is empty -> move to next sector 
            if(buffer_is_empty(dicio_sector_buffer, ARRAY_LENGTH(dicio_sector_buffer))){
                i = i + DICIO_NODE_N_SECTORS - ((i - DICIO_NODE_CONFIG_START_ADDRESS) % DICIO_NODE_N_SECTORS); 
                continue;
            }

            // general info
            if(((i - DICIO_NODE_CONFIG_START_ADDRESS) % DICIO_NODE_N_SECTORS) == 0) 
            {
                j = 0;
                
                node_config_t* node_config = &node_configs[n_nodes];
                clear_buffer(dicio_sector_buffer, ARRAY_LENGTH(dicio_sector_buffer));

                node_config->node_id = dicio_sector_buffer[j++];
                node_config->node_id = (node_config->node_id << 8) | dicio_sector_buffer[j++];

                node_config->node_type = dicio_sector_buffer[j++];
                node_config->n_interfaces = dicio_sector_buffer[j++];

                node_config->v_hw = dicio_sector_buffer[j++];
                node_config->v_sw_u = dicio_sector_buffer[j++];
                node_config->v_sw_l = dicio_sector_buffer[j++];
                
                n_nodes++;
            // configuration info (= wrapped messages) -> send messages over CAN
            } else {
                j = 0;
                while(j < SDCARD_SECTOR_SIZE)
                {
                    switch(state)
                    {
                        // detect start byte
                        case 0:
                            if(dicio_sector_buffer[j++] == SDCARD_START_BYTE)
                            {
                                state = 1;
                            }
                            break;
                        // read message
                        case 1:
                            if(j < (SDCARD_SECTOR_SIZE-4)){
                                cmd = dicio_sector_buffer[j++];
                                interface_id = dicio_sector_buffer[j++];
                                sensor_id = dicio_sector_buffer[j++];
                                length = dicio_sector_buffer[j++];
                                
                                if( (j+length) < SDCARD_SECTOR_SIZE) {
                                    message_init(&m_tx,
                                        node_configs[n_nodes-1].node_id,
                                        false,
                                        cmd,
                                        interface_id,
                                        sensor_id,
                                        &dicio_sector_buffer[j],
                                        length);
                                    j += length;
                                    
                                    state = 2;
                                }
                            } else {
                                // not enough data in buffer, move to end
                                j = SDCARD_SECTOR_SIZE;
                                state = 0;
                            }
                            
           
                            break;
                        // detect stop byte
                        case 2:
                            if(dicio_sector_buffer[j++] == SDCARD_STOP_BYTE)
                            {
                                message_send(&m_tx);
                                
                                state = 0;
                            }
                            break;
                        default:
                            state = 0;
                            break;
                    }
                }
            }
        }
        i++;
    }
}

void dicio_read_sdconfig_data(void)
{
    uint8_t dicio_sector_buffer[SDCARD_SECTOR_SIZE] = {0};

    if (!SD_SPI_SectorRead(DICIO_MEASUREMENT_RUNNING_ADDRESS, dicio_sector_buffer, 1))
    {
        UART_DEBUG_PRINT("Unable to read configuration data!");
    }
 
    // an experiment was running when the device was powered down, resume this experiment
    if(dicio_sector_buffer[0] == 1)
    {
        // search where last data frame was written
        
        // load sensor config of this node
        
        // load configuration data for each node from memory
        dicio_load_node_configs();
        
        // start readout
    }
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

void dicio_register_node(const message_t *m)
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
    
    dicio_save_node_info(config, n_nodes-1);
}

void dicio_save_node_info(node_config_t* config, uint8_t index)
{
    uint16_t write_counter = 0;
    clear_buffer(dicio_sector_buffer, ARRAY_LENGTH(dicio_sector_buffer));
    
    dicio_sector_buffer[write_counter++] = (uint8_t) config->node_id >> 8;
    dicio_sector_buffer[write_counter++] = (uint8_t) config->node_id;

    dicio_sector_buffer[write_counter++] = config->node_type;
    dicio_sector_buffer[write_counter++] = config->n_interfaces;

    dicio_sector_buffer[write_counter++] = config->v_hw;
    dicio_sector_buffer[write_counter++] = config->v_sw_u;
    dicio_sector_buffer[write_counter++] = config->v_sw_l;
    
    SD_SPI_SectorWrite(config->sector_address, dicio_sector_buffer, 1);
}

void dicio_process_node_config(const message_t *m)
{
    static uint16_t write_counter = 0;
    //message_t m_rqst;

    //UART_DEBUG_PRINT("STORING SENSOR CONFIG OF OTHER NODE");
    
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
        if ((m->interface_id + 1) == (node_configs[i].n_interfaces))
        {
            // config readout complete -> set bool
            node_configs[i].stored_config = true;
        }
    } else {
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
    bool auto_stop = (sector_stop == 0);
    
    if(auto_stop)
    {
        sector_stop = SDCARD_SECTOR_UPPER_ADDRESS;
    }

    if (sector_stop < sector_start)
    {
        return;
    }

    // send start address of first sector
    m_data[0] = (uint8_t) (sector_start >> 8);
    m_data[1] = (uint8_t) (sector_start);
    
    message_init(&m,
                 controller_address,
                 MESSAGE_NO_REQUEST,
                 M_DATA_BURST_START,
                 NO_INTERFACE_ID,
                 NO_SENSOR_ID,
                 m_data,
                 2);
    send_message_uart(&m);

    // loop over all sectors
    for (i = sector_start; i < sector_stop; i++)
    {
        if (SD_SPI_SectorRead(i, dicio_sector_buffer, 1))
        {
            // check if sector is empty, if so, stop sending data if auto_stop is on
            if(auto_stop)
            {                
                if(buffer_is_empty(dicio_sector_buffer, ARRAY_LENGTH(dicio_sector_buffer)))
                {
                    sector_stop = i;
                    continue;
                }
            }
            
            for (uint16_t j = 0; j < SDCARD_SECTOR_SIZE; j++)
            {
                m_data[j % CAN_MAX_N_BYTES] = dicio_sector_buffer[j];

                // send 8 bytes at a time
                if ((j % CAN_MAX_N_BYTES) == (CAN_MAX_N_BYTES - 1))
                {
                    message_init(&m,
                        controller_address,
                        MESSAGE_NO_REQUEST,
                        M_DATA_BURST,
                        NO_INTERFACE_ID,
                        NO_SENSOR_ID,
                        m_data,
                        8);
                    
                    send_message_uart(&m);
                }
            }
        }
        else
        {
            // send address of the last sector (failed readout)
            m_data[0] = (uint8_t) (i >> 8);
            m_data[1] = (uint8_t) (i);

            message_init(&m,
                         controller_address,
                         MESSAGE_NO_REQUEST,
                         M_DATA_BURST_STOP,
                        NO_INTERFACE_ID,
                         NO_SENSOR_ID,
                         m_data,
                         2);
            send_message_uart(&m);

            return;
        }
    }

    // send address of the last sector (not incl.)
    m_data[0] = (uint8_t) (sector_stop >> 8);
    m_data[1] = (uint8_t) (sector_stop);

    message_init(&m,
                 controller_address,
                 MESSAGE_NO_REQUEST,
                 M_DATA_BURST_STOP,
                 NO_INTERFACE_ID,
                 NO_SENSOR_ID,
                 m_data,
                 2);
    send_message_uart(&m);
}

void cmd_data_clear(const message_t* m) {
    if (m->request_message_bit) {
        dicio_clear_data();
    }
}

void cmd_data_read(const message_t* m) {
    if (m->request_message_bit) {
        dicio_dump_sdcard_data(DICIO_DATA_START_ADDRESS, SD_SPI_GetSectorCount());
    } else {
        if(m->length == 4)
        {
            uint32_t sector_start = (m->data[0] << 8) | (m->data[1]);
            uint32_t sector_stop = (m->data[2] << 8) | (m->data[3]);
            
            dicio_dump_sdcard_data(sector_start, sector_stop);
        }
    }
}

void cmd_data_write(const message_t* m) {
    uint8_t buffer[SDCARD_SECTOR_SIZE];

    if(m->length < 5)
        return;

    // calculate sector address
    uint32_t address = (((uint32_t) m->data[0]) << 24) | (((uint32_t) m->data[1]) << 16) | (((uint32_t) m->data[2]) << 8) | m->data[3];
    address -= address % SDCARD_SECTOR_SIZE;

    for(int i = 0; i < m->length - 4; i++)
    {
        buffer[(address % SDCARD_SECTOR_SIZE) + i] = m->data[i+4];
    }


    SD_SPI_SectorWrite(address, buffer, 1);
}
