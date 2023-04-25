#include <xc.h>
#include <sensor_adc16.h>
#include <utilities.h>
#include <dsp.h>
#include <fir_common.h>
#include <spi.h>
#include <uart.h>
#include <stdio.h>
#include <string.h>
#include <libpic30.h>

unsigned int adc16_tx_buffer[ADC16_TX_BUFFER_LENGTH] __attribute__((space(dma), eds));
unsigned int adc16_rx_buffer_a[ADC16_MAX_BUFFER_LENGTH] __attribute__((space(dma), eds, address(0xD9C0)));
unsigned int adc16_rx_buffer_b[ADC16_MAX_BUFFER_LENGTH] __attribute__((space(dma), eds, address(0xD830)));

void (*rx_callback)(void) = adc16_callback_dummy;

void adc16_callback_dummy(void)
{
}

uint16_t adc16_parse_cfr_write(sensor_adc16_config_t *config)
{
    uint16_t word = 0xE000;

    word |= (config->channel_select << 11) | (config->conversion_clock_source << 10) | (config->trigger_select << 9) | (config->auto_trigger_rate << 8) | (config->pin10_polarity << 7) | (config->pin10_output << 6) | (config->pin10_io << 5) | (config->auto_nap << 4) | (config->nap_powerdown << 3) | (config->deep_powerdown << 2) | (config->tag_output << 1) | (config->sw_reset);

    return word;
}

void adc16_update(sensor_adc16_config_t *config)
{
    spi_message_t m;
    uint16_t write_data[1];
    uint16_t read_data[1];
    void (*send_spi_message)(spi_message_t * m);

    send_spi_message = spi_get_handler(config->spi_module);

    // update callback
    rx_callback = config->rx_callback;

    // disable sampling
    OC15CON1bits.OCM = 0b000;

    // wake ADC
    write_data[0] = 0xB000;
    spi_init_message(&m, write_data, read_data, 1, &config->cs_pin);
    send_spi_message(&m);

    // select actual channel in config
    write_data[0] = ADC16_COMMAND_TO_BITS(config->channel);
    spi_init_message(&m, write_data, read_data, 1, &config->cs_pin);
    send_spi_message(&m);

    // update status
    write_data[0] = adc16_parse_cfr_write(config);
    spi_init_message(&m, write_data, read_data, 1, &config->cs_pin);
    send_spi_message(&m);

    if (config->deep_powerdown == ADC16_DEEP_POWERDOWN_ENABLE)
    {
        config->status = ADC16_STATUS_OFF;
    }
    else
    {
        config->status = ADC16_STATUS_IDLE;
    }

    OC15CON1bits.OCM = 0b000;                      // Disable Output Compare Module
    OC15R = (FCY / config->sample_frequency) - 5;  // Write the duty cycle for the PWM pulse
    OC15RS = (FCY / config->sample_frequency) - 1; // Write the PWM frequency

    // configure the CS pin
    OC4CON1bits.OCM = 0b000;                       // Disable Output Compare Module
    OC4R = (FCY / config->sample_frequency) - 120; // Write the duty cycle for the PWM pulse
    OC4RS = (FCY / config->sample_frequency) - 1;  // frequency
}

