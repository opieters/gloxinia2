#include "sylvatica.h"
#include "address.h"
#include "address.h"
#include <can.h>
#include <sensor.h>
#include <spi.h>
#include <pga.h>
#include <dsp.h>
#include <sensor_adc16.h>
#include <fir_common.h>
#include <libpic30.h>
#include <adc16.h>
//#include <event_controller.h>


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

pga_config_t pga_config[N_SENSOR_INTERFACES] = {
    { .cs_pin = PIN_INIT(C, 13) },
    { .cs_pin = PIN_INIT(C, 14) },
    { .cs_pin = PIN_INIT(D, 0) },
    { .cs_pin = PIN_INIT(D, 11) },
    { .cs_pin = PIN_INIT(G, 2) },
    { .cs_pin = PIN_INIT(G, 3) },
    { .cs_pin = PIN_INIT(F, 6) },
    { .cs_pin = PIN_INIT(F, 2) },
};

bool uart_connection_active = false;

adc16_config_t adc16_config = {
    .channel_select = ADC16_CHANNEL_SELECT_MODE_AUTO,
    .conversion_clock_source = ADC16_CONVERSION_CLOCK_SOURCE_INTERNAL,
    .trigger_select = ADC16_TRIGGER_SELECT_MANUAL,
    .auto_trigger_rate = ADC16_SAMPLE_RATE_AUTO_TRIGGER_500KSPS,
    .pin10_polarity = ADC16_PIN10_POLARITY_SELECT_ACTIVE_LOW,
    .pin10_output = ADC16_PIN10_OUTPUT_EOC,
    .pin10_io = ADC16_PIN10_IO_SELECT_EOC_INT,
    .auto_nap = ADC16_AUTO_NAP_POWERDOWN_DISABLE,
    .nap_powerdown = ADC16_NAP_POWERDOWN_DISABLE,
    .deep_powerdown = ADC16_DEEP_POWERDOWN_DISABLE,
    .tag_output = ADC16_TAG_OUTPUT_DISABLE,
    .sw_reset = ADC16_NORMAL_OPERATION,
    .channel = ADC16_CH0,
    .sample_frequency = SYLVATICA_ADC16_SAMPLE_FREQUENCY,
    .adc16_buffer_size = SYLVATICA_ADC16_BUFFER_LENGTH,
    .rx_callback = sylvatic_adc16_callback,
    .spi_module = SPI_MODULE_SELECTOR_3,
    .rst_pin = PIN_INIT(D, 3),
    .cs_pin = PIN_INIT(D, 6),
    .conv_pin = PIN_INIT(D, 1)}
;

sensor_interface_t sensor_interface1;
sensor_interface_t sensor_interface2;
sensor_interface_t sensor_interface3;
sensor_interface_t sensor_interface4;
sensor_interface_t sensor_interface5;
sensor_interface_t sensor_interface6;
sensor_interface_t sensor_interface7;
sensor_interface_t sensor_interface8;

sensor_interface_t* sensor_interfaces[N_SENSOR_INTERFACES] = 
{
    &sensor_interface1,
    &sensor_interface2,
    &sensor_interface3,
    &sensor_interface4,
    &sensor_interface5,
    &sensor_interface6,
    &sensor_interface7,
    &sensor_interface8,
};

task_t pin_interrupt_tasks[N_SENSOR_INTERFACES];


const uint8_t n_sensor_interfaces = N_SENSOR_INTERFACES;


void sensor_adc12_process_block0(void);
void sensor_adc12_init_filters(void);

