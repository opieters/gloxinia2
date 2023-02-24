#include "can2.h"
#include <libpic30.h>
#include <address.h>
#include "can_bootloader.h"
#include "message.h"

uint16_t ecan_message_buffer[NUM_OF_ECAN_BUFFERS][8] __attribute__((aligned(NUM_OF_ECAN_BUFFERS * 16)));

int16_t n_connected_devices = 0;

// internal functions of message.c
extern bool uart_connection_active;

void can2_init(void)
{
    // switch ECAN module to configuration mode
    C2CTRL1bits.REQOP = CAN_CONFIG_MODE;
    while (C2CTRL1bits.OPMODE != CAN_CONFIG_MODE)
        ; // wait for mode switch

    C2CTRL1bits.WIN = 0; // use buffer control and status window

    /* Set up the CAN module for 250kbps speed with 16 Tq per bit. */
    // C2CTRL1bits.CANCKS = 1; // 1FP, check errata (error in datasheet)
    /*C2CFG1bits.SJW = 3; // sync jump with bits to maintain sync with transmitted message
    C2CFG1bits.BRP = 7; // baud rate prescaler: 2xBRP/F_CAN = TQ
    C2CFG2bits.WAKFIL = 0;
    C2CFG2bits.PRSEG = 4; // propagation time segment bits -> 4TQ
    C2CFG2bits.SEG1PH = 7; // phase segment 1 bits -> length is 6 TQ
    C2CFG2bits.SEG2PH = 5; // phase segment 2 bits in TQ -> 5 TQ
    C2CFG2bits.SEG2PHTS = 1; // phase segment 2 time select bit is freely programmable
    C2CFG2bits.SAM = 1; // CAN line bus is sampled 3 time at the same point
     * */

    /* Set up the CAN module for 250kbps speed with 10 Tq per bit. */
    C2CFG1bits.SJW = 1; // sync jump with bits to maintain sync with transmitted message
    C2CFG1bits.BRP = 7; // baud rate prescaler: 2xBRP/F_CAN = TQ
    C2CFG2bits.WAKFIL = 0;
    C2CFG2bits.SEG2PH = 2;   // phase segment 2 bits in TQ -> 2TQ
    C2CFG2bits.SEG2PHTS = 1; // phase segment 2 time select bit is freely programmable
    C2CFG2bits.SAM = 1;      // CAN line bus is sampled 3 time at the same point
    C2CFG2bits.SEG1PH = 2;   // phase segment 1 bits -> length is 5 TQ
    C2CFG2bits.PRSEG = 2;    // propagation time segment bits -> 3TQ

    /* Configure ECAN module buffers */
    C2FCTRLbits.DMABS = 0b110; // 32 buffers in device RAM
    C2FCTRLbits.FSA = 0b01000; // FIFO start address = read buffer RB8
    // C2FCTRLbits.FSA = 0b11111; // same as example now

    C2CTRL1bits.WIN = 1; // use filter control and status window

    // configure all filter mask 0
    C2FMSKSEL1 = 0;
    C2FMSKSEL2 = 0;

    // configure all filters to store messages in FIFO
    C2BUFPNT1 = 0xFFFF;
    C2BUFPNT2 = 0xFFFF;
    C2BUFPNT3 = 0xFFFF;
    C2BUFPNT4 = 0xFFFF;

    /*if(controller_address != 0U){
        // configure mask 0 to ignore last 3 bits bits
        C2RXM0SIDbits.SID = 0x7F8;

        // configure filter 0 to only accept messages from gateway
        C2RXF0SIDbits.SID = 0;

        // configure mask to check for standard and extended (SID) frames
        C2RXM0SIDbits.MIDE = 0;
    } else {*/
    // configure mask 0 to ignore all bits
    C2RXM0SIDbits.SID = 0b0;
    C2RXM0SIDbits.EID = 0;
    C2RXM0EIDbits.EID = 0;

    // configure mask to check for standard and extended (SID) frames
    C2RXM0SIDbits.MIDE = 0;
    //}

    C2CTRL1bits.WIN = 0;

    // interrupt configuration
    // C2INTEbits.IVRIE = 0; // invalid message interrupt disabled
    // C2INTEbits.WAKIE = 0; // bus wake-up activity interrupt disabled
    // C2INTEbits.ERRIE = 1; // error interrupt enable bit enabled TODO: interrupt service!
    // C2INTEbits.FIFOIE = 0; // FIFO almost full interrupt enable off
    // C2INTEbits.RBOVIE = 0; // buffer overflow interrupt disabled
    // C2INTEbits.RBIE = 1; // RX buffer interrupt enabled
    // C2INTEbits.TBIE = 1; // TX buffer interrupt disabled

    // enable ECAN interrupts
    //_C2IE = 1;

    // configure first 8 buffers for sending data
    C2TR01CONbits.TXEN0 = 1;  // configure buffer 0 for transmission
    C2TR01CONbits.TX0PRI = 3; // assign priority
    C2TR01CONbits.TXEN1 = 1;  // configure buffer 1 for transmission
    C2TR01CONbits.TX1PRI = 3; // assign priority
    C2TR23CONbits.TXEN2 = 1;  // configure buffer 2 for transmission
    C2TR23CONbits.TX2PRI = 3; // assign priority
    C2TR23CONbits.TXEN3 = 1;  // configure buffer 3 for transmission
    C2TR23CONbits.TX3PRI = 3; // assign priority
    C2TR45CONbits.TXEN4 = 1;  // configure buffer 4 for transmission
    C2TR45CONbits.TX4PRI = 3; // assign priority
    C2TR45CONbits.TXEN5 = 1;  // configure buffer 5 for transmission
    C2TR45CONbits.TX5PRI = 3; // assign priority
    C2TR67CONbits.TXEN6 = 1;  // configure buffer 6 for transmission
    C2TR67CONbits.TX6PRI = 3; // assign priority
    C2TR67CONbits.TXEN7 = 1;  // configure buffer 7 for transmission
    C2TR67CONbits.TX7PRI = 3; // assign priority

    // other buffers are for receiving data

    // configure TX DMA channel
    DMA13CONbits.SIZE = 0x0;                  // Word Transfer Mode
    DMA13CONbits.DIR = 0x1;                   // Data Transfer Direction: device RAM to Peripheral
    DMA13CONbits.AMODE = 0x2;                 // peripheral indirect addressing mode
    DMA13CONbits.MODE = 0x0;                  // Operating Mode: Continuous, Ping-Pong modes disabled
    DMA13REQ = 0b01000110;                    // select TX data ready as IRQ
    DMA13CNT = 7;                             // 8 DMA Transfers per ECAN message
    DMA13PAD = (volatile unsigned int)&C2TXD; // ECAN transmit register

    // data source to be transferred
    DMA13STAL = (unsigned int)&ecan_message_buffer;
    DMA13STAH = 0; //(unsigned int) &ecan_message_buffer;

    _DMA13IE = 1;          // enable DMA channel interrupt
    _DMA13IF = 0;          // clear DMA interrupt flag
    DMA13CONbits.CHEN = 1; // enable DMA channel

    // configure RX DMA channel
    DMA14CONbits.SIZE = 0;                    // Word Transfer Mode
    DMA14CONbits.DIR = 0;                     // Data Transfer Direction: device RAM to Peripheral
    DMA14CONbits.AMODE = 0x2;                 // peripheral indirect addressing mode
    DMA14CONbits.MODE = 0x0;                  // Operating Mode: Continuous, Ping-Pong modes disabled
    DMA14REQ = 0b00100010;                    // select RX data ready as IRQ
    DMA14CNT = 7;                             // 8 DMA Transfers per ECAN message
    DMA14PAD = (volatile unsigned int)&C2RXD; // ECAN receive register

    // data source to be transferred
    DMA14STAL = (unsigned int)&ecan_message_buffer;
    DMA14STAH = 0; // (unsigned int) &ecan_message_buffer;

    _DMA14IE = 1;          // enable DMA channel interrupt
    _DMA14IF = 0;          // clear DMA interrupt flag
    DMA14CONbits.CHEN = 1; // enable DMA channel

    // switch ECAN module to normal operation mode
    C2CTRL1bits.REQOP = CAN_MODULE_ENABLE;
    while (C2CTRL1bits.OPMODE != CAN_MODULE_ENABLE)
        ; // wait for mode switch
}

