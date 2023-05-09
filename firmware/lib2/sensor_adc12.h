#ifndef __ADC12_H__
#define	__ADC12_H__

#include <xc.h>
#include <dsp.h>
#include <stdint.h>
#include <stdbool.h>
#include <sensor_common.h>

#define ADC12_CHANNEL_SAMPLE_RATE         8000UL
#define ADC12_N_CHANNELS                      8U
#define ADC12_DMA_BUFFER_SIZE               100U
#define ADC12_FULL_SAMPLE_CONVERSION_T_AD    20U

/// @brief ADC12 sensor sample data length
#define SENSOR_ADC12_DATA_LENGTH 2

/// @brief ADC12 sensor data length
#define SENSOR_ADC12_CAN_DATA_LENGTH SENSOR_ADC12_DATA_LENGTH

#define SENSOR_ADC12_DEC_FACT_F0 10
#define SENSOR_ADC12_DEC_FACT_F1 10
#define SENSOR_ADC12_DEC_FACT_F2 10
#define SENSOR_ADC12_DEC_FACT_F3 5

#define SENSOR_ADC12_COPY_BUFFER_SIZE         (10*SENSOR_ADC12_DEC_FACT_F0)
#define SENSOR_ADC12_BLOCK1_INPUT_SIZE        (10*SENSOR_ADC12_DEC_FACT_F1)
#define SENSOR_ADC12_BLOCK2_INPUT_SIZE        (1*SENSOR_ADC12_DEC_FACT_F2)
#define SENSOR_ADC12_BLOCK3_INPUT_SIZE        (SENSOR_ADC12_DEC_FACT_F3)
#define SENSOR_ADC12_BLOCK0_OUTPUT_SIZE       (SENSOR_ADC12_COPY_BUFFER_SIZE  / SENSOR_ADC12_DEC_FACT_F0)
#define SENSOR_ADC12_BLOCK1_OUTPUT_SIZE       (SENSOR_ADC12_BLOCK1_INPUT_SIZE / SENSOR_ADC12_DEC_FACT_F1)
#define SENSOR_ADC12_BLOCK2_OUTPUT_SIZE       (SENSOR_ADC12_BLOCK2_INPUT_SIZE / SENSOR_ADC12_DEC_FACT_F2)
#define SENSOR_ADC12_BLOCK3_OUTPUT_SIZE       (SENSOR_ADC12_BLOCK3_INPUT_SIZE / SENSOR_ADC12_DEC_FACT_F3)

/// @brief Internal 12-bit ADC sensor configuration structure

typedef enum {
    sensor_adc12_gloxinia_register_general = 0x00,
    sensor_adc12_gloxinia_register_config = 0x01,
} sensor_adc12_gloxinia_register_t;

/**
 * @brief Configuration structure for each of the ADC channels
 * 
 * @param enable_ch_a enable conversion in channel A
 * @param enable_ch_b enable conversion in channel B
 * @param normalise_ch_a normalise conversion result of channel A
 * @param normalise_ch_b normalise conversion result of channel B
 * @param result_ch_a register for intermediate storage of data from channel A
 * @param result_ch_b register for intermediate storage of data from channel B
 */
typedef struct sensor_adc12_config_s {
    bool enable;
    
    fractional result;
} sensor_adc12_config_t;

/**
 * @brief Configuration structure for ADC hardware
 */
typedef struct sensor_adc12_engine_config_s {    
    sensor_adc12_config_t* channel_config[ADC12_N_CHANNELS];
} sensor_adc12_engine_config_t;

struct sensor_gconfig_s;

extern fractional adc12_output_buffer[ADC12_N_CHANNELS];
extern int32_t adc12_output_sum_buffer[ADC12_N_CHANNELS];
extern uint16_t adc12_output_sum_counter[ADC12_N_CHANNELS];
extern volatile uint8_t sensor_adc12_adc_buffer_selector;
extern fractional adc12_buffer_a[ADC12_DMA_BUFFER_SIZE*ADC12_N_CHANNELS]  __attribute__( (eds, aligned(256), space(xmemory)) );
extern fractional adc12_buffer_b[ADC12_DMA_BUFFER_SIZE*ADC12_N_CHANNELS]  __attribute__( (eds, aligned(256), space(xmemory)) );

#ifdef	__cplusplus
extern "C" {
#endif 
    

    void init_adc12(void);
    
    /**
     * @brief 
     *
     * @details .
     *
     * @return  indicating status of the I2C transfer.
     */
    sensor_status_t sensor_adc12_config(struct sensor_gconfig_s *intf, uint8_t *buffer, uint8_t length);
    void sensor_adc12_get_config(struct sensor_gconfig_s *intf, uint8_t reg, uint8_t *buffer, uint8_t *length);

    
    void sensor_adc12_init(struct sensor_gconfig_s *config);
    void sensor_adc12_activate(struct sensor_gconfig_s *config);
    
    bool validate_sensor_adc12_config(sensor_adc12_config_t *config);
    bool validate_adc12_config(sensor_adc12_config_t *config);
    
    void sensor_adc12_measure(void *data);
    
    void adc12_callback_dummy(void);
    void sensor_adc12_set_callback(void (*cb)(void) );


#ifdef	__cplusplus
}
#endif 

#endif

