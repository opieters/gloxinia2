#ifndef __SPI1_H__
#define __SPI1_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


typedef enum { 
    SDFAST,
    SDSLOW
} spi_master_configurations_t;

/**
*   \ingroup doc_driver_spi_code
*   \struct spi_master_functions_t spi_master.h
*/
typedef struct {    void (*spiClose)(void);
                    bool (*spiOpen)(void);
                    uint8_t (*exchangeByte)(uint8_t b);
                    void (*exchangeBlock)(void * block, size_t blockSize);
                    void (*writeBlock)(void * block, size_t blockSize);
                    void (*readBlock)(void * block, size_t blockSize);
                    void (*writeByte)(uint8_t byte);
                    uint8_t (*readByte)(void);
                    void (*setSpiISR)(void(*handler)(void));
                    void (*spiISR)(void);
} spi_master_functions_t;

extern const spi_master_functions_t spiMaster[];

bool spi_master_open(spi_master_configurations_t config);   //for backwards compatibility

/* SPI interfaces */
typedef enum { 
    SDFAST_CONFIG,
    SDSLOW_CONFIG,
    SPI1_DEFAULT
} spi1_modes;



/* arbitration interface */
void spi1_close(void);

bool spi1_open(spi1_modes spiUniqueConfiguration);
/* SPI native data exchange function */
uint8_t spi1_exchangeByte(uint8_t b);
/* SPI Block move functions }(future DMA support will be here) */
void spi1_exchangeBlock(void *block, size_t blockSize);
void spi1_writeBlock(void *block, size_t blockSize);
void spi1_readBlock(void *block, size_t blockSize);

void spi1_writeByte(uint8_t byte);
uint8_t spi1_readByte(void);

void spi1_isr(void);
void spi1_setSpiISR(void(*handler)(void));

#endif
