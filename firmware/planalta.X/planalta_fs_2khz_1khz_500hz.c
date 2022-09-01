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

void reset_buffers_fs_2khz_1khz_500hz(void){
    select_f0_to_f1 = 0;
    select_f1_to_f2 = 0;
    select_f2_to_f3 = 0;
    select_f3_to_f4 = 0;

    start_filter_block1 = 0;
    start_filter_block2 = 0;
    start_filter_block3 = 0;
    start_filter_block4 = 0; 
    
    fs_fo1_buffer_read = f4_to_f5_buffer_i_a[0];
    fs_fo0_buffer_write = f4_to_f5_buffer_i_b[0];
    
    fs_fo2_buffer_i_read = f1_to_f2_buffer_i_a[0];
    fs_fo2_buffer_q_read = f1_to_f2_buffer_q_a[0];
    fs_fo1_buffer_i_write = f1_to_f2_buffer_i_b[0];
    fs_fo1_buffer_q_write = f1_to_f2_buffer_q_b[0];

    fs_fo3_buffer_i_read = f2_to_f3_buffer_i_a[0];
    fs_fo3_buffer_q_read = f2_to_f3_buffer_q_a[0];
    fs_fo2_buffer_i_write = f2_to_f3_buffer_i_b[0];
    fs_fo2_buffer_q_write = f2_to_f3_buffer_q_b[0];

    fs_fo4_buffer_i_read = f3_to_f4_buffer_i_a[0];
    fs_fo4_buffer_q_read = f3_to_f4_buffer_q_a[0];
    fs_fo3_buffer_i_write = f3_to_f4_buffer_i_b[0];
    fs_fo3_buffer_q_write = f3_to_f4_buffer_q_b[0];
}


void filters_fs_2khz_1khz_500hz_init(planalta_fs_freq_t freq){
    // low pass filter
    // 40kHz -> 4kHz
    FIRStructInit(&planalta_fs_filters_0,
        N_FIR_COEFFS10,
        fir_coeffs_10,
        COEFFS_IN_DATA,
        delay_buffers_5_i_0
    );
    FIRDelayInit(&planalta_fs_filters_0);
    
    // bandpass filter
    // 4kHz -> 4kHz
    FIRStructInit(&planalta_fs_filters_1,
        N_FIR_COEFFS0,
        fir_coeffs_0,
        COEFFS_IN_DATA,
        delay_buffers_0_0
    );
    FIRDelayInit(&planalta_fs_filters_1);
    
    // special filter after mixer
    // 4kHz (2kHz) -> 400Hz
    FIRStructInit(&planalta_fs_filters_2_i,
        N_FIR_COEFFS1_I,
        fir_coeffs_1_i,
        COEFFS_IN_DATA,
        delay_buffers_1_i_0
    );
    FIRDelayInit(&planalta_fs_filters_2_i);
    
    FIRStructInit(&planalta_fs_filters_2_q,
        N_FIR_COEFFS1_Q,
        fir_coeffs_1_q,
        COEFFS_IN_DATA,
        delay_buffers_1_q_0
    );
    FIRDelayInit(&planalta_fs_filters_2_q);
    
    // 400Hz -> 40Hz
    FIRStructInit(&planalta_fs_filters_3_i,
        N_FIR_COEFFS3,
        fir_coeffs_3,
        COEFFS_IN_DATA,
        delay_buffers_3_i_0
    );
    FIRDelayInit(&planalta_fs_filters_3_i);
    
    FIRStructInit(&planalta_fs_filters_3_q,
        N_FIR_COEFFS3,
        fir_coeffs_3,
        COEFFS_IN_DATA,
        delay_buffers_3_q_0
    );

    FIRDelayInit(&planalta_fs_filters_3_q);
        
    switch(freq){
        case PLANALTA_FS_FREQ_2KHZ:
            // 40Hz -> 10Hz
            FIRStructInit(&planalta_fs_filters_4_i,
                N_FIR_COEFFS9,
                fir_coeffs_9,
                COEFFS_IN_DATA,
                delay_buffers_5_i_1
            );

            FIRStructInit(&planalta_fs_filters_4_q,
                N_FIR_COEFFS9,
                fir_coeffs_9,
                COEFFS_IN_DATA,
                delay_buffers_5_q_1
            );
            
            final_decimation_factor = PLANALTA_FS_2KHZ_F4_DEC_FACT;
            final_output_size = PLANALTA_FS_2KHZ_F4_OUTPUT_SIZE;
            final_input_size = PLANALTA_FS_2KHZ_F4_INPUT_SIZE;

            break;
        case PLANALTA_FS_FREQ_1KHZ:
            // 40Hz -> 10Hz
            FIRStructInit(&planalta_fs_filters_4_i,
                N_FIR_COEFFS7,
                fir_coeffs_7,
                COEFFS_IN_DATA,
                delay_buffers_5_i_1
            );

            FIRStructInit(&planalta_fs_filters_4_q,
                N_FIR_COEFFS7,
                fir_coeffs_7,
                COEFFS_IN_DATA,
                delay_buffers_5_q_1
            );
            
            final_decimation_factor = PLANALTA_FS_1KHZ_F4_DEC_FACT;
            final_output_size = PLANALTA_FS_1KHZ_F4_OUTPUT_SIZE;
            final_input_size = PLANALTA_FS_1KHZ_F4_INPUT_SIZE;
            
            break;
        case PLANALTA_FS_FREQ_500HZ: 
        default:
            FIRStructInit(&planalta_fs_filters_4_i,
                N_FIR_COEFFS8,
                fir_coeffs_8,
                COEFFS_IN_DATA,
                delay_buffers_5_i_1
            );

            FIRStructInit(&planalta_fs_filters_4_q,
                N_FIR_COEFFS8,
                fir_coeffs_8,
                COEFFS_IN_DATA,
                delay_buffers_5_q_1
            );
            
            final_decimation_factor = PLANALTA_FS_500HZ_F4_DEC_FACT;
            final_output_size = PLANALTA_FS_500HZ_F4_OUTPUT_SIZE;
            final_input_size = PLANALTA_FS_500HZ_F4_INPUT_SIZE;
            
            break;
    }
    
    FIRDelayInit(&planalta_fs_filters_4_i);
    FIRDelayInit(&planalta_fs_filters_4_q);
    
}

