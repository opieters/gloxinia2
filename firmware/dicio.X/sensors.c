#include <xc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "sensors.h"
#include "address.h"
#include "dicio.h"
#include "sensor_sht35.h"

#include <uart_logging.h>


uart_message_t serial_meas_trigger;
can_message_t can_meas_trigger;

volatile bool sensor_error_detected = false;
volatile bool sensor_reset_env_sensors = false;
volatile bool sensor_reset_plant_sensors = false;

// stored sensor configurations
sensor_config_t sensor_configs[MAX_N_SENSORS];
static uint8_t n_connected_sensors = 0;

void register_sensor(uint8_t address, uint8_t* data) {
}

i2c_message_t config_m, config_mc;

void config_sensor(uint8_t address, i2c_bus_t i2c_bus, bool write, bool read,
        uint8_t* write_data, size_t write_length, uint8_t* read_data, size_t read_length) {
    void (*controller)(i2c_message_t*);

    i2c_message_t* config_m = malloc(sizeof (i2c_message_t));

    switch (i2c_bus) {
        case I2C1_BUS:
            if (write && read) {
                controller = i2c1_write_read_controller;
            } else if (write) {
                controller = i2c1_write_controller;
            } else if (read) {
                controller = i2c1_read_controller;
            } else {
                return;
            }
            break;
        case I2C2_BUS:
            if (write && read) {
                controller = i2c2_write_read_controller;
            } else if (write) {
                controller = i2c2_write_controller;
            } else if (read) {
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
    if (write) {
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
                NULL,
                0);
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
                NULL,
                0);
    }

    i2c_queue_message(config_m);
}

i2c_error_t setup_sensor(uint8_t can_data_lenght) {
    //void (*controller)(i2c_message_t* m);

    sensor_init_common_config(&sensor_configs[n_connected_sensors], can_data_lenght);

    n_connected_sensors++;

    // ERROR??
    return I2C_NO_ERROR;
}

void sensors_init(void) {

    /*i2c_add_reset_callback(I2C2_BUS, sensors_reset_environmental, 
            sensors_init_environmental);*/

    delay_ms(1000);

    //i2c_add_reset_callback(I2C1_BUS, sensors_reset_plant, sensors_init_plant);

    if (controller_address == 0) {
        uart_init_message(&serial_meas_trigger,
                SERIAL_MEAS_TRIGGER_CMD,
                controller_address,
                CAN_HEADER(CAN_INFO_CMD_MEASUREMENT_START, 0),
                NULL,
                0);
    }

    dicio_set_sensor_callback(sensor_callback);
}

void sensor_i2c_callback(void) {


#ifdef __ENABLE_SYLVATICA2__
    sensor_sylvatica2_cb();
#endif


}

bool sensors_plant_reset_check(void) {
    return false;
}

void sensors_error_check(void) {
    sensor_error_detected = sensor_error_detected || sensor_reset_env_sensors;
}

void sensors_error_recover(void) {

    if (!sensor_error_detected) {
        return;
    }


    UART_DEBUG_PRINT("Sensor readout error occurred. Resetting...");

    delay_ms(100);

    asm("RESET");

    // make sure I2C queue is empty
    i2c_empty_queue();

    sensor_error_detected = false;
}

void sensor_callback(void) {
    static uint8_t interrupt_counter = 0;

    interrupt_counter++;

    if (((interrupt_counter % 10) == 0) && (controller_address == 0)) {
        uart_reset_message(&serial_meas_trigger);
        uart_queue_message(&serial_meas_trigger);
    }

    if (sensor_error_detected) {
        return;
    }

    switch (interrupt_counter) {
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

void send_sensor_status(sensor_config_t* config) {
    uint8_t data[DICIO_SENSOR_STATUS_LOG_MESSAGE];

    data[0] = config->sensor_type;
    data[1] = config->sensor_id;
    data[2] = config->status;

    dicio_send_message(SERIAL_SENSOR_STATUS_CMD,
            CAN_HEADER(CAN_MSG_SENSOR_STATUS, 0), data,
            DICIO_SENSOR_STATUS_LOG_MESSAGE);
}

void populate_sht35_config(uint8_t* data, uint8_t len, sensor_sht35_config_t* config) {
    // format:
    // [sensor_type, sensor_id, address, i2c_bus, ]
    if (len != 8) {
        return;
    }
    config->general.sensor_type = data[0];
    config->general.sensor_id = data[1];
    config->address = data[2];
    config->i2c_bus = data[3];
    config->repeatability = data[4];
    config->clock = data[5];
    config->rate = data[6];
    config->periodicity = data[7];

    validate_sht35_config(config);
}