void can2_disable(void)
{
    C2CTRL1bits.ABAT = 1; // abort all transmissions

    C2CTRL1bits.REQOP = CAN_MODULE_DISABLE; // disable CAN module
    while (C2CTRL1bits.OPMODE != CAN_MODULE_DISABLE)
        ; // wait for mode switch

    // disable DMA
    DMA14CONbits.CHEN = 0;
    DMA13CONbits.CHEN = 0;
    
    _DMA13IE = 0;
    _DMA14IE = 0;
    _DMA13IF = 0;
    _DMA14IF = 0;
}

void can_reset(void) {
    // when the module is still running fine, do not restart
    if(C2CTRL1bits.OPMODE == CAN_MODULE_ENABLE){
        return;
    }

    _TRISE4 = 0;
    _RP84R = 0;
    _RE4 = 0;
    __delay_ms(10);
    _RE4 = 1;
    __delay_ms(10);
    _RP84R = _RPOUT_C2TX;

    can2_init();
}

bool parse_from_can2_buffer(can2_message_t *m, uint16_t *raw_data)
{
    uint16_t i, identifier, extended_identifier;
    bool extended_frame = false, remote_frame = false;
    
    identifier = raw_data[0] >> 2;
    if ((raw_data[0] & 0x03) == 0b11)
    {
        extended_frame = 1;
        extended_identifier = (((uint32_t)raw_data[1]) << 6) | (raw_data[2] >> 10);
    }
    else
    {
        extended_frame = 0;
        extended_identifier = 0;
    }
    
    m->command = (extended_identifier >> 8) & 0xFFU;
    m->unlock = (extended_identifier & 0xFFU) == CAN2_UNLOCK_SEQUENCE ? true : false;

    m->length = raw_data[2] & 0xF;

    if (extended_frame)
    {
        if (((raw_data[2] & 0x200U) == 0x200U))
        {
            remote_frame = true;
        }
        else
        {
            remote_frame = false;
        }
    }
    else
    {
        if ((raw_data[0] & 0b10) == 0b10)
        {
            remote_frame = true;
        }
        else
        {
            remote_frame = false;
        }
    }
    if (remote_frame == false)
    {
        for (i = 0; i < m->length; i++)
        {
            if (i % 2 == 0)
            {
                m->data[i] = (uint8_t)raw_data[i / 2 + 3];
            }
            else
            {
                m->data[i] = (uint8_t)(raw_data[(i - 1) / 2 + 3] >> 8);
            }
        }
    }
    
    return (!remote_frame) && (extended_frame) && (identifier == 0U);
}