void adc_rx_callback_fs_2khz_1khz_500hz(void){
    
    // convert samples to fractional format and split the different channels
    //  from this point onwards, all channels are processed one by one
    /*if (adc_buffer_selector == 0) {
        copy_uint_to_fract(ADC_FS_2KHZ_1KHZ_500HZ_BUFFER_LENGTH,
                (fractional*) &adc_rx_buffer_a[0],
                conversion_buffer,
                1);
    } else {
        copy_uint_to_fract(ADC_FS_2KHZ_1KHZ_500HZ_BUFFER_LENGTH,
                (fractional*) &adc_rx_buffer_b[0],
                conversion_buffer,
                1);
    }*/

    // FO0
    fir_compressed(PLANALTA_FS_2KHZ_1KHZ_500HZ_F0_OUTPUT_SIZE,
        fs_fo0_buffer_write,
        conversion_buffer,
        &planalta_fs_filters_0,
        PLANALTA_FS_2KHZ_1KHZ_500HZ_F0_DEC_FACT);

    select_f0_to_f1 ^= 1;

    if(select_f0_to_f1){
        fs_fo1_buffer_read = f4_to_f5_buffer_i_b[0];
        fs_fo0_buffer_write = f4_to_f5_buffer_i_a[0];
    } else {
        fs_fo1_buffer_read = f4_to_f5_buffer_i_a[0];
        fs_fo0_buffer_write = f4_to_f5_buffer_i_b[0];
    }
    
    start_filter_block1 = 1;
    
    adc_buffer_selector ^= 1;
}

void run_fs_filter_block1_2khz_1khz_500hz(void){
    static uint16_t block_counter = 0;
    start_filter_block1 = 0;
    
    fir_compressed(PLANALTA_FS_2KHZ_1KHZ_500HZ_F1_OUTPUT_SIZE,
        sample_buffer,
        fs_fo1_buffer_read,
        &planalta_fs_filters_1,
        PLANALTA_FS_2KHZ_1KHZ_500HZ_F1_DEC_FACT);
    
    // mixing operation: 1/2 of output samples are 0 and thus not stored in
    // the buffer to save space
    lia_mixer_no_dc(PLANALTA_FS_2KHZ_1KHZ_500HZ_F1_OUTPUT_SIZE,
        sample_buffer,
        &sample_buffer[PLANALTA_FS_2KHZ_1KHZ_500HZ_F1_OUTPUT_SIZE],
        &sample_buffer[2*PLANALTA_FS_2KHZ_1KHZ_500HZ_F1_OUTPUT_SIZE]);
    
    fir_compressed(PLANALTA_FS_2KHZ_1KHZ_500HZ_F2_OUTPUT_SIZE,
            fs_fo1_buffer_i_write,
            &sample_buffer[PLANALTA_FS_2KHZ_1KHZ_500HZ_F1_OUTPUT_SIZE],
            &planalta_fs_filters_2_i,
            PLANALTA_FS_2KHZ_1KHZ_500HZ_F2_DEC_FACT);
    fir_compressed(PLANALTA_FS_2KHZ_1KHZ_500HZ_F2_OUTPUT_SIZE,
            fs_fo1_buffer_q_write,
            &sample_buffer[2*PLANALTA_FS_2KHZ_1KHZ_500HZ_F1_OUTPUT_SIZE],
            &planalta_fs_filters_2_q,
            PLANALTA_FS_2KHZ_1KHZ_500HZ_F2_DEC_FACT);
    
    fs_fo1_buffer_i_write += PLANALTA_FS_2KHZ_1KHZ_500HZ_F2_OUTPUT_SIZE;
    fs_fo1_buffer_q_write += PLANALTA_FS_2KHZ_1KHZ_500HZ_F2_OUTPUT_SIZE;
    
    block_counter += PLANALTA_FS_2KHZ_1KHZ_500HZ_F2_OUTPUT_SIZE;
    if(block_counter == PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_INPUT_SIZE){
        start_filter_block2 = 1;
        block_counter = 0;
        
        select_f1_to_f2 ^= 1;
        
        if(select_f1_to_f2){
            fs_fo2_buffer_i_read = f1_to_f2_buffer_i_b[0];
            fs_fo2_buffer_q_read = f1_to_f2_buffer_q_b[0];
            fs_fo1_buffer_i_write = f1_to_f2_buffer_i_a[0];
            fs_fo1_buffer_q_write = f1_to_f2_buffer_q_a[0];
        } else {
            fs_fo2_buffer_i_read = f1_to_f2_buffer_i_a[0];
            fs_fo2_buffer_q_read = f1_to_f2_buffer_q_a[0];
            fs_fo1_buffer_i_write = f1_to_f2_buffer_i_b[0];
            fs_fo1_buffer_q_write = f1_to_f2_buffer_q_b[0];
        }
    }
}