void adc16_init(sensor_adc16_config_t *config)
{
    uint16_t i, eds_read;
    spi_message_t m;
    uint16_t write_data[2];
    uint16_t read_data[2];
    read_data[0] = 0x0;
    void (*send_spi_message)(spi_message_t * m);
    adc16_channel_select_mode_t selection_config;
    adc16_channel_t original_channel = config->channel;

    // configure reset pin and set high)
    CLEAR_BIT(config->rst_pin.tris_r, config->rst_pin.n);
    SET_BIT(config->rst_pin.lat_r, config->rst_pin.n);

    // hardware reset of ADC
    __delay_us(100);
    CLEAR_BIT(config->rst_pin.lat_r, config->rst_pin.n);
    __delay_us(100);
    SET_BIT(config->rst_pin.lat_r, config->rst_pin.n);

    __delay_ms(5);

    // force conversion to prevent unresponsive ADC
    CLEAR_BIT(config->conv_pin.lat_r, config->conv_pin.n);
    __delay_us(10);
    SET_BIT(config->conv_pin.lat_r, config->conv_pin.n);
    __delay_us(10);

    // configure DMA4 to write data over SPI
    _DMA4IF = 0;
    _DMA4IE = 0;

    DMA4CONbits.SIZE = 0;  // word data transfer
    DMA4CONbits.HALF = 0;  // interrupt when all data is transfered
    DMA4CONbits.NULLW = 0; // normal operation
    DMA4CONbits.DIR = 1;   // read from RAM to peripheral
    DMA4CONbits.AMODE = 0; // register indirect with post increment
    DMA4CONbits.MODE = 0;  // continuous ping-pong mode

    DMA4REQbits.IRQSEL = 0b00011010; // output compare 4
    // DMA4REQbits.IRQSEL = 0b00000001; // select input compare 1 as trigger source

    DMA4STAL = __builtin_dmaoffset(adc16_tx_buffer);
    DMA4STAH = __builtin_dmapage(adc16_tx_buffer);

    switch (config->spi_module)
    {
    case SPI_MODULE_SELECTOR_1:
        DMA4PAD = (volatile unsigned int)&SPI1BUF; // peripheral address
        break;
    case SPI_MODULE_SELECTOR_2:
        DMA4PAD = (volatile unsigned int)&SPI2BUF; // peripheral address
        break;
    case SPI_MODULE_SELECTOR_3:
        DMA4PAD = (volatile unsigned int)&SPI3BUF; // peripheral address
        break;
    default:
        // report_error("ADC: unsupported SPI module.");
        break;
    }

    DMA4CNT = ADC16_TX_BUFFER_LENGTH - 1;

    _DMA4IE = 0;

    // configure DMA5 to read data over SPI
    _DMA5IF = 0;
    _DMA5IE = 0;

    DMA5CONbits.SIZE = 0;  // word data transfer
    DMA5CONbits.HALF = 0;  // interrupt when all data is transfered
    DMA5CONbits.NULLW = 0; // normal operation
    DMA5CONbits.DIR = 0;   // read from peripheral to RAM
    DMA5CONbits.AMODE = 0; // register indirect with post increment
    DMA5CONbits.MODE = 2;  // continuous ping-pong mode

    switch (config->spi_module)
    {
    case SPI_MODULE_SELECTOR_1:
        DMA5REQbits.IRQSEL = 0b00001010;           // select SPI1 transfer done as trigger source
        DMA5PAD = (volatile unsigned int)&SPI1BUF; // peripheral address
        break;
    case SPI_MODULE_SELECTOR_2:
        DMA5REQbits.IRQSEL = 0b00100001;           // select SPI2 transfer done as trigger source
        DMA5PAD = (volatile unsigned int)&SPI2BUF; // peripheral address
        break;
    case SPI_MODULE_SELECTOR_3:
        DMA5REQbits.IRQSEL = 0b01011011;           // select SPI3 transfer done as trigger source
        DMA5PAD = (volatile unsigned int)&SPI3BUF; // peripheral address
        break;
    default:
        // report_error("ADC: unsupported SPI module.");
        break;
    }

    DMA5STAL = __builtin_dmaoffset(adc16_rx_buffer_a); // buffer A
    DMA5STAH = __builtin_dmapage(adc16_rx_buffer_a);   // buffer A
    DMA5STBL = __builtin_dmaoffset(adc16_rx_buffer_b); // buffer B
    DMA5STBH = __builtin_dmapage(adc16_rx_buffer_b);

    DMA5CNT = config->adc16_buffer_size - 1;

    _DMA5IE = 1;

    // update callback
    rx_callback = config->rx_callback;

    // fill buffers with zeros and read command
    eds_read = DSRPAG;
    DSRPAG = __builtin_edspage(adc16_rx_buffer_a);
    for (i = 0; i < ADC16_MAX_BUFFER_LENGTH; i++)
    {
        adc16_rx_buffer_a[i] = 0;
        adc16_rx_buffer_b[i] = 0;
    }
    DSRPAG = __builtin_edspage(adc16_tx_buffer);
    for (i = 0; i < ADC16_TX_BUFFER_LENGTH; i++)
    {
        adc16_tx_buffer[i] = ADC16_COMMAND_TO_BITS(ADC16_READ_DATA);
    }
    DSRPAG = eds_read;

    // configure input capture module 1 to detect nEOC edge and start
    // SPI data transfer

    _IC1IF = 0;
    _IC1IE = 0;

    IC1CON1bits.ICSIDL = 0;     // continue operation in CPU idle mode
    IC1CON1bits.ICTSEL = 0b111; // peripheral clock is clock source
    IC1CON1bits.ICI = 0b00;     // interrupt after every match
    IC1CON1bits.ICM = 0b010;    // capture every falling edge

    IC1CON2bits.IC32 = 0;     // disable cascade mode
    IC1CON2bits.TRIGSTAT = 0; // no trigger has occurred
    IC1CON2bits.ICTRIG = 1;   // trigger mode
    IC1CON2bits.SYNCSEL = 0;  // no sync or trigger source
    IC1CON2 = 0;
    _IC1IE = 0;

    // update the configuration and prepare for conversion
    config->status = ADC16_STATUS_IDLE;

    spi_init_message(&m, write_data, read_data, 1, &config->cs_pin);

    send_spi_message = spi_get_handler(config->spi_module);

    // disable sampling
    OC15CON1bits.OCM = 0b000;

    // wake ADC
    m.status = SPI_TRANSFER_PENDING;
    write_data[0] = 0xB000;
    send_spi_message(&m);

    // force conversion to prevent unresponsive ADC
    CLEAR_BIT(config->conv_pin.lat_r, config->conv_pin.n);
    __delay_us(10);
    SET_BIT(config->conv_pin.lat_r, config->conv_pin.n);
    __delay_us(10);

    // store channel selection setting
    selection_config = config->channel_select;
    config->channel_select = ADC16_CHANNEL_SELECT_MODE_MANUAL;

    // update configuration to read all registers once in manual mode
    write_data[0] = adc16_parse_cfr_write(config);
    m.status = SPI_TRANSFER_PENDING;
    send_spi_message(&m);

    UART_DEBUG_PRINT("ADC config set: %04x.", m.write_data[0]);

    write_data[0] = ADC16_COMMAND_TO_BITS(ADC16_READ_CFR);
    m.status = SPI_TRANSFER_PENDING;
    send_spi_message(&m);

    UART_DEBUG_PRINT("ADC config read: %04x.", m.read_data[0]);

    // read all channels once in manual mode
    for (i = 0; i < ADC16_N_CHANNELS; i++)
    {
        config->channel = i;

        read_data[0] = adc16_read_channel(config);

        covert_uint_to_fract(1, &m.read_data[0], (fractional *)&m.read_data[1]);

        UART_DEBUG_PRINT("ADC read %x: %04x, %.6f (%.6f)", i, m.read_data[0], ((double)m.read_data[0]) / 0x10000, (double)Fract2Float((fractional)m.read_data[1]));

        __delay_ms(10);
    }

    config->channel = original_channel;

    // select actual channel in config
    write_data[0] = ADC16_COMMAND_TO_BITS(config->channel);
    spi_init_message(&m, write_data, read_data, 1, &config->cs_pin);
    send_spi_message(&m);

    // update ADC channel to final trigger mode
    config->channel_select = selection_config;

    // update status config
    write_data[0] = adc16_parse_cfr_write(config);

    UART_DEBUG_PRINT("ADC config set: %04x.", m.write_data[0]);

    spi_init_message(&m, write_data, read_data, 1, &config->cs_pin);
    send_spi_message(&m);

    write_data[0] = ADC16_COMMAND_TO_BITS(ADC16_READ_CFR);
    spi_init_message(&m, write_data, read_data, 1, &config->cs_pin);
    send_spi_message(&m);

    UART_DEBUG_PRINT("ADC config read: %04x.", m.read_data[0]);

    if (config->deep_powerdown == ADC16_DEEP_POWERDOWN_ENABLE)
    {
        config->status = ADC16_STATUS_OFF;
    }

    // configure OC15 module to generate nCONVST signal to trigger sampling
    // minimum pulse width = 40ns (or 3 clock cycles) and there must be 1500ns
    // (96 cycli) between conversions

    OC15CON1bits.OCM = 0b000;                      // Disable Output Compare Module
    OC15R = (FCY / config->sample_frequency) - 5;  // Write the duty cycle for the PWM pulse
    OC15RS = (FCY / config->sample_frequency) - 1; // Write the PWM frequency
    OC15CON1bits.OCTSEL = 0b111;                   // Select peripheral clock as output compare time base
    OC15CON2bits.SYNCSEL = 0b11111;                // no sync or trigger source

    // OC15CON2bits.OCINV = 1; // invert output

    _OC15IF = 0; // clear the OC15 interrupt flag
    _OC15IE = 0; // disable OC155 interrupt

    // configure the CS pin
    OC4CON1bits.OCM = 0b000; // Disable Output Compare Module
    // SPI configured to operate at 64MHz/7 -> 16 periods needed -> 16*7=112 cycles needed
    OC4R = (FCY / config->sample_frequency) - 120; // Write the duty cycle for the PWM pulse
    OC4RS = (FCY / config->sample_frequency) - 1;  // frequency
    OC4CON1bits.OCSIDL = 0;                        // continue operation in CPU idle mode
    OC4CON1bits.OCTSEL = 0b111;                    // user peripheral clock as clock source
    OC4CON2bits.SYNCSEL = 0b11111;                 // no sync or trigger source

    OC4CON2bits.OCINV = 1; // invert output

    _OC4IF = 0; // clear the OC4 interrupt flag
    _OC4IE = 0; // disable OC4 interrupt
}

