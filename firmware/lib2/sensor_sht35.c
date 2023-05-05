#include <sensor_sht35.h>
#include <sensor.h>
#include <address.h>
#include <utilities.h>

static void sht35_config_phase1_cb(i2c_message_t *m);
static void sht35_config_phase2_cb(i2c_message_t *m);

void sensor_sht35_get_config(struct sensor_gconfig_s* gsc, uint8_t reg, uint8_t* buffer, uint8_t* length){
    sensor_sht35_config_t *config = &gsc->sensor_config.sht35;
    
    buffer[0] = SENSOR_TYPE_SHT35;
    buffer[1] = reg;
    
    switch(reg){
        case sensor_sht35_gloxinia_register_general:
            gsc->measure.task.cb = sensor_sht35_measure;
            gsc->measure.task.data = (void *)gsc;

            buffer[2] = gsc->measure.period >> 8;
            buffer[3] = gsc->measure.period & 0x0ff;
            *length = 4;
            break;
        case sensor_sht35_gloxinia_register_config:
            buffer[2] = config->address;
            buffer[3] = config->repeatability;
            buffer[4] = config->clock;
            buffer[5] = config->rate;
            buffer[6] = config->periodicity;
            *length = 7;
            break;
        default:
            *length = 0;
    }
}

sensor_status_t sensor_sht35_config(struct sensor_gconfig_s *gsc, uint8_t *buffer, uint8_t length)
{
    if (length < 1)
    {
        return SENSOR_STATUS_ERROR;
    }

    UART_DEBUG_PRINT("Configuring SHT35 sensor");

    sensor_sht35_config_t *config = &gsc->sensor_config.sht35;
    
    config->data_ready = false;
    
    switch(buffer[0]){
        case sensor_sht35_gloxinia_register_general:
            if (length != 3){ return SENSOR_STATUS_ERROR; }
            
            gsc->measure.task.cb = sensor_sht35_measure;
            gsc->measure.task.data = (void *)gsc;

            schedule_init(&gsc->measure, gsc->measure.task, (((uint16_t)buffer[1]) << 8) | buffer[2]);

            return SENSOR_STATUS_IDLE;
            
            break;
        case sensor_sht35_gloxinia_register_config:
            if(length != 6) { return SENSOR_STATUS_ERROR; }
            
            // load configuration from buffer into data structure
            config->address = buffer[1];
            config->i2c_bus = sensor_get_i2c_bus(gsc->interface->interface_id);
            config->repeatability = (sensor_sht35_repeatability_t)buffer[2];
            config->clock = (sensor_sht35_clock_stretching_t)buffer[3];
            config->rate = (sensor_sht35_sample_rate_t)buffer[4];
            config->periodicity = (sensor_sht35_periodicity_t)buffer[5];

            // validate configuration
            if(!validate_sht35_config(config)){
                UART_DEBUG_PRINT("Configuring SHT35 INVALID CONFIG");
                return SENSOR_STATUS_ERROR;
            } else {
                // start sensor initialisation (async) only when configuration is OK
                sht35_init_sensor(gsc);
            }
            break;
        default:
            break;
    }

    return SENSOR_STATUS_IDLE;
}

void sensor_sht35_measure(void *data)
{
    sensor_gconfig_t* gsc = (struct sensor_gconfig_s *)data;

    if (gsc->sensor_config.sht35.periodicity == S_SHT35_SINGLE_SHOT)
    {
        if (i2c_check_message_sent(&gsc->sensor_config.sht35.m_read) &&
            i2c_check_message_sent(&gsc->sensor_config.sht35.m_config))
        {
            i2c_reset_message(&gsc->sensor_config.sht35.m_read, 1);
            i2c_queue_message(&gsc->sensor_config.sht35.m_read);
            i2c_reset_message(&gsc->sensor_config.sht35.m_config, 1);
            i2c_queue_message(&gsc->sensor_config.sht35.m_config);
        }
        else
        {
            UART_DEBUG_PRINT("SHT35 on %x not fully processed.", gsc->sensor_id | (gsc->interface->interface_id << 4));
            gsc->sensor_config.sht35.m_read.status = I2C_MESSAGE_CANCELED;
            gsc->sensor_config.sht35.m_config.status = I2C_MESSAGE_CANCELED;

            gsc->status = SENSOR_STATUS_ERROR;
            sensor_error_handle(gsc);
        }
    }
    else
    {
        if (i2c_check_message_sent(&gsc->sensor_config.sht35.m_fetch))
        {
            i2c_reset_message(&gsc->sensor_config.sht35.m_fetch, 1);
            i2c_queue_message(&gsc->sensor_config.sht35.m_fetch);
        }
        else
        {
            UART_DEBUG_PRINT("SHT35 %x not fully processed.", gsc->sensor_id | (gsc->interface->interface_id << 4));
            gsc->status = SENSOR_STATUS_ERROR;

            sensor_error_handle(gsc);
        }
    }
}

