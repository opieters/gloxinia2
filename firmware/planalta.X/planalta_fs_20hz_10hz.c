#include <xc.h>
#include <dsp.h>
#include <fir_common.h>
#include <uart.h>
#include "planalta_filters.h"
#include "planalta.h"

extern planalta_config_t gconfig;

extern i2c_message_t i2c_channel_message[PLANALTA_N_ADC_CHANNELS];
extern uint8_t i2c_channel_data[PLANALTA_N_ADC_CHANNELS][PLANALTA_I2C_READ_CH_BUFFER_LENGTH];

static uint16_t final_decimation_factor;
static uint16_t final_output_size;
static uint16_t final_input_size;

void reset_buffers_fs_20hz(void){
    start_filter_block1 = 0;
    start_filter_block2 = 0;
    start_filter_block3 = 0;

    select_f0_to_f1 = 0;
    select_f1_to_f2 = 0;
    select_f2_to_f3 = 0;
    
    fs_fo1_buffer_read = f4_to_f5_buffer_i_a[0];
    fs_fo0_buffer_write = f4_to_f5_buffer_i_b[0];

    fs_fo2_buffer_read = f1_to_f2_buffer_i_a[0];
    fs_fo1_buffer_write = f1_to_f2_buffer_i_b[0];

    fs_fo3_buffer_read = f4_to_f5_buffer_i_a[1];
    fs_fo2_buffer_write = f4_to_f5_buffer_i_b[1];
}

void filters_fs_20hz_10hz_init(planalta_fs_freq_t freq){
    FIRStructInit(&planalta_fs_filters_0,
        N_FIR_COEFFS10,
        fir_coeffs_10,
        COEFFS_IN_DATA,
        delay_buffers_5_i_0
    );
    
    FIRStructInit(&planalta_fs_filters_1,
        N_FIR_COEFFS10,
        fir_coeffs_10,
        COEFFS_IN_DATA,
        delay_buffers_5_q_0
    );

    FIRStructInit(&planalta_fs_filters_2,
        N_FIR_COEFFS10,
        fir_coeffs_10,
        COEFFS_IN_DATA,
        delay_buffers_5_i_1
    );
    
    FIRStructInit(&planalta_fs_filters_3,
        N_FIR_COEFFS0,
        fir_coeffs_0,
        COEFFS_IN_DATA,
        delay_buffers_0_0
    );
    
    switch(freq){
        case PLANALTA_FS_FREQ_20HZ:
            FIRStructInit(&planalta_fs_filters_4_i,
                N_FIR_COEFFS11_I,
                fir_coeffs_11_i,
                COEFFS_IN_DATA,
                delay_buffers_4_i_0
            );

            FIRStructInit(&planalta_fs_filters_4_q,
                N_FIR_COEFFS11_Q,
                fir_coeffs_11_q,
                COEFFS_IN_DATA,
                delay_buffers_4_q_0
            );
            
            final_decimation_factor = PLANALTA_FS_20HZ_F4_DEC_FACT;
            final_output_size = PLANALTA_FS_20HZ_F4_OUTPUT_SIZE;
            final_input_size = PLANALTA_FS_20HZ_F4_INPUT_SIZE;
            break;
        case PLANALTA_FS_FREQ_10HZ:
        default:
            FIRStructInit(&planalta_fs_filters_4_i,
                N_FIR_COEFFS12_I,
                fir_coeffs_12_i,
                COEFFS_IN_DATA,
                delay_buffers_4_i_0
            );

            FIRStructInit(&planalta_fs_filters_4_q,
                N_FIR_COEFFS12_Q,
                fir_coeffs_12_q,
                COEFFS_IN_DATA,
                delay_buffers_4_q_0
            );
            
            final_decimation_factor = PLANALTA_FS_10HZ_F4_DEC_FACT;
            final_output_size = PLANALTA_FS_10HZ_F4_OUTPUT_SIZE;
            final_input_size = PLANALTA_FS_10HZ_F4_INPUT_SIZE;
            break;
    }
    FIRDelayInit(&planalta_fs_filters_4_i);
    FIRDelayInit(&planalta_fs_filters_4_q);

}

void adc_rx_callback_fs_20hz_10hz(void){
    static uint16_t copy_counter = 0;
    
    // convert samples to fractional format and split the different channels
    //  from this point onwards, all channels are processed one by one
    /*if (adc_buffer_selector == 0) {
        copy_uint_to_fract(ADC_FS_20HZ_10HZ_BUFFER_LENGTH,
                (fractional*) &adc_rx_buffer_a[0],
                conversion_buffer,
                1);
    } else {
        copy_uint_to_fract(ADC_FS_20HZ_10HZ_BUFFER_LENGTH,
                (fractional*) &adc_rx_buffer_b[0],
                conversion_buffer,
                1);
    }*/

    // FO0
    fir_compressed(PLANALTA_FS_20HZ_10HZ_F0_OUTPUT_SIZE,
        fs_fo0_buffer_write,
        conversion_buffer,
        &planalta_fs_filters_0,
        PLANALTA_FS_20HZ_10HZ_F0_DEC_FACT);
    
    copy_counter += PLANALTA_FS_20HZ_10HZ_F0_OUTPUT_SIZE;

    if(copy_counter == PLANALTA_FS_20HZ_10HZ_F1_INPUT_SIZE){
        select_f0_to_f1 ^= 1;

        if(select_f0_to_f1){
            fs_fo1_buffer_read = f4_to_f5_buffer_i_b[0];
            fs_fo0_buffer_write = f4_to_f5_buffer_i_a[0];
        } else {
            fs_fo1_buffer_read = f4_to_f5_buffer_i_a[0];
            fs_fo0_buffer_write = f4_to_f5_buffer_i_b[0];
        }
        start_filter_block1 = 1;
        
        copy_counter = 0;
    }
    
    adc_buffer_selector ^= 1;
}

