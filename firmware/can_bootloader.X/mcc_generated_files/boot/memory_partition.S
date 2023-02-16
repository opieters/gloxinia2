/* This code will block out the space reserved by the application so that the 
 * boot code can't link into that area.
 *
 * The value in the address() field defines the start address of the
 * application code.
 *
 * The number on the .space defines the size of this block in PC units (2 per
 * instruction).
 */
#include "boot_config.h"    
    .section *, code, address(BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW), noload, keep
reserved_application_memory:
    .space 0x557FE  -  BOOT_CONFIG_PROGRAMMABLE_ADDRESS_LOW, 0x00
