#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <uart.h>

/// @brief Macro to log debug messages, automatically deactives logging of __DEBUG__ is not defined.
#ifdef __DEBUG__
#define DEBUG_BUFFER_DEPTH 8
#define DEBUG_BUFFER_LENGTH 64
 #define UART_DEBUG_PRINT(X, ...)                                                           \
    sprintf(print_buffer[print_buffer_idx], X, ##__VA_ARGS__);                               \
    uart_print(print_buffer[print_buffer_idx], strlen(print_buffer[print_buffer_idx])); \
    print_buffer_idx = (print_buffer_idx + 1) % DEBUG_BUFFER_DEPTH;
#else

#define UART_DEBUG_PRINT(...)
#define DEBUG_BUFFER_LENGTH 0
#define DEBUG_BUFFER_DEPTH 0
#endif

/// @brief Software version
#define __SOFWARE_MAJOR_VERSION__ 0 ///< Software major version
#define __SOFWARE_MINOR_VERSION__ 1 ///< Software minor version

/// @brief Macro to get length of array (assumes that the array is not a pointer)
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

/// @brief Macro to get the minimum of two values
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/// @brief Macro to get the maximum of two values
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/// @brief Macro to get the absolute value of two values
#define ABS(x) ((x) > 0 ? (x) : -(x))

/// @brief Macro to initialise a pin
#define PIN_INIT(x, y)                                                   \
    {                                                                    \
        .port_r = &PORT##x, .tris_r = &TRIS##x, .lat_r = &LAT##x, .n = y \
    }

/// @brief Macro to set a bit in a register
#define SET_BIT(x, y) ((*(x)) |= 1UL << (y))

/// @brief Macro to clear a bit in a register
#define CLEAR_BIT(x, y) ((*(x)) &= ~(1UL << (y)))

/// @brief Macro to set a pin low
#define CLEAR_PORT_BIT(x) ((*((x).port_r)) &= ~(1UL << ((x).n)))

/// @brief Macro to set a pin high
#define SET_PORT_BIT(x) ((*((x).port_r)) |= 1UL << ((x).n))

/// @brief Macro to toggle a pin
#define TOGGLE_BIT(x, y) ((*(x)) ^= 1UL << (y))

/// @brief Macro to extract bit value (and shift to lowest position)
#define GET_BIT(x, y) (((*(x)) >> (y)) & 1UL)

/**
 * @brief Pin structure
 * 
 * @details Structure to hold the port, tris and lat registers for a pin.
 * 
 * @param port_r Pointer to the port register
 * @param tris_r Pointer to the tris register
 * @param lat_r Pointer to the lat register
 * @param n Pin number
*/
typedef struct
{
    volatile uint16_t *port_r;
    volatile uint16_t *tris_r;
    volatile uint16_t *lat_r;
    uint8_t n;
} pin_t;

#ifdef __cplusplus
extern "C"
{
#endif

    /// @brief Print buffer
    extern char print_buffer[DEBUG_BUFFER_DEPTH][DEBUG_BUFFER_LENGTH];

    /// @brief Print buffer index
    extern uint8_t print_buffer_idx;

    /**
     * @brief Clear a buffer
     * 
     * @details Clears a buffer by setting all bytes to 0.
     * 
     * @param buffer Pointer to the buffer
     * @param length Length of the buffer
     */
    void clear_buffer(uint8_t *buffer, size_t length);

#ifdef __cplusplus
}
#endif

#endif
