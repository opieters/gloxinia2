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

static boot_command_response_t CommandError(boot_command_response_t);
static boot_command_response_t ReadVersion(void);

static boot_command_response_t ResetDevice(void);
static boot_command_response_t EraseFlash(void);
static boot_command_response_t WriteFlash(void);
static boot_command_response_t ReadFlash(void);
static boot_command_response_t SelfVerify(void);
static boot_command_response_t GetMemoryAddressRange(void);
static boot_command_response_t WriteFlashInit(void);
static boot_command_response_t WriteFlashDone(void);

extern volatile bool received_bootloader_message;
extern volatile uint32_t last_address_written;


extern uart1_message_t uart1_rx_m;

boot_command_response_t boot_process_command(void)
{    
    received_bootloader_message = true;
    
    switch (uart1_rx_m.command)
    {
        
    case M_BOOT_WRITE_FLASH:
        return WriteFlash();

    case M_BOOT_READ_FLASH:
        return ReadFlash();

    case M_BOOT_READ_VERSION:
        return ReadVersion();

    case M_BOOT_ERASE_FLASH:
        return EraseFlash();

    case M_BOOT_RESET_DEVICE:
        ResetDevice();
        return COMMAND_SUCCESS;

    case M_BOOT_SELF_VERIFY:
        return SelfVerify();

     
    case M_BOOT_GET_MEMORY_ADDRESS_RANGE_COMMAND:
        return GetMemoryAddressRange();
        
        case     M_BOOT_WRITE_FLASH_INIT:
            return WriteFlashInit();
        case M_BOOT_WRITE_FLASH_DONE:
            return WriteFlashDone();

    default:
        return CommandError(UNSUPPORTED_COMMAND);
    }
    
    return BOOT_CMD_ERROR;
}

/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/

extern uart1_message_t uart1_rx_m;


static boot_command_response_t CommandError(boot_command_response_t errorType)
{    
    uart1_message_t err_response;
    
    copy_uart1_message(&uart1_rx_m, &err_response);
    err_response.length = 5;
    err_response.data[4] = errorType;

    uart1_tx_message(&err_response);
    
    return BOOT_CMD_ERROR;
}

static boot_command_response_t ReadVersion(void)
{
    uart1_message_t response;
    //__psv__ const char __attribute__((eds,space(prog),address(__DEVID_BASE))) device_information_area;
    //static __psv__ const char __attribute__((space(psv),address(__DEVID_BASE))) device_information_area;
    //static const char * __attribute__((space(prog))) device_information_area = __DEVID_BASE;
    //__prog__ unsigned int __attribute__((space(prog))) myPROGvar = 0x1234;
    
    //static __psv__ const char __attribute__((space(prog), address(__DEVID_BASE))) device_information;
    //__psv__ char* device_information_area = device_information;
    //__psv__ char* device_information_area = (__psv__ char*) __DEVID_BASE;
    //__psv__ char* device_information_area = (__psv__ char*) __DEVID_BASE;
    
    
    copy_uart1_message(&uart1_rx_m, &response);
    response.length = 13;
    
    // address copied by means of previous command
    // bootloader version
    response.data[0] = BOOT_CONFIG_VERSION >> 8;
    response.data[1] = BOOT_CONFIG_VERSION & 0xff;
    // max packet size
    response.data[2] = BOOT_CONFIG_MAX_PACKET_SIZE;
    // software version of image
    uint32_t sw_version;
    BOOT_Read32Data(&sw_version, BOOT_CONFIG_DOWNLOAD_LOW+22);
    response.data[3] = (uint8_t) (sw_version >> 24);
    response.data[4] = (uint8_t) (sw_version >> 16);
    response.data[5] = (uint8_t) (sw_version >> 8);
    response.data[6] = (uint8_t) (sw_version & 0xff);
    // hardware version of image
    uint32_t hw_version;
    BOOT_Read32Data(&hw_version, BOOT_CONFIG_DOWNLOAD_LOW+30);
    response.data[7] = (uint8_t) (hw_version >> 24);
    response.data[8] = (uint8_t) (hw_version >> 16);
    response.data[9] = (uint8_t) (hw_version >> 8);
    response.data[10] = (uint8_t) (hw_version & 0xff);
    // erase row size
    response.data[11] = BOOT_EraseSizeGet();
    // write row size
    response.data[12] = MINIMUM_WRITE_BLOCK_SIZE;

    uart1_tx_message(&response);

    return COMMAND_SUCCESS;
}

