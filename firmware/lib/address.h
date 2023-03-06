#ifndef __DEVICE_ADDRESS_H__
#define	__DEVICE_ADDRESS_H__ 

#include <xc.h>
#include <utilities.h>
#include <i2c.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    //i2c_error_status_t get_device_address(void);

    extern volatile uint16_t controller_address;

    void get_device_address(void);
    void set_device_address(uint16_t address);




#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __ADDRESS_H__ */
