#include "sensor_sht35.h"
#include <gpio_expander.h>
#include <stddef.h>
#include <can.h>
#include "address.h"
#include <i2c.h>
#include <uart.h>
#include <device_configuration.h>
#include <utilities.h>
#include <adc.h>
#include <dac.h>
#include <spi.h>

i2c_error_t sht35_init_sensor(sensor_sht35_config_t* config){  
    void (*callback)(i2c_message_t* m);
    
    sensor_init_common_config(&config->general, SENSOR_SHT35_CAN_DATA_LENGTH);
    
    switch(config->periodicity){
        case S_SHT35_SINGLE_SHOT:
            callback = sht35_i2c_cb_single_shot_m_read;
            break;
        case S_SHT35_PERIODIC:
            callback = sht35_i2c_cb_periodic_m_read;
            break;
        default:
            report_error("SHT35: periodicity option not supported.");
    }
    
    i2c_init_message(
            &config->m_read,
            I2C_READ_ADDRESS(config->address),
            config->i2c_bus,
            NULL,
            0,
            config->m_read_data,
            SENSOR_SHT35_DATA_LENGTH,
            get_read_controller(config->i2c_bus),
            3,
            callback,
            config,
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
        NULL,
        0,
        get_write_controller(config->i2c_bus),
        3,
        i2c_dummy_callback,
        config,
        0);
    
    i2c_queue_message(&config->m_config);
    
    i2c_empty_queue();
    
    delay_ms(100);

    if(config->periodicity == S_SHT35_PERIODIC){
        uint8_t* data = config->m_config_data;
        
        switch (config->rate) {
            case S_SHT35_0_5_MPS:
                data[0] = 0x20;
                switch (config->repeatability) {
                    case S_SHT35_HIGH_REPEATABILIBTY:
                        data[1] = 0x32;
                        break;
                    case S_SHT35_MEDIUM_REPEATABILITY:
                        data[1] = 0x24;
                        break;
                    default:
                        data[1] = 0x2F;
                        break;
                }
                break;
            case S_SHT35_1_MPS:
                data[0] = 0x21;
                switch (config->repeatability) {
                    case S_SHT35_HIGH_REPEATABILIBTY:
                        data[1] = 0x30;
                        break;
                    case S_SHT35_MEDIUM_REPEATABILITY:
                        data[1] = 0x26;
                        break;
                    default:
                        data[1] = 0x2D;
                        break;
                }
                break;
            case S_SHT35_2_MPS:
                data[0] = 0x22;
                switch (config->repeatability) {
                    case S_SHT35_HIGH_REPEATABILIBTY:
                        data[1] = 0x36;
                        break;
                    case S_SHT35_MEDIUM_REPEATABILITY:
                        data[1] = 0x20;
                        break;
                    default:
                        data[1] = 0x2B;
                        break;
                }
                break;
            case S_SHT35_4_MPS:
                data[0] = 0x23;
                switch (config->repeatability) {
                    case S_SHT35_HIGH_REPEATABILIBTY:
                        data[1] = 0x34;
                        break;
                    case S_SHT35_MEDIUM_REPEATABILITY:
                        data[1] = 0x22;
                        break;
                    default:
                        data[1] = 0x29;
                        break;
                }
                break;
            default: // SHT_35_10_MPS
                data[0] = 0x27;
                switch (config->repeatability) {
                    case S_SHT35_HIGH_REPEATABILIBTY:
                        data[1] = 0x37;
                        break;
                    case S_SHT35_MEDIUM_REPEATABILITY:
                        data[1] = 0x21;
                        break;
                    default:
                        data[1] = 0x2A;
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
                NULL,
                0,
                get_write_controller(config->i2c_bus),
                3,
                sht35_i2c_cb_periodic_m_fetch,
                config,
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
        NULL,
        0,
        get_write_controller(config->i2c_bus),
        3,
        i2c_dummy_callback,
        (uint8_t*) config,
        0);

    i2c_queue_message(&config->m_config);
    
    i2c_empty_queue();
    
    delay_ms(100);
    
    switch(config->periodicity){
        case S_SHT35_PERIODIC:
            config->m_config.callback = i2c_dummy_callback;
            break;
        default:
            config->m_config.callback = sht35_i2c_cb_single_shot_m_config;
            break;
    }
    
    sensor_update_status(&config->general, config->m_config.error);
    
    return config->m_config.error;
}

void sht35_i2c_cb_periodic_m_fetch(i2c_message_t* m){
    sensor_sht35_config_t* config = (sensor_sht35_config_t*) m->callback_data;
    
    if(m->error != I2C_NO_ERROR){
        sensor_send_error(&config->general.elog, m);
    } else {
        i2c_reset_message(&config->m_read, 1);
        i2c_queue_message(&config->m_read);
    }
}

void sht35_i2c_cb_periodic_m_read(i2c_message_t* m){
    // check CRC
    //uint8_t crc_temperature = 0xFF, crc_rh = 0xFF;
    sensor_sht35_config_t* config = (sensor_sht35_config_t*) m->callback_data;
    
    if(m->error == I2C_NO_ERROR){
        /*crc_temperature = sht35_calculate_crc(m->data[0], crc_temperature, SHT35_CRC_POLY);
        crc_temperature = sht35_calculate_crc(m->data[1], crc_temperature, SHT35_CRC_POLY);

        crc_rh = sht35_calculate_crc(m->data[3], crc_rh, SHT35_CRC_POLY);
        crc_rh = sht35_calculate_crc(m->data[4], crc_rh, SHT35_CRC_POLY);

        if((crc_temperature != m->data[2]) || (crc_rh != m->data[5])){
            m->error = I2C_INCORRECT_DATA;
            
            sensor_send_error(&config->general.elog, m);
        } else {
            sensor_send_data(&config->general.dlog, config->general.tx_data, SENSOR_SHT35_CAN_DATA_LENGTH);

        }*/
        sensor_send_data(&config->general.dlog, m->read_data, SENSOR_SHT35_CAN_DATA_LENGTH);
    } else {
        sensor_send_error(&config->general.elog, m);
    }
}

void sht35_i2c_cb_single_shot_m_config(i2c_message_t* m){
    if(m->error != I2C_NO_ERROR){
        sensor_sht35_config_t* config = (sensor_sht35_config_t*) m->callback_data;
        sensor_send_error(&config->general.elog, m);
    }
}

void sht35_i2c_cb_single_shot_m_read(i2c_message_t* m){
    sensor_sht35_config_t* config = (sensor_sht35_config_t*) m->callback_data;
    
    if(m->error == I2C_NO_ERROR){         
        sensor_send_data(&config->general.dlog, m->read_data, SENSOR_SHT35_CAN_DATA_LENGTH);
    } else {
        sensor_send_error(&config->general.elog, m);
    }
}

uint8_t sht35_calculate_crc(uint8_t b, uint8_t crc, uint8_t poly){
    uint16_t i;
    crc = crc ^ b;
    for(i = 0; i < 8; i++){
        if((crc & 0x80) == 0){
            crc = crc << 1;
        } else {
            crc = (crc << 1) ^ poly;
        }
    }
    return crc;
}

void validate_sht35_config(sensor_sht35_config_t* config){
    SENSOR_CONFIG_CHECK_EQUAL_SET(config->general.sensor_type, SENSOR_TYPE_SHT35);
    SENSOR_CONFIG_CHECK_MAX_SET(config->repeatability,S_SHT35_LOW_REPEATABILITY,S_SHT35_HIGH_REPEATABILIBTY);
    SENSOR_CONFIG_CHECK_MAX_SET(config->clock,S_SHT35_DISABLE_CLOCK_STRETCHING,S_SHT35_ENABLE_CLOCK_STRETCHING);
    SENSOR_CONFIG_CHECK_MAX_SET(config->rate,S_SHT35_10_MPS,S_SHT35_1_MPS);
    SENSOR_CONFIG_CHECK_MAX_SET(config->periodicity,S_SHT35_PERIODIC,S_SHT35_SINGLE_SHOT);
}
