#ifndef __ADC_H__
#define	__ADC_H__

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <sensor_common.h>

#define ADC12_CHANNEL_SAMPLE_RATE         20000
#define ADC12_N_CHANNELS                      8
#define ADC12_DMA_BUFFER_SIZE                 8
#define ADC12_FULL_SAMPLE_CONVERSION_T_AD    20

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
    bool enable_ch_a;
    bool enable_ch_b;
    
    bool normalise_ch_a;
    bool normalise_ch_b;
    
    uint32_t result_ch_a;
    uint32_t result_ch_b;
} sensor_adc12_config_t;

/**
 * @brief Configuration structure for ADC hardware
 */
typedef struct sensor_adc12_engine_config_s {    
    sensor_adc12_config_t* channel_config[ADC12_N_CHANNELS];
} sensor_adc12_engine_config_t;

struct sensor_interface_s;

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
    sensor_status_t sensor_adc12_config(struct sensor_interface_s *intf, uint8_t *buffer, uint8_t length);
    void sensor_adc12_get_config(struct sensor_interface_s *intf, uint8_t reg, uint8_t *buffer, uint8_t *length);

    
    void sensor_adc12_init_sensor(struct sensor_interface_s *intf);
    void sensor_adc12_activate(struct sensor_interface_s *intf);
    
    bool validate_sensor_adc12_config(sensor_adc12_config_t *config);
    
    void sensor_adc12_measure(void *data);


#ifdef	__cplusplus
}
#endif 

#endif

