#include "dicio.h"
#include <event_controller.h>
#include <message.h>
#include <address.h>
#include <uart.h>
#include <libpic30.h>
#include <can.h>
#include <i2c.h>
#include <sensor.h>
#include "sdcard.h"
#include <rtcc.h>
#include "mcc_generated_files/interrupt_manager.h"

// TODO REMOVE DEBUG INCLUDES
#include <sensor_apds9306_065.h>

uint8_t n_nodes = 0;
node_config_t node_configs[DICIO_MAX_N_NODES];

bool uart_connection_active = false;
uint32_t dicio_first_empty_sector = 0;

i2c_config_t dicio_i2c1_config = {
    .i2c_address = 0x0,
    .status = I2C_STATUS_PRIMARY_ON,
    .pw_sr_cb = I2C_NO_CALLBACK,
    .pr_sw_cb = I2C_NO_CALLBACK,
    .scl_pin = PIN_INIT(G, 2),
    .sda_pin = PIN_INIT(G, 3)};

i2c_config_t dicio_i2c2_config = {
    .i2c_address = 0x0,
    .status = I2C_STATUS_PRIMARY_ON,
    .pw_sr_cb = I2C_NO_CALLBACK,
    .pr_sw_cb = I2C_NO_CALLBACK,
    .scl_pin = PIN_INIT(F, 5),
    .sda_pin = PIN_INIT(F, 4)};

void dicio_init(void)
{
    dicio_init_pins();

    __builtin_enable_interrupts();

    uart_init(50000);

    UART_DEBUG_PRINT("Configured UART.");

    can_init();
    UART_DEBUG_PRINT("Initialised ECAN.");

    i2c1_init(&dicio_i2c1_config);
    UART_DEBUG_PRINT("Initialised I2C1.");

    i2c2_init(&dicio_i2c2_config);
    UART_DEBUG_PRINT("Initialised I2C2.");

    sensors_init();
    UART_DEBUG_PRINT("Initialised sensor interface.");

    event_controller_init();
    UART_DEBUG_PRINT("Initialised event controller.");

    dicio_init_node_configurations();

    if (SD_SPI_MediaInitialize() == true)
    {
        dicio_read_sdconfig_data();
        UART_DEBUG_PRINT("Initialised SD card");
    }
    else
    {
        UART_DEBUG_PRINT("Unable to initialise SD card");
    }

    // TODO:REMOVE???
    uart_connection_active = true;

    // task_schedule_t dicio_read_log = {{dicio_send_ready_message, NULL}, 1, 0};
    // schedule_specific_event(&dicio_read_log, ID_READY_SCHEDULE);

    // UART_DEBUG_PRINT("Detecting if other devices connected using CAN.");
    // can_detect_devices();

    /*if(controller_address == 0){
        _TRISD0 = 0;
        _RP64R = _RPOUT_OC2;
        init_trigger_generation();
    } else {
        _TRISD0 = 1;
        init_sample_detection();
    }*/

    // UART_DEBUG_PRINT("Initialising device address.");
    // address_get();
    // UART_DEBUG_PRINT("Initialised device address to 0x%x.", controller_address);

    __delay_ms(100);

    task_schedule_t dicio_read_log;
    task_t dicio_read_log_task = {dicio_send_ready_message, NULL};
    schedule_init(&dicio_read_log, dicio_read_log_task, 10);
    // schedule_specific_event(&dicio_read_log, ID_READY_SCHEDULE);
}

void dicio_init_node_configurations(void)
{
    for (int i = 0; i < ARRAY_LENGTH(node_configs); i++)
    {
        node_configs[i].node_id = ADDRESS_UNSET;
        node_configs[i].node_type = M_NODE_UNKNOWN;
        node_configs[i].sector_address = 0xffffffff;
        node_configs[i].n_interfaces = 0;
        node_configs[i].stored_config = false;
    }
}

void dicio_read_sdconfig_data(void)
{
    uint8_t sector_buffer[SDCARD_SECTOR_SIZE];

    if (!SD_SPI_SectorRead(DICIO_CONFIG_ADDRESS, sector_buffer, 1))
    {
        UART_DEBUG_PRINT("Unable to read sector 0!");
    }
    // TODO
}

