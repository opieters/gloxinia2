#ifndef __SENSOR_COMMON_H__
#define	__SENSOR_COMMON_H__

#include <xc.h>
#ifdef	__cplusplus
extern "C" {
#endif

/// @brief Check if sensor value is within range and saturate if it is not.
#define SENSOR_CONFIG_CHECK_MAX_SET(X, Y, Z) if((X) > (Y)) {(X) = (Z);}

/// @brief Check is sensor is set to a value and set it if it is not.
#define SENSOR_CONFIG_CHECK_EQUAL_SET(X, Y) if((X) != (Y)) {(X) = (Y);}

/// @brief Sensor type enumeration.
typedef enum {
    SENSOR_NOT_SET           = 0,
    SENSOR_TYPE_SHT35        = 1,   ///< SHT35 sensor
    SENSOR_TYPE_APDS9306_065 = 2,   ///< APDS9306_065 sensor
    SENSOR_TYPE_ADC12        = 3,   ///< 12-bit ADC readout
    SENSOR_TYPE_ADC16        = 4,   ///< 16-bit ADC readour
    SENSOR_TYPE_LIA          = 5,   ///< lock-in amplifier sensor
    SENSOR_TYPE_ADS1219      = 6,   ///< ADS1219 sensor
} sensor_type_t;

/// @brief Sensor status enumeration.
typedef enum {
    SENSOR_STATUS_INACTIVE = 0, ///< Sensor not configured
    SENSOR_STATUS_IDLE = 1,     ///< Sensor configured, but not runnning
    SENSOR_STATUS_ACTIVE = 2,   ///< Sensor start requested, but not yet fully active in hardware
    SENSOR_STATUS_RUNNING = 3,  ///< Sensor measurement campaign started
    SENSOR_STATUS_STOPPED = 4,  ///< Sensor stopped measurement campaign
    SENSOR_STATUS_ERROR = 5,    ///< An error occurred, sensor campaign stopped/not started or sensor not supported
} sensor_status_t;

#ifdef	__cplusplus
}
#endif

#endif

