#include "planalta_filters.h"
#include <fir_common.h>
#include <dsp.h>
#include "planalta.h"
#include <utilities.h>
#include <uart.h>

extern planalta_config_t gconfig;


void init_filters_5khz_lia(void){
    uint16_t i;

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        FIRStructInit(&planalta_lia_filters_0[i],
            N_FIR_COEFFS0,
            fir_coeffs_0,
            COEFFS_IN_DATA,
            delay_buffers_0[i]
        );

        FIRDelayInit(&planalta_lia_filters_0[i]);
    }

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        FIRStructInit(&planalta_lia_filters_1_q[i],
            N_FIR_COEFFS1_Q,
            fir_coeffs_1_q,
            COEFFS_IN_DATA,
            delay_buffers_1_q[i]
        );

        FIRDelayInit(&planalta_lia_filters_1_q[i]);
    }

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        FIRStructInit(&planalta_lia_filters_1_i[i],
            N_FIR_COEFFS1_I,
            fir_coeffs_1_i,
            COEFFS_IN_DATA,
            delay_buffers_1_i[i]
        );

        FIRDelayInit(&planalta_lia_filters_1_i[i]);
    }

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        FIRStructInit(&planalta_lia_filters_2_q[i],
            N_FIR_COEFFS2,
            fir_coeffs_2,
            COEFFS_IN_DATA,
            delay_buffers_2_q[i]
        );

        FIRDelayInit(&planalta_lia_filters_2_q[i]);
    }

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        FIRStructInit(&planalta_lia_filters_2_i[i],
            N_FIR_COEFFS2,
            fir_coeffs_2,
            COEFFS_IN_DATA,
            delay_buffers_2_i[i]
        );

        FIRDelayInit(&planalta_lia_filters_2_i[i]);
    }

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        FIRStructInit(&planalta_lia_filters_3_q[i],
            N_FIR_COEFFS3,
            fir_coeffs_3,
            COEFFS_IN_DATA,
            delay_buffers_3_q[i]
        );

        FIRDelayInit(&planalta_lia_filters_3_q[i]);
    }

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        FIRStructInit(&planalta_lia_filters_3_i[i],
            N_FIR_COEFFS3,
            fir_coeffs_3,
            COEFFS_IN_DATA,
            delay_buffers_3_i[i]
        );

        FIRDelayInit(&planalta_lia_filters_3_i[i]);
    }

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        FIRStructInit(&planalta_lia_filters_4_q[i],
            N_FIR_COEFFS4,
            fir_coeffs_4,
            COEFFS_IN_DATA,
            delay_buffers_4_q[i]
        );

        FIRDelayInit(&planalta_lia_filters_4_q[i]);
    }

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        FIRStructInit(&planalta_lia_filters_4_i[i],
            N_FIR_COEFFS4,
            fir_coeffs_4,
            COEFFS_IN_DATA,
            delay_buffers_4_i[i]
        );

        FIRDelayInit(&planalta_lia_filters_4_i[i]);
    }

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        FIRStructInit(&planalta_lia_filters_5_q[i],
            N_FIR_COEFFS8,
            fir_coeffs_8,
            COEFFS_IN_DATA,
            delay_buffers_8_q[i]
        );

        FIRDelayInit(&planalta_lia_filters_5_q[i]);
    }

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        FIRStructInit(&planalta_lia_filters_5_i[i],
            N_FIR_COEFFS8,
            fir_coeffs_8,
            COEFFS_IN_DATA,
            delay_buffers_8_i[i]
        );

        FIRDelayInit(&planalta_lia_filters_5_i[i]);
    }
}

