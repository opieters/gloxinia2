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

static bool inBootloadMode = false;
static bool executionImageRequiresValidation = true;
static bool executionImageValid = false;

static bool EnterBootloadMode(void);

volatile bool received_bootloader_message = false;

void BOOT_DEMO_Initialize(void)
{    
    uart1_message_t broadcast;
    broadcast.command = M_BOOT_READY;
    broadcast.unlock = false;
    broadcast.length = 0;
    for(int i = 0; i < 3; i++){
        uart1_tx_message(&broadcast);
        __delay_ms(100);
    }
    __delay_ms(100);
    __delay_ms(100);
}


void BOOT_DEMO_Tasks(void)
{
    if(inBootloadMode == false)
    {
        if( EnterBootloadMode() == true )
        {
            inBootloadMode = true;
        }
        else
        {
            if( executionImageRequiresValidation == true )
            {
                executionImageValid = BOOT_ImageVerify(EXECUTION_IMAGE);
            }

            if(executionImageValid == false)
            {
                inBootloadMode = true;
            }

            if(inBootloadMode == false)
            {
                /* NOTE: Before starting the application, all interrupts used
                 * by the bootloader must be disabled. Add code here to return
                 * the peripherals/interrupts to the reset state before starting
                 * the application code. */
				 //#warning "All interrupt sources and peripherals should be disabled before starting the application.  Add any code required here to disable all interrupts and peripherals used in the bootloader."
				 
                PIN_MANAGER_Deactivate();
                UART1_Deactivate();
                BOOT_StartApplication();
            }
        }
    }
}

static bool EnterBootloadMode(void)
{
    return received_bootloader_message;
}