void dicio_write_sdconfig_data(void)
{
    uint8_t sector_buffer[SDCARD_SECTOR_SIZE];
    uint16_t ctime[4];
    size_t write_counter = 0;
    uint32_t address = DICIO_CONFIG_ADDRESS;
    int i;

    // clear sector buffer
    clear_buffer(sector_buffer, ARRAY_LENGTH(sector_buffer));

    // write data into the buffer
    // write software version
    sector_buffer[write_counter++] = SOFTWARE_VERSION_MAJOR;
    sector_buffer[write_counter++] = SOFTWARE_VERSION_MINOR;
    // write hardware version
    sector_buffer[write_counter++] = __HARDWARE_VERSION__;
    // write current time
    clock_get_raw_time(ctime);
    for (int i = 0; i < ARRAY_LENGTH(ctime); i++)
    {
        sector_buffer[write_counter++] = (uint8_t)((ctime[i] >> 8) & 0xff);
        sector_buffer[write_counter++] = (uint8_t)(ctime[i] & 0xff);
    }

    // write number of connected nodes
    sector_buffer[write_counter++] = n_nodes;

    SD_SPI_SectorWrite(address++, sector_buffer, 1);

    clear_buffer(sector_buffer, ARRAY_LENGTH(sector_buffer));

    // write sensor configuration to buffer
    for (i = 0; i < N_SENSOR_INTERFACES; i++)
    {
        if (sizeof(sensor_interfaces[i]) >= SDCARD_SECTOR_SIZE)
        {
            UART_DEBUG_PRINT("Sensor configuration does not fit in a single sector!");
            break;
        }

        // load data into buffer and size
        memcpy(&sector_buffer[0], &sensor_interfaces[i], sizeof(sensor_interfaces[i]));
        write_counter += sizeof(sensor_interfaces[i]);
        SD_SPI_SectorWrite(address++, sector_buffer, 1);
        clear_buffer(sector_buffer, ARRAY_LENGTH(sector_buffer));
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
                         0,
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
    static uint8_t sector_buffer[SDCARD_SECTOR_SIZE];
    static uint16_t write_counter = 0;
    message_t m_rqst;

    // this clears all buffers
    if (m == NULL)
    {
        clear_buffer(sector_buffer, ARRAY_LENGTH(sector_buffer));
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
        address = node_configs[i].sector_address + 1 + m->sensor_identifier;

        // write data to SD card
        SD_SPI_SectorWrite(address, sector_buffer, 1);
        clear_buffer(sector_buffer, ARRAY_LENGTH(sector_buffer));

        // request next interface if it exists
        if ((m->sensor_identifier + 1) < (node_configs[i].n_interfaces))
        {
            message_init(&m_rqst, node_configs[i].node_id,
                         MESSAGE_REQUEST,
                         M_SENSOR_CONFIG,
                         m->sensor_identifier + 1,
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
                             0,
                             NULL,
                             0);

                message_send(&m_rqst);
            }
        }

        return;
    }

    // write message meta data
    sector_buffer[write_counter++] = SDCARD_START_BYTE;
    sector_buffer[write_counter++] = m->command;
    sector_buffer[write_counter++] = (uint8_t)((m->sensor_identifier >> 8) & 0xff);
    sector_buffer[write_counter++] = (uint8_t)(m->sensor_identifier & 0xff);
    sector_buffer[write_counter++] = m->length;

    // write message data
    for (int i = 0; i < m->length; i++)
    {
        sector_buffer[write_counter++] = m->data[i];
    }
    sector_buffer[write_counter++] = SDCARD_STOP_BYTE;
}

bool dicio_clear_data(void)
{
    uint32_t n_sectors = SD_SPI_GetSectorCount();
    uint8_t sector_buffer[SDCARD_SECTOR_SIZE];
    for (int i = 0; i < SDCARD_SECTOR_SIZE; i++)
        sector_buffer[i] = 0;

    for (uint32_t i = DICIO_DATA_START_ADDRESS; i < n_sectors; i++)
    {
        if (!SD_SPI_SectorWrite(i, sector_buffer, 1))

            return false;
    }

    return true;
}

