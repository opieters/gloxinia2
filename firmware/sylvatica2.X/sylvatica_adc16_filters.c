#include "sylvatica.h"
#include "address.h"
#include <can.h>
#include <sensor.h>
#include <spi.h>
#include <pga.h>
#include <dsp.h>
#include <sensor_adc16.h>
#include <fir_common.h>
#include <libpic30.h>
//#include <event_controller.h>

FIRStruct filters_0[SYLVATICA_N_CHANNELS];
FIRStruct filters_1[SYLVATICA_N_CHANNELS];
FIRStruct filters_2[SYLVATICA_N_CHANNELS];
FIRStruct filters_3[SYLVATICA_N_CHANNELS];

fractional __attribute__((space(ymemory), aligned(256), eds)) delay_buffers_0[SYLVATICA_N_CHANNELS][100];
fractional __attribute__((space(ymemory), aligned(256), eds)) delay_buffers_1[SYLVATICA_N_CHANNELS][100];
fractional __attribute__((space(ymemory), aligned(256), eds)) delay_buffers_2[SYLVATICA_N_CHANNELS][100];
fractional __attribute__((space(ymemory), aligned(512), eds)) delay_buffers_3[SYLVATICA_N_CHANNELS][192];

uint16_t copy_buffers_a[SYLVATICA_N_CHANNELS][SYLVATICA_COPY_BUFFER_SIZE];
uint16_t copy_buffers_b[SYLVATICA_N_CHANNELS][SYLVATICA_COPY_BUFFER_SIZE];
fractional* output_block0_buffer[SYLVATICA_N_CHANNELS];
fractional* output_block1_buffer[SYLVATICA_N_CHANNELS];
fractional* output_block2_buffer[SYLVATICA_N_CHANNELS];
fractional* input_block1_buffer[SYLVATICA_N_CHANNELS];
fractional* input_block2_buffer[SYLVATICA_N_CHANNELS];
fractional* input_block3_buffer[SYLVATICA_N_CHANNELS];
uint8_t output_buffer0_select = 0, output_buffer1_select = 0, output_buffer2_select = 0;
fractional output_block0_buffers_a[SYLVATICA_N_CHANNELS][SYLVATICA_BLOCK1_INPUT_SIZE];
fractional output_block1_buffers_a[SYLVATICA_N_CHANNELS][SYLVATICA_BLOCK2_INPUT_SIZE];
fractional output_block2_buffers_a[SYLVATICA_N_CHANNELS][SYLVATICA_BLOCK3_INPUT_SIZE];
fractional output_block0_buffers_b[SYLVATICA_N_CHANNELS][SYLVATICA_BLOCK1_INPUT_SIZE];
fractional output_block1_buffers_b[SYLVATICA_N_CHANNELS][SYLVATICA_BLOCK2_INPUT_SIZE];
fractional output_block2_buffers_b[SYLVATICA_N_CHANNELS][SYLVATICA_BLOCK3_INPUT_SIZE];

fractional output_buffer_a[SYLVATICA_N_CHANNELS];

uint8_t adc16_buffer_selector = 0;
volatile uint8_t copy_buffer_selector = 0;

void sylvatica_filters_init(void)
{
    uint16_t i;

    for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
        FIRStructInit(&filters_0[i],
            N_FIR_COEFFS0,
            fir_coeffs_0,
            COEFFS_IN_DATA,
            delay_buffers_0[i]
        );

        FIRDelayInit(&filters_0[i]);
    }

    for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
        FIRStructInit(&filters_1[i],
            N_FIR_COEFFS1,
            fir_coeffs_1,
            COEFFS_IN_DATA,
            delay_buffers_1[i]
        );

        FIRDelayInit(&filters_1[i]);
    }

    for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
        FIRStructInit(&filters_2[i],
            N_FIR_COEFFS2,
            fir_coeffs_2,
            COEFFS_IN_DATA,
            delay_buffers_2[i]
        );

        FIRDelayInit(&filters_2[i]);
    }

    for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
        FIRStructInit(&filters_3[i],
            N_FIR_COEFFS3,
            fir_coeffs_3,
            COEFFS_IN_DATA,
            delay_buffers_3[i]
        );

        FIRDelayInit(&filters_3[i]);
    }

}


void sylvatic_adc16_callback(void){
    uint16_t i;
    static uint16_t copy_counter = 0;
    
    if (adc16_buffer_selector == 0) {
        if(copy_buffer_selector == 0){
            for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
                copy_adc_data(SYLVATICA_ADC16_BUFFER_LENGTH / SYLVATICA_N_CHANNELS, (fractional*) &copy_buffers_a[i][copy_counter], (fractional*) &adc16_rx_buffer_a[i]);
            }
        } else {
            for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
                copy_adc_data(SYLVATICA_ADC16_BUFFER_LENGTH / SYLVATICA_N_CHANNELS, (fractional*) &copy_buffers_b[i][copy_counter], (fractional*) &adc16_rx_buffer_a[i]);
            }
        }
    } else {
        if(copy_buffer_selector == 0){
            for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
                copy_adc_data(SYLVATICA_ADC16_BUFFER_LENGTH / SYLVATICA_N_CHANNELS, (fractional*) &copy_buffers_a[i][copy_counter], (fractional*) &adc16_rx_buffer_b[i]);
            }
        } else {
            for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
                copy_adc_data(SYLVATICA_ADC16_BUFFER_LENGTH / SYLVATICA_N_CHANNELS, (fractional*) &copy_buffers_b[i][copy_counter], (fractional*) &adc16_rx_buffer_b[i]);
            }
        }
    }
    copy_counter += SYLVATICA_ADC16_BUFFER_LENGTH / SYLVATICA_N_CHANNELS;
    
    if(copy_counter == SYLVATICA_COPY_BUFFER_SIZE){
        task_t task = {process_filter_block0, NULL};
        push_queued_task(task);
        copy_buffer_selector ^= 1;
        copy_counter = 0;
    }
    
    adc16_buffer_selector ^= 1;
}


