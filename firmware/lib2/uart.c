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
volatile uint8_t n_uart_tx_messages2 = 0;
volatile uint8_t uart_tx_queue_idx2 = 0;
volatile uint8_t uart_tx_queue_valid2 = 0;
uart_message_t uart_tx_queue2[UART_FIFO_TX_PRINT_BUFFER_SIZE];
uint8_t uart_tx_data2[UART_FIFO_TX_PRINT_BUFFER_SIZE][UART_FIFO_TX_PRINT_BUFFER_DATA_SIZE];
bool uart_tx_text = false;

// DMA buffer
uint8_t uart_dma_tx_message_data[128];
uint8_t uart_dma_rx_message_data[16];

extern bool uart_connection_active;

message_t uart_rx_queue[UART_FIFO_RX_BUFFER_SIZE];
task_t uart_rx_tasks[UART_FIFO_RX_BUFFER_SIZE];
//volatile uart_rx_state_t uart_rx_state = 0;
//volatile uint8_t uart_rx_data_idx = 0;
volatile uint8_t uart_rx_idx = 0;
//message_t *uart_rx_m;

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

#ifdef __DICIO__
    U2MODEbits.UEN = 0b10; // use flow control
#else 
    U2MODEbits.UEN = 0b00; // no control flow
#endif
    
    //  Configure UART for DMA transfers
    U2STAbits.UTXISEL0 = 0; // Interrupt after one Tx character is transmitted
    U2STAbits.UTXISEL1 = 0;
    U2STAbits.URXISEL = 0; // Interrupt after one RX character is received
    U2STAbits.TXINV = 0;

    //  Enable UART Rx and Tx
    U2MODEbits.UARTEN = 1; // Enable UART
    U2STAbits.UTXEN = 1;   // Enable UART Tx
    
    // only enable RX interrupt
    _U2RXIE = 0;
    _U2TXIE = 0;
    
    // clear error bit, enable error interrupt
    _U2EIE = 1;
    _U2EIF = 0;

    // enable UART RX and TX
    U2MODEbits.UARTEN = 1; // Enable UART
    U2STAbits.UTXEN = 1;   // Enable UART Tx

    // wait for at least one baud period to continue
    __delay_us(2 * (FCY / baudrate));

    DMA12CONbits.DIR = 1;     // RAM-to-Peripheral
    DMA12CONbits.SIZE = 1;    // byte transfer mode
    DMA12CONbits.MODE = 0b01; // One-Shot, Ping-Pong modes disabled
    DMA12CNT = 0;             // number of  DMA requests
    DMA12REQ = 0x001F;        // Select UART2 transmitter
    DMA12PAD = (volatile unsigned int)&U2TXREG;
    _DMA12IF = 0; // Clear DMA Interrupt Flag
    _DMA12IE = 1; // Enable DMA interrupt

    DMA12STAL = __builtin_dmaoffset(uart_dma_tx_message_data);
    DMA12STAH = __builtin_dmapage(uart_dma_tx_message_data);

    // update interrupt priority
    _DMA12IP = 7;
    
    DMA13CONbits.DIR = 0;     // RAM-to-Peripheral
    DMA13CONbits.SIZE = 1;    // byte transfer mode
    DMA13CONbits.MODE = 0b01; // One-Shot, Ping-Pong modes disabled
    DMA13CNT = ARRAY_LENGTH(uart_dma_rx_message_data) - 1;             // number of  DMA requests
    DMA13REQ = 0b00011110;        // Select UART2 transmitter
    DMA13PAD = (volatile unsigned int)&U2RXREG;
    DMA13STAL = __builtin_dmaoffset(uart_dma_rx_message_data);
    DMA13STAH = __builtin_dmapage(uart_dma_rx_message_data);
    DMA13CONbits.CHEN = 1;
    _DMA13IF = 0; // Clear DMA Interrupt Flag
    _DMA13IE = 1; // Enable DMA interrupt
    
    // update interrupt priority
    _DMA13IP = 7;

    // RX buffer
    for (i = 0; i < UART_FIFO_RX_BUFFER_SIZE; i++)
    {
        uart_rx_queue[i].status = M_RX_FROM_UART;
    }

    // TX buffer
    for (i = 0; i < ARRAY_LENGTH(uart_tx_queue); i++)
    {
        uart_tx_queue[i].length = UART_FIFO_TX_DATA_BUFFER_SIZE;
        uart_tx_queue[i].status = M_TX_SENT;
    }
    
    n_uart_tx_messages = 0;
    uart_tx_queue_idx = 0;
    uart_tx_queue_valid = 0;
    
    for(i = 0; i < ARRAY_LENGTH(uart_tx_queue2); i++)
    {
        uart_tx_queue2[i].data = NULL;
        uart_tx_queue2[i].length = 0;
    }
    
    n_uart_tx_messages2 = 0;
    uart_tx_queue_idx2 = 0;
    uart_tx_queue_valid2 = 0;
    
    uart_tx_ongoing = 0;
}

