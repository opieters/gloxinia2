#include <dsp.h>
#include "planalta_filters.h"
//#include "fir_coeffs.h"
#include "planalta.h"


FIRStruct planalta_lia_filters_0[PLANALTA_N_ADC_CHANNELS];
FIRStruct planalta_lia_filters_1_i[PLANALTA_N_ADC_CHANNELS];
FIRStruct planalta_lia_filters_1_q[PLANALTA_N_ADC_CHANNELS];
FIRStruct planalta_lia_filters_2_i[PLANALTA_N_ADC_CHANNELS];
FIRStruct planalta_lia_filters_2_q[PLANALTA_N_ADC_CHANNELS];
FIRStruct planalta_lia_filters_3_i[PLANALTA_N_ADC_CHANNELS];
FIRStruct planalta_lia_filters_3_q[PLANALTA_N_ADC_CHANNELS];
FIRStruct planalta_lia_filters_4_i[PLANALTA_N_ADC_CHANNELS];
FIRStruct planalta_lia_filters_4_q[PLANALTA_N_ADC_CHANNELS];
FIRStruct planalta_lia_filters_5_i[PLANALTA_N_ADC_CHANNELS];
FIRStruct planalta_lia_filters_5_q[PLANALTA_N_ADC_CHANNELS];
FIRStruct* planalta_lia_filters_6_i = &planalta_lia_filters_0[1];
FIRStruct* planalta_lia_filters_6_q = &planalta_lia_filters_0[2];

FIRStruct planalta_fs_filters_0;
FIRStruct planalta_fs_filters_1;
FIRStruct planalta_fs_filters_1_i;
FIRStruct planalta_fs_filters_1_q;
FIRStruct planalta_fs_filters_2;
FIRStruct planalta_fs_filters_2_i;
FIRStruct planalta_fs_filters_2_q;
FIRStruct planalta_fs_filters_3;
FIRStruct planalta_fs_filters_3_i;
FIRStruct planalta_fs_filters_3_q;
FIRStruct planalta_fs_filters_4_i;
FIRStruct planalta_fs_filters_4_q;
FIRStruct planalta_fs_filters_5_i;
FIRStruct planalta_fs_filters_5_q;


fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_0[N_FIR_COEFFS0];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_1[N_FIR_COEFFS0];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_2[N_FIR_COEFFS0];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_3[N_FIR_COEFFS0];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_4[N_FIR_COEFFS0];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_5[N_FIR_COEFFS0];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_6[N_FIR_COEFFS0];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_7[N_FIR_COEFFS0];

fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_0[N_FIR_COEFFS1_Q];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_0[N_FIR_COEFFS1_I];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_1[N_FIR_COEFFS1_Q];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_1[N_FIR_COEFFS1_I];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_2[N_FIR_COEFFS1_Q];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_2[N_FIR_COEFFS1_I];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_3[N_FIR_COEFFS1_Q];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_3[N_FIR_COEFFS1_I];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_4[N_FIR_COEFFS1_Q];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_4[N_FIR_COEFFS1_I];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_5[N_FIR_COEFFS1_Q];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_5[N_FIR_COEFFS1_I];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_6[N_FIR_COEFFS1_Q];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_6[N_FIR_COEFFS1_I];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_7[N_FIR_COEFFS1_Q];
fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_7[N_FIR_COEFFS1_I];

fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_0[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_0[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_1[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_1[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_2[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_2[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_3[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_3[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_4[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_4[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_5[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_5[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_6[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_6[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_7[N_FIR_COEFFS2];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_7[N_FIR_COEFFS2];

fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_0[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_0[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_1[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_1[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_2[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_2[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_3[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_3[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_4[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_4[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_5[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_5[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_6[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_6[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_7[N_FIR_COEFFS3];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_7[N_FIR_COEFFS3];

fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_0[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_0[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_1[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_1[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_2[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_2[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_3[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_3[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_4[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_4[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_5[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_5[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_6[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_6[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_7[N_FIR_COEFFS4];
fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_7[N_FIR_COEFFS4];

fractional __attribute__((space(xmemory), aligned(512), eds)) delay_buffers_5_q_0[N_FIR_SHARED];
fractional __attribute__((space(xmemory), aligned(512), eds)) delay_buffers_5_i_0[N_FIR_SHARED];
fractional __attribute__((space(xmemory), aligned(512), eds)) delay_buffers_5_q_1[N_FIR_SHARED];
fractional __attribute__((space(xmemory), aligned(512), eds)) delay_buffers_5_i_1[N_FIR_SHARED];


fractional* const __attribute__((space(xmemory), eds)) delay_buffers_0[FIR0_N_COPIES] = {
    delay_buffers_0_0, delay_buffers_0_1, delay_buffers_0_2, delay_buffers_0_3,
    delay_buffers_0_4, delay_buffers_0_5, delay_buffers_0_6, delay_buffers_0_7
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_1_q[FIR1_N_COPIES] = {
    delay_buffers_1_q_0, delay_buffers_1_q_1, delay_buffers_1_q_2, 
    delay_buffers_1_q_3, delay_buffers_1_q_4, delay_buffers_1_q_5, 
    delay_buffers_1_q_6, delay_buffers_1_q_7
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_1_i[FIR1_N_COPIES] = {
    delay_buffers_1_i_0, delay_buffers_1_i_1, delay_buffers_1_i_2,
    delay_buffers_1_i_3, delay_buffers_1_i_4, delay_buffers_1_i_5, 
    delay_buffers_1_i_6, delay_buffers_1_i_7
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_2_q[FIR2_N_COPIES] = {
    delay_buffers_2_q_0, delay_buffers_2_q_1, delay_buffers_2_q_2,
    delay_buffers_2_q_3, delay_buffers_2_q_4, delay_buffers_2_q_5,
    delay_buffers_2_q_6, delay_buffers_2_q_7
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_2_i[FIR2_N_COPIES] = {
    delay_buffers_2_i_0, delay_buffers_2_i_1, delay_buffers_2_i_2,
    delay_buffers_2_i_3, delay_buffers_2_i_4, delay_buffers_2_i_5,
    delay_buffers_2_i_6, delay_buffers_2_i_7
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_3_q[FIR3_N_COPIES] = {
    delay_buffers_3_q_0, delay_buffers_3_q_1, delay_buffers_3_q_2,
    delay_buffers_3_q_3, delay_buffers_3_q_4, delay_buffers_3_q_5,
    delay_buffers_3_q_6, delay_buffers_3_q_7
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_3_i[FIR3_N_COPIES] = {
    delay_buffers_3_i_0, delay_buffers_3_i_1, delay_buffers_3_i_2,
    delay_buffers_3_i_3, delay_buffers_3_i_4, delay_buffers_3_i_5,
    delay_buffers_3_i_6, delay_buffers_3_i_7
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_4_q[FIR4_N_COPIES] = {
    delay_buffers_4_q_0, delay_buffers_4_q_1, delay_buffers_4_q_2,
    delay_buffers_4_q_3, delay_buffers_4_q_4, delay_buffers_4_q_5,
    delay_buffers_4_q_6, delay_buffers_4_q_7,
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_4_i[FIR4_N_COPIES] = {
    delay_buffers_4_i_0, delay_buffers_4_i_1, delay_buffers_4_i_2,
    delay_buffers_4_i_3, delay_buffers_4_i_4, delay_buffers_4_i_5,
    delay_buffers_4_i_6, delay_buffers_4_i_7,
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_5_q[FIR6_N_COPIES] = {
    delay_buffers_5_q_0, delay_buffers_5_q_1
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_5_i[FIR6_N_COPIES] = {
    delay_buffers_5_i_0, delay_buffers_5_i_1
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_7_q[FIR7_N_COPIES] = {
    &delay_buffers_5_q_0[0], &delay_buffers_5_q_0[128], 
    &delay_buffers_5_q_1[0], &delay_buffers_5_q_1[128]
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_7_i[FIR7_N_COPIES] = {
    &delay_buffers_5_i_0[0], &delay_buffers_5_i_0[128], 
    &delay_buffers_5_i_1[0], &delay_buffers_5_i_1[128]
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_8_q[FIR8_N_COPIES] = {
    &delay_buffers_5_q_0[0],   &delay_buffers_5_q_0[64], 
    &delay_buffers_5_q_0[128], &delay_buffers_5_q_0[192],
    &delay_buffers_5_q_1[0],   &delay_buffers_5_q_1[64],
    &delay_buffers_5_q_1[128], &delay_buffers_5_q_1[192],
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_8_i[FIR8_N_COPIES] = {
    &delay_buffers_5_i_0[0], &delay_buffers_5_i_0[64], 
    &delay_buffers_5_i_0[128], &delay_buffers_5_i_0[192],
    &delay_buffers_5_i_1[0], &delay_buffers_5_i_1[64],
    &delay_buffers_5_i_1[128], &delay_buffers_5_i_1[192],
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_9_q[FIR9_N_COPIES] = {
    delay_buffers_5_q_0,
};
fractional* const __attribute__((space(xmemory), eds)) delay_buffers_9_i[FIR9_N_COPIES] = {
    delay_buffers_5_i_0,
};

uint8_t adc_buffer_selector = 0;

volatile uint8_t start_filter_block0 = 0;
uint8_t start_filter_block1 = 0, start_filter_block2 = 0, start_filter_block3 = 0;
uint8_t start_filter_block4 = 0, start_filter_block5 = 0, start_filter6 = 0;

fractional* fo2_buffer_i_read[PLANALTA_N_ADC_CHANNELS];
fractional* fo2_buffer_q_read[PLANALTA_N_ADC_CHANNELS];
fractional* fo3_buffer_i_read[PLANALTA_N_ADC_CHANNELS];
fractional* fo3_buffer_q_read[PLANALTA_N_ADC_CHANNELS];
fractional* fo4_buffer_i_read[PLANALTA_N_ADC_CHANNELS];
fractional* fo4_buffer_q_read[PLANALTA_N_ADC_CHANNELS];
fractional* fo5_buffer_i_read[PLANALTA_N_ADC_CHANNELS];
fractional* fo5_buffer_q_read[PLANALTA_N_ADC_CHANNELS];

fractional* fo1_buffer_i_write[PLANALTA_N_ADC_CHANNELS];
fractional* fo1_buffer_q_write[PLANALTA_N_ADC_CHANNELS];
fractional* fo2_buffer_i_write[PLANALTA_N_ADC_CHANNELS];
fractional* fo2_buffer_q_write[PLANALTA_N_ADC_CHANNELS];
fractional* fo3_buffer_i_write[PLANALTA_N_ADC_CHANNELS];
fractional* fo3_buffer_q_write[PLANALTA_N_ADC_CHANNELS];
fractional* fo4_buffer_i_write[PLANALTA_N_ADC_CHANNELS];
fractional* fo4_buffer_q_write[PLANALTA_N_ADC_CHANNELS];


uint8_t select_f0_to_f1, select_f1_to_f2, select_f2_to_f3, select_f3_to_f4, select_f4_to_f5;

fractional f1_to_f2_buffer_i_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F2_INPUT_SIZE];
fractional f1_to_f2_buffer_q_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F2_INPUT_SIZE];
fractional f2_to_f3_buffer_i_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F3_INPUT_SIZE];
fractional f2_to_f3_buffer_q_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F3_INPUT_SIZE];
fractional f1_to_f2_buffer_i_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F2_INPUT_SIZE];
fractional f1_to_f2_buffer_q_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F2_INPUT_SIZE];
fractional f2_to_f3_buffer_i_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F3_INPUT_SIZE];
fractional f2_to_f3_buffer_q_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F3_INPUT_SIZE];
fractional f3_to_f4_buffer_i_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F4_INPUT_SIZE];
fractional f3_to_f4_buffer_q_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F4_INPUT_SIZE];
fractional f4_to_f5_buffer_i_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F5_INPUT_SIZE];
fractional f4_to_f5_buffer_q_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F5_INPUT_SIZE];
fractional f3_to_f4_buffer_i_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F4_INPUT_SIZE];
fractional f3_to_f4_buffer_q_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F4_INPUT_SIZE];
fractional f4_to_f5_buffer_i_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F5_INPUT_SIZE];
fractional f4_to_f5_buffer_q_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F5_INPUT_SIZE];


bool planalta_lia_obuffer_selector = 0;
fractional planalta_lia_obuffer_a_i[PLANALTA_N_ADC_CHANNELS];
fractional planalta_lia_obuffer_a_q[PLANALTA_N_ADC_CHANNELS];
fractional planalta_lia_obuffer_b_i[PLANALTA_N_ADC_CHANNELS];
fractional planalta_lia_obuffer_b_q[PLANALTA_N_ADC_CHANNELS];


fractional fs_output_buffer_i;
fractional fs_output_buffer_q;

fractional* fs_fo0_buffer_write;
fractional* fs_fo1_buffer_write;
fractional* fs_fo2_buffer_write;

fractional* fs_fo1_buffer_read;
fractional* fs_fo2_buffer_read;
fractional* fs_fo3_buffer_read;

fractional* fs_fo1_buffer_i_write;
fractional* fs_fo1_buffer_q_write;
fractional* fs_fo2_buffer_i_write;
fractional* fs_fo2_buffer_q_write;
fractional* fs_fo3_buffer_i_write;
fractional* fs_fo3_buffer_q_write;
fractional* fs_fo4_buffer_i_write;
fractional* fs_fo4_buffer_q_write;

fractional* fs_fo2_buffer_i_read;
fractional* fs_fo2_buffer_q_read;
fractional* fs_fo3_buffer_i_read;
fractional* fs_fo3_buffer_q_read;
fractional* fs_fo4_buffer_i_read;
fractional* fs_fo4_buffer_q_read;
fractional* fs_fo5_buffer_i_read;
fractional* fs_fo5_buffer_q_read;

volatile uint16_t planalta_fs_n_coeffs_written = 0;
volatile bool planalta_fs_all_coeffs_written = false;

fractional conversion_buffer[PLANALTA_ADC16_BUFFER_LENGTH];
fractional sample_buffer[PLANALTA_ADC16_BUFFER_LENGTH];

fractional planalta_fs_data[PLANALTA_FS_CHANNELS][PLANALTA_FS_FREQ_N][2];

void init_filters(void){
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
}
