#include <xc.h>
#include "dicio.h"
#include <spi.h>
#include <string.h>
#include <stdio.h>
#include <uart.h>
#include <uart_logging.h>
#include "address.h"
#include <device_configuration.h>
#include <message.h>
#include "sensors.h"
#include "actuators.h"
#include "event_controller.h"

#define BRGVAL      ( (FCY / BAUDRATE) / 16 ) - 1
#define DICIO_BLINKY_PIN PIN_INIT(B, 5)
#define DICIO_ERROR_PIN PIN_INIT(B, 4)

#define SS1_PIN1 PIN_INIT(D, 2)
#define SS1_PIN2 PIN_INIT(D, 4)
#define SS1_PIN3 PIN_INIT(D, 3)

#define SS2_PIN1 PIN_INIT(B, 10)
#define SS2_PIN2 PIN_INIT(B, 11)

#define RST1_SENSOR_PIN PIN_INIT(C, 14)
#define RST2_SENSOR_PIN PIN_INIT(D, 8)

#define DICIO_I2C1_CONFIG {               \
    .i2c_address = 0x0,                   \
    .status = I2C_STATUS_MASTER_ON,       \
    .mw_sr_cb = dicio_i2c_mw_sr_callback, \
    .mr_sw_cb = dicio_i2c_mr_sw_callback, \
    .scl_pin = PIN_INIT(G, 2),            \
    .sda_pin = PIN_INIT(G, 3)}

#define DICIO_I2C2_CONFIG {\
    .i2c_address = 0x0,\
    .status = I2C_STATUS_MASTER_ON,\
    .mw_sr_cb = dicio_i2c_mw_sr_callback,\
    .mr_sw_cb = dicio_i2c_mr_sw_callback,\
    .scl_pin = PIN_INIT(F, 5),\
    .sda_pin = PIN_INIT(F, 4)}

#define DICIO_ONE_WIRE_CONFIG {\
    .pin = PIN_INIT(D, 9)}

static dicio_config_t config = {
    .blinky_pin = DICIO_BLINKY_PIN,
    .error_pin = DICIO_ERROR_PIN,
    .rst1_sensor_pin = RST1_SENSOR_PIN,
    .rst2_sensor_pin = RST2_SENSOR_PIN,
    .spi1_ss =
    {SS1_PIN1, SS1_PIN2, SS1_PIN3},
    .spi2_ss =
    {SS2_PIN1, SS2_PIN2},
    .i2c_config =
    {DICIO_I2C1_CONFIG, DICIO_I2C2_CONFIG},
    .one_wire_config = DICIO_ONE_WIRE_CONFIG,
    .output_frequency = 8000000,
};

int16_t n_connected_devices;

static void (*__sensor_timer_callback)(void) = dicio_dummy_callback;
static void (*__actuator_timer_callback)(void) = dicio_dummy_callback;

void dicio_dummy_callback(void) {

}

void dicio_set_sensor_callback(void (*cb)(void)) {
    __sensor_timer_callback = cb;
}

void dicio_set_actuator_callback(void (*cb)(void)) {
    __actuator_timer_callback = cb;
}

void init_sample_timer(void) {
    T9CONbits.TON = 0;
    T9CONbits.TCS = 0; // use internal instruction cycle as clock source
    T9CONbits.TGATE = 0; // disable gated timer
    T9CONbits.TCKPS = 0b11; // prescaler 1:256
    TMR9 = 0; // clear timer register
    PR9 = (uint16_t) ((FCY / 256) / DICIO_READ_FREQUENCY) - 1; // set period
    _T9IF = 0; // clear interrupt flag
    _T9IE = 1; // enable interrupt

    // start timer 
    T9CONbits.TON = 1;
}

void init_sample_detection(void) {
    // start timer 
    _INT0EP = 0; // interrupt on the positive edge

    _INT0IF = 0; // clear interrupt flag
    _INT0IE = 1; // enable interrupt
}

