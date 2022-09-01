/*
 * File:   address.c
 * Author: opieters
 *
 * Created on June 28, 2018, 2:52 PM
 */

#include <xc.h>
#include <stdint.h>
#include "address.h"
#include <gpio_expander.h>
/**
 * @brief Variable containing the device address.
 * 
 * @details The device address is read by means of the 
 * `i2c_device_address_transceiver` FSM and is NOT set at start-up. An 
 * explicit I2C transfer is required to initialise the address.
 */
volatile uint16_t controller_address;
   

void get_device_address(void){
    
}