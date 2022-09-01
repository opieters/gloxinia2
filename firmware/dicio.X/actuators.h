#ifndef __ACTUATORS_H__
#define	__ACTUATORS_H__

#include <xc.h>  
#include <uart.h>
#include <stdbool.h>
#include "actuator_common.h"

#define N_ACTUATOR_RELAY_BOARD 4
#define ACTIVATE_GC 
#define ACTUATOR_PERIOD (100) // expressed in in 100ms, 16-bit value! Should be at least 5

#define ACTUATOR_ERROR_TH      1

#define DICIO_ACTUATOR_STATUS_LOG_MESSAGE 3

#ifdef	__cplusplus
extern "C" {
#endif 
   
    
    /*
     * Initialises the configuration data vectors of all actuators that support
     * multiple instantiations. 
     */
    void actuators_data_init(void);

    /*
     * Initialise the actuators (calls each of the initialisers)
     */
    void actuators_init(void);

    /*
     * Can be used to activate dedicated actuator timers prior to the actual start
     * of the experiment. This is only exectuted just before the main loop. 
     */
    void actuators_start(void);

    /*
     * Callback runs every 100ms and executes the actuator code
     */
    void actuator_callback(void);
    
    bool process_actuator_serial_command(uart_message_t* m);
    
    void actuators_error_recover(void);
    
    void actuators_error_check(void);
    
    void actuator_status_report(void);
    
    void actuator_send_status(actuator_general_config_t* config);
    
#ifdef	__cplusplus
}
#endif

#endif

