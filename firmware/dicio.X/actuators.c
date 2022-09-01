#include "actuators.h"
#include "dicio.h"

bool actuator_error_detected = false;


uart_message_t serial_actuator_trigger;
can_message_t can_actuator_trigger;



void actuators_init(void){
    i2c_empty_queue();
    
    actuator_status_report();
}

void actuator_callback(void) {

}

void actuators_start(void){
    
}

void actuators_data_init(void){
   
}

bool process_actuator_serial_command(uart_message_t* m){
    return true;
}

void actuators_error_check(void){
   
}

void actuators_error_recover(void){
   
}

void actuator_send_status(actuator_general_config_t* config){
    uint8_t data[DICIO_ACTUATOR_STATUS_LOG_MESSAGE];
    
    data[0] = config->global_id;
    data[1] = config->local_id;
    data[2] = config->status;
    
    dicio_send_message(SERIAL_ACTUATOR_STATUS_CMD, 
            CAN_HEADER(CAN_MSG_ACTUATOR_STATUS, 0), data, 
            DICIO_ACTUATOR_STATUS_LOG_MESSAGE);
}

void actuator_status_report(void){

}
