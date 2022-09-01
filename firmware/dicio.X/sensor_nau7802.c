#include "sensor_nau7802.h"
#include "actuator_common.h"
#include <utilities.h>

#ifdef __SENSOR_SPECIFIC_CONFIG

bool nau7802_init_sensor(sensor_nau7802_config_t* config){
    size_t i;
    
    sensor_init_common_config(&config->general, SENSOR_NAU7802_CAN_DATA_LENGTH);
    
    // check if sensor is connected
    bool connection = false;
    for(i = 0; i < 2; i++){
        connection = connection | nau7802_sensor_check_connection(config);
        if(connection){
            break;
        }
    }
    
    if(!connection){
        return false;
    }
    
    connection = true;
    
    delay_ms(100);
    //connection &= nau7802_powerUp(config);
    
    
    connection &= nau7802_reset(config);
    delay_ms(20);
    connection &= nau7802_set_ldo(config, config->ldo_config); //  LDO to 3.3V
    delay_ms(20);
    connection &= nau7802_enable_ldo(config);
    delay_ms(20);
    connection &= nau7802_set_gain(config, config->gain); // gain to 128
    delay_ms(20);
    connection &= nau7802_set_sample_rate(config, config->rate); // sample at 10Hz
    delay_ms(20);
    connection &= nau7802_set_register(config, NAU7802_ADC, 0x30); // turn off CLK_CHP
    delay_ms(20);
    connection &= nau7802_set_bit(config, NAU7802_PGA_PWR_PGA_CAP_EN, NAU7802_PGA_PWR); // enable 330pF decoupling cap on CH2 (see 9.14 in AN)
    delay_ms(20);
    connection &= nau7802_set_register(config, NAU7802_I2C_CONTROL, 0b00000000);
    delay_ms(20);
    //connection &= nau7802_powerUp(config);
    //delay_ms(2000);
    connection &= nau7802_set_register(config, NAU7802_PU_CTRL, (1<<NAU7802_PU_CTRL_PUD) | (1<<NAU7802_PU_CTRL_PUA) | (1<<NAU7802_PU_CTRL_AVDDS)); 
    delay_ms(20);
    nau7802_get_register(config, NAU7802_PU_CTRL); 
    connection &= nau7802_calibrate_analogue_front_end(config); // calibrate analogue front end
    delay_ms(2000);
    connection = true;
    
    nau7802_sensor_init_readout_i2c_messages(config);
    
    if(connection){
        sensor_update_status(&config->general, I2C_NO_ERROR);
    } else {
        sensor_update_status(&config->general, I2C_NO_ACK);
    }
    
    return connection;
}

void start_nau7802_sensor(sensor_nau7802_config_t* config){
    
}

void nau7802_sensor_init_readout_i2c_messages(sensor_nau7802_config_t* config){
    void (*controller)(i2c_message_t* m);
    
    switch(config->general.i2c_bus){
        case I2C2_BUS:
            controller = i2c2_write_read_controller;
            break;
        case I2C1_BUS:
        default:
            controller = i2c1_write_read_controller;
            break;
    }
    
    config->m3d[0] = NAU7802_PU_CTRL;
    
    i2c_init_message(
            &config->m3, 
            I2C_WRITE_ADDRESS(config->general.address), 
            config->m3d,
            ARRAY_LENGTH(config->m3d), 
            controller,
            1, 
            nau7802_check_data_cb, 
            i2c_dummy_callback,
            (uint8_t*) config, 
            0,  
            config->general.i2c_bus,
            &config->m3s);
    
    i2c_init_connected_message(&config->m3s, &config->m3, config->m3sd, ARRAY_LENGTH(config->m1sd));
    
    config->m1d[0] = NAU7802_ADCO_B2;
    
    // initialise i2c messages
    i2c_init_message(
            &config->m1, 
            I2C_WRITE_ADDRESS(config->general.address), 
            config->m1d,
            ARRAY_LENGTH(config->m1d), 
            controller,
            1, 
            nau7802_readout_cb, 
            i2c_dummy_callback,
            (uint8_t*) config, 
            0,
            config->general.i2c_bus,
            &config->m1s);
    
    i2c_init_connected_message(&config->m1s, &config->m1, config->m1sd, ARRAY_LENGTH(config->m1sd));
    
    switch(config->general.i2c_bus){
        case I2C2_BUS:
            controller = i2c2_write_controller;
            break;
        case I2C1_BUS:
        default:
            controller = i2c1_write_controller;
            break;
    }
    
    config->m2d[0] = nau7802_get_register(config, NAU7802_PU_CTRL);
    config->m2d[0] |= (1 << NAU7802_PU_CTRL_CS);
    
    i2c_init_message(
        &config->m2, 
        I2C_WRITE_ADDRESS(config->general.address), 
        config->m2d,
        ARRAY_LENGTH(config->m2d), 
        controller,
        1, 
        i2c_dummy_callback, 
        i2c_dummy_callback,
        (uint8_t*) config, 
        0,  
        config->general.i2c_bus,
        NULL);
}

