#include <xc.h>
#include <can.h>
#include <uart.h>
#include <stdio.h>
#include <string.h>
#include <utilities.h>
#include <address.h>

#define NUM_OF_ECAN_BUFFERS 32
#define MAX_CAN_TO_UART_SIZE (2+CAN_MAX_N_BYTES)

uint16_t ecan_message_buffer[NUM_OF_ECAN_BUFFERS][8] __attribute__((aligned(NUM_OF_ECAN_BUFFERS * 16)));

volatile uint8_t __init_sensors = 0;

uint8_t uart_can_message_data[UART_MESSAGE_BUFFER_LENGTH][5 + CAN_MAX_N_BYTES + 2];
uart_message_t uart_can_messages[UART_MESSAGE_BUFFER_LENGTH];
volatile uint8_t uart_can_message_idx = 0;

uart_message_t uart_m;
uint8_t uart_data[CAN_MAX_N_BYTES];

void deactivate_can_bus(void) {
    C1CTRL1bits.REQOP = CAN_MODULE_DISABLE;
    while (C1CTRL1bits.OPMODE != CAN_MODULE_DISABLE); // wait for mode switch
}

void can_init(void) {
    uint16_t i;

    UART_DEBUG_PRINT("Initialised ECAN module");

    // switch ECAN module to configuration mode
    C1CTRL1bits.REQOP = CAN_CONFIG_MODE;
    while (C1CTRL1bits.OPMODE != CAN_CONFIG_MODE); // wait for mode switch

    C1CTRL1bits.WIN = 0; // use buffer control and status window

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

    for (i = 0; i < UART_MESSAGE_BUFFER_LENGTH; i++) {
        uart_can_messages[i].data = uart_can_message_data[i];
        uart_can_messages[i].length = 0;
        uart_can_messages[i].status = UART_MSG_NONE;

        uart_can_message_data[i][0] = UART_CMD_START;
    }

    uart_can_message_idx = 0;
}

void can_disable(void) {
    C1CTRL1bits.ABAT = 1; // abort all transmissions

    C1CTRL1bits.REQOP = 1; // disable CAN module
    while (C1CTRL1bits.OPMODE != 1); // wait for mode switch

    // disable DMA
    DMA2CONbits.CHEN = 0;
    DMA1CONbits.CHEN = 0;
}

void __attribute__((interrupt, no_auto_psv)) _DMA1Interrupt(void) {
    UART_DEBUG_PRINT("DMA TX CAN DONE");

    _DMA1IF = 0; // Clear the DMA0 Interrupt Flag;
}

void __attribute__((interrupt, no_auto_psv)) _DMA2Interrupt(void) {
    can_message_t m;
    uint8_t data[CAN_MAX_N_BYTES];
    m.data = data;

    if (LATBbits.LATB5) {
        LATBbits.LATB5 = 0;
    } else {
        LATBbits.LATB5 = 1;
    }

    // clear buffer full interrupt bit
    while (C1FIFObits.FBP != C1FIFObits.FNRB) {
        can_parse_message(&m, ecan_message_buffer[C1FIFObits.FNRB]);
        can_process_message(&m);

        if (C1FIFObits.FNRB > 15) {
            CLEAR_BIT(&C1RXFUL2, C1FIFObits.FNRB - 16);
        } else {
            CLEAR_BIT(&C1RXFUL1, C1FIFObits.FNRB);
        }
    }

    _DMA2IF = 0; // Clear the DMA2 Interrupt Flag;
}