void sylvatica_init_pins(void){
    // I2C 1
    _ODCD9 = 1; // SDA1
    _TRISD9 = 0;
    _CNPUD9 = 1;
    _ODCD10 = 1; // SCL1
    _TRISD10 = 0;
    _CNPUD10 = 1;
    _ODCD8 = 1; // nINT
    _TRISD8 = 0; 
    _RD8 = 1;
    
    // I2C 2
    _TRISF4 = 0; // SDA2
    _ODCF4 = 1;
    _CNPUF4 = 1;
    _ODCF5 = 1; // SCL2
    _TRISF5 = 0;
    _CNPUF5 = 1;
    _ANSB10 = 0; // nINT2
    _TRISB10 = 0;
    _CNPUB10 = 1;
    _RB10 = 1;
    
    // SPI1 for PGA
    _ANSE0 = 0; // SCK
    _TRISE0 = 0;
    _RP80R = _RPOUT_SCK1;
    _TRISF1 = 0; // SDO
    _RP97R = _RPOUT_SDO1;
    _ANSC13 = 0; // nCS1
    _TRISC13 = 0;
    _RC13 = 1;
    _ANSC14 = 0; // nCS2
    _TRISC14 = 0;
    _RC14 = 1;
    _TRISD0 = 0; // nCS3
    _RD0 = 1;
    _TRISD11 = 0; // nCS4
    _RD11 = 1;
    _TRISG2 = 0; // nCS5
    _RG2 = 1;
    _TRISG3 = 0; // nCS6
    _RG3 = 1;
    _TRISF6 = 0; // nCS7
    _RF6 = 1;
    _TRISF2 = 0; // nCS8
    _RF2 = 1;
    
    // SPI & pins for communication with ADC
    _TRISD5 = 0; // SCK
    _RP69R = _RPOUT_SCK3;
    _ANSD7 = 0; // SDO
    _TRISD7 = 0;
    _RP71R = _RPOUT_SDO3;
    _TRISD2 = 1; // SDI
    _SDI3R = 66;
    _ANSD6 = 0; // nCS_ADC
    _TRISD6 = 0;
    _RD6 = 1;
    //_RP70R = _RPOUT_OC4;
    _TRISD4 = 1; // nINT_ADC
    _IC1R = 68;
    _TRISD3 = 0; // nRST_ADC
    _RD3 = 1;
    _TRISD1 = 0; // nCONV
    _RP65R = _RPOUT_OC15;
    
    // one-wire readout
    _ANSB9 = 0; // OW1
    _ANSB8 = 0; // OW2
    _ANSB5 = 0; // OW3
    _ANSE4 = 0; // OW4
    _ANSB14 = 0; // OW5
    _ANSB13 = 0; // OW6
    _ANSB4 = 0; // OW7
    _ANSB3 = 0; // OW8
    
    // CAN configuration
    _ANSG8 = 0; // CAN1 TX
    _TRISG8 = 0;
    _RP120R = _RPOUT_C1TX;
    _ANSG7 = 0; // CAN1 RX
    _TRISG7 = 1;
    _C1RXR = 119;
    _ANSB11 = 0; // TERM 
    _TRISB11 = 0;
    _RB11 = 1;
    _ANSG9 = 0; // CAN_C1
    _TRISG9 = 1;
    _ANSB2 = 0; // CAN_C2
    _TRISB2 = 1;
    
    // UART configuration
    _ANSE6 = 0; // RXD
    _TRISE6 = 1;
    _U2RXR = 86;
    _CNPDE6 = 1; // enable weak pull-down resistor. If the RX pin is not 
    // connected to TX of a computer, this will drive the pin low, which can be 
    // used for auto-detection of readout device
    _ANSG6 = 0; // RTS
    _TRISG6 = 0; 
    _RP118R = _RPOUT_U2RTS;
    _ANSE7 = 0;  // CTS
    _TRISE7 = 1;
    _U2CTSR = 87;
    _ANSE5 = 0; // TXD
    _TRISE5 = 0;
    _RP85R = _RPOUT_U2TX;
   
    // power control
    _ANSB0 = 0; //AS_PS_SW2
    _TRISB0 = 0; 
    _RB0 = 0;
    _ANSB1 = 0; // S_PS_SW2
    _TRISB1 = 0;
    _RB1 = 0;
    _ANSE2 = 0; // AS_PS_SW
    _TRISE2 = 0;
    _RE2 = 0;
    _TRISF0 = 0; // S_OS_SW
    _RF0 = 0;
    _ANSB15 = 0; // EN_VDDA
    _TRISB15 = 0;
    _RB15 = 1;
    _ANSE1 = 0; // EN_VDDA2
    _TRISE1 = 0;
    _RE1 = 1;
    _ANSB12 = 0; // REF_C2
    _TRISB12 = 0;
    _RB12 = 1;
    _TRISF3 = 0; // REF_C1
    _RF3 = 0;

    // blinky and error (shared)
    _ANSE3 = 0;
    _TRISE3 = 0;
    _RE3 = 0;
}

