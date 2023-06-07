#include "planalta.h"
#include "address.h"
#include <can.h>
#include <sensor.h>
#include <spi.h>
#include <pga.h>
#include <dsp.h>
#include <sensor_adc16.h>
#include <fir_common.h>
#include "planalta_definitions.h"
#include "planalta_filters.h"

i2c_config_t planalta_i2c1_config =  {
    .i2c_address = 0x0,
    .status = I2C_STATUS_PRIMARY_ON,
    .pw_sr_cb = I2C_NO_CALLBACK,
    .pr_sw_cb = I2C_NO_CALLBACK,
    .scl_pin = PIN_INIT(G, 2),
    .sda_pin = PIN_INIT(G, 3)};

i2c_config_t planalta_i2c2_config =  {
    .i2c_address = 0x0,
    .status = I2C_STATUS_PRIMARY_ON,
    .pw_sr_cb = I2C_NO_CALLBACK,
    .pr_sw_cb = I2C_NO_CALLBACK,
    .scl_pin = PIN_INIT(F, 5),
    .sda_pin = PIN_INIT(F, 4)};

uint16_t copy_buffers_a[PLANALTA_N_CHANNELS][PLANALTA_COPY_BUFFER_SIZE];
uint16_t copy_buffers_b[PLANALTA_N_CHANNELS][PLANALTA_COPY_BUFFER_SIZE];

bool uart_connection_active = false;

uint8_t adc16_buffer_selector = 0;
volatile uint8_t copy_buffer_selector = 0;


sensor_adc16_hw_config_t adc16_config = {
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
    //.sample_frequency = PLANALTA_ADC16_SAMPLE_FREQUENCY, // TODO: should be set somewhere
    .adc16_buffer_size = PLANALTA_ADC16_BUFFER_LENGTH,
    .rx_callback = adc_rx_callback_5khz,
    .spi_module = SPI_MODULE_SELECTOR_2,
    .rst_pin = PIN_INIT(B, 15),
    .cs_pin = PIN_INIT(E, 5),
    .conv_pin = PIN_INIT(F, 3)}
;

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

pga_config_t pga_config[N_SENSOR_INTERFACES] = {
    { .cs_pin = PIN_INIT(B, 14) },
    { .cs_pin = PIN_INIT(B, 2) },
    { .cs_pin = PIN_INIT(B, 0) },
    { .cs_pin = PIN_INIT(B, 1) }
};

const uint8_t n_sensor_interfaces = N_SENSOR_INTERFACES;


void planalta_clock_init(void)
{
    // FRCDIV FRC/1; PLLPRE 2; DOZE 1:8; PLLPOST 1:4; DOZEN disabled; ROI disabled; 
    CLKDIV = 0x3040;
    // TUN Center frequency; 
    OSCTUN = 0x00;
    // ROON disabled; ROSEL FOSC; RODIV 0; ROSSLP disabled; 
    REFOCON = 0x00;
    // PLLDIV 50; 
    PLLFBD = 0x32;
    // ENAPLL disabled; APLLPOST 1:256; FRCSEL FRC; SELACLK Auxiliary Oscillators; ASRCSEL Auxiliary Oscillator; AOSCMD AUX; APLLPRE 1:1; 
    ACLKCON3 = 0x2200;
    // APLLDIV 15; 
    ACLKDIV3 = 0x00;
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
    // CF no clock failure; NOSC FRCDIV; LPOSCEN disabled; CLKLOCK unlocked; OSWEN Switch is Complete; IOLOCK not-active; 
    __builtin_write_OSCCONH((uint8_t) (0x07));
    __builtin_write_OSCCONL((uint8_t) (0x00));
}


