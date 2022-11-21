#ifndef __UART_H__
#define	__UART_H__

#include <xc.h>
#include <stdint.h>
#include <stddef.h>
#include <message.h>

#define UART_CMD_START ('\x01')
#define UART_CMD_STOP  ('\x04')

#define UART_MESSAGE_BUFFER_LENGTH 64U
#define UART_HEADER_SIZE 9U

// UART RX cache size before processig (FIFO)
#define UART_RX_BUFFER_SIZE 4U // number of unique messages
#define PRINT_BUFFER_LENGTH 64 // max length of message data

// UART FIFO TX buffer
#define TXIE 16U
#define UART_TX_DATA_BUFFER_SIZE 16




#ifdef	__cplusplus
extern "C" {
#endif

    void uart_init(uint32_t baudrate);
    
    
    void message_reset(message_t* m);
    
    //extern volatile size_t n_uart_rx_messages;

    //void uart_rx_callback(void);

    void uart_queue_message(message_t* m);

    void uart_parse_to_raw_buffer(uint8_t* data, message_t* m, const size_t max_length);

    //void uart_await_tx(uart_message_t* m);

    //void uart_wait(void);

    //void uart_log_init(uint32_t baudrate);

    void uart_print(const char* message, size_t lengh);
    
    void uart_send_message(message_t* m);
    
    void process_uart_tx_queue(void);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __UART_H__ */

