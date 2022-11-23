#include <sensor.h>
#include <address.h>

static sensor_interface_t sensor_interfaces[N_SENSOR_INTERFACES];

void sensors_init(void){
    int i;
    
    for(i = 0; i < N_SENSOR_INTERFACES; i++){
        task_t task = {task_dummy, NULL};
        schedule_init(&sensor_interfaces->measure, task, 0);
    }
}

void sensor_set_config_from_buffer(uint8_t interface_id, uint8_t* buffer, uint8_t length){
    sensor_interface_t* intf;
    sensor_type_t stype;
    message_t m;
    uint8_t data[2];
    
    // input validation
    if(length < 2){
        return;
    }
    if(interface_id >= N_SENSOR_INTERFACES){
        return;
    }
    
    intf = &sensor_interfaces[interface_id];
    stype = (sensor_type_t) buffer[0];
    
    intf->sensor_type = stype;
    
    int status = 0;
    
    // register 0 contains the measurement period, and status fields
    if(buffer[1] == 0){
        if(length != 5){
            return;
        }
        
        intf->status = (sensor_status_t) buffer[2];
        schedule_init(&sensor_interfaces->measure, sensor_interfaces->measure.task, (((uint16_t) buffer[3]) << 8) | buffer[4]);
    }
    
    switch(stype){
        case SENSOR_NOT_SET:
            break;
        case SENSOR_TYPE_SHT35:
            status = sensor_sht35_config(intf, &buffer[1], length - 1);
            break;
        case SENSOR_TYPE_APDS9306_065:
            status = sensor_apds9306_065_config(intf, &buffer[1], length - 1);
            break;
        case SENSOR_TYPE_ANALOGUE:
            // TODO
            break;
        default:
            break;
    }
    
    if(status == SENSOR_STATUS_ERROR){
        intf->status = SENSOR_STATUS_ERROR;
    } else {
        intf->status = SENSOR_STATUS_IDLE;
    }
    
    // we report the status back to the central
    message_init(&m,  controller_address, MESSAGE_NO_REQUEST, M_SENSOR_STATUS,
        interface_id, data, ARRAY_LENGTH(data));
    data[0] = intf->status;
   
    message_send(&m);
}

void sensor_error_handle(sensor_interface_t* intf){
    message_t m_status;
    uint8_t data[1];
    
    message_init(&m_status, controller_address, MESSAGE_NO_REQUEST,
        M_SENSOR_STATUS, intf->sensor_id, data, ARRAY_LENGTH(data));
    data[0] = intf->status;

    schedule_remove_event(intf->measure.id);
}

void sensor_i2c_error_handle(sensor_interface_t* intf, i2c_message_t* m, uint8_t location){
    intf->log_data[0] = m->status;
    intf->log_data[1] = m->error;
    intf->log_data[2] = location;

    message_init(&intf->log,
            controller_address,
            MESSAGE_NO_REQUEST,
            M_SENSOR_STATUS,
            intf->sensor_id,
            intf->log_data,
            3);
    message_send(&intf->log);

    schedule_remove_event(intf->measure.id);
}

void sensor_start(uint8_t sensor_id){
    // check input
    if(sensor_id >= N_SENSOR_INTERFACES){
        return;
    }
    
    // check if init was done correctly
    if(sensor_interfaces[sensor_id].measure.task.cb == task_dummy){
        return;
    }
    
    sensor_interfaces[sensor_id].status = SENSOR_STATUS_RUNNING;
    
    schedule_event(&sensor_interfaces[sensor_id].measure);
}