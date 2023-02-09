#include <xc.h>
#include <p33EP512MC806.h>
#include "uart1.h"
#include "boot/boot_config.h"
#include <uart.h>
#include <utilities.h>
#include "boot/boot_process.h"


uint8_t uart1_tx_buffer[10]; // TODO: check 10
uint16_t uart1_tx_mlength = 0;
uart1_message_t uart1_tx_m;
uint16_t uart_tx_data_idx = 0;
volatile uart_tx_state_t uart1_tx_state = UART_TX_STATE_DONE;

volatile uart_rx_state_t uart1_rx_state = UART_RX_STATE_FIND_START_BYTE;
uint16_t uart_rx_data_idx = 0;
uart1_message_t uart1_rx_m;


void UART1_Initialize(void)
{
/**    
     Set the UART1 module to the options selected in the user interface.
     Make sure to set LAT bit corresponding to TxPin as high before UART initialization
*/
    // STSEL 1; IREN disabled; PDSEL 8N; UARTEN enabled; RTSMD disabled; USIDL disabled; WAKE disabled; ABAUD disabled; LPBACK disabled; BRGH enabled; URXINV disabled; UEN TX_RX; 
    U1MODEbits.UARTEN = 0;  // disabling UARTEN bit
    U1MODEbits.BRGH = 1;  // BRGH enabled
    U1MODEbits.UEN = 0b10; // RTS and CTS enabled
    // UTXISEL0 TX_ONE_CHAR; UTXINV disabled; OERR NO_ERROR_cleared; URXISEL RX_ONE_CHAR; UTXBRK COMPLETED; UTXEN disabled; ADDEN disabled; 
    U1STA = 0x0;
    // BaudRate = 50000; Frequency = 64000000 Hz; BRG 319; 
    U1BRG = 0x13F;
    
    U1MODEbits.UARTEN = 1;  // enabling UARTEN bit
    U1STAbits.UTXEN = 1;  
    
    // enable interrupts
    _U1RXIF = 0;
    _U1RXIE = 1;
    _U1TXIF = 0;
    _U1TXIE = 1;
    _U1EIF = 0;
    _U1EIE = 1;
}

void UART1_Deactivate(void)
{
    // disable UART module
    U1MODEbits.UARTEN = 0;  // disabling UARTEN bit
    U1STAbits.UTXEN = 0;  
    
    // disable interrupts
    _U1RXIF = 0;
    _U1RXIE = 0;
    _U1TXIF = 0;
    _U1TXIE = 0;
    _U1EIF = 0;
    _U1EIE = 0;
}


void copy_uart1_message(uart1_message_t* m1, uart1_message_t* m2)
{
    m2->command = m1->command;
    m2->length = m1->length;
    m2->unlock = m1->unlock;
    
    for(unsigned int i = 0; i < m1->length; i++)
    {
        m2->data[i] = m1->data[i];
    }
}

void __attribute__((interrupt, no_auto_psv)) _U1ErrInterrupt(void)
{
    if(U1STAbits.OERR == 1)
        U1STAbits.OERR = 0;
    _U1EIF = 0; // Clear the UART2 Error Interrupt Flag
}

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void)
{
   register uint8_t rx_value = U1RXREG;

    switch (uart1_rx_state)
    {
    case UART_RX_STATE_FIND_START_BYTE:
        if (rx_value == UART_CMD_START)
        {
            uart1_rx_state = UART_RX_STATE_READ_IDH;
            uart1_rx_m.length = 0;
            uart1_rx_m.unlock = false;
        }
        break;
    case UART_RX_STATE_READ_IDH:
        if(rx_value != 0x00)
            uart1_rx_state = UART_CMD_START;
        else 
            uart1_rx_state = UART_RX_STATE_READ_IDL;
        break;
        if(rx_value != 0x00)
            uart1_rx_state = UART_CMD_START;
        else 
            uart1_rx_state = UART_RX_STATE_READ_CMD;
        break;
        break;
    case UART_RX_STATE_READ_CMD:
        uart1_rx_m.command = rx_value;
        uart1_rx_state = UART_RX_STATE_READ_REQUEST;
        break;
    case UART_RX_STATE_READ_REQUEST:
        if(rx_value != 0x00)
            uart1_rx_state = UART_CMD_START;
        else 
            uart1_rx_state = UART_RX_STATE_READ_SIDH;
        break;
    case UART_RX_STATE_READ_SIDH:
        if((uart1_rx_m.command == M_BOOT_WRITE_FLASH) || (uart1_rx_m.command == M_BOOT_ERASE_FLASH)){
            if(rx_value != 0x55){
                uart1_rx_state = UART_CMD_START;
            } else {
                uart1_rx_state = UART_RX_STATE_READ_SIDL;
            }
        } else {
            if(rx_value != 0x00){
                uart1_rx_state = UART_CMD_START;
            } else {
                uart1_rx_state = UART_RX_STATE_READ_SIDL;
            }
        }
        break;
    case UART_RX_STATE_READ_SIDL:
        if((uart1_rx_m.command == M_BOOT_WRITE_FLASH) || (uart1_rx_m.command == M_BOOT_ERASE_FLASH)){
            if(rx_value != 0xAA){
                uart1_rx_state = UART_CMD_START;
                uart1_rx_m.unlock = true;
            } else {
                uart1_rx_state = UART_RX_STATE_READ_LENGTH;
            }
        } else {
            if(rx_value != 0x00){
                uart1_rx_state = UART_CMD_START;
            } else {
                uart1_rx_state = UART_RX_STATE_READ_LENGTH;
            }
        }
        uart1_rx_state = UART_RX_STATE_READ_LENGTH;
        break;
    case UART_RX_STATE_READ_LENGTH:
        uart1_rx_m.length = rx_value;
        if (rx_value > 0)
        {
            uart1_rx_state = UART_RX_STATE_READ_DATA;
            uart_rx_data_idx = 0;
        }
        else
        {
            uart1_rx_state = UART_RX_STATE_DETECT_STOP;
        }

        break;
    case UART_RX_STATE_READ_DATA:
        // only write data until buffer is full
        if (uart_rx_data_idx < ARRAY_LENGTH(uart1_rx_m.data))
        {
            uart1_rx_m.data[uart_rx_data_idx] = rx_value;
        }

        uart_rx_data_idx++;

        if (uart_rx_data_idx == uart1_rx_m.length)
        {
            uart1_rx_state = UART_RX_STATE_DETECT_STOP;
        }

        break;
    case UART_RX_STATE_DETECT_STOP:
        if (rx_value == UART_CMD_STOP)
        {
            BOOT_ProcessCommand();
        }

        uart1_rx_state = UART_RX_STATE_FIND_START_BYTE;
        

        break;
    default:
        uart1_rx_state = UART_RX_STATE_FIND_START_BYTE;
        break;
    }
    _U1RXIF = 0;
}

