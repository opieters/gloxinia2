#include "planalta.h"
#include <uart.h>
#include <spi.h>
#include "filters_planalta.h"
#include "fir_coeffs.h"
#include <dsp.h>
#include <fir_common.h>
#include "planalta_filters.h"
#include "planalta_calibration.h"


#define PLANALTA_ADC { \
    .channel_select = ADC_CHANNEL_SELECT_MODE_AUTO,\
    .conversion_clock_source = ADC_CONVERSION_CLOCK_SOURCE_INTERNAL,\
    .trigger_select = ADC_TRIGGER_SELECT_MANUAL,\
    .auto_trigger_rate = ADC_SAMPLE_RATE_AUTO_TRIGGER_500KSPS,\
    .pin10_polarity = ADC_PIN10_POLARITY_SELECT_ACTIVE_LOW,\
    .pin10_output = ADC_PIN10_OUTPUT_EOC,\
    .pin10_io = ADC_PIN10_IO_SELECT_EOC_INT,\
    .auto_nap = ADC_AUTO_NAP_POWERDOWN_DISABLE,\
    .nap_powerdown = ADC_NAP_POWERDOWN_DISABLE,\
    .deep_powerdown = ADC_DEEP_POWERDOWN_DISABLE,\
    .tag_output = ADC_TAG_OUTPUT_DISABLE,\
    .sw_reset = ADC_NORMAL_OPERATION,\
    .channel = ADC_CH0,\
    .sample_frequency = PLANALTA_ADC_SAMPLE_FREQUENCY,\
    .rx_callback = adc_rx_callback_5khz,\
    .spi_module = SPI_MODULE_SELECTOR_2,\
    .rst_pin = PIN_INIT(B, 3),\
    .cs_pin = PIN_INIT(E, 5),\
    .conv_pin = PIN_INIT(E, 7)} 

#define PLANALTA_PGA0 {      \
    .channel = PGA_MUX_CH1,   \
    .gain = PGA_GAIN_1,       \
    .cs_pin = PIN_INIT(B, 14), \
    .spi_message_handler = spi1_send_message,\
    .status = PGA_STATUS_OFF}

#define PLANALTA_PGA1 {      \
    .channel = PGA_MUX_CH1,   \
    .gain = PGA_GAIN_1,       \
    .cs_pin = PIN_INIT(B, 13), \
    .spi_message_handler = spi1_send_message,\
    .status = PGA_STATUS_OFF}

#define PLANALTA_PGA2 {      \
    .channel = PGA_MUX_CH1,   \
    .gain = PGA_GAIN_1,       \
    .cs_pin = PIN_INIT(B, 0), \
    .spi_message_handler = spi1_send_message,\
    .status = PGA_STATUS_OFF}

#define PLANALTA_PGA3 {      \
    .channel = PGA_MUX_CH1,   \
    .gain = PGA_GAIN_1,       \
    .cs_pin = PIN_INIT(B, 1), \
    .spi_message_handler = spi1_send_message,\
    .status = PGA_STATUS_OFF}

#define PLANALTA_PGA_DUMMY { \
    .status = PGA_STATUS_DUMMY}

#define PLANALTA_I2C_CONFIG {\
    .i2c_address = PLANALTA_I2C_BASE_ADDRESS,\
    .status = I2C_STATUS_SLAVE_ON,\
    .mw_sr_cb = i2c_mw_sr_cb_planalta,\
    .mr_sw_cb = i2c_mr_sw_cb_planalta,\
    .scl_pin = PIN_INIT(G, 2),\
    .sda_pin = PIN_INIT(G, 3)}

#define PLANALTA_DAC_CONFIG {       \
    .signal_period = 64, \
    .pwm_period = 1000, \
    .status = DAC_STATUS_ON}

