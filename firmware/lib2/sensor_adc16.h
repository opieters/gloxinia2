#ifndef __SENSOR_ADC16_H__
#define __SENSOR_ADC16_H__

#include <xc.h>
#include <utilities.h>
#include <spi.h>
#include <pga.h> 

#define SENSOR_ADC16_CAN_DATA_LENGTH (sizeof(fractional))



/**
 * @brief ADC16 software configuration structure
 * 
 * @param enable_ch_a: enable 16-bit ADC channel
 * @param enable_ch_b: enable 12-bit ADC channel
 * 
 * @param normalise_ch_a: normalise accumulation of 16-bit readout to 16-bit
 * @param normalise_ch_b: normalise accumulation of 12-bit readout to 12-bit
 * 
 * @param result_ch_a: 16-bit register result (unnormalised)
 * @param result_ch_b: 12-bit register result (unnormalised)
 */
typedef struct {    
    bool average;
    bool auto_gain_setting;
    
    fractional result;
    uint16_t count;
    uint32_t sum;
    
    pga_config_t* pga;
} sensor_adc16_config_t;

/**
 * @brief ADC16 sensor configuration registers
 */
typedef enum {
    sensor_adc16_gloxinia_register_general = 0x00, ///< General register with sample rate info
    sensor_adc16_gloxinia_register_config = 0x01,  ///< Channel and normalisation enable/disable register
    sensor_adc16_gloxinia_register_pga = 0x02,
} sensor_adc16_gloxinia_register_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    
    struct sensor_gconfig_s;
    void sensor_adc16_get_config(struct sensor_gconfig_s* intf, uint8_t reg, uint8_t* buffer, uint8_t* length);
    sensor_status_t sensor_adc16_config(struct sensor_gconfig_s *intf, const uint8_t *buffer, uint8_t length);
    bool validate_adc16_config(sensor_adc16_config_t *config);
    
    void sensor_adc16_activate(struct sensor_gconfig_s* intf);
    void sensor_adc16_deactivate(struct sensor_gconfig_s* intf);
    
    void sensor_adc16_measure(void *data);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SENSOR_ADC16_H__ */
