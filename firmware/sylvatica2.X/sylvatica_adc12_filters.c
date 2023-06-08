#include <dsp.h>
#include <sensor_adc12.h>
#include "sylvatica.h"
#include <event_controller.h>
#include <filtering.h>
#include <fir_common.h>
#include <sensor.h>


fractional __attribute__((space(ymemory), aligned(256), eds)) sensor_adc12_delay_buffers_0[ADC12_N_CHANNELS][100];
fractional __attribute__((space(ymemory), aligned(256), eds)) sensor_adc12_delay_buffers_1[ADC12_N_CHANNELS][100];
fractional __attribute__((space(ymemory), aligned(256), eds)) sensor_adc12_delay_buffers_2[ADC12_N_CHANNELS][100];
fractional __attribute__((space(ymemory), aligned(256), eds)) sensor_adc12_delay_buffers_3[ADC12_N_CHANNELS][128];

FIRStruct sensor_adc12_filters_0[ADC12_N_CHANNELS];
FIRStruct sensor_adc12_filters_1[ADC12_N_CHANNELS];
FIRStruct sensor_adc12_filters_2[ADC12_N_CHANNELS];
FIRStruct sensor_adc12_filters_3[ADC12_N_CHANNELS];

fractional* adc12_output_block0_buffer[ADC12_N_CHANNELS];
fractional* adc12_input_block1_buffer[ADC12_N_CHANNELS];
uint8_t adc12_output_buffer0_select = 0;
fractional adc12_output_block0_buffers_a[ADC12_N_CHANNELS][SENSOR_ADC12_BLOCK1_INPUT_SIZE];
fractional adc12_output_block1_buffers[ADC12_N_CHANNELS][SENSOR_ADC12_BLOCK2_INPUT_SIZE];
fractional adc12_output_block2_buffers[ADC12_N_CHANNELS][SENSOR_ADC12_BLOCK3_INPUT_SIZE];
fractional adc12_output_block0_buffers_b[ADC12_N_CHANNELS][SENSOR_ADC12_BLOCK1_INPUT_SIZE];

extern sensor_interface_t* sensor_interfaces[];
extern const uint8_t n_sensor_interfaces;

fractional  __attribute__((space(xmemory), aligned(256), eds)) sensor_adc12_fir_coeffs_0[] = {
 0xffff, 0xffff, 0xfffe, 0xfffd, 0xfffc, 0xfffa, 0xfff7, 0xfff4, 0xfff0,
 0xffec, 0xffe6, 0xffdf, 0xffd8, 0xffd0, 0xffc7, 0xffbf, 0xffb5, 0xffad,
 0xffa5, 0xff9e, 0xff9a, 0xff98, 0xff99, 0xff9f, 0xffa9, 0xffb9, 0xffcf,
 0xffec, 0x0011, 0x003d, 0x0071, 0x00ad, 0x00f1, 0x013c, 0x018e, 0x01e6,
 0x0242, 0x02a2, 0x0304, 0x0367, 0x03c8, 0x0426, 0x0480, 0x04d2, 0x051d,
 0x055e, 0x0593, 0x05bd, 0x05d9, 0x05e7, 0x05e7, 0x05d9, 0x05bd, 0x0593,
 0x055e, 0x051d, 0x04d2, 0x0480, 0x0426, 0x03c8, 0x0367, 0x0304, 0x02a2,
 0x0242, 0x01e6, 0x018e, 0x013c, 0x00f1, 0x00ad, 0x0071, 0x003d, 0x0011,
 0xffec, 0xffcf, 0xffb9, 0xffa9, 0xff9f, 0xff99, 0xff98, 0xff9a, 0xff9e,
 0xffa5, 0xffad, 0xffb5, 0xffbf, 0xffc7, 0xffd0, 0xffd8, 0xffdf, 0xffe6,
 0xffec, 0xfff0, 0xfff4, 0xfff7, 0xfffa, 0xfffc, 0xfffd, 0xfffe, 0xffff,
 0xffff};




