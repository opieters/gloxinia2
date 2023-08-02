#include <xc.h>
#include "spi1.h"

void spi1_close(void)
{
    SPI1STATbits.SPIEN = 0;
}

bool spi1_open(void)
{
    if(!SPI1STATbits.SPIEN)
    {
        SPI1CON1 = 0x0060;
        SPI1CON2 = 0x0000;
        SPI1STAT = 0x0000 | 0x8000;
        
        //_SPI1IE = 1;
        
        
        //TODO: fix
        //TRISDbits.TRISD1 = spi1_configuration[spiUniqueConfiguration].operation;
        //TRISDbits.TRISD11 = spi1_configuration[spiUniqueConfiguration].operation;
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