void sylatica_clock_init(void)
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


void sylvatica_interface_init(void)
{
    for(int i = 0; i < N_SENSOR_INTERFACES; i++)
    {
        sensor_interfaces[i]->interface_id = i;
        for(int j = 0; j < SENSOR_INTERFACE_MAX_SENSORS; j++)
        {
            sensor_interfaces[i]->gsensor_config[j].sensor_id = j;
            sensor_interfaces[i]->gsensor_config[j].interface = sensor_interfaces[i];
            //sensor_interfaces[i]->gsensor_config[j].measure = NULL;
            sensor_interfaces[i]->gsensor_config[j].sensor_type = SENSOR_NOT_SET;
            sensor_interfaces[i]->gsensor_config[j].status = SENSOR_STATUS_INACTIVE;
        }
    }
    
    pin_t pin1 = PIN_INIT(D, 8);
    pin_t pin2 = PIN_INIT(B, 10);
    sensor_interface1.int_pin = pin1;
    sensor_interface2.int_pin = pin1;
    sensor_interface3.int_pin = pin1;
    sensor_interface4.int_pin = pin1;
    sensor_interface5.int_pin = pin2;
    sensor_interface6.int_pin = pin2;
    sensor_interface7.int_pin = pin2;
    sensor_interface8.int_pin = pin2;
    
    sylvatica_sensor_interface_supply(0, INTERFACE_SUPPY_ANALOGUE);
    sylvatica_sensor_interface_supply(4, INTERFACE_SUPPY_ANALOGUE);
    
    for(int i = 0; i < N_SENSOR_INTERFACES; i++)
    {
        pin_interrupt_tasks[i].cb = NULL;
    }
}

void sylvatica_sensor_interface_supply(uint8_t interface_id, interface_supply_t vconfig)
{
    // apply new voltage supply
    if(interface_id < 4){
        switch(vconfig)
        {
            case INTERFACE_SUPPY_ANALOGUE:
                _RF0 = 0;
                __delay_us(10);
                _RE2 = 1;
                break;
            case INTERFACE_SUPPLY_DIGITAL:
                _RE2 = 0;
                __delay_us(10);
                _RF0 = 1;
                break;
            default:
                return;
        }
    } else {
        switch(vconfig)
        {
            case INTERFACE_SUPPY_ANALOGUE:
                _RB1 = 0;
                __delay_us(10);
                _RB0 = 1;
                break;
            case INTERFACE_SUPPLY_DIGITAL:
                _RB0 = 0;
                __delay_us(10);
                _RB1 = 1;
                break;
            default:
                return;
        }
    }
    int if_start, if_end;
    if(interface_id < 4){
        if_start = 0; 
        if_end = 4;
    } else {
        if_start = 4;
        if_end = N_SENSOR_INTERFACES;
    }
    
    // update status and notify host
    for(int i = if_start; i < if_end; i++){
        sensor_interface_set_supply(i, vconfig);
    };
}

