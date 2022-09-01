#include "sensor_ds18b20.h"
#include <utilities.h>
#ifdef __SENSOR_SPECIFIC_CONFIG
one_wire_status_t ds18b20_start_conversion(sensor_ds18b20_config_t* config){
    one_wire_status_t status = one_wire_reset(config->config); 
    one_wire_write_byte(config->config, 0xCC); // Issue skip ROM command (CCh)
    one_wire_write_byte(config->config, 0x44); // Convert T command (44h)
    
    return status;
}


// ds18b20_start_conversion must be called before a result can be read
one_wire_status_t ds18b20_read_result(sensor_ds18b20_config_t* config){
    uint8_t lsb, msb;
    int16_t n_tests = 500;
    
    while((!one_wire_read(config->config)) && (n_tests > 0)){
        n_tests--;
        delay_us(500);
    }
    
    if(n_tests == 0){
        config->general.status = SENSOR_STATUS_ERROR;
        return ONE_WIRE_ERROR;
    }
    
    one_wire_reset(config->config);
    one_wire_write_byte(config->config, 0xCC); // Issue skip ROM command 
    one_wire_write_byte(config->config, 0xBE); // Read Scratchpad (BEh) - 15 bits
    
    lsb = one_wire_read_byte(config->config);
    msb = one_wire_read_byte(config->config);
    
    one_wire_reset(config->config); // Stop Reading 
    
    config->data = (msb << 8) | lsb;
    
    return ONE_WIRE_NO_ERROR;
}

// should be called to send the result to the logger
void ds18b20_readout_cb(sensor_ds18b20_config_t* config){
    
    if(config->general.status == SENSOR_STATUS_ACTIVE){
        uint8_t data[2];
        data[0] = (uint8_t) (config->data >> 8) & 0xff;
        data[1] = (uint8_t) config->data & 0xff;
        
        sensor_send_data(&config->general.dlog, data, SENSOR_DS18B20_DATA_LENGTH);
    } else {
        sensor_send_general_error(&config->general.elog, 0x10+config->general.status, 0);
    }
}

// convert result to float
float ds18b20_result_to_float(sensor_ds18b20_config_t* config){    
    return ((int16_t) config->data) / 16.0;
}

one_wire_status_t ds18b20_sensor_init(sensor_ds18b20_config_t* config){
    one_wire_status_t status;
    
    sensor_init_common_config(&config->general, SENSOR_DS18B20_CAN_DATA_LENGTH);
    
    // perform a readout to check if sensor active and responsive
    status = ds18b20_start_conversion(config);
    delay_ms(10);
    status = ds18b20_read_result(config);
    
    
    if(status == ONE_WIRE_NO_ERROR){
        sensor_update_status(&config->general, I2C_NO_ERROR);
    } else {
        sensor_update_status(&config->general, I2C_NO_ADDRESS_ACK);
    }
    
    return status;
}
#endif
