#include <xc.h>
#include <dsp.h>
#include <fir_common.h>
#include <uart.h>
#include "planalta_filters.h"
#include "planalta.h"

extern planalta_config_t gconfig;

extern i2c_message_t i2c_channel_message[PLANALTA_N_ADC_CHANNELS];
extern uint8_t i2c_channel_data[PLANALTA_N_ADC_CHANNELS][PLANALTA_I2C_READ_CH_BUFFER_LENGTH];

void reset_buffers_fs_50khz(void){
    select_f1_to_f2 = 0;
    select_f2_to_f3 = 0;
    select_f3_to_f4 = 0;
    select_f4_to_f5 = 0;
    
    start_filter_block3 = 0;
    start_filter_block4 = 0;
    start_filter_block5 = 0;
        
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

    fs_fo5_buffer_i_read = f4_to_f5_buffer_i_a[0];
    fs_fo5_buffer_q_read = f4_to_f5_buffer_q_a[0];
    fs_fo4_buffer_i_write = f4_to_f5_buffer_i_b[0];
    fs_fo4_buffer_q_write = f4_to_f5_buffer_q_b[0];    
}

void filters_fs_50khz_init(){
    // bandpass filter
    // 200kHz -> 200kHz
    FIRStructInit(&planalta_fs_filters_0,
        N_FIR_COEFFS0,
        fir_coeffs_0,
        COEFFS_IN_DATA,
        delay_buffers_0_0
    );
    FIRDelayInit(&planalta_fs_filters_0);
    
    // special filter after mixer
    // 200kHz (100kHz) -> 20Hz
    FIRStructInit(&planalta_fs_filters_1_i,
        N_FIR_COEFFS1_I,
        fir_coeffs_1_i,
        COEFFS_IN_DATA,
        delay_buffers_1_i_0
    );
    FIRDelayInit(&planalta_fs_filters_1_i);
    
    FIRStructInit(&planalta_fs_filters_1_q,
        N_FIR_COEFFS1_Q,
        fir_coeffs_1_q,
        COEFFS_IN_DATA,
        delay_buffers_1_q_0
    );
    FIRDelayInit(&planalta_fs_filters_1_q);
    
    FIRStructInit(&planalta_fs_filters_2_i,
        N_FIR_COEFFS2,
        fir_coeffs_2,
        COEFFS_IN_DATA,
        delay_buffers_2_i_0
    );
    FIRDelayInit(&planalta_fs_filters_2_i);
    
    FIRStructInit(&planalta_fs_filters_2_q,
        N_FIR_COEFFS2,
        fir_coeffs_2,
        COEFFS_IN_DATA,
        delay_buffers_2_q_0
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
        

    FIRStructInit(&planalta_fs_filters_4_i,
        N_FIR_COEFFS4,
        fir_coeffs_4,
        COEFFS_IN_DATA,
        delay_buffers_4_i_0
    );
    FIRDelayInit(&planalta_fs_filters_4_i);

    FIRStructInit(&planalta_fs_filters_4_q,
        N_FIR_COEFFS4,
        fir_coeffs_4,
        COEFFS_IN_DATA,
        delay_buffers_4_q_0
    );
    FIRDelayInit(&planalta_fs_filters_4_q);
    
    FIRStructInit(&planalta_fs_filters_5_i,
        N_FIR_COEFFS8,
        fir_coeffs_8,
        COEFFS_IN_DATA,
        delay_buffers_4_i_1
    );
    FIRDelayInit(&planalta_fs_filters_5_i);

    FIRStructInit(&planalta_fs_filters_5_q,
        N_FIR_COEFFS8,
        fir_coeffs_8,
        COEFFS_IN_DATA,
        delay_buffers_4_q_1
    );
    FIRDelayInit(&planalta_fs_filters_5_q);
}

void adc_rx_callback_fs_50khz(void){
    static uint16_t copy_counter = 0;
    
    // convert samples to fractional format and split the different channels
    //  from this point onwards, all channels are processed one by one
    /*if (adc_buffer_selector == 0) {
        copy_uint_to_fract(ADC_FS_50KHZ_BUFFER_LENGTH,
                (fractional*) &adc_rx_buffer_a[0],
                conversion_buffer,
                1);
    } else {
        copy_uint_to_fract(ADC_FS_50KHZ_BUFFER_LENGTH,
                (fractional*) &adc_rx_buffer_b[0],
                conversion_buffer,
                1);
    }*/

    // FO0
    fir_compressed(PLANALTA_FS_50KHZ_F0_OUTPUT_SIZE,
        sample_buffer,
        conversion_buffer,
        &planalta_fs_filters_0,
        PLANALTA_FS_50KHZ_F0_DEC_FACT);

    // mixing operation: 1/2 of output samples are 0 and thus not stored in
    // the buffer to save space
    lia_mixer_no_dc(PLANALTA_FS_50KHZ_F0_OUTPUT_SIZE,
        sample_buffer,
        conversion_buffer,
        &conversion_buffer[PLANALTA_FS_50KHZ_F1_INPUT_SIZE]);

    // FO1
    // due to not storing the zeros, this filter has a modified layout:
    // decimation factor is half of the expected value since the output
    // already got "decimated" by the mixing operation. Additionally, not
    // all coefficients are stored since some will be multiplied with a 0. 
    // Finally, the "actual" decimation factor is half of the expected one,
    // thus requiring the decimation factor to always be greater than or 
    // equal to 2.
    fir_compressed(PLANALTA_FS_50KHZ_F1_OUTPUT_SIZE,
            fs_fo1_buffer_i_write,
            conversion_buffer,
            &planalta_fs_filters_1_i,
            PLANALTA_FS_50KHZ_F1_DEC_FACT);
    fir_compressed(PLANALTA_FS_50KHZ_F1_OUTPUT_SIZE,
            fs_fo1_buffer_q_write,
            &conversion_buffer[PLANALTA_FS_50KHZ_F1_INPUT_SIZE],
            &planalta_fs_filters_1_q,
            PLANALTA_FS_50KHZ_F1_DEC_FACT);

    fs_fo1_buffer_i_write += PLANALTA_FS_50KHZ_F1_OUTPUT_SIZE;
    fs_fo1_buffer_q_write += PLANALTA_FS_50KHZ_F1_OUTPUT_SIZE;

    copy_counter += PLANALTA_FS_50KHZ_F1_OUTPUT_SIZE;
    
    if(copy_counter == PLANALTA_FS_50KHZ_F2_INPUT_SIZE){
        start_filter_block2 = 1;
        
        copy_counter = 0;
        
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
    adc_buffer_selector ^= 1;
}

void run_fs_filter2_50khz(void){
    static uint16_t block_counter = 0;
    start_filter_block2 = 0;

    fir_compressed(PLANALTA_FS_50KHZ_F2_OUTPUT_SIZE,
            fs_fo2_buffer_i_write,
            fs_fo2_buffer_i_read,
            &planalta_fs_filters_2_i,
            PLANALTA_FS_50KHZ_F2_DEC_FACT);
    fir_compressed(PLANALTA_FS_50KHZ_F2_OUTPUT_SIZE,
            fs_fo2_buffer_q_write,
            fs_fo2_buffer_q_read,
            &planalta_fs_filters_2_q,
            PLANALTA_FS_50KHZ_F2_DEC_FACT);

    fs_fo2_buffer_i_write += PLANALTA_FS_50KHZ_F2_OUTPUT_SIZE;
    fs_fo2_buffer_q_write += PLANALTA_FS_50KHZ_F2_OUTPUT_SIZE;
    
    block_counter += PLANALTA_FS_50KHZ_F2_OUTPUT_SIZE;
    if(block_counter == PLANALTA_FS_50KHZ_F3_INPUT_SIZE){
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
void run_fs_filter3_50khz(void){
    static uint16_t block_counter = 0;
    start_filter_block3 = 0;

    fir_compressed(PLANALTA_FS_50KHZ_F3_OUTPUT_SIZE,
            &fs_fo3_buffer_i_write[block_counter],
            fs_fo3_buffer_i_read,
            &planalta_fs_filters_3_i,
            PLANALTA_FS_50KHZ_F3_DEC_FACT);

    fir_compressed(PLANALTA_FS_50KHZ_F3_OUTPUT_SIZE,
            &fs_fo3_buffer_q_write[block_counter],
            fs_fo3_buffer_q_read,
            &planalta_fs_filters_3_q,
            PLANALTA_FS_50KHZ_F3_DEC_FACT);
        
    block_counter += PLANALTA_FS_50KHZ_F3_OUTPUT_SIZE;
    if(block_counter == PLANALTA_FS_50KHZ_F4_INPUT_SIZE){
        start_filter_block4 = 1;
        block_counter = 0;
        
        select_f3_to_f4 ^= 1;
        
        if(select_f3_to_f4){
            fs_fo4_buffer_i_read = f3_to_f4_buffer_i_b[0];
            fs_fo4_buffer_q_read = f3_to_f4_buffer_q_b[0];
            fs_fo3_buffer_i_write = f3_to_f4_buffer_i_a[0];
            fs_fo3_buffer_q_write = f3_to_f4_buffer_q_a[0];
        } else {
            fs_fo4_buffer_i_read = f3_to_f4_buffer_i_a[0];
            fs_fo4_buffer_q_read = f3_to_f4_buffer_q_a[0];
            fs_fo3_buffer_i_write = f3_to_f4_buffer_i_b[0];
            fs_fo3_buffer_q_write = f3_to_f4_buffer_q_b[0];
        }
    }
}

void run_fs_filter4_50khz(void){
    static uint16_t block_counter = 0;
    start_filter_block4 = 0;

    fir_compressed(PLANALTA_FS_50KHZ_F4_OUTPUT_SIZE,
            &fs_fo4_buffer_i_write[block_counter],
            fs_fo4_buffer_i_read,
            &planalta_fs_filters_4_i,
            PLANALTA_FS_50KHZ_F4_DEC_FACT);

    fir_compressed(PLANALTA_FS_50KHZ_F4_OUTPUT_SIZE,
            &fs_fo4_buffer_q_write[block_counter],
            fs_fo4_buffer_q_read,
            &planalta_fs_filters_4_q,
            PLANALTA_FS_50KHZ_F4_DEC_FACT);
        
    block_counter += PLANALTA_FS_50KHZ_F4_OUTPUT_SIZE;
    if(block_counter == PLANALTA_FS_50KHZ_F5_INPUT_SIZE){
        block_counter = 0;
        start_filter_block5 = 1;
        
        select_f4_to_f5 ^= 1;
        
        if(select_f4_to_f5){
            fs_fo5_buffer_i_read = f4_to_f5_buffer_i_b[0];
            fs_fo5_buffer_q_read = f4_to_f5_buffer_q_b[0];
            fs_fo4_buffer_i_write = f4_to_f5_buffer_i_a[0];
            fs_fo4_buffer_q_write = f4_to_f5_buffer_q_a[0];
        } else {
            fs_fo5_buffer_i_read = f4_to_f5_buffer_i_a[0];
            fs_fo5_buffer_q_read = f4_to_f5_buffer_q_a[0];
            fs_fo4_buffer_i_write = f4_to_f5_buffer_i_b[0];
            fs_fo4_buffer_q_write = f4_to_f5_buffer_q_b[0];
        }
    }
}


void run_fs_filter5_50khz(void){    
    start_filter_block5 = 0; 

    fir_compressed(PLANALTA_FS_50KHZ_F5_OUTPUT_SIZE,
        &fs_output_buffer_i,
        fs_fo5_buffer_i_read,
        &planalta_fs_filters_5_i,
        PLANALTA_FS_50KHZ_F5_DEC_FACT);

    fir_compressed(PLANALTA_FS_50KHZ_F5_OUTPUT_SIZE,
        &fs_output_buffer_q,
        fs_fo5_buffer_q_read,
        &planalta_fs_filters_5_q,
        PLANALTA_FS_50KHZ_F5_DEC_FACT);

    
    planalta_fs_n_coeffs_written += PLANALTA_FS_50KHZ_F5_INPUT_SIZE;
    if(planalta_fs_n_coeffs_written >= planalta_fs_filters_5_q.numCoeffs){
        planalta_fs_all_coeffs_written = true;
    }
}

void planalta_fs_sample_50khz(void) {
    planalta_fs_all_coeffs_written = false;
    reset_buffers_fs_50khz();
    planalta_fs_n_coeffs_written = 0;
    
    filters_fs_50khz_init();
    
    while(!planalta_fs_all_coeffs_written){
        i2c1_detect_stop();
        
        // ping-pong buffered data of in & out
        if(start_filter_block2){
            run_fs_filter2_50khz();
        }
        
        i2c1_detect_stop();
        
        if(start_filter_block5){
            run_fs_filter5_50khz();
            continue;
        }
        
        i2c1_detect_stop();
        
        if(start_filter_block4){
            run_fs_filter4_50khz();
            continue;
        }
        
        i2c1_detect_stop();
        
        if(start_filter_block3){
            run_fs_filter3_50khz();
            continue;
        }
    }
}