void planalta_init_pins(void){
   // I2C configuration
    _ODCG2 = 1; // configure I2C pins as open drain output
    _ODCG3 = 1; // configure I2C pins as open drain output
    _TRISG2 = 0;
    _TRISG3 = 0;
    _ODCD0 = 1; // nINT
    _TRISD0 = 0;
    _RD0 = 1;
    _CNPUG2 = 1;
    _CNPUG3 = 1;
    _CNPUD0 = 1;
    
    // UART
    _TRISD3 = 1; // U2RX
    _U2RXR = 67;            
    _TRISD2 = 0; // RTS
    _RP66R = _RPOUT_U2RTS;
    _TRISD1 = 0; // U2TX
    _RP65R = _RPOUT_U2TX;
    _TRISD4 = 1; // CTS
    _U2CTSR = 68;
    
    // SPI for ADC
    _TRISG6 = 0; // SCK2 is output
    _ANSG6 = 0;
    _TRISG7 = 1; // SDI2 is input
    _ANSG7 = 0;
    _TRISG8 = 0; // SDO2 is output
    _ANSG8 = 0;
    _TRISE5 = 0;     // CS pin
    _ANSE5 = 0;
    _RP85R = _RPOUT_OC4;
    //_RP85R = _RPOUT_SS2;
    _ANSE7 = 0;     // CONV pin
    _TRISE7 = 0;
    _RP87R = _RPOUT_OC15;
    _ANSB4 = 0;     // nEOC
    _TRISB4 = 1;
    _IC1R = 36;
    _ANSB3 = 0;     // nRESET pin
    _TRISB3 = 0;
    
    // SPI to PGA
    _ANSD6 = 0;
    _TRISD6 = 0;
    _RP69R = _RPOUT_SDO1;
    _TRISD5 = 0;
    _RP70R = _RPOUT_SCK1;
    
    // drive signals
    _TRISF2 = 0;
    _RP98R = _RPOUT_OC2;   // DRV1 signal
    _TRISF3 = 0;
    _RP99R = _RPOUT_OC2;   // DRV2 signal
    _TRISF4 = 0;
    _RP101R = _RPOUT_OC2;  // DRV3 signal
    _TRISF5 = 0;
    _RP100R = _RPOUT_OC2;  // DRV4 signal
    
    // filter selection
    _ANSB6 = 0;
    _TRISB6 = 0;
    _LATB6 = 0;
    _ANSB7 = 0;
    _TRISB7 = 0;
    _LATB7 = 0;
    
    // 4V LDO
    _ANSE1 = 0;
    _TRISE1 = 0;
    _RE1 = 1;
    
    // blinky
    _TRISF0 = 0;
    _LATF0 = 0;
    
    // 3.3V reference
    _ANSE2 = 0;
    _TRISE2 = 0;
    _RE2 = 1;
    
    // 3.3V ref to PGA
    _ANSE4 = 0;
    _TRISE4 = 0;
    _RE4 = 1;
    
    // address selection
    _ANSB15 = 0;    // A0
    _TRISB15 = 1;
    _CNPUB15 = 1;
    _ANSB10 = 0;    // A1
    _TRISB10 = 1;
    _CNPUB10 = 1;
    _ANSB9 = 0;     // A2
    _TRISB9 = 1;
    _CNPUB9 = 1;
    _ANSB12 = 0;    // A3
    _TRISB12 = 1;
    _CNPUB12 = 1;
    _TRISF6 = 1;    // A4
    _CNPUF6 = 1;
    _TRISD8 = 1;    // A5
    _CNPUD8 = 1;
    _TRISD9 = 1;    // A6
    _CNPUD9 = 1;
    _TRISD10 = 1;   // A7
    _CNPUD10 = 1;
    
    // PGA enable/disable
    /*_TRISF1 = 0;        // SD1
    _ODCF1 = 1;
    _RF1 = 0;           // disable by default
    
    _TRISD11 = 0;       // SD2
    _ODCD11 = 1;
    _RD11 = 0;          // disable by default*/
}



