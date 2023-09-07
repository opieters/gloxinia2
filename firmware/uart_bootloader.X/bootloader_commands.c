#include <xc.h>
#include <libpic30.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <message.h>
#include <utilities.h>
#include "bootloader_config.h"
#include "uart1.h"
#include "flash.h"
#include "app_image.h"


#define EXECUTABLE_IMAGE_FIRST_ADDRESS BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW
#define EXECUTABLE_IMAGE_LAST_ADDRESS ((unsigned long)BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW + (unsigned long)BOOT_IMAGE_SIZE - (unsigned long)2u)


#if ((BOOT_CONFIG_MAX_PACKET_SIZE - SIZE_OF_CMD_STRUCT_0) < MINIMUM_WRITE_BLOCK_SIZE )
#error "The maximum packet size is not large enough to store a full write block plus header. Make the max packet size larger."
#endif


/******************************************************************************/
/* Private Function Prototypes                                                */
/******************************************************************************/

static boot_command_response_t CommandError(bootloader_message_t* m, boot_command_response_t);
static boot_command_response_t ReadHWVersion(bootloader_message_t* m);
static boot_command_response_t ReadSWVersion(bootloader_message_t* m);
static boot_command_response_t ReadBootVersion(bootloader_message_t* m);

static boot_command_response_t ResetDevice(bootloader_message_t* m);
static boot_command_response_t EraseFlash(bootloader_message_t* m);
static boot_command_response_t WriteFlash(bootloader_message_t* m);
static boot_command_response_t ReadFlash(bootloader_message_t* m);
static boot_command_response_t SelfVerify(bootloader_message_t* m);
static boot_command_response_t GetMemoryAddressRange(bootloader_message_t* m);
static boot_command_response_t WriteFlashInit(bootloader_message_t* m);
static boot_command_response_t WriteFlashDone(bootloader_message_t* m);
static boot_command_response_t SetAddressOffset(bootloader_message_t* m);

extern volatile bool received_bootloader_message;
extern volatile uint32_t last_address_written;
uint32_t address_offset = 0;

bool uart_not_can_active = true;

boot_command_response_t boot_process_command(bootloader_message_t* m)
{    
    received_bootloader_message = true;
    
    switch (m->command)
    {     
        case M_BOOT_WRITE_FLASH:
            return WriteFlash(m);

        case M_BOOT_READ_FLASH:
            return ReadFlash(m);

        case M_BOOT_READ_SW_VERSION:
            return ReadHWVersion(m);
            
        case M_BOOT_READ_HW_VERSION:
            return ReadSWVersion(m);
            
        case M_BOOT_READ_BOOT_VERSION:
            return ReadBootVersion(m);

        case M_BOOT_ERASE_FLASH:
            return EraseFlash(m);

        case M_BOOT_RESET_DEVICE:
            return ResetDevice(m);

        case M_BOOT_SELF_VERIFY:
            return SelfVerify(m);

        case M_BOOT_GET_MEMORY_ADDRESS_RANGE_COMMAND:
            return GetMemoryAddressRange(m);

        case M_BOOT_WRITE_FLASH_INIT:
            return WriteFlashInit(m);
            
        case M_BOOT_WRITE_FLASH_DONE:
            return WriteFlashDone(m);
        
        case M_BOOT_SET_ADDRESS_OFFSET:
            return SetAddressOffset(m);
            
    default:
        return CommandError(m, UNSUPPORTED_COMMAND);
    }
    
    return BOOT_CMD_ERROR;
}

/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/


static boot_command_response_t CommandError(bootloader_message_t* m, boot_command_response_t errorType)
{    
    bootloader_message_t err_response;
    
    bootloader_copy_message(m, &err_response);
    err_response.length = 1;
    err_response.data[0] = errorType;

    bootloader_tx_message(&err_response);
    
    return BOOT_CMD_ERROR;
}

static boot_command_response_t ReadHWVersion(bootloader_message_t* m)
{
    bootloader_message_t response;

    bootloader_copy_message(m, &response);
    response.length = 4;

    // hardware version of image
    uint32_t hw_version;
    BOOT_Read32Data(&hw_version, BOOT_CONFIG_DOWNLOAD_LOW+30);
    response.data[0] = (uint8_t) (hw_version >> 24);
    response.data[1] = (uint8_t) (hw_version >> 16);
    response.data[2] = (uint8_t) (hw_version >> 8);
    response.data[3] = (uint8_t) (hw_version & 0xff);

    bootloader_tx_message(&response);

    return COMMAND_SUCCESS;
}

