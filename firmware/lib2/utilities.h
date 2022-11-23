#ifndef __UTILITIES_H__
#define	__UTILITIES_H__

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <uart.h>

#ifdef __DEBUG__
#define DEBUG_BUFFER_DEPTH 8
#define DEBUG_BUFFER_LENGTH 64
#define UART_DEBUG_PRINT(...) sprintf(print_buffer[print_buffer_idx], __VA_ARGS__); uart_print(print_buffer[print_buffer_idx], strlen(print_buffer[print_buffer_idx])); print_buffer_idx = (print_buffer_idx+1) % DEBUG_BUFFER_DEPTH;
#else

#define UART_DEBUG_PRINT(...)
#define DEBUG_BUFFER_LENGTH 0
#define DEBUG_BUFFER_DEPTH 0
#endif

#define __SOFWARE_MAJOR_VERSION__ 0
#define __SOFWARE_MINOR_VERSION__ 1

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))
#define ABS(x)   ((x) > 0?(x):-(x))

#define PIN_INIT(x, y)   {.port_r = &PORT##x, .tris_r = &TRIS##x, .lat_r = &LAT##x, .n = y}
#define SET_BIT(x, y)    ((*(x)) |= 1UL << (y))
#define CLEAR_BIT(x, y)  ((*(x)) &= ~(1UL << (y)))
#define CLEAR_PORT_BIT(x)  ((*((x).port_r)) &= ~(1UL << ((x).n)))
#define SET_PORT_BIT(x)    ((*((x).port_r)) |= 1UL << ((x).n))
#define TOGGLE_BIT(x, y) ((*(x)) ^= 1UL << (y))

typedef struct {
    volatile uint16_t* port_r;
    volatile uint16_t* tris_r;
    volatile uint16_t* lat_r;
    uint8_t n;
} pin_t;


#ifdef	__cplusplus
extern "C" {
#endif

    extern char print_buffer[DEBUG_BUFFER_DEPTH][DEBUG_BUFFER_LENGTH];
    extern uint8_t print_buffer_idx;


#ifdef	__cplusplus
}
#endif 

#endif

