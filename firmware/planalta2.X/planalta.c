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

pga_config_t pga_config[N_SENSOR_INTERFACES];


uint16_t copy_buffers_a[PLANALTA_N_CHANNELS][PLANALTA_COPY_BUFFER_SIZE];
uint16_t copy_buffers_b[PLANALTA_N_CHANNELS][PLANALTA_COPY_BUFFER_SIZE];

bool uart_connection_active = false;

uint8_t adc16_buffer_selector = 0;
volatile uint8_t copy_buffer_selector = 0;
extern volatile uint8_t start_filter_block0;



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
    //.sample_frequency = PLANALTA_ADC16_SAMPLE_FREQUENCY, // TODO: should be set somewhere
    .adc16_buffer_size = PLANALTA_ADC16_BUFFER_LENGTH,
    .rx_callback = planalta_adc16_callback,
    .spi_module = SPI_MODULE_SELECTOR_2,
    .rst_pin = PIN_INIT(B, 15),
    .cs_pin = PIN_INIT(E, 5),
    .conv_pin = PIN_INIT(F, 3)}
;

void planalta_init_pins(void){
   // I2C configuration
    _ODCG2 = 1; // configure I2C pins as open drain output
    _ODCG3 = 1; // configure I2C pins as open drain output
    _TRISG2 = 0;
    _TRISG3 = 0;
    _ODCD0 = 1; // nINT
    _TRISD0 = 0;
    _RD0 = 1;
    
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
    
    // TODO: there are only 4 PGAs on the board
    for(int i = 0; i < PLANALTA_N_CHANNELS; i++){
        pga_config[i].status = PGA_STATUS_ON;
        pga_init(&pga_config[i]);
    }
    UART_DEBUG_PRINT("Initialised PGAs.");
    
    planalta_filters_init();
    planalta_clear_filter_buffers();
    UART_DEBUG_PRINT("Initialised filters.");

    adc16_init(&adc16_config);
    UART_DEBUG_PRINT("Initialised ADC.");

    task_schedule_t planalta_read_log;
    task_t planalta_read_log_task = {planalta_send_ready_message, NULL};
    schedule_init(&planalta_read_log, planalta_read_log_task, 10);
    // schedule_specific_event(&dicio_read_log, ID_READY_SCHEDULE);
    
}

void planalta_send_ready_message(void *data)
{
    message_t m;

    message_init(&m, controller_address, 0, M_READY, 0, NULL, 0);
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
        start_filter_block0 = 1;
        copy_buffer_selector ^= 1;
        copy_counter = 0;
    }
    
    adc16_buffer_selector ^= 1;
}