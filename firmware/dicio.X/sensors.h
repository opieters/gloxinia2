#ifndef __SENSORS_H__
#define	__SENSORS_H__

#include <xc.h>
#include <stdint.h>
#include "i2c.h"
#include "sensor_common.h"


#define MAX_N_SENSORS          8

#define SENSOR_ERROR_TH        32

#define DICIO_SENSOR_STATUS_LOG_MESSAGE 3

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

