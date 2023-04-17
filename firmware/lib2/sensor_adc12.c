#include <dsp.h>
#include <libpic30.h>
#include <sensor_adc12.h>
#include <sensor.h>
#include <address.h>
#include <utilities.h>

fractional adc12_buffer_a[ADC12_N_CHANNELS][ADC12_DMA_BUFFER_SIZE]  __attribute__( (eds, aligned(256), space(xmemory)) );
fractional adc12_buffer_b[ADC12_N_CHANNELS][ADC12_DMA_BUFFER_SIZE]  __attribute__( (eds, aligned(256), space(xmemory)) );


void sensor_adc12_get_config(struct sensor_interface_s* intf, uint8_t reg, uint8_t* buffer, uint8_t* length){
    sensor_adc12_config_t *config = &intf->config.adc12;
    
    buffer[0] = SENSOR_TYPE_ADC12;
    buffer[1] = reg;
    
    switch(reg){
        case sensor_adc12_gloxinia_register_general:
            intf->measure.task.cb = sensor_adc12_measure;
            intf->measure.task.data = (void *)intf;

            buffer[2] = intf->measure.period >> 8;
            buffer[3] = intf->measure.period & 0x0ff;
            *length = 4;
            break;
        case sensor_adc12_gloxinia_register_config:
            buffer[2] = config->enable_ch_a;
            buffer[3] = config->enable_ch_b;
            buffer[4] = config->normalise_ch_a;
            buffer[5] = config->normalise_ch_b;
            *length = 7;
            break;
        default:
            *length = 0;
    }
}

sensor_status_t sensor_adc12_config(struct sensor_interface_s *intf, uint8_t *buffer, uint8_t length)
{
    if (length < 1)
    {
        return SENSOR_STATUS_ERROR;
    }

    UART_DEBUG_PRINT("Configuring ADC12");

    sensor_adc12_config_t *config = &intf->config.adc12;
    
    config->result_ch_a = 0;
    config->result_ch_b = 0;
    
    switch(buffer[0]){
        case sensor_adc12_gloxinia_register_general:
            if (length != 3){ return SENSOR_STATUS_ERROR; }
            
            intf->measure.task.cb = sensor_adc12_measure;
            intf->measure.task.data = (void *)intf;

            schedule_init(&intf->measure, intf->measure.task, (((uint16_t)buffer[1]) << 8) | buffer[2]);

            return SENSOR_STATUS_IDLE;
            
            break;
        case sensor_adc12_gloxinia_register_config:
            if(length != 6) { return SENSOR_STATUS_ERROR; }
            
            // load configuration from buffer into data structure
            config->enable_ch_a = buffer[2];
            config->enable_ch_b = buffer[3];
            config->normalise_ch_a = buffer[4];
            config->normalise_ch_b = buffer[5];

            // validate configuration
            if(!validate_adc12_config(config)){
                UART_DEBUG_PRINT("Configuring ADC12 INVALID CONFIG");
                return SENSOR_STATUS_ERROR;
            } else {
                // start sensor initialisation (async) only when configuration is OK
                sensor_adc12_init_sensor(intf);
            }
            break;
        default:
            break;
    }

    return SENSOR_STATUS_IDLE;
}

void sensor_adc12_measure(void *data)
{
    struct sensor_interface_s *intf = (struct sensor_interface_s *)data;

    if (intf->config.sht35.periodicity == S_SHT35_SINGLE_SHOT)
    {
        if (i2c_check_message_sent(&intf->config.sht35.m_read) &&
            i2c_check_message_sent(&intf->config.sht35.m_config))
        {
            i2c_reset_message(&intf->config.sht35.m_read, 1);
            i2c_queue_message(&intf->config.sht35.m_read);
            i2c_reset_message(&intf->config.sht35.m_config, 1);
            i2c_queue_message(&intf->config.sht35.m_config);
        }
        else
        {
            UART_DEBUG_PRINT("SHT35 on %x not fully processed.", intf->sensor_id);
            intf->config.sht35.m_read.status = I2C_MESSAGE_CANCELED;
            intf->config.sht35.m_config.status = I2C_MESSAGE_CANCELED;

            intf->status = SENSOR_STATUS_ERROR;
            sensor_error_handle(intf);
        }
    }
    else
    {
        if (i2c_check_message_sent(&intf->config.sht35.m_fetch))
        {
            i2c_reset_message(&intf->config.sht35.m_fetch, 1);
            i2c_queue_message(&intf->config.sht35.m_fetch);
        }
        else
        {
            UART_DEBUG_PRINT("SHT35 %x not fully processed.", intf->sensor_id);
            intf->status = SENSOR_STATUS_ERROR;

            sensor_error_handle(intf);
        }
    }
}

