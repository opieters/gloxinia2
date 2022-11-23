#include "dicio.h"
#include <event_controller.h>
#include <message.h>
#include <address.h>
#include <uart.h>
#include <libpic30.h>
#include <can.h>
#include <i2c.h>
#include <sensor.h>

// TODO REMOVE DEBUG INCLUDES
#include <sensor_apds9306_065.h>

bool uart_connection_active = false;

i2c_config_t dicio_i2c1_config = {
    .i2c_address = 0x0,
    .status = I2C_STATUS_PRIMARY_ON,
    .pw_sr_cb = I2C_NO_CALLBACK,
    .pr_sw_cb = I2C_NO_CALLBACK,
    .scl_pin = PIN_INIT(G, 2),
    .sda_pin = PIN_INIT(G, 3)
};

i2c_config_t dicio_i2c2_config = {
    .i2c_address = 0x0,
    .status = I2C_STATUS_PRIMARY_ON,
    .pw_sr_cb = I2C_NO_CALLBACK,
    .pr_sw_cb = I2C_NO_CALLBACK,
    .scl_pin = PIN_INIT(F, 5),
    .sda_pin = PIN_INIT(F, 4)
};

void dicio_init(void) {
    
    dicio_init_pins();
    
    uart_init(500000);
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

    uart_connection_active = true;
    message_t m;
    uint8_t data[] = {
        SENSOR_TYPE_APDS9306_065, 
        0,
        I2C_ADDRESS_SENSOR_APDS9306_065, 
        I2C2_BUS,
        SENSOR_APDS9306_065_ALS_MEAS_RATE_100MS,
        SENSOR_APDS9306_065_ALS_RESOLUTION_20BIT,
        SENSOR_APDS9306_065_ALS_GAIN_1};
    
    message_init(&m, controller_address, MESSAGE_NO_REQUEST, M_SENSOR_CONFIG, 0,
        data, ARRAY_LENGTH(data));
    message_process(&m);
    
    //task_schedule_t dicio_read_log = {{dicio_send_ready_message, NULL}, 1, 0};
    //schedule_specific_event(&dicio_read_log, ID_READY_SCHEDULE);
    
    //UART_DEBUG_PRINT("Detecting if other devices connected using CAN.");
    //can_detect_devices();

    /*if(controller_address == 0){
        _TRISD0 = 0;
        _RP64R = _RPOUT_OC2;
        init_trigger_generation();
    } else {
        _TRISD0 = 1;
        init_sample_detection();
    }*/

    //UART_DEBUG_PRINT("Initialising device address.");
    //address_get();
    //UART_DEBUG_PRINT("Initialised device address to 0x%x.", controller_address);

    __delay_ms(100);
    
    task_schedule_t dicio_read_log;
    task_t dicio_read_log_task = {dicio_send_ready_message, NULL};
    schedule_init(&dicio_read_log, dicio_read_log_task, 10);
    schedule_specific_event(&dicio_read_log, ID_READY_SCHEDULE);
}

void dicio_init_pins(void) {
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

    _ODCD9 = 1; // configure I2C pins as open drain output
    _ODCD10 = 1; // configure I2C pins as open drain outputs

    _ANSE2 = 0; // configure nINT2 as digital pin
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

    _ANSB4 = 0; // configure nINT2 as digital pin
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
    _ANSB14 = 0; // TERM
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


    _ANSC13 = 0; // configure nINT1 as digital pin
    _TRISC13 = 1; // configure nINT1 as digital input pin    
    _ANSC14 = 0; // configure nRST1 as digital pin
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

    _ANSB15 = 0; // configure nINT1 as digital pin
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
     * SPI1 configuration
     */
    _TRISD11 = 1; // SDI1 
    _SDI1R = 75;
    _TRISF6 = 0; // SCK1
    _RP102R = _RPOUT_SCK1;
    _TRISD1 = 0; // SDO1
    _RP65R = _RPOUT_SDO1;

    /*
     * SPI3 configuration
     */
    _ANSB9 = 0; // SDI3
    _TRISB9 = 1;
    _SDI3R = 41;
    _ANSG8 = 0; // SCK3
    _TRISG8 = 0;
    _RP120R = _RPOUT_SCK3;
    _ANSG6 = 0; // SCK3
    _TRISG6 = 0;
    _RP118R = _RPOUT_SDO3;

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
    _TRISD0 = 1;

    // LED (BLINKY, ERROR)
    _ANSB5 = 0;
    _ANSB4 = 0;

    // one wire interface (1W)
    _TRISD9 = 1;

    // reset sensors
    //CLEAR_BIT(config.rst_sensor_pin.tris_r, config.rst_sensor_pin.n);
    //SET_BIT(config.rst_sensor_pin.lat_r, config.rst_sensor_pin.n);
#endif
}

void dicio_send_ready_message(void* data) {
    message_t m;
    
    message_init(&m, controller_address, 0, M_READY, 0, NULL, 0);
    message_send(&m);
}