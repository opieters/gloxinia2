#include "planalta_filters.h"
#include <fir_common.h>
#include <dsp.h>
#include "planalta.h"
#include <uart.h>

extern planalta_config_t gconfig;

void adc_rx_callback_25khz(void){
    uint16_t i = 0;
    static uint16_t copy_counter = 0;

    for(i = 0; i < PLANALTA_25KHZ_N_ADC_CHANNELS; i++){
        /*if (adc_buffer_selector == 0) {
            copy_uint_to_fract(PLANALTA_25KHZ_F0_OUTPUT_SIZE,
                    (fractional*) &adc_rx_buffer_a[i],
                    conversion_buffer,
                    PLANALTA_25KHZ_N_ADC_CHANNELS);
        } else {
            copy_uint_to_fract(PLANALTA_25KHZ_F0_OUTPUT_SIZE,
                    (fractional*) &adc_rx_buffer_b[i],
                    conversion_buffer,
                    PLANALTA_25KHZ_N_ADC_CHANNELS);
        }*/
        fir_compressed(PLANALTA_25KHZ_F0_OUTPUT_SIZE,
            sample_buffer,
            conversion_buffer,
            &planalta_lia_filters_0[i],
            PLANALTA_DEC_FACT_F0);

        lia_mixer_no_dc(PLANALTA_25KHZ_F0_OUTPUT_SIZE,
            sample_buffer,
            conversion_buffer,
            &conversion_buffer[PLANALTA_25KHZ_F1_INPUT_SIZE]);

        fir_compressed(PLANALTA_25KHZ_F1_OUTPUT_SIZE,
                fo1_buffer_i_write[i],
                conversion_buffer,
                &planalta_lia_filters_1_i[i],
                PLANALTA_DEC_FACT_F1);
        fir_compressed(PLANALTA_25KHZ_F1_OUTPUT_SIZE,
                fo1_buffer_q_write[i],
                &conversion_buffer[PLANALTA_25KHZ_F1_INPUT_SIZE],
                &planalta_lia_filters_1_q[i],
                PLANALTA_DEC_FACT_F1);
        
        
        fo1_buffer_i_write[i] += PLANALTA_25KHZ_F1_OUTPUT_SIZE;
        fo1_buffer_q_write[i] += PLANALTA_25KHZ_F1_OUTPUT_SIZE;
    }

    copy_counter += PLANALTA_25KHZ_F1_OUTPUT_SIZE;
    
    if(copy_counter == PLANALTA_25KHZ_F2_INPUT_SIZE){
        start_filter_block2 = 1;
        
        copy_counter = 0;
        
        select_f1_to_f2 ^= 1;
        
        if(select_f1_to_f2){
            for(i = 0; i < PLANALTA_25KHZ_N_ADC_CHANNELS; i++){
                fo2_buffer_i_read[i] = f1_to_f2_buffer_i_b[i];
                fo2_buffer_q_read[i] = f1_to_f2_buffer_q_b[i];
                fo1_buffer_i_write[i] = f1_to_f2_buffer_i_a[i];
                fo1_buffer_q_write[i] = f1_to_f2_buffer_q_a[i];
            }
        } else {
            for(i = 0; i < PLANALTA_25KHZ_N_ADC_CHANNELS; i++){
                fo2_buffer_i_read[i] = f1_to_f2_buffer_i_a[i];
                fo2_buffer_q_read[i] = f1_to_f2_buffer_q_a[i];
                fo1_buffer_i_write[i] = f1_to_f2_buffer_i_b[i];
                fo1_buffer_q_write[i] = f1_to_f2_buffer_q_b[i];
            }
        }
    }
    adc_buffer_selector ^= 1;

}

