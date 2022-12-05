#include <sensor_apds9306_065.h>
#include <sensor.h>
#include <address.h>

static void sensor_apds9306_065_config_phase1_cb(i2c_message_t* m);
static void sensor_apds9306_065_config_phase2_cb(i2c_message_t* m);
static void sensor_apds9306_065_config_phase3_cb(i2c_message_t* m);
static void sensor_apds9306_065_config_phase4(sensor_interface_t* intf);
static void sensor_apds9306_065_config_phase4_cb(i2c_message_t* m);
static void sensor_apds9306_065_config_phase5(sensor_interface_t* intf);
static void sensor_apds9306_065_config_phase5_cb(i2c_message_t* m);
static void sensor_apds9306_065_config_phase6_cb(i2c_message_t* m);
static void sensor_apds9306_065_config_phase7_cb(i2c_message_t* m);

void sensor_apds9306_065_get_config(struct sensor_interface_s* intf, uint8_t reg, uint8_t* buffer, uint8_t* length){
    sensor_apds9306_065_config_t* config = &intf->config.apds9306_065;
    int i;
    uint32_t temp;

    buffer[0] = SENSOR_TYPE_APDS9306_065;
    buffer[1] = reg;

    switch (reg) {
        case sensor_apds9306_065_gloxinia_register_general:
            buffer[2] = (uint8_t) (intf->measure.period >> 8);
            buffer[3] = (uint8_t) intf->measure.period;
            *length = 4;
            break;
        case sensor_apds9306_065_gloxinia_register_config:
            buffer[2] = config->address;
            buffer[3] = config->meas_rate;
            buffer[4] = config->meas_resolution;
            buffer[5] = config->gain;
            *length = 6;
            break;
        case sensor_apds9306_065_gloxinia_register_als_th_high:
            temp = config->als_threshold_high;
            for (i = 0; i < 4; i++) {
                buffer[5-i] = (uint8_t) (temp & 0xff);
                temp = temp >> 8;
            }
            *length = 6;
            break;
        case sensor_apds9306_065_gloxinia_register_als_th_low:
            temp = config->als_threshold_low;
            for (i = 0; i < 4; i++) {
                buffer[5-i] = (uint8_t) (temp & 0xff);
                temp = temp >> 8;
            }
            *length = 6;
            break;
        default:
            break;
    }
}

sensor_status_t sensor_apds9306_065_config(struct sensor_interface_s* intf, uint8_t* buffer, uint8_t length) {
    sensor_apds9306_065_config_t* config = &intf->config.apds9306_065;
    int i;

    if (length < 1) {
        return SENSOR_STATUS_ERROR;
    }

    switch (buffer[0]) {
        case sensor_apds9306_065_gloxinia_register_general:
            if (length != 3){ return SENSOR_STATUS_ERROR; }

            intf->measure.task.cb = sensor_apds9306_065_measure;
            intf->measure.task.data = (void*) intf;
            
            schedule_init(&intf->measure, intf->measure.task, (((uint16_t)buffer[1]) << 8) | buffer[2]);
            
            return SENSOR_STATUS_ACTIVE;
            
            break;
        case sensor_apds9306_065_gloxinia_register_config:
            if (length != 6) {
                return SENSOR_STATUS_ERROR;
            }
            config->address = buffer[1];
            config->i2c_bus = (i2c_bus_t) buffer[2];
            config->meas_rate = (sensor_apds9306_065_als_meas_rate_t) buffer[3];
            config->meas_resolution = (sensor_apds9306_065_als_resolution_t) buffer[4];
            config->gain = (sensor_apds9306_065_als_gain_t) buffer[5];
            if(!validate_sensor_apds9306_065_config(config)){
                return SENSOR_STATUS_ERROR;
            } else {
                sensor_apds9306_065_init_sensor(intf);
            }
            break;
        case sensor_apds9306_065_gloxinia_register_als_th_high:
            if (length != 5) {
                return SENSOR_STATUS_ERROR;
            }
            config->als_threshold_high = 0;
            for (i = 0; i < 4; i++) {
                config->als_threshold_high = (config->als_threshold_high << 8) | buffer[i + 1];
            }
            sensor_apds9306_065_config_phase4(intf);
            break;
        case sensor_apds9306_065_gloxinia_register_als_th_low:
            if (length != 5) {
                return SENSOR_STATUS_ERROR;
            }
            config->als_threshold_low = 0;
            for (i = 0; i < 4; i++) {
                config->als_threshold_low = (config->als_threshold_low << 8) | buffer[1 + i];
            }
            sensor_apds9306_065_config_phase5(intf);
            break;
        default:
            break;
    }

    return SENSOR_STATUS_IDLE;
}

