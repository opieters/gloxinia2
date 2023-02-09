#ifndef BOOT_PROCESS_H
#define BOOT_PROCESS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BOOT_COMMAND_SUCCESS,
    BOOT_COMMAND_NONE,
    BOOT_COMMAND_INCOMPLETE,
    BOOT_COMMAND_ERROR,
} boot_command_result_t;


void BOOT_Initialize(void);
boot_command_result_t BOOT_ProcessCommand(void);
void BOOT_StartApplication(void);
bool BOOT_Verify(void);

#endif 