void dicio_dump_sdcard_data(uint32_t sector_start, uint32_t sector_stop)
{
    uint8_t sector_buffer[SDCARD_SECTOR_SIZE];
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
                 NO_SENSOR_ID,
                 m_data,
                 4);
    message_send(&m);

    message_init(&m,
                 controller_address,
                 MESSAGE_NO_REQUEST,
                 M_DATA_BURST,
                 NO_SENSOR_ID,
                 m_data,
                 8);

    // loop over all sectors
    for (i = sector_start; i < sector_stop; i++)
    {
        if (SD_SPI_SectorRead(i, sector_buffer, 1))
        {
            for (uint16_t j = 0; j < SDCARD_SECTOR_SIZE; j++)
            {
                m_data[j % CAN_MAX_N_BYTES] = sector_buffer[j];

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
                 NO_SENSOR_ID,
                 m_data,
                 4);
    message_send(&m);
}

void dicio_init_pins(void)
{
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
#ifdef __dsPIC33EP256MU806__

    /*****************************
     * I2C2 pin configuration (HW interface I2C1)
     * D10 is connected to SDA2
     * D9 is connected to SCL2
     *****************************/

    _TRISD9 = 0;
    _TRISD10 = 0;

    // reset sequence
    _LATD9 = 1;
    _LATD10 = 0;
    __delay_ms(1);
    _LATD9 = 1;
    _LATD10 = 1;
    __delay_ms(1);

    _ODCD9 = 1;  // configure I2C pins as open drain output
    _ODCD10 = 1; // configure I2C pins as open drain outputs

    _ANSE2 = 0;  // configure nINT2 as digital pin
    _TRISE2 = 1; // configure nINT2 as digital input pin

    /*****************************
     * I2C1 pin configuration (hardware interface I2C2)
     * F4 is connected to SDA1
     * F5 is connected to SCL1
     *****************************/

    _TRISF4 = 0;
    _TRISF5 = 0;

    // reset sequence
    _LATF5 = 1;
    _LATF4 = 0;
    __delay_ms(1);
    _LATF4 = 1;
    _LATF5 = 1;
    __delay_ms(1);

    _ODCF4 = 1; // configure I2C pins as open drain output
    _ODCF5 = 1; // configure I2C pins as open drain output

    _ANSB4 = 0;  // configure nINT2 as digital pin
    _TRISB4 = 1; // configure nINT2 as digital input pin

    /**
     * UART connection to debug port
     * TX -> RG6 / PR118
     * RX -> RG8 / RP120
     * RTS -> RE5 / RP85
     * CTS -> RG7 / RPI119
     */

    _ANSE5 = 0;
    _TRISE5 = 0; // U1 RTS
    _RP85R = _RPOUT_U1RTS;

    _ANSG6 = 0;
    _TRISG6 = 0; // U1 TX
    _LATG6 = 0;
    _RP118R = _RPOUT_U1TX;

    _ANSG7 = 0;
    _TRISG7 = 1; // U1 CTS
    _U1CTSR = 119;

    _ANSG8 = 0;
    _TRISG8 = 1; // U1 RX
    _U1RXR = 120;

    /*
     * ECAN pin configuration
     * CAN TX -> D3/RP67
     * CAN RX -> B13/RPI46
     * CAN C1 -> B11/RPI43 (notification pin)
     * CAN C2 -> B10/RPI42 (detection pin)
     * TERM   -> B14 (termination trigger)
     * SYNC   -> B9 (synchronization trigger)
     **/
    _TRISD3 = 0; // CAN1 TX
    _RP67R = _RPOUT_C1TX;
    _ANSB13 = 0; // CAN1 RX
    _TRISB13 = 1;
    _C1RXR = 46;
    _ANSB11 = 0; // CAN C1
    _TRISB11 = 0;
    _LATB11 = 1;
    _ANSB10 = 0; // CAN C2
    _TRISB10 = 1;
    _CNPDB10 = 1; // enable pull-down
    _ANSB14 = 0;  // TERM
    _TRISB14 = 0;
    _LATB14 = 0;
    _ANSB9 = 0; // SYNC
    _TRISB9 = 1;

    /*
     * SPI connection to memory interface
     * SCLK -> D1/RP65
     * SDO -> D0/RP64
     * SDI -> D6/RP70
     * nCS -> D11/RPI75
     */
    _TRISD6 = 1; // SDI1
    _SDI1R = 70;
    _TRISD1 = 0; // SCK1
    _RP65R = _RPOUT_SCK1;
    _TRISD0 = 0; // SDO1
    _RP64R = _RPOUT_SDO1;
    _TRISD11 = 0; // nCS
    _LATD11 = 1;

    /*
     * SQI interface configuration to memory
     * nHOLD -> E1/RPI81
     * nRESET -> F0/RP96
     * nSQICS1 -> D5/RP69
     * nSQICS0 ->D4/RP68
     * nWP -> B15/RPI47
     * SI -> E0/RP80
     * SQICLK -> D7/RP70
     * SO -> RF1/RP97
     */
    _ANSE1 = 0; // nHOLD
    _TRISE1 = 0;
    _LATE1 = 1;
    _TRISF0 = 0; // nRESET
    _LATF0 = 1;
    _TRISD5 = 0; // nSQICS1
    _LATD5 = 1;
    _TRISD4 = 0; // nSQICS0
    _LATD4 = 1;
    _TRISB15 = 0; // nWP
    _LATB15 = 1;
    _ANSE0 = 0; // SI
    _TRISE0 = 0;
    _SDI3R = 80;
    _TRISD7 = 0; // SQICLK
    _RP71R = _RPOUT_SCK3;
    _ANSG6 = 0;
    _TRISF1 = 0; // SO
    _RP97R = _RPOUT_SDO3;

    /*
     * one wire interfaces (1W)
     * 0W1 -> E6/RPI86
     * OW2 -> B3/RPI35
     * OW3 -> E7/RP87
     * OW4 -> B1/RPI33
     */
    _ANSE6 = 0; // OW1
    _TRISE6 = 0;
    _LATE6 = 1;
    _ANSB3 = 0; // OW2
    _TRISB3 = 0;
    _LATB3 = 1;
    _ANSE7 = 0; // OW3
    _TRISE7 = 0;
    _LATE7 = 1;
    _ANSB1 = 0; // OW4
    _TRISB1 = 0;
    _LATB1 = 1;

    /*
     * analogue sensors
     * AS1 -> E4/AN28
     * AS2 -> B1/AN2
     * AS3 -> E3/AN27
     * AS4 -> B0/AN0
     */
    _ANSE4 = 1; // AS1
    _ANSB1 = 1; // AS2
    _ANSE3 = 1; // AS3
    _ANSB0 = 1; // AS4

    /*
     * USB connection
     * D+ -> G2
     * D- -> G3
     */
    // TODO

    /*
     * user interface
     * BTN -> D8 (user button for development)
     * ERR -> F3 (error light)
     */
    _TRISD8 = 1;
    _TRISF3 = 0;
    _LATF3 = 1;

    /*
     * power configuration
     * S_PS_SW -> B5
     * EN_VA   -> B8
     */
    _ANSB5 = 0;
    _TRISB5 = 0;
    _LATB5 = 0;
    _ANSB8 = 0;
    _TRISB8 = 0;
    _LATB8 = 0;
#endif
#ifdef __dsPIC33EP512MC806__
    /*
     *  I2C1 pins
     */

    _TRISG2 = 0;
    _TRISG3 = 0;
    _LATG2 = 1;
    _LATG3 = 0;
    __delay_ms(1);
    _LATG2 = 1;
    _LATG3 = 1;
    __delay_ms(1);
    _ODCG2 = 1; // configure I2C pins as open drain output
    _ODCG3 = 1; // configure I2C pins as open drain outputs

    _ANSC13 = 0;  // configure nINT1 as digital pin
    _TRISC13 = 1; // configure nINT1 as digital input pin
    _ANSC14 = 0;  // configure nRST1 as digital pin
    //_ODCC14 = 1; // nRST1 cannot be configured as open-drain pin -> set as input
    _TRISC14 = 1; // configure nRST1 as digital input pin
    _LATC14 = 1;

    /*
     * I2C2 pins
     */
    _TRISF4 = 0;
    _TRISF5 = 0;

    _LATF5 = 1;
    _LATF4 = 0;
    __delay_ms(1);
    _LATF4 = 1;
    _LATF5 = 1;
    __delay_ms(1);
    _ODCF4 = 1; // configure I2C pins as open drain output
    _ODCF5 = 1; // configure I2C pins as open drain output

    _ANSB15 = 0;  // configure nINT1 as digital pin
    _TRISB15 = 1; // configure nINT2 as digital input pin
    _ODCD8 = 1;
    _CNPUD8 = 1;
    _TRISD8 = 0; // configure nRST2 as digital output pin
    _LATD8 = 1;

    /*
     * Address selection
     */

    _ANSB8 = 0; // dedicated reset to address selector
    _TRISB8 = 0;
    _LATB8 = 1;

    /*
     * UART to PC logging
     */
    _ANSE5 = 0; // U2 RTS
    _TRISE5 = 0;
    _RP85R = _RPOUT_U2RTS;
    _ANSE6 = 0; // U2 CTS
    _U2CTSR = 86;
    _TRISE6 = 1;
    _ANSE7 = 0; // U2 TX
    _TRISE7 = 0;
    _RP87R = _RPOUT_U2TX;
    _ANSG7 = 0; // U2 RX
    _TRISG7 = 1;
    _U2RXR = 119;

    /*
     * UART to peripheral (not used currently)
     */
    //_TRISF3 = 0;            // U1 RTS
    //_RP99R = _RPOUT_U1RTS;
    _TRISF2 = 0; // U1 TX
    _LATF2 = 0;
    //_RP98R = _RPOUT_U1TX;
    //_TRISD9 = 1;            // U1 CTS
    //_U1CTSR = 73;
    _TRISD10 = 1; // U1 RX
    _U1RXR = 74;

    /*
     * UART to peripheral (not used currently)
     */

    /*_TRISF3 = 0;            // U3 TX
    _RP99R = _RPOUT_U3TX;
    _TRISD9 = 1;           // U3 RX
    _U3RXR = 73;*/
    _TRISF3 = 1;
    _TRISD9 = 0;
    _LATD9 = 0;

    /*
     * ECAN pin configuration
     */
    _ANSE4 = 0; // CAN1 TX
    _TRISE4 = 0;
    _RP84R = _RPOUT_C1TX;
    _ANSE3 = 0; // CAN1 RX
    _TRISE3 = 1;
    _C1RXR = 83;

    /*
     * SPI3 configuration
     */
    _ANSB9 = 0; // SDI3
    _TRISB9 = 1;
    _SDI1R = 41;
    _ANSG8 = 0; // SCK3
    _TRISG8 = 0;
    _RP120R = _RPOUT_SCK1;
    _ANSG6 = 0; // SDO1
    _TRISG6 = 0;
    _RP118R = _RPOUT_SDO1;
    _ANSB10 = 0; // CS
    _TRISB10 = 0;
    _LATB10 = 1;

    /*
     * SYNC configuration
     */
    _TRISD5 = 0;
    _RP69R = _RPOUT_OC1;
    _LATD5 = 0;

    /*
     * external pins: default config is digital input
     */
    _ANSE2 = 0; // E0 / CLK
    _TRISE2 = 1;
    _ANSE1 = 0; // E1
    _TRISE1 = 1;
    _ANSE0 = 0; // E2
    _TRISE0 = 1;
    _TRISF1 = 1; // E3

    /*
     * control pins: default config is digital input
     */
    _ANSD6 = 0; // D0
    _TRISD6 = 1;
    _ANSD7 = 0; // D1
    _TRISD7 = 1;
    _TRISF0 = 1; // D2

    // CAN ETH IO
    //_TRISD0 = 1;

    // LED (BLINKY, ERROR)
    _ANSB5 = 0;
    _ANSB4 = 0;

    // one wire interface (1W)
    _TRISD9 = 1;

    // reset sensors
    // CLEAR_BIT(config.rst_sensor_pin.tris_r, config.rst_sensor_pin.n);
    // SET_BIT(config.rst_sensor_pin.lat_r, config.rst_sensor_pin.n);
#endif
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
}

void dicio_send_ready_message(void *data)
{
    message_t m;

    message_init(&m, controller_address, 0, M_READY, 0, NULL, 0);
    message_send(&m);
}
