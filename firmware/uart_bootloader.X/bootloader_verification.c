#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "flash_memory.h"
#include "bootloader_config.h"
#include <stdio.h>
#include "uart_bootloader.h"
#include "app_image.h"

#define MEMCPY_BUFFERS_MATCH 0

extern uint32_t CRC32Bit(uint32_t crc, uint32_t input);
extern uint32_t CRCFlash(uint32_t crcSeed, uint32_t startAddress, uint32_t endAddress);

struct BOOT_VERIFY_APPLICATION_HEADER
{
    uint32_t crc32;
    uint32_t startAddress;
    uint32_t endAddress;
};

static const uint32_t applicationHeaderAddress = BOOT_CONFIG_APPLICATION_IMAGE_APPLICATION_HEADER_ADDRESS;

static inline bool isOdd(uint32_t number)
{
    return ((number & 0x00000001) == 0x00000001);
}

static void ApplicationHeaderRead(uint32_t sourceAddress, struct BOOT_VERIFY_APPLICATION_HEADER *applicationHeader)
{
    BOOT_Read32Data (&applicationHeader->crc32, sourceAddress);
    BOOT_Read32Data (&applicationHeader->startAddress, sourceAddress + 4);
    //applicationHeader->endAddress = applicationHeader->startAddress + 2;
    BOOT_Read32Data (&applicationHeader->endAddress,   sourceAddress + 8);
}

bool app_image_verification(boot_image_t image, uint32_t* checksum)
{   
    struct BOOT_VERIFY_APPLICATION_HEADER applicationHeader;
    uint32_t c;
   
   
    if( image >= BOOT_IMAGE_COUNT )
    {
        return false;
    }

    ApplicationHeaderRead(app_image_address_get(image, applicationHeaderAddress), &applicationHeader);

    if( isOdd(applicationHeader.startAddress) )
    {
        return false;
    }
    
    if( isOdd(applicationHeader.endAddress) )
    {
        return false;
    }

    if( applicationHeader.startAddress < BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW )
    {
        return false;
    }

    if( applicationHeader.endAddress < BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW )
    {
        return false;
    }

    if( applicationHeader.startAddress > BOOT_CONFIG_PROGRAMMABLE_ADDRESS_HIGH )
    {
        return false;
    }

    if( applicationHeader.endAddress > BOOT_CONFIG_PROGRAMMABLE_ADDRESS_HIGH )
    {
        return false;
    }
    
    if( applicationHeader.startAddress > applicationHeader.endAddress)
    {
        return false;
    }
    
    // get address range for checksum
    uint32_t address, instruction;
    
    c = 0;
    
    // read all data blocks from memory and process
    for(address = applicationHeader.startAddress; address <= applicationHeader.endAddress; address +=2)
    {
        instruction = FLASH_ReadWord24(address);
        c = c + instruction;
    }
    
    *checksum = c;
    
    return ( c  == applicationHeader.crc32 );
}

bool bootloader_verify_application_image(uint32_t* checksum)
{
    return app_image_verification(0, checksum);
}
