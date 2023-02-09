#ifndef BOOT_PRIVATE_H
#define BOOT_PRIVATE_H

#include "boot_config.h" 
#define MINIMUM_WRITE_BLOCK_SIZE 8u


#define SIZE_OF_CMD_STRUCT_0 11

struct __attribute__((__packed__)) CMD_STRUCT_0_WITH_PAYLOAD{
    uint8_t cmd;
    uint16_t dataLength;
    uint32_t unlockSequence;
    uint32_t address;
    uint8_t data[BOOT_CONFIG_MAX_PACKET_SIZE - SIZE_OF_CMD_STRUCT_0];
};

struct __attribute__((__packed__)) GET_VERSION_RESPONSE{
    uint8_t cmd;
    uint16_t dataLength;
    uint32_t unlockSequence;
    uint32_t address;
    //---
    uint16_t version;
    uint16_t maxPacketLength;
    uint16_t unused1;
    uint16_t deviceId;
    uint16_t unused2;
    uint16_t eraseSize;
    uint16_t writeSize;
    uint32_t unused3;
    uint32_t userRsvdStartSddress;
    uint32_t userRsvdEndSddress;
};   
    
struct __attribute__((__packed__)) RESPONSE_TYPE_0{
    uint8_t cmd;
    uint16_t dataLength;
    uint32_t unlockSequence;
    uint32_t address;
    //---
    uint8_t success;
};
struct __attribute__((__packed__)) RESPONSE_TYPE_0_2_PAYLOAD{
    uint8_t cmd;
    uint16_t dataLength;
    uint32_t unlockSequence;
    uint32_t address;
    //---
    uint8_t success;
    uint16_t data;
};
struct __attribute__((__packed__)) RESPONSE_TYPE_0_WITH_PAYLOAD{
    uint8_t cmd;
    uint16_t dataLength;
    uint32_t unlockSequence;
    uint32_t address;
    //---
    uint8_t success;    
    uint8_t data[BOOT_CONFIG_MAX_PACKET_SIZE - SIZE_OF_CMD_STRUCT_0 - 1u];
}; 
struct __attribute__((__packed__)) GET_MEMORY_ADDRESS_RANGE_RESPONSE{
    uint8_t  cmd;
    uint16_t dataLength;  
    uint32_t unlockSequence; 
    uint32_t address; 
     uint8_t success; 
    //---
    uint32_t programFlashStart; 
    uint32_t programFlashEnd; 
};

#endif
