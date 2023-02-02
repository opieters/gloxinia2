

/**
  @Generated 16-bit Bootloader Source File

  @Company:
    Microchip Technology Inc.

  @File Name: 
    boot_image.c

  @Summary:
    This is the boot_image.c file generated using 16-bit Bootloader

  @Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  16-bit Bootloader - 1.22.1
        Device            :  dsPIC33EP512MC806
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.36B
        MPLAB             :  MPLAB X v5.15
*/

/*
Copyright (c) [2012-2023] Microchip Technology Inc.  

    All rights reserved.

    You are permitted to use the accompanying software and its derivatives 
    with Microchip products. See the Microchip license agreement accompanying 
    this software, if any, for additional info regarding your rights and 
    obligations.
    
    MICROCHIP SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT 
    WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT 
    LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT 
    AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP OR ITS
    LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT 
    LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE 
    THEORY FOR ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT 
    LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES, 
    OR OTHER SIMILAR COSTS. 
    
    To the fullest extend allowed by law, Microchip and its licensors 
    liability will not exceed the amount of fees, if any, that you paid 
    directly to Microchip to use this software. 
    
    THIRD PARTY SOFTWARE:  Notwithstanding anything to the contrary, any 
    third party software accompanying this software is subject to the terms 
    and conditions of the third party's license agreement.  To the extent 
    required by third party licenses covering such third party software, 
    the terms of such license will apply in lieu of the terms provided in 
    this notice or applicable license.  To the extent the terms of such 
    third party licenses prohibit any of the restrictions described here, 
    such restrictions will not apply to such third party software.
*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../memory/flash.h"
#include "boot_private.h"
#include "boot_image.h"
#include "boot_config.h"


struct IMAGE {
    uint32_t startAddress;
};

const static struct IMAGE images[] = 
{
    {
        .startAddress = 0x2800
    },
};

#define FLASH_ERASE_MASK (~((FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS*2UL) - 1)) 

static bool IsLegalAddress(uint32_t addressToCheck)
{
   return ( (addressToCheck >= EXECUTABLE_IMAGE_FIRST_ADDRESS) && (addressToCheck <= EXECUTABLE_IMAGE_LAST_ADDRESS) );
}


bool IsLegalRange(uint32_t startRangeToCheck, uint32_t endRangeToCheck)
{
    return ( IsLegalAddress(startRangeToCheck) && IsLegalAddress(endRangeToCheck - 2u) );
}


NVM_RETURN_STATUS BOOT_BlockWrite(uint32_t deviceAddress, uint32_t lengthInBytes, uint8_t *sourceData, uint32_t key)
{
    uint32_t count = 0;
    enum NVM_RETURN_STATUS response = NVM_SUCCESS;
    
    if ((lengthInBytes % MINIMUM_WRITE_BLOCK_SIZE) == 0u)
    {
        if ( IsLegalRange(deviceAddress, deviceAddress + (lengthInBytes/2u)) ) 
        {

        FLASH_Unlock(key);
 

        for (count = 0; count < lengthInBytes; count += MINIMUM_WRITE_BLOCK_SIZE)
        {
            uint32_t flashData[MINIMUM_WRITE_BLOCK_SIZE/sizeof(uint32_t)];
            uint32_t physicalWriteAddress = BOOT_ImageAddressGet(DOWNLOAD_IMAGE_NUMBER, deviceAddress + (count/2u));

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

NVM_RETURN_STATUS BOOT_BlockRead (uint8_t *destinationData, uint32_t lengthInBytes, uint32_t nvmAddress)
{
    uint32_t count;
    uint32_t flashData;
    enum NVM_RETURN_STATUS response = NVM_SUCCESS;
    
    if ((lengthInBytes % 4u) == 0u)
    {
        if (IsLegalRange(nvmAddress, nvmAddress + (lengthInBytes/2u))) 
        {
            for (count = 0u; count < lengthInBytes; count += 4u)
                {
                    uint32_t physicalReadAddress = BOOT_ImageAddressGet(DOWNLOAD_IMAGE_NUMBER, nvmAddress + (count/2u));
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


NVM_RETURN_STATUS BOOT_BlockErase (uint32_t nvmAddress, uint32_t lengthInPages, uint32_t key)
{
    enum NVM_RETURN_STATUS response = NVM_SUCCESS;
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
        if (IsLegalRange(eraseAddress, eraseAddress+FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS))  
        {
            uint32_t physicalEraseAddress = BOOT_ImageAddressGet(DOWNLOAD_IMAGE_NUMBER, eraseAddress);
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

uint32_t BOOT_ImageAddressGet(enum BOOT_IMAGE image, uint32_t addressInExecutableImage)
{
    uint32_t offset = addressInExecutableImage - images[BOOT_IMAGE_0].startAddress;
    return images[image].startAddress + offset;
}

uint16_t BOOT_EraseSizeGet()
{
    return (FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS * 2u);
}

NVM_RETURN_STATUS BOOT_Read32Data (uint32_t *destinationData,  uint32_t nvmAddress)
{
    *destinationData = FLASH_ReadWord16(nvmAddress);
    *destinationData += ((uint32_t)FLASH_ReadWord16(nvmAddress + 2)) << 16;
    
    return NVM_SUCCESS;
}

NVM_RETURN_STATUS BOOT_Read16Data (uint16_t *destinationData,  uint32_t nvmAddress)
{
    *destinationData = FLASH_ReadWord16(nvmAddress);

    return NVM_SUCCESS;
}


        
   
   