void adc_rx_callback_5khz(void){
    uint16_t i = 0;
    static uint16_t copy_counter = 0;
    
    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        // convert samples to fractional format and split the different channels
        //  from this point onwards, all channels are processed one by one
        /*if (adc_buffer_selector == 0) {
            for(j = 0; j < PLANALTA_5KHZ_F0_OUTPUT_SIZE; j++){
                conversion_buffer[j] = adc_rx_buffer_a[i+j*PLANALTA_5KHZ_N_ADC_CHANNELS] - 0x8000;
            }
        } else {
            for(j = 0; j < PLANALTA_5KHZ_F0_OUTPUT_SIZE; j++){
                conversion_buffer[j] = adc_rx_buffer_b[i+j*PLANALTA_5KHZ_N_ADC_CHANNELS] - 0x8000;
            }
        }*/
        
        if (adc_buffer_selector == 0) {
            copy_uint_to_fract(PLANALTA_5KHZ_F0_OUTPUT_SIZE,
                    (fractional*) &adc_rx_buffer_a[i],
                    conversion_buffer,
                    PLANALTA_5KHZ_N_ADC_CHANNELS);
        } else {
            copy_uint_to_fract(PLANALTA_5KHZ_F0_OUTPUT_SIZE,
                    (fractional*) &adc_rx_buffer_b[i],
                    conversion_buffer,
                    PLANALTA_5KHZ_N_ADC_CHANNELS);
        }
        
        // FO0
        fir_compressed(PLANALTA_5KHZ_F0_OUTPUT_SIZE,
            sample_buffer,
            conversion_buffer,
            &planalta_lia_filters_0[i],
            PLANALTA_5KHZ_DEC_FACT_F0);

        // mixing operation: 1/2 of output samples are 0 and thus not stored in
        // the buffer to save space
        lia_mixer_no_dc(PLANALTA_5KHZ_F0_OUTPUT_SIZE,
            sample_buffer,
            conversion_buffer,
            &conversion_buffer[PLANALTA_5KHZ_F1_INPUT_SIZE]);

        // FO1
        // due to not storing the zeros, this filter has a modified layout:
        // decimation factor is half of the expected value since the output
        // already got "decimated" by the mixing operation. Additionally, not
        // all coefficients are stored since some will be multiplied with a 0. 
        // Finally, the "actual" decimation factor is half of the expected one,
        // thus requiring the decimation factor to always be greater than or 
        // equal to 2.
        fir_compressed(PLANALTA_5KHZ_F1_OUTPUT_SIZE,
                fo1_buffer_i_write[i],
                conversion_buffer,
                &planalta_lia_filters_1_i[i],
                PLANALTA_5KHZ_DEC_FACT_F1);
        fir_compressed(PLANALTA_5KHZ_F1_OUTPUT_SIZE,
                fo1_buffer_q_write[i],
                &conversion_buffer[PLANALTA_5KHZ_F1_INPUT_SIZE],
                &planalta_lia_filters_1_q[i],
                PLANALTA_5KHZ_DEC_FACT_F1);
        
        fo1_buffer_i_write[i] += PLANALTA_5KHZ_F1_OUTPUT_SIZE;
        fo1_buffer_q_write[i] += PLANALTA_5KHZ_F1_OUTPUT_SIZE;
    }

    copy_counter += PLANALTA_5KHZ_F1_OUTPUT_SIZE;
    
    if(copy_counter == PLANALTA_5KHZ_F2_INPUT_SIZE){
        start_filter_block2 = 1;
        
        copy_counter = 0;
        
        select_f1_to_f2 ^= 1;
        
        if(select_f1_to_f2){
            for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
                fo2_buffer_i_read[i] = f1_to_f2_buffer_i_b[i];
                fo2_buffer_q_read[i] = f1_to_f2_buffer_q_b[i];
                fo1_buffer_i_write[i] = f1_to_f2_buffer_i_a[i];
                fo1_buffer_q_write[i] = f1_to_f2_buffer_q_a[i];
            }
        } else {
            for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
                fo2_buffer_i_read[i] = f1_to_f2_buffer_i_a[i];
                fo2_buffer_q_read[i] = f1_to_f2_buffer_q_a[i];
                fo1_buffer_i_write[i] = f1_to_f2_buffer_i_b[i];
                fo1_buffer_q_write[i] = f1_to_f2_buffer_q_b[i];
            }
        }
    }
    adc_buffer_selector ^= 1;
}

