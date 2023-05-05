#include <sensor_apds9306_065.h>
#include <sensor.h>
#include <address.h>

static void sensor_apds9306_065_config_phase1_cb(i2c_message_t* m);
static void sensor_apds9306_065_config_phase2_cb(i2c_message_t* m);
static void sensor_apds9306_065_config_phase3_cb(i2c_message_t* m);
static void sensor_apds9306_065_config_phase4(sensor_gconfig_t* intf);
static void sensor_apds9306_065_config_phase4_cb(i2c_message_t* m);
static void sensor_apds9306_065_config_phase5(sensor_gconfig_t* intf);
static void sensor_apds9306_065_config_phase5_cb(i2c_message_t* m);
//static void sensor_apds9306_065_config_phase6_cb(i2c_message_t* m);
//static void sensor_apds9306_065_config_phase7_cb(i2c_message_t* m);

void sensor_apds9306_065_get_config(struct sensor_gconfig_s* gsc, uint8_t reg, uint8_t* buffer, uint8_t* length){
    sensor_apds9306_065_config_t* config = &gsc->sensor_config.apds9306_065;
    int i;
    uint32_t temp;

    buffer[0] = SENSOR_TYPE_APDS9306_065;
    buffer[1] = reg;

    switch (reg) {
        case sensor_apds9306_065_gloxinia_register_general:
            gsc->measure.task.cb = sensor_apds9306_065_measure;
            gsc->measure.task.data = (void *)gsc;

            buffer[2] = (uint8_t) (gsc->measure.period >> 8);
            buffer[3] = (uint8_t) gsc->measure.period;
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
            *length = 0;
            break;
    }
}

sensor_status_t sensor_apds9306_065_config(struct sensor_gconfig_s* gsc, uint8_t* buffer, uint8_t length) {
    sensor_apds9306_065_config_t* config = &gsc->sensor_config.apds9306_065;
    int i;

    if (length < 1) {
        return SENSOR_STATUS_ERROR;
    }

    UART_DEBUG_PRINT("Configuring APDS9306 065 sensor");

    switch (buffer[0]) {
        case sensor_apds9306_065_gloxinia_register_general:
            if (length != 3){ return SENSOR_STATUS_ERROR; }

            gsc->measure.task.cb = sensor_apds9306_065_measure;
            gsc->measure.task.data = (void*) gsc;
            
            schedule_init(&gsc->measure, gsc->measure.task, (((uint16_t)buffer[1]) << 8) | buffer[2]);
            
            return SENSOR_STATUS_IDLE;
            
            break;
        case sensor_apds9306_065_gloxinia_register_config:
            if (length != 6) {
                return SENSOR_STATUS_ERROR;
            }
            config->address = buffer[1];
            config->i2c_bus = sensor_get_i2c_bus(gsc->interface->interface_id);
            config->meas_rate = (sensor_apds9306_065_als_meas_rate_t) buffer[3];
            config->meas_resolution = (sensor_apds9306_065_als_resolution_t) buffer[4];
            config->gain = (sensor_apds9306_065_als_gain_t) buffer[5];
            if(!validate_sensor_apds9306_065_config(config)){
                return SENSOR_STATUS_ERROR;
            } else {
                sensor_apds9306_065_init_sensor(gsc);
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
            sensor_apds9306_065_config_phase4(gsc);
            break;
        case sensor_apds9306_065_gloxinia_register_als_th_low:
            if (length != 5) {
                return SENSOR_STATUS_ERROR;
            }
            config->als_threshold_low = 0;
            for (i = 0; i < 4; i++) {
                config->als_threshold_low = (config->als_threshold_low << 8) | buffer[1 + i];
            }
            sensor_apds9306_065_config_phase5(gsc);
            break;
        default:
            break;
    }

    return SENSOR_STATUS_IDLE;
}

void sensor_apds9306_065_init_sensor(sensor_gconfig_t* sgc) {
    sensor_apds9306_065_config_t* config = &sgc->sensor_config.apds9306_065;

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
            sgc,
            0);
    config->m_config1_data[0] = SENSOR_APDS9306_065_R_ALS_MEAS_RATE;
    config->m_config1_data[1] = (config->meas_resolution << 4) | config->meas_rate;

    i2c_queue_message(&config->m_config1);
    
    UART_DEBUG_PRINT("Configuring APDS9306 065 measurement rate");

    // configure readout message
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
        sgc,
        0);
    
    config->m_read_address[0] = SENSOR_APDS9306_065_R_CLEAR_DATA_0;
}