void sensor_apds9306_065_init_sensor(struct sensor_interface_s* intf) {
    sensor_apds9306_065_config_t* config = &intf->config.apds9306_065;

    // configure measurement rate
    i2c_init_message(&config->m_config1,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_config1_data,
            2,
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            3,
            sensor_apds9306_065_config_phase1_cb,
            intf,
            0);
    config->m_config1_data[0] = SENSOR_APDS9306_065_R_ALS_MEAS_RATE;
    config->m_config1_data[1] = (config->meas_resolution << 4) | config->meas_rate;

    i2c_queue_message(&config->m_config1);
}

static void sensor_apds9306_065_config_phase1_cb(i2c_message_t* m) {
    sensor_interface_t* intf = (sensor_interface_t*) m->callback_data;
    sensor_apds9306_065_config_t* config = &intf->config.apds9306_065;

    if (m->error != I2C_NO_ERROR) {
        intf->status = SENSOR_STATUS_ERROR;
        sensor_i2c_error_handle(intf, m,  1);
        return;
    }

    // configure measurement gain
    i2c_init_message(&config->m_config2,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_config2_data,
            2,
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            3,
            sensor_apds9306_065_config_phase2_cb,
            intf,
            0);
    config->m_config2_data[0] = SENSOR_APDS9306_065_R_ALS_GAIN;
    config->m_config2_data[1] = config->gain;

    i2c_queue_message(&config->m_config2);
}

static void sensor_apds9306_065_config_phase2_cb(i2c_message_t* m) {
    sensor_interface_t* intf = (sensor_interface_t*) m->callback_data;
    sensor_apds9306_065_config_t* config = &intf->config.apds9306_065;

    if (m->error != I2C_NO_ERROR) {
        intf->status = SENSOR_STATUS_ERROR;
        sensor_i2c_error_handle(intf, m,  2);
        return;
    }

    // configure interrupts
    i2c_init_message(&config->m_config1,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_config1_data,
            2,
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            3,
            sensor_apds9306_065_config_phase3_cb,
            intf,
            0);
    config->m_config1_data[0] = SENSOR_APDS9306_065_R_INT_CFG;
    config->m_config1_data[1] = (0b01 << 4) | (0 << 3) | (0 << 2);
    /**             |          |          |- interrupts disabled   
     *              |          ------------- threshold interrupt mode
     *              ------------------------ ALS channel as interrupt source
     */

    i2c_queue_message(&config->m_config1);
}

static void sensor_apds9306_065_config_phase3_cb(i2c_message_t* m) {
    sensor_interface_t* intf = (sensor_interface_t*) m->callback_data;
    sensor_apds9306_065_config_t* config = &intf->config.apds9306_065;

    if (m->error != I2C_NO_ERROR) {
        intf->status = SENSOR_STATUS_ERROR;
        sensor_i2c_error_handle(intf, m,  3);
        return;
    }

    // configure interrupt persistence    
    i2c_init_message(&config->m_config2,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_config2_data,
            2,
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            3,
            NULL,
            intf,
            0);
    config->m_config2_data[0] = SENSOR_APDS9306_065_R_INT_PERSISTENCE;
    config->m_config2_data[1] = (0b00 << 4); // every ALS value out of the range triggers an interrupt

    i2c_queue_message(&config->m_config2);
}