static boot_command_response_t ResetDevice(void)
{
    uart1_message_t response;    
    
    copy_uart1_message(&uart1_rx_m, &response);
    response.length = 5;
    
    // address copied by means of previous command
    // bootloader version
    response.data[4] = COMMAND_SUCCESS;

    
    uart1_tx_message(&response);   
    uart1_wait_tx();
    
    uart1_message_t broadcast;
    broadcast.command = M_BOOT_READY;
    broadcast.unlock = false;
    broadcast.length = 0;
     uart1_tx_message(&broadcast);
     uart1_wait_tx();
    
    //asm ("reset");
     
     return COMMAND_SUCCESS;
 }


static boot_command_response_t EraseFlash(void)
{
    uint32_t eraseAddress = 0;
    uint16_t data_length = 0;
    uint32_t unlock_sequence = 0;
    boot_command_response_t success = BAD_ADDRESS;
    uart1_message_t response;    
    
    copy_uart1_message(&uart1_rx_m, &response);

    for(int i = 0; i < sizeof(eraseAddress); i++){
        eraseAddress = (eraseAddress << 8) | uart1_rx_m.data[i];
    }
    for(int i = 0; i < sizeof(data_length); i++){
        data_length = (data_length << 8) | uart1_rx_m.data[sizeof(eraseAddress)+i];
    }
    
    //(uint32_t nvmAddress, uint32_t lengthInPages, uint32_t key)
    if(uart1_rx_m.unlock)
    {
        /* destroy the unlock key so it isn't sitting around in memory. */
        uart1_rx_m.unlock = false;
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
static boot_command_response_t WriteFlash(void)
{
    uint32_t flash_address = 0;   
    uint16_t data_length;
    uart1_message_t response;
    uint32_t unlock_sequence = 0;
    
    copy_uart1_message(&uart1_rx_m, &response);
    response.length = 5;
    
    data_length = uart1_rx_m.length - 4;
    
    for(int i = 0; i < 4; i++)
    {
        flash_address = (flash_address << 8) | uart1_rx_m.data[i];
    }
    
    response.data[4] = COMMAND_SUCCESS;
    
    if(uart1_rx_m.unlock)
    {
        unlock_sequence = 0x00AA0055;
        /* destroy the unlock key so it isn't sitting around in memory. */
        uart1_rx_m.unlock = false;
    } 
    else 
    {
        unlock_sequence = 0;
    }
    
    nvm_return_status_t s = BOOT_BlockWrite(flash_address, data_length, &uart1_rx_m.data[4], unlock_sequence);
    
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
static boot_command_response_t ReadFlash(void)
{
    uint32_t flash_address = 0;   
    uart1_message_t response;
    uint16_t data_length = uart1_rx_m.length - 4;
    
    copy_uart1_message(&uart1_rx_m, &response);

    for(int i = 0; i < sizeof(flash_address); i++)
    {
        flash_address = (flash_address << 8) | uart1_rx_m.data[i];
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

static boot_command_response_t SelfVerify(void)
{
    uart1_message_t response;
    uint32_t checksum = 0;
    
    copy_uart1_message(&uart1_rx_m, &response);

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

static boot_command_response_t GetMemoryAddressRange(void)
{
    uart1_message_t response;
    
    copy_uart1_message(&uart1_rx_m, &response);
    
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


static boot_command_response_t WriteFlashInit(void)
{
    return COMMAND_SUCCESS;
}


static boot_command_response_t WriteFlashDone(void)
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
   