planalta_config_t gconfig = {
    .adc_config = PLANALTA_ADC,
    .pga_config = {
        PLANALTA_PGA0,
        PLANALTA_PGA_DUMMY,
        PLANALTA_PGA1,
        PLANALTA_PGA_DUMMY,
        PLANALTA_PGA2,
        PLANALTA_PGA_DUMMY,
        PLANALTA_PGA3,
        PLANALTA_PGA_DUMMY,
    },
    .channel_status = {
        PLANALTA_CHANNEL_ENABLED,
        PLANALTA_CHANNEL_ENABLED,
        PLANALTA_CHANNEL_ENABLED,
        PLANALTA_CHANNEL_ENABLED,
        PLANALTA_CHANNEL_ENABLED,
        PLANALTA_CHANNEL_ENABLED,
        PLANALTA_CHANNEL_ENABLED,
        PLANALTA_CHANNEL_ENABLED,
    },
    .i2c_config = PLANALTA_I2C_CONFIG,
    .address_selection = {
        PIN_INIT(B, 15),
        PIN_INIT(B, 10),
        PIN_INIT(B, 9),
        PIN_INIT(B, 12),
        PIN_INIT(F, 6),
        PIN_INIT(D, 8),
        PIN_INIT(D, 9),
        PIN_INIT(D, 10),
    },
    .blinky_pin = PIN_INIT(F, 0),
    .dac_config =  PLANALTA_DAC_CONFIG,
    .operation_mode = PLANALTA_LIA_F_5KHZ,
    .filter_selection_pins = {
        PIN_INIT(B, 6),
        PIN_INIT(B, 7),
    },
    .int_pin = PIN_INIT(D, 0),
    .status = PLANALTA_STATUS_INITIALISING,
};

i2c_message_t i2c_mr_message;
uint8_t i2c_mr_message_data[PLANALTA_I2C_READ_BUFFER_LENGTH];

volatile planalta_reg_t i2c_write_reg = PLANALTA_REG_STATUS;

uint8_t i2c_get_address_planalta(planalta_config_t* config){
    uint16_t address = 0x00, i;
    for(i = 0; i < PLANALTA_N_ADDRESS_SEL_PINS; i++){
        address |= (GET_BIT(config->address_selection[i].port_r, config->address_selection[i].n) << i);
    }
    address = ~address;
    address &= 0x7f;
    
    sprintf(print_buffer, "Read address: %x", address);
    uart_print(print_buffer, strlen(print_buffer));
   
    // if the address is not valid, use the default one
    if(!i2c_check_address(address)){
        address = PLANALTA_I2C_BASE_ADDRESS;
    }
    
    sprintf(print_buffer, "Actual address: %x", address);
    uart_print(print_buffer, strlen(print_buffer));
    
    return address;
}

void init_pins_planalta(void){
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

void update_config_variables(planalta_config_t* config){
    //TODO
}

void init_planalta(void){  
    uint16_t i;
    
    // error function
    set_error_loop_fn(i2c1_detect_stop);
    set_error_pin(&gconfig.blinky_pin);
    
    init_filtering();
    planalta_clear_buffers();
    
    init_pins_planalta();
#ifdef ENABLE_DEBUG
    sprintf(print_buffer, "Initialised pins.");
    uart_print(print_buffer, strlen(print_buffer));
#endif
    
    blinky_init(&gconfig.blinky_pin, 1);
#ifdef ENABLE_DEBUG
    sprintf(print_buffer, "Initialised blinky.");
    uart_print(print_buffer, strlen(print_buffer));
#endif
    
    // read I2C slave address (sets slave address for communication with dicio)
    gconfig.i2c_config.i2c_address = i2c_get_address_planalta(&gconfig);
    
#ifdef ENABLE_DEBUG
    sprintf(print_buffer, "Initialised I2C slave address to 0x%x.", gconfig.i2c_config.i2c_address);
    uart_print(print_buffer, strlen(print_buffer));
#endif
    
    i2c1_init(&gconfig.i2c_config);
#ifdef ENABLE_DEBUG
    sprintf(print_buffer, "Initialised I2C.");
    uart_print(print_buffer, strlen(print_buffer));
#endif
    
    update_config_variables(&gconfig);
    
    spi1_init();
    spi2_init();
#ifdef ENABLE_DEBUG
    sprintf(print_buffer, "Initialised SPI2.");
    uart_print(print_buffer, strlen(print_buffer));
#endif
    
    delay_ms(100);
    
    for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i+=2){
        if(i % 2 == 0){
            gconfig.pga_config[i].status = PGA_STATUS_ON;
            init_pga(&gconfig.pga_config[i]);
        }
        delay_ms(100);
    }
#ifdef ENABLE_DEBUG
    sprintf(print_buffer, "Initialised PGAs.");
    uart_print(print_buffer, strlen(print_buffer));
#endif    
    
    init_dac(&gconfig, false);
#ifdef ENABLE_DEBUG
    sprintf(print_buffer, "Initialised DAC.");
    uart_print(print_buffer, strlen(print_buffer));
#endif
    
    init_adc(&gconfig.adc_config);  
#ifdef ENABLE_DEBUG
    sprintf(print_buffer, "Initialised ADC.");
    uart_print(print_buffer, strlen(print_buffer));
#endif
    
/*#ifdef ENABLE_DEBUG
    sprintf(print_buffer, "Calibrating input channels...");
    uart_print(print_buffer, strlen(print_buffer));
#endif
    //planalta_input_calibration(&gconfig);
    
#ifdef ENABLE_DEBUG
    sprintf(print_buffer, "Calibrating output channels...");
    uart_print(print_buffer, strlen(print_buffer));
#endif
    //planalta_output_calibration(&gconfig);*/
    
    // TODO: fix to READY
    //gconfig.status = PLANALTA_STATUS_RUNNING;
    gconfig.status = PLANALTA_STATUS_READY;
    
    //gconfig.operation_mode = PLANALTA_LIA_F_5KHZ;
}