bool nau7802_sensor_check_connection(sensor_nau7802_config_t* config){
    i2c_message_t m;
    void (*controller)(i2c_message_t* m);
    
    switch(config->general.i2c_bus){
        case I2C2_BUS:
            controller = i2c2_write_controller;
            break;
        case I2C1_BUS:
            controller = i2c1_write_controller;
        default:
            break;
    }
    
    i2c_init_message(
            &m, 
            I2C_WRITE_ADDRESS(config->general.address), 
            NULL, 
            0, 
            controller,
            1, 
            i2c_dummy_callback, 
            i2c_dummy_callback,
            NULL, 
            0,  
            config->general.i2c_bus,
            NULL);
    
    i2c_queue_message(&m);
    i2c_empty_queue();
    
    return m.error == I2C_NO_ERROR;
}
bool nau7802_check_data_available(sensor_nau7802_config_t* config){
    return nau7802_get_bit(config, NAU7802_PU_CTRL_CR, NAU7802_PU_CTRL);
}

void nau7802_read_data(sensor_nau7802_config_t* config);

int32_t nau7802_blocking_average_read(sensor_nau7802_config_t* config, uint8_t n_reads){
i2c_message_t m, cm;
    uint8_t m_data[1], cm_data[3];
    void (*controller)(i2c_message_t* m);
    size_t i;
    int32_t sum_data = 0;
    uint32_t raw_value;
    
    switch(config->general.i2c_bus){
        case I2C2_BUS:
            controller = i2c2_write_read_controller;
            break;
        case I2C1_BUS:
        default:
            controller = i2c1_write_read_controller;
            break;
    }
    
    // initialise i2c messages
    i2c_init_message(
            &m, 
            I2C_WRITE_ADDRESS(config->general.address), 
            m_data,
            ARRAY_LENGTH(m_data), 
            controller,
            1, 
            i2c_dummy_callback, 
            i2c_dummy_callback,
            (uint8_t*) config, 
            0,  
            config->general.i2c_bus,
            &cm);
    
    i2c_init_connected_message(&cm, &m, cm_data, ARRAY_LENGTH(cm_data));
    
    for(i = 0; i < n_reads; i++){
        i2c_reset_message(&m, 1);
        i2c_queue_message(&m);
        i2c_empty_queue();
        
        raw_value = (((uint32_t) cm_data[0]) << 16) | (((uint32_t) cm_data[1]) << 8) | ((uint32_t) cm_data[2]);
        sum_data = sum_data + (((int32_t) (raw_value << 8)) >> 8);
    }
    
    sum_data = sum_data / n_reads;
    
    return sum_data;
}

void nau7802_tare(sensor_nau7802_config_t* config){
    config->b = nau7802_blocking_average_read(config, 8);
    
}
void nau7802_set_zero(sensor_nau7802_config_t* config, uint32_t b){
    config->b = b;
}

void nau7802_set_scale(sensor_nau7802_config_t* config, float a){
    config->a = a;
}

void nau7802_calibrate_scale(sensor_nau7802_config_t* config){
    int32_t readout_1kg = nau7802_blocking_average_read(config, 8);
    float a;
    
    a = (0.0 - 1.0) / (1.0*config->b - readout_1kg);
    
    config->a = a;
}