void run_filter2_5khz(void){
    uint16_t i;
    static uint16_t block_counter = 0;
    start_filter_block2 = 0;

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        fir_compressed(PLANALTA_5KHZ_F2_OUTPUT_SIZE,
                fo2_buffer_i_write[i],
                fo2_buffer_i_read[i],
                &planalta_lia_filters_2_i[i],
                PLANALTA_5KHZ_DEC_FACT_F2);
        fir_compressed(PLANALTA_5KHZ_F2_OUTPUT_SIZE,
                fo2_buffer_q_write[i],
                fo2_buffer_q_read[i],
                &planalta_lia_filters_2_q[i],
                PLANALTA_5KHZ_DEC_FACT_F2);
        
        fo2_buffer_i_write[i] += PLANALTA_5KHZ_F2_OUTPUT_SIZE;
        fo2_buffer_q_write[i] += PLANALTA_5KHZ_F2_OUTPUT_SIZE;
    }
    
    block_counter += PLANALTA_5KHZ_F2_OUTPUT_SIZE;
    if(block_counter == PLANALTA_5KHZ_F3_INPUT_SIZE){
        start_filter_block3 = 1;
        block_counter = 0;
        
        select_f2_to_f3 ^= 1;
        
        if(select_f2_to_f3){
            for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
                fo3_buffer_i_read[i] = f2_to_f3_buffer_i_b[i];
                fo3_buffer_q_read[i] = f2_to_f3_buffer_q_b[i];
                fo2_buffer_i_write[i] = f2_to_f3_buffer_i_a[i];
                fo2_buffer_q_write[i] = f2_to_f3_buffer_q_a[i];
            }
        } else {
            for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
                fo3_buffer_i_read[i] = f2_to_f3_buffer_i_a[i];
                fo3_buffer_q_read[i] = f2_to_f3_buffer_q_a[i];
                fo2_buffer_i_write[i] = f2_to_f3_buffer_i_b[i];
                fo2_buffer_q_write[i] = f2_to_f3_buffer_q_b[i];
            }
        }
    }
}
void run_filter3_5khz(void){
    uint16_t i;
    static uint16_t block_counter = 0;
    start_filter_block3 = 0;

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        fir_compressed(PLANALTA_5KHZ_F3_OUTPUT_SIZE,
                fo3_buffer_i_write[i],
                fo3_buffer_i_read[i],
                &planalta_lia_filters_3_i[i],
                PLANALTA_5KHZ_DEC_FACT_F3);

        fir_compressed(PLANALTA_5KHZ_F3_OUTPUT_SIZE,
                fo3_buffer_q_write[i],
                fo3_buffer_q_read[i],
                &planalta_lia_filters_3_q[i],
                PLANALTA_5KHZ_DEC_FACT_F3);
        
        fo3_buffer_i_write[i] += PLANALTA_5KHZ_F3_OUTPUT_SIZE;
        fo3_buffer_q_write[i] += PLANALTA_5KHZ_F3_OUTPUT_SIZE;
    }
    
    block_counter += PLANALTA_5KHZ_F3_OUTPUT_SIZE;
    
    if(block_counter == PLANALTA_5KHZ_F4_INPUT_SIZE){
        start_filter_block4 = 1;
        block_counter = 0;
        
        select_f3_to_f4 ^= 1;
        
        if(select_f3_to_f4){
            for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
                fo4_buffer_i_read[i] = f3_to_f4_buffer_i_b[i];
                fo4_buffer_q_read[i] = f3_to_f4_buffer_q_b[i];
                fo3_buffer_i_write[i] = f3_to_f4_buffer_i_a[i];
                fo3_buffer_q_write[i] = f3_to_f4_buffer_q_a[i];
            }
        } else {
            for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
                fo4_buffer_i_read[i] = f3_to_f4_buffer_i_a[i];
                fo4_buffer_q_read[i] = f3_to_f4_buffer_q_a[i];
                fo3_buffer_i_write[i] = f3_to_f4_buffer_i_b[i];
                fo3_buffer_q_write[i] = f3_to_f4_buffer_q_b[i];
            }
        }
    }
}

void run_filter4_5khz(void){
    uint16_t i;
    static uint16_t block_counter = 0;
    start_filter_block4 = 0;

    for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
        fir_compressed(PLANALTA_5KHZ_F4_OUTPUT_SIZE,
                fo4_buffer_i_write[i],
                fo4_buffer_i_read[i],
                &planalta_lia_filters_4_i[i],
                PLANALTA_5KHZ_DEC_FACT_F4);

        fir_compressed(PLANALTA_5KHZ_F4_OUTPUT_SIZE,
                fo4_buffer_q_write[i],
                fo4_buffer_q_read[i],
                &planalta_lia_filters_4_q[i],
                PLANALTA_5KHZ_DEC_FACT_F4);
        
        fo4_buffer_i_write[i] += PLANALTA_5KHZ_F4_OUTPUT_SIZE;
        fo4_buffer_q_write[i] += PLANALTA_5KHZ_F4_OUTPUT_SIZE;
    }
    
    block_counter += PLANALTA_5KHZ_F4_OUTPUT_SIZE;
    
    if(block_counter == PLANALTA_5KHZ_F5_INPUT_SIZE){
        block_counter = 0;
        start_filter_block5 = 1;
        
        select_f4_to_f5 ^= 1;
        
        if(select_f4_to_f5){
            for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
                fo5_buffer_i_read[i] = f4_to_f5_buffer_i_b[i];
                fo5_buffer_q_read[i] = f4_to_f5_buffer_q_b[i];
                fo4_buffer_i_write[i] = f4_to_f5_buffer_i_a[i];
                fo4_buffer_q_write[i] = f4_to_f5_buffer_q_a[i];
            }
        } else {
            for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
                fo5_buffer_i_read[i] = f4_to_f5_buffer_i_a[i];
                fo5_buffer_q_read[i] = f4_to_f5_buffer_q_a[i];
                fo4_buffer_i_write[i] = f4_to_f5_buffer_i_b[i];
                fo4_buffer_q_write[i] = f4_to_f5_buffer_q_b[i];
            }
        }
    }
}


