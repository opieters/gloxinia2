#ifndef __UART_BOOTLOADER_H__
#define	__UART_BOOTLOADER_H__

#include <xc.h> 
#include <stdbool.h>
#include <stdint.h>
#include <message.h>
#include "bootloader_config.h"

#define DOWNLOAD_IMAGE_NUMBER 0u
#define BOOT_IMAGE_COUNT 1
#define BOOT_IMAGE_SIZE 0x53000
#define BOOTLOADER_ADDRESS (0x00)

#define DOWNLOADED_IMAGE 0u
#define EXECUTION_IMAGE 0u

typedef enum boot_image_enum
{
    BOOT_IMAGE_0 = 0,
} boot_image_t;

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

typedef struct {
    uint8_t data[BOOT_CONFIG_MAX_PACKET_SIZE];
    message_cmd_t command;
    uint8_t address_low;
    uint8_t length;
    bool unlock;
} bootloader_message_t;

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

#ifdef	__cplusplus
extern "C" {
#endif 
    
    void system_initialise(void);
    
    void bootloader_init(void);
    void bootloader_run(void);
    void clock_init(void);
    void pins_init(void);
    void pins_reset (void);
    bool app_image_verification(boot_image_t image, uint32_t* checksum);
    
    boot_command_response_t boot_process_command(bootloader_message_t* m);
    void bootloader_start_app_image(void);
    bool bootloader_verify(uint32_t* checksum);
    void bootloader_tx_message(bootloader_message_t* m);
    bool boot_queue_push(const bootloader_message_t* m);
    bootloader_message_t* boot_queue_pop(void);

#ifdef	__cplusplus
}
#endif 

#endif