void __attribute__((__interrupt__, no_auto_psv)) _T9Interrupt(void) {
    __actuator_timer_callback();
    __sensor_timer_callback();
    _T9IF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _INT0Interrupt(void) {
    __actuator_timer_callback();
    __sensor_timer_callback();
    _INT0IF = 0;
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
    delay_ms(1);
    _LATD9 = 1;
    _LATD10 = 1;
    delay_ms(1);

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
    delay_ms(1);
    _LATF4 = 1;
    _LATF5 = 1;
    delay_ms(1);

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
    delay_ms(1);
    _LATG2 = 1;
    _LATG3 = 1;
    delay_ms(1);
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
    delay_ms(1);
    _LATF4 = 1;
    _LATF5 = 1;
    delay_ms(1);
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

void detect_can_devices(void) {
    message_t m;
    bool no_device_found = false;

    init_message(
            &m, 
            controller_address, 
            CAN_NO_REMOTE_FRAME,
            M_HELL0,
            0,
            NULL, 0, CAN_INTERFACE);

    send_message(&m);

    delay_ms(100);

    // check CAN status
    if (C1TR01CONbits.TXREQ0 == 1) {
        no_device_found = true;
    }
    if (C1TR01CONbits.TXREQ1 == 1) {
        no_device_found = true;
    }
    if (C1TR23CONbits.TXREQ2 == 1) {
        no_device_found = true;
    }
    if (C1TR23CONbits.TXREQ3 == 1) {
        no_device_found = true;
    }
    if (C1TR45CONbits.TXREQ4 == 1) {
        no_device_found = true;
    }
    if (C1TR45CONbits.TXREQ5 == 1) {
        no_device_found = true;
    }
    if (C1TR67CONbits.TXREQ6 == 1) {
        no_device_found = true;
    }
    if (C1TR67CONbits.TXREQ7 == 1) {
        no_device_found = true;
    }

    // disable CAN if needed
    if (no_device_found) {
        n_connected_devices = 0;

        deactivate_can_bus();

        UART_DEBUG_PRINT("No devices found.");

        //can_disable();
    } else {
        n_connected_devices = 1;

        UART_DEBUG_PRINT("At least one device found.");

    }
}

void init_trigger_generation(void) {
    // configure OC3 and OC4 to generate a pulse every 100ms
    OC3CON1bits.OCM = 0b000; // Disable Output Compare Module
    OC4CON1bits.OCM = 0b000; // Disable Output Compare Module

    // user peripheral clock as clock source
    OC3CON1bits.OCTSEL = 0b111;
    OC4CON1bits.OCTSEL = 0b111;

    // Write the frequency for the PWM pulse
    uint32_t period_value = FCY / 10;

    OC3RS = (uint16_t) (period_value - 1);
    OC4RS = (uint16_t) ((period_value - 1) >> 16);

    // Write the duty cycle for the PWM pulse
    period_value /= 2;
    OC3R = (uint16_t) (period_value - 1);
    OC4R = (uint16_t) ((period_value - 1) >> 16);

    // no sync or trigger source
    OC3CON2bits.SYNCSEL = 0b11111;
    OC4CON2bits.SYNCSEL = 0b11111;

    // continue operation in CPU idle mode
    OC3CON1bits.OCSIDL = 0;
    OC4CON1bits.OCSIDL = 0;

    // synchronised mode
    OC3CON2bits.OCTRIG = 0;
    OC4CON2bits.OCTRIG = 0;

    // output 
    OC3CON2bits.OCTRIS = 1; // since the OC3 pin is not used, the output should be tri-stated

    // enable cascade operation
    OC4CON2bits.OC32 = 1; // even module must be enabled first
    OC3CON2bits.OC32 = 1;

    _OC3IF = 0; // clear the OC3 interrupt flag
    _OC3IE = 0; // disable OC3 interrupt
    _OC4IF = 0; // clear the OC4 interrupt flag
    _OC4IE = 0; // disable OC4 interrupt

    // Select the Output Compare mode: EDGE-ALIGNED PWM MODE
    OC4CON1bits.OCM = 0b110;
    OC3CON1bits.OCM = 0b110;
}

void dicio_init(void) {
    set_error_loop_fn(i2c_process_queue);

    set_error_pin(&config.error_pin);


    UART_DEBUG_PRINT("Initialising pins...");

    dicio_init_pins();

    UART_DEBUG_PRINT("Initialising output clock reference.");
    dicio_init_clock_sync();
    dicio_start_clock_sync();


    UART_DEBUG_PRINT("Initialised blinky.");

    blinky_init(&config.blinky_pin, 0);

    UART_DEBUG_PRINT("Initialising I2C1.");
    i2c1_init(&config.i2c_config[0]);


    UART_DEBUG_PRINT("Initialising I2C2.");

    i2c2_init(&config.i2c_config[1]);

    UART_DEBUG_PRINT("Initialising ECAN.");

    can_init();


    UART_DEBUG_PRINT("Detecting if other devices connected using CAN.");

    detect_can_devices();

    /*if(controller_address == 0){
        _TRISD0 = 0;
        _RP64R = _RPOUT_OC2;
        init_trigger_generation();
    } else {
        _TRISD0 = 1;
        init_sample_detection();
    }*/

    UART_DEBUG_PRINT("Initialising device address.");

    get_device_address();
    UART_DEBUG_PRINT("Initialised device address to 0x%x.", controller_address);
}

void dicio_loop(void) {


}

void dicio_i2c_mw_sr_callback(i2c_message_t* m) {

}

void dicio_i2c_mr_sw_callback(i2c_message_t* m) {

}

void dicio_init_clock_sync(void) {
    // configure OC1 to generate clock signal at 8MHz
    OC1CON1bits.OCM = 0b000; // Disable Output Compare Module
    OC1R = (FCY / config.output_frequency) / 2 - 1; // Write the duty cycle for the PWM pulse
    OC1RS = (FCY / config.output_frequency) - 1; // frequency
    OC1CON1bits.OCSIDL = 0; // continue operation in CPU idle mode
    OC1CON1bits.OCTSEL = 0b111; // user peripheral clock as clock source
    OC1CON2bits.SYNCSEL = 0b11111; // no sync or trigger source

    _OC1IF = 0; // clear the OC1 interrupt flag
    _OC1IE = 0; // disable OC1 interrupt
}

void dicio_start_clock_sync(void) {
    OC1CON1bits.OCM = 0b110; // Select the Output Compare mode: EDGE-ALIGNED PWM MODE
}

void dicio_stop_clock_sync(void) {
    OC1CON1bits.OCM = 0b000; // Select the Output Compare mode: EDGE-ALIGNED PWM MODE
}

void __attribute__((__interrupt__, no_auto_psv)) _OC1Interrupt(void) {
    _OC1IF = 0;
}

void dicio_send_ready_message(void) {

}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {
    static unsigned int ctime = 0;
    int i;
    _LATB5 = ~_LATB5;

    for (i = 0; i < n_scheduled_events; i++) {
        schedule_list[i].trigger_time = ctime;
        schedule_list[i].trigger_time += schedule_list[i].period_s;
        push_queued_task(schedule_list[i].task);
    }

    _T1IF = 0;
}