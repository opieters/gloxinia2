#include <can.h>
#include <address.h>
#include <libpic30.h>
#include "message.h"

__eds__ uint16_t ecan_message_buffer[NUM_OF_ECAN_BUFFERS][ECAN_BUFFER_SIZE] __attribute__((space(dma), eds, aligned(NUM_OF_ECAN_BUFFERS * ECAN_BUFFER_SIZE * sizeof(uint16_t))));

int16_t n_connected_devices = 0;

// internal functions of message.c
extern bool uart_connection_active;

void can_tx_dma_init(void) {
    // configure TX DMA channel
    DMA1CONbits.SIZE = 0x0; // Word Transfer Mode
    DMA1CONbits.DIR = 0x1; // Data Transfer Direction: device RAM to Peripheral
    DMA1CONbits.AMODE = 0x2; // peripheral indirect addressing mode
    DMA1CONbits.MODE = 0x0; // Operating Mode: Continuous, Ping-Pong modes disabled
    DMA1REQ = 0b01000110; // select TX data ready as IRQ
    DMA1CNT = ECAN_BUFFER_SIZE-1; // 8 DMA Transfers per ECAN message
    DMA1PAD = (volatile unsigned int) &C1TXD; // ECAN transmit register

    DMA1STAL = __builtin_dmaoffset(ecan_message_buffer);
    DMA1STAH = __builtin_dmapage(ecan_message_buffer);

    DMA1CONbits.CHEN = 1;
}

void can_rx_dma_init(void) {
    DMA2CONbits.SIZE = 0; // Word Transfer Mode
    DMA2CONbits.DIR = 0; // Data Transfer Direction: device RAM to Peripheral
    DMA2CONbits.AMODE = 0x2; // peripheral indirect addressing mode
    DMA2CONbits.MODE = 0x0; // Operating Mode: Continuous, Ping-Pong modes disabled
    DMA2REQ = 0b00100010; // select RX data ready as IRQ
    DMA2CNT = ECAN_BUFFER_SIZE-1; // 8 DMA Transfers per ECAN message
    DMA2PAD = (volatile unsigned int) &C1RXD; // ECAN receive register

    DMA2STAL = __builtin_dmaoffset(ecan_message_buffer);
    DMA2STAH = __builtin_dmapage(ecan_message_buffer);
    DMA2CONbits.CHEN = 1;
}

void can_init(void) {
    // clear ECAN memory
    for(int i = 0; i < NUM_OF_ECAN_BUFFERS; i++)
    {
        for(int j = 0; j < CAN_MAX_N_BYTES; j++) 
        {
            ecan_message_buffer[i][j] = 0;
        }
    }
    
    // switch ECAN module to configuration mode
    C1CTRL1bits.REQOP = CAN_CONFIG_MODE;
    while (C1CTRL1bits.OPMODE != CAN_CONFIG_MODE)
        ; // wait for mode switch

    C1CTRL1bits.WIN = 0; // use buffer control and status window

    /* Set up the CAN module for 250kbps speed with 16 Tq per bit. */
    // C1CTRL1bits.CANCKS = 1; // 1FP, check errata (error in datasheet)
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
    C1CFG1bits.SJW = 3; // sync jump with bits to maintain sync with transmitted message
    C1CFG1bits.BRP = 7; // baud rate prescaler: 2xBRP/F_CAN = TQ
    C1CFG2bits.WAKFIL = 0;
    C1CFG2bits.SEG2PH = 5; // phase segment 2 bits in TQ -> 2TQ
    C1CFG2bits.SEG2PHTS = 1; // phase segment 2 time select bit is freely programmable
    C1CFG2bits.SAM = 1; // CAN line bus is sampled 3 time at the same point
    C1CFG2bits.SEG1PH = 7; // phase segment 1 bits -> length is 5 TQ
    C1CFG2bits.PRSEG = 4; // propagation time segment bits -> 3TQ

    /* Configure ECAN module buffers */
    C1FCTRLbits.DMABS = 0b100; // 16 buffers in device RAM
    C1FCTRLbits.FSA = 0b01000; // FIFO start address = read buffer RB8
    // C1FCTRLbits.FSA = 0b11111; // same as example now

    // setup filter and mask to receive messages from gateway and logging node
    can_configure_filter(0, ADDRESS_GATEWAY, 0x0, true, 0, CAN_FIFO_BUFFER);
    can_configure_filter(1, ADDRESS_SEARCH_START, 0x0, true, 0, CAN_FIFO_BUFFER);

    // configure mask 0 to filter for specific addresses
    can_configure_mask(0, 0x7ff, 0x00000, true);

    // dicio nodes process all messages
#ifdef __DICIO__
    // setup filter and mask to receive all messages on the bus
    can_configure_filter(2, 0x0, 0x0, true, 1, CAN_FIFO_BUFFER);

    can_configure_mask(1, 0x000, 0x00000, true);
#endif

    // address request and allocation
    // setup filter for address requests
    can_configure_filter(3, 0x0, CAN_HEADER(M_REQUEST_ADDRESS_AVAILABLE, NO_INTERFACE_ID, NO_SENSOR_ID), true, 2, CAN_FIFO_BUFFER);

    // setup filter for address taken
    can_configure_filter(4, 0x0, CAN_HEADER(M_ADDRESS_TAKEN, NO_INTERFACE_ID, NO_SENSOR_ID), true, 2, CAN_FIFO_BUFFER);

    // mask for command filtering
    can_configure_mask(2, 0x0000, 0x3FF00, true);
    
    /* ECAN transmit/receive message control */
    C1RXFUL1 = C1RXFUL2 = C1RXOVF1 = C1RXOVF2 = 0x0000;
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

    /* Enter Normal Mode */
    C1CTRL1bits.REQOP = CAN_MODULE_ENABLE;
    while (C1CTRL1bits.OPMODE != CAN_MODULE_ENABLE);

    can_tx_dma_init();
    can_rx_dma_init();

    /* Enable ECAN1 Interrupt */
    _C1IE = 1;
    C1INTEbits.TBIE = 1;
    C1INTEbits.RBIE = 1;
    C1INTEbits.RBOVIE = 1;
}

