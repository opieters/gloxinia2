#ifndef __APP_IMAGE_H__
#define __APP_IMAGE_H__

#include <xc.h>
#include <stdbool.h>
#include "bootloader.h"


typedef enum nvm_return_status_s {
    NVM_SUCCESS,
    NVM_INVALID_LENGTH,
    NVM_INVALID_ADDRESS,
    NVM_WRITE_ERROR,
    NVM_READ_ERROR,
}nvm_return_status_t; 

#define FLASH_ERASE_MASK (~((FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS*2UL) - 1)) 

typedef struct app_image_s
{
    uint32_t start_address;
} app_image_t;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize the bootloader application (not the image)
     *
     * @details This function sets the default values of the bootloader. It is
     * not used to initialise the system, see system_initialise for that.
     */
    void bootloader_init(void);

    /**
     * @brief Run the bootloader application
     */
    void bootloader_run(void);
 
void bootloader_start_app_image();
bool app_image_is_legal_address(uint32_t addressToCheck);


bool app_image_is_legal_range(uint32_t startRangeToCheck, uint32_t endRangeToCheck);


nvm_return_status_t app_image_write_flash_row(uint32_t deviceAddress, uint32_t lengthInBytes, uint8_t *sourceData, uint32_t key);
nvm_return_status_t BOOT_BlockRead (uint8_t *destinationData, uint32_t lengthInBytes, uint32_t nvmAddress);

nvm_return_status_t BOOT_BlockErase (uint32_t nvmAddress, uint32_t lengthInPages, uint32_t key);
uint32_t app_image_address_get(enum boot_image_enum image, uint32_t address_in_executable_image);

uint16_t BOOT_EraseSizeGet();
nvm_return_status_t BOOT_Read32Data (uint32_t *destinationData,  uint32_t nvmAddress);
nvm_return_status_t BOOT_Read16Data (uint16_t *destinationData,  uint32_t nvmAddress);
nvm_return_status_t BOOT_BlockWrite(uint32_t deviceAddress, uint32_t lengthInBytes, uint8_t *sourceData, uint32_t key);
nvm_return_status_t BOOT_BlockRead (uint8_t *destinationData, uint32_t lengthInBytes, uint32_t nvmAddress);
nvm_return_status_t BOOT_BlockErase (uint32_t nvmAddress, uint32_t lengthInPages, uint32_t key);

#ifdef __cplusplus
}
#endif

#endif
