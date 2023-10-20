#include "boot_config.h"
    
/* If you have reached this line, then an interrupt triggered that should only
 * be available in the boot loader, not in the application.
 */
__bootload_interrupt_error:
    bra $
    
/* This section defines the interrupt vector table in hardware to map to the 
 * user remap table.
 */
    .equ remap_offset, 4
    
    .macro remap_to_application NAME
    .pword BOOT_CONFIG_USER_IVT_TABLE_ADDRESS+remap_offset
    .equ remap_offset, remap_offset+4
    .endm
    
    .macro keep_in_bootloader NAME
    .pword __bootload_interrupt_error
    .endm
    
    .macro remap_to_application_default NAME
    .pword BOOT_CONFIG_USER_IVT_TABLE_ADDRESS
    .endm
    
/*
 * IVT section information.
 */
    .section .ivt, code, address(0x4), keep
    
    #define REMAP_TABLE
    .include "mcc_generated_files/boot/interrupts.S"