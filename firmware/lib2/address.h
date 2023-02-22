#ifndef __DEVICE_ADDRESS_H__
#define __DEVICE_ADDRESS_H__

#include <xc.h>
#include <stdint.h>

/// reserved addresses
#define ADDRESS_GATEWAY 0x00U      ///< gateway address of the readout PC
#define ADDRESS_SEARCH_START 0x01U ///< start address for searching for a free address
#define ADDRESS_UNSET 0xffU        ///< unset address, initial value after reset

#ifdef __cplusplus
extern "C"
{
#endif

    /// The address of the device.
    extern volatile uint16_t controller_address;

    /**
     * @brief Finds a non-reserved address.
     *
     * @details This function is used to find a non-reserved address. Addresses are
     * searched for from ADDRESS_SEARCH_START to ADDRESS_UNSET-1. After an address
     * is selected, a request message is sent. After the external timeout expires
     * and the address was not reset to ADDRESS_UNSET, the address is considered
     * available and assigned to the current node. If all addresses are in use, this
     * function will never assign an address, but will also not thorw an error. The
     * application code should check for this externally.
     */
    void address_find_non_reserved(void);

    /**
     * @brief Sets the address and checks if it is available.
     *
     * @details This function is used to set the address of the current node to
     * a predefined address and check if it is available. If the address is not
     * available, it is not assigned.
     *
     * @param address The address to be set.
     */
    void address_set_and_check_available(uint16_t address);

#ifdef __cplusplus
}
#endif

#endif
