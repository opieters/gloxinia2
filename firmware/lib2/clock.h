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
    void clock_set_config(clock_time_t* time);
    uint8_t clock_sync(clock_time_t* time);

#ifdef	__cplusplus
}
#endif

#endif