void loop_planalta(void){
#ifdef ENABLE_DEBUG
    sprintf(print_buffer, "Entering loop...");
    uart_print(print_buffer, strlen(print_buffer));
#endif
    
    while(1){
        
        while(gconfig.status != PLANALTA_STATUS_RUNNING){
            i2c1_detect_stop();
        }
        
        switch(gconfig.operation_mode){
            case PLANALTA_LIA_F_50KHZ:
                planalta_filter_50khz();
                break;
            case PLANALTA_LIA_F_25KHZ:
                planalta_filter_25khz();
                break;
            case PLANALTA_LIA_F_10KHZ:
                planalta_filter_10khz();
                break;
            case PLANALTA_LIA_F_5KHZ:
                planalta_filter_5khz();
                break;
            case PLANALTA_FS:
                planalta_sweep_frequency(&gconfig);
                gconfig.status = PLANALTA_STATUS_READY;
                break;
            default:
                report_error("planalta: configuration error signal frequency not supported.");
                break;
        }
    }
}

void i2c_mr_sw_cb_planalta(i2c_message_t* m){
    
}

void planalta_i2c_read_config(const planalta_reg_t reg){
    switch(reg){
        case PLANALTA_REG_STATUS:
            i2c_mr_message_data[0] = 1;
            i2c_mr_message_data[1] = 0;
            i2c_mr_message_data[1] |= gconfig.status;

            i2c_init_read_message(
                &i2c_mr_message,
                i2c_mr_message_data,
                1+1);
            break;
        default:
            i2c_init_read_message(
                &i2c_mr_message,
                i2c_mr_message_data,
                0);
            return;
    }
    
    i2c_set_read_message(&i2c_mr_message);
}

void i2c_mw_sr_cb_planalta(i2c_message_t* m){
  
    // master sends data to slave (last bit is 0)
    if(m->data_length < 1){
        return;
    }
        
    i2c_write_reg = (planalta_reg_t) m->data[0];
    
    //sprintf(print_buffer, "MW SR reg: %x", i2c_write_reg);
    //uart_print(print_buffer, strlen(print_buffer));
    
    switch(i2c_write_reg){
        case PLANALTA_REG_STATUS:
            if(m->data_length >= 2){      
                if((m->data[1] & PLANALTA_STATUS_OFF) == PLANALTA_STATUS_OFF){
                    // TODO: real low power mode
                    gconfig.status = PLANALTA_STATUS_READY;
                }
                
                if((m->data[1] & PLANALTA_STATUS_RESET_BUFFER) == PLANALTA_STATUS_RESET_BUFFER){
                    planalta_clear_buffers();
                }
                
                if((m->data[1] & PLANALTA_STATUS_RESET) == PLANALTA_STATUS_RESET){
                    asm ("RESET");
                }
            }
            planalta_i2c_read_config(i2c_write_reg);
            break;                
        case PLANALTA_REG_ADC:
            planalta_i2c_read_config(i2c_write_reg);
            break;
        case PLANALTA_REG_START_STOP:
            if(m->data_length >= 2){
                if(m->data[1] == 0){
                    gconfig.status = PLANALTA_STATUS_READY;
                    planalta_fs_all_coeffs_written = true;
                } else {
                    gconfig.status = PLANALTA_STATUS_RUNNING;
                }
            }
            
            break;
        case PLANALTA_REG_MODE:
            if(m->data_length >= 2){
                if(m->data[1] < PLANALTA_N_OP_MODES){
                    gconfig.operation_mode = m->data[1];
                }
            }
            break;
        case PLANALTA_REG_CONFIG_CH0:
            planalta_i2c_channel_config(0, &m->data[1]);
            break;
        case PLANALTA_REG_CONFIG_CH1:
            planalta_i2c_channel_config(1, &m->data[1]);
            break;
        case PLANALTA_REG_CONFIG_CH2:
            planalta_i2c_channel_config(2, &m->data[1]);
            break;
        case PLANALTA_REG_CONFIG_CH3:
            planalta_i2c_channel_config(3, &m->data[1]);
            break;
        case PLANALTA_REG_CONFIG_CH4:
            planalta_i2c_channel_config(4, &m->data[1]);
            break;
        case PLANALTA_REG_CONFIG_CH5:
            planalta_i2c_channel_config(5, &m->data[1]);
            break;
        case PLANALTA_REG_CONFIG_CH6:
            planalta_i2c_channel_config(6, &m->data[1]);
            break;
        case PLANALTA_REG_CONFIG_CH7:
            planalta_i2c_channel_config(7, &m->data[1]);
            break;
        case PLANALTA_REG_DATA_LIA:
            planalta_i2c_read_copy_buffer_data();
            break;
        case PLANALTA_REG_CONFIG_T0:
            if(m->data_length >= 3){
                //TODO
                //gconfig.dac_config.channels[0].pwm_period = (m->data[1] << 8) + m->data[2];
            } 
            planalta_i2c_read_config(i2c_write_reg);
            break;
        case PLANALTA_REG_CONFIG_T1:
            if(m->data_length >= 3){
                //TODO
                //gconfig.dac_config.channels[1].pwm_period = (m->data[1] << 8) + m->data[2];
            }
            planalta_i2c_read_config(i2c_write_reg);
            break;
        case PLANALTA_REG_DATA_FS:
            planalta_fs_i2c_copy_data();
            break;
        default:
            break;
    }
}


