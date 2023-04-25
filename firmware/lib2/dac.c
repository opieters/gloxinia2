#include <xc.h>
#include <dsp.h>
#include "dac.h"
#include <math.h>
#include <sensor_adc12.h>
#include <utilities.h>

__eds__ fractional dac_buffer[MAX_N_UPDATES_PER_PERIOD] __attribute__((space(dma), eds));


void init_sine_pwm_buffer(dac_config_t* config, __eds__ fractional* buffer){
    uint16_t i, j, duty_cycle_index, n_possible_duty_cylces;
    const uint16_t length = config->signal_period;
    float duty_cycle_value;
    float c_offset, p_offset, sine_value;
    
    n_possible_duty_cylces = MIN(MAX_N_CYCLES_PER_PERIOD, config->pwm_period);
    
    // calculate actual duty cycles
    for(i = 0; i < length; i++){
        p_offset = 1.0;
        sine_value = (1.0+sin(2.0 * PI * i / length)) / 2.0;
        duty_cycle_index = 0;
        // find the closest PWM value
        for(j = 0; j < n_possible_duty_cylces; j++){
            // calculate possible duty cycles (not uniform!)
            if(j == 0){
                duty_cycle_value = 0.0;
            } else { 
                if(j == n_possible_duty_cylces-1) {
                    duty_cycle_value = 1.0;
                }
                else {
                    duty_cycle_value = 1.0 * (j+1) / (n_possible_duty_cylces);
                }
            }
            c_offset = duty_cycle_value - sine_value;
            c_offset = c_offset>=0?c_offset:-c_offset;
            if(c_offset < p_offset){
                p_offset = c_offset;
                duty_cycle_index = j;
            }
        }
        buffer[i] = duty_cycle_index;
        //buffer[i] = n_possible_duty_cylces-1;
    }
}


void init_buffer_calibration(double duty_cycle, dac_config_t* config, __eds__ fractional* buffer){
    uint16_t i, j, duty_cycle_index = 0, n_possible_duty_cylces;
    const uint16_t length = config->signal_period;
    float c_offset, p_offset = 1.0, current_duty_cycle;
    
    n_possible_duty_cylces = MIN(MAX_N_CYCLES_PER_PERIOD, config->pwm_period);
    
    // find the closest PWM value
    for(j = 0; j < n_possible_duty_cylces; j++){
        // calculate possible duty cycles (not uniform!)
        if(j == 0){
            current_duty_cycle = 0.0;
        } else { 
            if(j == n_possible_duty_cylces-1) {
                current_duty_cycle = 1.0;
            }
            else {
                current_duty_cycle = 1.0 * (j+1) / (n_possible_duty_cylces);
            }
        }
        c_offset = current_duty_cycle - duty_cycle;
        c_offset = c_offset>=0?c_offset:-c_offset;
        if(c_offset < p_offset){
            p_offset = c_offset;
            duty_cycle_index = j;
        }
    }
    
    // calculate actual duty cycles
    for(i = 0; i < length; i++){
        buffer[i] = duty_cycle_index;
    }
}

void init_oc(dac_config_t* config){
    // initialise Output Compare Module in PWM mode
    OC2CON1bits.OCM = 0b000;              // Disable Output Compare Module
    OC2R = (config->pwm_period)/2-1;     // Write the duty cycle for the PWM pulse
    OC2RS = (config->pwm_period) -1;     // Write the PWM frequency
    OC2CON1bits.OCTSEL = 0b111;           // Select peripheral clock as output compare time base
    
    _OC2IF = 0;                           // clear the OC1 interrupt flag
    _OC2IE = 0;                           // disable OC1 interrupt
    
    OC2CON2bits.SYNCSEL=0b11111;          // no sync event
}

void init_dac_timer(dac_config_t* config){
    // configure timer to trigger DMA OC updates
    T4CONbits.TON = 0; 
    
    T4CONbits.TCS = 0; // use internal instruction cycle as clock source
    T4CONbits.TGATE = 0; // disable gated timer
    T4CONbits.TCKPS = 0b00; // prescaler 1:1
    TMR4 = 0; // clear timer register
    
    PR4 = config->pwm_period-1;
    
    _T4IF = 0; // clear interrupt flag
    _T4IE = 0; // disable interrupt
}

void start_dac(void){
    OC2CON1bits.OCM = 0b110;
    T4CONbits.TON = 1;
}

void stop_dac(void){
    T4CONbits.TON = 0;
    OC2CON1bits.OCM = 0b000;
    DMA0CONbits.CHEN = 0;
}

void init_dac_dma(dac_config_t* config){
    // configure DMA0
    DMA0CONbits.SIZE = 0; // word data transfer
    DMA0CONbits.HALF = 0; // interrupt when all data is transfered
    DMA0CONbits.NULLW = 0; // normal operation
    DMA0CONbits.DIR = 1; // read from RAM to peripheral
    DMA0CONbits.AMODE = 0; // register indirect with post increment
    DMA0CONbits.MODE = 0; // continuous no ping-pong mode
    
    // TODO: for some reason this does not work well
    //DMA0REQbits.IRQSEL = DMAREQ_IRQ_TMR4; // select timer 4 as trigger source
    
    DMA0STAL = __builtin_dmaoffset(dac_buffer); // buffer A
    DMA0STAH = 0x0000;
    
    DMA0PAD = (volatile unsigned int) &OC2R; // peripheral address
    
    DMA0CNT = config->signal_period - 1;
    
    _DMA0IF = 0; // clear interrupt flag
    _DMA0IE = 0; // disable interrupt
    
    DMA0CONbits.CHEN = 1; // enable DMA channel
}