static boot_command_response_t ReadSWVersion(bootloader_message_t* m)
{
    bootloader_message_t response;
    
    bootloader_copy_message(m, &response);
    response.length = 4;
    

    // software version of image
    uint32_t sw_version;
    BOOT_Read32Data(&sw_version, BOOT_CONFIG_DOWNLOAD_LOW+22);
    response.data[0] = (uint8_t) (sw_version >> 24);
    response.data[1] = (uint8_t) (sw_version >> 16);
    response.data[2] = (uint8_t) (sw_version >> 8);
    response.data[3] = (uint8_t) (sw_version & 0xff);

    bootloader_tx_message(&response);

    return COMMAND_SUCCESS;
}


static boot_command_response_t ReadBootVersion(bootloader_message_t* m)
{
    bootloader_message_t response;
    //__psv__ const char __attribute__((eds,space(prog),address(__DEVID_BASE))) device_information_area;
    //static __psv__ const char __attribute__((space(psv),address(__DEVID_BASE))) device_information_area;
    //static const char * __attribute__((space(prog))) device_information_area = __DEVID_BASE;
    //__prog__ unsigned int __attribute__((space(prog))) myPROGvar = 0x1234;
    
    //static __psv__ const char __attribute__((space(prog), address(__DEVID_BASE))) device_information;
    //__psv__ char* device_information_area = device_information;
    //__psv__ char* device_information_area = (__psv__ char*) __DEVID_BASE;
    //__psv__ char* device_information_area = (__psv__ char*) __DEVID_BASE;
    
    
    bootloader_copy_message(m, &response);
    response.length = 5;
    
    // address copied by means of previous command
    // bootloader version
    response.data[0] = BOOT_CONFIG_VERSION >> 8;
    response.data[1] = BOOT_CONFIG_VERSION & 0xff;
    // max packet size
    response.data[2] = BOOT_CONFIG_MAX_PACKET_SIZE;
    // erase row size
    response.data[3] = BOOT_EraseSizeGet();
    // write row size
    response.data[4] = MINIMUM_WRITE_BLOCK_SIZE;

    bootloader_tx_message(&response);

    return COMMAND_SUCCESS;
}


static boot_command_response_t ResetDevice(bootloader_message_t* m)
{
    bootloader_message_t response;    
    
    bootloader_copy_message(m, &response);
    response.length = 1;
    
    // address copied by means of previous command
    // bootloader version
    response.data[0] = COMMAND_SUCCESS;

    
    uart1_tx_message(&response);   
    uart1_wait_tx();
    
    bootloader_message_t broadcast;
    broadcast.command = M_BOOT_READY;
    broadcast.unlock = false;
    broadcast.length = 0;
     bootloader_tx_message(&broadcast);
     uart1_wait_tx();
    
    //asm ("reset");
     
     return COMMAND_SUCCESS;
 }


static boot_command_response_t EraseFlash(bootloader_message_t* m)
{
    uint32_t eraseAddress = 0;
    uint16_t data_length = 0;
    uint32_t unlock_sequence = 0;
    boot_command_response_t success = BAD_ADDRESS;
    bootloader_message_t response;    
    
    bootloader_copy_message(m, &response);

    for(int i = 0; i < sizeof(eraseAddress); i++){
        eraseAddress = (eraseAddress << 8) | m->data[i];
    }
    for(int i = 0; i < sizeof(data_length); i++){
        data_length = (data_length << 8) | m->data[sizeof(eraseAddress)+i];
    }
    
    //(uint32_t nvmAddress, uint32_t lengthInPages, uint32_t key)
    if(m->unlock)
    {
        /* destroy the unlock key so it isn't sitting around in memory. */
        m->unlock = false;
        unlock_sequence = 0x00AA0055;
    }
    
    if ( BOOT_BlockErase(eraseAddress, data_length, unlock_sequence) == NVM_SUCCESS)
    {
        response.data[4] = COMMAND_SUCCESS;
        success = COMMAND_SUCCESS;
    } else {
        response.data[4] = BAD_ADDRESS;
    }   
    response.length = 5;
    
    uart1_tx_message(&response);   
    
    if(success == COMMAND_SUCCESS)
    {
        return COMMAND_SUCCESS;
    }
    
    return BOOT_CMD_ERROR;
}

