#include <sensor_sht35.h>
#include <sensor.h>
#include <address.h>

static void sht35_config_phase1_cb(i2c_message_t* m);
static void sht35_config_phase2_cb(i2c_message_t* m);

sensor_status_t sensor_sht35_config(struct sensor_interface_s* intf, uint8_t* buffer, uint8_t length) {
    if (length < 6) {
        return SENSOR_STATUS_ERROR;
    }

    intf->measure.task.cb = sensor_sht35_measure;
    intf->measure.task.data = (void*) intf;

    sensor_sht35_config_t* config = &intf->config.sht35;

    // load configuration from buffer into data structure
    config->address = buffer[0];
    config->i2c_bus = (i2c_bus_t) buffer[1];
    config->repeatability = (sensor_sht35_repeatability_t) buffer[2];
    config->clock = (sensor_sht35_clock_stretching_t) buffer[3];
    config->rate = (sensor_sht35_sample_rate_t) buffer[4];
    config->periodicity = (sensor_sht35_periodicity_t) buffer[5];

    // validate configuration
    validate_sht35_config(config);

    // start sensor initialisation (async)
    sht35_init_sensor(intf);

    return SENSOR_STATUS_IDLE;
}

void sensor_sht35_measure(void* data) {
    struct sensor_interface_s* intf = (struct sensor_interface_s*) data;


    if (intf->config.sht35.periodicity == S_SHT35_SINGLE_SHOT) {
        if (i2c_check_message_sent(&intf->config.sht35.m_read) &&
                i2c_check_message_sent(&intf->config.sht35.m_config)) {
            i2c_reset_message(&intf->config.sht35.m_read, 1);
            i2c_queue_message(&intf->config.sht35.m_read);
            i2c_reset_message(&intf->config.sht35.m_config, 1);
            i2c_queue_message(&intf->config.sht35.m_config);
        } else {
            UART_DEBUG_PRINT("SHT35 on %x not fully processed.", intf->sensor_id);
            intf->config.sht35.m_read.status = I2C_MESSAGE_CANCELED;
            intf->config.sht35.m_config.status = I2C_MESSAGE_CANCELED;
            
            intf->status = SENSOR_STATUS_ERROR;
            sensor_error_handle(intf);
        }
    } else {
        if (i2c_check_message_sent(&intf->config.sht35.m_fetch)) {
            i2c_reset_message(&intf->config.sht35.m_fetch, 1);
            i2c_queue_message(&intf->config.sht35.m_fetch);
        } else {
            UART_DEBUG_PRINT("SHT35 %x not fully processed.", intf->sensor_id);
            intf->status = SENSOR_STATUS_ERROR;

            sensor_error_handle(intf);
        }
    }
}

void sht35_init_sensor(struct sensor_interface_s* intf) {
    void (*callback)(i2c_message_t * m);

    sensor_sht35_config_t* config = &intf->config.sht35;

    switch (intf->config.sht35.periodicity) {
        case S_SHT35_SINGLE_SHOT:
            callback = sht35_i2c_cb_single_shot_m_read;
            break;
        case S_SHT35_PERIODIC:
            callback = sht35_i2c_cb_periodic_m_read;
            break;
        default:
            UART_DEBUG_PRINT("SHT35: periodicity option not supported.");
    }

    i2c_init_message(
            &config->m_read,
            I2C_READ_ADDRESS(config->address),
            config->i2c_bus,
            I2C_NO_DATA,
            0,
            config->m_read_data,
            SENSOR_SHT35_DATA_LENGTH,
            i2c_get_read_controller(config->i2c_bus),
            3,
            callback,
            (uint8_t*) intf,
            0);


    // send break command to stop acquisition if one is ongoing
    config->m_config_data[0] = 0x30;
    config->m_config_data[1] = 0x93;

    i2c_init_message(
            &config->m_config,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_config_data,
            SENSOR_SHT35_CONFIG_DATA_LENGTH,
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            3,
            sht35_config_phase1_cb,
            (uint8_t*) intf,
            0);

    i2c_queue_message(&config->m_config);

}

