#ifndef __BOOTLOADER_CONFIG_H__
#define __BOOTLOADER_CONFIG_H__

/// @brief Application image low address
#define BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW 0x2800

/// @brief Application image upper limit address
#define BOOT_CONFIG_PROGRAMMABLE_ADDRESS_HIGH 0x557FE

/// @brief Bootloader can download to addresses starting here
#define BOOT_CONFIG_DOWNLOAD_LOW 0x2800

/// @brief Bootloader can download to addresses until here (included)
#define BOOT_CONFIG_DOWNLOAD_HIGH 0x557FE

/// @brief Application image header size
#define BOOT_CONFIG_VERIFICATION_APPLICATION_HEADER_SIZE (0x20)

/// @brief Application image header address
#define BOOT_CONFIG_APPLICATION_IMAGE_APPLICATION_HEADER_ADDRESS (BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW)

/// @brief Application image reset address
#define BOOT_CONFIG_APPLICATION_RESET_ADDRESS (BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW + BOOT_CONFIG_VERIFICATION_APPLICATION_HEADER_SIZE)

/// @brief Application image interrupt table address
#define BOOT_CONFIG_USER_IVT_TABLE_ADDRESS 0x2A00

/// @brief Bootloader version
#define BOOT_CONFIG_VERSION (0x0102)

/// @brief Bootloader max packet size
#define BOOT_CONFIG_MAX_PACKET_SIZE (0x8)

/// @brief Bootloader block write size
#define MINIMUM_WRITE_BLOCK_SIZE 4U

/// @brief Start address of the application image
#define EXECUTABLE_IMAGE_FIRST_ADDRESS BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW

/// @brief End address of the application image
#define EXECUTABLE_IMAGE_LAST_ADDRESS ((unsigned long)BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW + (unsigned long)BOOT_IMAGE_SIZE - (unsigned long)2u)

#endif