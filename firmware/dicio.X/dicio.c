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
#include "dicio_adc12_filters.h"
#include "spi1.h"
#include <stdbool.h>

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

sensor_interface_t sensor_interface1;
sensor_interface_t sensor_interface2;
sensor_interface_t sensor_interface3;
sensor_interface_t sensor_interface4;

sensor_interface_t* sensor_interfaces[N_SENSOR_INTERFACES] = 
{
    &sensor_interface1,
    &sensor_interface2,
    &sensor_interface3,
    &sensor_interface4,
};


const uint8_t n_sensor_interfaces = N_SENSOR_INTERFACES;

extern uint32_t sdcard_data_address;

// internal functions
void dicio_uart_rx_overflow(void* data);


void dicio_init_clock(void)
{
    // FRCDIV FRC/1; PLLPRE 2; DOZE 1:8; PLLPOST 1:2; DOZEN disabled; ROI disabled; 
    CLKDIV = 0x3000;
    // TUN Center frequency; 
    OSCTUN = 0x00;
    // ROON disabled; ROSEL FOSC; RODIV 0; ROSSLP disabled; 
    REFOCON = 0x00;
    // PLLDIV 62; 
    PLLFBD = 0x3E;
    // ENAPLL disabled; APLLPOST 1:256; FRCSEL FRC; SELACLK Auxiliary Oscillators; ASRCSEL Auxiliary Oscillator; AOSCMD AUX; APLLPRE 1:2; 
    ACLKCON3 = 0x2201;
    // APLLDIV 24; 
    ACLKDIV3 = 0x07;
    // AD1MD enabled; PWMMD enabled; T3MD enabled; T4MD enabled; T1MD enabled; U2MD enabled; T2MD enabled; U1MD enabled; QEI1MD enabled; SPI2MD enabled; SPI1MD enabled; C2MD enabled; C1MD enabled; DCIMD enabled; T5MD enabled; I2C1MD enabled; 
    PMD1 = 0x00;
    // OC5MD enabled; OC6MD enabled; OC7MD enabled; OC8MD enabled; OC1MD enabled; IC2MD enabled; OC2MD enabled; IC1MD enabled; OC3MD enabled; OC4MD enabled; IC6MD enabled; IC7MD enabled; IC5MD enabled; IC8MD enabled; IC4MD enabled; IC3MD enabled; 
    PMD2 = 0x00;
    // AD2MD enabled; PMPMD enabled; CMPMD enabled; U3MD enabled; QEI2MD enabled; RTCCMD enabled; T9MD enabled; T8MD enabled; CRCMD enabled; T7MD enabled; I2C2MD enabled; DAC1MD enabled; T6MD enabled; 
    PMD3 = 0x00;
    // U4MD enabled; REFOMD enabled; 
    PMD4 = 0x00;
    // OC9MD enabled; OC16MD enabled; IC10MD enabled; IC11MD enabled; IC12MD enabled; IC13MD enabled; IC14MD enabled; IC15MD enabled; IC16MD enabled; IC9MD enabled; OC14MD enabled; OC15MD enabled; OC12MD enabled; OC13MD enabled; OC10MD enabled; OC11MD enabled; 
    PMD5 = 0x00;
    // PWM2MD enabled; PWM1MD enabled; SPI4MD enabled; PWM4MD enabled; SPI3MD enabled; PWM3MD enabled; 
    PMD6 = 0x00;
    // DMA8MD enabled; DMA4MD enabled; DMA12MD enabled; DMA0MD enabled; 
    PMD7 = 0x00;
    // CF no clock failure; NOSC PRIPLL; LPOSCEN disabled; CLKLOCK unlocked; OSWEN Switch is Complete; IOLOCK not-active; 
    __builtin_write_OSCCONH((uint8_t) (0x03));
    __builtin_write_OSCCONL((uint8_t) (0x01));
    // Wait for Clock switch to occur
    while (OSCCONbits.OSWEN != 0);
    while (OSCCONbits.LOCK != 1);
}


