#ifndef __SENSORS_H__
#define	__SENSORS_H__

#include <xc.h>
#include <stdint.h>
#include <i2c.h>
#include <sensor_common.h>


#define MAX_N_SENSORS          8

#define SENSOR_ERROR_TH        32

#define DICIO_SENSOR_STATUS_LOG_MESSAGE 3

typedef enum {
    SENSOR_T_I2C,
    SENSOR_T_OW,
    SENSOR_T_ANALOG,
} sensor_t;

typedef struct sensor_config2_s {
    sensor_t t;
    
    uint8_t i2c_address;
    
    void (*init_fn)(sensor_config_t* s_cf);
    void (*measure_fn)(sensor_config_t* s_cf);
    void (*post_measure_fn)(sensor_config_t* s_cf);
    
    // TODO: add one wire configuration
} sensor_config2_t;

typedef union sensor_u {
    
} sensor_u_t;

#ifdef	__cplusplus
extern "C" {
#endif
    
    //extern uint16_t n_sample_messages;
    
    void sensors_init(void);
    void sensors_start(void);

    void sensor_callback(void);
    void sensor_status_report(void);
    void send_sensor_status(sensor_config_t* config);

    void sensors_reset(void);

    void sensors_data_init(void);

    void sensors_error_check(void);
    void sensors_error_recover(void);

#ifdef	__cplusplus
}
#endif

#endif

