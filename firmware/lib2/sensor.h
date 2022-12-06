#ifndef __SENSOR_H__
#define	__SENSOR_H__

#include <xc.h>
#include <can.h>
#include <i2c.h>
#include <sensor_sht35.h>
#include <sensor_apds9306_065.h>
#include <sensor_analog.h>
#include <sensor_common.h>
#include <event_controller.h>

#ifdef __SYLVATICA__
#define N_SENSOR_INTERFACES 8
#else 
#define N_SENSOR_INTERFACES 4
#endif


    
    typedef union sensor_config_u {
        sensor_sht35_config_t sht35;
        sensor_analogue_config_t analogue;
        sensor_apds9306_065_config_t apds9306_065;
    } sensor_config_t;
    

    typedef struct sensor_interface_s {
        sensor_type_t sensor_type;
        uint8_t sensor_id;
        sensor_config_t config;

        sensor_status_t status;
        
        task_schedule_t measure;

        message_t log;
        uint8_t log_data[CAN_MAX_N_BYTES];
    } sensor_interface_t;

#ifdef	__cplusplus
extern "C" {
#endif

    void sensors_init(void);
    void sensor_set_config_from_buffer(uint8_t interface_id, uint8_t* buffer, uint8_t length);
    void sensor_get_config(uint8_t interface_id, uint8_t reg, uint8_t* buffer, uint8_t* length);
    i2c_bus_t sensor_get_i2c_bus(uint8_t interface_id);
    void sensor_error_log(sensor_interface_t* intf, uint8_t* data, uint8_t length);
    void sensor_error_handle(sensor_interface_t* intf);
    void sensor_i2c_error_handle(sensor_interface_t* intf, i2c_message_t* m, uint8_t location);
    void sensor_start(void);
    void sensor_send_status(uint8_t interface_id);
    void sensor_set_status(uint8_t interface_id, sensor_status_t s);


#ifdef	__cplusplus
}
#endif

#endif

