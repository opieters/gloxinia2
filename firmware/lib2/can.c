#include <can.h>
#include <address.h>
#include <libpic30.h>

uint16_t ecan_message_buffer[NUM_OF_ECAN_BUFFERS][8] __attribute__((aligned(NUM_OF_ECAN_BUFFERS * 16)));

int16_t n_connected_devices = 0;


void can_init(void) {
    // switch ECAN module to configuration mode
    C1CTRL1bits.REQOP = CAN_CONFIG_MODE;
    while (C1CTRL1bits.OPMODE != CAN_CONFIG_MODE); // wait for mode switch

    C1CTRL1bits.WIN = 0; // use buffer control and status window

    /* Set up the CAN module for 250kbps speed with 16 Tq per bit. */
    //C1CTRL1bits.CANCKS = 1; // 1FP, check errata (error in datasheet)
    /*C1CFG1bits.SJW = 3; // sync jump with bits to maintain sync with transmitted message
    C1CFG1bits.BRP = 7; // baud rate prescaler: 2xBRP/F_CAN = TQ
    C1CFG2bits.WAKFIL = 0;
    C1CFG2bits.PRSEG = 4; // propagation time segment bits -> 4TQ
    C1CFG2bits.SEG1PH = 7; // phase segment 1 bits -> length is 6 TQ
    C1CFG2bits.SEG2PH = 5; // phase segment 2 bits in TQ -> 5 TQ
    C1CFG2bits.SEG2PHTS = 1; // phase segment 2 time select bit is freely programmable
    C1CFG2bits.SAM = 1; // CAN line bus is sampled 3 time at the same point
     * */
    
        /* Set up the CAN module for 250kbps speed with 10 Tq per bit. */
    C1CFG1bits.SJW = 1; // sync jump with bits to maintain sync with transmitted message
    C1CFG1bits.BRP = 7; // baud rate prescaler: 2xBRP/F_CAN = TQ
    C1CFG2bits.WAKFIL = 0;
    C1CFG2bits.SEG2PH = 2; // phase segment 2 bits in TQ -> 2TQ
    C1CFG2bits.SEG2PHTS = 1; // phase segment 2 time select bit is freely programmable
    C1CFG2bits.SAM = 1; // CAN line bus is sampled 3 time at the same point
    C1CFG2bits.SEG1PH = 2; // phase segment 1 bits -> length is 5 TQ
    C1CFG2bits.PRSEG = 2; // propagation time segment bits -> 3TQ
    
    

    /* Configure ECAN module buffers */
    C1FCTRLbits.DMABS = 0b110; // 32 buffers in device RAM
    C1FCTRLbits.FSA = 0b01000; // FIFO start address = read buffer RB8
    //C1FCTRLbits.FSA = 0b11111; // same as example now

    C1CTRL1bits.WIN = 1; // use filter control and status window

    // configure all filter mask 0 
    C1FMSKSEL1 = 0;
    C1FMSKSEL2 = 0;

    // configure all filters to store messages in FIFO
    C1BUFPNT1 = 0xFFFF;
    C1BUFPNT2 = 0xFFFF;
    C1BUFPNT3 = 0xFFFF;
    C1BUFPNT4 = 0xFFFF;

    /*if(controller_address != 0U){
        // configure mask 0 to ignore last 3 bits bits
        C1RXM0SIDbits.SID = 0x7F8;    

        // configure filter 0 to only accept messages from gateway
        C1RXF0SIDbits.SID = 0;
        
        // configure mask to check for standard and extended (SID) frames
        C1RXM0SIDbits.MIDE = 0;
    } else {*/
    // configure mask 0 to ignore all bits
    C1RXM0SIDbits.SID = 0b0;
    C1RXM0SIDbits.EID = 0;
    C1RXM0EIDbits.EID = 0;

    // configure mask to check for standard and extended (SID) frames
    C1RXM0SIDbits.MIDE = 0;
    //}

    C1CTRL1bits.WIN = 0;

    // interrupt configuration
    //C1INTEbits.IVRIE = 0; // invalid message interrupt disabled
    //C1INTEbits.WAKIE = 0; // bus wake-up activity interrupt disabled
    //C1INTEbits.ERRIE = 1; // error interrupt enable bit enabled TODO: interrupt service!
    //C1INTEbits.FIFOIE = 0; // FIFO almost full interrupt enable off
    //C1INTEbits.RBOVIE = 0; // buffer overflow interrupt disabled
    //C1INTEbits.RBIE = 1; // RX buffer interrupt enabled
    //C1INTEbits.TBIE = 1; // TX buffer interrupt disabled

    // enable ECAN interrupts
    //_C1IE = 1;

    // configure first 8 buffers for sending data
    C1TR01CONbits.TXEN0 = 1; // configure buffer 0 for transmission
    C1TR01CONbits.TX0PRI = 3; // assign priority
    C1TR01CONbits.TXEN1 = 1; // configure buffer 1 for transmission
    C1TR01CONbits.TX1PRI = 3; // assign priority
    C1TR23CONbits.TXEN2 = 1; // configure buffer 2 for transmission
    C1TR23CONbits.TX2PRI = 3; // assign priority
    C1TR23CONbits.TXEN3 = 1; // configure buffer 3 for transmission
    C1TR23CONbits.TX3PRI = 3; // assign priority
    C1TR45CONbits.TXEN4 = 1; // configure buffer 4 for transmission
    C1TR45CONbits.TX4PRI = 3; // assign priority
    C1TR45CONbits.TXEN5 = 1; // configure buffer 5 for transmission
    C1TR45CONbits.TX5PRI = 3; // assign priority
    C1TR67CONbits.TXEN6 = 1; // configure buffer 6 for transmission
    C1TR67CONbits.TX6PRI = 3; // assign priority
    C1TR67CONbits.TXEN7 = 1; // configure buffer 7 for transmission
    C1TR67CONbits.TX7PRI = 3; // assign priority

    // other buffers are for receiving data

    // configure TX DMA channel
    DMA1CONbits.SIZE = 0x0; // Word Transfer Mode
    DMA1CONbits.DIR = 0x1; // Data Transfer Direction: device RAM to Peripheral
    DMA1CONbits.AMODE = 0x2; // peripheral indirect addressing mode
    DMA1CONbits.MODE = 0x0; // Operating Mode: Continuous, Ping-Pong modes disabled
    DMA1REQ = 0b01000110; // select TX data ready as IRQ
    DMA1CNT = 7; // 8 DMA Transfers per ECAN message
    DMA1PAD = (volatile unsigned int) &C1TXD; // ECAN transmit register

    // data source to be transferred
    DMA1STAL = (unsigned int) &ecan_message_buffer;
    DMA1STAH = 0; //(unsigned int) &ecan_message_buffer;

    _DMA1IE = 1; // enable DMA channel interrupt
    _DMA1IF = 0; // clear DMA interrupt flag
    DMA1CONbits.CHEN = 1; // enable DMA channel

    // configure RX DMA channel 
    DMA2CONbits.SIZE = 0; // Word Transfer Mode
    DMA2CONbits.DIR = 0; // Data Transfer Direction: device RAM to Peripheral
    DMA2CONbits.AMODE = 0x2; // peripheral indirect addressing mode
    DMA2CONbits.MODE = 0x0; // Operating Mode: Continuous, Ping-Pong modes disabled
    DMA2REQ = 0b00100010; // select RX data ready as IRQ
    DMA2CNT = 7; // 8 DMA Transfers per ECAN message
    DMA2PAD = (volatile unsigned int) &C1RXD; // ECAN receive register

    // data source to be transferred
    DMA2STAL = (unsigned int) &ecan_message_buffer;
    DMA2STAH = 0; // (unsigned int) &ecan_message_buffer;

    _DMA2IE = 1; // enable DMA channel interrupt
    _DMA2IF = 0; // clear DMA interrupt flag
    DMA2CONbits.CHEN = 1; // enable DMA channel

    // switch ECAN module to normal operation mode
    C1CTRL1bits.REQOP = CAN_MODULE_ENABLE;
    while (C1CTRL1bits.OPMODE != CAN_MODULE_ENABLE); // wait for mode switch
}

