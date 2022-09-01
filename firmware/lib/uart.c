#include <uart.h>
#include <stdint.h>
#include <utilities.h>
#include <device_configuration.h>
#include <can.h>
#include <address.h>


// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************

volatile size_t n_uart_rx_messages = 0;
volatile size_t uart_rx_read_idx = 0;  // current or previous read index

uart_message_t uart_rx_queue[UART_RX_BUFFER_SIZE];

uart_message_t* uart_queue[UART_MESSAGE_BUFFER_LENGTH];
uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE][PRINT_BUFFER_LENGTH] __attribute__( (space(xmemory)) );



volatile uint8_t n_uart_messages = 0;
volatile uint8_t uart_queue_idx = 0;
volatile uint8_t uart_queue_valid = 0;
volatile uint8_t uart_ongoing = 0;

uart_message_t uart_loopback;
uint8_t uart_loopback_data[PRINT_BUFFER_LENGTH];

uint8_t uart_dma_message_data[128];

volatile uint8_t uart_rx_state = 0;
volatile uint8_t uart_rx_idx = 0;

uart_message_t uart_print_message = {.status = UART_MSG_SENT};
uint8_t uart_print_message_data[128];
uart_message_t uart_command_message;
uint8_t uart_command_message_data[UART_MESSAGE_BUFFER_LENGTH];

void (*uart_rx_command_cb)(void) = uart_rx_command_cb_dummy;

void uart_rx_command_cb_dummy(void){
    
}

void uart_set_callback(void (*cb)(void) ){
    uart_rx_command_cb = cb;
}

void uart_init( uint32_t baudrate ) { 

#ifdef ENABLE_DEBUG
    unsigned int i;
#endif
    
#ifdef ENABLE_DEBUG
    U2MODEbits.STSEL = 0;   // 1-stop bit
    U2MODEbits.PDSEL = 0;   // No Parity, 8-data bits
    U2MODEbits.ABAUD = 0;   // Autobaud Disabled
    U2MODEbits.BRGH = 0; // 16x mode (16 clock cycles per data bit)
    U2MODEbits.RTSMD = 0;    // DTE-DTE mode
    U2MODEbits.URXINV = 0;
    U2BRG = ( (FCY / baudrate) / 16 ) - 1;         // BAUD Rate Setting for 9600
    
    U2MODEbits.UEN   = 0b00; // do not use flow control
    
    //  Configure UART for DMA transfers
    U2STAbits.UTXISEL0 = 0; // Interrupt after one Tx character is transmitted
    U2STAbits.UTXISEL1 = 0;
    U2STAbits.URXISEL = 0;  // Interrupt after one RX character is received
    U2STAbits.TXINV = 0;
    
    //  Enable UART Rx and Tx
    U2MODEbits.UARTEN = 1;  // Enable UART
    U2STAbits.UTXEN = 1;    // Enable UART Tx
    
    // clear error bit, do not enable error interrupt
    _U2EIF = 0;
    _U2EIE = 0;
    
    // only enable RX interrupt
    _U2RXIE = 1;
    _U2TXIE = 0;
    _U2EIE = 1;
    
    // enable UART RX and TX
    U2MODEbits.UARTEN = 1;  // Enable UART
    U2STAbits.UTXEN = 1;    // Enable UART Tx
    
    // wait for at least one baud period to continue
    delay_us(2 * (FCY / baudrate));
    
    DMA14CONbits.DIR = 1; // RAM-to-Peripheral
    DMA14CONbits.SIZE = 1; // byte transfer mode
    DMA14CONbits.MODE = 0b01; // One-Shot, Ping-Pong modes disabled
    DMA14CNT = 0;              // number of  DMA requests
    DMA14REQ = 0x001F;         // Select UART2 transmitter
    DMA14PAD = (volatile unsigned int) &U2TXREG;
    _DMA14IF = 0;      // Clear DMA Interrupt Flag
    _DMA14IE = 1;      // Enable DMA interrupt
    
    DMA14STAL = (uint16_t) uart_dma_message_data;
    DMA14STAH = 0x0;
    
    // update interrupt priority 
    _DMA14IP = 7;
    
#ifdef __LOG__
    sprintf(print_buffer, "Initialised UART module.");
    uart_print(print_buffer, strlen(print_buffer));
#endif
    for(i = 0; i < UART_RX_BUFFER_SIZE; i++){
        uart_rx_queue[i].status = UART_MSG_NONE;
        uart_rx_queue[i].data = uart_rx_buffer[i];
    }
    
#endif
    
    uart_init_message(&uart_print_message, 
            SERIAL_TEXT_MESSAGE_CMD,
            0,
            0,            
            uart_print_message_data,
            0);
    
    uart_command_message.status = UART_MSG_SENT;
    uart_print_message.status = UART_MSG_SENT;
}


void __attribute__ ( (interrupt, no_auto_psv) ) _U2ErrInterrupt( void )
{
    U2STAbits.OERR = 0;
    _U2EIF = 0;                 // Clear the UART2 Error Interrupt Flag
}