void run_filter5_5khz(void){
    uint16_t i;
    start_filter_block5 = 0; 
    
    if(planalta_lia_obuffer_selector) {
        for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
            fir_compressed(PLANALTA_5KHZ_F5_OUTPUT_SIZE,
                    &planalta_lia_obuffer_a_i[i],
                    fo5_buffer_i_read[i],
                    &planalta_lia_filters_5_i[i],
                    PLANALTA_5KHZ_DEC_FACT_F5);

            fir_compressed(PLANALTA_5KHZ_F5_OUTPUT_SIZE,
                    &planalta_lia_obuffer_a_q[i],
                    fo5_buffer_q_read[i],
                    &planalta_lia_filters_5_q[i],
                    PLANALTA_5KHZ_DEC_FACT_F5);
        }
    } else {
        for(i = 0; i < PLANALTA_5KHZ_N_ADC_CHANNELS; i++){
            fir_compressed(PLANALTA_5KHZ_F5_OUTPUT_SIZE,
                    &planalta_lia_obuffer_b_i[i],
                    fo5_buffer_i_read[i],
                    &planalta_lia_filters_5_i[i],
                    PLANALTA_5KHZ_DEC_FACT_F5);

            fir_compressed(PLANALTA_5KHZ_F5_OUTPUT_SIZE,
                    &planalta_lia_obuffer_b_q[i],
                    fo5_buffer_q_read[i],
                    &planalta_lia_filters_5_q[i],
                    PLANALTA_5KHZ_DEC_FACT_F5);
        };
    }
    
    ///_SI2C1IE = 0;
    planalta_lia_obuffer_selector ^= 1;
    //_SI2C1IE = 1;
    
    CLEAR_PORT_BIT(gconfig.int_pin);    
}

void planalta_init_i2c_messages(void){
    planalta_lia_obuffer_selector = 0;
        
    i2c_init_read_message(
        &i2c_mr_message,
        i2c_mr_message_data,
        0);
}

void planalta_filter_5khz(void) {
    size_t i;
    
#ifdef ENABLE_DEBUG
    sprintf(print_buffer, "LIA at 5kHz activated.");
    uart_print(print_buffer, strlen(print_buffer));
#endif
    
    // enable PGA
    //_RF1 = 1;
    //_RD11 = 1;
    
    gconfig.adc_config.channel_select = ADC_CHANNEL_SELECT_MODE_AUTO;
    gconfig.adc_config.channel = ADC_CH0;
    gconfig.adc_config.sample_frequency = 5000UL*4*8;
    gconfig.adc_config.rx_callback = adc_rx_callback_5khz;
    gconfig.adc_config.adc_buffer_size = ADC_5KHZ_BUFFER_LENGTH;
    
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
    
    planalta_init_i2c_messages();
    
    for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i++){
        sprintf(print_buffer, "Channel %x status: %x", i, gconfig.channel_status[i]);
        uart_print(print_buffer, strlen(print_buffer));
        delay_ms(10);
    }
    
    while(gconfig.status == PLANALTA_STATUS_RUNNING){
        i2c1_detect_stop();
        
        // ping-pong buffered data of in & out
        if(start_filter_block2){
           run_filter2_5khz();
        }
        
        i2c1_detect_stop();
        
        if(start_filter_block5){
            run_filter5_5khz();
            continue;
        }
        

        i2c1_detect_stop();
        
        if(start_filter_block4){
            run_filter4_5khz();
            continue;
        }
        
        i2c1_detect_stop();
        
        if(start_filter_block3){
            run_filter3_5khz();
            continue;
        }
    }
    
    adc_stop(&gconfig.adc_config);
    stop_dac();
    
    // disable PGA
    //_RF1 = 0;
    //_RD11 = 0;
}