void run_filter2_25khz(void){
    uint16_t i;
    static uint16_t block_counter = 0;
    start_filter_block2 = 0;

    for(i = 0; i < PLANALTA_25KHZ_N_ADC_CHANNELS; i++){
        fir_compressed(PLANALTA_25KHZ_F2_OUTPUT_SIZE,
                &fo2_buffer_i_write[i][block_counter],
                fo2_buffer_i_read[i],
                &planalta_lia_filters_2_i[i],
                PLANALTA_DEC_FACT_F2);
        fir_compressed(PLANALTA_25KHZ_F2_OUTPUT_SIZE,
                &fo2_buffer_q_write[i][block_counter],
                fo2_buffer_q_read[i],
                &planalta_lia_filters_2_q[i],
                PLANALTA_DEC_FACT_F2);
        
        fo2_buffer_i_write[i] += PLANALTA_25KHZ_F2_OUTPUT_SIZE;
        fo2_buffer_q_write[i] += PLANALTA_25KHZ_F2_OUTPUT_SIZE;
    }
    
    block_counter += PLANALTA_25KHZ_F2_OUTPUT_SIZE;
    if(block_counter == PLANALTA_25KHZ_F3_INPUT_SIZE){
        start_filter_block3 = 1;
        block_counter = 0;
        
        select_f2_to_f3 ^= 1;
        
        if(select_f2_to_f3){
            for(i = 0; i < PLANALTA_25KHZ_N_ADC_CHANNELS; i++){
                fo3_buffer_i_read[i] = f2_to_f3_buffer_i_b[i];
                fo3_buffer_q_read[i] = f2_to_f3_buffer_q_b[i];
                fo2_buffer_i_write[i] = f2_to_f3_buffer_i_a[i];
                fo2_buffer_q_write[i] = f2_to_f3_buffer_q_a[i];
            }
        } else {
            for(i = 0; i < PLANALTA_25KHZ_N_ADC_CHANNELS; i++){
                fo3_buffer_i_read[i] = f2_to_f3_buffer_i_a[i];
                fo3_buffer_q_read[i] = f2_to_f3_buffer_q_a[i];
                fo2_buffer_i_write[i] = f2_to_f3_buffer_i_b[i];
                fo2_buffer_q_write[i] = f2_to_f3_buffer_q_b[i];
            }
        }
    }
}
void run_filter3_25khz(void){
    uint16_t i;
    static uint16_t block_counter = 0;
    start_filter_block3 = 0;

    for(i = 0; i < PLANALTA_25KHZ_N_ADC_CHANNELS; i++){
        fir_compressed(PLANALTA_25KHZ_F3_OUTPUT_SIZE,
                &fo3_buffer_i_write[i][block_counter],
                fo3_buffer_i_read[i],
                &planalta_lia_filters_3_i[i],
                PLANALTA_DEC_FACT_F3);

        fir_compressed(PLANALTA_25KHZ_F3_OUTPUT_SIZE,
                &fo3_buffer_q_write[i][block_counter],
                fo3_buffer_q_read[i],
                &planalta_lia_filters_3_q[i],
                PLANALTA_DEC_FACT_F3);
    }
    block_counter += PLANALTA_25KHZ_F3_OUTPUT_SIZE;
    if(block_counter == PLANALTA_25KHZ_F4_INPUT_SIZE){
        start_filter_block4 = 1;
        block_counter = 0;
        
        select_f3_to_f4 ^= 1;
        
        if(select_f3_to_f4){
            for(i = 0; i < PLANALTA_25KHZ_N_ADC_CHANNELS; i++){
                fo4_buffer_i_read[i] = f3_to_f4_buffer_i_b[i];
                fo4_buffer_q_read[i] = f3_to_f4_buffer_q_b[i];
                fo3_buffer_i_write[i] = f3_to_f4_buffer_i_a[i];
                fo3_buffer_q_write[i] = f3_to_f4_buffer_q_a[i];
            }
        } else {
            for(i = 0; i < PLANALTA_25KHZ_N_ADC_CHANNELS; i++){
                fo4_buffer_i_read[i] = f3_to_f4_buffer_i_a[i];
                fo4_buffer_q_read[i] = f3_to_f4_buffer_q_a[i];
                fo3_buffer_i_write[i] = f3_to_f4_buffer_i_b[i];
                fo3_buffer_q_write[i] = f3_to_f4_buffer_q_b[i];
            }
        }
    }
}

