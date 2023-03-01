#include <xc.h>
#include <stdint.h>
#include <libpic30.h>
#include "pin_manager.h"
#include "can_bootloader.h"
#include "flash_memory.h"
#include "can2.h"
#include "traps.h"
#include "app_image.h"

extern volatile uint32_t last_address_written;
extern volatile bool received_bootloader_message;
uint32_t live_checksum = 0;

static boot_command_response_t boot_cmd_reset_device(can2_message_t *m);
static boot_command_response_t bootloader_cmd_erase_flash(can2_message_t *m);
static boot_command_response_t bootloader_cmd_write_flash(can2_message_t *m);
static boot_command_response_t bootloader_cmd_read_flash(can2_message_t *m);
static boot_command_response_t bootloader_cmd_self_verify(can2_message_t *m);
static boot_command_response_t bootloader_cmd_get_mem_address_range(can2_message_t *m);
static boot_command_response_t bootloader_cmd_write_flash_init(can2_message_t *m);
static boot_command_response_t bootloader_cmd_flash_done(can2_message_t *m);
static boot_command_response_t bootloader_cmd_error(boot_command_response_t, can2_message_t *m);
static boot_command_response_t bootloader_cmd_read_version(can2_message_t *);

boot_command_response_t boot_process_command(can2_message_t *m)
{
    received_bootloader_message = true;

    switch (m->command)
    {

    case M_BOOT_WRITE_FLASH:
        return bootloader_cmd_write_flash(m);

    case M_BOOT_READ_FLASH:
        return bootloader_cmd_read_flash(m);

    case M_BOOT_READ_VERSION:
        return bootloader_cmd_read_version(m);

    case M_BOOT_ERASE_FLASH:
        return bootloader_cmd_erase_flash(m);

    case M_BOOT_RESET_DEVICE:
        boot_cmd_reset_device(m);
        return COMMAND_SUCCESS;

    case M_BOOT_SELF_VERIFY:
        return bootloader_cmd_self_verify(m);

    case M_BOOT_GET_MEMORY_ADDRESS_RANGE_COMMAND:
        return bootloader_cmd_get_mem_address_range(m);

    case M_BOOT_WRITE_FLASH_INIT:
        return bootloader_cmd_write_flash_init(m);

    case M_BOOT_WRITE_FLASH_DONE:
        return bootloader_cmd_flash_done(m);

    default:
        return bootloader_cmd_error(UNSUPPORTED_COMMAND, m);
    }

    return UNSUPPORTED_COMMAND;
}

static boot_command_response_t bootloader_cmd_error(boot_command_response_t errorType, can2_message_t *m)
{
    can2_message_t err_response;

    copy_can2_message(m, &err_response);
    err_response.length = 5;
    err_response.data[4] = errorType;

    can2_send_message_any_ch(&err_response);

    return BOOT_CMD_ERROR;
}

static boot_command_response_t bootloader_cmd_read_version(can2_message_t *m)
{
    can2_message_t response;
    //__psv__ const char __attribute__((eds,space(prog),address(__DEVID_BASE))) device_information_area;
    // static __psv__ const char __attribute__((space(psv),address(__DEVID_BASE))) device_information_area;
    // static const char * __attribute__((space(prog))) device_information_area = __DEVID_BASE;
    //__prog__ unsigned int __attribute__((space(prog))) myPROGvar = 0x1234;

    // static __psv__ const char __attribute__((space(prog), address(__DEVID_BASE))) device_information;
    //__psv__ char* device_information_area = device_information;
    //__psv__ char* device_information_area = (__psv__ char*) __DEVID_BASE;
    //__psv__ char* device_information_area = (__psv__ char*) __DEVID_BASE;

    copy_can2_message(m, &response);
    response.length = 13;

    // address copied by means of previous command
    // bootloader version
    response.data[0] = BOOT_CONFIG_VERSION >> 8;
    response.data[1] = BOOT_CONFIG_VERSION & 0xff;
    // max packet size
    response.data[2] = BOOT_CONFIG_MAX_PACKET_SIZE;
    // software version of image
    uint32_t sw_version;
    BOOT_Read32Data(&sw_version, BOOT_CONFIG_DOWNLOAD_LOW + 22);
    response.data[3] = (uint8_t)(sw_version >> 24);
    response.data[4] = (uint8_t)(sw_version >> 16);
    response.data[5] = (uint8_t)(sw_version >> 8);
    response.data[6] = (uint8_t)(sw_version & 0xff);
    // hardware version of image
    uint32_t hw_version;
    BOOT_Read32Data(&hw_version, BOOT_CONFIG_DOWNLOAD_LOW + 30);
    response.data[7] = (uint8_t)(hw_version >> 24);
    response.data[8] = (uint8_t)(hw_version >> 16);
    response.data[9] = (uint8_t)(hw_version >> 8);
    response.data[10] = (uint8_t)(hw_version & 0xff);
    // erase row size
    response.data[11] = BOOT_EraseSizeGet();
    // write row size
    response.data[12] = MINIMUM_WRITE_BLOCK_SIZE;

    can2_send_message_any_ch(&response);

    return COMMAND_SUCCESS;
}