void uart_print(const char *message, size_t length)
{
#ifdef __DEBUG__
    if (U2STAbits.UTXEN == 0)
    {
        return;
    }
    
    // wait for space in the queue
    while (n_uart_tx_messages2 == ARRAY_LENGTH(uart_tx_queue2))
        ;

    uart_message_t* m = &uart_tx_queue2[uart_tx_queue_idx2];

    m->data = uart_tx_data2[uart_tx_queue_idx2];
    m->length = MIN(length, ARRAY_LENGTH(uart_tx_data2[uart_tx_queue_idx2]));
    for(int i = 0; i < m->length; i++)
    {
        m->data[i] = message[i];
    }

    uart_tx_queue_idx2 = (uart_tx_queue_idx2 + 1) % ARRAY_LENGTH(uart_tx_queue2);
    n_uart_tx_messages2++;

    process_uart_tx_queue();
#endif
}

void uart_queue_message(message_t *m)
{
    // wait for space in the queue
    while (n_uart_tx_messages == (ARRAY_LENGTH(uart_tx_queue) - 1))
        ;

    if (m->status != M_TX_INIT_DONE)
    {
        return;
    }

    // queue message
    m->status = M_TX_QUEUED;
    uart_tx_queue[uart_tx_queue_idx] = *m;
    /*message_t *mtx = &uart_tx_queue[uart_tx_queue_idx];


    mtx->length = MIN(UART_FIFO_TX_DATA_BUFFER_SIZE, m->length);

    for (unsigned int i = 0; i < mtx->length; i++)
    {
        mtx->data[i] = m->data[i];
    }*/

    uart_tx_queue_idx = (uart_tx_queue_idx + 1) % ARRAY_LENGTH(uart_tx_queue);
    n_uart_tx_messages++;

    process_uart_tx_queue();
}

