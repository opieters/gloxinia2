#ifndef __UART_H__
#define	__UART_H__

#include <xc.h>
#include <stdint.h>
#include <stddef.h>
#include <message.h>

#define UART_CMD_START ('\x5B')
#define UART_CMD_STOP  ('\x5D')

#define UART_MESSAGE_BUFFER_LENGTH 64U
#define UART_HEADER_SIZE 9U

// UART RX cache size before processig (FIFO)
#define UART_RX_BUFFER_SIZE 4U // number of unique messages
#define PRINT_BUFFER_LENGTH 64 // max length of message data

// UART FIFO TX buffer
#define UART_FIFO_TX_BUFFER_SIZE 16U
#define UART_FIFO_RX_BUFFER_SIZE 16U
#define UART_TX_DATA_BUFFER_SIZE 16


// receive buffer variables
typedef enum
{
    UART_RX_STATE_FIND_START_BYTE,
    UART_RX_STATE_READ_IDH,
    UART_RX_STATE_READ_IDL,
    UART_RX_STATE_READ_CMD,
    UART_RX_STATE_READ_REQUEST,
    UART_RX_STATE_READ_SIDH,
    UART_RX_STATE_READ_SIDL,
    UART_RX_STATE_READ_LENGTH,
    UART_RX_STATE_READ_DATA,
    UART_RX_STATE_DETECT_STOP
} uart_rx_state_t;

// receive buffer variables
typedef enum
{
    UART_TX_STATE_SEND_START_BYTE,
    UART_TX_STATE_SEND_IDH,
    UART_TX_STATE_SEND_IDL,
    UART_TX_STATE_SEND_CMD,
    UART_TX_STATE_SEND_REQUEST,
    UART_TX_STATE_SEND_SIDH,
    UART_TX_STATE_SEND_SIDL,
    UART_TX_STATE_SEND_LENGTH,
    UART_TX_STATE_SEND_DATA,
    UART_TX_STATE_SEND_STOP,
    UART_TX_STATE_DONE,
} uart_tx_state_t;


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

