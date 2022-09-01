#ifndef UART_PROCESSING_H
#define	UART_PROCESSING_H

#include <xc.h>
#include <uart.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
void uart_init_message(uart_message_t* m, 
            serial_cmd_t command,
            uint8_t id,
            uint16_t extended_id,            
            uint8_t* data,
            size_t length);
    
    void uart_queue_message(uart_message_t* m);
    
    void uart_parse_to_buffer(uint8_t* data, uart_message_t* m, size_t max_length);
    
    void uart_await_tx(uart_message_t* m);
    
    void uart_reset_message(uart_message_t* m);
void uart_rx_command_cb_dicio(void);


#ifdef	__cplusplus
}
#endif

#endif

