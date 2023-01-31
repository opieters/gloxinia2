#ifndef __CLOCK_H__
#define	__CLOCK_H__

#include <xc.h>

typedef struct {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t wday;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} clock_time_t;

#ifdef	__cplusplus
extern "C" {
#endif
    
    void clock_init(void);
    void clock_set_time(clock_time_t* time);
    void clock_start_calibration(void);
    void clock_stop_calibration(void);
    void clock_set_calibration(uint8_t value);
    void clock_get_raw_time(uint16_t* ctime);


#ifdef	__cplusplus
}
#endif

#endif