void sht35_init_sensor(struct sensor_gconfig_s *gsc)
{
    void (*callback)(i2c_message_t * m);

    sensor_sht35_config_t *config = &gsc->sensor_config.sht35;
    
    UART_DEBUG_PRINT("Configuring SHT35 init sensor");

    switch (config->periodicity)
    {
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
        (uint8_t *)gsc,
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
        (uint8_t *)gsc,
        0);

    i2c_queue_message(&config->m_config);
}

static void sht35_config_phase1_cb(i2c_message_t *m)
{
    struct sensor_gconfig_s *gsc = (struct sensor_gconfig_s *)m->callback_data;
    sensor_sht35_config_t *config = (sensor_sht35_config_t *)&gsc->sensor_config.sht35;
    
    if (m->error != I2C_NO_ERROR)
    {
        gsc->log_data[0] = m->status;
        gsc->log_data[1] = m->error;
        gsc->log_data[2] = S_SHT35_ERROR_PHASE1_CB;
        sensor_error_log(gsc, gsc->log_data, 3);
        
        UART_DEBUG_PRINT("Configuring SHT35 CB1 ERR");
        
        gsc->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(gsc);
        return;
    }
    
    UART_DEBUG_PRINT("Configuring SHT35 CB1");

    if (config->periodicity == S_SHT35_PERIODIC)
    {
        switch (config->rate)
        {
        case S_SHT35_0_5_MPS:
            config->m_config_data[0] = 0x20;
            switch (config->repeatability)
            {
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
            switch (config->repeatability)
            {
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
            switch (config->repeatability)
            {
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
            switch (config->repeatability)
            {
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
            switch (config->repeatability)
            {
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
            (uint8_t *)gsc,
            0);
    }
    else
    {
        switch (config->clock)
        {
        case S_SHT35_ENABLE_CLOCK_STRETCHING:
            config->m_config_data[0] = 0x2C;
            switch (config->repeatability)
            {
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
            switch (config->repeatability)
            {
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

    i2c_init_message(&config->m_config2,
                     I2C_WRITE_ADDRESS(config->address),
                     config->i2c_bus,
                     config->m_config_data,
                     SENSOR_SHT35_CONFIG_DATA_LENGTH,
                     I2C_NO_DATA,
                     0,
                     i2c_get_write_controller(config->i2c_bus),
                     3,
                     sht35_config_phase2_cb,
                     (uint8_t *)gsc,
                     0);

    i2c_queue_message(&config->m_config2);
}

static void sht35_config_phase2_cb(i2c_message_t *m)
{
    struct sensor_gconfig_s *gsc = (struct sensor_gconfig_s *)m->callback_data;
    sensor_sht35_config_t *config = (sensor_sht35_config_t *)&gsc->sensor_config.sht35;
    message_t m_status;
    uint8_t data[1];
    void (*callback)(i2c_message_t * m);
    
    if (m->error != I2C_NO_ERROR)
    {
        UART_DEBUG_PRINT("Configuring SHT35 CB2 ERR");
        
        gsc->log_data[0] = m->status;
        gsc->log_data[1] = m->error;
        gsc->log_data[2] = S_SHT35_ERROR_PHASE2_CB;
        sensor_error_log(gsc, gsc->log_data, 3);
        
        gsc->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(gsc);
        return;
    }
    
    UART_DEBUG_PRINT("Configuring SHT35 CB2");
    
    switch (config->periodicity)
    {
    case S_SHT35_PERIODIC:
        callback = NULL;
        break;
    default:
        callback = sht35_i2c_cb_single_shot_m_config;
        break;
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
                     callback,
                     (uint8_t *)gsc,
                     0);

    if (m->error != I2C_NO_ERROR)
    {
        gsc->status = SENSOR_STATUS_ERROR;
    }
    else
    {
        gsc->status = SENSOR_STATUS_IDLE;
    }

    message_init(&m_status, controller_address, MESSAGE_NO_REQUEST, M_SENSOR_STATUS,
                 gsc->sensor_id | (gsc->interface->interface_id << 4), data, ARRAY_LENGTH(data));
    data[0] = gsc->status;

    message_send(&m_status);
}

void sensor_sht35_activate(struct sensor_gconfig_s* gsc){
    gsc->status = SENSOR_STATUS_RUNNING;
}

void sht35_i2c_cb_periodic_m_fetch(i2c_message_t *m)
{
    struct sensor_gconfig_s *gsc = (struct sensor_gconfig_s *)m->callback_data;
    sensor_sht35_config_t *config = (sensor_sht35_config_t *)&gsc->sensor_config.sht35;

    if (m->error != I2C_NO_ERROR)
    {
        gsc->log_data[0] = m->status;
        gsc->log_data[1] = m->error;
        gsc->log_data[2] = S_SHT35_ERROR_FETCH_PERIODIC;

        sensor_error_log(gsc, gsc->log_data, 3);
        
        gsc->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(gsc);
        
    }
    else
    {
        i2c_reset_message(&config->m_read, 1);
        i2c_queue_message(&config->m_read);
    }
}

void sht35_i2c_cb_periodic_m_read(i2c_message_t *m)
{
    // check CRC
    uint8_t crc_temperature = 0xFF, crc_rh = 0xFF;
    struct sensor_gconfig_s *gsc = (struct sensor_gconfig_s *)m->callback_data;

    if (m->error == I2C_NO_ERROR)
    {
        crc_temperature = sht35_calculate_crc(m->read_data[0], crc_temperature, SHT35_CRC_POLY);
        crc_temperature = sht35_calculate_crc(m->read_data[1], crc_temperature, SHT35_CRC_POLY);

        crc_rh = sht35_calculate_crc(m->read_data[3], crc_rh, SHT35_CRC_POLY);
        crc_rh = sht35_calculate_crc(m->read_data[4], crc_rh, SHT35_CRC_POLY);

        if ((crc_temperature != m->read_data[2]) || (crc_rh != m->read_data[5]))
        {
            m->error = I2C_INCORRECT_DATA;
        }
        else
        {
            message_init(&gsc->log,
                         controller_address,
                         MESSAGE_NO_REQUEST,
                         M_SENSOR_DATA,
                         gsc->sensor_id | (gsc->interface->interface_id << 4),
                         m->read_data,
                         SENSOR_SHT35_CAN_DATA_LENGTH);
            message_send(&gsc->log);
            
#ifdef __DICIO__
            sdcard_save_sensor_data(SENSOR_TYPE_SHT35, m->read_data, SENSOR_SHT35_CAN_DATA_LENGTH);
#endif
        }
    }
    if (m->error != I2C_NO_ERROR)
    {
        gsc->log_data[0] = m->status;
        gsc->log_data[1] = m->error;
        gsc->log_data[2] = S_SHT35_ERROR_READOUT_PERIODIC;

        sensor_error_log(gsc, gsc->log_data, 3);
        
        gsc->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(gsc);
    }
}

void sht35_i2c_cb_single_shot_m_config(i2c_message_t *m)
{
    struct sensor_gconfig_s *gsc = (struct sensor_gconfig_s *)m->callback_data;
    sensor_sht35_config_t *config = (sensor_sht35_config_t *)&gsc->sensor_config.sht35;
    
    if (m->error != I2C_NO_ERROR)
    {
        gsc->log_data[0] = m->status;
        gsc->log_data[1] = m->error;
        gsc->log_data[2] = S_SHT35_ERROR_CONFIG_SINGLE_SHOT;

        sensor_error_log(gsc, gsc->log_data, 3);
        
        gsc->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(gsc);
        
        config->data_ready = false;
    } else {
        config->data_ready = true;
    }
}

void sht35_i2c_cb_single_shot_m_read(i2c_message_t *m)
{
    struct sensor_gconfig_s* gsc = (struct sensor_gconfig_s *)m->callback_data;
    uint8_t crc_temperature = 0xFF, crc_rh = 0xFF;
    sensor_sht35_config_t *config = (sensor_sht35_config_t *)&gsc->sensor_config.sht35;
    
    // if there is no data, stop processing
    if(!config->data_ready)
        return;
    
    config->data_ready = false;
    
    if (m->error == I2C_NO_ERROR)
    {
        crc_temperature = sht35_calculate_crc(m->read_data[0], crc_temperature, SHT35_CRC_POLY);
        crc_temperature = sht35_calculate_crc(m->read_data[1], crc_temperature, SHT35_CRC_POLY);

        crc_rh = sht35_calculate_crc(m->read_data[3], crc_rh, SHT35_CRC_POLY);
        crc_rh = sht35_calculate_crc(m->read_data[4], crc_rh, SHT35_CRC_POLY);

        if ((crc_temperature != m->read_data[2]) || (crc_rh != m->read_data[5]))
        {
            m->error = I2C_INCORRECT_DATA;
        }
        else
        {
            message_init(&gsc->log,
                         controller_address,
                         MESSAGE_NO_REQUEST,
                         M_SENSOR_DATA,
                         gsc->sensor_id | (gsc->interface->interface_id),
                         m->read_data,
                         SENSOR_SHT35_CAN_DATA_LENGTH);
            message_send(&gsc->log);
            
#ifdef __DICIO__
            sdcard_save_sensor_data(SENSOR_TYPE_SHT35, m->read_data, SENSOR_SHT35_CAN_DATA_LENGTH);
#endif
        }
    }
    if (m->error != I2C_NO_ERROR)
    {
        gsc->log_data[0] = m->status;
        gsc->log_data[1] = m->error;
        gsc->log_data[2] = S_SHT35_ERROR_READOUT_SINGLE_SHOT;

        sensor_error_log(gsc, gsc->log_data, 3);
        
        gsc->status = SENSOR_STATUS_ERROR;
        sensor_error_handle(gsc);
    }
}

uint8_t sht35_calculate_crc(uint8_t b, uint8_t crc, uint8_t poly)
{
    uint16_t i;
    crc = crc ^ b;
    for (i = 0; i < 8; i++)
    {
        if ((crc & 0x80) == 0)
        {
            crc = crc << 1;
        }
        else
        {
            crc = (crc << 1) ^ poly;
        }
    }
    return crc;
}

bool validate_sht35_config(sensor_sht35_config_t *config)
{
    sensor_sht35_config_t original = *config;
    SENSOR_CONFIG_CHECK_MAX_SET(config->repeatability, S_SHT35_LOW_REPEATABILITY, S_SHT35_HIGH_REPEATABILIBTY);
    SENSOR_CONFIG_CHECK_MAX_SET(config->clock, S_SHT35_DISABLE_CLOCK_STRETCHING, S_SHT35_ENABLE_CLOCK_STRETCHING);
    SENSOR_CONFIG_CHECK_MAX_SET(config->rate, S_SHT35_10_MPS, S_SHT35_1_MPS);
    SENSOR_CONFIG_CHECK_MAX_SET(config->periodicity, S_SHT35_PERIODIC, S_SHT35_SINGLE_SHOT);
    
    if(config->repeatability != original.repeatability){
        return false;
    }
    if(config->clock != original.clock){
        return false;
    }
    if(config->rate != original.rate){
        return false;
    }
    if(config->periodicity != original.periodicity){
        return false;
    }
    
    return true;
}