void can_disable(void) {
    C1CTRL1bits.ABAT = 1; // abort all transmissions

    C1CTRL1bits.REQOP = CAN_MODULE_DISABLE; // disable CAN module
    while (C1CTRL1bits.OPMODE != CAN_MODULE_DISABLE); // wait for mode switch

    // disable DMA
    DMA2CONbits.CHEN = 0;
    DMA1CONbits.CHEN = 0;
}

void can_reset(void) {
    // when the module is still running fine, do not restart
    if(C1CTRL1bits.OPMODE == CAN_MODULE_ENABLE){
        return;
    }
    
    _TRISE4 = 0;
    _RP84R = 0;
    _RE4 = 0;
    __delay_ms(10);
    _RE4 = 1;
    __delay_ms(10);
    _RP84R = _RPOUT_C1TX;
    
    can_init();
}

void can_init_message(can_message_t* m,
        uint16_t identifier,
        uint8_t remote_frame,
        uint8_t extended_frame,
        uint32_t extended_identifier,
        uint8_t* data,
        uint8_t data_length) {
    m->identifier = identifier;
    m->remote_frame = remote_frame;
    m->extended_frame = extended_frame;
    m->extended_identifier = extended_identifier;
    m->data_length = data_length;
    m->data = data;
}

void parse_from_can_buffer(can_message_t* m, uint16_t* raw_data){
    uint16_t i;

    m->identifier = raw_data[0] >> 2;
    if ((raw_data[0] & 0x03) == 0b11) {
        m->extended_frame = 1;
        m->extended_identifier = (((uint32_t) raw_data[1]) << 6) | (raw_data[2] >> 10);
    } else {
        m->extended_frame = 0;
        m->extended_identifier = 0;
    }

    m->data_length = raw_data[2] & 0xF;

    if(m->extended_frame){
        if (((raw_data[2] & 0x200U) == 0x200U)) {
            m->remote_frame = 1;
        } else {
            m->remote_frame = 0;
        }
    } else {
        if((raw_data[0] & 0b10) == 0b10){
            m->remote_frame = 1;
        } else {
            m->remote_frame = 0;
        }
    }
    if(m->remote_frame == 0){    
        for (i = 0; i < m->data_length; i++) {
            if (i % 2 == 0) {
                m->data[i] = (uint8_t) raw_data[i / 2 + 3];
            } else {
                m->data[i] = (uint8_t) (raw_data[(i - 1) / 2 + 3] >> 8);
            }
        }
    }
}

