#include "boot_config.h"
    .equ    ERASE_PAGE_MASK,(~((2048) - 1)) 
    .equ    LAST_PAGE_START_ADDRESS, (0x557FE & ERASE_PAGE_MASK)
    .equ    RESERVED_MEMORY_START, (0x557FE+2)
    .equ    PROGRAM_MEMORY_ORIGIN, (0x200)
    .equ    LAST_ADDRESS_OF_MEMORY, (0x55FFE)
   
    /* The exectuable application image lives between:
     *   Start Address = 0x2800
     *   End Address = 0x557FE
     * 
     * All other memory on the device must be reserved so that executable
     * application code can't link to those addresses.  This includes the space
     * required by the bootloader.
     *
     * The value in the address() field defines the start address.
     * The number on the .space defines the size of this block in PC units (2 per
     * instruction).
     */

    /* Reserve the memory used by the bootloader */
    .section *, code, address(PROGRAM_MEMORY_ORIGIN), noload, keep
boot_loader_memory:
    .space (BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW  - PROGRAM_MEMORY_ORIGIN), 0x00

    /* Reserve all of the memory after the last instruction of the executable
     * image, through the end of device memory.
     */
    .section *, code, address(RESERVED_MEMORY_START), noload, keep
config_page_memory:
    .space (LAST_ADDRESS_OF_MEMORY-RESERVED_MEMORY_START), 0x00
    