void planalta_i2c_read_buffer_write(uint8_t* data, size_t length){
    size_t i;
    
    length = MIN(ARRAY_LENGTH(i2c_mr_message_data), length);
    
    i2c_mr_message_data[0] = (uint8_t) length;
    
    for(i = 0; i < length; i++){
        i2c_mr_message_data[i+1] = data[i];
    }
    i2c_init_read_message(
        &i2c_mr_message,
        i2c_mr_message_data,
        length+1);
    
    i2c_set_read_message(&i2c_mr_message);
    
    SET_PORT_BIT(gconfig.int_pin);
    
}

void planalta_i2c_read_copy_buffer_data(){
    size_t i;
    
    i2c_mr_message_data[0] = 4*PLANALTA_N_ADC_CHANNELS;
    
    if(planalta_lia_obuffer_selector){
        for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i++){
            i2c_mr_message_data[1+4*i] = (planalta_lia_obuffer_b_i[PLANALTA_N_ADC_CHANNELS-i-1] >> 8) & 0xff;
            i2c_mr_message_data[1+4*i+1] = planalta_lia_obuffer_b_i[PLANALTA_N_ADC_CHANNELS-i-1] & 0xff;
            i2c_mr_message_data[1+4*i+2] = (planalta_lia_obuffer_b_q[PLANALTA_N_ADC_CHANNELS-i-1] >> 8) & 0xff;
            i2c_mr_message_data[1+4*i+3] = planalta_lia_obuffer_b_q[PLANALTA_N_ADC_CHANNELS-i-1] & 0xff;
        }
    } else {
        for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i++){
            i2c_mr_message_data[1+4*i] = (planalta_lia_obuffer_a_i[PLANALTA_N_ADC_CHANNELS-i-1] >> 8) & 0xff;
            i2c_mr_message_data[1+4*i+1] = planalta_lia_obuffer_a_i[PLANALTA_N_ADC_CHANNELS-i-1] & 0xff;
            i2c_mr_message_data[1+4*i+2] = (planalta_lia_obuffer_a_q[PLANALTA_N_ADC_CHANNELS-i-1] >> 8) & 0xff;
            i2c_mr_message_data[1+4*i+3] = planalta_lia_obuffer_a_q[PLANALTA_N_ADC_CHANNELS-i-1] & 0xff;
        }
    }
    
    i2c_init_read_message(&i2c_mr_message, i2c_mr_message_data, 1+4*PLANALTA_N_ADC_CHANNELS);
    
    i2c_set_read_message(&i2c_mr_message);
    SET_PORT_BIT(gconfig.int_pin);
}