void parse_from_can_message(message_t* m, can_message_t* cm){
    message_init(m, cm->identifier,
        cm->remote_frame,
        (message_cmd_t) cm->extended_identifier >> 8,
        cm->extended_identifier & 0xff,
        cm->data,
        cm->data_length);
    m->status = M_RX_FROM_CAN;
}


can_status_t can_send_message(can_message_t* message, uint8_t can_channel) {
    uint16_t i;

    if (C1CTRL1bits.OPMODE == CAN_MODULE_DISABLE) {
        return CAN_NOT_ENABLED;
    }
    
    if (controller_address == ADDRESS_UNSET){
        return CAN_ADDRESS_NOT_SET;
    }

    switch (can_channel) {
        case 0:
            if (C1TR01CONbits.TXREQ0 == 1) {
                return CAN_TX_PENDING;
            }
            break;
        case 1:
            if (C1TR01CONbits.TXREQ1 == 1) {
                return CAN_TX_PENDING;
            }
            break;
        case 2:
            if (C1TR23CONbits.TXREQ2 == 1) {
                return CAN_TX_PENDING;
            }
            break;
        case 3:
            if (C1TR23CONbits.TXREQ3 == 1) {
                return CAN_TX_PENDING;
            }
            break;
        case 4:
            if (C1TR45CONbits.TXREQ4 == 1) {
                return CAN_TX_PENDING;
            }
            break;
        case 5:
            if (C1TR45CONbits.TXREQ5 == 1) {
                return CAN_TX_PENDING;
            }
            break;
        case 6:
            if (C1TR67CONbits.TXREQ6 == 1) {
                return CAN_TX_PENDING;
            }
            break;
        case 7:
            if (C1TR67CONbits.TXREQ7 == 1) {
                return CAN_TX_PENDING;
            }
            break;
        default:
            break;
    }

    ecan_message_buffer[can_channel][0] = ((message->identifier & 0x7FFU) << 2) | (message->extended_frame == 0 ? 0b00 : 0b11) | (message->remote_frame == 0 ? 0b00 : 0b10);
    ecan_message_buffer[can_channel][1] = (message->extended_identifier >> 6) & 0xFFFU;
    ecan_message_buffer[can_channel][2] = ((message->extended_identifier << 10) & 0xFC00U) | (message->remote_frame == 0U ? 0x000U : 0x200U) | (message->data_length & 0xFU);
    for (i = 0; i < message->data_length; i++) {
        if ((i % 2U) == 0) {
            ecan_message_buffer[can_channel][3 + i / 2] = message->data[i];
        } else {
            ecan_message_buffer[can_channel][3 + (i - 1) / 2] = ecan_message_buffer[can_channel][3 + (i - 1) / 2] | (((uint16_t) message->data[i]) << 8);
        }
    }

    switch (can_channel) {
        case 0:
            C1TR01CONbits.TXREQ0 = 1;
            break;
        case 1:
            C1TR01CONbits.TXREQ1 = 1;
            break;
        case 2:
            C1TR23CONbits.TXREQ2 = 1;
            break;
        case 3:
            C1TR23CONbits.TXREQ3 = 1;
            break;
        case 4:
            C1TR45CONbits.TXREQ4 = 1;
            break;
        case 5:
            C1TR45CONbits.TXREQ5 = 1;
            break;
        case 6:
            C1TR67CONbits.TXREQ6 = 1;
            break;
        case 7:
            C1TR67CONbits.TXREQ7 = 1;
            break;
        default:
            break;
    }
    return CAN_NO_ERROR;
}


