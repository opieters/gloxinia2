#include <xc.h>
#include <stdint.h>
#include "address.h"
#include <stdbool.h>
#include <can.h>
#include <libpic30.h>

/**
 * @brief Variable containing the device address.
 * 
 * @details The device address is read by means of the 
 * `i2c_device_address_transceiver` FSM and is NOT set at start-up. An 
 * explicit I2C transfer is required to initialise the address.
 */
volatile uint16_t controller_address = ADDRESS_UNSET;
static uint16_t temp_can_address = ADDRESS_SEARCH_START;

extern int16_t n_connected_devices;

void address_find_non_reserved(void) {
    message_t m;
    
    if(controller_address != ADDRESS_UNSET){
        return;
    }
    
    controller_address = temp_can_address;
    message_init(&m, controller_address, CAN_NO_REMOTE_FRAME, M_REQUEST_ADDRESS_AVAILABLE, NO_INTERFACE_ID, NO_SENSOR_ID, NULL, 0);
    
    // configure CAN buffer for this address
    can_configure_filter(14, controller_address, 0x0, true, 0, CAN_FIFO_BUFFER);

    // send message to check for address availability
    message_send(&m);
    
    temp_can_address--;
}

void address_set_and_check_available(uint16_t address) {
    message_t m;

    controller_address = address;

    // check if address is available
    message_init(&m, controller_address, CAN_NO_REMOTE_FRAME, M_REQUEST_ADDRESS_AVAILABLE, NO_INTERFACE_ID, NO_SENSOR_ID, NULL, 0);

    // send message to check for address availability
    message_send(&m);
}