void run_fs_filter1_20hz_10hz(void){
    static uint16_t block_counter = 0;
    start_filter_block1 = 0;
    
    fir_compressed(PLANALTA_FS_20HZ_10HZ_F1_OUTPUT_SIZE,
        fs_fo1_buffer_write,
        fs_fo1_buffer_read,
        &planalta_fs_filters_1,
        PLANALTA_FS_20HZ_10HZ_F1_DEC_FACT);
    
    block_counter += PLANALTA_FS_20HZ_10HZ_F1_OUTPUT_SIZE;
    fs_fo1_buffer_write += PLANALTA_FS_20HZ_10HZ_F1_OUTPUT_SIZE;
    
    if(block_counter == PLANALTA_FS_20HZ_10HZ_F2_INPUT_SIZE){
        start_filter_block2 = 1;
        block_counter = 0;
        
        select_f1_to_f2 ^= 1;
        
        if(select_f1_to_f2){
            fs_fo2_buffer_read = f1_to_f2_buffer_i_b[0];
            fs_fo1_buffer_write = f1_to_f2_buffer_i_a[0];
        } else {
            fs_fo2_buffer_read = f1_to_f2_buffer_i_a[0];
            fs_fo1_buffer_write = f1_to_f2_buffer_i_b[0];
        }
    }
}

void run_fs_filter2_20hz_10hz(void){
    static uint16_t block_counter = 0;
    start_filter_block2 = 0;

    fir_compressed(PLANALTA_FS_20HZ_10HZ_F2_OUTPUT_SIZE,
            sample_buffer,
            fs_fo2_buffer_read,
            &planalta_fs_filters_2,
            PLANALTA_FS_20HZ_10HZ_F2_DEC_FACT);
    
    fir_compressed(PLANALTA_FS_20HZ_10HZ_F3_OUTPUT_SIZE,
            fs_fo2_buffer_write,
            sample_buffer,
            &planalta_fs_filters_3,
            PLANALTA_FS_20HZ_10HZ_F3_DEC_FACT);
    
    block_counter += PLANALTA_FS_20HZ_10HZ_F3_OUTPUT_SIZE;
    fs_fo2_buffer_write += PLANALTA_FS_20HZ_10HZ_F3_OUTPUT_SIZE;
    
    if(block_counter == final_input_size){
        start_filter_block3 = 1;
        block_counter = 0;
        
        select_f2_to_f3 ^= 1;
        
        if(select_f2_to_f3){
            fs_fo3_buffer_read = f4_to_f5_buffer_i_b[1];
            fs_fo2_buffer_write = f4_to_f5_buffer_i_a[1];
        } else {
            fs_fo3_buffer_read = f4_to_f5_buffer_i_a[1];
            fs_fo2_buffer_write = f4_to_f5_buffer_i_b[1];
        }
    }
}

void run_fs_filter3_20hz_10hz(void){
    start_filter_block3 = 0;
    
    // mixing operation: 1/2 of output samples are 0 and thus not stored in
    // the buffer to save space
    lia_mixer_no_dc(final_input_size,
        fs_fo3_buffer_read,
        f4_to_f5_buffer_i_a[2],
        f4_to_f5_buffer_q_a[2]);
    
    // FO1
    // due to not storing the zeros, this filter has a modified layout:
    // decimation factor is half of the expected value since the output
    // already got "decimated" by the mixing operation. Additionally, not
    // all coefficients are stored since some will be multiplied with a 0. 
    // Finally, the "actual" decimation factor is half of the expected one,
    // thus requiring the decimation factor to always be greater than or 
    // equal to 2.
    fir_compressed(final_output_size,
            &fs_output_buffer_i,
            f4_to_f5_buffer_i_a[2],
            &planalta_fs_filters_4_i,
            final_decimation_factor);
    fir_compressed(final_output_size,
            &fs_output_buffer_q,
            f4_to_f5_buffer_q_a[2],
            &planalta_fs_filters_4_q,
            final_decimation_factor);
    
    planalta_fs_n_coeffs_written += (final_input_size / 2);
    if(planalta_fs_n_coeffs_written >= planalta_fs_filters_4_q.numCoeffs){
        planalta_fs_all_coeffs_written = true;
    }
}


void planalta_fs_sample_20hz_10hz(planalta_fs_freq_t freq) {
    planalta_fs_all_coeffs_written = false;
    reset_buffers_fs_20hz();
    planalta_fs_n_coeffs_written = 0;
    
    filters_fs_20hz_10hz_init(freq);
    
    while(!planalta_fs_all_coeffs_written){
        i2c1_detect_stop();
        
        if(start_filter_block1){
            run_fs_filter1_20hz_10hz();
        }
        
        i2c1_detect_stop();
        
        if(start_filter_block3){
            run_fs_filter3_20hz_10hz();
            continue;
        }
        
        i2c1_detect_stop();
        
        if(start_filter_block2){
            run_fs_filter2_20hz_10hz();
            continue;
        }
    }
}
