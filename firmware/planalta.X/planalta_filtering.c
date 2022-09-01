#include <xc.h>
#include <stdbool.h>

#include "planalta_filters.h"
#include "filters_planalta.h"
#include <adc.h>
#include <fir_common.h>
#include <dsp.h>
#include "planalta.h"

     
void init_filtering(void){
    uint16_t i;

    fs_fo1_buffer_i_write = f1_to_f2_buffer_i_a[0];
    fs_fo1_buffer_q_write = f1_to_f2_buffer_q_a[0];
    fs_fo2_buffer_i_write = f2_to_f3_buffer_i_a[0];
    fs_fo2_buffer_q_write = f2_to_f3_buffer_q_a[0];
    fs_fo3_buffer_i_write = f3_to_f4_buffer_i_a[0];
    fs_fo3_buffer_q_write = f3_to_f4_buffer_q_a[0];
    fs_fo4_buffer_i_write = f4_to_f5_buffer_i_a[0];
    fs_fo4_buffer_q_write = f4_to_f5_buffer_q_a[0];
    
    for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i++){
        fo2_buffer_i_read[i] = f1_to_f2_buffer_i_b[i];
        fo2_buffer_q_read[i] = f1_to_f2_buffer_q_b[i];
        fo3_buffer_i_read[i] = f2_to_f3_buffer_i_b[i];
        fo3_buffer_q_read[i] = f2_to_f3_buffer_q_b[i];
        fo4_buffer_i_read[i] = f3_to_f4_buffer_i_b[i];
        fo4_buffer_q_read[i] = f3_to_f4_buffer_q_b[i];
        fo5_buffer_i_read[i] = f4_to_f5_buffer_i_b[i];
        fo5_buffer_q_read[i] = f4_to_f5_buffer_q_b[i];
        
        fo1_buffer_i_write[i] = f1_to_f2_buffer_i_a[i];
        fo1_buffer_q_write[i] = f1_to_f2_buffer_q_a[i];
        fo2_buffer_i_write[i] = f2_to_f3_buffer_i_a[i];
        fo2_buffer_q_write[i] = f2_to_f3_buffer_q_a[i];
        fo3_buffer_i_write[i] = f3_to_f4_buffer_i_a[i];
        fo3_buffer_q_write[i] = f3_to_f4_buffer_q_a[i];
        fo4_buffer_i_write[i] = f4_to_f5_buffer_i_a[i];
        fo4_buffer_q_write[i] = f4_to_f5_buffer_q_a[i];
    }
    select_f0_to_f1 = 0;
    select_f1_to_f2 = 0;
    select_f2_to_f3 = 0;
    select_f3_to_f4 = 0;
    select_f4_to_f5 = 0;
    
    planalta_fs_n_coeffs_written = 0;
}

void planalta_clear_filter_buffers(void){
    uint16_t i, j;
    
    init_filtering();
    
    // todo: clear delay buffers
    
    for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i++){
        for(j = 0; j < PLANALTA_F2_INPUT_SIZE; j++){
            f1_to_f2_buffer_i_a[i][j] = 0;
            f1_to_f2_buffer_q_a[i][j] = 0;
            
            f1_to_f2_buffer_i_b[i][j] = 0;
            f1_to_f2_buffer_q_b[i][j] = 0;
        }
    }
    for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i++){
        for(j = 0; j < PLANALTA_F3_INPUT_SIZE; j++){
            f2_to_f3_buffer_i_a[i][j] = 0;
            f2_to_f3_buffer_q_a[i][j] = 0;
            
            f2_to_f3_buffer_i_b[i][j] = 0;
            f2_to_f3_buffer_q_b[i][j] = 0;
        }
    }
    for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i++){
        for(j = 0; j < PLANALTA_F4_INPUT_SIZE; j++){
            f3_to_f4_buffer_i_a[i][j] = 0;
            f3_to_f4_buffer_q_a[i][j] = 0;
            
            f3_to_f4_buffer_i_b[i][j] = 0;
            f3_to_f4_buffer_q_b[i][j] = 0;
        }
    }
    for(i = 0; i < PLANALTA_N_ADC_CHANNELS; i++){
        for(j = 0; j < PLANALTA_F5_INPUT_SIZE; j++){
            f4_to_f5_buffer_i_a[i][j] = 0;
            f4_to_f5_buffer_i_a[i][j] = 0;
            
            f4_to_f5_buffer_i_b[i][j] = 0;
            f4_to_f5_buffer_i_b[i][j] = 0;
        }
    }
}