void dicio_check_stored_config(void)
{
    uint8_t buffer[SDCARD_SECTOR_SIZE];
    SD_SPI_SectorRead(DICIO_MEASUREMENT_RUNNING_ADDRESS, buffer, 1);
    
    if(buffer[0])
    {
        UART_DEBUG_PRINT("Detected unfinished experiment.");
        
        // load node configurations and broadcast these to resp. nodes
        dicio_load_node_configs();
        
        // search final sector
        bool final_sector_found = false;
        const uint32_t sector_address_max = SD_SPI_GetSectorCount();
        
        uint32_t sector_address = sector_address_max - 1;
        uint32_t address_range_start = DICIO_DATA_START_ADDRESS;
        uint32_t address_range_stop = SD_SPI_GetSectorCount() - 1;
        uint32_t sector_step = address_range_stop - address_range_start + 1;
        uint32_t sector_empty_address, sector_written_address;
        
        // sanity check
        SD_SPI_SectorRead(address_range_start, buffer, 1);
        if(buffer_is_empty(buffer, ARRAY_LENGTH(buffer)))
        {
            sdcard_data_address = DICIO_DATA_START_ADDRESS;
        } else {
            SD_SPI_SectorRead(address_range_stop, buffer, 1);
            if(!buffer_is_empty(buffer, ARRAY_LENGTH(buffer)))
            {
                // sdcard is full
                sdcard_data_address = address_range_stop + 1;
            } else {
                while(!final_sector_found && (sector_step > 1))
                {
                    do {
                        sector_empty_address = sector_address;
                        sector_address = sector_address - sector_step;
                        sector_step = MAX(sector_step / 2, 1);
                        SD_SPI_SectorRead(sector_address, buffer, 1);
                    } while(buffer_is_empty(buffer, ARRAY_LENGTH(buffer)) && (sector_written_address < sector_empty_address));
                    
                    if((sector_written_address + 1) == sector_empty_address)
                    {
                        final_sector_found = true;
                    }
                    
                    do {
                        sector_written_address = sector_address;
                        sector_address = sector_address + sector_step;
                        sector_step = MAX(sector_step / 2, 1);
                        SD_SPI_SectorRead(sector_address, buffer, 1);
                    } while(!buffer_is_empty(buffer, ARRAY_LENGTH(buffer)) && (sector_written_address < sector_empty_address));
                    
                    if((sector_written_address + 1) == sector_empty_address)
                    {
                        final_sector_found = true;
                    }
                }
            }
            UART_DEBUG_PRINT("First empty sector: %08lx.", sector_empty_address);

            sdcard_data_address = sector_empty_address;
        }
            
        // start readout
        message_t m;
        message_init(&m,
                controller_address,
                true,
                M_SENSOR_START,
                NO_INTERFACE_ID,
                NO_SENSOR_ID,
                NULL,
                0);
        dicio_broadcast_measurement_start(&m);
        cmd_sensor_start(&m);
    } else {
        sdcard_data_address = DICIO_DATA_START_ADDRESS;
    }
}

void dicio_load_clock_config(void)
{
    uint8_t dicio_sector_buffer[SDCARD_SECTOR_SIZE];
    if(SD_SPI_SectorRead(DICIO_TIME_CONFIG_ADDRESS, dicio_sector_buffer, 1))
    {
        // readout values
        
        // TODO
        //dicio_sector_buffer
    }
}


