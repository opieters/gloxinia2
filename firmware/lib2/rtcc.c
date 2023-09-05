#include "rtcc.h"

void clock_init(void)
{
    NVMKEY = 0x55;
    NVMKEY = 0xaa;
    RCFGCALbits.RTCWREN = 0;    //disable the RTCC peripheral       

    /* Configure the alarm settings*/
    ALCFGRPTbits.CHIME = 0;     // no rolloever of the repeat count
    ALCFGRPTbits.AMASK = 0;     // alarm mask configuration bits
    ALCFGRPTbits.ARPT = 0;      // alarm repeat counter value bits
    RCFGCALbits.RTCOE = 0;      // disable RTCC output

    /* Load the initial values to the RTCC value registers*/
    RCFGCALbits.RTCPTR = 3;     // point to year register
    clock_time_t base_time = {
            .year = 23,
            .month = 1,
            .day = 1,
            .wday = 6,
            .hour = 0,
            .minute = 0,
            .second = 0
    };
    
    clock_set_time(&base_time);

    RCFGCALbits.RTCEN = 1;      //enable RTCC peripheral
    RCFGCALbits.RTCWREN = 0;    //lock the RTCC value registers

    /* Enable the RTCC interrupt*/
    _RTCIF = 0;                 //clear the RTCC interrupt flag
    _RTCIE = 1;                 //enable the RTCC interrupt
}

void clock_set_time(clock_time_t* time)
{
    uint16_t bcd_values;
    
    // enable writes to configuration
    NVMKEY = 0x55;
    NVMKEY = 0xaa;
    RCFGCALbits.RTCWREN = 1;
    RCFGCALbits.RTCEN = 0;
    
    // write year
    RCFGCALbits.RTCPTR = 0b11;
    bcd_values = ((0xf & (time->year / 10)) << 4) | (0xf & (time->year % 10));
    RTCVAL = bcd_values;
    bcd_values = ((((0xf & (time->month / 10)) << 4) | (0xf & (time->month % 10))) << 8) | (0xf & ((time->day / 10) << 4)) | (0xf & (time->day % 10));
    RTCVAL = ((time->wday & 0xf) << 8) | ((0xf & (time->hour / 10)) << 4) | (0xf & (time->hour % 10));
    RTCVAL = ((((0xf & (time->minute / 10)) << 4) | (0xf & (time->minute % 10))) << 8) | (0xf & ((time->second / 10) << 4)) | (0xf & (time->second % 10));
    
     // enable RTCC
    RCFGCALbits.RTCWREN = 0;
}

void clock_correct_time(clock_time_t* time)
{
    uint16_t bcd_values;
    
    // enable writes to configuration
    NVMKEY = 0x55;
    NVMKEY = 0xaa;
    RCFGCALbits.RTCWREN = 1;
    RCFGCALbits.RTCEN = 0;
    
    
    RCFGCALbits.RTCPTR = 0b11;
    // write year
    bcd_values = ((0xf & (time->year / 10)) << 4) | (0xf & (time->year % 10));
    RTCVAL = bcd_values;
    // write month and day
    bcd_values = ((((0xf & (time->month / 10)) << 4) | (0xf & (time->month % 10))) << 8) | (0xf & ((time->day / 10) << 4)) | (0xf & (time->day % 10));
    RTCVAL = bcd_values;
    // write weekday and hour
    bcd_values = ((time->wday & 0xf) << 8) | ((0xf & (time->hour / 10)) << 4) | (0xf & (time->hour % 10));
    RTCVAL = bcd_values;
    // write minute and second
    bcd_values = ((((0xf & (time->minute / 10)) << 4) | (0xf & (time->minute % 10))) << 8) | (0xf & ((time->second / 10) << 4)) | (0xf & (time->second % 10));
    RTCVAL = bcd_values;
    
    RCFGCALbits.RTCWREN = 0;
}

void clock_start_calibration(void) {
    // enable writes to configuration
    NVMKEY = 0x55;
    NVMKEY = 0xaa;
    RCFGCALbits.RTCWREN = 1;
    
    PADCFG1bits.RTSECSEL = 1; // connect RTCC pin to seconds pulse
    RCFGCALbits.RTCOE = 1; // enable RTCC pin
    
    // enable RTCC
    RCFGCALbits.RTCWREN = 0;
}

void clock_set_calibration(uint8_t value)
{
    // enable writes to configuration
    NVMKEY = 0x55;
    NVMKEY = 0xaa;
    RCFGCALbits.RTCWREN = 1;
    
    RCFGCALbits.CAL = (value & 0x7f);
    
    RCFGCALbits.RTCWREN = 0;
}

void clock_stop_calibration(void)
{
    // enable writes to configuration
    NVMKEY = 0x55;
    NVMKEY = 0xaa;
    RCFGCALbits.RTCWREN = 1;
    
    RCFGCALbits.RTCOE = 0; // enable RTCC pin
    
    // enable RTCC
    RCFGCALbits.RTCWREN = 0;
}

void __attribute__((interrupt, no_auto_psv)) _RTCCInterrupt(void) {
    IFS3bits.RTCIF = 0;
}

void clock_get_raw_time(uint16_t* ctime)
{
    uint8_t gie_value = _GIE;
    _GIE = 0;
    RCFGCALbits.RTCPTR = 0b11;
    ctime[0] = RTCVAL;
    ctime[1] = RTCVAL;
    ctime[2] = RTCVAL;
    ctime[3] = RTCVAL;
    _GIE = gie_value;
}

void clock_get_time(clock_time_t* time)
{
    uint16_t data[4];
    clock_get_raw_time(data);
    
    time->year = (data[0] & 0xf) + ((data[0] >> 4) & 0xf)*10;
    time->month = ((data[1] >> 8) & 0xf) + (data[1] >> 12)*10;
    time->day = (data[1] & 0xf) + ((data[1] >> 4) & 0x3)*10;
    time->wday = (data[2] >> 8) & 0x7;
    time->hour = (data[2] & 0xf) + ((data[2] >> 4) & 0x3)*10;
    time->minute = ((data[3] >> 8) & 0xf) + (data[3] >> 12)*10;
    time->second = (data[3] & 0xf) + ((data[3] >> 4) & 0x7)*10;
}