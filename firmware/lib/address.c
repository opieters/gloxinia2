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
#include <can.h>
#include <stdbool.h>

/**
 * @brief Variable containing the device address.
 * 
 * @details The device address is read by means of the 
 * `i2c_device_address_transceiver` FSM and is NOT set at start-up. An 
 * explicit I2C transfer is required to initialise the address.
 */
volatile uint16_t controller_address = 0;

extern int16_t n_connected_can_devices;

void get_device_address(void) {
    bool free_address_found = false;
    can_message_t m;
    int16_t temp_can_address = 2;

    if (n_connected_can_devices > 0) {
        while (!free_address_found) {
            controller_address = temp_can_address;
            // init message
            can_init_message(&m, controller_address, CAN_NO_REMOTE_FRAME, CAN_EXTENDED_FRAME, CAN_HEADER(CAN_REQUEST_ADDRESS_AVAILABLE, 0), NULL, 0);

            // send message to check for address availability
            can_send_message_any_ch(&m);

            // wait for one second
            delay_ms(1000);

            // the address is reset if it is already taken -> check and try again if needed
            if (controller_address != 0) {
                free_address_found = true;
            } else {
                temp_can_address++;
            }
        }

        // send the final address to everybody
        set_device_address(temp_can_address);

    } else {
        controller_address = 1;
    }
}

void set_device_address(uint16_t address) {
    can_message_t m;

    controller_address = address;

    can_init_message(&m, controller_address, CAN_NO_REMOTE_FRAME, CAN_EXTENDED_FRAME, CAN_HEADER(CAN_REQUEST_ADDRESS_AVAILABLE, 0), NULL, 0);

    // send message to check for address availability
    can_send_message_any_ch(&m);
}