void process_filter_block0(void* data)
{
    uint16_t i;
    static uint16_t block_counter = 0;
    static uint16_t sample_counter = 0;
    fractional sample_buffer[SYLVATICA_COPY_BUFFER_SIZE];
    
    sample_counter += SYLVATICA_COPY_BUFFER_SIZE;
    
    for(i = 0; i < SYLVATICA_N_CHANNELS; i++){   
        if(copy_buffer_selector == 0){
            covert_uint_to_fract(SYLVATICA_COPY_BUFFER_SIZE, copy_buffers_b[i], sample_buffer);
        } else {
            covert_uint_to_fract(SYLVATICA_COPY_BUFFER_SIZE, copy_buffers_a[i], sample_buffer);
        }
            
        FIRDecimate(SYLVATICA_BLOCK0_OUTPUT_SIZE, 
                &output_block0_buffer[i][block_counter], 
                sample_buffer, 
                &filters_0[i], 
                SYLVATICA_DEC_FACT_F0);
    }
    
    if(sample_counter == 10000){
        sample_counter = 0;
    }
    
    block_counter += SYLVATICA_BLOCK0_OUTPUT_SIZE;
    if(block_counter == SYLVATICA_BLOCK1_INPUT_SIZE){
        task_t task = {process_filter_block1, NULL};
        push_queued_task(task);
        block_counter = 0;
        
        output_buffer0_select ^= 1;
        if(output_buffer0_select == 0){
            for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
                output_block0_buffer[i] = output_block0_buffers_b[i];
                input_block1_buffer[i] = output_block0_buffers_a[i];
            }
        } else {
            for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
                output_block0_buffer[i] = output_block0_buffers_a[i];
                input_block1_buffer[i] = output_block0_buffers_b[i];
            }
        }
    }
}

void process_filter_block1(void* data){
    uint16_t i;
    static uint16_t block_counter = 0;
    
    for(i = 0; i < SYLVATICA_N_CHANNELS; i++){   
            
        FIRDecimate(SYLVATICA_BLOCK1_OUTPUT_SIZE, 
                &output_block1_buffer[i][block_counter], 
                input_block1_buffer[i], 
                &filters_1[i], 
                SYLVATICA_DEC_FACT_F1);
    }
    block_counter += SYLVATICA_BLOCK1_OUTPUT_SIZE;
    if(block_counter == SYLVATICA_BLOCK2_INPUT_SIZE){
        task_t task = {process_filter_block2, NULL};
        push_queued_task(task);
        block_counter = 0;
        
        output_buffer1_select ^= 1;
        if(output_buffer1_select == 0){
            for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
                output_block1_buffer[i] = output_block1_buffers_b[i];
                input_block2_buffer[i] = output_block1_buffers_a[i];
            }
        } else {
            for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
                output_block1_buffer[i] = output_block1_buffers_a[i];
                input_block2_buffer[i] = output_block1_buffers_b[i];
            }
        }
        
    }
}

void process_filter_block2(void* data){
    uint16_t i;
    static uint16_t block_counter = 0;
    
    for(i = 0; i < SYLVATICA_N_CHANNELS; i++){   
        FIRDecimate(SYLVATICA_BLOCK2_OUTPUT_SIZE, 
                &output_block2_buffer[i][block_counter], 
                input_block2_buffer[i], 
                &filters_2[i], 
                SYLVATICA_DEC_FACT_F2);
    }
    block_counter += SYLVATICA_BLOCK2_OUTPUT_SIZE;
    if(block_counter == SYLVATICA_BLOCK3_INPUT_SIZE){
        block_counter = 0;
        task_t task = {process_filter_block3, NULL};
        push_queued_task(task);
        
        output_buffer2_select ^= 1;
        if(output_buffer2_select == 0){
            for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
                output_block2_buffer[i] = output_block2_buffers_b[i];
                input_block3_buffer[i] = output_block2_buffers_a[i];
            }
        } else {
            for(i = 0; i < SYLVATICA_N_CHANNELS; i++){
                output_block2_buffer[i] = output_block2_buffers_a[i];
                input_block3_buffer[i] = output_block2_buffers_b[i];
            }
        }
        
    }
}


void process_filter_block3(void* data)
{
    uint16_t i;
    
    for(i = 0; i < SYLVATICA_N_CHANNELS; i++)
    {   
        FIRDecimate(SYLVATICA_BLOCK3_OUTPUT_SIZE, 
                &output_buffer_a[i], 
                input_block3_buffer[i], 
                &filters_3[i], 
                SYLVATICA_DEC_FACT_F3);
       
        // the interface/sensor alloc guarantees that it's always on this location
        sensor_interfaces[i]->gsensor_config[0].sensor_config.adc16.result_ch += output_buffer_a[i];
        sensor_interfaces[i]->gsensor_config[0].sensor_config.adc16.count++;
    }
}
