#include <sensor.h>
#include <address.h>
#include <rtcc.h>
#ifdef __DICIO__
#include <../dicio.X/sdcard.h>
#include <../dicio.X/dicio.h>
#endif

#if defined(__SYLVATICA__) || defined(__PLANALTA__)
extern pga_config_t pga_config[N_SENSOR_INTERFACES];
#endif

extern sensor_interface_t* sensor_interfaces[];
extern const uint8_t n_sensor_interfaces;


void sensors_init(void) {
    int i, j;

    for (i = 0; i < N_SENSOR_INTERFACES; i++) {
        task_t task;
        task_init(&task, task_dummy, NULL);
        for(j = 0; j < SENSOR_INTERFACE_MAX_SENSORS; j++)
        {
            sensor_gconfig_t* sgc = &sensor_interfaces[i]->gsensor_config[j];
            schedule_init(&sgc->measure, task, 0);

            sgc->sensor_id = j;
            sgc->sensor_type = SENSOR_NOT_SET;
            sgc->status = SENSOR_STATUS_INACTIVE;
            sgc->interface = sensor_interfaces[i];
        }
        sensor_interfaces[i]->interface_id = i;
    }


}

i2c_bus_t sensor_get_i2c_bus(uint8_t interface_id) {
    if (interface_id < (N_SENSOR_INTERFACES / 2)) {
        return I2C1_BUS;
    } else {
        return I2C2_BUS;
    }
}

void sensor_get_config(uint8_t interface_id, uint8_t sensor_id, uint8_t reg, uint8_t* buffer, uint8_t* length) {
    sensor_interface_t* intf = sensor_interfaces[interface_id];
    sensor_gconfig_t* sgc = &intf->gsensor_config[sensor_id];
    sensor_type_t sensor_type = sgc->sensor_type;

    switch (sensor_type) 
    {
        case SENSOR_TYPE_SHT35:
            sensor_sht35_get_config(sgc, reg, buffer, length);
            break;
        case SENSOR_TYPE_APDS9306_065:
            sensor_apds9306_065_get_config(sgc, reg, buffer, length);
            break;
        case SENSOR_TYPE_ADS1219:
            sensor_ads1219_get_config(sgc, reg, buffer, length);
            break;
#if defined(__DICIO__) || defined(__SYLVATICA__)
        case SENSOR_TYPE_ADC12:
            sensor_adc12_get_config(sgc, reg, buffer, length);
            break;
#endif
#if defined(__SYLVATICA__) || defined(__PLANALTA__)
        case SENSOR_TYPE_ADC16:
            sensor_adc16_get_config(sgc, reg, buffer, length);
            break;
#endif
#ifdef __PLANALTA__
        case SENSOR_TYPE_LIA:
            sensor_lia_get_config(sgc, reg, buffer, length);
            break;
#endif
        case SENSOR_NOT_SET:
            *length = 0;
            break;
        default:
            *length = 0;
            break;
    }
}

