#ifndef BOOT_VERIFY_CRC32_H
#define BOOT_VERIFY_CRC32_H

struct __attribute__((packed)) BOOT_VERIFY_APPLICATION_HEADER
{
    uint32_t crc32;
    uint32_t startAddress;
    uint32_t endAddress;
};

#endif