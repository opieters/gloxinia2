#ifndef __PGA_H__
#define __PGA_H__

#include <xc.h>
#include <utilities.h>
#include <spi.h>

/// @brief PGA channel selection enumeration
typedef enum
{
    PGA_MUX_VCAL_CH0 = 0b0000, ///< Channel 0 / calibration voltage
    PGA_MUX_CH1 = 0b0001,      ///< Channel 1
    PGA_CAL1 = 0b1100,         ///< Internal calibration reference 1
    PGA_CAL2 = 0b1101,         ///< Internal calibration reference 2
    PGA_CAL3 = 0b1110,         ///< Internal calibration reference 3
    PGA_CAL4 = 0b1111,         ///< Internal calibration reference 4
} pga_mux_channel_t;

/// @brief PGA gain setting enumeration
typedef enum
{
    PGA_GAIN_1 = 0b0000,   ///< Gain = 1
    PGA_GAIN_2 = 0b0001,   ///< Gain = 2
    PGA_GAIN_5 = 0b0010,   ///< Gain = 5
    PGA_GAIN_10 = 0b0011,  ///< Gain = 10
    PGA_GAIN_20 = 0b0100,  ///< Gain = 20
    PGA_GAIN_50 = 0b0101,  ///< Gain = 50
    PGA_GAIN_100 = 0b0110, ///< Gain = 100
    PGA_GAIN_200 = 0b0111, ///< Gain = 200
} pga_gain_t;

///@ brief PGA status enumeration
typedef enum
{
    PGA_STATUS_OFF,     ///< PGA off
    PGA_STATUS_ON,      ///< PGA on
    PGA_STATUS_ERROR,   ///< PGA configuration or communication error
} pga_status_t;

/**
 * @brief PGA configuration structure
 *
 * @param channel Selected PGA channel
 * @param gain Selected PGA gain setting
 * @param status PGA status
 * @param cs_pin nCS pin for SPI communication
 * @param spi_message_handler Callback to process incoming SPI data
 */
typedef struct
{
    pga_mux_channel_t channel;
    pga_gain_t gain;
    pga_status_t status;
    pin_t cs_pin;
    void (*spi_message_handler)(spi_message_t *m);
} pga_config_t;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize PGA
    */
    void pga_init(pga_config_t *config);
    void pga_update_status(pga_config_t *config, pga_status_t status);

#ifdef __cplusplus
}
#endif

#endif
