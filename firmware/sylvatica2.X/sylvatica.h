#ifndef __SYLVATICA_H__
#define	__SYLVATICA_H__

#include <xc.h> 
#include <dsp.h>
#include <i2c.h>

#define SYLVATICA_N_CHANNELS               8
#define SYLVATICA_CHANNEL_BUFFER_SIZE      8
#define SYLVATICA_I2C_BASE_ADDRESS         0b1000000
#define SYLVATICA_ADC16_BUFFER_LENGTH        200

#define SYLVATICA_DEC_FACT_F0 10
#define SYLVATICA_DEC_FACT_F1 10
#define SYLVATICA_DEC_FACT_F2 10
#define SYLVATICA_DEC_FACT_F3 5

#define SYLVATICA_ADC16_SAMPLE_FREQUENCY     80000
#define SYLVATICA_COPY_BUFFER_SIZE         (10*SYLVATICA_DEC_FACT_F0)
#define SYLVATICA_BLOCK1_INPUT_SIZE        (10*SYLVATICA_DEC_FACT_F1)
#define SYLVATICA_BLOCK2_INPUT_SIZE        (SYLVATICA_DEC_FACT_F2)
#define SYLVATICA_BLOCK3_INPUT_SIZE        (SYLVATICA_DEC_FACT_F3)
#define SYLVATICA_BLOCK0_OUTPUT_SIZE       (SYLVATICA_COPY_BUFFER_SIZE  / SYLVATICA_DEC_FACT_F0)
#define SYLVATICA_BLOCK1_OUTPUT_SIZE       (SYLVATICA_BLOCK1_INPUT_SIZE / SYLVATICA_DEC_FACT_F1)
#define SYLVATICA_BLOCK2_OUTPUT_SIZE       (SYLVATICA_BLOCK2_INPUT_SIZE / SYLVATICA_DEC_FACT_F2)
#define SYLVATICA_BLOCK3_OUTPUT_SIZE       (SYLVATICA_BLOCK3_INPUT_SIZE / SYLVATICA_DEC_FACT_F3)

#define SYLVATICA_I2C_READ_CH_BUFFER_LENGTH 3

// length in words/samples per channel of the buffer to save the output data into after filtering

// length in bytes of the I2C read buffer
#define SYLVATICA_I2C_READ_BUFFER_LENGTH   (1+8*2)

// length in bytes of the I2C (master) write buffer, i.e. the max length a
// write message to this slave can be
#define SYLVATICA_I2C_WRITE_BUFFER_LENGTH  (5)

#define SYLVATICA_STATUS_ON           (0 << 7)
#define SYLVATICA_STATUS_OFF          (1 << 7)
#define SYLVATICA_STATUS_RESET_BUFFER (1 << 6)
#define SYLVATICA_STATUS_RESET        (1 << 5)

#define SYLVATICA_ADC_ON (1<<7)

#define SYLVATICA_CH_CONFIG_ON (1<<7)
#define SYLVATICA_CH_CONFIG_SET_GAIN(X) (((X)-PGA_GAIN_1) << 3)
#define SYLVATICA_CH_CONFIG_GAIN (0b1111 << 3)

#define SYLVATICA_N_ADDRESS_SEL_PINS 8

#define N_FIR_COEFFS0 100
#define N_FIR_COEFFS1 100
#define N_FIR_COEFFS2 100
#define N_FIR_COEFFS3 192

typedef enum {
    SYLVATICA_REG_STATUS     = 0,
    SYLVATICA_REG_ADC        = 1,
    SYLVATICA_REG_CONFIG_CH0 = 2,
    SYLVATICA_REG_CONFIG_CH1 = 3,
    SYLVATICA_REG_CONFIG_CH2 = 4,
    SYLVATICA_REG_CONFIG_CH3 = 5,
    SYLVATICA_REG_CONFIG_CH4 = 6,
    SYLVATICA_REG_CONFIG_CH5 = 7,
    SYLVATICA_REG_CONFIG_CH6 = 8,
    SYLVATICA_REG_CONFIG_CH7 = 9,
    SYLVATICA_REG_DATA       = 10,
    SYLVATICA_REG_DATA_RAW   = 18,
    SYLVATICA_N_REG
} sylvatica_reg_t;

typedef enum {
    SYLVATICA_CHANNEL_ENABLED,
    SYLVATICA_CHANNEL_DISABLED,
} sylvatica_channel_status_t;

typedef enum {
    SYLVATICA_STATUS_INIT,
    SYLVATICA_STATUS_READY,
    SYLVATICA_STATUS_RUNNING,        
} sylvatica_status_t;

#ifdef	__cplusplus
extern "C" {
#endif
    
    extern fractional fir_coeffs_0[100];
    extern fractional fir_coeffs_1[100];
    extern fractional fir_coeffs_2[100];
    extern fractional fir_coeffs_3[192];
    
    void sylvatica_init(void);
    void sylvatica_send_ready_message(void *data);
    void sylvatica_init_pins(void);
    void sylvatica_filters_init(void);
    void sylvatic_adc16_callback(void);
    
    
#ifdef	__cplusplus
}
#endif 

#endif

