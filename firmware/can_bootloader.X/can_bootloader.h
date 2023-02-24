#ifndef __CAN_BOOTLOADER_H__
#define __CAN_BOOTLOADER_H__

#include <xc.h>
#include "bootloader_config.h"
#include "can2.h"

#define DOWNLOAD_IMAGE_NUMBER 0u
#define BOOT_IMAGE_COUNT 1
#define BOOT_IMAGE_SIZE 0x53000

#define DOWNLOADED_IMAGE 0u
#define EXECUTION_IMAGE 0u

typedef enum boot_image_enum
{
    BOOT_IMAGE_0 = 0,
} boot_image_t;

typedef enum nvm_return_status_s
{
    NVM_SUCCESS,
    NVM_INVALID_LENGTH,
    NVM_INVALID_ADDRESS,
    NVM_WRITE_ERROR,
    NVM_READ_ERROR,
} nvm_return_status_t;

/**
 * @brief CORCON initialization type enumerator.
 */
typedef enum
{
    ///< @brief Use POR values of CORCON
    CORCON_MODE_PORVALUES = 0x0020,

    /**<
     * @brief Enable saturation for ACCA, ACCB and Dataspace write, enable
     * normal ACCA/ACCB saturation mode and set rounding to Biased
     * (conventional) mode. Rest of CORCON settings are set to the default POR
     * values.
     */
    CORCON_MODE_ENABLEALLSATNORMAL_ROUNDBIASED = 0x00E2,
    /**<
     *  @brief Enable saturation for ACCA, ACCB and Dataspace write, enable normal
     *  ACCA/ACCB saturation mode and set rounding to Unbiased (convergent)
     *  mode. Rest of CORCON settings are set to the default POR values.
     */
    CORCON_MODE_ENABLEALLSATNORMAL_ROUNDUNBIASED = 0x00E0,
    /**<  @brief Disable saturation for ACCA, ACCB
     *  and Dataspace write and set
     *  rounding to Biased (conventional)
     *  mode. Rest of CORCON settings are
     *  set to the default POR values.
     *  */
    CORCON_MODE_DISABLEALLSAT_ROUNDBIASED = 0x0022,
    /**<  @brief Disable saturation for ACCA, ACCB and Dataspace write and set
     *  rounding to Unbiased (convergent) mode. Rest of CORCON settings are
     *  set to the default POR values.
     */
    CORCON_MODE_DISABLEALLSAT_ROUNDUNBIASED = 0x0020,
    /**<  @brief Enable saturation for ACCA, ACCB and Dataspace write, enable super
     *  ACCA/ACCB saturation mode and set rounding to Biased (conventional)
     *  mode. Rest of CORCON settings are set to the default POR values.
     *  */
    CORCON_MODE_ENABLEALLSATSUPER_ROUNDBIASED = 0x00F2,
    /**<  @brief Enable saturation for ACCA, ACCB and Dataspace write, enable super
     *  ACCA/ACCB saturation mode and set rounding to Unbiased (convergent)
     *  mode. Rest of CORCON settings are set to the default POR values.
     *  */
    CORCON_MODE_ENABLEALLSATSUPER_ROUNDUNBIASED = 0x00F0,
} system_corcon_mode_t;


/// @brief Bootloader command response enumeration.
typedef enum
{
    COMMAND_SUCCESS = 0x01,     ///< @brief Command was successful.
    UNSUPPORTED_COMMAND = 0xFF, ///< @brief Command is not supported.
    BAD_ADDRESS = 0xFE,         ///< @brief Address is out of range.
    BAD_LENGTH = 0xFD,          ///< @brief Length is out of range.
    VERIFY_FAIL = 0xFC,         ///< @brief Verify failed.
            BOOT_CMD_ERROR = 0xF0, ///< @brief General error code.
    FLASH_INIT_FAIL = 0xFB,            ///< @brief Flash initialization failed.
} boot_command_response_t;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Verify that the start and end address are in the legal range for the image.
     * 
     * @param start_address The start address to check.
     * @param end_address The end address to check.
     * 
     * @return true if the address range is legal, false otherwise.
    */
    bool app_image_is_legal_range(uint32_t start_address, uint32_t end_address);

    /**
     * @brief Erase the application image.
     * 
     * @param image The image to erase.
     * @return true if the erase was successful, false otherwise.
    */
    bool app_image_erase(enum boot_image_enum image);

    /**
     * @brief Verify the application image.
     * 
     * @param image The image to verify.
     * @param checksum The checksum of the image.
     * @return true if the image is valid, false otherwise.
    */
    bool app_image_verification(enum boot_image_enum image, uint32_t *checksum);

    /**
     * @brief Get the start address of the image.
     * 
     * @param image The image to get the start address of.
     * @param address_in_executable_image The address in the executable image.
     * 
     * @return The start address of the image.
    */
    uint32_t app_image_address_get(enum boot_image_enum image, uint32_t address_in_executable_image);
    
    uint16_t BOOT_EraseSizeGet();
    nvm_return_status_t app_image_write_flash_row(uint32_t nvmAddress, uint32_t lengthInBytes, uint8_t *sourceData, uint32_t key);
    nvm_return_status_t BOOT_BlockRead(uint8_t *destinationData, uint32_t lengthInBytes, uint32_t nvmAddress);
    nvm_return_status_t BOOT_BlockErase(uint32_t nvmAddress, uint32_t lengthInBytes, uint32_t key);

    // Functions to read RAM data stored in flash in 16 bit values in each instruction location.
    nvm_return_status_t BOOT_Read32Data(uint32_t *destinationData, uint32_t nvmAddress);
    nvm_return_status_t BOOT_Read16Data(uint16_t *destinationData, uint32_t nvmAddress);

    void system_initialise(void);
    boot_command_response_t boot_process_command(can2_message_t *m);

    void bootloader_init(void);
    void bootloader_start_application_image(void);
    bool bootloader_verify_application_image(uint32_t *checksum);

    void clock_init(void);
    void system_initialise(void);

#ifdef __cplusplus
}
#endif

#endif