static void sensor_apds9306_065_config_phase1_cb(i2c_message_t* m) {
    sensor_gconfig_t* sgc = (sensor_gconfig_t*) m->callback_data;
    sensor_apds9306_065_config_t* config = &sgc->sensor_config.apds9306_065;

    if (m->error != I2C_NO_ERROR) {
        sgc->log_data[0] = m->status;
        sgc->log_data[1] = m->error;
        sgc->log_data[2] = S_APDS9306_065_ERROR_PHASE1_CB;

        sensor_error_log(sgc, sgc->log_data, 3);
        
        sgc->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(sgc);
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
            sgc,
            0);
    config->m_config2_data[0] = SENSOR_APDS9306_065_R_ALS_GAIN;
    config->m_config2_data[1] = config->gain;

    i2c_queue_message(&config->m_config2);
    
    UART_DEBUG_PRINT("Configuring APDS9306 065 gain");
}

static void sensor_apds9306_065_config_phase2_cb(i2c_message_t* m) {
    sensor_gconfig_t* sgc = (sensor_gconfig_t*) m->callback_data;
    sensor_apds9306_065_config_t* config = &sgc->sensor_config.apds9306_065;

    if (m->error != I2C_NO_ERROR) {
        sgc->log_data[0] = m->status;
        sgc->log_data[1] = m->error;
        sgc->log_data[2] = S_APDS9306_065_ERROR_PHASE2_CB;

        sensor_error_log(sgc, sgc->log_data, 3);
        
        sgc->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(sgc);
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
            sgc,
            0);
    config->m_config1_data[0] = SENSOR_APDS9306_065_R_INT_CFG;
    config->m_config1_data[1] = (0b01 << 4) | (0 << 3) | (0 << 2);
    /**             |          |          |- interrupts disabled   
     *              |          ------------- threshold interrupt mode
     *              ------------------------ ALS channel as interrupt source
     */

    i2c_queue_message(&config->m_config1);
    
    UART_DEBUG_PRINT("Configuring APDS9306 065 interrupts");
}

static void sensor_apds9306_065_config_phase3_cb(i2c_message_t* m) {
    sensor_gconfig_t* sgc = (sensor_gconfig_t*) m->callback_data;
    sensor_apds9306_065_config_t* config = &sgc->sensor_config.apds9306_065;

    if (m->error != I2C_NO_ERROR) {
        sgc->log_data[0] = m->status;
        sgc->log_data[1] = m->error;
        sgc->log_data[2] = S_APDS9306_065_ERROR_PHASE3_CB;

        sensor_error_log(sgc, sgc->log_data, 3);
        
        sgc->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(sgc);
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
            sgc,
            0);
    config->m_config2_data[0] = SENSOR_APDS9306_065_R_INT_PERSISTENCE;
    config->m_config2_data[1] = (0b00 << 4); // every ALS value out of the range triggers an interrupt

    i2c_queue_message(&config->m_config2);
    
    UART_DEBUG_PRINT("Configuring APDS9306 065 persistence");
}

static void sensor_apds9306_065_config_phase4(sensor_gconfig_t* sgc) {
    sensor_apds9306_065_config_t* config = &sgc->sensor_config.apds9306_065;

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
            sgc,
            0);
    config->m_config1_data[0] = SENSOR_APDS9306_065_R_ALS_THRES_UP_0;
    config->m_config1_data[1] = config->als_threshold_high & 0xff;
    config->m_config1_data[2] = (config->als_threshold_high >> 8) & 0xff;
    config->m_config1_data[3] = (config->als_threshold_high >> 16) & 0xf;

    i2c_queue_message(&config->m_config1);
    
    UART_DEBUG_PRINT("Configuring APDS9306 065 TH high");
}

static void sensor_apds9306_065_config_phase4_cb(i2c_message_t* m) {
    sensor_gconfig_t* sgc = (sensor_gconfig_t*) m->callback_data;

    if (m->error != I2C_NO_ERROR) {
        sgc->log_data[0] = m->status;
        sgc->log_data[1] = m->error;
        sgc->log_data[2] = S_APDS9306_065_ERROR_PHASE4_CB;

        sensor_error_log(sgc, sgc->log_data, 3);
        
        sgc->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(sgc);
        return;
    }
}

static void sensor_apds9306_065_config_phase5_cb(i2c_message_t* m) {
    sensor_gconfig_t* sgc = (sensor_gconfig_t*) m->callback_data;

    if (m->error != I2C_NO_ERROR) {
        sgc->log_data[0] = m->status;
        sgc->log_data[1] = m->error;
        sgc->log_data[2] = S_APDS9306_065_ERROR_PHASE5_CB;

        sensor_error_log(sgc, sgc->log_data, 3);
        
        sgc->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(sgc);
        return;
    }
}

