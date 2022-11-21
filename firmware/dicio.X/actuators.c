#include "actuators.h"
#include "dicio.h"
#include "address.h"

bool actuator_error_detected = false;


uart_message_t serial_actuator_trigger;
can_message_t can_actuator_trigger;

void actuator_callback(void) {

}

bool process_actuator_serial_command(uart_message_t* m) {
    return true;
}

void actuators_error_check(void) {

}

void actuators_error_recover(void) {

}

void actuator_send_status(actuator_general_config_t* config) {
    message_t m;
    uint8_t data[1] = {config->status};
    init_message(&m, controller_address,
        0,
        M_ACTUATOR_STATUS,
        config->local_id,
        data,
        DICIO_ACTUATOR_STATUS_LOG_MESSAGE,
        NO_INTERFACE);

    send_message(&m);
}