void dicio_init(void)
{
    CORCONbits.VAR = 0;
    
    dicio_init_clock();
    
    dicio_init_pins();
    
    sensors_init();
    dicio_init_node_configurations();

    __builtin_enable_interrupts();

    can_init();
    uart_init(50000);

    UART_DEBUG_PRINT("Configured UART.");
    
    if(_RF2 == 1)
    {
        uart_connection_active = true;
        UART_DEBUG_PRINT("Detected UART host.");
    }

    i2c1_init(&dicio_i2c1_config);
    UART_DEBUG_PRINT("Initialised I2C1.");

    i2c2_init(&dicio_i2c2_config);
    UART_DEBUG_PRINT("Initialised I2C2.");

    event_controller_init();
    UART_DEBUG_PRINT("Initialised event controller.");
    
    clock_init();
    
    if(spi1_open())
    {
        UART_DEBUG_PRINT("Initialised SPI");
    } else 
    {
        UART_DEBUG_PRINT("SPI ERROR");
    }
    
    sensor_adc12_init_filters();
    sensor_adc12_set_callback(sensor_adc12_process_block0);
    UART_DEBUG_PRINT("Initialised ADC12.");
    
    if(SD_SPI_MediaInitialize() == true)
    {
        //dicio_read_sdconfig_data();
        //dicio_check_stored_config();
        // TODO: remove and reactivate above lines
        sdcard_data_address = DICIO_DATA_START_ADDRESS;
        UART_DEBUG_PRINT("Initialised SD card");
        
        sdcard_init_dma();
        //dicio_load_clock_config();
    }
    else
    {
        UART_DEBUG_PRINT("Unable to initialise SD card");
    }
    
    if(uart_connection_active)
    {
        // request time
        message_t m;
        message_init(&m,
                ADDRESS_GATEWAY,
                true,
                M_DICIO_TIME,
                NO_INTERFACE_ID,
                NO_SENSOR_ID,
                NULL,
                0);
        message_send(&m);
    }
    
    

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
    task_t dicio_read_log_task;
    task_init(&dicio_read_log_task, dicio_send_ready_message, NULL);
    schedule_init(&dicio_read_log, dicio_read_log_task, 10);
    //schedule_specific_event(&dicio_read_log, ID_READY_SCHEDULE);
    
    task_schedule_t dicio_uart_recovery;
    task_t dicio_uart_rx_overflow_check;
    task_init(&dicio_uart_rx_overflow_check, dicio_uart_rx_overflow, NULL);
    schedule_init(&dicio_uart_recovery, dicio_uart_rx_overflow_check, 10);
    //schedule_specific_event(&dicio_uart_recovery, ID_UART_OVERFLOW_SCHEDULE);
    
    // try to run ADC12
    uint8_t buffer1[4] = {SENSOR_TYPE_ADC12, sensor_adc12_gloxinia_register_general, 0, 9};
    sensor_set_config_from_buffer(2, 2, buffer1, 4);
    
    uint8_t buffer2[4] = {SENSOR_TYPE_ADC12, sensor_adc12_gloxinia_register_config, true};
    sensor_set_config_from_buffer(2, 2, buffer2, 3);
    
    //sensor_set_status( (0<<4) | 1, SENSOR_STATUS_ACTIVE);
    //sensor_adc12_activate(sensor_config);
    
    sdcard_data_address = DICIO_DATA_START_ADDRESS;
}

void dicio_uart_rx_overflow(void* data)
{
    if(U2STAbits.OERR)
    {
        UART_DEBUG_PRINT("UART2 ERROR 2");
        
        U2STAbits.OERR = 0;
        _U2EIF = 0;
    }
}

