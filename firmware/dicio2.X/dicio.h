#ifndef __DICIO_H__
#define	__DICIO_H__ 

#include <xc.h>
#include <utilities.h>
#include <uart.h>


#ifdef	__cplusplus
extern "C" {
#endif

    void dicio_init(void);
    
    void dicio_send_ready_message(void);
    
    void dicio_init_pins(void);

#ifdef	__cplusplus
}
#endif

#endif	/* __DICIO_H__ */
