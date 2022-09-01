/* 
 * File: address.h  
 * Author: opieters
 * Comments:
 * Revision history: 0.0.1
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __DEVICE_ADDRESS_H__
#define	__DEVICE_ADDRESS_H__ 

#include <xc.h>
#include <utilities.h>
#include <i2c.h>

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
    
    //i2c_error_status_t get_device_address(void);
    
    extern volatile uint16_t controller_address;
    
    void get_device_address(void);
    


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __ADDRESS_H__ */