can_status_t can2_send_message(can2_message_t *message, uint8_t can_channel)
{
    uint16_t i;

    if (C2CTRL1bits.OPMODE == CAN_MODULE_DISABLE)
    {
        return CAN_NOT_ENABLED;
    }

    switch (can_channel)
    {
    case 0:
        if (C2TR01CONbits.TXREQ0 == 1)
        {
            return CAN_TX_PENDING;
        }
        break;
    case 1:
        if (C2TR01CONbits.TXREQ1 == 1)
        {
            return CAN_TX_PENDING;
        }
        break;
    case 2:
        if (C2TR23CONbits.TXREQ2 == 1)
        {
            return CAN_TX_PENDING;
        }
        break;
    case 3:
        if (C2TR23CONbits.TXREQ3 == 1)
        {
            return CAN_TX_PENDING;
        }
        break;
    case 4:
        if (C2TR45CONbits.TXREQ4 == 1)
        {
            return CAN_TX_PENDING;
        }
        break;
    case 5:
        if (C2TR45CONbits.TXREQ5 == 1)
        {
            return CAN_TX_PENDING;
        }
        break;
    case 6:
        if (C2TR67CONbits.TXREQ6 == 1)
        {
            return CAN_TX_PENDING;
        }
        break;
    case 7:
        if (C2TR67CONbits.TXREQ7 == 1)
        {
            return CAN_TX_PENDING;
        }
        break;
    default:
        break;
    }
    
    uint16_t identifier = (message->command << 8);
    if(message->unlock)
        identifier |= (CAN2_UNLOCK_SEQUENCE);

    ecan_message_buffer[can_channel][0] = ((ADDRESS_GATEWAY & 0x7FFU) << 2) | (false == 0 ? 0b00 : 0b11) | (false == 0 ? 0b00 : 0b10);
    ecan_message_buffer[can_channel][1] = (identifier >> 6) & 0xFFFU;
    ecan_message_buffer[can_channel][2] = ((identifier << 10) & 0xFC00U) | (false == 0U ? 0x000U : 0x200U) | (message->length & 0xFU);
    for (i = 0; i < message->length; i++)
    {
        if ((i % 2U) == 0)
        {
            ecan_message_buffer[can_channel][3 + i / 2] = message->data[i];
        }
        else
        {
            ecan_message_buffer[can_channel][3 + (i - 1) / 2] = ecan_message_buffer[can_channel][3 + (i - 1) / 2] | (((uint16_t)message->data[i]) << 8);
        }
    }

    switch (can_channel)
    {
    case 0:
        C2TR01CONbits.TXREQ0 = 1;
        break;
    case 1:
        C2TR01CONbits.TXREQ1 = 1;
        break;
    case 2:
        C2TR23CONbits.TXREQ2 = 1;
        break;
    case 3:
        C2TR23CONbits.TXREQ3 = 1;
        break;
    case 4:
        C2TR45CONbits.TXREQ4 = 1;
        break;
    case 5:
        C2TR45CONbits.TXREQ5 = 1;
        break;
    case 6:
        C2TR67CONbits.TXREQ6 = 1;
        break;
    case 7:
        C2TR67CONbits.TXREQ7 = 1;
        break;
    default:
        break;
    }
    return CAN_NO_ERROR;
}

