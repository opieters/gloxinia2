
// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __SENSOR_COMMON_H__
#define	__SENSOR_COMMON_H__

#include <xc.h> // include processor files - each processor file is guarded.  
#include <i2c.h>
#include <uart.h>
#include <can.h>

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

#define ERROR_DATA_SIZE 4

#define SENSOR_CONFIG_CHECK_EQUAL_SET(X, Y) if((X) != (Y)) {(X) = (Y);}
#define SENSOR_CONFIG_CHECK_MAX_SET(X, Y, Z) if((X) > (Y)) {(X) = (Z);}

    typedef enum {
        SENSOR_STATUS_INACTIVE = 0,
        SENSOR_STATUS_IDLE = 1,
        SENSOR_STATUS_ACTIVE = 2,
        SENSOR_STATUS_ERROR = 3,
        N_SENSOR_STATUS = 4,
    } sensor_status_t;

    typedef struct sensor_log_s {
        can_message_t can_message;
        uart_message_t uart_message;
    } sensor_log_t;

    typedef struct sensor_elog_s {
        can_message_t can_message;
        uart_message_t uart_message;
        uint16_t n_errors;
    } sensor_elog_t;

    typedef enum {
        SENSOR_PHASE_PRE_READOUT,
        SENSOR_PHASE_READOUT,
        SENSOR_PHASE_POST_READOUT,
    } sensor_message_phase_t;

    typedef enum {
        SENSOR_TYPE_SHT35,
    } sensor_type_t;

    typedef struct i2c_sensor_config_s {
        uint8_t sensor_id;
        sensor_type_t sensor_type;

        sensor_status_t status;

        sensor_log_t dlog;
        sensor_elog_t elog;

        uint8_t tx_data[CAN_MAX_N_BYTES];
        uint8_t error_data[ERROR_DATA_SIZE];
    } sensor_config_t;

    /*typedef struct {
    
    } i2c_sensor_config_t;*/


    void sensor_update_status(sensor_config_t* config, const i2c_error_t i2c_error);
    void sensor_send_error(sensor_elog_t* config, i2c_message_t* m);
    void sensor_send_general_error(sensor_elog_t* config, uint8_t status, uint8_t error);
    void sensor_send_data(sensor_log_t* config, uint8_t* data, uint8_t length);
    void sensor_send_data_no_copy(sensor_log_t* config, uint8_t* data, uint8_t length);

    uint8_t sensor_get_local_id(void);
    void sensor_reset_local_id(void);

    void sensor_init_common_config(sensor_config_t* config, uint8_t length);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