void planalta_init(void){
    planalta_init_pins();
    
    __builtin_enable_interrupts();
    
    // UART serial communication (debug + print interface)
    uart_init(50000);
    UART_DEBUG_PRINT("Configured UART.");
    
    can_init();
    UART_DEBUG_PRINT("Initialised ECAN.");

    i2c1_init(&planalta_i2c1_config);
    UART_DEBUG_PRINT("Initialised I2C1.");

    i2c2_init(&planalta_i2c2_config);
    UART_DEBUG_PRINT("Initialised I2C2.");

    sensors_init();
    UART_DEBUG_PRINT("Initialised sensor interface.");

    event_controller_init();
    UART_DEBUG_PRINT("Initialised event controller.");
    
    spi1_init();
    spi2_init();
    UART_DEBUG_PRINT("Initialised SPI.");
    
    for(int i = 0; i < N_SENSOR_INTERFACES; i++){
        pga_config[i].status = PGA_STATUS_ON;
        pga_config[i].spi_message_handler = spi1_send_message;
        pga_init(&pga_config[i]);
        
        sensor_interfaces[i]->gsensor_config[0].sensor_config.adc16.pga = &pga_config[i];
    }
    UART_DEBUG_PRINT("Initialised PGAs.");
    
    planalta_filters_init();
    planalta_clear_filter_buffers();
    UART_DEBUG_PRINT("Initialised filters.");

    sensor_adc16_init(&adc16_config);
    UART_DEBUG_PRINT("Initialised ADC.");

    task_schedule_t planalta_read_log;
    task_t planalta_read_log_task = {planalta_send_ready_message, NULL};
    schedule_init(&planalta_read_log, planalta_read_log_task, 10);
    // schedule_specific_event(&dicio_read_log, ID_READY_SCHEDULE);
    
}

void planalta_send_ready_message(void *data)
{
    message_t m;

    message_init(&m, controller_address, 0, M_READY, NO_SENSOR_ID, NULL, 0);
    message_send(&m);
}


void planalta_filters_init(void)
{
    uint16_t i;

    fs_fo1_buffer_i_write = f1_to_f2_buffer_i_a[0];
    fs_fo1_buffer_q_write = f1_to_f2_buffer_q_a[0];
    fs_fo2_buffer_i_write = f2_to_f3_buffer_i_a[0];
    fs_fo2_buffer_q_write = f2_to_f3_buffer_q_a[0];
    fs_fo3_buffer_i_write = f3_to_f4_buffer_i_a[0];
    fs_fo3_buffer_q_write = f3_to_f4_buffer_q_a[0];
    fs_fo4_buffer_i_write = f4_to_f5_buffer_i_a[0];
    fs_fo4_buffer_q_write = f4_to_f5_buffer_q_a[0];
    
    for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i++){
        fo2_buffer_i_read[i] = f1_to_f2_buffer_i_b[i];
        fo2_buffer_q_read[i] = f1_to_f2_buffer_q_b[i];
        fo3_buffer_i_read[i] = f2_to_f3_buffer_i_b[i];
        fo3_buffer_q_read[i] = f2_to_f3_buffer_q_b[i];
        fo4_buffer_i_read[i] = f3_to_f4_buffer_i_b[i];
        fo4_buffer_q_read[i] = f3_to_f4_buffer_q_b[i];
        fo5_buffer_i_read[i] = f4_to_f5_buffer_i_b[i];
        fo5_buffer_q_read[i] = f4_to_f5_buffer_q_b[i];
        
        fo1_buffer_i_write[i] = f1_to_f2_buffer_i_a[i];
        fo1_buffer_q_write[i] = f1_to_f2_buffer_q_a[i];
        fo2_buffer_i_write[i] = f2_to_f3_buffer_i_a[i];
        fo2_buffer_q_write[i] = f2_to_f3_buffer_q_a[i];
        fo3_buffer_i_write[i] = f3_to_f4_buffer_i_a[i];
        fo3_buffer_q_write[i] = f3_to_f4_buffer_q_a[i];
        fo4_buffer_i_write[i] = f4_to_f5_buffer_i_a[i];
        fo4_buffer_q_write[i] = f4_to_f5_buffer_q_a[i];
    }
    select_f0_to_f1 = 0;
    select_f1_to_f2 = 0;
    select_f2_to_f3 = 0;
    select_f3_to_f4 = 0;
    select_f4_to_f5 = 0;
    
    planalta_fs_n_coeffs_written = 0;
}