static void sht35_config_phase1_cb(i2c_message_t* m) {
    struct sensor_interface_s* intf = (struct sensor_interface_s*) m->callback_data;
    sensor_sht35_config_t* config = (sensor_sht35_config_t*) & intf->config.sht35;

    if (config->periodicity == S_SHT35_PERIODIC) {
        switch (config->rate) {
            case S_SHT35_0_5_MPS:
                config->m_config_data[0] = 0x20;
                switch (config->repeatability) {
                    case S_SHT35_HIGH_REPEATABILIBTY:
                        config->m_config_data[1] = 0x32;
                        break;
                    case S_SHT35_MEDIUM_REPEATABILITY:
                        config->m_config_data[1] = 0x24;
                        break;
                    default:
                        config->m_config_data[1] = 0x2F;
                        break;
                }
                break;
            case S_SHT35_1_MPS:
                config->m_config_data[0] = 0x21;
                switch (config->repeatability) {
                    case S_SHT35_HIGH_REPEATABILIBTY:
                        config->m_config_data[1] = 0x30;
                        break;
                    case S_SHT35_MEDIUM_REPEATABILITY:
                        config->m_config_data[1] = 0x26;
                        break;
                    default:
                        config->m_config_data[1] = 0x2D;
                        break;
                }
                break;
            case S_SHT35_2_MPS:
                config->m_config_data[0] = 0x22;
                switch (config->repeatability) {
                    case S_SHT35_HIGH_REPEATABILIBTY:
                        config->m_config_data[1] = 0x36;
                        break;
                    case S_SHT35_MEDIUM_REPEATABILITY:
                        config->m_config_data[1] = 0x20;
                        break;
                    default:
                        config->m_config_data[1] = 0x2B;
                        break;
                }
                break;
            case S_SHT35_4_MPS:
                config->m_config_data[0] = 0x23;
                switch (config->repeatability) {
                    case S_SHT35_HIGH_REPEATABILIBTY:
                        config->m_config_data[1] = 0x34;
                        break;
                    case S_SHT35_MEDIUM_REPEATABILITY:
                        config->m_config_data[1] = 0x22;
                        break;
                    default:
                        config->m_config_data[1] = 0x29;
                        break;
                }
                break;
            default: // SHT_35_10_MPS
                config->m_config_data[0] = 0x27;
                switch (config->repeatability) {
                    case S_SHT35_HIGH_REPEATABILIBTY:
                        config->m_config_data[1] = 0x37;
                        break;
                    case S_SHT35_MEDIUM_REPEATABILITY:
                        config->m_config_data[1] = 0x21;
                        break;
                    default:
                        config->m_config_data[1] = 0x2A;
                        break;
                }
                break;
        }

        config->m_fetch_data[0] = 0xE0;
        config->m_fetch_data[1] = 0x00;

        i2c_init_message(
                &config->m_fetch,
                I2C_WRITE_ADDRESS(config->address),
                config->i2c_bus,
                config->m_fetch_data,
                SENSOR_SHT35_FETCH_DATA_LENGTH,
                I2C_NO_DATA,
                0,
                i2c_get_write_controller(config->i2c_bus),
                3,
                sht35_i2c_cb_periodic_m_fetch,
                (uint8_t*) intf,
                0);
    } else {
        switch (config->clock) {
            case S_SHT35_ENABLE_CLOCK_STRETCHING:
                config->m_config_data[0] = 0x2C;
                switch (config->repeatability) {
                    case S_SHT35_HIGH_REPEATABILIBTY:
                        config->m_config_data[1] = 0x06;
                        break;
                    case S_SHT35_MEDIUM_REPEATABILITY:
                        config->m_config_data[1] = 0x0D;
                        break;
                    default:
                        config->m_config_data[1] = 0x10;
                        break;
                }
                break;
            default:
                config->m_config_data[0] = 0x24;
                switch (config->repeatability) {
                    case S_SHT35_HIGH_REPEATABILIBTY:
                        config->m_config_data[1] = 0x00;
                        break;
                    case S_SHT35_MEDIUM_REPEATABILITY:
                        config->m_config_data[1] = 0x0B;
                        break;
                    default:
                        config->m_config_data[1] = 0x16;
                        break;
                }
                break;
        }
    }

    i2c_init_message(&config->m_config,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->m_config_data,
            SENSOR_SHT35_CONFIG_DATA_LENGTH,
            I2C_NO_DATA,
            0,
            i2c_get_write_controller(config->i2c_bus),
            3,
            sht35_config_phase2_cb,
            (uint8_t*) intf,
            0);

    i2c_queue_message(&config->m_config);
}

static void sht35_config_phase2_cb(i2c_message_t* m) {
    struct sensor_interface_s* intf = (struct sensor_interface_s*) m->callback_data;
    sensor_sht35_config_t* config = (sensor_sht35_config_t*) & intf->config.sht35;
    message_t m_status;
    uint8_t data[1];

    switch (config->periodicity) {
        case S_SHT35_PERIODIC:
            config->m_config.callback = NULL;
            break;
        default:
            config->m_config.callback = sht35_i2c_cb_single_shot_m_config;
            break;
    }

    if (m->error != I2C_NO_ERROR) {
        intf->status = SENSOR_STATUS_ERROR;
    } else {
        intf->status = SENSOR_STATUS_ACTIVE;
    }

    message_init(&m_status, controller_address, MESSAGE_NO_REQUEST, M_SENSOR_STATUS,
            intf->sensor_id, data, ARRAY_LENGTH(data));
    data[0] = intf->status;

    message_send(&m_status);
}