void sylvatica_init_sensors(void)
{
    // ADS1219
    uint8_t buffer0_ads1219[4] = {SENSOR_TYPE_ADS1219, sensor_ads1219_gloxinia_register_general, 0, 9};
    sensor_set_config_from_buffer(0, 0, buffer0_ads1219, ARRAY_LENGTH(buffer0_ads1219));
    sensor_set_config_from_buffer(1, 0, buffer0_ads1219, ARRAY_LENGTH(buffer0_ads1219));
    
    // OW pins are used as data ready input signal
    // TODO: remove hard-coded values and upgrade to software-set system
    _CNPUB9 = 1;
    _CNPUB8 = 1;
    
    uint8_t buffer1_ads1219[8] = {
        SENSOR_TYPE_ADS1219, 
        sensor_ads1219_gloxinia_register_config, 
        0x46, 
        0xff, 
        sensor_ads1219_gain_1, 
        sensor_ads1219_90_sps, 
        sensor_ads1219_conversion_mode_single_shot, sensor_ads1219_vref_external};
    buffer1_ads1219[2] = 0x4A;
    sensor_set_config_from_buffer(0, 0, buffer1_ads1219, ARRAY_LENGTH(buffer1_ads1219));
    buffer1_ads1219[2] = 0x49;
    sensor_set_config_from_buffer(1, 0, buffer1_ads1219, ARRAY_LENGTH(buffer1_ads1219));
    
    
        // manually configure sensors and interfaces
    /*uint8_t buffer1[4] = {SENSOR_TYPE_ADC12, sensor_adc12_gloxinia_register_general, 0, 9};
    sensor_set_config_from_buffer(1, buffer1, 4);
    
    uint8_t buffer2[4] = {SENSOR_TYPE_ADC12, sensor_adc12_gloxinia_register_config, true};
    sensor_set_config_from_buffer(1, buffer2, 3);
    
    sensor_set_status( (0<<4) | 1, SENSOR_STATUS_ACTIVE);*/
    //sensor_adc12_activate(sensor_config);
    
    // manually configure sensors and interfaces
    //uint8_t buffer1[] = {SENSOR_TYPE_ADC16, sensor_adc16_gloxinia_register_general, 0, 9};
    //sensor_set_config_from_buffer(0, 0, buffer1, 4);
    
    //uint8_t buffer2[] = {SENSOR_TYPE_ADC16, sensor_adc16_gloxinia_register_config, true};
    //sensor_set_config_from_buffer(0, 0, buffer2, 3);
    
    //uint8_t buffer3[] = {SENSOR_TYPE_ADC16, sensor_adc16_gloxinia_register_pga, 0, false};
    //sensor_set_config_from_buffer(0, 0, buffer3, 4);
    
    //sensor_set_status( (0<<4) | 1, SENSOR_STATUS_ACTIVE);
    //sensor_adc12_activate(sensor_config);
}

