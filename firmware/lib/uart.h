#ifndef __UART_H__
#define	__UART_H__

#include <xc.h>
#include <stdint.h>
#include <utilities.h>
#include <stddef.h>
#include <uart_common.h>




#define uart_simple_print(X) uart_print(X, strlen(X));



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern volatile size_t n_uart_rx_messages;
    
    void uart_rx_callback(void);
    
    void uart_queue_message(uart_message_t* m);
    
    void uart_parse_to_buffer(uint8_t* data, uart_message_t* m, size_t max_length);
    
    void uart_await_tx(uart_message_t* m);
    
    void uart_wait(void);
    
    void uart_log_init(uint32_t baudrate);
    
    void uart_print(const char* message, size_t lengh);
    

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __UART_H__ */

