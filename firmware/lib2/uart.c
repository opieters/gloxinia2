#include <uart.h>
#include <utilities.h>
#include <libpic30.h>
#include <address.h>
#include <can.h>
#include <event_controller.h>

#include "i2c.h"

// UART TX FIFO buffer variables
volatile uint8_t n_uart_tx_messages = 0;
volatile uint8_t uart_tx_queue_idx = 0;
volatile uint8_t uart_tx_queue_valid = 0;
volatile uint8_t uart_tx_ongoing = 0;
message_t uart_tx_queue[UART_FIFO_TX_BUFFER_SIZE];
uint8_t uart_tx_data[UART_FIFO_TX_BUFFER_SIZE][UART_TX_DATA_BUFFER_SIZE];

// DMA buffer
uint8_t uart_dma_message_data[128];

extern bool uart_connection_active;

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

message_t uart_rx_queue[UART_FIFO_RX_BUFFER_SIZE];
uint8_t uart_rx_data[UART_FIFO_RX_BUFFER_SIZE][CAN_MAX_N_BYTES];
task_t uart_rx_tasks[UART_FIFO_RX_BUFFER_SIZE];
//uint8_t uart_rx_buffer[PRINT_BUFFER_LENGTH] __attribute__((space(xmemory)));
volatile uart_rx_state_t uart_rx_state = 0;
volatile uint8_t uart_rx_data_idx = 0;
volatile uint8_t uart_rx_idx = 0;
message_t *uart_rx_m;

// debug message and data
message_t *uart_print_message = NULL;

void uart_init(uint32_t baudrate)
{
    unsigned int i;

    U2MODEbits.STSEL = 0; // 1-stop bit
    U2MODEbits.PDSEL = 0; // No Parity, 8-data bits
    U2MODEbits.ABAUD = 0; // Autobaud Disabled
    U2MODEbits.BRGH = 0;  // 16x mode (16 clock cycles per data bit)
    U2MODEbits.RTSMD = 0; // DTE-DTE mode
    U2MODEbits.URXINV = 0;
    U2BRG = ((FCY / baudrate) / 16) - 1; // BAUD Rate Setting for 9600

    U2MODEbits.UEN = 0b10; // use flow control

    //  Configure UART for DMA transfers
    U2STAbits.UTXISEL0 = 0; // Interrupt after one Tx character is transmitted
    U2STAbits.UTXISEL1 = 0;
    U2STAbits.URXISEL = 0; // Interrupt after one RX character is received
    U2STAbits.TXINV = 0;

    //  Enable UART Rx and Tx
    U2MODEbits.UARTEN = 1; // Enable UART
    U2STAbits.UTXEN = 1;   // Enable UART Tx

    // clear error bit, do not enable error interrupt
    _U2EIF = 0;
    _U2EIE = 0;

    // only enable RX interrupt
    _U2RXIE = 1;
    _U2TXIE = 0;
    _U2EIE = 1;

    // enable UART RX and TX
    U2MODEbits.UARTEN = 1; // Enable UART
    U2STAbits.UTXEN = 1;   // Enable UART Tx

    // wait for at least one baud period to continue
    __delay_us(2 * (FCY / baudrate));

    DMA14CONbits.DIR = 1;     // RAM-to-Peripheral
    DMA14CONbits.SIZE = 1;    // byte transfer mode
    DMA14CONbits.MODE = 0b01; // One-Shot, Ping-Pong modes disabled
    DMA14CNT = 0;             // number of  DMA requests
    DMA14REQ = 0x001F;        // Select UART2 transmitter
    DMA14PAD = (volatile unsigned int)&U2TXREG;
    _DMA14IF = 0; // Clear DMA Interrupt Flag
    _DMA14IE = 1; // Enable DMA interrupt

    DMA14STAL = (uint16_t)uart_dma_message_data;
    DMA14STAH = 0x0;

    // update interrupt priority
    _DMA14IP = 7;

    // RX buffer
    for (i = 0; i < UART_RX_BUFFER_SIZE; i++)
    {
        uart_rx_queue[i].status = M_RX_FROM_UART;
    }

    // TX buffer
    for (i = 0; i < UART_FIFO_TX_BUFFER_SIZE; i++)
    {
        uart_tx_queue[i].data = uart_tx_data[i];
        uart_tx_queue[i].length = UART_FIFO_TX_BUFFER_SIZE;
        uart_tx_queue[i].status = M_TX_SENT;
    }
    n_uart_tx_messages = 0;
    uart_tx_queue_idx = 0;
    uart_tx_queue_valid = 0;
    uart_tx_ongoing = 0;
}