void planalta_clear_filter_buffers(void){
    uint16_t i, j;
    
    // todo: clear delay buffers
    
    for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i++){
        for(j = 0; j < PLANALTA_F2_INPUT_SIZE; j++){
            f1_to_f2_buffer_i_a[i][j] = 0;
            f1_to_f2_buffer_q_a[i][j] = 0;
            
            f1_to_f2_buffer_i_b[i][j] = 0;
            f1_to_f2_buffer_q_b[i][j] = 0;
        }
    }
    for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i++){
        for(j = 0; j < PLANALTA_F3_INPUT_SIZE; j++){
            f2_to_f3_buffer_i_a[i][j] = 0;
            f2_to_f3_buffer_q_a[i][j] = 0;
            
            f2_to_f3_buffer_i_b[i][j] = 0;
            f2_to_f3_buffer_q_b[i][j] = 0;
        }
    }
    for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i++){
        for(j = 0; j < PLANALTA_F4_INPUT_SIZE; j++){
            f3_to_f4_buffer_i_a[i][j] = 0;
            f3_to_f4_buffer_q_a[i][j] = 0;
            
            f3_to_f4_buffer_i_b[i][j] = 0;
            f3_to_f4_buffer_q_b[i][j] = 0;
        }
    }
    for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i++){
        for(j = 0; j < PLANALTA_F5_INPUT_SIZE; j++){
            f4_to_f5_buffer_i_a[i][j] = 0;
            f4_to_f5_buffer_i_a[i][j] = 0;
            
            f4_to_f5_buffer_i_b[i][j] = 0;
            f4_to_f5_buffer_i_b[i][j] = 0;
        }
    }
}


void planalta_adc16_callback(void){
    uint16_t i;
    static uint16_t copy_counter = 0;
    
    if (adc16_buffer_selector == 0) {
        if(copy_buffer_selector == 0){
            for(i = 0; i < PLANALTA_N_CHANNELS; i++){
                copy_adc_data(PLANALTA_ADC16_BUFFER_LENGTH / PLANALTA_N_CHANNELS, (fractional*) &copy_buffers_a[i][copy_counter], (fractional*) &adc16_rx_buffer_a[i]);
            }
        } else {
            for(i = 0; i < PLANALTA_N_CHANNELS; i++){
                copy_adc_data(PLANALTA_ADC16_BUFFER_LENGTH / PLANALTA_N_CHANNELS, (fractional*) &copy_buffers_b[i][copy_counter], (fractional*) &adc16_rx_buffer_a[i]);
            }
        }
    } else {
        if(copy_buffer_selector == 0){
            for(i = 0; i < PLANALTA_N_CHANNELS; i++){
                copy_adc_data(PLANALTA_ADC16_BUFFER_LENGTH / PLANALTA_N_CHANNELS, (fractional*) &copy_buffers_a[i][copy_counter], (fractional*) &adc16_rx_buffer_b[i]);
            }
        } else {
            for(i = 0; i < PLANALTA_N_CHANNELS; i++){
                copy_adc_data(PLANALTA_ADC16_BUFFER_LENGTH / PLANALTA_N_CHANNELS, (fractional*) &copy_buffers_b[i][copy_counter], (fractional*) &adc16_rx_buffer_b[i]);
            }
        }
    }
    copy_counter += PLANALTA_ADC16_BUFFER_LENGTH / PLANALTA_N_CHANNELS;
    
    if(copy_counter == PLANALTA_COPY_BUFFER_SIZE){
        //push_queued_task(planalta_process_task);
        copy_buffer_selector ^= 1;
        copy_counter = 0;
    }
    
    adc16_buffer_selector ^= 1;
}


void planalta_start_lia(void)
{
   /*     size_t i;
    
    UART_DEBUG_PRINT("LIA at 5kHz activated.");

    
    // enable PGA
    //_RF1 = 1;
    //_RD11 = 1;
    
    gconfig.adc16_config.channel_select = ADC16_CHANNEL_SELECT_MODE_AUTO;
    gconfig.adc16_config.channel = ADC16_CH0;
    gconfig.adc16_config.sample_frequency = 5000UL*4*8;
    gconfig.adc16_config.rx_callback = adc_rx_callback_5khz;
    gconfig.adc16_config.adc16_buffer_size = ADC_5KHZ_BUFFER_LENGTH;
    
    init_filters_5khz_lia();
    
    // update DAC configuration
    init_dac(&gconfig, false); // also starts DAC
    
    init_adc(&gconfig.adc_config);
    
    for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i+=2){
        gconfig.pga_config[i].status = PGA_STATUS_ON;
        init_pga(&gconfig.pga_config[i]);
    }
    
    planalta_set_filters(&gconfig);
    
    adc_start(&gconfig.adc_config); 
    

    stop_dac();*/
}