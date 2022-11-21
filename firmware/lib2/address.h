#ifndef __DEVICE_ADDRESS_H__
#define	__DEVICE_ADDRESS_H__ 

#include <xc.h>
#include <stdint.h>

// reserved addresses
#define ADDRESS_GATEWAY      0x00U
#define ADDRESS_SEARCH_START 0x01U
#define ADDRESS_UNSET        0xffU

#ifdef	__cplusplus
extern "C" {
#endif

    extern volatile uint16_t controller_address;

    void address_get(void);
    void address_set(uint16_t address);

#ifdef	__cplusplus
}
#endif

#endif
