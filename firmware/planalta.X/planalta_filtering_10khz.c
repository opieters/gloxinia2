#include "planalta_filters.h"
#include <fir_common.h>
#include <dsp.h>
#include "planalta.h"
#include <uart.h>

void init_filters_10khz_lia(void){
    uint16_t i;

    for(i = 0; i < FIR0_N_COPIES; i++){
        FIRStructInit(&planalta_lia_filters_0[i],
            N_FIR_COEFFS0,
            fir_coeffs_0,
            COEFFS_IN_DATA,
            delay_buffers_0[i]
        );

        FIRDelayInit(&planalta_lia_filters_0[i]);
    }

    for(i = 0; i < FIR1_N_COPIES; i++){
        FIRStructInit(&planalta_lia_filters_1_q[i],
            N_FIR_COEFFS1_Q,
            fir_coeffs_1_q,
            COEFFS_IN_DATA,
            delay_buffers_1_q[i]
        );

        FIRDelayInit(&planalta_lia_filters_1_q[i]);
    }

    for(i = 0; i < FIR1_N_COPIES; i++){
        FIRStructInit(&planalta_lia_filters_1_i[i],
            N_FIR_COEFFS1_I,
            fir_coeffs_1_i,
            COEFFS_IN_DATA,
            delay_buffers_1_i[i]
        );

        FIRDelayInit(&planalta_lia_filters_1_i[i]);
    }

    for(i = 0; i < FIR2_N_COPIES; i++){
        FIRStructInit(&planalta_lia_filters_2_q[i],
            N_FIR_COEFFS2,
            fir_coeffs_2,
            COEFFS_IN_DATA,
            delay_buffers_2_q[i]
        );

        FIRDelayInit(&planalta_lia_filters_2_q[i]);
    }

    for(i = 0; i < FIR2_N_COPIES; i++){
        FIRStructInit(&planalta_lia_filters_2_i[i],
            N_FIR_COEFFS2,
            fir_coeffs_2,
            COEFFS_IN_DATA,
            delay_buffers_2_i[i]
        );

        FIRDelayInit(&planalta_lia_filters_2_i[i]);
    }

    for(i = 0; i < FIR3_N_COPIES; i++){
        FIRStructInit(&planalta_lia_filters_3_q[i],
            N_FIR_COEFFS3,
            fir_coeffs_3,
            COEFFS_IN_DATA,
            delay_buffers_3_q[i]
        );

        FIRDelayInit(&planalta_lia_filters_3_q[i]);
    }

    for(i = 0; i < FIR3_N_COPIES; i++){
        FIRStructInit(&planalta_lia_filters_3_i[i],
            N_FIR_COEFFS3,
            fir_coeffs_3,
            COEFFS_IN_DATA,
            delay_buffers_3_i[i]
        );

        FIRDelayInit(&planalta_lia_filters_3_i[i]);
    }

    for(i = 0; i < FIR4_N_COPIES; i++){
        FIRStructInit(&planalta_lia_filters_4_q[i],
            N_FIR_COEFFS4,
            fir_coeffs_4,
            COEFFS_IN_DATA,
            delay_buffers_4_q[i]
        );

        FIRDelayInit(&planalta_lia_filters_4_q[i]);
    }

    for(i = 0; i < FIR4_N_COPIES; i++){
        FIRStructInit(&planalta_lia_filters_4_i[i],
            N_FIR_COEFFS4,
            fir_coeffs_4,
            COEFFS_IN_DATA,
            delay_buffers_4_i[i]
        );

        FIRDelayInit(&planalta_lia_filters_4_i[i]);
    }

    for(i = 0; i < FIR8_N_COPIES; i++){
        FIRStructInit(&planalta_lia_filters_5_q[i],
            N_FIR_COEFFS7,
            fir_coeffs_7,
            COEFFS_IN_DATA,
            delay_buffers_7_q[i]
        );

        FIRDelayInit(&planalta_lia_filters_5_q[i]);
    }

    for(i = 0; i < FIR8_N_COPIES; i++){
        FIRStructInit(&planalta_lia_filters_5_i[i],
            N_FIR_COEFFS7,
            fir_coeffs_7,
            COEFFS_IN_DATA,
            delay_buffers_8_i[i]
        );

        FIRDelayInit(&planalta_lia_filters_5_i[i]);
    }    
}