can_status_t can2_send_message_any_ch(can2_message_t *m)
{
    static uint16_t i = 0;
    uint16_t n_attempts = 0;
    can_status_t sent_status = CAN_NO_ERROR;
    do
    {
        sent_status = can2_send_message(m, i);
        i = (i + 1) % 8;
        n_attempts++;
    } while ((sent_status != CAN_NO_ERROR) && (n_attempts < 8U));
    return sent_status;
}


void can_detect_devices(void)
{
    can2_message_t m;
    bool no_device_found = false;
    
    m.command = M_BOOT_READY;
    m.unlock = false;
    m.length = 0;

    
    can2_send_message_any_ch(&m);

    __delay_ms(100);

    // check CAN status
    if (C2TR01CONbits.TXREQ0 == 1)
    {
        no_device_found = true;
    }
    if (C2TR01CONbits.TXREQ1 == 1)
    {
        no_device_found = true;
    }
    if (C2TR23CONbits.TXREQ2 == 1)
    {
        no_device_found = true;
    }
    if (C2TR23CONbits.TXREQ3 == 1)
    {
        no_device_found = true;
    }
    if (C2TR45CONbits.TXREQ4 == 1)
    {
        no_device_found = true;
    }
    if (C2TR45CONbits.TXREQ5 == 1)
    {
        no_device_found = true;
    }
    if (C2TR67CONbits.TXREQ6 == 1)
    {
        no_device_found = true;
    }
    if (C2TR67CONbits.TXREQ7 == 1)
    {
        no_device_found = true;
    }

    // disable CAN if needed
    if (no_device_found)
    {
        n_connected_devices = 0;

        can2_disable();

        UART_DEBUG_PRINT("No devices found.");
    }
    else
    {
        n_connected_devices = 1;

        UART_DEBUG_PRINT("At least one device found.");
    }
}

void __attribute__((interrupt, no_auto_psv)) _DMA13Interrupt(void)
{
    // UART_DEBUG_PRINT("DMA TX CAN DONE");

    _DMA13IF = 0; // Clear the DMA13 Interrupt Flag;
}

void __attribute__((interrupt, no_auto_psv)) _DMA14Interrupt(void)
{
    can2_message_t m;
    bool status;

    // toggle_led();

    while (C2FIFObits.FBP != C2FIFObits.FNRB)
    {
        // handle message
        status = parse_from_can2_buffer(&m, ecan_message_buffer[C2FIFObits.FNRB]);

        if (status)
        {
            boot_process_command(&m);
        }
        else
        {

        }

        // clear buffer full interrupt bit
        if (C2FIFObits.FNRB > 15)
        {
            CLEAR_BIT(&C2RXFUL2, C2FIFObits.FNRB - 16);
        }
        else
        {
            CLEAR_BIT(&C2RXFUL1, C2FIFObits.FNRB);
        }
    }

    _DMA14IF = 0; // Clear the DMA14 Interrupt Flag;
}

void copy_can2_message(can2_message_t* m1, can2_message_t* m2)
{
    m2->command = m1->command;
    m2->length = m1->length;
    m2->unlock = m1->unlock;
    
    for(unsigned int i = 0; i < m1->length; i++)
    {
        m2->data[i] = m1->data[i];
    }
}