void nau7802_check_data_cb(i2c_message_t* m){
    sensor_nau7802_config_t* config = (sensor_nau7802_config_t*) m->processor_data;
    
    if(m->connected_message->data[0] & (1 << NAU7802_PU_CTRL_CR)){
        i2c_reset_message(&config->m1, 1);
        i2c_queue_message(&config->m1);
    } else {
        sensor_send_error(&config->general.elog, m);
    }
}

void nau7802_readout_cb(i2c_message_t* m){
    sensor_nau7802_config_t* config = (sensor_nau7802_config_t*) m->processor_data;
    
    if(m->error == I2C_NO_ERROR){
        uint32_t raw_data = (((uint32_t) m->connected_message->data[0]) << 16) 
                | (((uint32_t) m->connected_message->data[1]) << 8) 
                | ((uint32_t) m->connected_message->data[2]);
        
        // convert from 24-bit to 32-bit signed integer
        int32_t formatted_data = (int32_t) (raw_data << 8);
        uint8_t corrected_data[4];
        
        // scale back to original
        formatted_data = formatted_data >> 8;
        
        corrected_data[0] = (uint8_t) (formatted_data >> 24);
        corrected_data[1] = (uint8_t) (formatted_data >> 16);
        corrected_data[2] = (uint8_t) (formatted_data >> 8);
        corrected_data[3] = (uint8_t) (formatted_data & 0xff);
        
        sensor_send_data(&config->general.dlog, corrected_data, SENSOR_NAU7802_DATA_LENGTH);
        
        i2c_reset_message(&config->m2, 1);
        i2c_queue_message(&config->m2);
    } else {
        sensor_send_error(&config->general.elog, m);
    }
}

bool nau7802_set_gain(sensor_nau7802_config_t* config, nau7802_gain_t gain){
    uint8_t data;
    
    gain = MIN(NAU7802_GAIN_128, gain);
    
    data = nau7802_get_register(config, NAU7802_CTRL1);
    data &= 0b11111000;
    data |= gain;
    
    return nau7802_set_register(config, NAU7802_CTRL1, data);
}
bool nau7802_set_ldo(sensor_nau7802_config_t* config, nau7802_ldo_t setting){
    uint8_t data;
    
    setting = MIN(NAU7802_LDO_2V4, setting);
    
    data = nau7802_get_register(config, NAU7802_CTRL1);
    data &= 0b11000111;
    data = data | (setting << 3);
    
    return nau7802_set_register(config, NAU7802_CTRL1, data);
}
bool nau7802_set_sample_rate(sensor_nau7802_config_t* config, nau7802_rate_t rate){
    uint8_t data;
    
    rate = MIN(NAU7802_SPS_320, rate);
    
    data = nau7802_get_register(config, NAU7802_CTRL2);
    data &= 0b10001111;
    data = data | (rate << 4);
    
    return nau7802_set_register(config, NAU7802_CTRL2, data);
}
bool nau7802_set_channel(sensor_nau7802_config_t* config, nau7802_channel_t channel){
    if(channel == NAU7802_CHANNEL_1){
        return nau7802_clear_bit(config, NAU7802_CTRL2_CHS, NAU7802_CTRL2);
    } else {
        return nau7802_set_bit(config, NAU7802_CTRL2_CHS, NAU7802_CTRL2);
    }
}

bool nau7802_calibrate_analogue_front_end(sensor_nau7802_config_t* config){
    nau7802_set_bit(config, NAU7802_CTRL2_CALS, NAU7802_CTRL2);
    delay_ms(1000);
    return nau7802_afe_calibration_status(config) == NAU7802_CAL_SUCCESS;
}

nau7802_calibration_status_t nau7802_afe_calibration_status(sensor_nau7802_config_t* config){
    if(nau7802_get_bit(config, NAU7802_CTRL2_CALS, NAU7802_CTRL2)){
        return NAU7802_CAL_IN_PROGRESS;
    }
    
    if(nau7802_get_bit(config, NAU7802_CTRL2_CAL_ERROR, NAU7802_CTRL2)){
        return NAU7802_CAL_FAILURE;
    }
    
    return NAU7802_CAL_SUCCESS;
}