void adc_rx_callback_10khz(void){
    uint16_t i = 0;
    static uint16_t copy_counter = 0;

    for(i = 0; i < PLANALTA_10KHZ_N_ADC_CHANNELS; i++){
        /*if (adc_buffer_selector == 0) {
            copy_uint_to_fract(PLANALTA_10KHZ_F0_OUTPUT_SIZE,
                    (fractional*) &adc_rx_buffer_a[i],
                    conversion_buffer,
                    PLANALTA_10KHZ_N_ADC_CHANNELS);
        } else {
            copy_uint_to_fract(PLANALTA_10KHZ_F0_OUTPUT_SIZE,
                    (fractional*) &adc_rx_buffer_b[i],
                    conversion_buffer,
                    PLANALTA_10KHZ_N_ADC_CHANNELS);
        }*/
        FIRDecimate(PLANALTA_10KHZ_F0_OUTPUT_SIZE,
            sample_buffer,
            conversion_buffer,
            &planalta_lia_filters_0[i],
            PLANALTA_DEC_FACT_F0);

        lia_mixer_no_dc(PLANALTA_10KHZ_F0_OUTPUT_SIZE,
            sample_buffer,
            conversion_buffer,
            &conversion_buffer[PLANALTA_10KHZ_F1_INPUT_SIZE]);

        FIRDecimate(PLANALTA_10KHZ_F1_OUTPUT_SIZE,
                fo1_buffer_i_write[i],
                conversion_buffer,
                &planalta_lia_filters_1_i[i],
                PLANALTA_DEC_FACT_F1);
        FIRDecimate(PLANALTA_10KHZ_F1_OUTPUT_SIZE,
                fo1_buffer_q_write[i],
                &conversion_buffer[PLANALTA_10KHZ_F1_INPUT_SIZE],
                &planalta_lia_filters_1_q[i],
                PLANALTA_DEC_FACT_F1);
    }

    copy_counter += PLANALTA_10KHZ_F1_OUTPUT_SIZE;
    
    if(copy_counter == PLANALTA_10KHZ_F2_INPUT_SIZE){
        start_filter_block2 = 1;
        
        copy_counter = 0;
        
        select_f1_to_f2 ^= 1;
        
        if(select_f1_to_f2){
            for(i = 0; i < PLANALTA_10KHZ_N_ADC_CHANNELS; i++){
                fo2_buffer_i_read[i] = f1_to_f2_buffer_i_b[i];
                fo2_buffer_q_read[i] = f1_to_f2_buffer_q_b[i];
                fo1_buffer_i_write[i] = f1_to_f2_buffer_i_a[i];
                fo1_buffer_q_write[i] = f1_to_f2_buffer_q_a[i];
            }
        } else {
            for(i = 0; i < PLANALTA_10KHZ_N_ADC_CHANNELS; i++){
                fo2_buffer_i_read[i] = f1_to_f2_buffer_i_a[i];
                fo2_buffer_q_read[i] = f1_to_f2_buffer_q_a[i];
                fo1_buffer_i_write[i] = f1_to_f2_buffer_i_b[i];
                fo1_buffer_q_write[i] = f1_to_f2_buffer_q_b[i];
            }
        }
    }
    adc_buffer_selector ^= 1;

}