uint32_t live_checksum = 0;
static boot_command_response_t WriteFlash(bootloader_message_t* m)
{
    uint32_t flash_address = 0;   
    uint16_t data_length;
    bootloader_message_t response;
    uint32_t unlock_sequence = 0;
    
    bootloader_copy_message(m, &response);
    response.length = 5;
    
    data_length = m->length - 4;
    
    for(int i = 0; i < 4; i++)
    {
        flash_address = (flash_address << 8) | m->data[i];
    }
    
    response.data[4] = COMMAND_SUCCESS;
    
    if(m->unlock)
    {
        unlock_sequence = 0x00AA0055;
        /* destroy the unlock key so it isn't sitting around in memory. */
        m->unlock = false;
    } 
    else 
    {
        unlock_sequence = 0;
    }
    
    nvm_return_status_t s = BOOT_BlockWrite(flash_address, data_length, &m->data[4], unlock_sequence);
    
    last_address_written = MAX(flash_address,last_address_written);
    
    // TODO: write BOOT_Read32Data(&address_last_data, BOOT_CONFIG_VAR_IMAGE_LAST_ADDRESS); final address in some way
    if ( s != NVM_SUCCESS)
    {
        response.data[4]  = BAD_ADDRESS;   
    }

    uart1_tx_message(&response);   
    
    if(response.data[4] == COMMAND_SUCCESS)
    {
        return COMMAND_SUCCESS;
    }
    
    return BOOT_CMD_ERROR;
}
static boot_command_response_t ReadFlash(bootloader_message_t* m)
{
    uint32_t flash_address = 0;   
    bootloader_message_t response;
    uint16_t data_length = m->length - 4;
    
    bootloader_copy_message(m, &response);

    for(int i = 0; i < sizeof(flash_address); i++)
    {
        flash_address = (flash_address << 8) | m->data[i];
    }
        
    nvm_return_status_t readStatus = BOOT_BlockRead ((uint8_t *)&response.data[4], data_length, flash_address );
    if (readStatus == NVM_SUCCESS)
    {    
        response.data[4] = COMMAND_SUCCESS;
        response.length = data_length+5;   
    } 
    else
    {
        response.data[4] = BAD_ADDRESS;
        response.length = 5;        
    }
            
    
    uart1_tx_message(&response);   

    if(response.data[4] == COMMAND_SUCCESS)
    {
        return COMMAND_SUCCESS;
    }
    
    return BOOT_CMD_ERROR;
}

static boot_command_response_t SelfVerify(bootloader_message_t* m)
{
    bootloader_message_t response;
    uint32_t checksum = 0;
    
    bootloader_copy_message(m, &response);

    if(app_image_verification(DOWNLOAD_IMAGE_NUMBER, &checksum) == false)
    {
        response.data[4] = VERIFY_FAIL;
    }
    else
    {
        response.data[4] = COMMAND_SUCCESS;
    }

    response.length = 5;
    uart1_tx_message(&response);  
    
    if(response.data[4] == COMMAND_SUCCESS)
    {
        return COMMAND_SUCCESS;
    }
    
    return BOOT_CMD_ERROR;
} 

static boot_command_response_t GetMemoryAddressRange(bootloader_message_t* m)
{
    bootloader_message_t response;
    
    bootloader_copy_message(m, &response);
    
    response.data[4] = COMMAND_SUCCESS;
    
    uint32_t flash_address = EXECUTABLE_IMAGE_FIRST_ADDRESS;
    int i = 5;
    response.data[i++] = (uint8_t) (flash_address >> 24);
    response.data[i++] = (uint8_t) (flash_address >> 16);
    response.data[i++] = (uint8_t) (flash_address >> 8);
    response.data[i++] = (uint8_t) (flash_address & 0xff);
    
    flash_address = EXECUTABLE_IMAGE_LAST_ADDRESS;
    response.data[i++] = (uint8_t) (flash_address >> 24);
    response.data[i++] = (uint8_t) (flash_address >> 16);
    response.data[i++] = (uint8_t) (flash_address >> 8);
    response.data[i++] = (uint8_t) (flash_address & 0xff);
    
    response.length = 5+2*sizeof(flash_address);

    uart1_tx_message(&response);

    return COMMAND_SUCCESS;
}


static boot_command_response_t WriteFlashInit(bootloader_message_t* m)
{
    return COMMAND_SUCCESS;
}


static boot_command_response_t WriteFlashDone(bootloader_message_t* m)
{
    uint32_t data[2] = {0};
    bool s;
    
    
    // write last location with image data
    data[0] = (uint16_t) (last_address_written);
    data[1] = (uint16_t) (last_address_written >> 16);

    FLASH_Unlock(FLASH_UNLOCK_KEY);    
    s = FLASH_WriteDoubleWord24(BOOT_CONFIG_APPLICATION_IMAGE_APPLICATION_HEADER_ADDRESS+8, data[0], data[1]);
    FLASH_Lock();
    
    BOOT_Read32Data((uint32_t*) &last_address_written, BOOT_CONFIG_APPLICATION_IMAGE_APPLICATION_HEADER_ADDRESS+8);
    
    if(s)
    {
        return COMMAND_SUCCESS;
    } else {
        return BAD_ADDRESS;
    }
}
   
static boot_command_response_t SetAddressOffset(bootloader_message_t* m)
{
    if(m->length != 3){
        return BAD_ADDRESS;
    }
    for(int i = 0; i < 3; i++)
        address_offset = (address_offset << 8) | m->data[i];
    address_offset = address_offset << 8;
    
    return COMMAND_SUCCESS;
}