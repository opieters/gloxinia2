#include <xc.h>
#include <stdio.h>
#include "pin_manager.h"

void pins_init (void)
{
    /****************************************************************************
     * Setting the Output Latch SFR(s)
     ***************************************************************************/
    LATB = 0x0000;
    LATC = 0x0000;
    LATD = 0x0000;
    LATE = 0x0080;
    LATF = 0x0000;
    LATG = 0x0000;

    /****************************************************************************
     * Setting the GPIO Direction SFR(s)
     ***************************************************************************/
    TRISB = 0xFFFF;
    TRISC = 0x7000;
    TRISD = 0x0FFF;
    TRISE = 0x005F;
    TRISF = 0x007F;
    TRISG = 0x03CC;

    /****************************************************************************
     * Setting the Weak Pull Up and Weak Pull Down SFR(s)
     ***************************************************************************/
    CNPDB = 0x0000;
    CNPDC = 0x0000;
    CNPDD = 0x0000;
    CNPDE = 0x0000;
    CNPDF = 0x0000;
    CNPDG = 0x0000;
    CNPUB = 0x0000;
    CNPUC = 0x0000;
    CNPUD = 0x0000;
    CNPUE = 0x0000;
    CNPUF = 0x0000;
    CNPUG = 0x0000;

    /****************************************************************************
     * Setting the Open Drain SFR(s)
     ***************************************************************************/
    ODCD = 0x0000;
    ODCE = 0x0000;
    ODCF = 0x0000;
    ODCG = 0x0000;

    /****************************************************************************
     * Setting the Analog/Digital Configuration SFR(s)
     ***************************************************************************/
    ANSELB = 0xFF3F;
    ANSELC = 0x6000;
    ANSELD = 0x00C0;
    ANSELE = 0x001F;
    ANSELG = 0x0340;
    
    /****************************************************************************
     * Set the PPS
     ***************************************************************************/
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS

    RPINR18bits.U1RXR = 0x0077;    //RG7->UART1:U1RX
    RPOR6bits.RP85R = 0x0002;    //RE5->UART1:U1RTS
    RPOR6bits.RP87R = 0x0001;    //RE7->UART1:U1TX
    RPINR18bits.U1CTSR = 0x0056;    //RE6->UART1:U1CTS

    __builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
}

void pins_reset (void)
{
    /****************************************************************************
     * Setting the Output Latch SFR(s)
     ***************************************************************************/
    LATB = 0x0000;
    LATC = 0x0000;
    LATD = 0x0000;
    LATE = 0x0080;
    LATF = 0x0000;
    LATG = 0x0000;

    /****************************************************************************
     * Setting the GPIO Direction SFR(s)
     ***************************************************************************/
    TRISB = 0xFFFF;
    TRISC = 0xFFFF;
    TRISD = 0xFFFF;
    TRISE = 0xFFFF;
    TRISF = 0xFFFF;
    TRISG = 0xFFFF;




    /****************************************************************************
     * Setting the Analog/Digital Configuration SFR(s)
     ***************************************************************************/
    ANSELB = 0xFFFF;
    ANSELC = 0xFFFF;
    ANSELD = 0xFFFF;
    ANSELE = 0xFFFF;
    ANSELG = 0xFFFF;
    
    /****************************************************************************
     * Set the PPS
     ***************************************************************************/
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS

    RPINR18bits.U1RXR = 0x0000; 
    RPOR6bits.RP85R = 0x0000;  
    RPOR6bits.RP87R = 0x0000;  
    RPINR18bits.U1CTSR = 0x0000;    

    __builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
}