void run_filter2_10khz(void *data){
    uint16_t i;
    static uint16_t block_counter = 0;
    start_filter_block2 = 0;

    for(i = 0; i < PLANALTA_10KHZ_N_ADC_CHANNELS; i++){
        FIRDecimate(PLANALTA_10KHZ_F2_OUTPUT_SIZE,
                &fo2_buffer_i_write[i][block_counter],
                fo2_buffer_i_read[i],
                &planalta_lia_filters_2_i[i],
                PLANALTA_DEC_FACT_F2);
        FIRDecimate(PLANALTA_10KHZ_F2_OUTPUT_SIZE,
                &fo2_buffer_q_write[i][block_counter],
                fo2_buffer_q_read[i],
                &planalta_lia_filters_2_q[i],
                PLANALTA_DEC_FACT_F2);
        
        fo2_buffer_i_write[i] += PLANALTA_10KHZ_F2_OUTPUT_SIZE;
        fo2_buffer_q_write[i] += PLANALTA_10KHZ_F2_OUTPUT_SIZE;
    }
    
    block_counter += PLANALTA_10KHZ_F2_OUTPUT_SIZE;
    if(block_counter == PLANALTA_10KHZ_F3_INPUT_SIZE){
        start_filter_block3 = 1;
        block_counter = 0;
        
        select_f2_to_f3 ^= 1;
        
        if(select_f2_to_f3){
            for(i = 0; i < PLANALTA_10KHZ_N_ADC_CHANNELS; i++){
                fo3_buffer_i_read[i] = f2_to_f3_buffer_i_b[i];
                fo3_buffer_q_read[i] = f2_to_f3_buffer_q_b[i];
                fo2_buffer_i_write[i] = f2_to_f3_buffer_i_a[i];
                fo2_buffer_q_write[i] = f2_to_f3_buffer_q_a[i];
            }
        } else {
            for(i = 0; i < PLANALTA_10KHZ_N_ADC_CHANNELS; i++){
                fo3_buffer_i_read[i] = f2_to_f3_buffer_i_a[i];
                fo3_buffer_q_read[i] = f2_to_f3_buffer_q_a[i];
                fo2_buffer_i_write[i] = f2_to_f3_buffer_i_b[i];
                fo2_buffer_q_write[i] = f2_to_f3_buffer_q_b[i];
            }
        }
    }
}
void run_filter3_10khz(void *data){
    uint16_t i;
    static uint16_t block_counter = 0;
    start_filter_block3 = 0;

    for(i = 0; i < PLANALTA_10KHZ_N_ADC_CHANNELS; i++){
        FIRDecimate(PLANALTA_10KHZ_F3_OUTPUT_SIZE,
                &fo3_buffer_i_write[i][block_counter],
                fo3_buffer_i_read[i],
                &planalta_lia_filters_3_i[i],
                PLANALTA_DEC_FACT_F3);

        FIRDecimate(PLANALTA_10KHZ_F3_OUTPUT_SIZE,
                &fo3_buffer_q_write[i][block_counter],
                fo3_buffer_q_read[i],
                &planalta_lia_filters_3_q[i],
                PLANALTA_DEC_FACT_F3);
    }
    block_counter += PLANALTA_10KHZ_F3_OUTPUT_SIZE;
    if(block_counter == PLANALTA_10KHZ_F4_INPUT_SIZE){
        start_filter_block4 = 1;
        block_counter = 0;
        
        select_f3_to_f4 ^= 1;
        
        if(select_f3_to_f4){
            for(i = 0; i < PLANALTA_10KHZ_N_ADC_CHANNELS; i++){
                fo4_buffer_i_read[i] = f3_to_f4_buffer_i_b[i];
                fo4_buffer_q_read[i] = f3_to_f4_buffer_q_b[i];
                fo3_buffer_i_write[i] = f3_to_f4_buffer_i_a[i];
                fo3_buffer_q_write[i] = f3_to_f4_buffer_q_a[i];
            }
        } else {
            for(i = 0; i < PLANALTA_10KHZ_N_ADC_CHANNELS; i++){
                fo4_buffer_i_read[i] = f3_to_f4_buffer_i_a[i];
                fo4_buffer_q_read[i] = f3_to_f4_buffer_q_a[i];
                fo3_buffer_i_write[i] = f3_to_f4_buffer_i_b[i];
                fo3_buffer_q_write[i] = f3_to_f4_buffer_q_b[i];
            }
        }
    }
}

void run_filter4_10khz(void *data){
    uint16_t i;
    static uint16_t block_counter = 0;
    start_filter_block4 = 0;

    for(i = 0; i < PLANALTA_10KHZ_N_ADC_CHANNELS; i++){
        FIRDecimate(PLANALTA_10KHZ_F4_OUTPUT_SIZE,
                &fo4_buffer_i_write[i][block_counter],
                fo4_buffer_i_read[i],
                &planalta_lia_filters_4_i[i],
                PLANALTA_DEC_FACT_F4);

        FIRDecimate(PLANALTA_10KHZ_F4_OUTPUT_SIZE,
                &fo4_buffer_q_write[i][block_counter],
                fo4_buffer_q_read[i],
                &planalta_lia_filters_4_q[i],
                PLANALTA_DEC_FACT_F4);
    }
    block_counter += PLANALTA_10KHZ_F4_OUTPUT_SIZE;
    if(block_counter == PLANALTA_10KHZ_F5_INPUT_SIZE){
        block_counter = 0;
        start_filter_block5 = 1;
    }
}


void run_filter5_10khz(void *data){
    uint16_t i;
    start_filter_block5 = 0;

    _SI2C1IE = 0;
    for(i = 0; i < PLANALTA_10KHZ_N_ADC_CHANNELS; i++){
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
    
    for(i = 0; i < PLANALTA_10KHZ_N_ADC_CHANNELS; i++){
        /*FIRDecimate(PLANALTA_10KHZ_F5_OUTPUT_SIZE,
                &planalta_lia_output_buffer_i[i],
                fo4_buffer_i_write[i],
                &planalta_lia_filters_5_i[i],
                PLANALTA_DEC_FACT_F7);

        FIRDecimate(PLANALTA_10KHZ_F5_OUTPUT_SIZE,
                &planalta_lia_output_buffer_q[i],
                fo4_buffer_q_write[i],
                &planalta_lia_filters_5_q[i],
                PLANALTA_DEC_FACT_F7);*/
    }
    
    planalta_fs_n_coeffs_written += PLANALTA_10KHZ_F5_OUTPUT_SIZE;
    if(planalta_fs_n_coeffs_written >= planalta_lia_filters_5_i[0].numCoeffs){
        planalta_fs_all_coeffs_written = true;
    }
    

}

