#include <xc.h>
#include "spi1.h"

void spi1_close(void)
{
    SPI1STATbits.SPIEN = 0;
}

bool spi1_open(void)
{
    // the SPI interface is configured to 125kHz, which is below the upper limit
    // for this interface (9MHz, see table 32-32 p526)
    if(!SPI1STATbits.SPIEN)
    {
        SPI1CON1bits.MSTEN = 1;     // enable master mode
        SPI1CON1bits.CKP = 1;       // high level is idle state
        SPI1CON1bits.SPRE = 0b000;  // set secondary prescaler to 1:8
        SPI1CON1bits.PPRE = 0b11;   // set primary prescaler to 1:64
        
        SPI1CON2 = 0x0000;
        SPI1STATbits.SPIEN = 1;    // enable SPI interface
        
        //_SPI1IE = 1;
        return true;
    }
    return false;
}


// Full Duplex SPI Functions
uint8_t spi1_exchange_byte(uint8_t b)
{
    SPI1BUF = b;
    while(!SPI1STATbits.SPIRBF);
    return SPI1BUF;
}

void spi1_exchange_block(uint8_t *block, size_t block_size)
{
    uint8_t *data = block;
    while(block_size--)
    {
        *data = spi1_exchange_byte(*data );
        data++;
    }
}

// Half Duplex SPI Functions
void spi1_write_block(uint8_t *block, size_t block_size)
{
    uint8_t *data = block;
    while(block_size--)
    {
        spi1_exchange_byte(*data++);
    }
}

void spi1_read_block(uint8_t *block, size_t block_size)
{
    uint8_t *data = block;
    while(block_size--)
    {
        *data++ = spi1_exchange_byte(0);
    }
}

void spi1_write_byte(uint8_t data)
{
    SPI1BUF = data;
}

uint8_t spi1_readByte(void)
{
    return SPI1BUF;
}

/**
 * Interrupt from SPI on bit 8 received and SR moved to buffer
 * If interrupts are not being used, then call this method from the main while(1) loop
 */
void __attribute__((interrupt, no_auto_psv)) _SPI1Interrupt(void)
{
    _SPI1IF = 0;
}