can_status_t can_send_message_any_ch(can_message_t* m) {
    static uint16_t i = 0;
    uint16_t n_attempts = 0;
    can_status_t sent_status = CAN_NO_ERROR;
    do {
        sent_status = can_send_message(m, i);
        i = (i + 1) % 8;
        n_attempts++;
    } while ((sent_status != CAN_NO_ERROR) && (n_attempts < 8U));
    return sent_status;
}

void can_message_from_fmessage(can_message_t* cm, message_t* m){
    cm->identifier = m->identifier;
    cm->remote_frame = m->request_message_bit;
    cm->extended_frame = CAN_EXTENDED_FRAME;
    cm->extended_identifier = CAN_HEADER(m->command, m->sensor_identifier);
    cm->data_length = m->length;
    cm->data = m->data;
}


can_status_t can_send_fmessage_any_ch(message_t* m) {
    can_message_t cm;
    static uint16_t i = 0;
    uint16_t n_attempts = 0;
    can_status_t sent_status = CAN_NO_ERROR;
    can_message_from_fmessage(&cm, m);
    do {
        sent_status = can_send_message(&cm, i);
        i = (i + 1) % 8;
        n_attempts++;
    } while ((sent_status != CAN_NO_ERROR) && (n_attempts < 8U));
    return sent_status;
}

void can_detect_devices(void) {
    message_t m;
    bool no_device_found = false;

    message_init(
            &m, 
            controller_address, 
            CAN_NO_REMOTE_FRAME,
            M_HELLO,
            0,
            NULL, 0);

    message_send(&m);

    __delay_ms(100);

    // check CAN status
    if (C1TR01CONbits.TXREQ0 == 1) {
        no_device_found = true;
    }
    if (C1TR01CONbits.TXREQ1 == 1) {
        no_device_found = true;
    }
    if (C1TR23CONbits.TXREQ2 == 1) {
        no_device_found = true;
    }
    if (C1TR23CONbits.TXREQ3 == 1) {
        no_device_found = true;
    }
    if (C1TR45CONbits.TXREQ4 == 1) {
        no_device_found = true;
    }
    if (C1TR45CONbits.TXREQ5 == 1) {
        no_device_found = true;
    }
    if (C1TR67CONbits.TXREQ6 == 1) {
        no_device_found = true;
    }
    if (C1TR67CONbits.TXREQ7 == 1) {
        no_device_found = true;
    }

    // disable CAN if needed
    if (no_device_found) {
        n_connected_devices = 0;

        can_disable();

        UART_DEBUG_PRINT("No devices found.");

        //can_disable();
    } else {
        n_connected_devices = 1;

        UART_DEBUG_PRINT("At least one device found.");

    }
}

void __attribute__((interrupt, no_auto_psv)) _DMA1Interrupt(void) {
    //UART_DEBUG_PRINT("DMA TX CAN DONE");

    _DMA1IF = 0; // Clear the DMA1 Interrupt Flag;
}

void __attribute__((interrupt, no_auto_psv)) _DMA2Interrupt(void) {
    can_message_t cm;
    message_t m;

    //toggle_led();
    
    while (C1FIFObits.FBP != C1FIFObits.FNRB) {
        // handle message
        parse_from_can_buffer(&cm, ecan_message_buffer[C1FIFObits.FNRB]);
        parse_from_can_message(&m, &cm);
        message_process(&m);
        
        // clear buffer full interrupt bit
        if (C1FIFObits.FNRB > 15) {
            CLEAR_BIT(&C1RXFUL2, C1FIFObits.FNRB - 16);
        } else {
            CLEAR_BIT(&C1RXFUL1, C1FIFObits.FNRB);
        }
    }

    _DMA2IF = 0; // Clear the DMA2 Interrupt Flag;
}

