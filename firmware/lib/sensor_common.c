#include "sensor_common.h"
#include <can.h>
#include "address.h"

static uint8_t sensor_id = 0;

void sensor_init_common_config(sensor_config_t* general, uint8_t length) {
    init_message(&general->dlog,
            controller_address,
            CAN_NO_REMOTE_FRAME,
            M_SENSOR_DATA,
            general->sensor_id,
            general->tx_data,
            length,
            NO_INTERFACE);
    init_message(&general->elog,
            controller_address,
            CAN_NO_REMOTE_FRAME,
            M_SENSOR_ERROR,
            general->sensor_id,
            general->error_data,
            ERROR_DATA_SIZE,
        NO_INTERFACE);

}

void sensor_update_status(sensor_config_t* config, const i2c_error_t i2c_error) {
    if (i2c_error != I2C_NO_ERROR) {
        config->status = SENSOR_STATUS_ERROR;
    } else {
        if (config->status != SENSOR_STATUS_ERROR) {
            config->status = SENSOR_STATUS_ACTIVE;
        }
    }
}

void sensor_send_error(message_t* elog, i2c_message_t* m) {
    sensor_send_general_error(elog, m->status, m->error);
}

void sensor_send_general_error(message_t* elog, uint8_t status, uint8_t error) {
    elog->data[2] = status;
    elog->data[3] = error;
    send_message(elog);

}

void sensor_send_data(message_t* m, uint8_t* data, uint8_t length) {
    uint16_t i;

    length = MIN(length, m->length);
    for (i = 0; i < length; i++) {
        m->data[i] = data[i];
    }
    m->length = length;

    send_message(m);

}

void sensor_send_data_no_copy(message_t* m, uint8_t* data, uint8_t length) {
    m->length = length;
    m->data = data;

    send_message(m);
}

uint8_t sensor_get_sensor_id(void) {
    return sensor_id++;
}

void sensor_reset_sensor_id(void) {
    sensor_id = 0;
}