/*
void init_dac(dac_config_t* config, planalta_op_mode_t operation_mode, planalta_fs_freq_t sweep_frequency, bool calibration){
    size_t i;
    __psv__ fractional const* dac_data_buffer = planalta_fs_50khz_dac_data;
    
    stop_dac();
    
    // update DAC config based on filter choice
    switch(operation_mode){
        case PLANALTA_LIA_F_50KHZ:
            config->pwm_period = 10;
            config->signal_period = 128;
            break;
        case PLANALTA_LIA_F_25KHZ:
            config->pwm_period = 10;
            config->signal_period = 256;
            break;
        case PLANALTA_LIA_F_10KHZ:
            config->pwm_period = 16;
            config->signal_period = 400;
            break;
        case PLANALTA_LIA_F_5KHZ:
            config->pwm_period = 16;
            config->signal_period = 800;
            break;
        case PLANALTA_FS:
            switch(sweep_frequency){
                case PLANALTA_FS_FREQ_50KHZ:
                    config->pwm_period = 10U;
                    config->signal_period = 128U;
                    dac_data_buffer = planalta_fs_50khz_dac_data;
                    break;
                case PLANALTA_FS_FREQ_20KHZ:
                    config->pwm_period = 16U;
                    config->signal_period = 200U;
                    dac_data_buffer = planalta_fs_20khz_dac_data;
                    break;
                case PLANALTA_FS_FREQ_10KHZ:
                    config->pwm_period = 16U;
                    config->signal_period = 400U;
                    dac_data_buffer = planalta_fs_10khz_dac_data;
                    break;
                case PLANALTA_FS_FREQ_5KHZ:
                    config->pwm_period = 16U;
                    config->signal_period = 800U;
                    dac_data_buffer = planalta_fs_5khz_dac_data;
                    break;
                case PLANALTA_FS_FREQ_2KHZ:
                    config->pwm_period = 40U;
                    config->signal_period = 800U;
                    dac_data_buffer = planalta_fs_2khz_dac_data;
                    break;
                case PLANALTA_FS_FREQ_1KHZ:
                    config->pwm_period = 80U;
                    config->signal_period = 800U;
                    dac_data_buffer = planalta_fs_1khz_dac_data;
                    break;
                case PLANALTA_FS_FREQ_500HZ:
                    config->pwm_period = 160U;
                    config->signal_period = 800U;
                    dac_data_buffer = planalta_fs_500hz_dac_data;
                    break;
                case PLANALTA_FS_FREQ_200HZ:
                    config->pwm_period = 400U;
                    config->signal_period = 800U;
                    dac_data_buffer = planalta_fs_200hz_dac_data;
                    break;
                case PLANALTA_FS_FREQ_100HZ:
                    config->pwm_period = 800U;
                    config->signal_period = 800U;
                    dac_data_buffer = planalta_fs_100hz_dac_data;
                    break;
                case PLANALTA_FS_FREQ_50HZ:
                    config->pwm_period = 1600U;
                    config->signal_period = 800U;
                    dac_data_buffer = planalta_fs_50hz_dac_data;
                    break;
                case PLANALTA_FS_FREQ_20HZ:
                    config->pwm_period = 4000U;
                    config->signal_period = 800U;
                    dac_data_buffer = planalta_fs_20hz_dac_data;
                    break;
                case PLANALTA_FS_FREQ_10HZ:
                    config->pwm_period = 8000U;
                    config->signal_period = 800U;
                    dac_data_buffer = planalta_fs_10hz_dac_data;
                    break;
                default:
                    //report_error("dac: no support for this configuration.");
                    break;
            }
            break;
        default:
            //report_error("dac: no support for this configuration.");
            break;
    }
    
    // initialise buffer with duty cycle values
    if(calibration){
        // initialise Output Compare Module in PWM mode
        OC2CON1bits.OCM = 0b000;              // Disable Output Compare Module
        OC2R = 0;     // Write the duty cycle for the PWM pulse
        OC2RS = 3;     // Write the PWM frequency
        OC2CON1bits.OCTSEL = 0b111;           // Select peripheral clock as output compare time base

        _OC2IF = 0;                           // clear the OC1 interrupt flag
        _OC2IE = 0;                           // disable OC1 interrupt

        OC2CON2bits.SYNCSEL=0b11111;          // no sync event

         OC2CON1bits.OCM = 0b110;
    } else {
        switch(operation_mode){
            case PLANALTA_LIA_F_50KHZ:
            case PLANALTA_LIA_F_25KHZ:
            case PLANALTA_LIA_F_10KHZ:
            case PLANALTA_LIA_F_5KHZ:
                init_sine_pwm_buffer(config, dac_buffer);
                break;
            case PLANALTA_FS:
                for(i = 0; i < config->signal_period; i++){
                    dac_buffer[i] = dac_data_buffer[i];
                }
                break;
            default:
                break;
        }
        
        init_oc(config);

        init_dac_timer(config);

        init_dac_dma(config);

        start_dac();
    }
    
    
}*/

// allocate interrupt routines to make sure there is no duplicate HW use
void __attribute__((__interrupt__,no_auto_psv)) _DMA3Interrupt(void){
    _DMA3IF = 0;              // Clear the DMA0 Interrupt Flag
}

void __attribute__((__interrupt__, no_auto_psv)) _OC2Interrupt(void){
    _OC2IF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _T4Interrupt(void){
    _T4IF = 0;
}

