#ifndef __ADC_H__
#define	__ADC_H__

#include <xc.h>

    /// @brief Internal 12-bit ADC sensor configuration structure
    typedef enum
    {
        sensor_adc12_gloxinia_register_general = 0x00,
        sensor_adc12_gloxinia_register_config = 0x01,
    } sensor_adc12_gloxinia_register_t;

/**
 * @brief Configuration structure for each of the ADC channels
 */
typedef struct sensor_adc12_config_s
{
} sensor_adc12_config_t;

/**
 * @brief Configuration structure for ADC hardware
 */
typedef struct sensor_adc12_engine_config_s
{
} sensor_adc12_engine_config_t;
    
#ifdef	__cplusplus
extern "C" {
#endif 

    void init_adc12(void);
    

#ifdef	__cplusplus
}
#endif 

#endi