void __attribute__ ( (interrupt, no_auto_psv) ) _U2RXInterrupt( void ) {
    
    register uint8_t rx_value = U2RXREG;
    register uart_message_t* m = &(uart_rx_queue[(n_uart_rx_messages+uart_rx_read_idx) % UART_RX_BUFFER_SIZE]);
    
    if((n_uart_rx_messages == UART_RX_BUFFER_SIZE) && m->status != UART_MSG_NONE){
        _U2RXIF = 0;
    }
    
    

    switch(uart_rx_state){
        case 0:
            if(rx_value==UART_CMD_START){
                uart_rx_state = 1;
            }
            break;
        case 1:
            m->command = rx_value;
            m->status = UART_MSG_TRANSFERRED;
            
            if(rx_value < N_SERIAL_CMD) {
                uart_rx_state = 2;
            } else {
                uart_rx_state = 0;
                m->status = UART_MSG_ERROR;
                n_uart_rx_messages++;
            }
            
            break;
        case 2:
            m->id = rx_value;
            uart_rx_state = 3;
            
            break;
        case 3:
            m->extended_id = rx_value;
            uart_rx_state = 4;
            
            break;
        case 4:
            m->extended_id = (m->extended_id << 8) + rx_value;
            uart_rx_state = 5;
            break;
        case 5:
            m->length = rx_value;
            if(rx_value > 0){
                uart_rx_state = 6;
                uart_rx_idx = 0;
            } else {
                uart_rx_state = 7;
            }
            
            break;
        case 6:
            // only write data until buffer is full
            if(uart_rx_idx < PRINT_BUFFER_LENGTH){
                m->data[uart_rx_idx] = rx_value;
            } else {
                // continue reading data, but indicate error occurred
                m->status = UART_MSG_ERROR;
            }
            
            uart_rx_idx++;
            
            if(uart_rx_idx == m->length){
                uart_rx_state = 7;
            }
            
            break;
        case 7:
            if(rx_value == UART_CMD_STOP){
                if(m->status != UART_MSG_ERROR){
                    m->status = UART_MSG_RECEIVED;
                }
            } else {
                m->status = UART_MSG_ERROR;
            }
            
            n_uart_rx_messages++;
            uart_rx_state = 0;
            
            uart_rx_command_cb();

            break;
        default:
            uart_rx_state = 0;
            break;
    }
    _U2RXIF = 0;
}

void __attribute__ ( (interrupt, no_auto_psv) ) _U2TXInterrupt( void ) {
    _U2TXIF = 0;
}



void uart_wait(){
#ifdef ENABLE_DEBUG
    while(uart_print_message.status != UART_MSG_SENT);
#endif
}

void uart_print(const char* message, size_t length){
#ifdef ENABLE_DEBUG
    
    uint16_t i;
    
    if(uart_print_message.status != UART_MSG_INIT_DONE){
        while(uart_print_message.status != UART_MSG_SENT);
    }
    
    uart_print_message.length = length;
    for(i = 0; i < length; i++){
        uart_print_message.data[i] = message[i];
    }
   
    uart_reset_message(&uart_print_message);
    uart_queue_message(&uart_print_message);
    
    //while(uart_print_message.status != UART_MSG_SENT);
#endif
}

void process_uart_queue(void){
    uart_message_t* m;
    
    if((uart_ongoing == 0) && (uart_queue_idx != uart_queue_valid) && (n_uart_messages > 0)){
        // copy to actual message to transmit
        
        uart_ongoing = 1;
        
        // start transmission of message
        m = uart_queue[uart_queue_valid];
        
        uart_parse_to_buffer(uart_dma_message_data, m, 
                ARRAY_LENGTH(uart_dma_message_data));
    
        while(U2STAbits.TRMT == 0);

        m->status = UART_MSG_TRANSFERRED;
        DMA14STAL = (uint16_t) uart_dma_message_data;
        DMA14STAH = 0x0;
        DMA14CNT = UART_HEADER_SIZE + (m->length) - 1;
        
        // start transfer 
        DMA14CONbits.CHEN = 1;
        DMA14REQbits.FORCE = 1;
    }
}

void uart_queue_message(uart_message_t* m){
    // wait for space in the queue
    while(n_uart_messages == UART_MESSAGE_BUFFER_LENGTH);
    
    if(m->status != UART_MSG_INIT_DONE){
        return;
    }
        
    
    // queue message
    m->status = UART_MSG_QUEUED;
    uart_queue[uart_queue_idx] = m;
    uart_queue_idx = (uart_queue_idx + 1) % UART_MESSAGE_BUFFER_LENGTH;
    n_uart_messages++;
    
    process_uart_queue();
}


void __attribute__((__interrupt__,no_auto_psv)) _DMA14Interrupt(void){
    // finish the current transfer
    uart_queue[uart_queue_valid]->status = UART_MSG_SENT;
    uart_queue_valid = (uart_queue_valid + 1) % UART_MESSAGE_BUFFER_LENGTH;
    n_uart_messages--;    
    
    // check if another message is available
    uart_ongoing = 0;
    process_uart_queue();
    
    _DMA14IF = 0;   // Clear the DMA0 Interrupt Flag
}

void uart_reset_message(uart_message_t* m){
    if((m->status != UART_MSG_QUEUED) && (m->status != UART_MSG_TRANSFERRED)){
        m->status = UART_MSG_INIT_DONE;
    }
}


void uart_parse_to_buffer(uint8_t* data, uart_message_t* m, const size_t max_length){
    uint16_t i;
    
    m->length = MIN(m->length, max_length-6-1);
    
    data[0] = UART_CMD_START;
    data[1] = (uint16_t) m->command;
    data[2] = m->id;
    data[3] = (uint8_t) (m->extended_id >> 8);
    data[4] = (uint8_t) m->extended_id;
    data[5] = (uint8_t) m->length;
    for(i = 0; i < m->length; i++){
        data[6+i] = m->data[i];
    }
    data[6+m->length] = UART_CMD_STOP;
}

void uart_init_message(uart_message_t* m, 
        serial_cmd_t command,
        uint8_t id,
        uint16_t extended_id,            
        uint8_t* data,
        uint16_t length){
    m->command = command;
    m->data = data;
    m->id = id;
    m->extended_id = extended_id;
    m->length = length;
    m->status = UART_MSG_INIT_DONE;
}

void uart_await_tx(uart_message_t* m){
    while(m->status != UART_MSG_SENT);
}
