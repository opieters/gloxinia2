#include "clock.h"

void clock_init(void)
{
    // TODO??
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