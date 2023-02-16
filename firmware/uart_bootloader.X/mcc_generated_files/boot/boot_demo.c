#include <libpic30.h>
#include <stdbool.h>
#include <stdint.h>
#include "boot_config.h"
#include "boot_application_header.h"
#include "boot_image.h"
#include "boot_process.h"
#include "../uart1.h"
#include "../pin_manager.h"

#define DOWNLOADED_IMAGE    0u
#define EXECUTION_IMAGE     0u

static bool executionImageRequiresValidation = true;
static bool executionImageValid = false;

volatile bool received_bootloader_message = false;

#include "../uart_bootloader.X/mcc_generated_files/memory/flash.h"

void BOOT_DEMO_Initialize(void) {
    uart1_message_t broadcast;
    broadcast.command = M_BOOT_READY;
    broadcast.unlock = false;
    broadcast.length = 0;
    __delay_ms(100);
    uart1_tx_message(&broadcast);
    uart1_wait_tx();    
}

void BOOT_DEMO_Tasks(void) {
    uint32_t checksum;
    
    if (received_bootloader_message) {
        // no nothing, commands are processed async
        //BOOT_ImageVerify(EXECUTION_IMAGE);
    } else {
        if (executionImageRequiresValidation == true) {
            executionImageValid = BOOT_ImageVerify(EXECUTION_IMAGE, &checksum);
        }

        // TODO: forced bootloader mode for now, remove later
        //executionImageValid = false;
        if (executionImageValid == false) {
            received_bootloader_message = true;
        } else {
            /* NOTE: Before starting the application, all interrupts used
             * by the bootloader must be disabled. Add code here to return
             * the peripherals/interrupts to the reset state before starting
             * the application code. */
            PIN_MANAGER_Deactivate();
            UART1_Deactivate();
            BOOT_StartApplication();
        }
    }
    if(_U1RXIE == 0){
        BOOT_ProcessCommand();
        _U1RXIE = 1;
    }   
    if(U1STAbits.OERR == 1)
        U1STAbits.OERR = 0;
}
