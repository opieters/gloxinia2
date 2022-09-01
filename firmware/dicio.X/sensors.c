#include <xc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "sensors.h"
#include "address.h"
#include "dicio.h"


uart_message_t serial_meas_trigger;
can_message_t can_meas_trigger;

volatile bool sensor_error_detected = false;
volatile bool sensor_reset_env_sensors = false;
volatile bool sensor_reset_plant_sensors = false;

// stored sensor configurations
sensor_config_t sensor_configs[MAX_N_SENSORS];
static uint8_t n_connected_sensors = 0;

void register_sensor(uint8_t address, uint8_t* data){
}

i2c_message_t config_m, config_mc;

void config_sensor(uint8_t address, i2c_bus_t i2c_bus, bool write, bool read, 
        uint8_t* write_data, size_t write_length, uint8_t* read_data, size_t read_length){
    void (*controller)(i2c_message_t*);
    
    i2c_message_t* config_m = malloc(sizeof(i2c_message_t));
    
    switch(i2c_bus){
        case I2C1_BUS:
            if(write && read){
                controller = i2c1_write_read_controller;
            } else if(write){
                controller = i2c1_write_controller;
            } else if(read){
                controller = i2c1_read_controller;
            } else {
                return;
            }
            break;
        case I2C2_BUS:
            if(write && read){
                controller = i2c2_write_read_controller;
            } else if(write){
                controller = i2c2_write_controller;
            } else if(read){
                controller = i2c2_read_controller;
            } else {
                return;
            }
            break;
        default:  
            return;
            break;
    }
    
    // init messages -> if we only read, write_data is not used
    if(write){
        i2c_init_message(
            config_m, 
            I2C_WRITE_ADDRESS(address),
            i2c_bus,
            write_data,
            write_length, 
            read_data,
            read_length, 
            controller, 
            3, 
            i2c_free_callback, 
            NULL);
    } else {
        i2c_init_message(
            config_m, 
            I2C_READ_ADDRESS(address),
            i2c_bus,
            write_data,
            write_length, 
            read_data,
            read_length, 
            controller, 
            3, 
            i2c_free_callback, 
            NULL);
    }
    
    i2c_queue_message(config_m);
}

void register_i2c_message(sensor_config_t* config, i2c_message_t* ref_m, sensor_message_phase_t ref_phase){
    uint8_t *write_data, *read_data;
    i2c_message_t *m;
    
    // allocate data
    if(config->n_messages > 0){
        config->messages = realloc(config->messages, sizeof(i2c_message_t)*(config->n_messages+1));
        config->phases = realloc(config->phases, sizeof(sensor_message_phase_t)*(config->n_messages+1));
        config->n_messages++;
    } else {
        config->messages = malloc(sizeof(i2c_message_t));
        config->phases = malloc(sizeof(sensor_message_phase_t));
        config->n_messages = 1;
    }
    
    m = &config->messages[config->n_messages-1];
    
    // allocate data if needed and copy
    if(ref_m->read_length > 0){
        read_data = malloc(ref_m->read_length*sizeof(uint8_t));
        memcpy(read_data, ref_m->read_data, ref_m->read_length);
    } else {
        read_data = NULL;
    }
    if(ref_m->write_length > 0){
        write_data = malloc(ref_m->write_length*sizeof(uint8_t));
        memcpy(write_data, ref_m->write_data, ref_m->write_length);
    } else {
        write_data = NULL;
    }
    
    // copy data
    config->phases[config->n_messages-1] = ref_phase;

    i2c_init_message(
            m, 
            ref_m->i2c_bus,
            ref_m->address, 
            write_data,
            ref_m->write_length, 
            read_data,
            ref_m->read_length, 
            ref_m->controller, 
            ref_m->n_attempts, 
            i2c_dummy_callback,
            NULL);
    
}

i2c_error_t setup_sensor(uint8_t can_data_lenght){
    //void (*controller)(i2c_message_t* m);
    
    sensor_init_common_config(&sensor_configs[n_connected_sensors], can_data_lenght);
    
    n_connected_sensors++;
    
    // ERROR??
    return I2C_NO_ERROR;
}



bool sensors_init_plant(void) {
    bool init_status = true;
    
    return init_status;
}

void sensors_init(void){
    sensors_reset();
    
    /*i2c_add_reset_callback(I2C2_BUS, sensors_reset_environmental, 
            sensors_init_environmental);*/
    
    delay_ms(1000);
    
    sensors_init_plant();
    
    //i2c_add_reset_callback(I2C1_BUS, sensors_reset_plant, sensors_init_plant);
    
    if(controller_address == 0){
        uart_init_message(&serial_meas_trigger,
            SERIAL_MEAS_TRIGGER_CMD,
            controller_address,
            CAN_HEADER(CAN_INFO_CMD_MEASUREMENT_START, 0),            
            NULL,
            0);
    }
    
    dicio_set_sensor_callback(sensor_callback);
    
    // report about all sensor status to host
    sensor_status_report();
    
}

void sensors_reset(void){
    // TODO
    
}



void sensor_i2c_callback(void){

    
#ifdef __ENABLE_SYLVATICA2__
    sensor_sylvatica2_cb();
#endif
    

}

bool sensors_plant_reset_check(void) {
    return false;
}

void sensors_error_check(void){
    sensor_error_detected = sensor_error_detected || sensor_reset_env_sensors;
} 

void sensors_error_recover(void){
    
    if(!sensor_error_detected){
        return;
    }
    
    #ifdef ENABLE_DEBUG        
        uart_simple_print("Sensor readout error occurred. Resetting...");
    #endif 

    delay_ms(100);

    asm("RESET");
        
    // make sure I2C queue is empty
    i2c_empty_queue();
    
    sensor_status_report();
    
    sensor_error_detected = false;
}


void sensor_callback(void){
    static uint8_t interrupt_counter = 0;
    
    interrupt_counter++;
    
    if(((interrupt_counter % 10) == 0) && (controller_address == 0)){
        uart_reset_message(&serial_meas_trigger);
        uart_queue_message(&serial_meas_trigger);
    }
    
    if(sensor_error_detected){
        return;
    }
    
    switch(interrupt_counter){
        case 10:
            sensor_i2c_callback();
            sensors_error_check();
            //sensor_ds18b20_start_measurement();
            break;
        case 20:
            sensor_i2c_callback();
            sensors_error_check();
            //sensor_ds18b20_cb();
            break;
        case 30:
            sensor_i2c_callback();
            sensors_error_check();
            
            interrupt_counter = 0;
            
            break;
        default:
            break;
    }
}

void sensors_start(void){
}


void send_sensor_status(sensor_config_t* config){
    uint8_t data[DICIO_SENSOR_STATUS_LOG_MESSAGE];
    
    data[0] = config->global_id;
    data[1] = config->local_id;
    data[2] = config->status;
    
    dicio_send_message(SERIAL_SENSOR_STATUS_CMD, 
            CAN_HEADER(CAN_MSG_SENSOR_STATUS, 0), data, 
            DICIO_SENSOR_STATUS_LOG_MESSAGE);
}

void sensor_status_report(void){
}

void sensors_data_init(void){    
}
