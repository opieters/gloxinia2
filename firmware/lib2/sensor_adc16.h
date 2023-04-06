#ifndef __ADC16_H__
#define __ADC16_H__

#include <xc.h>
#include <utilities.h>
#include <spi.h>

/// @brief ADC16 ping-pong buffer length for incoming data.
#define ADC16_MAX_BUFFER_LENGTH 200

/// @brief ADC16 output buffer length for outgoing data.
#define ADC16_TX_BUFFER_LENGTH 2

/// @brief Convert ADC16 command to binary word.
#define ADC16_COMMAND_TO_BITS(CMD) (((uint16_t)(CMD & 0xf)) << 12)

/// @brief ADC16 number of calibration samples.
#define ADC16_N_CALIBRATION_SAMPLES 64

/// @brief ADC16 number of channels.
#define ADC16_N_CHANNELS 8

/// @brief ADC16 command codes.
typedef enum
{
    ADC16_SELECT_CH0 = 0x0,       ///< Select channel 0.
    ADC16_SELECT_CH1H1 = 0x1,     ///< Select channel 1.
    ADC16_SELECT_CH2 = 0x2,       ///< Select channel 2.
    ADC16_SELECT_CH3 = 0x3,       ///< Select channel 3.
    ADC16_SELECT_CH4_CH4 = 0x4,   ///< Select channel 4.
    ADC16_SELECT_CH5CT_CH5 = 0x5, ///< Select channel 5.
    ADC16_SELECT_CH6 = 0x6,       ///< Select channel 6.
    ADC16_SELECT_CH7H7 = 0x7,     ///< Select channel 7.
    ADC16_WAKE_UP = 0xB,          ///< Wake up from sleep mode.
    ADC16_READ_CFR = 0xC,         ///< Read configuration register.
    ADC16_READ_DATA = 0xD,        ///< Read data register.
    ADC16_WRITE_CFR = 0xE,        ///< Write configuration register.
    ADC16_DEFAULT_MODE = 0xF,     ///< Default mode.
} adc16_command_t;

/// @brief ADC16 channel selection enumeration.
typedef enum
{
    ADC16_CH0 = 0, ///< Channel 0.
    ADC16_CH1 = 1, ///< Channel 1.
    ADC16_CH2 = 2, ///< Channel 2.
    ADC16_CH3 = 3, ///< Channel 3.
    ADC16_CH4 = 4, ///< Channel 4.
    ADC16_CH5 = 5, ///< Channel 5.
    ADC16_CH6 = 6, ///< Channel 6.
    ADC16_CH7 = 7, ///< Channel 7.
} adc16_channel_t;

/// @brief ADC16 channel selection mode enumeration.
typedef enum
{
    ADC16_CHANNEL_SELECT_MODE_MANUAL = 0, ///< Manual channel selection.
    ADC16_CHANNEL_SELECT_MODE_AUTO = 1,   ///< Auto channel selection.
} adc16_channel_select_mode_t;

/// @brief ADC16 clock source enumeration.
typedef enum
{
    ADC16_CONVERSION_CLOCK_SOURCE_SCLK = 0,    ///< Clock source is SCLK (derived from SPI).
    ADC16_CONVERSION_CLOCK_SOURCE_INTERNAL = 1 ///< Clock source is internal.
} adc16_conversion_clock_source_t;

/// @brief ADC16 trigger select enumeration.
typedef enum
{
    ADC16_TRIGGER_SELECT_AUTO = 0,  ///< Auto trigger.
    ADC16_TRIGGER_SELECT_MANUAL = 1 ///< Manual trigger.
} adc16_trigger_select_t;

/// @brief ADC16 auto trigger rate enumeration.
typedef enum
{
    ADC16_SAMPLE_RATE_AUTO_TRIGGER_500KSPS = 0, ///< Sample rate is 500 kSPS.
    ADC16_SAMPLE_RATE_AUTO_TRIGGER_250KSPS = 1  ///< Sample rate is 250 kSPS.
} adc16_sample_rate_auto_trigger_t;

/// @brief ADC16 pin 10 (INT/EOC/CDI) polarity select enumeration.
typedef enum
{
    ADC16_PIN10_POLARITY_SELECT_ACTIVE_HIGH = 0, ///< Pin 10 (INT/EOC/CDI) polarity is active high.
    ADC16_PIN10_POLARITY_SELECT_ACTIVE_LOW = 1,  ///< Pin 10 (INT/EOC/CDI) polarity is active low.
} adc16_pin10_polarity_select_t;