static boot_command_response_t boot_cmd_reset_device(can2_message_t *m)
{
    can2_message_t response;

    copy_can2_message(m, &response);
    response.length = 5;

    // address copied by means of previous command
    // bootloader version
    response.data[4] = COMMAND_SUCCESS;

    can2_send_message_any_ch(&response);

    can2_message_t broadcast;
    broadcast.command = M_BOOT_READY;
    broadcast.unlock = false;
    broadcast.length = 0;
    can2_send_message_any_ch(&broadcast);

    C2CTRL1bits.REQOP = CAN_MODULE_DISABLE; // disable CAN module
    while (C2CTRL1bits.OPMODE != CAN_MODULE_DISABLE)
        ; // wait for mode switch

    // asm ("reset");
    
    return BOOT_CMD_ERROR;
}

static boot_command_response_t bootloader_cmd_erase_flash(can2_message_t *m)
{
    uint32_t eraseAddress = 0;
    uint16_t data_length = 0;
    uint32_t unlock_sequence = 0;
    boot_command_response_t success = BAD_ADDRESS;
    can2_message_t response;

    copy_can2_message(m, &response);

    for (int i = 0; i < sizeof(eraseAddress); i++)
    {
        eraseAddress = (eraseAddress << 8) | m->data[i];
    }
    for (int i = 0; i < sizeof(data_length); i++)
    {
        data_length = (data_length << 8) | m->data[sizeof(eraseAddress) + i];
    }

    //(uint32_t nvmAddress, uint32_t lengthInPages, uint32_t key)
    if (m->unlock)
    {
        /* destroy the unlock key so it isn't sitting around in memory. */
        m->unlock = false;
        unlock_sequence = 0x00AA0055;
    }

    if (BOOT_BlockErase(eraseAddress, data_length, unlock_sequence) == NVM_SUCCESS)
    {
        response.data[4] = COMMAND_SUCCESS;
        success = COMMAND_SUCCESS;
    }
    else
    {
        response.data[4] = BAD_ADDRESS;
    }
    response.length = 5;

    can2_send_message_any_ch(&response);

    if (success == COMMAND_SUCCESS)
    {
        return COMMAND_SUCCESS;
    }

    return BOOT_CMD_ERROR;
}


