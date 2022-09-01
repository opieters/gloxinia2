#include "actuator_growth_chamber.h"
#include <address.h>
#include <actuator_common.h>

void init_growth_chamber(growth_chamber_config_t* config){
    void (*controller)(i2c_message_t*);
    
    uart_simple_print("Initialising GC I2C.");
    
    switch(config->i2c_bus){
        case I2C1_BUS:
            controller = i2c1_write_controller;
            break;
        case I2C2_BUS:
            controller = i2c2_write_controller;
            break;
        default:
            report_error("growth chamber: I2C module not supported.");;
            break;
    }
    
    reset_actuator_local_id();
    
    config->general.local_id = get_actuator_local_id();
    config->general.global_id = CAN_DATA_CMD_GROWTH_CHAMBER;
    
    actuator_init_common_config(&config->general, ACTUATOR_GROWTH_CHAMBER_LOG_LENGTH);
    
    i2c_init_message(
            &config->m_temp,
            I2C_WRITE_ADDRESS(config->address),
            config->i2c_bus,
            config->i2c_temp_data,
            ACTUATOR_GROWTH_CHAMBER_I2C_DATA_LENGTH,
            NULL,
            0,
            controller,
            3,
            i2c_cb_growth_chamber,
            NULL);

    i2c_init_message(
            &config->m_rh, 
            I2C_WRITE_ADDRESS(config->address), 
            config->i2c_bus,
            config->i2c_rh_data,
            ACTUATOR_GROWTH_CHAMBER_I2C_DATA_LENGTH,
            NULL,
            0,
            controller,
            3,
            i2c_cb_growth_chamber,
            NULL);

    // set default values
    parse_gc_i2c_data(config, ACTUATOR_GROWTH_CHAMBER_TEMP_CHANNEL);
    parse_gc_i2c_data(config, ACTUATOR_GROWTH_CHAMBER_RH_CHANNEL);

    i2c_queue_message(&config->m_rh);
    i2c_queue_message(&config->m_temp);
    i2c_empty_queue();
    
    if(config->m_temp.error != I2C_NO_ERROR){
        config->general.status = ACTUATOR_STATE_ERROR;
    } else {
        config->general.status = ACTUATOR_STATE_ACTIVE;
    }
 
}

void i2c_cb_growth_chamber(i2c_message_t* m){
    /*growth_chamber_config_t* config = (growth_chamber_config_t*) m->processor_data;
    
    
    if(m->error != I2C_NO_ERROR){
        actuator_send_error(&config->general, m);
    }
    
    actuator_send_data(&config->general, m->data, m->data_length);*/
}


void parse_gc_i2c_data(growth_chamber_config_t* config, uint8_t channel_n){
    // parse I2C message data
    switch(channel_n){
        case ACTUATOR_GROWTH_CHAMBER_TEMP_CHANNEL:
            config->temperature = config->new_temperature;
            config->m_temp.write_data[0] = ACTUATOR_GROWTH_CHAMBER_DAC_CH_0;
            config->m_temp.write_data[1] = (config->temperature & 0xff0) >> 4;
            config->m_temp.write_data[2] = (config->temperature & 0xf) << 4; 
            break;
        case ACTUATOR_GROWTH_CHAMBER_RH_CHANNEL:
            config->relative_humidity = config->new_relative_humidity;
            config->m_rh.write_data[0] = ACTUATOR_GROWTH_CHAMBER_DAC_CH_1;
            config->m_rh.write_data[1] = (config->relative_humidity & 0xff0) >> 4;
            config->m_rh.write_data[2] = (config->relative_humidity & 0xf) << 4; 
            break;
        default:
            report_error("growth chamber: channel not configured.");
            break;
    }
    
    
    
}


void gc_i2c_send_temperature(growth_chamber_config_t* config){
    if(config->temperature != config->new_temperature){
        i2c_reset_message(&config->m_temp, 3);

        parse_gc_i2c_data(config, ACTUATOR_GROWTH_CHAMBER_TEMP_CHANNEL);

        i2c_queue_message(&config->m_temp);
    }
}

void gc_i2c_send_rh(growth_chamber_config_t* config){
    if(config->relative_humidity != config->new_relative_humidity){
        i2c_reset_message(&config->m_rh, 3);

        parse_gc_i2c_data(config, ACTUATOR_GROWTH_CHAMBER_RH_CHANNEL);

        i2c_queue_message(&config->m_rh);
    }
}


void actuator_gc_callback(growth_chamber_config_t* config){
    
    static uint16_t timer_value = 0;
    
    if(config->general.status == ACTUATOR_STATE_ACTIVE){
        if(timer_value == 0){
            timer_value = config->period;
            
            gc_i2c_send_temperature(config);
            gc_i2c_send_rh(config);
        }

        timer_value--;
    }
}
