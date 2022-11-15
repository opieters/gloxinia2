#include <xc.h>
#include <utilities.h>
#include <uart.h>

void __attribute__((__interrupt__, no_auto_psv)) _OscillatorFail(void){
        INTCON1bits.OSCFAIL = 0;        //Clear the trap flag
        
        // make sure I2C releases bus
        _T2IF = 1;
        
        UART_DEBUG_PRINT("Oscillator failure.");
        
        set_error_led();
        while (1){
            UART_DEBUG_PRINT(".");
        }
}

void __attribute__((__interrupt__, no_auto_psv)) _AddressError(void){
        INTCON1bits.ADDRERR = 0;        //Clear the trap flag
        
        // make sure I2C releases bus
        _T2IF = 1;
        
        UART_DEBUG_PRINT("Address error.");
        
        set_error_led();
        while (1){
            UART_DEBUG_PRINT(".");
        }
}

void __attribute__((__interrupt__, no_auto_psv)) _StackError(void){
        INTCON1bits.STKERR = 0;         //Clear the trap flag
        
        // make sure I2C releases bus
        _T2IF = 1;
        
        UART_DEBUG_PRINT("Stack error.");
        
        set_error_led();
        while (1){
            UART_DEBUG_PRINT(".");
        }
}

void __attribute__((__interrupt__, no_auto_psv)) _MathError(void){
        INTCON1bits.MATHERR = 0;        //Clear the trap flag
        
        // make sure I2C releases bus
        _T2IF = 1;
        
        UART_DEBUG_PRINT("Math error.");
        
        set_error_led();
        while (1){
            UART_DEBUG_PRINT(".");
        }
}


void __attribute__((__interrupt__, no_auto_psv)) _DMACError(void){
		
        INTCON1bits.DMACERR = 0;        //Clear the trap flag
        
        // make sure I2C releases bus
        _T2IF = 1;
        
        UART_DEBUG_PRINT("DMAC error.");
        
        set_error_led();
        while (1){
            UART_DEBUG_PRINT(".");
        }
}

