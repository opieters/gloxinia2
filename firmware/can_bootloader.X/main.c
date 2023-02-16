/**
  Section: Included Files
*/
#include <xc.h>
#include <stdbool.h>
#include <libpic30.h>
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/boot/boot_demo.h"
#include "mcc_generated_files/uart1.h"

/*
                         Main application
 */
int main(void)
{

    //BOOT_StartApplication();
    
    //return 1;
    
    // initialize the device
    SYSTEM_Initialize();
    BOOT_DEMO_Initialize();
    
    while (1)
    {
        // Add your application code
        BOOT_DEMO_Tasks();
    }
    return 1; 
}