void run_filter4_25khz(void){
    uint16_t i;
    static uint16_t block_counter = 0;
    start_filter_block4 = 0;

    for(i = 0; i < PLANALTA_25KHZ_N_ADC_CHANNELS; i++){
        fir_compressed(PLANALTA_25KHZ_F4_OUTPUT_SIZE,
                &fo4_buffer_i_write[i][block_counter],
                fo4_buffer_i_read[i],
                &planalta_lia_filters_4_i[i],
                PLANALTA_DEC_FACT_F4);

        fir_compressed(PLANALTA_25KHZ_F4_OUTPUT_SIZE,
                &fo4_buffer_q_write[i][block_counter],
                fo4_buffer_q_read[i],
                &planalta_lia_filters_4_q[i],
                PLANALTA_DEC_FACT_F4);
    }
    block_counter += PLANALTA_25KHZ_F4_OUTPUT_SIZE;
    if(block_counter == PLANALTA_25KHZ_F5_INPUT_SIZE){
        block_counter = 0;
        start_filter_block5 = 1;
    }
}


void run_filter5_25khz(void){
    uint16_t i;
    start_filter_block5 = 0;
    
    _SI2C1IE = 0;
    for(i = 0; i < PLANALTA_25KHZ_N_ADC_CHANNELS; i++){
        /*i2c_channel_data[i][1] = (uint8_t) ((planalta_lia_output_buffer_i[i] >> 8) & 0xff);
        i2c_channel_data[i][2] = (uint8_t) (planalta_lia_output_buffer_i[i] & 0xff);
        i2c_channel_data[i][3] = (uint8_t) ((planalta_lia_output_buffer_q[i] >> 8) & 0xff);
        i2c_channel_data[i][4] = (uint8_t) (planalta_lia_output_buffer_q[i] & 0xff);

        i2c_init_read_message(
            &i2c_channel_message[i],
            i2c_channel_data[i],
            PLANALTA_I2C_READ_CH_BUFFER_LENGTH);*/
    }
    _SI2C1IE = 1; 
    

    for(i = 0; i < PLANALTA_25KHZ_N_ADC_CHANNELS; i++){
        /*fir_compressed(PLANALTA_25KHZ_F5_OUTPUT_SIZE,
                &planalta_lia_output_buffer_i[i],
                fo4_buffer_i_write[i],
                &planalta_lia_filters_5_i[i],
                PLANALTA_DEC_FACT_F6);

        fir_compressed(PLANALTA_25KHZ_F5_OUTPUT_SIZE,
                &planalta_lia_output_buffer_q[i],
                fo4_buffer_q_write[i],
                &planalta_lia_filters_5_q[i],
                PLANALTA_DEC_FACT_F6);*/
    }
    
    planalta_fs_n_coeffs_written += PLANALTA_25KHZ_F5_OUTPUT_SIZE;
    if(planalta_fs_n_coeffs_written >= planalta_lia_filters_5_i[0].numCoeffs){
        planalta_fs_all_coeffs_written = true;
    }
    
    CLEAR_PORT_BIT(gconfig.int_pin);   
}

void planalta_filter_25khz(void) {
    UART_DEBUG_PRINT("LIA at 25kHz activated.");

    
    // enable PGA
    //_RF1 = 1;
    //_RD11 = 1;
    
    while(gconfig.status == PLANALTA_STATUS_READY){
        i2c1_detect_stop();
        
        if(start_filter_block2){
            run_filter2_25khz();
        }
        
        i2c1_detect_stop();
        
        if(start_filter_block5){
            run_filter5_25khz();
            continue;
        }
        
        i2c1_detect_stop();
        
        if(start_filter_block4){
            run_filter4_25khz();
            continue;
        }
        
        i2c1_detect_stop();
        
        if(start_filter_block3){
            run_filter3_25khz();
            continue;
        }
    }
    
    // disable PGA
    //_RF1 = 0;
    //_RD11 = 0;
}