void uart_print(const char *message, size_t length)
{
#ifdef __DEBUG__
    if (U2STAbits.UTXEN == 0)
    {
        return;
    }

    // if ((uart_print_message != NULL) && (uart_print_message->status != M_TX_INIT_DONE)) {
    //     while (uart_print_message->status != M_TX_SENT);
    // }

    uart_print_message = &uart_tx_queue[uart_tx_queue_idx];

    message_init(
        uart_print_message,
        controller_address,
        0,
        M_MSG_TEXT,
        NO_SENSOR_ID,
        (uint8_t *)message,
        length);

    message_reset(uart_print_message);
    uart_queue_message(uart_print_message);

    // while(uart_print_message.status != UART_MSG_SENT);
#endif
}

void uart_queue_message(message_t *m)
{
    // wait for space in the queue
    while (n_uart_tx_messages == UART_MESSAGE_BUFFER_LENGTH)
        ;

    if (m->status != M_TX_INIT_DONE)
    {
        return;
    }

    // queue message
    m->status = M_TX_QUEUED;
    uart_tx_queue[uart_tx_queue_idx] = *m;
    message_t *mtx = &uart_tx_queue[uart_tx_queue_idx];

    // we only need to check this for messages other than the special print message
    if (m != uart_print_message)
    {
        mtx->length = MIN(UART_TX_DATA_BUFFER_SIZE, m->length);
        mtx->data = uart_tx_data[uart_tx_queue_idx];

        for (unsigned int i = 0; i < mtx->length; i++)
        {
            mtx->data[i] = m->data[i];
        }
    }

    uart_tx_queue_idx = (uart_tx_queue_idx + 1) % UART_FIFO_TX_BUFFER_SIZE;
    n_uart_tx_messages++;

    process_uart_tx_queue();
}

void process_uart_tx_queue(void)
{
    message_t *m;

    if ((uart_tx_ongoing == 0) && (uart_tx_queue_idx != uart_tx_queue_valid) && (n_uart_tx_messages > 0))
    {
        // copy to actual message to transmit

        uart_tx_ongoing = 1;

        // start transmission of message
        m = &uart_tx_queue[uart_tx_queue_valid];

        uart_parse_to_raw_buffer(uart_dma_message_data, m,
                                 ARRAY_LENGTH(uart_dma_message_data));

        while (U2STAbits.TRMT == 0)
            ;

        m->status = M_TX_SENT;
        DMA14STAL = (uint16_t)uart_dma_message_data;
        DMA14STAH = 0x0;
        DMA14CNT = UART_HEADER_SIZE + (m->length) - 1;

        // start transfer
        DMA14CONbits.CHEN = 1;
        DMA14REQbits.FORCE = 1;
    }
}

void uart_parse_to_raw_buffer(uint8_t *data, message_t *m, const size_t max_length)
{
    uint16_t i;

    m->length = MIN(m->length, max_length - 9);

    data[0] = UART_CMD_START;
    data[1] = (uint8_t)(m->identifier >> 8);
    data[2] = (uint8_t)m->identifier;
    data[3] = m->command;
    data[4] = m->request_message_bit;
    data[5] = (uint8_t)(m->sensor_identifier >> 8);
    data[6] = (uint8_t)m->sensor_identifier;
    data[7] = (uint8_t)m->length;
    for (i = 0; i < m->length; i++)
    {
        data[8 + i] = m->data[i];
    }
    data[8 + m->length] = UART_CMD_STOP;
}

void __attribute__((interrupt, no_auto_psv)) _DMA14Interrupt(void)
{
    // finish the current transfer
    // uart_tx_queue[uart_tx_queue_valid].status = M_TX_SENT;
    uart_tx_queue_valid = (uart_tx_queue_valid + 1) % UART_FIFO_TX_BUFFER_SIZE;
    n_uart_tx_messages--;

    // check if another message is available
    uart_tx_ongoing = 0;
    process_uart_tx_queue();

    _DMA14IF = 0; // Clear the DMA14 Interrupt Flag
}

void __attribute__((interrupt, no_auto_psv)) _U2ErrInterrupt(void)
{
    uart_rx_m->status = M_ERROR_HW_OVERFLOW;
    U2STAbits.OERR = 0;
    _U2EIF = 0; // Clear the UART2 Error Interrupt Flag
}

