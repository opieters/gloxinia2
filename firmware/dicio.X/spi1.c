#include <xc.h>
#include "spi1.h"

void (*spi1_interruptHandler)(void); 

void spi1_close(void)
{
    SPI1STATbits.SPIEN = 0;
}

//con1 == SPIxCON1, con2 == SPIxCON2, stat == SPIxSTAT, operation == Master/Slave
typedef struct { uint16_t con1; uint16_t con2; uint16_t stat; uint8_t operation;} spi1_configuration_t;
static const spi1_configuration_t spi1_configuration[] = {   
    { 0x007D, 0x0000, 0x0000, 0 },
    { 0x0060, 0x0000, 0x0000, 0 },
    { 0x0126, 0x0000, 0x0000, 0 }
};

bool spi1_open(spi1_modes spiUniqueConfiguration)
{
    if(!SPI1STATbits.SPIEN)
    {
        SPI1CON1 = spi1_configuration[spiUniqueConfiguration].con1;
        SPI1CON2 = spi1_configuration[spiUniqueConfiguration].con2;
        SPI1STAT = spi1_configuration[spiUniqueConfiguration].stat | 0x8000;
        
        _SPI1IE = 1;
        
        
        //TODO: fix
        //TRISDbits.TRISD1 = spi1_configuration[spiUniqueConfiguration].operation;
        TRISBbits.TRISB10 = spi1_configuration[spiUniqueConfiguration].operation;
        return true;
    }
    return false;
}

// Full Duplex SPI Functions
uint8_t spi1_exchangeByte(uint8_t b)
{
    SPI1BUF = b;
    while(!SPI1STATbits.SPIRBF);
    return SPI1BUF;
}

void spi1_exchangeBlock(void *block, size_t blockSize)
{
    uint8_t *data = block;
    while(blockSize--)
    {
        *data = spi1_exchangeByte(*data );
        data++;
    }
}

// Half Duplex SPI Functions
void spi1_writeBlock(void *block, size_t blockSize)
{
    uint8_t *data = block;
    while(blockSize--)
    {
        spi1_exchangeByte(*data++);
    }
}

void spi1_readBlock(void *block, size_t blockSize)
{
    uint8_t *data = block;
    while(blockSize--)
    {
        *data++ = spi1_exchangeByte(0);
    }
}

void spi1_writeByte(uint8_t byte)
{
    SPI1BUF = byte;
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
    if(IFS0bits.SPI1IF == 1){
        if(spi1_interruptHandler){
            spi1_interruptHandler();
        }
        IFS0bits.SPI1IF = 0;
    }
}

void spi1_setSpiISR(void(*handler)(void))
{
    spi1_interruptHandler = handler;
}

/**
\file
\addtogroup doc_driver_spi_code
\brief This file contains the functions that implement the SPI master driver functionalities.

\copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
\page License
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
 */



bool SDFAST_open(void);
bool SDSLOW_open(void);

const spi_master_functions_t spiMaster[] = {   
    { spi1_close, SDFAST_open, spi1_exchangeByte, spi1_exchangeBlock, spi1_writeBlock, spi1_readBlock, spi1_writeByte, spi1_readByte, spi1_setSpiISR, _SPI1Interrupt },
    { spi1_close, SDSLOW_open, spi1_exchangeByte, spi1_exchangeBlock, spi1_writeBlock, spi1_readBlock, spi1_writeByte, spi1_readByte, spi1_setSpiISR, _SPI1Interrupt }
};

bool SDFAST_open(void){
    return spi1_open(SDFAST_CONFIG);
}

bool SDSLOW_open(void){
    return spi1_open(SDSLOW_CONFIG);
}

/**
 *  \ingroup doc_driver_spi_code
 *  \brief Open the SPI interface.
 *
 *  This function is to keep the backward compatibility with older API users
 *  \param[in] configuration The configuration to use in the transfer
 *
 *  \return Initialization status.
 *  \retval false The SPI open was unsuccessful
 *  \retval true  The SPI open was successful
 */
bool spi_master_open(spi_master_configurations_t config){
    switch(config){
        case SDFAST:
            return SDFAST_open();
        case SDSLOW:
            return SDSLOW_open();
        default:
            return 0;
    }
}

/**
 End of File
 */