/// @brief ADC16 pin 10 (INT/EOC/CDI) output select enumeration.
typedef enum
{
    ADC16_PIN10_OUTPUT_INT = 0, ///< Pin 10 (INT/EOC/CDI) output is INT.
    ADC16_PIN10_OUTPUT_EOC = 1, ///< Pin 10 (INT/EOC/CDI) output is EOC.
} adc16_pin10_output_t;

/// @brief ADC16 pin 10 (INT/EOC/CDI) I/O select enumeration.
typedef enum
{
    ADC16_PIN10_IO_SELECT_CDI = 0,     ///< Pin 10 (INT/EOC/CDI) I/O is CDI (input).
    ADC16_PIN10_IO_SELECT_EOC_INT = 1, ///< Pin 10 (INT/EOC/CDI) I/O is EOC/INT (output).
} adc16_pin10_io_select_t;

/// @brief ADC16 auto nap powerdown enumeration.
typedef enum
{
    ADC16_AUTO_NAP_POWERDOWN_ENABLE = 0,  ///< Auto nap powerdown is enabled.
    ADC16_AUTO_NAP_POWERDOWN_DISABLE = 1, ///< Auto nap powerdown is disabled.
} adc16_auto_nap_powerdown_t;

/// @brief ADC16 nap powerdown enumeration.
typedef enum
{
    ADC16_NAP_POWERDOWN_ENABLE = 0,  ///< Nap powerdown is enabled.
    ADC16_NAP_POWERDOWN_DISABLE = 1, ///< Nap powerdown is disabled.
} adc16_nap_powerdown_t;

/// @brief ADC16 deep powerdown enumeration.
typedef enum
{
    ADC16_DEEP_POWERDOWN_ENABLE = 0,  ///< Deep powerdown is enabled.
    ADC16_DEEP_POWERDOWN_DISABLE = 1, ///< Deep powerdown is disabled.
} adc16_deep_powerdown_t;

/// @brief ADC16 tag output enumeration.
typedef enum
{
    ADC16_TAG_OUTPUT_DISABLE = 0, ///< Tag output is disabled.
    ADC16_TAG_OUTPUT_ENABLE = 1,  ///< Tag output is enabled.
} adc16_tag_output_t;

/// @brief ADC16 software reset enumeration.
typedef enum
{
    ADC16_SOFTWARE_RESET = 0,   ///< Software reset.
    ADC16_NORMAL_OPERATION = 1, ///< Normal operation.
} adc16_software_reset_t;

/// @brief ADC16 status enumeration.
typedef enum
{
    ADC16_STATUS_ON,   ///< ADC16 is on.
    ADC16_STATUS_IDLE, ///< ADC16 is idle.
    ADC16_STATUS_OFF,  ///< ADC16 is off.
} adc16_status_t;

/**
 * @brief ADC16 configuration structure.
 *
 * This structure holds the configuration settings for the ADC16 peripheral.
 *
 * @param channel_select Channel selection mode.
 * @param conversion_clock_source Conversion clock source.
 * @param trigger_select Trigger selection.
 * @param auto_trigger_rate Auto trigger rate.
 * @param pin10_polarity Pin 10 (INT/EOC/CDI) polarity selection.
 * @param pin10_output Pin 10 (INT/EOC/CDI) output selection.
 * @param pin10_io Pin 10 (INT/EOC/CDI) I/O selection.
 * @param auto_nap Auto nap powerdown status.
 * @param nap_powerdown Nap powerdown status.
 * @param deep_powerdown Deep powerdown status.
 * @param tag_output Tag outputs.
 * @param sw_reset Software reset status.
 * @param channel (First) selected channel.
 * @param status Overall device status.
 * @param sample_frequency Sample frequency.
 * @param spi_module SPI module to use.
 * @param adc16_buffer_size buffer size for DMA transfers from ADC16. Should be smaller than ADC16_MAX_BUFFER_LENGTH.
 * @param rst_pin Reset pin.
 * @param cs_pin Chip select pin.
 * @param convst_pin Conversion start pin.
 * @param channel_offset Offset from ADC calibration.
 * @param rx_callback Callback function when DMA transfer is complete.
 */