bool nau7802_reset(sensor_nau7802_config_t* config){
    bool status = nau7802_set_bit(config, NAU7802_PU_CTRL_RR, NAU7802_PU_CTRL);
    delay_ms(2000);
    status |= nau7802_set_register(config, NAU7802_PU_CTRL, 0x00);
    delay_ms(2000);
    return status;
}

bool nau7802_enable_ldo(sensor_nau7802_config_t* config){
    return nau7802_set_bit(config, NAU7802_PU_CTRL_AVDDS, NAU7802_PU_CTRL);
}

bool nau7802_powerUp(sensor_nau7802_config_t* config){
    uint16_t counter = 0;
    uint8_t value;
    bool status = true;
    
    status &= nau7802_set_bit(config, NAU7802_PU_CTRL_PUD, NAU7802_PU_CTRL);
    delay_ms(1000);
    status &= nau7802_set_bit(config, NAU7802_PU_CTRL_PUA, NAU7802_PU_CTRL);
    delay_ms(1000);
    
    while(1){
        value = nau7802_get_bit(config, NAU7802_PU_CTRL_PUR, NAU7802_PU_CTRL);
        if(value != 0){
            return status;
        }
        delay_ms(100);
        counter++;
        if(counter > 10){
            return false;
        }
    }
}
bool nau7802_powerDown(sensor_nau7802_config_t* config){
    nau7802_clear_bit(config, NAU7802_PU_CTRL_PUD, NAU7802_PU_CTRL);
    return nau7802_clear_bit(config, NAU7802_PU_CTRL_PUA, NAU7802_PU_CTRL);
}

bool nau7802_setIntPolarityHigh(sensor_nau7802_config_t* config){
    return nau7802_clear_bit(config, NAU7802_CTRL1_CRP, NAU7802_CTRL1);
}
bool nau7802_setIntPolarityLow(sensor_nau7802_config_t* config){
    return nau7802_set_bit(config, NAU7802_CTRL1_CRP, NAU7802_CTRL1);
}

uint8_t nau7802_get_revision_code(sensor_nau7802_config_t* config){
    uint8_t code = nau7802_get_register(config, NAU7802_DEVICE_REV);
    return code & 0b00001111;
}

bool nau7802_set_bit(sensor_nau7802_config_t* config, uint8_t bitNumber, uint8_t registerAddress){
    uint8_t data = nau7802_get_register(config, registerAddress);
    data |= (1 << bitNumber);
    return nau7802_set_register(config, registerAddress, data);
}

bool nau7802_clear_bit(sensor_nau7802_config_t* config, uint8_t bitNumber, uint8_t registerAddress){
    uint8_t data = nau7802_get_register(config, registerAddress);
    data &= ~(1 << bitNumber);
    return nau7802_set_register(config, registerAddress, data);
}

uint8_t nau7802_get_bit(sensor_nau7802_config_t* config, uint8_t bitNumber, uint8_t registerAddress){
    uint8_t data = nau7802_get_register(config, registerAddress);
    data &= (1 << bitNumber);
    return data;
}

void nau7802_get_register_nb(sensor_nau7802_config_t* config, uint8_t reg, void (*cb)(i2c_message_t* m)){
    void (*controller)(i2c_message_t* m);
    
    switch(config->general.i2c_bus){
        case I2C2_BUS:
            controller = i2c2_write_read_controller;
            break;
        case I2C1_BUS:
        default:
            controller = i2c1_write_read_controller;
            break;
    }
    
    if(config->config_selection == 0){
        config->m_config = &config->m_config_b;
        config->m_configd = config->m_configd_b;
        config->m_config_rm = &config->m_config_rm_b;
        config->m_config_rmd = config->m_config_rmd_b;
    } else {
        config->m_config = &config->m_config_a;
        config->m_configd = config->m_configd_a;
        config->m_config_rm = &config->m_config_rm_a;
        config->m_config_rmd = config->m_config_rmd_a;
    }
    config->config_selection ^= 1;
    
    config->m_configd[0] = reg;
    i2c_init_message(
            config->m_config, 
            I2C_WRITE_ADDRESS(config->general.address), 
            config->m_configd, 
            ARRAY_LENGTH(config->m_configd_b), 
            controller,
            1, 
            cb, 
            i2c_dummy_callback,
            NULL, 
            0,  
            config->general.i2c_bus,
            config->m_config_rm);
    i2c_init_connected_message(config->m_config_rm, config->m_config, config->m_config_rmd, ARRAY_LENGTH(config->m_config_rmd_b));
    
    config->m_config_rmd[0] = 0;
    
    i2c_queue_message(config->m_config);
}