void can_configure_filter(uint8_t n, uint16_t sid, uint32_t eid, bool exide, uint8_t mask, uint8_t target) {

    uint16_t    *sid_reg;
    uint16_t    *eid_reg;
    uint16_t    *buffer_reg;
    uint16_t    *mask_sel_reg;
    
    const can_module_status_t can_mode = C1CTRL1bits.OPMODE;
    
    // set CAN module into configuration mode if needed
    if(can_mode != CAN_CONFIG_MODE){
        C1CTRL1bits.REQOP = CAN_CONFIG_MODE;
        while (C1CTRL1bits.OPMODE != CAN_CONFIG_MODE)
            ; // wait for mode switch
    }
    
    C1CTRL1bits.WIN = 1;
    
    // make sure values are within range
    n &= 0xf;
    sid &= 0x7ff;
    eid &= 0x3ffff;
    exide &= 0b1;
    target &= 0xf;
    mask &= 0b11;

    sid_reg = ( uint16_t * ) ( &C1RXF0SID + (n << 1) );
    eid_reg = ( uint16_t * ) ( &C1RXF0EID + (n << 1) );
    buffer_reg = ( uint16_t * ) ( &C1BUFPNT1 + (n >> 2) );
    mask_sel_reg = ( uint16_t * ) ( &C1FMSKSEL1 + (n >> 3) );

    *sid_reg = (sid << 5) | (exide << 3) | ((uint16_t) (eid >> 16) & 0b11);
    *eid_reg = (uint16_t) (eid & 0xffff);
    *mask_sel_reg = (*mask_sel_reg) & (~(0b11 << (2*(n % 8))));
    *mask_sel_reg = (*mask_sel_reg) | (mask << (2*(n % 8)));
    *buffer_reg = (*buffer_reg) & (~(0b1111 << (4*(n % 4))));
    *buffer_reg = (*buffer_reg) | (target << (4*(n % 4)));
    
    SET_BIT(&C1FEN1, n);
    
    C1CTRL1bits.WIN = 0;
    
    // set CAN module into configuration mode if needed
    if(can_mode != CAN_CONFIG_MODE){
        C1CTRL1bits.REQOP = can_mode;
        while (C1CTRL1bits.OPMODE != can_mode)
            ; // wait for mode switch
    }
}