static void sensor_apds9306_065_config_phase4(sensor_interface_t* intf) {
    sensor_apds9306_065_config_t* config = &intf->config.apds9306_065;

    // configure high threshold
    i2c_init_message(&config->m_config1,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_config1_data,
            4,
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            3,
            sensor_apds9306_065_config_phase4_cb,
            intf,
            0);
    config->m_config1_data[0] = SENSOR_APDS9306_065_R_ALS_THRES_UP_0;
    config->m_config1_data[1] = config->als_threshold_high & 0xff;
    config->m_config1_data[2] = (config->als_threshold_high >> 8) & 0xff;
    config->m_config1_data[3] = (config->als_threshold_high >> 16) & 0xf;

    i2c_queue_message(&config->m_config1);
}

static void sensor_apds9306_065_config_phase4_cb(i2c_message_t* m) {
    sensor_interface_t* intf = (sensor_interface_t*) m->callback_data;

    if (m->error != I2C_NO_ERROR) {
        intf->status = SENSOR_STATUS_ERROR;
        sensor_i2c_error_handle(intf, m,  4);
        return;
    }
}

static void sensor_apds9306_065_config_phase5_cb(i2c_message_t* m) {
    sensor_interface_t* intf = (sensor_interface_t*) m->callback_data;

    if (m->error != I2C_NO_ERROR) {
        intf->status = SENSOR_STATUS_ERROR;
        sensor_i2c_error_handle(intf, m,  5);
        return;
    }
}

static void sensor_apds9306_065_config_phase5(sensor_interface_t* intf) {
    sensor_apds9306_065_config_t* config = &intf->config.apds9306_065;
    
    // configure low threshold
    i2c_init_message(&config->m_config2,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_config2_data,
            4,
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            3,
            sensor_apds9306_065_config_phase5_cb,
            intf,
            0);
    config->m_config2_data[0] = SENSOR_APDS9306_065_R_ALS_THRES_LOW_0;
    config->m_config2_data[1] = config->als_threshold_low & 0xff;
    config->m_config2_data[2] = (config->als_threshold_low >> 8) & 0xff;
    config->m_config2_data[3] = (config->als_threshold_low >> 16) & 0xf;

    i2c_queue_message(&config->m_config2);
}

static void sensor_apds9306_065_config_phase6_cb(i2c_message_t* m) {
    sensor_interface_t* intf = (sensor_interface_t*) m->callback_data;

    if (m->error != I2C_NO_ERROR) {
        intf->status = SENSOR_STATUS_ERROR;
        sensor_i2c_error_handle(intf, m,  6);
        return;
    }
}

static void sensor_apds9306_065_config_phase7_cb(i2c_message_t* m) {
    sensor_interface_t* intf = (sensor_interface_t*) m->callback_data;

    if (m->error != I2C_NO_ERROR) {
        intf->status = SENSOR_STATUS_ERROR;
        sensor_i2c_error_handle(intf, m,  7);
        return;
    } else {
        intf->status = SENSOR_STATUS_ACTIVE;
    }
}