void sylvatica_init(void)
{
    sylvatica_init_pins();
    sylvatica_interface_init();
    
    sylatica_clock_init();
    
    __builtin_enable_interrupts();
    
    // UART serial communication (debug + print interface)
    uart_init(50000);
    UART_DEBUG_PRINT("Configured UART.");
    
    if(_RE6 == 1)
    {
        uart_connection_active = true;
        UART_DEBUG_PRINT("Detected UART host.");
    }
   
    can_init();
    UART_DEBUG_PRINT("Initialised ECAN.");
    
    can_detect_devices();
    address_find_non_reserved();

    i2c1_init(&sylvatica_i2c1_config);
    UART_DEBUG_PRINT("Initialised I2C1.");

    i2c2_init(&sylvatica_i2c2_config);
    UART_DEBUG_PRINT("Initialised I2C2.");

    sensors_init();
    UART_DEBUG_PRINT("Initialised sensor interface.");

    event_controller_init();
    UART_DEBUG_PRINT("Initialised event controller.");
    
    spi1_init();
    UART_DEBUG_PRINT("Initialised SPI for PGA.");
    
    for(int i = 0; i < N_SENSOR_INTERFACES; i++){
        pga_config[i].status = PGA_STATUS_ON;
        pga_config[i].spi_message_handler = spi1_send_message;
        pga_init(&pga_config[i]);
        
        sensor_interfaces[i]->gsensor_config[0].sensor_config.adc16.pga = &pga_config[i];
        sensor_interfaces[i]->gsensor_config[0].sensor_config.adc16.adc = &adc16_config;
    }
    UART_DEBUG_PRINT("Initialised PGAs.");
    
    sylvatica_filters_init();
    UART_DEBUG_PRINT("Initialised filters.");

    spi3_init();
    UART_DEBUG_PRINT("Initialised SPI for ADC16.");
    adc16_init(&adc16_config);
    UART_DEBUG_PRINT("Initialised ADC16.");
    
    sensor_adc12_init_filters();
    sensor_adc12_set_callback(sensor_adc12_process_block0);
    UART_DEBUG_PRINT("Initialised ADC12.");

    task_schedule_t sylvatica_read_log;
    task_t sylvatica_read_log_task = {sylvatica_send_ready_message, NULL};
    schedule_init(&sylvatica_read_log, sylvatica_read_log_task, 100);
    //schedule_specific_event(&sylvatica_read_log, ID_READY_SCHEDULE);
    
    // INFO: configuration similar to dicio project
    sylvatica_init_sensors();
    
    
    sensor_start();
    
}

void sylvatica_send_ready_message(void *data)
{
    message_t m;
    
    message_init(&m, controller_address, 0, M_HELLO, NO_INTERFACE_ID, NO_SENSOR_ID, NULL, 0);
    message_send(&m);
    
    _RE3 ^= 1;
}

bool sylvatica_configure_interrupt_routine(sensor_interface_t* interface, task_t task)
{
    switch(interface->interface_id)
    {
        case 0:
            CNENBbits.CNIEB9  = 1;
            break;
        case 1:
            CNENBbits.CNIEB8  = 1;
            break;
        case 2:
            CNENBbits.CNIEB5  = 1;
            break;
        case 3:
            CNENEbits.CNIEE4  = 1;
            break;
        case 4:
            CNENBbits.CNIEB14  = 1;
            break;
        case 5:
            CNENBbits.CNIEB13  = 1;
            break;
        case 6:
            CNENBbits.CNIEB4  = 1;
            break;
        case 7:
            CNENBbits.CNIEB3  = 1;
            break;
        default:
            return false;
    }
    
    pin_interrupt_tasks[interface->interface_id] = task;
    
    _CNIE = 1;
    
    return true;
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void) {
    if((_RB9 == 0) && (pin_interrupt_tasks[0].cb != NULL))
    {
        push_queued_task(&pin_interrupt_tasks[0]);
    }
    if((_RB8 == 0) && (pin_interrupt_tasks[1].cb != NULL))
    {
        push_queued_task(&pin_interrupt_tasks[1]);
    }
    if((_RB5 == 0) && (pin_interrupt_tasks[2].cb != NULL))
    {
        push_queued_task(&pin_interrupt_tasks[2]);
    }
    if((_RE4 == 0) && (pin_interrupt_tasks[3].cb != NULL))
    {
        push_queued_task(&pin_interrupt_tasks[3]);
    }
    if((_RB14 == 0) && (pin_interrupt_tasks[4].cb != NULL))
    {
        push_queued_task(&pin_interrupt_tasks[4]);
    }
    if((_RB13 == 0) && (pin_interrupt_tasks[5].cb != NULL))
    {
        push_queued_task(&pin_interrupt_tasks[5]);
    }
    if((_RB4 == 0) && (pin_interrupt_tasks[6].cb != NULL))
    {
        push_queued_task(&pin_interrupt_tasks[6]);
    }
    if((_RB3 == 0) && (pin_interrupt_tasks[7].cb != NULL))
    {
        push_queued_task(&pin_interrupt_tasks[7]);
    }
    
    _CNIF = 0;
}