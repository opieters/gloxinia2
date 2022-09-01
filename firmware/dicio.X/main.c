#include <xc.h>
#include <uart.h>
#include "dicio.h"
#include <utilities.h>
#include "sensors.h"
#include "actuators.h"
#include "uart_processing.h"

#pragma config GWRP = OFF                       // General Segment Write-Protect bit (General Segment may be written)
#pragma config GSS = OFF                        // General Segment Code-Protect bit (General Segment Code protect is disabled)
#pragma config GSSK = OFF                       // General Segment Key bits (General Segment Write Protection and Code Protection is Disabled)

// FOSCSEL
#pragma config FNOSC = FRC                      // Initial Oscillator Source Selection Bits (Internal Fast RC (FRC))
#pragma config IESO = OFF                       // Two-speed Oscillator Start-up Enable bit (Start up with user-selected oscillator source)

// FOSC
#pragma config POSCMD = XT                    // Primary Oscillator Mode Select bits (Primary Oscillator disabled)
#pragma config OSCIOFNC = OFF                   // OSC2 Pin Function bit (OSC2 is clock output)
#pragma config IOL1WAY = OFF                    // Peripheral pin select configuration (Allow multiple reconfigurations)
#pragma config FCKSM = CSECMD                   // Clock Switching Mode bits (Clock switching is enabled,Fail-safe Clock Monitor is disabled)

// FWDT
#pragma config WDTPOST = PS32768                // Watchdog Timer Postscaler Bits (1:32,768)
#pragma config WDTPRE = PR128                   // Watchdog Timer Prescaler bit (1:128)
#pragma config PLLKEN = ON                      // PLL Lock Wait Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.)
#pragma config WINDIS = OFF                     // Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF                     // Watchdog Timer Enable bit (Watchdog timer enabled/disabled by user software)

// FPOR
#pragma config FPWRT = PWR128                   // Power-on Reset Timer Value Select bits (128ms)
#pragma config BOREN = ON                       // Brown-out Reset (BOR) Detection Enable bit (BOR is enabled)
#pragma config ALTI2C1 = OFF                    // Alternate I2C pins for I2C1 (ASDA1/ASCK1 pins are selected as the I/O pins for I2C1)

// FICD
#pragma config ICS = PGD1                       // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
#pragma config RSTPRI = PF                      // Reset Target Vector Select bit (Device will obtain reset instruction from Primary flash)
#pragma config JTAGEN = OFF                     // JTAG Enable bit (JTAG is disabled)

// FAS
#pragma config AWRP = OFF                       // Auxiliary Segment Write-protect bit (Auxiliary program memory is not write-protected)
#pragma config APL = OFF                        // Auxiliary Segment Code-protect bit (Aux Flash Code protect is disabled)
#pragma config APLK = OFF                       // Auxiliary Segment Key bits (Aux Flash Write Protection and Code Protection is Disabled)

extern volatile uint8_t start_sensors_init;
uint16_t i;

typedef enum {
    DICIO_STATE_INIT_DATA,
    DICIO_STATE_INIT_ALL,
    DICIO_STATE_WAIT_START,
    DICIO_STATE_RUNNING,
            DICIO_STATE_START,
    DICIO_STATE_STOP,
} dicio_state_t;

dicio_state_t main_control_state = DICIO_STATE_INIT_ALL;

int main ( void ){

    // Configure Oscillator to operate the device at 64MHz with internal 
    // crystal
    // Fosc= Fin*M/(N1*N2), Fcy=Fosc/2
    // Fosc= 7.37M*70/(2*2)=128Mhz for 8M input clock
    PLLFBD = 62;                                // M=70
    CLKDIVbits.PLLPOST = 0;                     // N1=2
    CLKDIVbits.PLLPRE = 0;                      // N2=2
    OSCTUN = 0;                                 // Tune FRC oscillator, if FRC is used

    // Disable Watch Dog Timer
    RCONbits.SWDTEN = 0;

    // Clock switch to incorporate PLL
    __builtin_write_OSCCONH( 0x03 );            // Initiate Clock Switch to

    // FRC with PLL (NOSC=0b001)
    __builtin_write_OSCCONL( OSCCON | 0x01 );  // Start clock switching
    while( OSCCONbits.COSC != 0b011 );

    // Wait for Clock switch to occur
    // Wait for PLL to lock
    while( OSCCONbits.LOCK != 1 );
    
#ifdef ENABLE_DEBUG
    uart_init(500000);
    uart_set_callback(uart_rx_command_cb_dicio);
    
    uart_simple_print("Configured UART.");
#endif
    
    delay_ms(1000);
    
    while(1){
        switch(main_control_state){
            case DICIO_STATE_INIT_DATA:
                actuators_data_init();
                sensors_data_init();

                delay_ms(1000);

                dicio_init();
                main_control_state = DICIO_STATE_WAIT_START;
                break;
            case DICIO_STATE_WAIT_START:
                uart_simple_print("Waiting for start command.");
                while(!start_sensors_init){
                    dicio_send_ready_message();
                    delay_ms(1000);
                }
                main_control_state = DICIO_STATE_INIT_ALL;
                break;
            case DICIO_STATE_INIT_ALL:
                actuators_init();
                delay_ms(1000);
                sensors_init();
                main_control_state = DICIO_STATE_START;
                break;
            case DICIO_STATE_START:
                sensors_start();
                actuators_start();
                main_control_state = DICIO_STATE_RUNNING;
                break;
            case DICIO_STATE_RUNNING:
                dicio_loop();
                
                main_control_state = DICIO_STATE_STOP;
                break;
            case DICIO_STATE_STOP:
                main_control_state = DICIO_STATE_INIT_DATA;
                break;
        }
        delay_ms(1000);
    }
    
    return 0;
}