void can_configure_mask(uint8_t n, int16_t sid, uint32_t eid, bool mide) 
{    
    uint16_t *sid_reg, *eid_reg;
    const can_module_status_t can_mode = C1CTRL1bits.OPMODE;
    
    // set CAN module into configuration mode if needed
    if(can_mode != CAN_CONFIG_MODE){
        C1CTRL1bits.REQOP = CAN_CONFIG_MODE;
        while (C1CTRL1bits.OPMODE != CAN_CONFIG_MODE)
            ; // wait for mode switch
    }

    C1CTRL1bits.WIN = 1;
    
    // make sure values are within range
    n &= 0xf;
    sid &= 0x7ff;
    eid &= 0x3ffff;
    mide &= 0b1;

    // get register
    sid_reg = (uint16_t *) (&C1RXM0SID + (n << 1));
    eid_reg = (uint16_t *) (&C1RXM0EID + (n << 1));
    
    // set registers
    *sid_reg = (sid << 5) | (mide << 3) | ((uint16_t) (eid >> 16)); // Write to CiRXMnEID Register
    *eid_reg = (uint16_t) (eid & 0xffff); // Write to CiRXMnEID Register

    C1CTRL1bits.WIN = 0;
    
    // set CAN module into configuration mode if needed
    if(can_mode != CAN_CONFIG_MODE){
        C1CTRL1bits.REQOP = can_mode;
        while (C1CTRL1bits.OPMODE != can_mode)
            ; // wait for mode switch
    }
}

void can_disable(void) {
    C1CTRL1bits.ABAT = 1; // abort all transmissions

    C1CTRL1bits.REQOP = CAN_MODULE_DISABLE; // disable CAN module
    while (C1CTRL1bits.OPMODE != CAN_MODULE_DISABLE)
        ; // wait for mode switch

    // disable DMA
    DMA2CONbits.CHEN = 0;
    DMA1CONbits.CHEN = 0;
}

void can_reset(void) {
    // when the module is still running fine, do not restart
    if (C1CTRL1bits.OPMODE == CAN_MODULE_ENABLE) {
        return;
    }

    /*_TRISE4 = 0;
    _RP84R = 0;
    _RE4 = 0;
    __delay_ms(10);
    _RE4 = 1;
    __delay_ms(10);
    _RP84R = _RPOUT_C1TX;*/

    can_init();
}

void can_init_message(can_message_t *m,
        const uint16_t identifier,
        const uint8_t remote_frame,
        const uint8_t extended_frame,
        const uint32_t extended_identifier,
        const uint8_t* data,
        const uint8_t data_length) {
    m->identifier = identifier;
    m->remote_frame = remote_frame;
    m->extended_frame = extended_frame;
    m->extended_identifier = extended_identifier;
    m->data_length = MIN(data_length, CAN_MAX_N_BYTES);
    for(int i = 0; i <  m->data_length; i++){
        m->data[i] = data[i];
    }
}

void parse_from_can_buffer(can_message_t *m, uint16_t buffer_index) {
    m->identifier = ecan_message_buffer[buffer_index][0] >> 2;
    if((ecan_message_buffer[buffer_index][0] & 0b1) == 0b1){
        m->extended_frame = 1;
    }else {
        m->extended_frame = 0;
    }
    if(m->extended_frame)
    {
        m->remote_frame = (ecan_message_buffer[buffer_index][2] & 0x200U) == 0x200U;
        m->extended_identifier = (((uint32_t) ecan_message_buffer[buffer_index][1]) << 6) | (ecan_message_buffer[buffer_index][2] >> 10);
    } else {
        m->remote_frame = (ecan_message_buffer[buffer_index][0] & 0b10) == 0b10;
        m->extended_identifier = 0;
    }
    
    /*if ((raw_data[0] & 0x03) == 0b11) {
        m->extended_frame = 1;
        m->extended_identifier = (((uint32_t) raw_data[1]) << 6) | (raw_data[2] >> 10);
    } else {
        m->extended_frame = 0;
        m->extended_identifier = 0;
    }*/

    m->data_length = ecan_message_buffer[buffer_index][2] & 0xF;

    /*if (m->extended_frame) {
        if (((raw_data[2] & 0x200U) == 0x200U)) {
            m->remote_frame = 1;
        } else {
            m->remote_frame = 0;
        }
    } else {
        if ((raw_data[0] & 0b10) == 0b10) {
            m->remote_frame = 1;
        } else {
            m->remote_frame = 0;
        }
    }*/
    if (m->remote_frame == 0) {
        m->data[0] = (uint8_t) ecan_message_buffer[buffer_index][0 + 3];
        m->data[1] = (uint8_t) (ecan_message_buffer[buffer_index][0 + 3] >> 8);
        m->data[2] = (uint8_t) ecan_message_buffer[buffer_index][1 + 3];
        m->data[3] = (uint8_t) (ecan_message_buffer[buffer_index][1 + 3] >> 8);
        m->data[4] = (uint8_t) ecan_message_buffer[buffer_index][2 + 3];
        m->data[5] = (uint8_t) (ecan_message_buffer[buffer_index][2 + 3] >> 8);
        m->data[6] = (uint8_t) ecan_message_buffer[buffer_index][3 + 3];
        m->data[7] = (uint8_t) (ecan_message_buffer[buffer_index][3 + 3] >> 8);
        /*for (i = 0; i < CAN_MAX_N_BYTES; i++) {
            if (i % 2 == 0) {
                m->data[i] = (uint8_t) raw_data[i / 2 + 3];
            } else {
                m->data[i] = (uint8_t) (raw_data[(i - 1) / 2 + 3] >> 8);
            }
        }*/
    }
}