void planalta_fs_i2c_copy_data(){
    size_t i, j;
    
    // in case the frequency sweep is still ongoing, do not reply
    if((gconfig.status != PLANALTA_STATUS_READY) && (gconfig.operation_mode != PLANALTA_FS)){
        i2c_init_read_message(
            &i2c_mr_message,
            i2c_mr_message_data,
            0);
        
        i2c_set_read_message(&i2c_mr_message);
        SET_PORT_BIT(gconfig.int_pin);
        
        #ifdef ENABLE_DEBUG
            uart_simple_print("FS error.");
        #endif

        return;
    }
    for(i = 0; i < PLANALTA_FS_CHANNELS; i++){
        if(gconfig.channel_data_status[i] != PLANALTA_CHANNEL_DATA_STATUS_NEW) {
            i2c_init_read_message(
                &i2c_mr_message,
                i2c_mr_message_data,
                0);
            i2c_set_read_message(&i2c_mr_message);
            SET_PORT_BIT(gconfig.int_pin);

            #ifdef ENABLE_DEBUG
                sprintf(print_buffer, "No new data available in channel %d", i);
                uart_print(print_buffer, strlen(print_buffer));
            #endif

            return;
        }
    }
    for(i = 0; i < PLANALTA_FS_CHANNELS; i++){
        gconfig.channel_data_status[i] = PLANALTA_CHANNEL_DATA_STATUS_READ;
    }
    
    i2c_mr_message_data[0] = 5*PLANALTA_FS_FREQ_N*4;

    for(j = 0; j < PLANALTA_FS_CHANNELS; j++){
        for(i = 0; i < PLANALTA_FS_FREQ_N; i++){
            // copy data to i2c buffer
            i2c_mr_message_data[1 + j*PLANALTA_FS_FREQ_N + 4*i]   = (planalta_fs_data[j][i][0] >> 8) & 0xff;
            i2c_mr_message_data[1 + j*PLANALTA_FS_FREQ_N + 4*i+1] = planalta_fs_data[j][i][0] & 0xff;
            i2c_mr_message_data[1 + j*PLANALTA_FS_FREQ_N + 4*i+2] = (planalta_fs_data[j][i][1] >> 8) & 0xff;
            i2c_mr_message_data[1 + j*PLANALTA_FS_FREQ_N + 4*i+3] = planalta_fs_data[j][i][1] & 0xff;

            // clear data
            planalta_fs_data[j][i][0] = 0;
            planalta_fs_data[j][i][1] = 0;
        }
    }

    i2c_init_read_message(
        &i2c_mr_message,
        i2c_mr_message_data,
        5*PLANALTA_FS_FREQ_N*4+1);


    i2c_set_read_message(&i2c_mr_message);

    #ifdef ENABLE_DEBUG
        uart_simple_print("Copied data of channels to buffer.");
    #endif
    
    SET_PORT_BIT(gconfig.int_pin);
}

void planalta_channel_config(uint8_t channel_n, 
        planalta_channel_status_t status, pga_gain_t gain){    
        // update configuration fields
    if(status == PLANALTA_CHANNEL_ENABLED){
        gconfig.channel_status[channel_n] = PLANALTA_CHANNEL_ENABLED;
        gconfig.pga_config[channel_n].status = PGA_STATUS_ON;
    } else {
        gconfig.channel_status[channel_n] = PLANALTA_CHANNEL_DISABLED;
        gconfig.pga_config[channel_n].status = PGA_STATUS_OFF;
    }
    
    gconfig.pga_config[channel_n].gain = gain; 
    
    // update actual hardware
    update_pga_status(&gconfig.pga_config[channel_n]);
    
    if(gconfig.channel_status[channel_n] == PLANALTA_CHANNEL_ENABLED){
        switch(channel_n){
            case 0:
                _RP98R = _RPOUT_OC2;   // DRV1 signal
                break;
            case 1:
                _RP99R = _RPOUT_OC2;   // DRV2 signal
                break;
            case 2:
                _RP100R = _RPOUT_OC2;  // DRV3 signal
                break;
            case 3:
                _RP101R = _RPOUT_OC2;  // DRV4 signal
                break;
            default:
                break;
        }
    } else {
        switch(channel_n){
            case 0:
                _RP98R = 0;   // DRV1 signal
                _RF2 = 0;
                break;
            case 1:
                _RP99R = 0;   // DRV2 signal
                _RF3 = 0;
                break;
            case 2:
                _RP100R = 0;  // DRV3 signal
                _RF4 = 0;
                break;
            case 3:
                _RP101R = 0;  // DRV4 signal
                _RF5 = 0;
                break;
            default:
                break;
        }
    }
}

