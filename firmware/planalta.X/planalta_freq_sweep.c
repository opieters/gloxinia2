#include <fir_common.h>
#include <dsp.h>
#include "planalta.h"
#include "planalta_freq_sweep.h"
#include <uart.h>
#include "planalta_filters.h"

extern planalta_config_t gconfig;

void planalta_update_sweep_frequency(planalta_config_t* config){
    switch(config->sweep_frequency){
        case PLANALTA_FS_FREQ_50KHZ:
            config->sweep_frequency = PLANALTA_FS_FREQ_20KHZ;
            break;
        case PLANALTA_FS_FREQ_20KHZ:
            config->sweep_frequency = PLANALTA_FS_FREQ_10KHZ;
            break;
        case PLANALTA_FS_FREQ_10KHZ:
            config->sweep_frequency = PLANALTA_FS_FREQ_5KHZ;
            break;
        case PLANALTA_FS_FREQ_5KHZ:
            config->sweep_frequency = PLANALTA_FS_FREQ_2KHZ;
            break;
        case PLANALTA_FS_FREQ_2KHZ:
            config->sweep_frequency = PLANALTA_FS_FREQ_1KHZ;
            break;
        case PLANALTA_FS_FREQ_1KHZ:
            config->sweep_frequency = PLANALTA_FS_FREQ_500HZ;
            break;
        case PLANALTA_FS_FREQ_500HZ:
            config->sweep_frequency = PLANALTA_FS_FREQ_200HZ;
            break;
        case PLANALTA_FS_FREQ_200HZ:
            config->sweep_frequency = PLANALTA_FS_FREQ_100HZ;
            break;
        case PLANALTA_FS_FREQ_100HZ:
            config->sweep_frequency = PLANALTA_FS_FREQ_50HZ;
            break;
        case PLANALTA_FS_FREQ_50HZ:
            config->sweep_frequency = PLANALTA_FS_FREQ_20HZ;
            break;
        case PLANALTA_FS_FREQ_20HZ:
            config->sweep_frequency = PLANALTA_FS_FREQ_10HZ;
            break;
        case PLANALTA_FS_FREQ_10HZ:
            config->sweep_frequency = PLANALTA_FS_FREQ_50KHZ;
            break;
        default:
            report_error("unable to update the sweep frequency.");
            break;
    }
}

void planalta_get_datapoint(planalta_config_t* config){
    switch(config->sweep_frequency){
        case PLANALTA_FS_FREQ_50KHZ:
            planalta_fs_sample_50khz();
            break;
        case PLANALTA_FS_FREQ_20KHZ:
            planalta_fs_sample_20khz_10khz_5khz(PLANALTA_FS_FREQ_20KHZ);
            break;
        case PLANALTA_FS_FREQ_10KHZ:
            planalta_fs_sample_20khz_10khz_5khz(PLANALTA_FS_FREQ_10KHZ);
            break;
        case PLANALTA_FS_FREQ_5KHZ:
            planalta_fs_sample_20khz_10khz_5khz(PLANALTA_FS_FREQ_5KHZ);
            break;
        case PLANALTA_FS_FREQ_2KHZ:
            planalta_fs_sample_2khz_1khz_500hz(PLANALTA_FS_FREQ_2KHZ);
            break;
        case PLANALTA_FS_FREQ_1KHZ:
            planalta_fs_sample_2khz_1khz_500hz(PLANALTA_FS_FREQ_1KHZ);
            break;
        case PLANALTA_FS_FREQ_500HZ:
            planalta_fs_sample_2khz_1khz_500hz(PLANALTA_FS_FREQ_500HZ);
            break;
        case PLANALTA_FS_FREQ_200HZ:
            planalta_fs_sample_200hz_100hz_50hz(PLANALTA_FS_FREQ_200HZ);
            break;
        case PLANALTA_FS_FREQ_100HZ:
            planalta_fs_sample_200hz_100hz_50hz(PLANALTA_FS_FREQ_100HZ);
            break;
        case PLANALTA_FS_FREQ_50HZ:
            planalta_fs_sample_200hz_100hz_50hz(PLANALTA_FS_FREQ_50HZ);
            break;
        case PLANALTA_FS_FREQ_20HZ:
            planalta_fs_sample_20hz_10hz(PLANALTA_FS_FREQ_20HZ);
            break;
        case PLANALTA_FS_FREQ_10HZ:
            planalta_fs_sample_20hz_10hz(PLANALTA_FS_FREQ_10HZ);
            break;
        default:
            report_error("Unable to get this point during sweep.");
            break;
    }
}

