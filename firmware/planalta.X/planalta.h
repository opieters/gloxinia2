
#ifndef __PLANALTA_H__
#define	__PLANALTA_H__

#include <xc.h> // include processor files - each processor file is guarded.  
#include <adc.h>
#include <pga.h>
#include <i2c.h>
#include "filters_planalta.h"
#include "dac.h"
#include "planalta_filters.h"
#include "planalta_definitions.h"

extern fractional planalta_fs_data[PLANALTA_FS_CHANNELS][PLANALTA_FS_FREQ_N][2];
extern i2c_message_t i2c_mr_message;
extern uint8_t i2c_mr_message_data[PLANALTA_I2C_READ_BUFFER_LENGTH];

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
    
    /*
     * Initialises all the relevant hardware that does not depend upon user
     * configuration. All user specific configuration options should be set 
     * after this function
     */
    void init_planalta(void);
    
    /*
     * Executes the main function of the application. This includes reading 
     * incoming I2C data that changes the configuration of the sensor, starts 
     * the operation (ADC sampling + filtering) and reads the generated data.
     * 
     */
    void loop_planalta(void);
    
    /*
     * Reads the user specified I2C address from the dip-switch on the board
     * and returns a valid I2C address.
     * 
     * When the I2C address that the user entered is not valid, a fallback 
     * address is used. This address is defined in `PLANALTA_I2C_BASE_ADDRESS`. 
     * Reading the address required correct initialisation of the corresponding
     * pins defined in `planalta_config_t.address_selection`. Only the least 
     * significant 7 bits are used. The most significant bit is reserved for
     * future and should be configured to 0.
     * 
     * The address selection uses inverted logic to make it easier to configure 
     * the address without using a voltage meter. Setting a bit in the ON-state
     * will turn the corresponding bit in the I2C-address to 1.
     * 
     * This function is called in `init_planalta`. 
     */
    uint8_t i2c_get_address_planalta(planalta_config_t* config);
    
    /*
     * Initialises all hardware pins on the board to the correct configuration.
     * This includes all the peripheral pins that are needed for various 
     * protocols (I2C, SPI, UART) and application specific functions (wave 
     * generation, ADC triggering...).
     * 
     * This function is called when appropriate by `init_planalta`. 
     */
    void init_pins_planalta(void);
    
    /*
     * I2C callback after completion of a master write (slave read) operation.
     * 
     * Note that this callback is executed after the execution of the message 
     * specific callback function.
     */
    void i2c_mw_sr_cb_planalta(i2c_message_t* m);
    
    /*
     * I2C callback after completion of a master read (slave write) operation.
     * 
     * Note that this callback is executed after the execution of the message 
     * specific callback function.
     */
    void i2c_mr_sw_cb_planalta(i2c_message_t* m);
    
    /*
     * Clears all sampling-related buffers. This includes the temporary storage
     * buffers, delay buffers and I2C message buffers.
     */
    void planalta_clear_buffers(void);
    
    
    void planalta_i2c_channel_config(uint8_t channel_n, uint8_t* data);
    
    void planalta_i2c_read_copy_buffer_data();
    
    
    void planalta_set_filters(planalta_config_t* config);
    
    void planalta_channel_config(uint8_t channel_n, 
        planalta_channel_status_t status, pga_gain_t gain);

    void planalta_sweep_frequency(struct planalta_config_s* config);
    
    void planalta_fs_i2c_copy_data();
    
#ifdef	__cplusplus
}
#endif

#endif

