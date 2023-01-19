#include "clock.h"

void clock_init(void)
{
    // TODO??
}

void clock_set_config(clock_time_t* time)
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

uint8_t clock_sync(clock_time_t* time)
{
    
}

void clock_(clock_time_t* time)
{
    uint16_t bcd_values;
    
    // enable writes to configuration
    NVMKEY = 0x55;
    NVMKEY = 0xaa;
    RCFGCALbits.RTCWREN = 1;
    // write year
    RCFGCALbits.RTCPTR = 0b11;
    bcd_values = ((0xf & (time->year / 10)) << 4) | (0xf & (time->year % 10));
    RTCVAL = bcd_values;
    bcd_values = ((((0xf & (time->month / 10)) << 4) | (0xf & (time->month % 10))) << 8) | (0xf & ((time->day / 10) << 4)) | (0xf & (time->day % 10));
    RTCVAL = ((time->wday & 0xf) << 8) | ((0xf & (time->hour / 10)) << 4) | (0xf & (time->hour % 10));
    RTCVAL = ((((0xf & (time->minute / 10)) << 4) | (0xf & (time->minute % 10))) << 8) | (0xf & ((time->second / 10) << 4)) | (0xf & (time->second % 10));
    
    RCFGCALbits.RTCEN = 1; // enable RTCC
    RCFGCALbits.RTCWREN = 0;
}