void planalta_fs_update_config(planalta_config_t* config){
    // stop ADC
    adc_stop(&config->adc_config);
    
    // stop DAC
    stop_dac();
    
    config->adc_config.channel_select = ADC_CHANNEL_SELECT_MODE_MANUAL;
    
    // update ADC sample frequency based on frequency point
    switch(config->sweep_frequency){
        case PLANALTA_FS_FREQ_50KHZ:
            config->adc_config.sample_frequency = 200000;
            config->adc_config.rx_callback = adc_rx_callback_fs_50khz;
            config->adc_config.adc_buffer_size = ADC_FS_50KHZ_BUFFER_LENGTH;
            break;
        case PLANALTA_FS_FREQ_20KHZ:
            config->adc_config.sample_frequency = 80000;
            config->adc_config.rx_callback = adc_rx_callback_fs_20khz_10khz_5khz;
            config->adc_config.adc_buffer_size = ADC_FS_20KHZ_10KHZ_5KHZ_BUFFER_LENGTH;
            break;
        case PLANALTA_FS_FREQ_10KHZ:
            config->adc_config.sample_frequency = 40000;
            config->adc_config.rx_callback = adc_rx_callback_fs_20khz_10khz_5khz;
            config->adc_config.adc_buffer_size = ADC_FS_20KHZ_10KHZ_5KHZ_BUFFER_LENGTH;
            break;
        case PLANALTA_FS_FREQ_5KHZ:
            config->adc_config.sample_frequency = 20000;
            config->adc_config.rx_callback = adc_rx_callback_fs_20khz_10khz_5khz;
            config->adc_config.adc_buffer_size = ADC_FS_20KHZ_10KHZ_5KHZ_BUFFER_LENGTH;
            break;
        case PLANALTA_FS_FREQ_2KHZ:
            config->adc_config.sample_frequency = 80000;
            config->adc_config.rx_callback = adc_rx_callback_fs_2khz_1khz_500hz;
            config->adc_config.adc_buffer_size = ADC_FS_2KHZ_1KHZ_500HZ_BUFFER_LENGTH;
            break;
        case PLANALTA_FS_FREQ_1KHZ:
            config->adc_config.sample_frequency = 40000;
            config->adc_config.rx_callback = adc_rx_callback_fs_2khz_1khz_500hz;
            config->adc_config.adc_buffer_size = ADC_FS_2KHZ_1KHZ_500HZ_BUFFER_LENGTH;
            break;
        case PLANALTA_FS_FREQ_500HZ:
            config->adc_config.sample_frequency = 20000;
            config->adc_config.rx_callback = adc_rx_callback_fs_2khz_1khz_500hz;
            config->adc_config.adc_buffer_size = ADC_FS_2KHZ_1KHZ_500HZ_BUFFER_LENGTH;
            break;
        case PLANALTA_FS_FREQ_200HZ:
            config->adc_config.sample_frequency = 80000;
            config->adc_config.rx_callback = adc_rx_callback_fs_200hz_100hz_50hz;
            config->adc_config.adc_buffer_size = ADC_FS_200HZ_100HZ_50HZ_BUFFER_LENGTH;
            break;
        case PLANALTA_FS_FREQ_100HZ:
            config->adc_config.sample_frequency = 40000;
            config->adc_config.rx_callback = adc_rx_callback_fs_200hz_100hz_50hz;
            config->adc_config.adc_buffer_size = ADC_FS_200HZ_100HZ_50HZ_BUFFER_LENGTH;
            break;
        case PLANALTA_FS_FREQ_50HZ:
            config->adc_config.sample_frequency = 20000;
            config->adc_config.rx_callback = adc_rx_callback_fs_200hz_100hz_50hz;
            config->adc_config.adc_buffer_size = ADC_FS_200HZ_100HZ_50HZ_BUFFER_LENGTH;
            break;
        case PLANALTA_FS_FREQ_20HZ:
            config->adc_config.sample_frequency = 80000;
            config->adc_config.rx_callback = adc_rx_callback_fs_20hz_10hz;
            config->adc_config.adc_buffer_size = ADC_FS_20HZ_10HZ_BUFFER_LENGTH;
            break;
        case PLANALTA_FS_FREQ_10HZ:
            config->adc_config.sample_frequency = 40000;
            config->adc_config.rx_callback = adc_rx_callback_fs_20hz_10hz;
            config->adc_config.adc_buffer_size = ADC_FS_20HZ_10HZ_BUFFER_LENGTH;
            break;
        default:
            report_error("Unable to get this point during sweep.");
            break;
    }
    
    // update DAC configuration
    init_dac(config, false); // also starts DAC
    
    // update the ADC configuration
    init_adc_fast(&config->adc_config);
    
    planalta_set_filters(config);
    
    // TODO
    // PGA calibration??
    // output calibration??
    
    // start ADC
    adc_start(&config->adc_config);
}

