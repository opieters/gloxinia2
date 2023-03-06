#include "bootloader_config.h"
    	     
__bootloader_DefaultInterrupt:
    goto __DefaultInterrupt
    
    .equ remap_offset, 4
    
    .macro remap_to_application NAME
    .pword BOOT_CONFIG_USER_IVT_TABLE_ADDRESS+remap_offset
    .equ remap_offset, remap_offset+4
    .endm
    
    .macro keep_in_bootloader NAME
    .weak __\NAME
    .global __\NAME
    .equiv __\NAME, __bootloader_DefaultInterrupt
    .pword __\NAME
    .endm
    
    .macro remap_to_application_default NAME
    .pword BOOT_CONFIG_USER_IVT_TABLE_ADDRESS
    .endm
    
/*
 * IVT section information.
 */
    .section .ivt, code, address(0x4), keep
    
    #define REMAP_TABLE
    .include "interrupts.S"