void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void)
{

    register uint8_t rx_value = U2RXREG;
    //register message_t *m = &(uart_rx_queue[(n_uart_rx_messages + uart_rx_read_idx) % UART_RX_BUFFER_SIZE]);
    
    //if (n_uart_rx_messages == UART_RX_BUFFER_SIZE)
    //{
    //    _U2RXIF = 0;
    //}

    switch (uart_rx_state)
    {
    case UART_RX_STATE_FIND_START_BYTE:
        if (rx_value == UART_CMD_START)
        {
            uart_rx_state = UART_RX_STATE_READ_IDH;
            uart_rx_m = &uart_rx_queue[uart_rx_idx];
            uart_rx_m->data = uart_rx_data[uart_rx_idx];
            uart_rx_m->status = M_EMPTY;
        }
        break;
    case UART_RX_STATE_READ_IDH:
        uart_rx_m->identifier = rx_value;
        uart_rx_state = UART_RX_STATE_READ_IDL;
        break;
    case UART_RX_STATE_READ_IDL:
        uart_rx_m->identifier = (uart_rx_m->identifier << 8) | rx_value;
        uart_rx_state = UART_RX_STATE_READ_CMD;
        break;
    case UART_RX_STATE_READ_CMD:
        uart_rx_m->command = rx_value;
        uart_rx_state = UART_RX_STATE_READ_REQUEST;
        break;
    case UART_RX_STATE_READ_REQUEST:
        uart_rx_m->request_message_bit = rx_value;
        // uart_rx_m->status = UART_MSG_TRANSFERRED;
        uart_rx_state = UART_RX_STATE_READ_SIDH;
        break;
    case UART_RX_STATE_READ_SIDH:
        uart_rx_m->sensor_identifier = rx_value;
        uart_rx_state = UART_RX_STATE_READ_SIDL;

        break;
    case UART_RX_STATE_READ_SIDL:
        uart_rx_m->sensor_identifier = (uart_rx_m->sensor_identifier << 8) | rx_value;
        uart_rx_state = UART_RX_STATE_READ_LENGTH;
        break;
    case UART_RX_STATE_READ_LENGTH:
        uart_rx_m->length = rx_value;
        if (rx_value > 0)
        {
            uart_rx_state = UART_RX_STATE_READ_DATA;
            uart_rx_data_idx = 0;
        }
        else
        {
            uart_rx_state = UART_RX_STATE_DETECT_STOP;
        }

        break;
    case UART_RX_STATE_READ_DATA:
        // only write data until buffer is full
        if (uart_rx_data_idx < PRINT_BUFFER_LENGTH)
        {
            uart_rx_m->data[uart_rx_data_idx] = rx_value;
        }
        else
        {
            // continue reading data, but indicate error occurred
            uart_rx_m->status = M_ERROR_OVERFLOW;
        }

        uart_rx_data_idx++;

        if (uart_rx_data_idx == uart_rx_m->length)
        {
            uart_rx_state = UART_RX_STATE_DETECT_STOP;
        }

        break;
    case UART_RX_STATE_DETECT_STOP:
        if (rx_value == UART_CMD_STOP)
        {
            if (uart_rx_m->status < M_ERROR)
            {
                uart_rx_m->status = M_RX_FROM_UART;
            }
        }
        else
        {
            uart_rx_m->status = M_ERROR_STOP_DETECT;
        }

        //n_uart_rx_messages++;
        uart_rx_state = UART_RX_STATE_FIND_START_BYTE;

        uart_connection_active = true;

        if ((uart_rx_m->identifier == controller_address) || (uart_rx_m->identifier == ADDRESS_GATEWAY))
        {
            send_message_uart(uart_rx_m);
            
            // schedule processing task
            task_t task = uart_rx_tasks[uart_rx_idx];
            task.cb =(void*) message_process;
            task.data = (void*) &uart_rx_queue[uart_rx_idx];
            push_queued_task(task);
        }
        else
        {
            // forward this message over CAN
            send_message_can(uart_rx_m);
        }
        
        uart_rx_idx = (uart_rx_idx+1) % UART_FIFO_RX_BUFFER_SIZE;

        break;
    default:
        uart_rx_state = UART_RX_STATE_FIND_START_BYTE;
        break;
    }
    _U2RXIF = 0;
}

void __attribute__((interrupt,no_auto_psv)) _U2TXInterrupt(void)
{
    _U2TXIF = 0;
}