uint8_t nau7802_get_register(sensor_nau7802_config_t* config, uint8_t reg){
    nau7802_get_register_nb(config, reg, i2c_dummy_callback);
    i2c_empty_queue();
    
    return config->m_config_rmd[0];
}

void nau7802_set_register_nb(sensor_nau7802_config_t* config, uint8_t reg, uint8_t value, void (*cb)(i2c_message_t* m)){
    void (*controller)(i2c_message_t* m);
    
    switch(config->general.i2c_bus){
        case I2C2_BUS:
            controller = i2c2_write_controller;
            break;
        case I2C1_BUS:
        default:
            controller = i2c1_write_controller;
            break;
    }
    
    if(config->config_selection == 0){
        config->m_config = &config->m_config_b;
        config->m_configd = config->m_configd_b;
        config->m_config_rm = &config->m_config_rm_b;
        config->m_config_rmd = config->m_config_rmd_b;
    } else {
        config->m_config = &config->m_config_a;
        config->m_configd = config->m_configd_a;
        config->m_config_rm = &config->m_config_rm_a;
        config->m_config_rmd = config->m_config_rmd_a;
    }
    config->config_selection ^= 1;
    
    config->m_configd[0] = reg;
    config->m_configd[1] = value;
    i2c_init_message(
            config->m_config, 
            I2C_WRITE_ADDRESS(config->general.address), 
            config->m_configd, 
            ARRAY_LENGTH(config->m_configd), 
            controller,
            1, 
            cb, 
            i2c_dummy_callback,
            NULL, 
            0,  
            config->general.i2c_bus,
            NULL);
    
    i2c_queue_message(config->m_config);
} 

bool nau7802_set_register(sensor_nau7802_config_t* config, uint8_t reg, uint8_t value){
    nau7802_set_register_nb(config, reg, value, i2c_dummy_callback);
    i2c_empty_queue();
    
    sprintf(print_buffer, "Set %x -> %x (%d)", reg, value, config->m_config->error);
    uart_print(print_buffer, strlen(print_buffer));
    
    return config->m_config->error == I2C_NO_ERROR;
} 


void nau7802_init_sensor_nb(sensor_nau7802_config_t* config){
    void (*controller)(i2c_message_t* m);
    
    switch(config->general.i2c_bus){
        case I2C2_BUS:
            controller = i2c2_write_controller;
            break;
        case I2C1_BUS:
        default:
            controller = i2c1_write_controller;
            break;
    }
    
    
    i2c_init_message(
            &config->m_config_a, 
            I2C_WRITE_ADDRESS(config->general.address), 
            config->m_configd_a, 
            ARRAY_LENGTH(config->m_configd), 
            controller,
            1, 
            nau7802_init_sensor_cb, 
            i2c_dummy_callback,
            (uint8_t*) config, 
            0,  
            config->general.i2c_bus,
            &config->m_config_rm_a);
    
    config->config_selection = 0;
    
    nau7802_init_sensor_cb(&config->m_config_a);
}

