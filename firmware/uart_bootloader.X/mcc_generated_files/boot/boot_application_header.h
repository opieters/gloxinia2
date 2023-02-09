#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "boot_image.h"

#ifndef BOOT_APPLICATION_HEADER_H
#define BOOT_APPLICATION_HEADER_H

enum BOOT_APPLICATION_DETAIL_ID
{
    BOOT_APPLICATION_DETAIL_ID_VERSION_NUMBER = 0x0002
};   

bool BOOT_ApplicationDetailGet(enum BOOT_IMAGE image, enum BOOT_APPLICATION_DETAIL_ID id, uint16_t* dataBuffer, size_t bufferLength);

/* Format of the version number is
 *   0x00 (8-bit) : major (8-bit) : minor (8-bit) : patch (8-bit)
 *   i.e. 0x0001020A is version 1.2.10
 */
bool BOOT_VersionNumberGet(enum BOOT_IMAGE image, uint32_t *versionNumber);

#endif