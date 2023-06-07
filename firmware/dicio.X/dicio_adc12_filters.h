#ifndef __DICIO_ADC12_H__
#define	__DICIO_ADC12_H__

#include <xc.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
void sensor_adc12_init_filters(void);
void sensor_adc12_process_block0();
void sensor_adc12_process_block1(void* data);
void sensor_adc12_process_block2(void* data);
void sensor_adc12_process_block3(void* data);


#ifdef	__cplusplus
}
#endif

#endif