void sensor_set_config_from_buffer(uint8_t interface_id, uint8_t sensor_id, const uint8_t *buffer, uint8_t length) {
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

    intf = sensor_interfaces[interface_id];
    stype = (sensor_type_t) buffer[0];

    sensor_gconfig_t* gsc = &intf->gsensor_config[sensor_id];
    gsc->sensor_type = stype;

    sensor_status_t status = SENSOR_STATUS_ERROR;

    switch (stype) {
        case SENSOR_NOT_SET:
            break;
        case SENSOR_TYPE_SHT35:
            status = sensor_sht35_config(gsc, &buffer[1], length - 1);
            break;
        case SENSOR_TYPE_APDS9306_065:
            status = sensor_apds9306_065_config(gsc, &buffer[1], length - 1);
            break;
        case SENSOR_TYPE_ADS1219:
            status = sensor_ads1219_config(gsc, &buffer[1], length - 1);
            break;
#if defined(__DICIO__) || defined(__SYLVATICA__)
        case SENSOR_TYPE_ADC12:
#ifdef __DICIO__
            if((sensor_id == 1) || (sensor_id == 2)) {
#else
            if((sensor_id == 1)) {
#endif
                status = sensor_adc12_config(gsc, &buffer[1], length - 1);
            } else {
                status = SENSOR_STATUS_ERROR;
            }
            break;
#endif
#if defined(__SYLVATICA__) || defined(__PLANALTA__)
        case SENSOR_TYPE_ADC16:
            if(sensor_id == 0){
                gsc->sensor_config.adc16.pga = &pga_config[interface_id];
                status = sensor_adc16_config(gsc, &buffer[1], length - 1);
            } else {
                status = SENSOR_STATUS_ERROR;
            }
            break;
#endif
#ifdef __PLANALTA__
        case SENSOR_TYPE_LIA:
            // make sure the right PGA config is loaded, even when the interface
            // is re-configured several times
            if(sensor_id == 0){
                gsc->sensor_config.lia.pga_config = &pga_config[interface_id];
                status = sensor_lia_config(gsc, &buffer[1], length - 1);
            } else {
                status = SENSOR_STATUS_ERROR;
            }
            break;
#endif
        default:
            break;
    }

    // update sensor status based on variables
    if ((gsc->status == SENSOR_STATUS_INACTIVE) || (gsc->status == SENSOR_STATUS_STOPPED))
        gsc->status = status;
    if (status == SENSOR_STATUS_ERROR) {
        gsc->status = status;
        UART_DEBUG_PRINT("ERROR when configuring");
    }


    // we report the status back to the central
    data[0] = gsc->status;
    message_init(&m, controller_address, MESSAGE_NO_REQUEST, M_SENSOR_STATUS,
            gsc->interface->interface_id, gsc->sensor_id, data, ARRAY_LENGTH(data));

    message_send(&m);
}

void sensor_send_status(uint8_t interface_id, uint8_t sensor_id) {
    message_t m;
    uint8_t data[1];

    if (interface_id >= N_SENSOR_INTERFACES)
        return;
    if(sensor_id >= SENSOR_INTERFACE_MAX_SENSORS)
        return;

    sensor_gconfig_t* gsc = &sensor_interfaces[interface_id]->gsensor_config[sensor_id];
    data[0] = gsc->status;

    message_init(&m, controller_address,
            MESSAGE_NO_REQUEST,
            M_SENSOR_STATUS,
            gsc->interface->interface_id,
            gsc->sensor_id,
            data,
            ARRAY_LENGTH(data));
    message_send(&m);
}

void sensor_set_status(uint8_t interface_id, uint8_t sensor_id, sensor_status_t status) {
    sensor_interface_t* intf = sensor_interfaces[interface_id];

    message_t m_status;
    uint8_t data[1];
    
    sensor_gconfig_t* gsc = &intf->gsensor_config[sensor_id];

    switch (status) {
        case SENSOR_STATUS_ACTIVE:
            if ((gsc->status == SENSOR_STATUS_IDLE) || (gsc->status == SENSOR_STATUS_STOPPED)) {
                if (gsc->measure.task.cb == task_dummy) {
                    break;
                }
                gsc->status = SENSOR_STATUS_ACTIVE;
                switch (gsc->sensor_type) {
                    case SENSOR_NOT_SET:
                        break;
                    case SENSOR_TYPE_SHT35:
                        sensor_sht35_activate(gsc);
                        break;
                    case SENSOR_TYPE_APDS9306_065:
                        sensor_apds9306_065_activate(gsc);
                        break;
                    case SENSOR_TYPE_ADS1219:
                        sensor_ads1219_activate(gsc);
                        break;
#if defined(__DICIO__) || defined(__SYLVATICA__)
                    case SENSOR_TYPE_ADC12:
                        sensor_adc12_activate(gsc);
                        break;
#endif
#if defined(__SYLVATICA__) || defined(__PLANALTA__)
                    case SENSOR_TYPE_ADC16:
                        sensor_adc16_activate(gsc);
                        break;
#endif
#ifdef __PLANALTA__
                    case SENSOR_TYPE_LIA:
                        sensor_lia_activate(gsc);
                        break;
#endif
                    default:
                        break;
                }
                schedule_event(&gsc->measure);
            }
            break;
        case SENSOR_STATUS_STOPPED:
            if (gsc->status == SENSOR_STATUS_RUNNING) {
                schedule_remove_event(gsc->measure.id);
            }
                switch (gsc->sensor_type) {
                    case SENSOR_NOT_SET:
                        break;
                    case SENSOR_TYPE_SHT35:
                        sensor_sht35_deactivate(gsc);
                        break;
                    case SENSOR_TYPE_APDS9306_065:
                        sensor_apds9306_065_deactivate(gsc);
                        break;
                    case SENSOR_TYPE_ADS1219:
                        sensor_ads1219_deactivate(gsc);
                        break;
#if defined(__DICIO__) || defined(__SYLVATICA__)
                    case SENSOR_TYPE_ADC12:
                        sensor_adc12_deactivate(gsc);
                        break;
#endif
#if defined(__SYLVATICA__) || defined(__PLANALTA__)
                    case SENSOR_TYPE_ADC16:
                        sensor_adc16_deactivate(gsc);
                        break;
#endif
#ifdef __PLANALTA__
                    case SENSOR_TYPE_LIA:
                        sensor_lia_deactivate(gsc);
                        break;
#endif
                    default:
                        break;
                }
            
            gsc->status = SENSOR_STATUS_STOPPED;
            break;
        case SENSOR_STATUS_INACTIVE:
            gsc->status = status;
            break;
        case SENSOR_STATUS_RUNNING:
        case SENSOR_STATUS_IDLE:
        case SENSOR_STATUS_ERROR:
            break;
        default:
            break;
    }

    data[0] = gsc->status;
    message_init(&m_status, controller_address, MESSAGE_NO_REQUEST,
            M_SENSOR_STATUS, gsc->interface->interface_id, gsc->sensor_id, data, ARRAY_LENGTH(data));
    message_send(&m_status);
}

void sensor_error_log(sensor_gconfig_t* sensor_config, uint8_t* data, uint8_t length) {
    message_init(&sensor_config->log,
            controller_address,
            MESSAGE_NO_REQUEST,
            M_SENSOR_ERROR,
            sensor_config->interface->interface_id, 
            sensor_config->sensor_id,
            data,
            length);
    message_send(&sensor_config->log);
}

void sensor_error_handle(sensor_gconfig_t *gsc) {
    message_t m_status;
    uint8_t data[1];

    data[0] = gsc->status;
    message_init(&m_status, controller_address, MESSAGE_NO_REQUEST,
            M_SENSOR_STATUS, gsc->interface->interface_id, gsc->sensor_id, data, ARRAY_LENGTH(data));
    message_send(&m_status);

    schedule_remove_event(gsc->measure.id);
}

void sensor_i2c_error_handle(sensor_gconfig_t *gsc, i2c_message_t *m, uint8_t location) {
    gsc->log_data[0] = gsc->status;
    gsc->log_data[1] = m->status;
    gsc->log_data[2] = m->error;
    gsc->log_data[3] = location;

    message_init(&gsc->log,
            controller_address,
            MESSAGE_NO_REQUEST,
            M_SENSOR_STATUS,
            gsc->interface->interface_id, gsc->sensor_id,
            gsc->log_data,
            4);
    message_send(&gsc->log);

    schedule_remove_event(gsc->measure.id);
}

void sensor_start(void) {
    for (int i = 0; i < N_SENSOR_INTERFACES; i++) {
        for(int j = 0; j < SENSOR_INTERFACE_MAX_SENSORS; j++){
            sensor_set_status(i, j, SENSOR_STATUS_ACTIVE);
        }
    }
}

void sensor_stop(void) {
    for (int i = 0; i < N_SENSOR_INTERFACES; i++) {
        for(int j = 0; j < SENSOR_INTERFACE_MAX_SENSORS; j++){
            if (sensor_interfaces[i]->gsensor_config[j].status == SENSOR_STATUS_RUNNING) {
                // update sensor status
                sensor_set_status(i, j, SENSOR_STATUS_STOPPED);
            }
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
#elif defined __SYLVATICA__
    if (sensor_interface_n < 4) {
        return I2C1_BUS;
    } else {
        return I2C2_BUS;
    }
#elif defined __PLANALTA__
    if (sensor_interface_n < 2) {
        return I2C1_BUS;
    } else {
        return I2C2_BUS;
    }
#else
#error "This board is not yet supported"
#endif
}

#ifdef __DICIO__
void sensor_save_data(uint8_t address, uint8_t interface, uint8_t sensor_id, const uint8_t* buffer, size_t length)
{
    uint8_t sd_buffer[CAN_MAX_N_BYTES+9];
    uint16_t ctime[4];
    
    // get current time
    clock_get_raw_time(ctime);
    
    // create new buffer
    sd_buffer[0] = SDCARD_START_BYTE;
    sd_buffer[1] = (uint8_t) (ctime[2] & 0xff);
    sd_buffer[2] = (uint8_t) ((ctime[3] >> 8) & 0xff);
    sd_buffer[3] = (uint8_t) (ctime[3] & 0xff);
    sd_buffer[4] = address;
    sd_buffer[5] = interface;
    sd_buffer[6] = sensor_id;
    sd_buffer[7] = length;
    for(size_t i = 0; i < length; i++)
    {
        sd_buffer[8+i] = buffer[i];
    }
    sd_buffer[8+length] = SDCARD_STOP_BYTE;
    
    // store data
    sdcard_save_data(sd_buffer, 9+length);
}
#endif

void sensor_interface_set_supply(uint8_t interface_id, interface_supply_t supply)
{
    sensor_interface_t* intf = sensor_interfaces[interface_id];
    message_t m_supply;
    uint8_t data[1];
    
    intf->analogue_supply = supply;
    
    data[0] = supply;
    message_init(
            &m_supply, 
            controller_address, 
            MESSAGE_NO_REQUEST,
            M_INTERFACE_SUPPLY, 
            interface_id, 
            NO_SENSOR_ID, data, 
            ARRAY_LENGTH(data));
    message_send(&m_supply);
}
