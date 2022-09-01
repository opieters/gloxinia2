#include <xc.h>
#include "dicio.h"
#include <spi.h>
#include <string.h>
#include <stdio.h>
#include <uart.h>
#include "address.h"
#include <device_configuration.h>
#include <can.h>
#include "sensors.h"
#include "actuators.h"

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
    .spi1_ss = {SS1_PIN1, SS1_PIN2, SS1_PIN3},
    .spi2_ss = {SS2_PIN1, SS2_PIN2},
    .i2c_config = {DICIO_I2C1_CONFIG, DICIO_I2C2_CONFIG},
    .one_wire_config = DICIO_ONE_WIRE_CONFIG,
    .output_frequency = 8000000,
};

int16_t n_connected_can_devices;

static void (*__sensor_timer_callback)(void) = dicio_dummy_callback;
static void (*__actuator_timer_callback)(void) = dicio_dummy_callback;

void dicio_dummy_callback(void){
    
}

void dicio_set_sensor_callback(void (*cb)(void)){
    __sensor_timer_callback = cb;
}

void dicio_set_actuator_callback(void (*cb)(void)){
    __actuator_timer_callback = cb;
}

void init_sample_timer(void){
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

void init_sample_detection(void){
    // start timer 
    _INT0EP = 0; // interrupt on the positive edge
    
    _INT0IF = 0; // clear interrupt flag
    _INT0IE = 1; // enable interrupt
}

void __attribute__ ( (__interrupt__, no_auto_psv) ) _T9Interrupt( void ){
    __actuator_timer_callback();
    __sensor_timer_callback();
    _T9IF = 0;
}

void __attribute__ ( (__interrupt__, no_auto_psv) ) _INT0Interrupt( void ){
    __actuator_timer_callback();
    __sensor_timer_callback();
    _INT0IF = 0;
}

void dicio_init_pins(void){
    /*****************************
     * I2C2 pin configuration
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
    
    _ODCD9 = 1;  // configure I2C pins as open drain output
    _ODCD10 = 1; // configure I2C pins as open drain outputs
    
    _ANSB4 = 0; // configure nINT1 as digital pin
    _TRISB4 = 1; // configure nINT1 as digital input pin    
    
    /*****************************
     * I2C1 pin configuration
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
    
    _ANSE2 = 0; // configure nINT1 as digital pin
    _TRISE2 = 1; // configure nINT2 as digital input pin 
    

    /**
     * UART connection to debug port
     * TX -> RE7 / PR87
     * RX -> RG8 / RP120
     * RTS -> RE5 / RP85
     * CTS -> RG7 / RPI119
     */
    
    _ANSE5 = 0;
    _TRISE5 = 0;            // U1 RTS
    _RP85R = _RPOUT_U1RTS;
    
    _ANSE7 = 0;
    _TRISE7 = 0;            // U1 TX
    _LATE7 = 0;
    _RP87R = _RPOUT_U1TX;
    
    _ANSG7 = 0;
    _TRISG7 = 1;            // U1 CTS
    _U1CTSR = 119;
    
    _ANSG8 = 0;
    _TRISG8 = 1;           // U1 RX
    _U1RXR = 120;
    
    
    /*
     * ECAN pin configuration
     * CAN TX -> D3/RP67
     * CAN RX -> B13/RPI46
     **/        
    _TRISD3 = 0;         // CAN1 TX
    _RP67R = _RPOUT_C1TX;
    _ANSB13 = 0;          // CAN1 RX
    _TRISB13 = 1;
    _C1RXR = 46;
    
    /* 
     * SPI connection to memory interface
     * SCLK -> D1/RP65
     * SDO -> D0/RP64
     * SDI -> D6/RP70
     * nCS -> D11/RPI75
     */
    _TRISD6 = 1;           // SDI1 
    _SDI1R = 70;
    _TRISD1 = 0;           // SCK1
    _RP65R = _RPOUT_SCK1;
    _TRISD0 = 0;           // SDO1
    _RP64R = _RPOUT_SDO1;
    _TRISD11 = 0;          // nCS
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
    _ANSE1 = 0;  // nHOLD
    _TRISE1 = 0;
    _LATE1 = 1;
    _TRISF0 = 0; // nRESET 
    _LATF0 = 1;
    _TRISD5 = 0;  // nSQICS1
    _LATD5 = 1;
    _TRISD4 = 0; // nSQICS0
    _LATD4 = 1;
    _TRISB15 = 0; // nWP
    _LATB15 = 1;
    _ANSE0 = 0; // SI
    _TRISE0 = 0;
    _SDI3R = 80;
    _RP120R = _RPOUT_SCK3;
    _ANSG6 = 0; // SCK3
    _TRISG6 = 0;
    _RP118R = _RPOUT_SDO3;
    
    // SYNC configuration
    _TRISD5 = 0;
    _RP69R = _RPOUT_OC1;
    _LATD5 = 0;
    
    // external pins: default config is digital input
    _ANSE2 = 0;   // E0 / CLK
    _TRISE2 = 1;
    _ANSE1 = 0;   // E1
    _TRISE1 = 1;
    _ANSE0 = 0;   // E2
    _TRISE0 = 1;
    _TRISF1 = 1;  // E3
    
    // control pins: default config is digital input 
    _ANSD6 = 0;   // D0
    _TRISD6 = 1;
    _ANSD7 = 0;   // D1
    _TRISD7 = 1;
    _TRISF0 = 1;  // D2
    
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
}

void detect_can_devices(void){
    can_message_t m;
    bool no_device_found = false;
    
    can_init_message(&m, controller_address, CAN_NO_REMOTE_FRAME, 
            CAN_EXTENDED_FRAME, CAN_HEADER(CAN_INFO_MSG_HELLO, 0), NULL, 0);
    
    can_send_message_any_ch(&m);
    
    delay_ms(100);
    
    // check CAN status
    if(C1TR01CONbits.TXREQ0 == 1){
        no_device_found = true;
    }
    if(C1TR01CONbits.TXREQ1 == 1){
        no_device_found = true;
    }
    if(C1TR23CONbits.TXREQ2 == 1){
        no_device_found = true;
    }
    if(C1TR23CONbits.TXREQ3 == 1){
        no_device_found = true;
    }
    if(C1TR45CONbits.TXREQ4 == 1){
        no_device_found = true;
    }
    if(C1TR45CONbits.TXREQ5 == 1){
        no_device_found = true;
    }
    if(C1TR67CONbits.TXREQ6 == 1){
        no_device_found = true;
    }
    if(C1TR67CONbits.TXREQ7 == 1){
        no_device_found = true;
    }
    
    // disable CAN if needed
    if(no_device_found){
        n_connected_can_devices = 0;

        deactivate_can_bus();
        
        #ifdef ENABLE_DEBUG        
        sprintf(print_buffer, "No CAN devices found.");
        uart_print(print_buffer, strlen(print_buffer));
        #endif

        can_disable();
    } else {
        n_connected_can_devices = 1;
        
        #ifdef ENABLE_DEBUG        
        sprintf(print_buffer, "At least one CAN device found.");
        uart_print(print_buffer, strlen(print_buffer));
        #endif
    }
}

void init_trigger_generation(void){
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
    
    _OC3IF = 0;              // clear the OC3 interrupt flag
    _OC3IE = 0;              // disable OC3 interrupt
    _OC4IF = 0;              // clear the OC4 interrupt flag
    _OC4IE = 0;              // disable OC4 interrupt
    
    // Select the Output Compare mode: EDGE-ALIGNED PWM MODE
    OC4CON1bits.OCM = 0b110; 
    OC3CON1bits.OCM = 0b110;
}

void dicio_init(void){    
    set_error_loop_fn(i2c_process_queue);
    
    set_error_pin(&config.error_pin);
    
    #ifdef ENABLE_DEBUG        
        sprintf(print_buffer, "Initialising pins...");
        uart_print(print_buffer, strlen(print_buffer));
    #endif
    dicio_init_pins();   
    
    #ifdef ENABLE_DEBUG        
        sprintf(print_buffer, "Initialising output clock reference.");
        uart_print(print_buffer, strlen(print_buffer));
    #endif
    dicio_init_clock_sync();
    dicio_start_clock_sync();

    #ifdef ENABLE_DEBUG        
        sprintf(print_buffer, "Initialised blinky.");
        uart_print(print_buffer, strlen(print_buffer));
    #endif
    blinky_init(&config.blinky_pin, 1);
    
    #ifdef ENABLE_DEBUG        
        sprintf(print_buffer, "Initialising I2C1.");
        uart_print(print_buffer, strlen(print_buffer));
    #endif
    i2c1_init(&config.i2c_config[0]);
    
    #ifdef ENABLE_DEBUG        
        sprintf(print_buffer, "Initialising I2C2.");
        uart_print(print_buffer, strlen(print_buffer));
    #endif
    i2c2_init(&config.i2c_config[1]);
    
    #ifdef ENABLE_DEBUG        
        sprintf(print_buffer, "Initialising device address.");
        uart_print(print_buffer, strlen(print_buffer));
    #endif
    get_device_address();
    sprintf(print_buffer, "Initialised device address to 0x%x.", controller_address);
    uart_print(print_buffer, strlen(print_buffer));
    
    #ifdef ENABLE_DEBUG        
        sprintf(print_buffer, "Initialising ECAN.");
        uart_print(print_buffer, strlen(print_buffer));
    #endif
    can_init();
    
    #ifdef ENABLE_DEBUG        
        sprintf(print_buffer, "Detecting if other devices connected using CAN.");
        uart_print(print_buffer, strlen(print_buffer));
    #endif
    detect_can_devices();
    
    if(controller_address == 0){
        _TRISD0 = 0;
        _RP64R = _RPOUT_OC2;
        init_trigger_generation();
    } else {
        _TRISD0 = 1;
        init_sample_detection();
    }
    
    // reset sensors
    _TRISC14 = 0;
    _LATC14 = 0;
    delay_ms(10);
    _LATC14 = 1;
    _TRISC14 = 1;
    
}

void dicio_loop(void){
    
    #ifdef ENABLE_DEBUG        
        sprintf(print_buffer, "Starting sample timer.");
        uart_print(print_buffer, strlen(print_buffer));
    #endif
    if(controller_address == 0){
        init_sample_timer();
    }
    
    #ifdef ENABLE_DEBUG        
        sprintf(print_buffer, "Starting main loop.");
        uart_print(print_buffer, strlen(print_buffer));
    #endif
    
    while(1){
        while(1){
            i2c_process_queue();
            
            sensors_error_recover();
            actuators_error_recover();
        }
    }
}

void dicio_i2c_mw_sr_callback(i2c_message_t* m){
    
}

void dicio_i2c_mr_sw_callback(i2c_message_t* m){

}

void dicio_init_clock_sync(void){
    // configure OC1 to generate clock signal at 8MHz
    OC1CON1bits.OCM = 0b000; // Disable Output Compare Module
    OC1R = (FCY / config.output_frequency) / 2 - 1;   // Write the duty cycle for the PWM pulse
    OC1RS = (FCY / config.output_frequency) - 1;   // frequency
    OC1CON1bits.OCSIDL = 0; // continue operation in CPU idle mode
    OC1CON1bits.OCTSEL = 0b111; // user peripheral clock as clock source
    OC1CON2bits.SYNCSEL = 0b11111; // no sync or trigger source
    
    _OC1IF = 0;              // clear the OC1 interrupt flag
    _OC1IE = 0;              // disable OC1 interrupt
}

void dicio_start_clock_sync(void){
    OC1CON1bits.OCM = 0b110; // Select the Output Compare mode: EDGE-ALIGNED PWM MODE
}

void dicio_stop_clock_sync(void){
    OC1CON1bits.OCM = 0b000; // Select the Output Compare mode: EDGE-ALIGNED PWM MODE
}

void __attribute__((__interrupt__, no_auto_psv)) _OC1Interrupt(void) {    
    _OC1IF = 0;
}

void dicio_send_message(serial_cmd_t cmd, uint16_t can_ext_id, 
        uint8_t* data, uint8_t data_length){
    uart_message_t m_uart;
    can_message_t m_can;
    
    if(controller_address == 0){
        uart_init_message(&m_uart, cmd, controller_address,
                can_ext_id, data, 
                data_length);
        
        uart_queue_message(&m_uart);
        uart_await_tx(&m_uart);
    } else {
        can_init_message(&m_can, controller_address, CAN_NO_REMOTE_FRAME, 
                CAN_EXTENDED_FRAME, 
                can_ext_id, data, 
                data_length);
        
        can_send_message_any_ch(&m_can);
    }
}

void dicio_send_ready_message(void){
    // transmit message over UART that this module is ready to start
    uart_message_t m_uart;
    
    if(controller_address == 0){
        uart_init_message(&m_uart, SERIAL_START_INIT, controller_address,
                CAN_HEADER(NO_CAN_CMD, 0), NULL, 
                0);
        
        uart_queue_message(&m_uart);
        uart_await_tx(&m_uart);
    }
}
