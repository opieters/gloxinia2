#ifndef COM_ADAPTOR_H
#define COM_ADAPTOR_H

#include <stdbool.h>
#include <stdint.h>

uint16_t BOOT_COM_Read(uint8_t* readBuffer, uint16_t length);
uint16_t BOOT_COM_GetBytesReady();
uint8_t BOOT_COM_Peek(uint16_t location);
void BOOT_COM_Write(uint8_t* data, uint16_t length);

#endif 
