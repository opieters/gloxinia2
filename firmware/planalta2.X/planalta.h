#ifndef __PLANALTA_H__
#define	__PLANALTA_H__

#include <xc.h> 
#include <dsp.h>
#include <i2c.h>

#define PLANALTA_N_CHANNELS               8
#define PLANALTA_N_ADC_CHANNELS           8
#define PLANALTA_ADC16_BUFFER_LENGTH      200

#define PLANALTA_F2_INPUT_SIZE 10
#define PLANALTA_F3_INPUT_SIZE 10
#define PLANALTA_F4_INPUT_SIZE 10
#define PLANALTA_F5_INPUT_SIZE 20

#define PLANALTA_STATUS_ON           (0 << 7)
#define PLANALTA_STATUS_OFF          (1 << 7)
#define PLANALTA_STATUS_RESET_BUFFER (1 << 6)
#define PLANALTA_STATUS_RESET        (1 << 5)

#define PLANALTA_ADC_ON (1<<7)



#define PLANALTA_N_ADDRESS_SEL_PINS 8

#ifdef	__cplusplus
extern "C" {
#endif
    
    void planalta_init(void);
    void planalta_send_ready_message(void *data);
    void planalta_init_pins(void);
    void planalta_filters_init(void);
    void planalta_clear_filter_buffers(void);
    void planalta_adc16_callback(void);
    
    
#ifdef	__cplusplus
}
#endif 

#endif

