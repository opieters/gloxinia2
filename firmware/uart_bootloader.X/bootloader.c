#include <xc.h>
#include <libpic30.h>
#include "bootloader.h"
#include "uart1.h"
#include "flash.h"
#include "traps.h"
#include "can2.h"

#define DOWNLOADED_IMAGE    0u
#define EXECUTION_IMAGE     0u

static bool app_image_run_validation = true;
static bool app_image_valid = false;

volatile bool received_bootloader_message = false;
volatile uint32_t last_address_written = 0xf;
bootloader_message_t* m_process = NULL;

void system_initialise(void)
{
    pins_init(); // configure pins and connect to peripherals
    clock_init(); // start 8MHz external clock
    //INTERRUPT_Initialize();
    uart1_init();
    __builtin_enable_interrupts();
    
    // set CORCON back to original values
    CORCON = (CORCON & 0x00F2) | CORCON_MODE_PORVALUES;

    bootloader_init();
}

void bootloader_init(void) {
    received_bootloader_message = false;
    last_address_written = 0x0;
    
    bootloader_message_t broadcast;
    broadcast.command = M_BOOT_READY;
    broadcast.unlock = false;
    broadcast.length = 0;
    __delay_ms(100);
    uart1_tx_message(&broadcast);
    uart1_wait_tx();    
}

void bootloader_run(void) {
    uint32_t checksum;
    
    while(1){
        if (!received_bootloader_message) {
            if (app_image_run_validation == true) {
                app_image_valid = app_image_verification(EXECUTION_IMAGE, &checksum);
            }

            if (!app_image_valid) {
                // stay in bootloader mode since the image is not valid
                received_bootloader_message = true;
            } else {
                /* NOTE: Before starting the application, all interrupts used
                 * by the bootloader must be disabled. Add code here to return
                 * the peripherals/interrupts to the reset state before starting
                 * the application code. */
                pins_reset();
                uart1_deactivate();
                can2_disable();
                bootloader_start_app_image();
            }
        }
        
        m_process = boot_queue_pop();
        
        if(m_process != NULL)
            boot_process_command(m_process);
        
        // make sure overflow does not block the UART module
        if(U1STAbits.OERR == 1){
            U1STAbits.OERR = 0;
        }
    }
}


void clock_init(void)
{
    // FRCDIV FRC/1; PLLPRE 2; DOZE 1:8; PLLPOST 1:2; DOZEN disabled; ROI disabled; 
    CLKDIV = 0x3000;
    // TUN Center frequency; 
    OSCTUN = 0x00;
    // ROON disabled; ROSEL FOSC; RODIV 0; ROSSLP disabled; 
    REFOCON = 0x00;
    // PLLDIV 62; 
    PLLFBD = 0x3E;
    // ENAPLL disabled; APLLPOST 1:256; FRCSEL FRC; SELACLK Auxiliary Oscillators; ASRCSEL Auxiliary Oscillator; AOSCMD AUX; APLLPRE 1:2; 
    ACLKCON3 = 0x2201;
    // APLLDIV 24; 
    ACLKDIV3 = 0x07;
    // AD1MD enabled; PWMMD enabled; T3MD enabled; T4MD enabled; T1MD enabled; U2MD enabled; T2MD enabled; U1MD enabled; QEI1MD enabled; SPI2MD enabled; SPI1MD enabled; C2MD enabled; C1MD enabled; DCIMD enabled; T5MD enabled; I2C1MD enabled; 
    PMD1 = 0x00;
    // OC5MD enabled; OC6MD enabled; OC7MD enabled; OC8MD enabled; OC1MD enabled; IC2MD enabled; OC2MD enabled; IC1MD enabled; OC3MD enabled; OC4MD enabled; IC6MD enabled; IC7MD enabled; IC5MD enabled; IC8MD enabled; IC4MD enabled; IC3MD enabled; 
    PMD2 = 0x00;
    // AD2MD enabled; PMPMD enabled; CMPMD enabled; U3MD enabled; QEI2MD enabled; RTCCMD enabled; T9MD enabled; T8MD enabled; CRCMD enabled; T7MD enabled; I2C2MD enabled; DAC1MD enabled; T6MD enabled; 
    PMD3 = 0x00;
    // U4MD enabled; REFOMD enabled; 
    PMD4 = 0x00;
    // OC9MD enabled; OC16MD enabled; IC10MD enabled; IC11MD enabled; IC12MD enabled; IC13MD enabled; IC14MD enabled; IC15MD enabled; IC16MD enabled; IC9MD enabled; OC14MD enabled; OC15MD enabled; OC12MD enabled; OC13MD enabled; OC10MD enabled; OC11MD enabled; 
    PMD5 = 0x00;
    // PWM2MD enabled; PWM1MD enabled; SPI4MD enabled; PWM4MD enabled; SPI3MD enabled; PWM3MD enabled; 
    PMD6 = 0x00;
    // DMA8MD enabled; DMA4MD enabled; DMA12MD enabled; DMA0MD enabled; 
    PMD7 = 0x00;
    // CF no clock failure; NOSC PRIPLL; LPOSCEN disabled; CLKLOCK unlocked; OSWEN Switch is Complete; IOLOCK not-active; 
    __builtin_write_OSCCONH((uint8_t) (0x03));
    __builtin_write_OSCCONL((uint8_t) (0x01));
    // Wait for Clock switch to occur
    while (OSCCONbits.OSWEN != 0);
    while (OSCCONbits.LOCK != 1);
}

void bootloader_tx_message(bootloader_message_t* m)
{
    // TODO
}

#define BOOT_QUEUE_SIZE 16
bootloader_message_t boot_message_queue[BOOT_QUEUE_SIZE];
volatile uint8_t boot_message_queue_index = 0;
volatile uint8_t bool_message_queue_n = 0;

bool boot_queue_push(const bootloader_message_t* m)
{
    if(bool_message_queue_n >= (BOOT_QUEUE_SIZE - 1)){
        return false;
    }
    
    bootloader_copy_message(m, &boot_message_queue[boot_message_queue_index]);
    boot_message_queue_index = (boot_message_queue_index+1) % BOOT_QUEUE_SIZE;
    
    bool_message_queue_n++;
    
    return true;
}

bootloader_message_t* boot_queue_pop(void)
{
    if(bool_message_queue_n == 0)
        return NULL;
    
    bool_message_queue_n--;
    
    int16_t index = (boot_message_queue_index - bool_message_queue_n) % BOOT_QUEUE_SIZE;
    
    return &boot_message_queue[index];
}

void bootloader_copy_message(const bootloader_message_t* m1, bootloader_message_t* m2)
{
    *m2 = *m1;
    
    
    /*for(unsigned int i = 0; i < m1->length; i++)
    {
        m2->data[i] = m1->data[i];
    }*/
}