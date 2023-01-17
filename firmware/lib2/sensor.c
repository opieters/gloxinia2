#include <sensor.h>
#include <address.h>

sensor_interface_t sensor_interfaces[N_SENSOR_INTERFACES];

void sensors_init(void) {
    int i;

    for (i = 0; i < N_SENSOR_INTERFACES; i++) {
        task_t task = {task_dummy, NULL};
        schedule_init(&sensor_interfaces[i].measure, task, 0);

        sensor_interfaces[i].sensor_type = SENSOR_NOT_SET;
        sensor_interfaces[i].status = SENSOR_STATUS_INACTIVE;
        sensor_interfaces[i].sensor_id = i;
    }


}

i2c_bus_t sensor_get_i2c_bus(uint8_t interface_id) {
    if (interface_id < (N_SENSOR_INTERFACES / 2)) {
        return I2C1_BUS;
    } else {
        return I2C2_BUS;
    }
}

void sensor_get_config(uint8_t interface_id, uint8_t reg, uint8_t* buffer, uint8_t* length) {
    sensor_interface_t* intf = &sensor_interfaces[interface_id];
    sensor_type_t stype = intf->sensor_type;

    switch (stype) {
        case SENSOR_TYPE_SHT35:
            sensor_sht35_get_config(intf, reg, buffer, length);
            break;
        case SENSOR_TYPE_APDS9306_065:
            sensor_apds9306_065_get_config(intf, reg, buffer, length);
            break;
        case SENSOR_TYPE_ANALOGUE:
            //TODO
            break;
        case SENSOR_NOT_SET:
        default:
            *length = 0;
            break;
    }

}

void sensor_set_config_from_buffer(uint8_t interface_id, uint8_t *buffer, uint8_t length) {
    sensor_interface_t *intf;
    sensor_type_t stype;
    message_t m;
    uint8_t data[1];

    // input validation
    if (length < 2) {
        return;
    }
    if (interface_id >= N_SENSOR_INTERFACES) {
        return;
    }

    intf = &sensor_interfaces[interface_id];
    stype = (sensor_type_t) buffer[0];

    intf->sensor_type = stype;

    sensor_status_t status = SENSOR_STATUS_ERROR;

    switch (stype) {
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

    // update sensor status based on variables
    if ((intf->status == SENSOR_STATUS_INACTIVE) || (intf->status == SENSOR_STATUS_STOPPED))
        intf->status = status;
    if (status == SENSOR_STATUS_ERROR) {
        intf->status = status;
        UART_DEBUG_PRINT("ERROR when configuring");
    }


    // we report the status back to the central
    message_init(&m, controller_address, MESSAGE_NO_REQUEST, M_SENSOR_STATUS,
            interface_id, data, ARRAY_LENGTH(data));
    data[0] = intf->status;

    message_send(&m);
}

void sensor_send_status(uint8_t interface_id) {
    message_t m;
    uint8_t data[1];

    if (interface_id >= N_SENSOR_INTERFACES)
        return;

    data[0] = sensor_interfaces[interface_id].status;

    message_init(&m, controller_address,
            MESSAGE_NO_REQUEST,
            M_SENSOR_STATUS,
            interface_id,
            data,
            ARRAY_LENGTH(data));
    message_send(&m);
}

void sensor_set_status(uint8_t interface_id, sensor_status_t status) {
    sensor_interface_t* intf = &sensor_interfaces[interface_id];

    message_t m_status;
    uint8_t data[1];

    switch (status) {
        case SENSOR_STATUS_ACTIVE:
            if ((intf->status == SENSOR_STATUS_IDLE) || (intf->status == SENSOR_STATUS_STOPPED)) {
                if (intf->measure.task.cb == task_dummy) {
                    break;
                }
                intf->status = SENSOR_STATUS_ACTIVE;
                switch (intf->sensor_type) {
                    case SENSOR_NOT_SET:
                        break;
                    case SENSOR_TYPE_SHT35:
                        sensor_sht35_activate(intf);
                        break;
                    case SENSOR_TYPE_APDS9306_065:
                        sensor_apds9306_065_activate(intf);
                        break;
                    case SENSOR_TYPE_ANALOGUE:
                        // TODO
                        break;
                    default:
                        break;
                }
                schedule_event(&intf->measure);
            }
            break;
        case SENSOR_STATUS_STOPPED:
            if (intf->status == SENSOR_STATUS_RUNNING) {
                schedule_remove_event(intf->measure.id);
            }
            intf->status = SENSOR_STATUS_STOPPED;
            break;
        case SENSOR_STATUS_INACTIVE:
            intf->status = status;
            break;
        case SENSOR_STATUS_RUNNING:
        case SENSOR_STATUS_IDLE:
        case SENSOR_STATUS_ERROR:
            break;
        default:
            break;
    }

    message_init(&m_status, controller_address, MESSAGE_NO_REQUEST,
            M_SENSOR_STATUS, intf->sensor_id, data, ARRAY_LENGTH(data));
    data[0] = intf->status;
    message_send(&m_status);
}

void sensor_error_log(sensor_interface_t* intf, uint8_t* data, uint8_t length) {
    message_init(&intf->log,
            controller_address,
            MESSAGE_NO_REQUEST,
            M_SENSOR_ERROR,
            intf->sensor_id,
            data,
            length);
    message_send(&intf->log);
}

void sensor_error_handle(sensor_interface_t *intf) {
    message_t m_status;
    uint8_t data[1];

    message_init(&m_status, controller_address, MESSAGE_NO_REQUEST,
            M_SENSOR_STATUS, intf->sensor_id, data, ARRAY_LENGTH(data));
    data[0] = intf->status;
    message_send(&m_status);

    schedule_remove_event(intf->measure.id);
}

void sensor_i2c_error_handle(sensor_interface_t *intf, i2c_message_t *m, uint8_t location) {
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

void sensor_start(void) {
    for (int i = 0; i < N_SENSOR_INTERFACES; i++) {
        // check if init was done correctly
        sensor_set_status(i, SENSOR_STATUS_RUNNING);
    }
}

void sensor_stop(void) {
    for (int i = 0; i < N_SENSOR_INTERFACES; i++) {

        if (sensor_interfaces[i].status == SENSOR_STATUS_RUNNING) {
            // cancel event
            schedule_remove_event(sensor_interfaces[i].measure.id);

            // update sensor status
            sensor_set_status(i, SENSOR_STATUS_STOPPED);
        }
    }
}

i2c_bus_t sensor_i2c_get_bus(uint8_t sensor_interface_n) {
#ifdef __DICIO__
    if (sensor_interface_n < 2) {
        return I2C1_BUS;
    } else {
        return I2C2_BUS;
    }
#else
#error "This board is not yet supported"
#endif
}