static boot_command_response_t bootloader_cmd_write_flash(can2_message_t *m)
{
    uint32_t flash_address = 0;
    uint16_t data_length;
    can2_message_t response;
    uint32_t unlock_sequence = 0;

    copy_can2_message(m, &response);
    response.length = 5;

    data_length = m->length - 4;

    for (int i = 0; i < 4; i++)
    {
        flash_address = (flash_address << 8) | m->data[i];
    }

    response.data[4] = COMMAND_SUCCESS;

    if (m->unlock)
    {
        unlock_sequence = 0x00AA0055;
        /* destroy the unlock key so it isn't sitting around in memory. */
        m->unlock = false;
    }
    else
    {
        unlock_sequence = 0;
    }

    nvm_return_status_t s = app_image_write_flash_row(flash_address, data_length, &m->data[4], unlock_sequence);

    last_address_written = MAX(flash_address, last_address_written);

    // TODO: write BOOT_Read32Data(&address_last_data, BOOT_CONFIG_VAR_IMAGE_LAST_ADDRESS); final address in some way
    if (s != NVM_SUCCESS)
    {
        response.data[4] = BAD_ADDRESS;
    }

    can2_send_message_any_ch(&response);

    if (response.data[4] == COMMAND_SUCCESS)
    {
        return COMMAND_SUCCESS;
    }

    return BOOT_CMD_ERROR;
}
static boot_command_response_t bootloader_cmd_read_flash(can2_message_t *m)
{
    uint32_t flash_address = 0;
    can2_message_t response;
    uint16_t data_length = m->length - 4;

    copy_can2_message(m, &response);

    for (int i = 0; i < sizeof(flash_address); i++)
    {
        flash_address = (flash_address << 8) | m->data[i];
    }

    nvm_return_status_t readStatus = BOOT_BlockRead((uint8_t *)&response.data[4], data_length, flash_address);
    if (readStatus == NVM_SUCCESS)
    {
        response.data[4] = COMMAND_SUCCESS;
        response.length = data_length + 5;
    }
    else
    {
        response.data[4] = BAD_ADDRESS;
        response.length = 5;
    }

    can2_send_message_any_ch(&response);

    if (response.data[4] == COMMAND_SUCCESS)
    {
        return COMMAND_SUCCESS;
    }

    return BOOT_CMD_ERROR;
}

static boot_command_response_t bootloader_cmd_self_verify(can2_message_t *m)
{
    can2_message_t response;
    uint32_t checksum = 0;

    copy_can2_message(m, &response);

    if (app_image_verification(DOWNLOAD_IMAGE_NUMBER, &checksum) == false)
    {
        response.data[4] = VERIFY_FAIL;
    }
    else
    {
        response.data[4] = COMMAND_SUCCESS;
    }

    response.length = 5;
    can2_send_message_any_ch(&response);

    if (response.data[4] == COMMAND_SUCCESS)
    {
        return COMMAND_SUCCESS;
    }

    return BOOT_CMD_ERROR;
}

static boot_command_response_t bootloader_cmd_get_mem_address_range(can2_message_t *m)
{
    can2_message_t response;

    copy_can2_message(m, &response);

    response.data[4] = COMMAND_SUCCESS;

    uint32_t flash_address = EXECUTABLE_IMAGE_FIRST_ADDRESS;
    int i = 5;
    response.data[i++] = (uint8_t)(flash_address >> 24);
    response.data[i++] = (uint8_t)(flash_address >> 16);
    response.data[i++] = (uint8_t)(flash_address >> 8);
    response.data[i++] = (uint8_t)(flash_address & 0xff);

    flash_address = EXECUTABLE_IMAGE_LAST_ADDRESS;
    response.data[i++] = (uint8_t)(flash_address >> 24);
    response.data[i++] = (uint8_t)(flash_address >> 16);
    response.data[i++] = (uint8_t)(flash_address >> 8);
    response.data[i++] = (uint8_t)(flash_address & 0xff);

    response.length = 5 + 2 * sizeof(flash_address);

    can2_send_message_any_ch(&response);

    return COMMAND_SUCCESS;
}

static boot_command_response_t bootloader_cmd_write_flash_init(can2_message_t *m)
{
    return COMMAND_SUCCESS;
}

static boot_command_response_t bootloader_cmd_flash_done(can2_message_t *m)
{
    uint32_t data[2] = {0};
    bool s;

    // write last location with image data
    data[0] = (uint16_t)(last_address_written);
    data[1] = (uint16_t)(last_address_written >> 16);

    FLASH_Unlock(FLASH_UNLOCK_KEY);
    s = FLASH_WriteDoubleWord24(BOOT_CONFIG_APPLICATION_IMAGE_APPLICATION_HEADER_ADDRESS + 8, data[0], data[1]);
    FLASH_Lock();

    BOOT_Read32Data((uint32_t *)&last_address_written, BOOT_CONFIG_APPLICATION_IMAGE_APPLICATION_HEADER_ADDRESS + 8);

    if (s)
    {
        return COMMAND_SUCCESS;
    }
    else
    {
        return BAD_ADDRESS;
    }
}