void planalta_i2c_channel_config(uint8_t channel_n, uint8_t* data){
    pga_gain_t gain; 
    planalta_channel_status_t status; 

    if(gconfig.adc_config.status == ADC_STATUS_ON){
        return;
    }
    
    gain = (pga_gain_t) ((data[0] & PLANALTA_CH_CONFIG_GAIN) >> 4); 
    
        // update configuration fields
    if((data[0] & PLANALTA_CH_CONFIG_ON) == PLANALTA_CH_CONFIG_ON){
        status = PLANALTA_CHANNEL_ENABLED;
    } else {
        status = PLANALTA_CHANNEL_DISABLED;
    }
    
    planalta_channel_config(channel_n, status, gain);
    
    // prepare read message
    switch(status){
        case PLANALTA_CHANNEL_ENABLED:
            i2c_mr_message_data[0] = 1;
            break;
        case PLANALTA_CHANNEL_DISABLED:
            i2c_mr_message_data[0] = 0;
            break;
    }
    
    i2c_mr_message_data[0] = (i2c_mr_message_data[0] << 3) | PLANALTA_CH_CONFIG_SET_GAIN(gain);
    i2c_mr_message_data[0] = i2c_mr_message_data[0] << 4;
    
    i2c_init_read_message(
        &i2c_mr_message,
        i2c_mr_message_data,
        1);
    
    i2c_set_read_message(&i2c_mr_message);
}

void planalta_clear_buffers(void){
    planalta_clear_filter_buffers();
    
    i2c_init_read_message(
        &i2c_mr_message,
        i2c_mr_message_data,
        0);
    
    i2c_set_read_message(&i2c_mr_message);
}

void planalta_set_filters(planalta_config_t* config){
    switch(config->operation_mode){
        case PLANALTA_LIA_F_50KHZ:
            CLEAR_PORT_BIT(gconfig.filter_selection_pins[0]);
            CLEAR_PORT_BIT(gconfig.filter_selection_pins[1]);
            break;
        case PLANALTA_LIA_F_25KHZ:
            SET_PORT_BIT(gconfig.filter_selection_pins[0]);
            CLEAR_PORT_BIT(gconfig.filter_selection_pins[1]);
            break;
        case PLANALTA_LIA_F_10KHZ:
            CLEAR_PORT_BIT(gconfig.filter_selection_pins[0]);
            SET_PORT_BIT(gconfig.filter_selection_pins[1]);
            break;
        case PLANALTA_LIA_F_5KHZ:
            SET_PORT_BIT(gconfig.filter_selection_pins[0]);
            SET_PORT_BIT(gconfig.filter_selection_pins[1]);
            break;
        case PLANALTA_FS:
            switch(config->sweep_frequency){
                case PLANALTA_FS_FREQ_50KHZ:
                    CLEAR_PORT_BIT(gconfig.filter_selection_pins[0]);
                    CLEAR_PORT_BIT(gconfig.filter_selection_pins[1]);
                    break;
                case PLANALTA_FS_FREQ_20KHZ:
                    SET_PORT_BIT(gconfig.filter_selection_pins[0]);
                    CLEAR_PORT_BIT(gconfig.filter_selection_pins[1]);
                    break;
                case PLANALTA_FS_FREQ_10KHZ:
                    CLEAR_PORT_BIT(gconfig.filter_selection_pins[0]);
                    SET_PORT_BIT(gconfig.filter_selection_pins[1]);
                    break;
                case PLANALTA_FS_FREQ_5KHZ:
                case PLANALTA_FS_FREQ_2KHZ:
                case PLANALTA_FS_FREQ_1KHZ:
                case PLANALTA_FS_FREQ_500HZ:
                case PLANALTA_FS_FREQ_200HZ:
                case PLANALTA_FS_FREQ_100HZ:
                case PLANALTA_FS_FREQ_50HZ:
                case PLANALTA_FS_FREQ_20HZ:
                case PLANALTA_FS_FREQ_10HZ:
                    SET_PORT_BIT(gconfig.filter_selection_pins[0]);
                    SET_PORT_BIT(gconfig.filter_selection_pins[1]);
                    break;
                default:
                    report_error("planalta: fs frequency not supported.");
                    break;
            }
            break;
        default:
            report_error("planalta: filter config not supported.");
            break;
    }
}
