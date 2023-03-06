#ifndef __SENSOR_ANALOG_H__
#define	__SENSOR_ANALOG_H__

#include <xc.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
    typedef struct sensor_analog_config_s {
        uint16_t period;
       
        bool k;
    } sensor_analogue_config_t;

#ifdef	__cplusplus
}
#endif

#endif

