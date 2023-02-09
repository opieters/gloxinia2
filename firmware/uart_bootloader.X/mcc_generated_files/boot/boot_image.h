#ifndef BOOT_IMAGE_H
#define BOOT_IMAGE_H

#include <stdbool.h>
#include <stdint.h>

#define EXECUTABLE_IMAGE_FIRST_ADDRESS BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW
#define EXECUTABLE_IMAGE_LAST_ADDRESS ((unsigned long)BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW + (unsigned long)BOOT_IMAGE_SIZE - (unsigned long)2u)

#define DOWNLOAD_IMAGE_NUMBER 0u
#define BOOT_IMAGE_COUNT 1
#define BOOT_IMAGE_SIZE 0x53000

enum BOOT_IMAGE{
    BOOT_IMAGE_0 = 0,
};

typedef enum NVM_RETURN_STATUS {
    NVM_SUCCESS,
    NVM_INVALID_LENGTH,
    NVM_INVALID_ADDRESS,
    NVM_WRITE_ERROR,
    NVM_READ_ERROR,
}NVM_RETURN_STATUS; 


bool IsLegalRange(uint32_t startRangeToCheck, uint32_t endRangeToCheck);
bool BOOT_ImageErase(enum BOOT_IMAGE image);

bool BOOT_ImageVerify(enum BOOT_IMAGE image);
uint32_t BOOT_ImageAddressGet(enum BOOT_IMAGE image, uint32_t addressInExecutableImage);
uint16_t BOOT_EraseSizeGet();
NVM_RETURN_STATUS BOOT_BlockWrite(uint32_t nvmAddress, uint32_t lengthInBytes, uint8_t *sourceData, uint32_t key);
NVM_RETURN_STATUS BOOT_BlockRead (uint8_t *destinationData, uint32_t lengthInBytes, uint32_t nvmAddress);
NVM_RETURN_STATUS BOOT_BlockErase (uint32_t nvmAddress, uint32_t lengthInBytes, uint32_t key);

// Functions to read RAM data stored in flash in 16 bit values in each instruction location.
NVM_RETURN_STATUS BOOT_Read32Data (uint32_t *destinationData,  uint32_t nvmAddress);
NVM_RETURN_STATUS BOOT_Read16Data (uint16_t *destinationData,  uint32_t nvmAddress);
#endif