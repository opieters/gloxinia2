#ifndef __SPI1_H__
#define __SPI1_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


bool spi1_open(void);
void spi1_close(void);

uint8_t spi1_exchange_byte(uint8_t data);

void spi1_exchange_block(uint8_t* block, size_t block_size);
void spi1_write_block(uint8_t* block, size_t block_size);
void spi1_read_block(uint8_t* block, size_t block_size);

void spi1_write_byte(uint8_t data);
uint8_t spi1_readByte(void);

#endif