void adc16_init_fast(sensor_adc16_config_t *config)
{
    spi_message_t m;
    uint16_t write_data[2];
    uint16_t read_data[2];
    read_data[0] = 0x0;
    void (*send_spi_message)(spi_message_t * m);

    DMA4CNT = ADC16_TX_BUFFER_LENGTH - 1;
    DMA5CNT = config->adc16_buffer_size - 1;

    // update callback
    rx_callback = config->rx_callback;

    // update the configuration and prepare for conversion
    config->status = ADC16_STATUS_IDLE;

    send_spi_message = spi_get_handler(config->spi_module);

    // disable sampling
    OC15CON1bits.OCM = 0b000;

    // wake ADC
    write_data[0] = 0xB000;
    spi_init_message(&m, write_data, read_data, 1, &config->cs_pin);
    send_spi_message(&m);

    // force conversion to prevent unresponsive ADC
    CLEAR_BIT(config->conv_pin.lat_r, config->conv_pin.n);
    __delay_us(10);
    SET_BIT(config->conv_pin.lat_r, config->conv_pin.n);
    __delay_us(10);

    // update configuration to read all registers once in manual mode
    write_data[0] = adc16_parse_cfr_write(config);
    spi_init_message(&m, write_data, read_data, 1, &config->cs_pin);
    send_spi_message(&m);

    // select actual channel in config
    write_data[0] = ADC16_COMMAND_TO_BITS(config->channel);
    spi_init_message(&m, write_data, read_data, 1, &config->cs_pin);
    send_spi_message(&m);

    if (config->deep_powerdown == ADC16_DEEP_POWERDOWN_ENABLE)
    {
        config->status = ADC16_STATUS_OFF;
    }

    OC15CON1bits.OCM = 0b000;                      // Disable Output Compare Module
    OC15R = (FCY / config->sample_frequency) - 5;  // Write the duty cycle for the PWM pulse
    OC15RS = (FCY / config->sample_frequency) - 1; // Write the PWM frequency

    // configure the CS pin
    OC4CON1bits.OCM = 0b000;                       // Disable Output Compare Module
    OC4R = (FCY / config->sample_frequency) - 120; // Write the duty cycle for the PWM pulse
    OC4RS = (FCY / config->sample_frequency) - 1;  // frequency
}