void parse_from_can_message(message_t *m, can_message_t *cm) {
    message_init(m, cm->identifier,
            cm->remote_frame,
            (message_cmd_t) cm->extended_identifier >> 8,
            (cm->extended_identifier >> 4) & 0xf,
            cm->extended_identifier & 0xf,
            cm->data,
            cm->data_length);
    m->status = M_RX_FROM_CAN;
}

can_status_t can_send_message(can_message_t *message, uint8_t can_channel) {
    uint16_t i;

    if (C1CTRL1bits.OPMODE == CAN_MODULE_DISABLE) {
        return CAN_NOT_ENABLED;
    }

    if (controller_address == ADDRESS_UNSET) {
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

can_status_t can_send_message_any_ch(can_message_t *m) {
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

void can_message_from_fmessage(can_message_t *cm, const message_t *m) {
    cm->identifier = m->identifier;
    cm->remote_frame = m->request_message_bit;
    cm->extended_frame = CAN_EXTENDED_FRAME;
    cm->extended_identifier = CAN_HEADER(m->command, m->interface_id, m->sensor_id);
    cm->data_length = m->length;
    for(int i = 0; i < m->length; i++)
        cm->data[i] = m->data[i];
}

can_status_t can_send_fmessage_any_ch(const message_t *m) {
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
            NO_INTERFACE_ID,
            NO_SENSOR_ID,
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
    } else {
        n_connected_devices = 1;

        UART_DEBUG_PRINT("At least one device found.");
    }
}

// DMA interrupts are not used, only for data transfer. CAN interrupt is more 
// informative and provides better handling of RX/TX messages
void __attribute__((interrupt, no_auto_psv)) _DMA1Interrupt(void)
{
    _DMA1IF = 0; // Clear the DMA1 Interrupt Flag;
}

void __attribute__((interrupt, no_auto_psv)) _DMA2Interrupt(void)
{
    _DMA2IF = 0; // Clear the DMA2 Interrupt Flag;
}


void __attribute__((interrupt, no_auto_psv)) _C1RxRdyInterrupt(void)
{
    _C1RXIF = 0;
}
void __attribute__((interrupt, auto_psv)) _C1Interrupt(void)
{
    _C1IF = 0; 
    // toggle_led();
    
    if( C1INTFbits.TBIF )
    {
        C1INTFbits.TBIF = 0;
    }

    if(C1INTFbits.RBIF) {
        
        can_message_t cm;
        message_t m;
        
        while (C1FIFObits.FBP != C1FIFObits.FNRB)
        {
            // handle message
            int i = C1FIFObits.FNRB;
            
            parse_from_can_buffer(&cm, i);
            parse_from_can_message(&m, &cm);
            
            // log every message that we received
            send_message_uart(&m);
            
            // if we received a message for this node, we need to process it
            if((m.identifier == controller_address) || (m.identifier == ADDRESS_GATEWAY))
            {
                 message_process(&m);
            } 
            //else // special case: the node with address ADDRESS_SEARCH_START needs to process all messages
            //{
            //    message_process(&m);
            //}

            // clear buffer full interrupt bit
            if (C1FIFObits.FNRB > 15)
            {
                CLEAR_BIT(&C1RXFUL2, C1FIFObits.FNRB - 16);
            }
            else
            {
                CLEAR_BIT(&C1RXFUL1, C1FIFObits.FNRB);
            }
        }
        
        C1INTFbits.RBIF = 0;
    }
    
    if(C1INTFbits.RBOVIF)
    {
        C1RXOVF1 = 0;
        C1RXOVF2 = 0;
        C1INTFbits.RBOVIF = 0;
    }
    if(C1INTFbits.FIFOIF)
    {
        C1INTFbits.FIFOIF = 0;
    }
    if(C1INTFbits.IVRIF)
    {
        C1INTFbits.IVRIF = 0;
    }
}