const fractional  __attribute__((space(xmemory), aligned(256), eds)) sensor_adc12_fir_coeffs_1[] = {
 0xffff, 0xffff, 0xfffe, 0xfffd, 0xfffc, 0xfffa, 0xfff7, 0xfff4, 0xfff0,
 0xffec, 0xffe6, 0xffdf, 0xffd8, 0xffd0, 0xffc7, 0xffbf, 0xffb5, 0xffad,
 0xffa5, 0xff9e, 0xff9a, 0xff98, 0xff99, 0xff9f, 0xffa9, 0xffb9, 0xffcf,
 0xffec, 0x0011, 0x003d, 0x0071, 0x00ad, 0x00f1, 0x013c, 0x018e, 0x01e6,
 0x0242, 0x02a2, 0x0304, 0x0367, 0x03c8, 0x0426, 0x0480, 0x04d2, 0x051d,
 0x055e, 0x0593, 0x05bd, 0x05d9, 0x05e7, 0x05e7, 0x05d9, 0x05bd, 0x0593,
 0x055e, 0x051d, 0x04d2, 0x0480, 0x0426, 0x03c8, 0x0367, 0x0304, 0x02a2,
 0x0242, 0x01e6, 0x018e, 0x013c, 0x00f1, 0x00ad, 0x0071, 0x003d, 0x0011,
 0xffec, 0xffcf, 0xffb9, 0xffa9, 0xff9f, 0xff99, 0xff98, 0xff9a, 0xff9e,
 0xffa5, 0xffad, 0xffb5, 0xffbf, 0xffc7, 0xffd0, 0xffd8, 0xffdf, 0xffe6,
 0xffec, 0xfff0, 0xfff4, 0xfff7, 0xfffa, 0xfffc, 0xfffd, 0xfffe, 0xffff,
 0xffff};

const fractional  __attribute__((space(xmemory), aligned(256), eds)) sensor_adc12_fir_coeffs_2[] = {
 0xffff, 0xffff, 0xfffe, 0xfffd, 0xfffc, 0xfffa, 0xfff7, 0xfff4, 0xfff0,
 0xffec, 0xffe6, 0xffdf, 0xffd8, 0xffd0, 0xffc7, 0xffbf, 0xffb5, 0xffad,
 0xffa5, 0xff9e, 0xff9a, 0xff98, 0xff99, 0xff9f, 0xffa9, 0xffb9, 0xffcf,
 0xffec, 0x0011, 0x003d, 0x0071, 0x00ad, 0x00f1, 0x013c, 0x018e, 0x01e6,
 0x0242, 0x02a2, 0x0304, 0x0367, 0x03c8, 0x0426, 0x0480, 0x04d2, 0x051d,
 0x055e, 0x0593, 0x05bd, 0x05d9, 0x05e7, 0x05e7, 0x05d9, 0x05bd, 0x0593,
 0x055e, 0x051d, 0x04d2, 0x0480, 0x0426, 0x03c8, 0x0367, 0x0304, 0x02a2,
 0x0242, 0x01e6, 0x018e, 0x013c, 0x00f1, 0x00ad, 0x0071, 0x003d, 0x0011,
 0xffec, 0xffcf, 0xffb9, 0xffa9, 0xff9f, 0xff99, 0xff98, 0xff9a, 0xff9e,
 0xffa5, 0xffad, 0xffb5, 0xffbf, 0xffc7, 0xffd0, 0xffd8, 0xffdf, 0xffe6,
 0xffec, 0xfff0, 0xfff4, 0xfff7, 0xfffa, 0xfffc, 0xfffd, 0xfffe, 0xffff,
 0xffff};