void __attribute__((__interrupt__, no_auto_psv)) _OC15Interrupt(void)
{
    _OC15IF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _OC4Interrupt(void)
{
    _OC4IF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _IC1Interrupt(void)
{
    _IC1IF = 0;
}

void adc16_start(sensor_adc16_config_t *config)
{
    // reset DMA transfer lengths (just in case)
    DMA5CNT = config->adc16_buffer_size - 1;
    DMA4CNT = ADC16_TX_BUFFER_LENGTH - 1;

    config->status = ADC16_STATUS_ON;

    // update SPI transfer mode
    switch (config->spi_module)
    {
    case SPI_MODULE_SELECTOR_1:
        adc16_spi1_init();
        break;
    case SPI_MODULE_SELECTOR_2:
        adc16_spi2_init();
        break;
    case SPI_MODULE_SELECTOR_3:
        adc16_spi3_init();
        break;
    default:
        // report_error("ADC: unsupported SPI module.");
        break;
    }

    // start SPI transfers
    DMA5CONbits.CHEN = 1; // enable DMA channel
    DMA4CONbits.CHEN = 1; // enable DMA channel

    asm volatile(
        "mov OC4CON1, w0  \n"
        "mov OC15CON1, w1 \n"
        "ior #0b110, w0 \n" // Select the Output Compare mode: EDGE-ALIGNED PWM MODE
        "ior #0b110, w1 \n" // Select the Output Compare mode: EDGE-ALIGNED PWM MODE
        "mov #112, w2\n"
        //"mov OC4R, w3\n"
        //"sub w3, w2, w2\n"
        "sub #4, w2\n"
        "mov w1, OC15CON1   \n"
        "repeat w2 \n"
        "nop \n"
        "mov w0, OC4CON1    \n");
}

void adc16_stop(sensor_adc16_config_t *config)
{
    uint16_t dummy_read;

    config->status = ADC16_STATUS_OFF;

    OC15CON1bits.OCM = 0b000; // disable OC1 module
    OC4CON1bits.OCM = 0b000;  // disable OC1 module

    // disable DMA transfer registers
    DMA5CONbits.CHEN = 0; // disable DMA channel
    DMA4CONbits.CHEN = 0; // disable DMA channel

    // check if unread data in SPI buffer
    switch (config->spi_module)
    {
    case SPI_MODULE_SELECTOR_1:
        if (SPI1STATbits.SPIRBF)
        {
            dummy_read = SPI1BUF;
        }
        break;
    case SPI_MODULE_SELECTOR_2:
        if (SPI2STATbits.SPIRBF)
        {
            dummy_read = SPI2BUF;
        }
        break;
    case SPI_MODULE_SELECTOR_3:
        if (SPI3STATbits.SPIRBF)
        {
            dummy_read = SPI3BUF;
        }
        break;
    default:
        // report_error("ADC: unsupported SPI module.");
        break;
    }

    SET_BIT(config->cs_pin.lat_r, config->cs_pin.n);
}

void __attribute__((__interrupt__, no_auto_psv)) _DMA5Interrupt(void)
{
    rx_callback();
    _DMA5IF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _DMA4Interrupt(void)
{
    _DMA4IF = 0;
}

void adc16_spi1_init(void)
{
    uint16_t dummy;
    dummy = SPI1BUF;
    SPI1STATbits.SPIROV = 0;
    _SPI1IE = 0;
}

void adc16_spi2_init(void)
{
    uint16_t dummy;
    dummy = SPI2BUF;
    SPI2STATbits.SPIROV = 0;
    _SPI2IE = 0;
}

void adc16_spi3_init(void)
{
    uint16_t dummy;
    dummy = SPI3BUF;
    SPI3STATbits.SPIROV = 0;
    _SPI3IE = 0;
}

uint16_t adc16_read_channel(sensor_adc16_config_t *config)
{
    uint16_t write_data[2], read_data[2];
    spi_message_t m;
    spi_handler_t spi_handler;

    // get the SPI handler
    spi_handler = spi_get_handler(config->spi_module);

    // select channel
    write_data[0] = ADC16_COMMAND_TO_BITS(config->channel);
    spi_init_message(&m, write_data, read_data, 1, &config->cs_pin);
    spi_handler(&m);

    // sample
    CLEAR_BIT(config->conv_pin.lat_r, config->conv_pin.n);
    __delay_us(10);
    SET_BIT(config->conv_pin.lat_r, config->conv_pin.n);
    __delay_us(10);

    // read sample
    write_data[0] = ADC16_COMMAND_TO_BITS(ADC16_READ_DATA);
    write_data[1] = 0x0000;
    spi_init_message(&m, write_data, read_data, 1, &config->cs_pin);
    spi_handler(&m);

    return m.read_data[0];
}

uint16_t adc16_run_calibration(sensor_adc16_config_t *config,
                              const uint16_t reference_value)
{

    uint16_t n;
    uint64_t sample_data;
    void (*send_spi_message)(spi_message_t * m);

    switch (config->spi_module)
    {
    case SPI_MODULE_SELECTOR_1:
        send_spi_message = spi1_send_message;
        break;
    case SPI_MODULE_SELECTOR_2:
        send_spi_message = spi2_send_message;
        break;
    case SPI_MODULE_SELECTOR_3:
        send_spi_message = spi3_send_message;
        break;
    default:
        // report_error("ADC: unsupported SPI module.");
        break;
    }

    sample_data = 0;

    // read all data
    for (n = 0; n < ADC16_N_CALIBRATION_SAMPLES; n++)
    {
        sample_data += adc16_read_channel(config);
    }

    // average data
    sample_data /= ADC16_N_CALIBRATION_SAMPLES;

    // check to reference value

    sample_data = sample_data - reference_value;

    config->channel_offset[config->channel] = (uint16_t) sample_data;

    return (uint16_t) sample_data;
}

void adc16_run_max_var(sensor_adc16_config_t *config, uint16_t *const max_value,
                       uint16_t *const min_value, uint16_t *const mean)
{

    uint64_t sum_value = 0;
    uint16_t n, sample_value;
    void (*send_spi_message)(spi_message_t * m);

    switch (config->spi_module)
    {
    case SPI_MODULE_SELECTOR_1:
        send_spi_message = spi1_send_message;
        break;
    case SPI_MODULE_SELECTOR_2:
        send_spi_message = spi2_send_message;
        break;
    case SPI_MODULE_SELECTOR_3:
        send_spi_message = spi3_send_message;
        break;
    default:
        // report_error("ADC: unsupported SPI module.");
        break;
    }

    // default values
    *max_value = 0;
    *min_value = 0;

    // read all data
    for (n = 0; n < ADC16_N_CALIBRATION_SAMPLES; n++)
    {
        sample_value = adc16_read_channel(config);
        sum_value += sample_value;

        *max_value = MAX(*max_value, sample_value);
        *min_value = MIN(*min_value, sample_value);
    }

    // average data
    *mean = (uint16_t)(sum_value / ADC16_N_CALIBRATION_SAMPLES);

    // values are passed by reference using the arguments
}
