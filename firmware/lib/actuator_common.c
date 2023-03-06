#include "actuator_common.h"
#include "can.h"
#include "address.h"

static uint8_t local_id = 0;

void update_actuator_config_status(actuator_general_config_t* config, const i2c_error_t i2c_error) {
    if (i2c_error != I2C_NO_ERROR) {
        config->status = ACTUATOR_STATE_ERROR;
    } else {
        config->status = ACTUATOR_STATE_IDLE;
    }
}

void actuator_send_error(actuator_general_config_t* config, i2c_message_t* m) {    
    config->elog.data[2] = m->status;
    config->elog.data[3] = m->error;
    
    send_message(&config->elog);
}

void actuator_send_data(actuator_general_config_t* config, uint8_t* data, uint8_t length) {
    uint16_t i;
    message_t* m = &config->dlog;

    m->length = MAX(m->length, length);

    for (i = 0; i < m->length; i++) {
        m->data[i] = data[i];
    }

    send_message(m);
}

void send_actuator_data_no_copy(actuator_general_config_t* config) {
    send_message(&config->dlog);
}

uint8_t get_actuator_local_id(void) {
    return local_id++;
}

void reset_actuator_local_id(void) {
    local_id = 0;
}

void actuator_init_common_config(actuator_general_config_t* general, uint8_t length) {
    // TODO
    /*init_message(&general->dlog,
            controller_address,
            CAN_NO_REMOTE_FRAME,
            CAN_EXTENDED_FRAME,
            CAN_HEADER(general->global_id, general->local_id),
            general->tx_data,
            length);


    init_message(&general->elog,
            controller_address,
            CAN_NO_REMOTE_FRAME,
            CAN_EXTENDED_FRAME,
            CAN_HEADER(CAN_MSG_ACTUATOR_ERROR, 0U),
            general->error_data,
            ERROR_DATA_SIZE);


    general->error_data[0] = general->global_id;
    general->error_data[1] = general->local_id;*/
}
