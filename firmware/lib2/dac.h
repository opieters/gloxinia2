#ifndef __DAC_H__
#define	__DAC_H__

#include <xc.h>
#include <dsp.h>
#include <stdbool.h>

#define DAC_SPI_BUFFER_LENGTH     16
#define DAC_PWM_FILTER_R_ADDRESS  0b01011000
#define DAC_PWM_FILTER_R_DEFAULT  0x6
#define DAC_PWM_FILTER_C          (1E-9)
#define DAC_PWM_FREQUENCY_DEFAULT 100000
#define DAC_N_CHANNELS            4

#define MAX_N_CYCLES_PER_PERIOD 1000
#define MAX_N_UPDATES_PER_PERIOD 800

typedef enum {
    DAC_STATUS_OFF,
    DAC_STATUS_IDLE,
    DAC_STATUS_ON
} dac_status_t;

/*
 * status: indicates the current status of the DAC
 * pwd_period: PWM period expressed in number of CPU cycles
 * signal_period: number of PWM cycles needed for one cycle of the target signal
 */
typedef struct dac_config_s {
    dac_status_t status;
    uint16_t pwm_period;
    uint16_t signal_period;
} dac_config_t;

#ifdef	__cplusplus
extern "C" {
#endif   
    
   // void init_dac(dac_config_t* config, planalta_op_mode_t operation_mode, planalta_fs_freq_t sweep_frequency, bool calibration);
    void stop_dac(void);
    void start_dac();
    void update_dac(dac_config_t* config);
    void init_oc(dac_config_t* config);
    void init_dac_dma(dac_config_t* config);
    void init_sine_pwm_buffer(dac_config_t* config, __eds__ fractional* buffer);
    void init_dac_timer(dac_config_t* config);
    void start_dac_timer(void);
    

#ifdef	__cplusplus
}
#endif

#endif
