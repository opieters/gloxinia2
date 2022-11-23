#ifndef __SENSOR_COMMON_H__
#define	__SENSOR_COMMON_H__

#include <xc.h>
#ifdef	__cplusplus
extern "C" {
#endif
    
#define SENSOR_CONFIG_CHECK_MAX_SET(X, Y, Z) if((X) > (Y)) {(X) = (Z);}
#define SENSOR_CONFIG_CHECK_EQUAL_SET(X, Y) if((X) != (Y)) {(X) = (Y);}


typedef enum {
    SENSOR_NOT_SET,
    SENSOR_TYPE_SHT35,
    SENSOR_TYPE_APDS9306_065,
    SENSOR_TYPE_ANALOGUE,
} sensor_type_t;

typedef enum {
    SENSOR_STATUS_INACTIVE = 0,
    SENSOR_STATUS_IDLE = 1,
    SENSOR_STATUS_ACTIVE = 2,
    SENSOR_STATUS_RUNNING = 3,
    SENSOR_STATUS_ERROR = 4,
} sensor_status_t;

#ifdef	__cplusplus
}
#endif

#endif

