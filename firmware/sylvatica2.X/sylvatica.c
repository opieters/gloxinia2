#include "sylvatica.h"
#include "address.h"
#include <can.h>
#include <sensor.h>

void sylvatica_init_pins(void){
    // I2C
    _ODCG2 = 1; // configure I2C pins as open drain output
    _ODCG3 = 1; // configure I2C pins as open drain output
    _TRISG2 = 0;
    _TRISG3 = 0;
    _ODCD0 = 1; // nINT
    _TRISD0 = 0; 
    _RD0 = 1;
    
    // SPI1 for PGA
    _TRISF5 = 0;            // SCK
    _RP101R = _RPOUT_SCK1;
    _ANSE2 = 0;             // SCK
    _TRISE2 = 0;
    _RP82R = _RPOUT_SCK1;
    _ANSE4 = 0;             // SDO
    _TRISE4 = 0;
    _RP84R = _RPOUT_SDO1;
    _ANSE3 = 0;             // CS1
    _TRISE3 = 0;
    _RE3 = 1;
    _TRISD11 = 0;           // CS2
    _RD11 = 1;
    _ANSE6 = 0;             // CS3
    _TRISE6 = 0;
    _RE6 = 1;
    _ANSE7 = 0;             // CS4
    _TRISE7 = 0;
    _RE7 = 1;
    _ANSB11 = 0;            // CS5
    _TRISB11 = 0;
    _RB11 = 1;
    _ANSB10 = 0;            // CS6
    _TRISB10 = 0;
    _RB10 = 1;
    _ANSB9 = 0;             // CS7
    _TRISB9 = 0;
    _RB9 = 1;
    _ANSB8 = 0;             // CS8
    _TRISB8 = 0;
    _RB8 = 1;
    
    // SPI2 for ADC
    _ANSE5 = 0;         // CS ADC
    _TRISE5 = 0;
    _RP85R = _RPOUT_OC4;
    _ANSG6 = 0;         // SCK
    _TRISG6 = 0;
    _ANSG7 = 0;         // SDI
    _TRISG7 = 1;
    _ANSG8 = 0;         // SDO
    _TRISG8 = 0;
    
    // ADC signals
    _ANSB14 = 0;        // INT ADC
    _TRISB14 = 0;
    _IC1R = 46;
    _ANSB15 = 0;        // RST ADC
    _TRISB15 = 0;
    _TRISF3 = 0;        // nCONV 
    _RP99R = _RPOUT_OC15;   
    
    // address readout
    _ANSE0 = 0;
    _TRISE0 = 1;
    _CNPUE0 = 1; // A0
    _ANSE1 = 0;
    _TRISE1 = 1;
    _CNPUE1 = 1; // A1
    _TRISF1 = 1;
    _CNPUF1 = 1; // A2
    _ANSB0 = 0;
    _TRISB0 = 1;
    _CNPUB0 = 1; // A3
    _ANSB1 = 0;
    _TRISB1 = 1;
    _CNPUB1 = 1; // A4
    _ANSB2 = 0;
    _TRISB2 = 1;
    _CNPUB2 = 1; // A5
    _ANSB3 = 0;
    _TRISB3 = 1;
    _CNPUB3 = 1; // A6
    _ANSB4 = 0;
    _TRISB4 = 1;
    _CNPUB4 = 1; // A7

    // UART configuration
    _ANSD6 = 0; // RXD
    _TRISD6 = 1;
    _U2RXR = 70;
    _TRISD5 = 0; // RTS
    _RP69R = _RPOUT_U2RTS;
    _TRISD4 = 1; // CTS
    _U2CTSR = 68;
    _TRISD3 = 0; // TXD
    _RP67R = _RPOUT_U2TX;
    
    // PGA calibration
    _ANSC14 = 0;  // REF enable
    _TRISC14 = 0;
    _LATC14 = 1;
    _TRISD2 = 0; // REF C1
    _LATD2 = 1;

    // blinky and error (shared)
    _ANSC13 = 0;
    _TRISC13 = 0;
    _LATC13 = 0;
    
    // 4V LDO control
    _ANSB5 = 0;
    _TRISB5 = 0;
    _LATB5 = 1;
}

i2c_config_t sylvatica_i2c1_config =  {
    .i2c_address = 0x0,
    .status = I2C_STATUS_PRIMARY_ON,
    .pw_sr_cb = I2C_NO_CALLBACK,
    .pr_sw_cb = I2C_NO_CALLBACK,
    .scl_pin = PIN_INIT(G, 2),
    .sda_pin = PIN_INIT(G, 3)};

i2c_config_t sylvatica_i2c2_config =  {
    .i2c_address = 0x0,
    .status = I2C_STATUS_PRIMARY_ON,
    .pw_sr_cb = I2C_NO_CALLBACK,
    .pr_sw_cb = I2C_NO_CALLBACK,
    .scl_pin = PIN_INIT(F, 5),
    .sda_pin = PIN_INIT(F, 4)};

void sylvatica_init(void){
    uint16_t i;
   
    sylvatica_init_pins();
    
    uart_init(500000);
    UART_DEBUG_PRINT("Configured UART.");
    
    can_init();
    UART_DEBUG_PRINT("Initialised ECAN.");

    i2c1_init(&sylvatica_i2c1_config);
    UART_DEBUG_PRINT("Initialised I2C1.");

    i2c2_init(&sylvatica_i2c2_config);
    UART_DEBUG_PRINT("Initialised I2C2.");

    sensors_init();
    UART_DEBUG_PRINT("Initialised sensor interface.");

    event_controller_init();
    UART_DEBUG_PRINT("Initialised event controller.");
    
    spi1_init();
    spi2_init();
    UART_DEBUG_PRINT("Initialised SPI.");
    
    for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
        gconfig.pga_config[i].status = PGA_STATUS_ON;
        pga_init(&gconfig.pga_config[i]);
    }
    UART_DEBUG_PRINT("Initialised PGAs.");
    
    sylvatica_filters_init();
    UART_DEBUG_PRINT("Initialised filters.");

    adc_init(&gconfig.adc_config);
    UART_DEBUG_PRINT("Initialised ADC.");

    task_schedule_t dicio_read_log;
    task_t dicio_read_log_task = {sylvatica_send_ready_message, NULL};
    schedule_init(&dicio_read_log, dicio_read_log_task, 10);
    // schedule_specific_event(&dicio_read_log, ID_READY_SCHEDULE);
    
}

void sylvatica_send_ready_message(void *data)
{
    message_t m;

    message_init(&m, controller_address, 0, M_READY, 0, NULL, 0);
    message_send(&m);
}