void planalta_fs_update_channel(planalta_config_t* config, uint8_t step){
    adc_channel_t channel = config->adc_config.channel;
    channel = channel - step;
    channel = channel % ADC_CH_N;
    
    config->adc_config.channel = channel;
}

void planalta_sweep_frequency(planalta_config_t* config){
    uint8_t i, j;
    
    config->adc_config.channel = ADC_CH7;
    
    // enable PGA
    _RF1 = 1;
    _RD11 = 1;
    
    for(j = 0; j < PLANALTA_FS_CHANNELS; j++){
        UART_DEBUG_PRINT("Reading next channel");
        
        // check if channel was activated
        if(config->channel_status[j % 4] == PLANALTA_CHANNEL_DISABLED){
            UART_DEBUG_PRINT("Skipping channel");
            
            continue;
        }
        
        config->sweep_frequency = PLANALTA_FS_FREQ_50KHZ;
        
        // read all data points
        for(i = 0; i < PLANALTA_FS_FREQ_N; i++){   
            // update hardware and filters
            planalta_fs_update_config(config);
            
            // read the channel
            planalta_get_datapoint(config);
            
            // set interrupt pin to low
            CLEAR_PORT_BIT(gconfig.int_pin);
            
            planalta_fs_data[j][i][0] = fs_output_buffer_i;
            planalta_fs_data[j][i][1] = fs_output_buffer_q;

            // more to next frequency point
            planalta_update_sweep_frequency(config);

            if(config->status != PLANALTA_STATUS_RUNNING){
                break;
            }
        }
        
        if(config->status != PLANALTA_STATUS_RUNNING){
            break;
        }
        
        if(j == PLANALTA_FS_CHANNELS-2){
            // read channel of output wave
            config->adc_config.channel = ADC_CH6;
        } else {
            planalta_fs_update_channel(config, 2);
        }
        
        gconfig.channel_data_status[j] = PLANALTA_CHANNEL_DATA_STATUS_NEW;
        
        delay_ms(1);
    }
    
    adc_stop(&config->adc_config);
    stop_dac();
    
    // disable PGA
    _RF1 = 0;
    _RD11 = 0;
}