void sht35_i2c_cb_periodic_m_fetch(i2c_message_t* m) {
    struct sensor_interface_s* intf = (struct sensor_interface_s*) m->callback_data;
    sensor_sht35_config_t* config = (sensor_sht35_config_t*) & intf->config.sht35;

    if (m->error != I2C_NO_ERROR) {
        intf->log_data[0] = m->status;
        intf->log_data[1] = m->error;
        intf->log_data[2] = 1;

        message_init(&intf->log,
                controller_address,
                MESSAGE_NO_REQUEST,
                M_SENSOR_STATUS,
                intf->sensor_id,
                intf->log_data,
                3);
        message_send(&intf->log);

    } else {
        i2c_reset_message(&config->m_read, 1);
        i2c_queue_message(&config->m_read);
    }
}

void sht35_i2c_cb_periodic_m_read(i2c_message_t* m) {
    // check CRC
    uint8_t crc_temperature = 0xFF, crc_rh = 0xFF;
    struct sensor_interface_s* intf = (struct sensor_interface_s*) m->callback_data;

    if (m->error == I2C_NO_ERROR) {
        crc_temperature = sht35_calculate_crc(m->read_data[0], crc_temperature, SHT35_CRC_POLY);
        crc_temperature = sht35_calculate_crc(m->read_data[1], crc_temperature, SHT35_CRC_POLY);

        crc_rh = sht35_calculate_crc(m->read_data[3], crc_rh, SHT35_CRC_POLY);
        crc_rh = sht35_calculate_crc(m->read_data[4], crc_rh, SHT35_CRC_POLY);

        if ((crc_temperature != m->read_data[2]) || (crc_rh != m->read_data[5])) {
            m->error = I2C_INCORRECT_DATA;
        } else {
            message_init(&intf->log,
                    controller_address,
                    MESSAGE_NO_REQUEST,
                    M_SENSOR_STATUS,
                    intf->sensor_id,
                    m->read_data,
                    SENSOR_SHT35_CAN_DATA_LENGTH);
            message_send(&intf->log);
        }
    }
    if (m->error != I2C_NO_ERROR) {
        intf->log_data[0] = m->status;
        intf->log_data[1] = m->error;
        intf->log_data[2] = 1;

        message_init(&intf->log,
                controller_address,
                MESSAGE_NO_REQUEST,
                M_SENSOR_STATUS,
                intf->sensor_id,
                intf->log_data,
                3);
        message_send(&intf->log);
    }
}

void sht35_i2c_cb_single_shot_m_config(i2c_message_t* m) {
    if (m->error != I2C_NO_ERROR) {
        struct sensor_interface_s* intf = (struct sensor_interface_s*) m->callback_data;

        intf->log_data[0] = m->status;
        intf->log_data[1] = m->error;
        intf->log_data[2] = 1;

        message_init(&intf->log,
                controller_address,
                MESSAGE_NO_REQUEST,
                M_SENSOR_STATUS,
                intf->sensor_id,
                intf->log_data,
                3);
        message_send(&intf->log);
    }
}

void sht35_i2c_cb_single_shot_m_read(i2c_message_t* m) {
    struct sensor_interface_s* intf = (struct sensor_interface_s*) m->callback_data;

    if (m->error == I2C_NO_ERROR) {
        message_init(&intf->log,
                controller_address,
                MESSAGE_NO_REQUEST,
                M_SENSOR_STATUS,
                intf->sensor_id,
                m->read_data,
                SENSOR_SHT35_CAN_DATA_LENGTH);
        message_send(&intf->log);
    } else {
        intf->log_data[0] = m->status;
        intf->log_data[1] = m->error;
        intf->log_data[2] = 1;

        message_init(&intf->log,
                controller_address,
                MESSAGE_NO_REQUEST,
                M_SENSOR_STATUS,
                intf->sensor_id,
                intf->log_data,
                3);
        message_send(&intf->log);
    }
}

uint8_t sht35_calculate_crc(uint8_t b, uint8_t crc, uint8_t poly) {
    uint16_t i;
    crc = crc ^ b;
    for (i = 0; i < 8; i++) {
        if ((crc & 0x80) == 0) {
            crc = crc << 1;
        } else {
            crc = (crc << 1) ^ poly;
        }
    }
    return crc;
}

void validate_sht35_config(sensor_sht35_config_t* config) {
    SENSOR_CONFIG_CHECK_MAX_SET(config->repeatability, S_SHT35_LOW_REPEATABILITY, S_SHT35_HIGH_REPEATABILIBTY);
    SENSOR_CONFIG_CHECK_MAX_SET(config->clock, S_SHT35_DISABLE_CLOCK_STRETCHING, S_SHT35_ENABLE_CLOCK_STRETCHING);
    SENSOR_CONFIG_CHECK_MAX_SET(config->rate, S_SHT35_10_MPS, S_SHT35_1_MPS);
    SENSOR_CONFIG_CHECK_MAX_SET(config->periodicity, S_SHT35_PERIODIC, S_SHT35_SINGLE_SHOT);
}