void run_fs_filter_block2_2khz_1khz_500hz(void){
    static uint16_t block_counter = 0;
    start_filter_block2 = 0;
    
    // FO1
    // due to not storing the zeros, this filter has a modified layout:
    // decimation factor is half of the expected value since the output
    // already got "decimated" by the mixing operation. Additionally, not
    // all coefficients are stored since some will be multiplied with a 0. 
    // Finally, the "actual" decimation factor is half of the expected one,
    // thus requiring the decimation factor to always be greater than or 
    // equal to 2.
    fir_compressed(PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_OUTPUT_SIZE,
            fs_fo2_buffer_i_write,
            fs_fo2_buffer_i_read,
            &planalta_fs_filters_3_i,
            PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_DEC_FACT);
    fir_compressed(PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_OUTPUT_SIZE,
            fs_fo2_buffer_q_write,
            fs_fo2_buffer_q_read,
            &planalta_fs_filters_3_q,
            PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_DEC_FACT);

    fs_fo2_buffer_i_write += PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_OUTPUT_SIZE;
    fs_fo2_buffer_q_write += PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_OUTPUT_SIZE;
    
    block_counter += PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_OUTPUT_SIZE;
    if(block_counter == final_input_size){
        start_filter_block3 = 1;
        block_counter = 0;
        
        select_f2_to_f3 ^= 1;
        
        if(select_f2_to_f3){
            fs_fo3_buffer_i_read = f2_to_f3_buffer_i_b[0];
            fs_fo3_buffer_q_read = f2_to_f3_buffer_q_b[0];
            fs_fo2_buffer_i_write = f2_to_f3_buffer_i_a[0];
            fs_fo2_buffer_q_write = f2_to_f3_buffer_q_a[0];
        } else {
            fs_fo3_buffer_i_read = f2_to_f3_buffer_i_a[0];
            fs_fo3_buffer_q_read = f2_to_f3_buffer_q_a[0];
            fs_fo2_buffer_i_write = f2_to_f3_buffer_i_b[0];
            fs_fo2_buffer_q_write = f2_to_f3_buffer_q_b[0];
        }
    }
}


void run_fs_filter_block3_2khz_1khz_500hz(void){
    start_filter_block3 = 0;

    fir_compressed(final_output_size,
            &fs_output_buffer_i,
            fs_fo3_buffer_i_read,
            &planalta_fs_filters_4_i,
            final_decimation_factor);

    fir_compressed(final_output_size,
            &fs_output_buffer_q,
            fs_fo3_buffer_q_read,
            &planalta_fs_filters_4_q,
            final_decimation_factor);
        
    planalta_fs_n_coeffs_written += final_decimation_factor;
    if(planalta_fs_n_coeffs_written >= planalta_fs_filters_4_q.numCoeffs){
        planalta_fs_all_coeffs_written = true;
    }  
}


void planalta_fs_sample_2khz_1khz_500hz(planalta_fs_freq_t freq) {
    planalta_fs_all_coeffs_written = false;
    reset_buffers_fs_2khz_1khz_500hz();
    planalta_fs_n_coeffs_written = 0;
    
    filters_fs_2khz_1khz_500hz_init(freq);
    
    while(!planalta_fs_all_coeffs_written){
        i2c1_detect_stop();
        
        // ping-pong buffered data of in & out
        if(start_filter_block1){
            run_fs_filter_block1_2khz_1khz_500hz();
        }
        
        i2c1_detect_stop();
        
        if(start_filter_block3){
            run_fs_filter_block3_2khz_1khz_500hz();
            continue;
        }
        
        i2c1_detect_stop();
        
        if(start_filter_block2){
            run_fs_filter_block2_2khz_1khz_500hz();
            continue;
        }
    }
}