void uart1_tx_message(uart1_message_t* m){
    // wait until previous message sent
    uart1_wait_tx();

    uart1_tx_m.command = m->command;
    uart1_tx_m.length = m->length;
    
    // copy data
    for(uint16_t i = 0; i <m->length; i++)
    {
        uart1_tx_m.data[i] = m->data[i];
    }

    // start first transmission
    U1TXREG = UART_CMD_START;
    uart1_tx_state = UART_TX_STATE_SEND_IDH;
}


void uart1_wait_tx(void){
    // wait until previous message sent
    while(uart1_tx_state != UART_TX_STATE_DONE);
}

void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void)
{
    _U1TXIF = 0;
    
    switch(uart1_tx_state)
    {
        case UART_TX_STATE_SEND_START_BYTE:
            break;
        case UART_TX_STATE_SEND_IDH:
            U1TXREG = 0x00;
            uart1_tx_state = UART_TX_STATE_SEND_IDL;
            break;
        case UART_TX_STATE_SEND_IDL:
            U1TXREG = 0x00;
            uart1_tx_state = UART_TX_STATE_SEND_CMD;
            break;
        case  UART_TX_STATE_SEND_CMD:
            U1TXREG = uart1_tx_m.command;
            uart1_tx_state = UART_TX_STATE_SEND_REQUEST;
            break;
        case  UART_TX_STATE_SEND_REQUEST:
            U1TXREG = 0x00;
            uart1_tx_state = UART_TX_STATE_SEND_SIDH;
            break;
        case  UART_TX_STATE_SEND_SIDH:
            if(uart1_tx_m.unlock)
                U1TXREG = 0x55;
            else
                U1TXREG = 0x00;
            uart1_tx_state = UART_TX_STATE_SEND_SIDL;
            break;
        case  UART_TX_STATE_SEND_SIDL:
            if(uart1_tx_m.unlock)
                U1TXREG = 0xaa;
            else
                U1TXREG = 0x00;
            uart1_tx_state = UART_TX_STATE_SEND_LENGTH;
            break;
        case UART_TX_STATE_SEND_LENGTH:
            U1TXREG = uart1_tx_m.length;
            if(uart1_tx_m.length > 0)
                uart1_tx_state = UART_TX_STATE_SEND_DATA;
            else
                uart1_tx_state = UART_TX_STATE_SEND_STOP;
            uart_tx_data_idx = 0;
            break;
        case  UART_TX_STATE_SEND_DATA:
            U1TXREG = uart1_tx_m.data[uart_tx_data_idx];
            uart_tx_data_idx++;
            if(uart_tx_data_idx == uart1_tx_m.length)
            {  
                uart1_tx_state = UART_TX_STATE_SEND_STOP;
            } 
            break;
        case UART_TX_STATE_SEND_STOP:
            U1TXREG = UART_CMD_STOP;
            uart1_tx_state = UART_TX_STATE_DONE;
            // all data sent!
            break;
        case  UART_TX_STATE_DONE:
            break;
        default:
            break;
    }
}

