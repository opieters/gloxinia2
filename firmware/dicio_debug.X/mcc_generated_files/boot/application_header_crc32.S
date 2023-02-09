/* This file puts an application header at the start of the application image that the
 * bootloader will use to verify the integrity of the application image.
 */

#include "boot_config.h"

//#error "A script to append verification checksum has been added to your project. You must add a call to it as a post-build step in your project properties. Delete this warning after you have done so."  
/*
* Steps to append verification checksum to the project hex file:  
* 1. Right click on your project, and click 'Properties'.
* 2. Select the 'Building' left navigation node.
* 3. Check the box next to 'Execute this line after the build'.
* 4. In the text field below, 
*      add "cd mcc_generated_files/boot && postBuild.bat $(MP_CC_DIR) ${ProjectDir} ${ImageDir} ${ImageName} ${IsDebug}" (without quotes) if you are on a Windows machine, 
*      or "cd mcc_generated_files/boot && ./postBuild.sh $(MP_CC_DIR) ${ProjectDir} ${ImageDir} ${ImageName} ${IsDebug}" (without quotes) if you are on a Linux/Unix/Mac machine.  
* 5. Delete or comment out the #error message above
*/

    .section .application_header_checksum16, code, address(BOOT_CONFIG_APPLICATION_IMAGE_APPLICATION_HEADER_ADDRESS), keep
    
    /* Skip the instruction that holds the CRC32 result - this will have to be injected
     * by a tool later, but we need to reserve the space for it. 
     */
    .long(0x00000000)
	
    /* Address where to start the checksum */
    .long(0x00002800)
	
    /* Address where to end the checksum */
    .long(0x000557FE)
    
    /* Firmware Image Reset Remap */
    goto __resetPRI 
    
    /* Number of Application Headers */
    .word (2) 
    
    /* First header: Software Version Number */

    /* Version Number ID */
    .word (2) 

    /* Version Number Length in bytes */
    .word (4) 
    
    /* Version Number Data - 32-bit number in the format of:
     *   0x00MMmmpp
     *     MM = Major revision
     *     mm = minor revision
     *     pp = patch revision
     *
     *     e.g. 0x00020A03 = version 2.10.3
     */
    .long(0x00010100)
    
    /* Second header: Device identification */
    .word (3)
    
    /* Device identification length */
    .word (4)
    
    /* Device identification data - 16-bit number formatted as:
     *   0xDDDDVVVV
     *     DDDD = device indicator
     *       0x000A = dicio
     *       0x000B = sylvatica
     *       0x000C = planalta
     *     VVVV = hardware version indicator
     */
    .long (0x000A0007)
	