typedef struct
{
    adc16_channel_select_mode_t channel_select;
    adc16_conversion_clock_source_t conversion_clock_source;
    adc16_trigger_select_t trigger_select;
    adc16_sample_rate_auto_trigger_t auto_trigger_rate;
    adc16_pin10_polarity_select_t pin10_polarity;
    adc16_pin10_output_t pin10_output;
    adc16_pin10_io_select_t pin10_io;
    adc16_auto_nap_powerdown_t auto_nap;
    adc16_nap_powerdown_t nap_powerdown;
    adc16_deep_powerdown_t deep_powerdown;
    adc16_tag_output_t tag_output;
    adc16_software_reset_t sw_reset;
    adc16_channel_t channel;
    adc16_status_t status;
    uint32_t sample_frequency;
    spi_module_selector_t spi_module;
    uint16_t adc16_buffer_size;
    const pin_t rst_pin;
    const pin_t cs_pin;
    const pin_t conv_pin;
    uint16_t channel_offset[ADC16_N_CHANNELS];
    void (*rx_callback)(void);
} adc16_config_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /// @brief ADC16 transmit buffer.
    extern unsigned int adc16_tx_buffer[ADC16_TX_BUFFER_LENGTH] __attribute__((space(dma), eds));

    /// @brief ADC16 receive buffers.
    extern unsigned int adc16_rx_buffer_a[ADC16_MAX_BUFFER_LENGTH] __attribute__((space(dma), eds));
    extern unsigned int adc16_rx_buffer_b[ADC16_MAX_BUFFER_LENGTH] __attribute__((space(dma), eds));

    /**
     * @brief Initialisation function for ADC.
     *
     * @details Initialises the ADC to operate in 16-bit mode.
     *
     *
     * @param config ADC16 configuration structure.
     */
    void adc16_init(adc16_config_t *config);

    /**
     * @brief Fast initialisation function of ADC.
     *
     * @details Initialises the ADC to operate in 16-bit mode. This function is
     * faster than adc16_init() but does not read each of the channels and
     * configuration to verify correct initialisation. It can be used after
     * calling adc16_init() to reinitialise the ADC.
     *
     * @param config ADC16 configuration structure.
     */
    void adc16_init_fast(adc16_config_t *config);

    /**
     * @brief Update function for ADC.
     *
     * @param config ADC16 configuration structure.
     */
    void adc16_update(adc16_config_t *config);

    /**
     * @brief Start ADC.
     *
     * @param config ADC16 configuration structure.
     */
    void adc16_start(adc16_config_t *config);

    /**
     * @brief Stop ADC.
     *
     * @param config ADC16 configuration structure.
     */
    void adc16_stop(adc16_config_t *config);

    /// @brief ADC16 dummy callback used in the default initalisation.
    void adc16_callback_dummy(void);

    /**
     * @brief Initialises SPI module when ADC16 starts operating.
     *
     * @details Disables SPI interrupt and clear any detected overflow to insure propper operation.
     */
    void adc16_spi1_init(void);
    void adc16_spi2_init(void);
    void adc16_spi3_init(void);

    /**
     * @brief Reads a single channel from the ADC.
     *
     * @details The channel is selected by the channel_select parameter in the
     * configuration structure. The channel is read and the result is returned.
     *
     * @return ADC reading.
     */
    uint16_t adc16_read_channel(adc16_config_t *config);

    /**
     * @brief Runs calibration of ADC.
     *
     * @details The ADC is calibrated using the reference voltage. The reference
     * voltage is selected by the reference parameter. The calibration offset is
     * stored in the channel_offset parameter in the configuration structure.
     *
     * The reference voltage is automatically substracted from the conversion 
     * result.
     *
     * @param config ADC16 configuration structure.
     * @param ref Reference voltage.
     * @return ADC reading.
     */
    uint16_t adc16_run_calibration(adc16_config_t *config, const uint16_t ref);

    /**
     * @brief Runs variance calibration of ADC.
     * 
     * @details Based on ADC16_N_CALIBRATION_SAMPLES, the ADC performs a number 
     * of conversions and automatically calculates the variation thereof. The 
     * minimum and maximum values are stored in the min_value and max_value, the
     * mean is also calculated.
     * 
     * @param config ADC16 configuration structure.
     * @param min_value Minimum value.
     * @param max_value Maximum value.
     * @param mean Mean value.
    */
    void adc16_run_max_var(adc16_config_t *config, uint16_t *const max_value,
                           uint16_t *const min_value, uint16_t *const mean);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ADC16_H__ */