static void sensor_apds9306_065_config_phase5(sensor_gconfig_t* gconfig) {
    sensor_apds9306_065_config_t* config = &gconfig->sensor_config.apds9306_065;
    
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
            gconfig,
            0);
    config->m_config2_data[0] = SENSOR_APDS9306_065_R_ALS_THRES_LOW_0;
    config->m_config2_data[1] = config->als_threshold_low & 0xff;
    config->m_config2_data[2] = (config->als_threshold_low >> 8) & 0xff;
    config->m_config2_data[3] = (config->als_threshold_low >> 16) & 0xf;

    i2c_queue_message(&config->m_config2);
    
    UART_DEBUG_PRINT("Configuring APDS9306 065 TH low");
}

/*static void sensor_apds9306_065_config_phase6_cb(i2c_message_t* m) {
    sensor_interface_t* intf = (sensor_interface_t*) m->callback_data;

    if (m->error != I2C_NO_ERROR) {
        intf->log_data[0] = m->status;
        intf->log_data[1] = m->error;
        intf->log_data[2] = S_APDS9306_065_ERROR_PHASE6_CB;

        sensor_error_log(intf, intf->log_data, 3);
        
        intf->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(intf);
        return;
    }
}

static void sensor_apds9306_065_config_phase7_cb(i2c_message_t* m) {
    sensor_interface_t* intf = (sensor_interface_t*) m->callback_data;

    if (m->error != I2C_NO_ERROR) {
        intf->log_data[0] = m->status;
        intf->log_data[1] = m->error;
        intf->log_data[2] = S_APDS9306_065_ERROR_PHASE7_CB;

        sensor_error_log(intf, intf->log_data, 3);
        
        intf->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(intf);
        return;
    } else {
        intf->status = SENSOR_STATUS_IDLE;
    }
}*/

void sensor_apds9306_065_activate(sensor_gconfig_t* gconfig){
    // activate sensor 
    sensor_apds9306_065_config_t* config = &gconfig->sensor_config.apds9306_065;
    
    i2c_init_message(&config->m_config1,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_config1_data,
            2,
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            3,
            NULL,
            gconfig,
            0);
    config->m_config1_data[0] = SENSOR_APDS9306_065_R_MAIN_CTRL;
    config->m_config1_data[1] = (1 << 1); // turn sensor on

    i2c_queue_message(&config->m_config1);
    
    config->m_read_data[0] = 0xAA;
    config->m_read_data[0] = 0xBB;
    config->m_read_data[0] = 0xCC;
    
    UART_DEBUG_PRINT("Activating sensor ADPS9306 065");
    
    gconfig->status = SENSOR_STATUS_RUNNING;
}

void sensor_apds9306_065_i2c_cb(i2c_message_t* m) {
    sensor_gconfig_t* sgc = (sensor_gconfig_t*) m->callback_data;

    if (m->error != I2C_NO_ERROR) {
        sgc->log_data[0] = m->status;
        sgc->log_data[1] = m->error;
        sgc->log_data[2] = S_APDS9306_065_ERROR_READOUT;

        sensor_error_log(sgc, sgc->log_data, 3);
        
        sgc->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(sgc);
    } else {
        message_init(&sgc->log,
                controller_address,
                MESSAGE_NO_REQUEST,
                M_SENSOR_DATA,
                sgc->sensor_id | (sgc->interface->interface_id << 4),
                m->read_data,
                SENSOR_APDS3906_065_CAN_DATA_LENGTH);
        message_send(&sgc->log);
        
#ifdef __DICIO__
        sdcard_save_sensor_data(SENSOR_TYPE_APDS9306_065, m->read_data, SENSOR_APDS3906_065_CAN_DATA_LENGTH);
#endif
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
    sensor_gconfig_t* sgc = (sensor_gconfig_t*) data;

    if(i2c_check_message_sent(&sgc->sensor_config.apds9306_065.m_read_setup)) {
        i2c_reset_message(&sgc->sensor_config.apds9306_065.m_read_setup, 1);
        i2c_queue_message(&sgc->sensor_config.apds9306_065.m_read_setup);
    } else {
        UART_DEBUG_PRINT("APDS9306 065 %x not fully processed.", sgc->sensor_id | (sgc->interface->interface_id << 4));
        sgc->sensor_config.apds9306_065.m_read_setup.status = I2C_MESSAGE_CANCELED;
        
        sgc->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(sgc);
    }
}