void sensor_adc12_init_sensor(struct sensor_interface_s *intf)
{
    // each channel is converted at a rate of 20kHz
    // the ADC converts at 160kHz in total
    // 12-bit mode required 14T_AD for conversion and at least 2-3 T_AD for sampling
    // We assume 20T_AD for the entire cycle (should be sufficient)
    // Assuming FCY = 64MHz, we get a divisor of 20 from the system clock
    
    // power reset the ADC module
    PMD1bits.AD1MD = 1;
    __delay_us(20);
    PMD1bits.AD1MD = 1;
    __delay_us(20);
    
    // disable the ADC module
    AD1CON1bits.ADON = 0;
    
    AD1CON1bits.ADDMABM = 1; // DMA buffers are written in the order of conversion
    AD1CON1bits.AD12B = 1; // 12-bit mode
    //AD1CON1bits.FORM = 0b10; // use fractional format (no sign)
    AD1CON1bits.FORM = 0b11; // use fractional format Q15 (includes sign)
    AD1CON1bits.SSRCG  = 0; // clock selection
    AD1CON1bits.SSRC = 0b010; // Timer3 compare ends sampling and starts conversion
    AD1CON1bits.ASAM = 1; // Sampling begins immediately after the last conversion; SAMP bit is auto-set
    AD1CON2bits.VCFG = 0b000; // AVDD and AVSS are VREFH and VREFL respectively
    AD1CON2bits.CSCNA = 1; // scan inputs
    AD1CON2bits.SMPI = 0b0000; // increment DMA address upon every conversion
    AD1CON4bits.ADDMAEN = 1; // Conversion results stored in ADCxBUF0 register for DMA transfer
    AD1CON4bits.DMABL = 0b0; // not used
    
    AD1CON3bits.ADRC = 0; // clock is derived from system clock
    AD1CON3bits.ADCS = (FCY / (ADC12_FULL_SAMPLE_CONVERSION_T_AD * ADC12_N_CHANNELS * ADC12_CHANNEL_SAMPLE_RATE)); // T_AD = (20*T_CY) // ADC conversion clock
    
    // input scan selection registers
    AD1CSSH = 0x0;
    AD1CSSL = 0x0;
    
    // for dicio
    AD1CSSHbits.CSS31 = 1; // OW3
    AD1CSSHbits.CSS30 = 1; // OW1
    AD1CSSHbits.CSS27 = 1; // AS3
    AD1CSSHbits.CSS28 = 1; // AS1
    AD1CSSLbits.CSS1 = 1; // OW4
    AD1CSSLbits.CSS3 = 1; // OW2
    AD1CSSLbits.CSS0 = 1; // AS4
    AD1CSSLbits.CSS2 = 1; // AS2
    
    // determine number of channels to be scanned
    int num_scan_channels = 0;
    for(int i = 0; i < 16; i++)
    {
        if(((AD1CSSH >> i) & 0b1) == 1)
            num_scan_channels++;
        if(((AD1CSSL >> i) & 0b1) == 1)
            num_scan_channels++;
    }
    // if none of the channels are activated, stop initialisation
    if(num_scan_channels == 0)
        return;
    
    AD1CON2bits.SMPI = ( num_scan_channels - 1 );
    
    _AD1IF = 0;
    _AD1IE = 0;
    
    ///// configure conversion timer
    TMR3 = 0x0;
    PR3 = (FCY / (ADC12_N_CHANNELS * ADC12_CHANNEL_SAMPLE_RATE)) - 1;
    
    _T3IF = 0;
    _T3IE = 0;
    
    
    ///// configure DMA channel
    DMA0CONbits.AMODE = 0;   // Configure DMA for register indirect with post increment
    DMA0CONbits.MODE  = 2;   // Configure DMA for Continuous Ping-Pong mode

    DMA0PAD = (volatile unsigned int) &ADC1BUF0;
    DMA0CNT = (ADC12_N_CHANNELS * ADC12_DMA_BUFFER_SIZE - 1);    

    DMA0REQ = 13; // ADC1 -> DMA request source

    DMA0STAL = __builtin_dmaoffset(adc12_buffer_a);
    DMA0STAH = __builtin_dmapage(adc12_buffer_a);

    DMA0STBL = __builtin_dmaoffset(adc12_buffer_b);
    DMA0STBH = __builtin_dmapage(adc12_buffer_b);


    _DMA0IF = 0;   //Clear the DMA interrupt flag bit
    _DMA0IE = 1;   //Set the DMA interrupt enable bit

    DMA0CONbits.CHEN = 1;

    TRISBbits.TRISB1 = 0;
}


void __attribute__ ( ( interrupt, no_auto_psv ) ) _DMA0Interrupt ( void )
{
    _DMA0IF = 0; 
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _T3Interrupt ( void )
{
    _T3IF = 0; 
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _AD1Interrupt ( void )
{
    _AD1IF = 0; 
}



void sensor_adc12_activate(sensor_interface_t* intf){
    intf->status = SENSOR_STATUS_RUNNING;
    
    // Start ADC
    AD1CON1bits.ADON = 1; // enable ADC
    T3CONbits.TON = 1; // start conversion trigger
}


bool validate_adc12_config(sensor_adc12_config_t *config)
{
    sensor_adc12_config_t original = *config;
    
    return true;
}