uint8_t nau7802_init_sensor_machine_cb = 0;  
void nau7802_init_sensor_cb(i2c_message_t* m){
    sensor_nau7802_config_t* config = (sensor_nau7802_config_t*) m->processor_data;
    uint8_t value;
    
    switch(nau7802_init_sensor_machine_cb){
        // get reset register
        case 0:
            nau7802_get_register_nb(config, NAU7802_PU_CTRL, nau7802_init_sensor_cb);
            break;
        // set reset register
        case 1:
            value = config->m_config_rmd[0];
            value = value | (1 << NAU7802_PU_CTRL_RR);
            nau7802_set_register_nb(config, NAU7802_PU_CTRL, value, nau7802_init_sensor_cb);
            break;
        // clear reset register
        case 2:
            nau7802_set_register_nb(config, NAU7802_PU_CTRL, 0x00, nau7802_init_sensor_cb);
            break;
        // get control register 1 for LDO setting
        case 3:
            nau7802_get_register_nb(config, NAU7802_CTRL1, nau7802_init_sensor_cb);
            break;
        // set LDO config
        case 4:
            value = config->m_config_rmd[0];
            value &= 0b11000111;
            value = value | (config->ldo_config << 3);
            nau7802_set_register_nb(config, NAU7802_CTRL1, value, nau7802_init_sensor_cb); 
            break;
        // get power control to enable LDO
        case 5:
            nau7802_get_register_nb(config, NAU7802_PU_CTRL, nau7802_init_sensor_cb);
            break;
        // activate LDO
        case 6:
            value = config->m_config_rmd[0];
            value |= (1 << NAU7802_PU_CTRL_AVDDS);
            nau7802_set_register_nb(config, NAU7802_PU_CTRL, value, nau7802_init_sensor_cb);
            break;
        // set PGA gain
        case 7:
            nau7802_get_register_nb(config, NAU7802_CTRL1, nau7802_init_sensor_cb);
            break;
        case 8:
            value = config->m_config_rmd[0];
            value &= 0b11111000;
            value |= config->gain;

            nau7802_set_register_nb(config, NAU7802_CTRL1, value, nau7802_init_sensor_cb);
            break;
        // configure measurement rate
        case 9:
            nau7802_get_register_nb(config, NAU7802_CTRL2, nau7802_init_sensor_cb);
            break;
        case 10:
            value = config->m_config_rmd[0];
            value &= 0b10001111;
            value = value | (config->rate << 4);
    
            nau7802_set_register_nb(config, NAU7802_CTRL2, value, nau7802_init_sensor_cb);
            break;
        // turn off CLK_CHP
        case 11:
            nau7802_set_register_nb(config, NAU7802_ADC, 0x30, nau7802_init_sensor_cb); 
            break;
        // enable 330pF decoupling cap on CH2 (see 9.14 in AN)
        case 12:
            nau7802_get_register_nb(config, NAU7802_PGA_PWR, nau7802_init_sensor_cb);
        case 13:
            value = config->m_config_rmd[0];
            value = value | (1 << NAU7802_PGA_PWR_PGA_CAP_EN);
            
            nau7802_set_register_nb(config, NAU7802_ADC, value, nau7802_init_sensor_cb); 
            break;
        // use standard I2C
        case 14:
            nau7802_set_register_nb(config, NAU7802_I2C_CONTROL, 0b00000000, nau7802_init_sensor_cb);
            break;
        // power up everything
        case 15:
            nau7802_set_register_nb(config, NAU7802_PU_CTRL, (1<<NAU7802_PU_CTRL_PUD) | (1<<NAU7802_PU_CTRL_PUA) | (1<<NAU7802_PU_CTRL_AVDDS), nau7802_init_sensor_cb); 
            break;
        // calibrate analogue front end
        case 16:
            nau7802_get_register_nb(config, NAU7802_CTRL2, nau7802_init_sensor_cb);
            break;
        case 17:
            value = config->m_config_rmd[0];
            value = value | (1 << NAU7802_CTRL2_CALS);
            
            nau7802_set_register_nb(config, NAU7802_CTRL2, value, nau7802_init_sensor_cb);
            break;
        // check calibration status
        case 18:
            nau7802_get_register_nb(config, NAU7802_CTRL2, nau7802_init_sensor_cb);
            value = config->m_config_rmd[0];
            value &= (1 << NAU7802_CTRL2_CALS);
            if(value != 0){
                config->calibration_status = NAU7802_CAL_IN_PROGRESS;
                nau7802_init_sensor_machine_cb--;
                break;
            }
            value = config->m_config_rmd[0];
            value &= (1 << NAU7802_CTRL2_CAL_ERROR);
            if(value != 0){
                config->calibration_status = NAU7802_CAL_FAILURE;
                
                sensor_update_status(&config->general, I2C_NO_ACK);
                break;
            }

            config->calibration_status =  NAU7802_CAL_SUCCESS;
            
            nau7802_sensor_init_readout_i2c_messages(config);
            
            break;
        default:
            break;
    }
    
    nau7802_init_sensor_machine_cb++;
}

#endif