void process_uart_tx_queue(void)
{
    if ((uart_tx_ongoing == 0) && (n_uart_tx_messages > 0))
    {
        // copy to actual message to transmit

        uart_tx_ongoing = 1;
        uart_tx_text = false;

        // start transmission of message
        message_t* m = &uart_tx_queue[uart_tx_queue_valid];

        uart_parse_to_raw_buffer(uart_dma_tx_message_data, m,
                                 ARRAY_LENGTH(uart_dma_tx_message_data));

        while (U2STAbits.TRMT == 0)
            ;

        m->status = M_TX_SENT;
        DMA12STAL = __builtin_dmaoffset(uart_dma_tx_message_data);
        DMA12STAH = __builtin_dmapage(uart_dma_tx_message_data);
        DMA12CNT = UART_HEADER_SIZE + (m->length) - 1;

        // start transfer
        DMA12CONbits.CHEN = 1;
        DMA12REQbits.FORCE = 1;
    }
    else if ((uart_tx_ongoing == 0) && (n_uart_tx_messages2 > 0))
    {
        uart_tx_ongoing = 1;
        uart_tx_text = true;

        // start transmission of message
        uart_message_t* m = &uart_tx_queue2[uart_tx_queue_valid2];
        
        uart_dma_tx_message_data[0] = UART_CMD_START;
        uart_dma_tx_message_data[1] = (uint8_t)(controller_address >> 8);
        uart_dma_tx_message_data[2] = (uint8_t) controller_address;
        uart_dma_tx_message_data[3] = M_MSG_TEXT;
        uart_dma_tx_message_data[4] = CAN_NO_REMOTE_FRAME;
        uart_dma_tx_message_data[5] = 0;
        uart_dma_tx_message_data[6] = (uint8_t) (NO_INTERFACE_ID << 4) | NO_SENSOR_ID;
        uart_dma_tx_message_data[7] = (uint8_t)m->length;

        uart_dma_tx_message_data[8 + m->length] = UART_CMD_STOP;
                
        // copy to actual message to transmit
        for (int i = 0; i < m->length; i++)
        {
            uart_dma_tx_message_data[8 + i] = m->data[i];
        }

        while (U2STAbits.TRMT == 0)
            ;

        DMA12STAL = __builtin_dmaoffset(uart_dma_tx_message_data);
        DMA12STAH = __builtin_dmapage(uart_dma_tx_message_data);
        DMA12CNT = UART_HEADER_SIZE + (m->length) - 1;

        // start transfer
        DMA12CONbits.CHEN = 1;
        DMA12REQbits.FORCE = 1;
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
    data[5] = (uint8_t) 0;
    data[6] = (uint8_t) (m->interface_id << 4) | m->sensor_id;
    data[7] = (uint8_t)m->length;
    for (i = 0; i < m->length; i++)
    {
        data[8 + i] = m->data[i];
    }
    data[8 + m->length] = UART_CMD_STOP;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _DMA12Interrupt ( void )
{
    // finish the current transfer
    // uart_tx_queue[uart_tx_queue_valid].status = M_TX_SENT;
    uart_tx_ongoing = 0;
    if(!uart_tx_text)
    {
        uart_tx_queue_valid = (uart_tx_queue_valid + 1) % ARRAY_LENGTH(uart_tx_queue);
        n_uart_tx_messages--;
        
    }
    else
    {
        uart_tx_queue_valid2 = (uart_tx_queue_valid2 + 1) % ARRAY_LENGTH(uart_tx_queue2);
        n_uart_tx_messages2--;
    } 

    // check if another message is available 
    process_uart_tx_queue();

    _DMA12IF = 0; // Clear the DMA12 Interrupt Flag
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _DMA13Interrupt ( void )
{   
    uart_connection_active = true;
    
    if((uart_dma_rx_message_data[0] == UART_CMD_START) && (uart_dma_rx_message_data[ARRAY_LENGTH(uart_dma_rx_message_data)-1] == UART_CMD_STOP)){
        // process message
        
        message_t* m = &uart_rx_queue[uart_rx_idx];
        message_init(m, 
                (uart_dma_rx_message_data[1] << 8) | uart_dma_rx_message_data[2],
                uart_dma_rx_message_data[5],
                uart_dma_rx_message_data[3],
                (uart_dma_rx_message_data[4] >> 4) & 0xf,
                uart_dma_rx_message_data[4] & 0xf,
                &uart_dma_rx_message_data[7],
                uart_dma_rx_message_data[6]
                );
        m->status = M_RX_FROM_UART;
        
        if ((m->identifier == controller_address) || (m->identifier == ADDRESS_GATEWAY))
        {
            // schedule processing task
            task_t task = uart_rx_tasks[uart_rx_idx];
            task.cb =(void*) message_process;
            task.data = (void*) m;
            push_queued_task(task);
            
            send_message_uart(m);
        }
        else
        {
            // forward this message over CAN (this needs to be done in main loop
            // since EDS space is written during send_message_can call)
            task_t task = uart_rx_tasks[uart_rx_idx];
            task.cb =(void*) send_message_can;
            task.data = (void*) &uart_rx_queue[uart_rx_idx];
            push_queued_task(task);
        }
        
        

        uart_rx_idx = (uart_rx_idx+1) % UART_FIFO_RX_BUFFER_SIZE;
        
        // restore defaults
        DMA13CNT = ARRAY_LENGTH(uart_dma_rx_message_data)-1;
        DMA13STAL = (unsigned int) &uart_dma_rx_message_data[0];
    } else {
                // search for start condition
        for(int i = 1; i < ARRAY_LENGTH(uart_dma_rx_message_data); i++){
            if(uart_dma_rx_message_data[i] == UART_CMD_START){
                DMA13CNT = i - 1;
                
                // move data to correct position
                for(int j = i; j < ARRAY_LENGTH(uart_dma_rx_message_data); j++){
                    uart_dma_rx_message_data[j-i] = uart_dma_rx_message_data[j];
                }
                
                // append data
                DMA13STAL = (unsigned int) &uart_dma_rx_message_data[ARRAY_LENGTH(uart_dma_rx_message_data)-i];
                break;
            }
        }
    }
    
    // start next transfer
    DMA13CONbits.CHEN = 1;
    
    _DMA13IF = 0;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2ErrInterrupt ( void )
{
    UART_DEBUG_PRINT("UART2 ERR IN IFS");
    
    //uart_rx_m->status = M_ERROR_HW_OVERFLOW;
    U2STAbits.OERR = 0;
    _U2EIF = 0; // Clear the UART2 Error Interrupt Flag
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2RXInterrupt ( void )
{
    _U2RXIF = 0;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2TXInterrupt ( void )
{
    _U2TXIF = 0;
}
