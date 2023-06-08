#include <xc.h>
#include "dicio.h"
#include <event_controller.h>
#include <utilities.h>
#include <i2c.h>
#include <libpic30.h>

#pragma config GWRP = OFF           //General Segment Write-Protect bit->General Segment may be written
#pragma config GSS = OFF            //General Segment Code-Protect bit->General Segment Code protect is disabled
#pragma config GSSK = OFF           //General Segment Key bits->General Segment Write Protection and Code Protection is Disabled

// FOSCSEL
#pragma config FNOSC = FRC          //Initial Oscillator Source Selection bits->FRC
#pragma config IESO = OFF           //Two-speed Oscillator Start-up Enable bit->Start up with user-selected oscillator source

// FOSC
#pragma config POSCMD = XT          //Primary Oscillator Mode Select bits->XT Crystal Oscillator Mode
#pragma config OSCIOFNC = ON        //OSC2 Pin Function bit->OSC2 is general purpose digital I/O pin
#pragma config IOL1WAY = OFF        //Peripheral pin select configuration->Allow multiple reconfigurations
#pragma config FCKSM = CSECMD       //Clock Switching Mode bits->Clock switching is enabled,Fail-safe Clock Monitor is disabled

// FWDT
#pragma config WDTPOST = PS32768    //Watchdog Timer Postscaler bits->1:32768
#pragma config WDTPRE = PR128       //Watchdog Timer Prescaler bit->1:128
#pragma config PLLKEN = ON          //PLL Lock Wait Enable bit->Clock switch to PLL source will wait until the PLL lock signal is valid.
#pragma config WINDIS = OFF         //Watchdog Timer Window Enable bit->Watchdog Timer in Non-Window mode
#pragma config FWDTEN = OFF         //Watchdog Timer Enable bit->Watchdog timer enabled/disabled by user software

// FPOR
#pragma config FPWRT = PWR128       //Power-on Reset Timer Value Select bits->128ms
#pragma config BOREN = ON           //Brown-out Reset (BOR) Detection Enable bit->BOR is enabled
#pragma config ALTI2C1 = OFF        //Alternate I2C pins for I2C1->SDA1/SCK1 pins are selected as the I/O pins for I2C1

// FICD
#pragma config ICS = PGD1           //ICD Communication Channel Select bits->Communicate on PGEC1 and PGED1
#pragma config RSTPRI = PF          //Reset Target Vector Select bit->Device will obtain reset instruction from Primary flash
#pragma config JTAGEN = OFF         //JTAG Enable bit->JTAG is disabled

// FAS
#pragma config AWRP = OFF           //Auxiliary Segment Write-protect bit->Aux Flash may be written
#pragma config APL = OFF            //Auxiliary Segment Code-protect bit->Aux Flash Code protect is disabled
#pragma config APLK = OFF           //Auxiliary Segment Key bits->Aux Flash Write Protection and Code Protection is Disabled
static task_t task;

extern volatile uint16_t controller_address;

int main(void) 
{
    
    dicio_init();

    while (1) 
    {
        i2c_process_queue();

        if (n_queued_tasks > 0)
        {
            task = pop_queued_task();
            task.cb(task.data);
        }
    }

    return 0;
}