const fractional  __attribute__((space(xmemory), aligned(512), eds)) sensor_adc12_fir_coeffs_3[] = {
 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
 0x0000, 0x0000, 0xffff, 0xfffe, 0xfffd, 0xfffc, 0xfffb, 0xfffa, 0xfffa,
 0xfff9, 0xfff9, 0xfffa, 0xfffb, 0xfffc, 0xfffe, 0x0001, 0x0004, 0x0007,
 0x000b, 0x000f, 0x0012, 0x0015, 0x0018, 0x0019, 0x0019, 0x0018, 0x0015,
 0x0010, 0x0009, 0x0001, 0xfff8, 0xffee, 0xffe3, 0xffd8, 0xffcd, 0xffc4,
 0xffbe, 0xffb9, 0xffb9, 0xffbc, 0xffc4, 0xffd0, 0xffe0, 0xfff4, 0x000c,
 0x0026, 0x0041, 0x005d, 0x0077, 0x008d, 0x009f, 0x00aa, 0x00ae, 0x00a8,
 0x0099, 0x007f, 0x005b, 0x002d, 0xfff7, 0xffbb, 0xff7b, 0xff3a, 0xfefc,
 0xfec3, 0xfe93, 0xfe71, 0xfe60, 0xfe63, 0xfe7d, 0xfeb0, 0xfefd, 0xff65,
 0xffe7, 0x0080, 0x0130, 0x01f2, 0x02c1, 0x0398, 0x0472, 0x0549, 0x0616,
 0x06d4, 0x077c, 0x0809, 0x0877, 0x08c2, 0x08e9, 0x08e9, 0x08c2, 0x0877,
 0x0809, 0x077c, 0x06d4, 0x0616, 0x0549, 0x0472, 0x0398, 0x02c1, 0x01f2,
 0x0130, 0x0080, 0xffe7, 0xff65, 0xfefd, 0xfeb0, 0xfe7d, 0xfe63, 0xfe60,
 0xfe71, 0xfe93, 0xfec3, 0xfefc, 0xff3a, 0xff7b, 0xffbb, 0xfff7, 0x002d,
 0x005b, 0x007f, 0x0099, 0x00a8, 0x00ae, 0x00aa, 0x009f, 0x008d, 0x0077,
 0x005d, 0x0041, 0x0026, 0x000c, 0xfff4, 0xffe0, 0xffd0, 0xffc4, 0xffbc,
 0xffb9, 0xffb9, 0xffbe, 0xffc4, 0xffcd, 0xffd8, 0xffe3, 0xffee, 0xfff8,
 0x0001, 0x0009, 0x0010, 0x0015, 0x0018, 0x0019, 0x0019, 0x0018, 0x0015,
 0x0012, 0x000f, 0x000b, 0x0007, 0x0004, 0x0001, 0xfffe, 0xfffc, 0xfffb,
 0xfffa, 0xfff9, 0xfff9, 0xfffa, 0xfffa, 0xfffb, 0xfffc, 0xfffd, 0xfffe,
 0xffff, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
 0x0001, 0x0001, 0x0001 };



void sensor_adc12_process_block0(void);
void sensor_adc12_process_block1(void* data);
void sensor_adc12_process_block2(void* data);
void sensor_adc12_process_block3(void* data);
void sensor_adc12_init_filters(void);

