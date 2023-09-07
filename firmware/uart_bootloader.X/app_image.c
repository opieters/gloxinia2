#include <string.h>
#include "app_image.h"
#include "bootloader_config.h"
#include "bootloader.h"
#include "flash_memory.h"

const static app_image_t images[] = 
{
    {
        .start_address = 0x2800
    },
};

void bootloader_start_app_image()
{
    int (*user_application)(void);
    user_application = (int(*)(void))BOOT_CONFIG_APPLICATION_RESET_ADDRESS;
    user_application();       
}  

bool app_image_is_legal_address(uint32_t addressToCheck)
{
   return ( (addressToCheck >= EXECUTABLE_IMAGE_FIRST_ADDRESS) && (addressToCheck <= EXECUTABLE_IMAGE_LAST_ADDRESS) );
}


bool app_image_is_legal_range(uint32_t startRangeToCheck, uint32_t endRangeToCheck)
{
    return ( app_image_is_legal_address(startRangeToCheck) && app_image_is_legal_address(endRangeToCheck - 2u) );
}


nvm_return_status_t app_image_write_flash_row(uint32_t deviceAddress, uint32_t lengthInBytes, uint8_t *sourceData, uint32_t key)
{
    uint32_t count = 0;
    nvm_return_status_t response = NVM_SUCCESS;
    
    if ((lengthInBytes % MINIMUM_WRITE_BLOCK_SIZE) == 0u)
    {
        if ( app_image_is_legal_range(deviceAddress, deviceAddress + (lengthInBytes/2u)) ) 
        {

        FLASH_Unlock(key);
 

        for (count = 0; count < lengthInBytes; count += MINIMUM_WRITE_BLOCK_SIZE)
        {
            uint16_t flashData[MINIMUM_WRITE_BLOCK_SIZE/sizeof(uint16_t)];
            uint32_t physicalWriteAddress = app_image_address_get(DOWNLOAD_IMAGE_NUMBER, deviceAddress + (count/2u));

            memcpy(&flashData[0], &sourceData[count], MINIMUM_WRITE_BLOCK_SIZE);

            if (FLASH_WriteRow24(physicalWriteAddress, flashData) == false)
            {
                response = NVM_WRITE_ERROR;
                break;
            }
        }

 
        FLASH_Lock();
        }
        else
        {
            response = NVM_INVALID_ADDRESS;
        }
    }
    else
    {
        response = NVM_INVALID_LENGTH;
    }
    
    return response;
}

nvm_return_status_t BOOT_BlockRead (uint8_t *destinationData, uint32_t lengthInBytes, uint32_t nvmAddress)
{
    uint32_t count;
    uint32_t flashData;
    nvm_return_status_t response = NVM_SUCCESS;
    
    if ((lengthInBytes % 4u) == 0u)
    {
        if (app_image_is_legal_range(nvmAddress, nvmAddress + (lengthInBytes/2u))) 
        {
            for (count = 0u; count < lengthInBytes; count += 4u)
                {
                    uint32_t physicalReadAddress = app_image_address_get(DOWNLOAD_IMAGE_NUMBER, nvmAddress + (count/2u));
                    flashData = FLASH_ReadWord24(physicalReadAddress);
                    memcpy(&destinationData[count], &flashData, 4u);
                }
        }
        else
        {
            response = NVM_INVALID_ADDRESS;
        }
    } else
    {
        response = NVM_INVALID_LENGTH;
    }
 
    return response;
}


nvm_return_status_t BOOT_BlockErase (uint32_t nvmAddress, uint32_t lengthInPages, uint32_t key)
{
    nvm_return_status_t response = NVM_SUCCESS;
    uint32_t eraseAddress = nvmAddress;
    bool goodErase = true;
    
    // check to make sure page is aligned here.
    if ( (eraseAddress & FLASH_ERASE_MASK) != eraseAddress)
    {
        goodErase = false;
    }
    
    FLASH_Unlock(key);

    #define ERASE_SIZE_REQUESTED ((uint32_t)(lengthInPages) * FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS)

    while (goodErase && (eraseAddress < (nvmAddress +  ERASE_SIZE_REQUESTED ) ))
    {
        if (app_image_is_legal_range(eraseAddress, eraseAddress+FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS))  
        {
            uint32_t physicalEraseAddress = app_image_address_get(DOWNLOAD_IMAGE_NUMBER, eraseAddress);
            goodErase = (uint8_t) FLASH_ErasePage(physicalEraseAddress);

            eraseAddress += FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS;
        }
        else
        {
            goodErase = false;
        }
    }
    
    FLASH_Lock();

    if ((!goodErase) || (eraseAddress != (nvmAddress + ERASE_SIZE_REQUESTED)))
    {
        response = NVM_INVALID_ADDRESS;
    } 
    
    return response;
}

uint32_t app_image_address_get(enum boot_image_enum image, uint32_t address_in_executable_image)
{
    uint32_t offset = address_in_executable_image - images[BOOT_IMAGE_0].start_address;
    return images[image].start_address + offset;
}

uint16_t BOOT_EraseSizeGet()
{
    return (FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS * 2u);
}

nvm_return_status_t BOOT_Read32Data (uint32_t *destinationData,  uint32_t nvmAddress)
{
    *destinationData = FLASH_ReadWord16(nvmAddress);
    *destinationData += ((uint32_t)FLASH_ReadWord16(nvmAddress + 2)) << 16;
    
    return NVM_SUCCESS;
}

nvm_return_status_t BOOT_Read16Data (uint16_t *destinationData,  uint32_t nvmAddress)
{
    *destinationData = FLASH_ReadWord16(nvmAddress);

    return NVM_SUCCESS;
}


nvm_return_status_t BOOT_BlockWrite(uint32_t deviceAddress, uint32_t lengthInBytes, uint8_t *sourceData, uint32_t key)
{
    uint32_t count = 0;
    nvm_return_status_t response = NVM_SUCCESS;
    
    if(deviceAddress == 0x2808){
        Nop();
        Nop();
        Nop();
    }
    
    if ((lengthInBytes % MINIMUM_WRITE_BLOCK_SIZE) == 0u)
    {
        if ( app_image_is_legal_range(deviceAddress, deviceAddress + (lengthInBytes/2u)) ) 
        {

        FLASH_Unlock(key);
 

        for (count = 0; count < lengthInBytes; count += MINIMUM_WRITE_BLOCK_SIZE)
        {
            uint32_t flashData[MINIMUM_WRITE_BLOCK_SIZE/sizeof(uint32_t)];
            uint32_t physicalWriteAddress = app_image_address_get(DOWNLOAD_IMAGE_NUMBER, deviceAddress + (count/2u));

            memcpy(&flashData[0], &sourceData[count], MINIMUM_WRITE_BLOCK_SIZE);

            if (FLASH_WriteDoubleWord24(physicalWriteAddress, flashData[0],flashData[1] ) == false)
            {
                response = NVM_WRITE_ERROR;
                break;
            }
        }

 
        FLASH_Lock();
        }
        else
        {
            response = NVM_INVALID_ADDRESS;
        }
    }
    else
    {
        response = NVM_INVALID_LENGTH;
    }
    
    return response;
}