can_status_t can_send_message(can_message_t* message, uint8_t can_channel) {
    uint16_t i;

    if (C1CTRL1bits.REQOP == CAN_MODULE_DISABLE) {
        return CAN_NOT_ENABLED;
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

static void check_address_match(can_message_t* m) {
    if (m->identifier == controller_address) {
        can_message_t rm;
        can_init_message(&rm, controller_address, CAN_NO_REMOTE_FRAME, CAN_EXTENDED_FRAME, CAN_HEADER(CAN_ADDRESS_TAKEN, 0), NULL, 0);
        can_send_message_any_ch(&rm);
    }
}

void can_tx_uart(can_message_t* can_message) {
    uart_message_t* m = &uart_can_messages[uart_can_message_idx];
    parse_can_to_uart_message(can_message, m);
    uart_reset_message(m);
    uart_queue_message(m);
    uart_can_message_idx = (uart_can_message_idx + 1) % UART_MESSAGE_BUFFER_LENGTH;
}

inline void can_process_message(can_message_t* m) {
    uint8_t sensor_id;
    can_cmd_t cmd;

    cmd = (can_cmd_t) CAN_EXTRACT_HEADER_CMD(m->extended_identifier);
    sensor_id = CAN_EXTRACT_HEADER_ID(m->extended_identifier);

    switch (cmd) {
        case CAN_REQUEST_ADDRESS_AVAILABLE:
            check_address_match(m);

            UART_DEBUG_PRINT("Received address request.");
            break;
        case CAN_ADDRESS_TAKEN:
            UART_DEBUG_PRINT("Requested address already in use.");

            if (controller_address == m->identifier) {
                controller_address = 0;
            }
            break;
        case CAN_UPDATE_ADDRESS:
            if ((m->identifier == controller_address) && (m->data_length == 2)) {
                set_device_address((m->data[0] << 8) | m->data[1]);
            }
            break;
        case CAN_DISCOVERY:
            if (m->identifier == 0) {
                can_init_message(m, controller_address, CAN_NO_REMOTE_FRAME, CAN_EXTENDED_FRAME, CAN_HEADER(CAN_DISCOVERY, 0), NULL, 0);
                can_send_message_any_ch(m);
            } else {
                can_tx_uart(m);
            }
            break;
        case CAN_INFO_MSG_TEXT:
            break;
        case CAN_INFO_MSG_START_MEAS:
            break;
        case CAN_INFO_MSG_STOP_MEAS:
            // TODO go into sleep mode?
            break;
        case CAN_INFO_MSG_RESET:
        {
            uint8_t reset_device = 0;
            if (m->data_length == 2U) {
                if (m->data[1] == controller_address) {
                    reset_device = 1;
                }
            } else {
                reset_device = 1;
            }
            if (reset_device) {
                delay_us(10);
                asm("RESET");
            }
            break;
        }
        case CAN_MSG_SENSOR_STATUS:

            break;
        case CAN_INFO_MSG_ACTUATOR_STATUS:
            // TODO
            break;
        case CAN_INFO_MSG_INIT_SENSORS:
            __init_sensors = 1;
            break;
        case CAN_INFO_MSG_SAMPLE:
            //sample_sensors = 1;
            break;
        case CAN_INFO_MSG_INT_ADC:
            break;
        case CAN_INFO_MSG_INIT_DAC:
            break;
        case CAN_INFO_MSG_SCHEDULE_I2C:
            break;
        case CAN_INFO_MSG_HELLO:
            UART_DEBUG_PRINT("Received HELLO message.");
            break;
        case CAN_INFO_LIA_GAIN_SET:
            if (m->data_length == 4) {
                if (m->data[1] == (controller_address >> 3)) {
                    // TODO
                    //select_gain_pga113(m->data[3]);
                }
            }
            break;
        default:
            break;
    }
}

void can_parse_message(can_message_t* m, uint16_t* raw_data) {
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

    if (((raw_data[2] & 0x10) == 0x10U)) {
        m->remote_frame = 1;
    } else {
        m->remote_frame = 0;

        for (i = 0; i < m->data_length; i++) {
            if (i % 2 == 0) {
                m->data[i] = (uint8_t) raw_data[i / 2 + 3];
            } else {
                m->data[i] = (uint8_t) (raw_data[(i - 1) / 2 + 3] >> 8);
            }
        }
    }

}

void parse_can_to_uart_message(can_message_t* can_message, uart_message_t* uart_message) {
    register uint16_t i = 0;

    uart_init_message(
            uart_message,
            CAN_EXTRACT_HEADER_CMD(can_message->extended_identifier),
            can_message->identifier,
            CAN_EXTRACT_HEADER_ID(can_message->extended_identifier),
            can_message->data,
            can_message->data_length);

    uart_message->id = (uint8_t) (can_message->identifier >> 3);
    uart_message->extended_id = can_message->extended_identifier;
    uart_message->length = can_message->data_length;

    for (i = 0; i < can_message->data_length; i++) {
        uart_message->data[i] = can_message->data[i];
    }
}

void can_cmd_info_rx(uint8_t cmd, uint8_t* data, uint8_t length) {
    if (cmd >= N_CAN_MSG) {
        return;
    }

    switch (cmd) {
        case CAN_INFO_MSG_TEXT:
            if (controller_address == 0) {
                size_t i;

                for (i = 0; i < length; i++) {
                    uart_m.data[i] = data[i];
                }

                uart_init_message(&uart_m,
                        SERIAL_TEXT_MESSAGE_CMD,
                        controller_address,
                        0,
                        uart_data,
                        length);

                uart_queue_message(&uart_m);
            }
            break;
        case CAN_INFO_MSG_START_MEAS:
            break;
        case CAN_INFO_MSG_STOP_MEAS:
            break;
        case CAN_INFO_MSG_RESET:
            __asm__ volatile ("reset");
            break;
        case CAN_MSG_SENSOR_STATUS:
            if (controller_address == 0) {
                size_t i;

                for (i = 0; i < length; i++) {
                    uart_m.data[i] = data[i];
                }
                uart_init_message(&uart_m,
                        SERIAL_SENSOR_STATUS_CMD,
                        controller_address,
                        0,
                        uart_data,
                        length);

                uart_queue_message(&uart_m);
            }
            break;
        case CAN_INFO_MSG_ACTUATOR_STATUS:
            if (controller_address == 0) {
                size_t i;

                for (i = 0; i < length; i++) {
                    uart_m.data[i] = data[i];
                }
                uart_init_message(&uart_m,
                        SERIAL_ACTUATOR_STATUS,
                        controller_address,
                        0,
                        uart_data,
                        length);

                uart_queue_message(&uart_m);
            }
            break;
        case CAN_INFO_MSG_INIT_SENSORS:
            break;
        default:
            return;
    }
}

void can_cmd_info_tx(uint8_t cmd, uint8_t* data, uint8_t length) {
    uint8_t message_data[8];
    uint16_t i;

    length = (length > 7) ? 7 : length;

    message_data[0] = cmd;
    for (i = 0; i < length; i++) {
        message_data[i + 1] = data[i];
    }

    can_message_t m;
    m.identifier = controller_address;
    m.data = message_data;
    m.data_length = length;
    m.remote_frame = 0;
    m.extended_identifier = 0;

    can_send_message_any_ch(&m);
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