void dicio_init_node_configurations(void)
{
    for (int i = 0; i < ARRAY_LENGTH(node_configs); i++)
    {
        node_configs[i].node_id = ADDRESS_UNSET;
        node_configs[i].node_type = M_NODE_UNKNOWN;
        node_configs[i].sector_address = DICIO_NODE_CONFIG_START_ADDRESS + i * (1 + DICIO_MAX_N_INTERFACES*DICIO_MAX_N_SENSORS);
        node_configs[i].n_interfaces = 0;
        node_configs[i].stored_config = false;
    }
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
    _TRISD9 = 0;
    _TRISD10 = 0;
    _LATD9 = 0;
    _LATD10 = 1;
    __delay_ms(1);
    _LATD9 = 1;
    _LATD10 = 1;
    __delay_ms(1);
    _ODCD9 = 1; // configure I2C pins as open drain output
    _ODCD10 = 1; // configure I2C pins as open drain outputs

    _ANSB4 = 0;  // configure nINT1 as digital pin
    _TRISB4 = 1; // configure nINT1 as digital input pin

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

    _ANSE2 = 0;  // configure nINT2 as digital pin
    _TRISE2 = 1; // configure nINT2 as digital input pin

    /*
     * communication UART
     */
    _TRISF6 = 0; // U2 RTS
    _RP102R = _RPOUT_U2RTS;
    _ANSB12 = 0; // U2 CTS
    _U2CTSR = 44;
    _TRISB12 = 1;
    _TRISF2 = 1; // U2 RX
    _CNPDF2 = 1; // enable weak pull-down resistor. If the RX pin is not 
    // connected to TX of a computer, this will drive the pin low, which can be 
    // used for auto-detection of readout device
    _RP99R = _RPOUT_U2TX;
    _TRISF3 = 0; // U2 TX
    _U2RXR = 98;
    
    /*
     * debug UART
     */
    _ANSE5 = 0; // U1 RTS
    _TRISE5 = 0;
    _RP85R = _RPOUT_U1RTS;
    _ANSG7 = 0; // U1 CTS
    _U1CTSR = 119;
    _TRISG7 = 1;
    _ANSG6 = 0; // U1 TX
    _TRISG6 = 0;
    _RP118R = _RPOUT_U1TX;
    _ANSG8 = 0; // U1 RX
    _TRISG8 = 1;
    _U1RXR = 120;
    _CNPDG8 = 1; // enable weak pull-down resistor. If the RX pin is not 
    // connected to TX of a computer, this will drive the pin low, which can be 
    // used for auto-detection of readout device

    /*
     * ECAN pin configuration
     */
    _TRISD3 = 0; // CAN1 TX
    _RP67R = _RPOUT_C1TX;
    _ANSB13 = 0; // CAN1 RX
    _TRISB13 = 1;
    _C1RXR = 45;
    _ANSB14 = 0; // TERM 
    _TRISB14 = 0;
    _LATB14 = 1;
    _ANSB11 = 0; // CAN_C1
    _TRISB11 = 1;
    _ANSB10 = 0; // CAN_C2
    _TRISB10 = 1;
    
    /* 
     * one wire interfaces (1W)
     * 
     * These can also be used as analogue input interfaces. By default they are 
     * configured as digital inputs
     */
    _ANSE6 = 0;
    _TRISE6 = 1;
    _ANSB3 = 0;
    _TRISB3 = 1;
    _ANSE7 = 0;
    _TRISE7 = 1;
    _ANSB1 = 0;
    _TRISB1 = 1;
    _ANSB0 = 0;
    _TRISB0 = 1;

    /*
     * SPI configuration
     * SD card communication
     */
    _ANSD6 = 0;
    _TRISD6 = 1;// SDI
     _RD6 = 0;
    //_SDI3R = 70; 
    //_ANSD5 = 0;
    _TRISD5 = 1;
    _RD5 = 0;
    _SDI1R = 70;
    
    _TRISD1 = 0; // SCK
    _RP65R = _RPOUT_SCK1;
    
    _TRISD0 = 0; // SDO1
    _RP64R = _RPOUT_SDO1;
    
    _TRISD11 = 0;  // CS
    _RD11 = 1;
    
    /*
     * SPI configuration
     * Flash memory communication
     */
    _ANSE1 = 0; // nHOLD
    _TRISE1 = 0;
    _LATE1 = 1;
    _TRISF0 = 0; // nRESET
    _LATF0 = 0;
    __delay_ms(10);
    _LATF0 = 1;
    _TRISD5 = 0; // nSQICS1
    _LATD5 = 1;
    _TRISD4 = 0; // nSQICS0
    _LATD4 = 1;
    _TRISF1 = 0; // SO
    _RP97R = _RPOUT_SDO3;
    _ANSD7 = 0; // SQICLK
    _TRISD7 = 0;
    _RP71R = _RPOUT_SCK3;
    _ANSE0 = 0; // SI
    _TRISE0 = 0;  
    //_SDI3R = 80;
    _ANSB15 = 0; // nWP
    _TRISB15 = 0;
    _LATB15 = 1;

    /*
     * misc configuration
     */
    _ANSB9 = 0; // SYNC
    _TRISB9 = 1; 
    _LATD9 = 0;
    _TRISD8 = 1; // BTN
    _LATD8 = 0;
    _ANSB8 = 0;// EN_VA
    _TRISB8 = 0; 
    _LATB8 = 0;
    _ANSB5 = 0; // S_PS_SW
    _TRISB5 = 0;
    _LATB5 = 0;
    _TRISG3 = 0; // ERR (LED)
    _LATG3 = 0;

    
    /*
     * analogue readout pins
     */
    _ANSE4 = 1;
    _ANSB2 = 1;
    _ANSE3 = 1;
    _ANSB0 = 1;

    /*
     * secondary clock source for high precision RTCC
     */
    _ANSC14 = 0;
    _ANSC13 = 0;
    
#endif
   // __builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
}

void dicio_send_ready_message(void *data)
{
    message_t m;

    message_init(&m, controller_address, false, M_READY, NO_INTERFACE_ID, NO_SENSOR_ID, NULL, 0);
    message_send(&m);
}