void sensor_adc12_init_filters(void)
{
    uint16_t i;

    
    for(i = 0; i < ADC12_N_CHANNELS; i++){
        FIRStructInit(&sensor_adc12_filters_0[i],
            ARRAY_LENGTH(sensor_adc12_fir_coeffs_0),
            (fractional*) sensor_adc12_fir_coeffs_0,
            COEFFS_IN_DATA,
            sensor_adc12_delay_buffers_0[i]
        );

        FIRDelayInit(&sensor_adc12_filters_0[i]);
    }

    for(i = 0; i < ADC12_N_CHANNELS; i++){
        FIRStructInit(&sensor_adc12_filters_1[i],
            ARRAY_LENGTH(sensor_adc12_fir_coeffs_1),
            (fractional*) sensor_adc12_fir_coeffs_1,
            COEFFS_IN_DATA,
            sensor_adc12_delay_buffers_1[i]
        );

        FIRDelayInit(&sensor_adc12_filters_1[i]);
    }

    for(i = 0; i < ADC12_N_CHANNELS; i++){
        FIRStructInit(&sensor_adc12_filters_2[i],
            N_FIR_COEFFS2,
            (fractional*) sensor_adc12_fir_coeffs_2,
            COEFFS_IN_DATA,
            sensor_adc12_delay_buffers_2[i]
        );

        FIRDelayInit(&sensor_adc12_filters_2[i]);
    }

    for(i = 0; i < ADC12_N_CHANNELS; i++){
        FIRStructInit(&sensor_adc12_filters_3[i],
            N_FIR_COEFFS3,
            (fractional*) sensor_adc12_fir_coeffs_3,
            COEFFS_IN_DATA,
            sensor_adc12_delay_buffers_3[i]
        );

        FIRDelayInit(&sensor_adc12_filters_3[i]);
    }
    
    if(adc12_output_buffer0_select == 0)
    {
        for(i = 0; i < ADC12_N_CHANNELS; i++)
        {
            adc12_output_block0_buffer[i] = adc12_output_block0_buffers_b[i];
            adc12_input_block1_buffer[i] = adc12_output_block0_buffers_a[i];
        }
    }
}


void sensor_adc12_process_block0()
{
    uint16_t i;
    static uint16_t block_counter = 0;
    static uint16_t sample_counter = 0;
    fractional sample_buffer[SENSOR_ADC12_COPY_BUFFER_SIZE];
    
    sample_counter += SENSOR_ADC12_COPY_BUFFER_SIZE;
    
    // TODO: change code to handle "weird" number of coefficients coming from DMA
    if(sensor_adc12_adc_buffer_selector == 0){
        vector_copy_jump(ADC12_DMA_BUFFER_SIZE, sample_buffer, adc12_buffer_b, ADC12_N_CHANNELS);
    } else {
        vector_copy_jump(ADC12_DMA_BUFFER_SIZE, sample_buffer, adc12_buffer_a, ADC12_N_CHANNELS);
    }
    
    for(i = 0; i < ADC12_N_CHANNELS; i++){
        FIRDecimate(SENSOR_ADC12_BLOCK0_OUTPUT_SIZE, 
                &adc12_output_block0_buffer[i][block_counter], 
                sample_buffer,
                &sensor_adc12_filters_0[i],
                SENSOR_ADC12_DEC_FACT_F0);
    }
    
    if(sample_counter == 10000){
        sample_counter = 0;
    }
    
    block_counter += SENSOR_ADC12_BLOCK0_OUTPUT_SIZE;
    if(block_counter == SENSOR_ADC12_BLOCK1_INPUT_SIZE){
        task_t task = {sensor_adc12_process_block1, NULL};
        push_queued_task(task);
        block_counter = 0;
        
        adc12_output_buffer0_select ^= 1;
        if(adc12_output_buffer0_select == 0)
        {
            for(i = 0; i < ADC12_N_CHANNELS; i++)
            {
                adc12_output_block0_buffer[i] = adc12_output_block0_buffers_b[i];
                adc12_input_block1_buffer[i] = adc12_output_block0_buffers_a[i];
            }
        } else {
            for(i = 0; i < ADC12_N_CHANNELS; i++)
            {
                adc12_output_block0_buffer[i] = adc12_output_block0_buffers_a[i];
                adc12_input_block1_buffer[i] = adc12_output_block0_buffers_b[i];
            }
        }
    }
}