void sensor_apds9306_065_activate(struct sensor_interface_s* intf){
    sensor_apds9306_065_config_t* config = &intf->config.apds9306_065;
    message_t m_status;
    uint8_t data[1];
    
    if((intf->status != SENSOR_STATUS_IDLE) && (intf->status != SENSOR_STATUS_STOPPED)){
        return;
    }
    
    // activate sensor
    i2c_init_message(&config->m_config1,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_config1_data,
            2,
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            3,
            sensor_apds9306_065_config_phase7_cb,
            intf,
            0);

    config->m_config1_data[0] = SENSOR_APDS9306_065_R_MAIN_CTRL;
    config->m_config1_data[1] = (1 << 1); // turn sensor on

    i2c_queue_message(&config->m_config1);
    
    // initialise message to read PD value
    // initialise measurement messages
    config->m_read_address[0] = SENSOR_APDS9306_065_R_ALS_DATA_0; // select DATA register

    i2c_init_message(&config->m_read_setup,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_read_address,
            ARRAY_LENGTH(config->m_read_address),
            config->m_read_data,
            ARRAY_LENGTH(config->m_read_data),
            i2c_get_write_read_controller(config->i2c_bus),
            3,
            sensor_apds9306_065_i2c_cb,
            intf,
            0);
    
    if(intf->status != SENSOR_STATUS_ERROR)
        intf->status = SENSOR_STATUS_RUNNING;

    message_init(&m_status, controller_address, MESSAGE_NO_REQUEST, M_SENSOR_STATUS,
            intf->sensor_id, data, ARRAY_LENGTH(data));
    data[0] = intf->status;

    message_send(&m_status);

    return;
}

void sensor_apds9306_065_i2c_cb(i2c_message_t* m) {
    sensor_interface_t* intf = (sensor_interface_t*) m->callback_data;

    if (m->error != I2C_NO_ERROR) {
        intf->status = SENSOR_STATUS_ERROR;
        sensor_i2c_error_handle(intf, m,  8);
    } else {
        message_init(&intf->log,
                controller_address,
                MESSAGE_NO_REQUEST,
                M_SENSOR_STATUS,
                intf->sensor_id,
                m->read_data,
                SENSOR_APDS3906_098_CAN_DATA_LENGTH);
        message_send(&intf->log);
    }
}

bool validate_sensor_apds9306_065_config(sensor_apds9306_065_config_t* config) {
    sensor_apds9306_065_config_t original = *config;
    
    SENSOR_CONFIG_CHECK_MAX_SET(config->meas_rate, SENSOR_APDS9306_065_ALS_MEAS_RATE_2000MS, SENSOR_APDS9306_065_ALS_MEAS_RATE_2000MS);
    SENSOR_CONFIG_CHECK_MAX_SET(config->meas_resolution, SENSOR_APDS9306_065_ALS_RESOLUTION_13BIT, SENSOR_APDS9306_065_ALS_RESOLUTION_13BIT);
    SENSOR_CONFIG_CHECK_MAX_SET(config->gain, SENSOR_APDS9306_065_ALS_GAIN_18, SENSOR_APDS9306_065_ALS_GAIN_18);
    // max 24 bits wide
    SENSOR_CONFIG_CHECK_MAX_SET(config->als_threshold_high, 0xffffff, 0xffffff);
    SENSOR_CONFIG_CHECK_MAX_SET(config->als_threshold_low, 0xffffff, 0xffffff);
    
    if(config->meas_rate != original.meas_rate){
        return false;
    }
    if(config->meas_resolution != original.meas_resolution){
        return false;
    }
    if(config->gain != original.gain){
        return false;
    }
    if(config->als_threshold_high != original.als_threshold_high){
        return false;
    }
    if(config->als_threshold_low != original.als_threshold_low){
        return false;
    }
    
    return true;
}

void sensor_apds9306_065_measure(void* data) {
    sensor_interface_t* intf = (sensor_interface_t*) data;

    if(i2c_check_message_sent(&intf->config.apds9306_065.m_read_setup)) {
        i2c_reset_message(&intf->config.apds9306_065.m_read_setup, 1);
        i2c_queue_message(&intf->config.apds9306_065.m_read_setup);
    } else {
        UART_DEBUG_PRINT("APDS9306 %x not fully processed.", intf->sensor_id);
        intf->config.apds9306_065.m_read_setup.status = I2C_MESSAGE_CANCELED;
        
        intf->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(intf);
    }
}