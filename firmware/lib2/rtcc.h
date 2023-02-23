#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <xc.h>

/**
 * @brief Structure for holding time information.
 *
 * @param year: year (0-99)
 * @param month: month (1-12)
 * @param day: day (1-31)
 * @param wday: day of week (0-6)
 * @param hour: hour (0-23)
 * @param minute: minute (0-59)
 * @param second: second (0-59)
 */
typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t wday;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} clock_time_t;

#ifdef __cplusplus
extern "C"
{
#endif

    /// @brief Initialize the clock.
    void clock_init(void);

    /**
     * @brief Stop the RTCC, set a new time value and resume operation
     * 
     * @param time: pointer to a clock_time_t structure containing the new time
     */
    void clock_set_time(clock_time_t *time);

    /**
     * @brief Start calibration pulses.
     * 
     * @details This function starts the calibration pulsing. The calibration 
     * pulses should have a period of 1Hz and can be used to correct the 
     * deviation of internal crystal oscillator. Pulsing occurs on the RTCC pin.
    */
    void clock_start_calibration(void);

    /// @brief Stops calibraion pulses.
    void clock_stop_calibration(void);

    /**
     * @brief Set the calibration value.
     * 
     * @param value: calibration value
    */
    void clock_set_calibration(uint8_t value);

    /**
     * @brief Get current time as stored in memory.
     * 
     * @param time: pointer to a array of four uint16_t to store the time. 
     * See device datasheet for details on the format. Interrupts are disbaled 
     * during time retrieval to ensure the time value is correct.
     */
    void clock_get_raw_time(uint16_t *ctime);

#ifdef __cplusplus
}
#endif

#endif