void sensor_adc12_process_block1(void* data){
    uint16_t i;
    static uint16_t block_counter = 0;
    
    for(i = 0; i < ADC12_N_CHANNELS; i++){   
            
        FIRDecimate(SENSOR_ADC12_BLOCK1_OUTPUT_SIZE, 
                &adc12_output_block1_buffers[i][block_counter], 
                adc12_input_block1_buffer[i], 
                &sensor_adc12_filters_1[i], 
                SENSOR_ADC12_DEC_FACT_F1);
    }
    block_counter += SENSOR_ADC12_BLOCK1_OUTPUT_SIZE;
    if(block_counter == SENSOR_ADC12_BLOCK2_INPUT_SIZE){
        task_t task = {sensor_adc12_process_block2, NULL};
        push_queued_task(task);
        block_counter = 0;
    }
}

void sensor_adc12_process_block2(void* data){
    uint16_t i;
    static uint16_t block_counter = 0;
    
    for(i = 0; i < ADC12_N_CHANNELS; i++){   
        FIRDecimate(SENSOR_ADC12_BLOCK2_OUTPUT_SIZE, 
                &adc12_output_block2_buffers[i][block_counter], 
                adc12_output_block1_buffers[i], 
                &sensor_adc12_filters_2[i], 
                SENSOR_ADC12_DEC_FACT_F2);
    }
    block_counter += SENSOR_ADC12_BLOCK2_OUTPUT_SIZE;
    if(block_counter == SENSOR_ADC12_BLOCK3_INPUT_SIZE){
        block_counter = 0;
        task_t task = {sensor_adc12_process_block3, NULL};
        push_queued_task(task);
    }
}


void sensor_adc12_process_block3(void* data)
{
    uint16_t i;
    fractional result;
    
    for(i = 0; i < ADC12_N_CHANNELS; i++)
    {   
        FIRDecimate(1, 
                &result, 
                adc12_output_block2_buffers[i], 
                &sensor_adc12_filters_3[i], 
                SENSOR_ADC12_DEC_FACT_F3);
        
        // the interface/sensor alloc guarantees that it's always on this location
#ifdef __DICIO__
        if(i % 2 == 0){
            if(sensor_interfaces[i / 2]->gsensor_config[1].sensor_type == SENSOR_TYPE_ADC12){
                if(sensor_interfaces[i / 2]->gsensor_config[1].sensor_config.adc12.average)
                {
                    sensor_interfaces[i / 2]->gsensor_config[1].sensor_config.adc12.sum += result;
                    sensor_interfaces[i / 2]->gsensor_config[1].sensor_config.adc12.count++;
                    result = (fractional) (sensor_interfaces[i / 2]->gsensor_config[1].sensor_config.adc12.sum / sensor_interfaces[i / 2]->gsensor_config[1].sensor_config.adc12.count);
                }
                sensor_interfaces[i / 2]->gsensor_config[1].sensor_config.adc12.result = result;
            }
        } else { 
                if(sensor_interfaces[i / 2]->gsensor_config[2].sensor_config.adc12.average)
                {
                    sensor_interfaces[i / 2]->gsensor_config[2].sensor_config.adc12.sum += result;
                    sensor_interfaces[i / 2]->gsensor_config[2].sensor_config.adc12.count++;
                    result = (fractional) (sensor_interfaces[i / 2]->gsensor_config[2].sensor_config.adc12.sum / sensor_interfaces[i / 2]->gsensor_config[2].sensor_config.adc12.count);
                }
            sensor_interfaces[i / 2]->gsensor_config[2].sensor_config.adc12.result = result;
        }
#else
        if(sensor_interfaces[i]->gsensor_config[1].sensor_type == SENSOR_TYPE_ADC12){
            if(sensor_interfaces[i]->gsensor_config[1].sensor_config.adc12.average)
            {
                sensor_interfaces[i]->gsensor_config[1].sensor_config.adc12.sum += result;
                sensor_interfaces[i]->gsensor_config[1].sensor_config.adc12.count++;
                result = (fractional) (sensor_interfaces[i]->gsensor_config[1].sensor_config.adc12.sum / sensor_interfaces[i]->gsensor_config[1].sensor_config.adc12.count);
            }
            
            sensor_interfaces[i]->gsensor_config[i].sensor_config.adc12.result = result;
        }
#endif
    }
}

