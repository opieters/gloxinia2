#include <uart.h>
#include <stdint.h>
#include <utilities.h>
#include <device_configuration.h>
#include <can.h>
#include <address.h>


// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************

volatile size_t n_uart_rx_messages = 0;
volatile size_t uart_rx_read_idx = 0; // current or previous read index



uart_message_t* uart_queue[UART_MESSAGE_BUFFER_LENGTH];




volatile uint8_t n_uart_messages = 0;
volatile uint8_t uart_queue_idx = 0;
volatile uint8_t uart_queue_valid = 0;
volatile uint8_t uart_ongoing = 0;

uart_message_t uart_loopback;
uint8_t uart_loopback_data[PRINT_BUFFER_LENGTH];

volatile uint8_t uart_rx_state = 0;
volatile uint8_t uart_rx_idx = 0;


uart_message_t uart_command_message;
uint8_t uart_command_message_data[UART_MESSAGE_BUFFER_LENGTH];

void (*uart_rx_command_cb)(void) = uart_rx_callback;

uart_message_t uart_print_message = {.status = UART_MSG_SENT};
uint8_t uart_print_message_data[128];

uint8_t uart_dma_message_data[128];

uart_message_t uart_rx_queue[UART_RX_BUFFER_SIZE];
uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE][PRINT_BUFFER_LENGTH] __attribute__((space(xmemory)));

uart_message_t uart_discovery_m;

void uart_log_init(uint32_t baudrate) {

    unsigned int i;

    U2MODEbits.STSEL = 0; // 1-stop bit
    U2MODEbits.PDSEL = 0; // No Parity, 8-data bits
    U2MODEbits.ABAUD = 0; // Autobaud Disabled
    U2MODEbits.BRGH = 0; // 16x mode (16 clock cycles per data bit)
    U2MODEbits.RTSMD = 0; // DTE-DTE mode
    U2MODEbits.URXINV = 0;
    U2BRG = ((FCY / baudrate) / 16) - 1; // BAUD Rate Setting for 9600

    U2MODEbits.UEN = 0b00; // do not use flow control

    //  Configure UART for DMA transfers
    U2STAbits.UTXISEL0 = 0; // Interrupt after one Tx character is transmitted
    U2STAbits.UTXISEL1 = 0;
    U2STAbits.URXISEL = 0; // Interrupt after one RX character is received
    U2STAbits.TXINV = 0;

    //  Enable UART Rx and Tx
    U2MODEbits.UARTEN = 1; // Enable UART
    U2STAbits.UTXEN = 1; // Enable UART Tx

    // clear error bit, do not enable error interrupt
    _U2EIF = 0;
    _U2EIE = 0;

    // only enable RX interrupt
    _U2RXIE = 1;
    _U2TXIE = 0;
    _U2EIE = 1;

    // enable UART RX and TX
    U2MODEbits.UARTEN = 1; // Enable UART
    U2STAbits.UTXEN = 1; // Enable UART Tx

    // wait for at least one baud period to continue
    delay_us(2 * (FCY / baudrate));

    DMA14CONbits.DIR = 1; // RAM-to-Peripheral
    DMA14CONbits.SIZE = 1; // byte transfer mode
    DMA14CONbits.MODE = 0b01; // One-Shot, Ping-Pong modes disabled
    DMA14CNT = 0; // number of  DMA requests
    DMA14REQ = 0x001F; // Select UART2 transmitter
    DMA14PAD = (volatile unsigned int) &U2TXREG;
    _DMA14IF = 0; // Clear DMA Interrupt Flag
    _DMA14IE = 1; // Enable DMA interrupt

    DMA14STAL = (uint16_t) uart_dma_message_data;
    DMA14STAH = 0x0;

    // update interrupt priority 
    _DMA14IP = 7;

    UART_DEBUG_PRINT("Initialised UART module.");

    for (i = 0; i < UART_RX_BUFFER_SIZE; i++) {
        uart_rx_queue[i].status = UART_MSG_NONE;
        uart_rx_queue[i].data = uart_rx_buffer[i];
    }


    uart_init_message(&uart_print_message,
            SERIAL_TEXT_MESSAGE_CMD,
            0,
            0,
            uart_print_message_data,
            0);

    uart_print_message.status = UART_MSG_SENT;
}

void uart_wait() {
    if (U2STAbits.UTXEN == 0) {
        return;
    }

    while (uart_print_message.status != UART_MSG_SENT);
}

void uart_print(const char* message, size_t length) {
    uint16_t i;

    if (U2STAbits.UTXEN == 0) {
        return;
    }

    if (uart_print_message.status != UART_MSG_INIT_DONE) {
        while (uart_print_message.status != UART_MSG_SENT);
    }

    uart_print_message.length = length;
    for (i = 0; i < length; i++) {
        uart_print_message.data[i] = message[i];
    }

    uart_reset_message(&uart_print_message);
    uart_queue_message(&uart_print_message);

    //while(uart_print_message.status != UART_MSG_SENT);
}

void __attribute__((interrupt, no_auto_psv)) _U2ErrInterrupt(void) {
    U2STAbits.OERR = 0;
    _U2EIF = 0; // Clear the UART2 Error Interrupt Flag
}

void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void) {

    register uint8_t rx_value = U2RXREG;
    register uart_message_t* m = &(uart_rx_queue[(n_uart_rx_messages + uart_rx_read_idx) % UART_RX_BUFFER_SIZE]);

    if ((n_uart_rx_messages == UART_RX_BUFFER_SIZE) && m->status != UART_MSG_NONE) {
        _U2RXIF = 0;
    }



    switch (uart_rx_state) {
        case 0:
            if (rx_value == UART_CMD_START) {
                uart_rx_state = 1;
            }
            break;
        case 1:
            m->command = rx_value;
            m->status = UART_MSG_TRANSFERRED;

            uart_rx_state = 2;
            break;
        case 2:
            m->command = (m->command << 8) | rx_value;
            uart_rx_state = 3;
            break;
        case 3:
            m->id = rx_value;
            uart_rx_state = 4;

            break;
        case 4:
            m->extended_id = rx_value;
            uart_rx_state = 5;

            break;
        case 5:
            m->extended_id = (m->extended_id << 8) + rx_value;
            uart_rx_state = 6;
            break;
        case 6:
            m->length = rx_value;
            if (rx_value > 0) {
                uart_rx_state = 7;
                uart_rx_idx = 0;
            } else {
                uart_rx_state = 8;
            }

            break;
        case 7:
            // only write data until buffer is full
            if (uart_rx_idx < PRINT_BUFFER_LENGTH) {
                m->data[uart_rx_idx] = rx_value;
            } else {
                // continue reading data, but indicate error occurred
                m->status = UART_MSG_ERROR;
            }

            uart_rx_idx++;

            if (uart_rx_idx == m->length) {
                uart_rx_state = 8;
            }

            break;
        case 8:
            if (rx_value == UART_CMD_STOP) {
                if (m->status != UART_MSG_ERROR) {
                    m->status = UART_MSG_RECEIVED;
                }
            } else {
                m->status = UART_MSG_ERROR;
            }

            n_uart_rx_messages++;
            uart_rx_state = 0;

            uart_rx_command_cb();

            break;
        default:
            uart_rx_state = 0;
            break;
    }
    _U2RXIF = 0;
}

void __attribute__((interrupt, no_auto_psv)) _U2TXInterrupt(void) {
    _U2TXIF = 0;
}

void process_uart_queue(void) {
    uart_message_t* m;

    if ((uart_ongoing == 0) && (uart_queue_idx != uart_queue_valid) && (n_uart_messages > 0)) {
        // copy to actual message to transmit

        uart_ongoing = 1;

        // start transmission of message
        m = uart_queue[uart_queue_valid];

        uart_parse_to_buffer(uart_dma_message_data, m,
                ARRAY_LENGTH(uart_dma_message_data));

        while (U2STAbits.TRMT == 0);

        m->status = UART_MSG_TRANSFERRED;
        DMA14STAL = (uint16_t) uart_dma_message_data;
        DMA14STAH = 0x0;
        DMA14CNT = UART_HEADER_SIZE + (m->length) - 1;

        // start transfer 
        DMA14CONbits.CHEN = 1;
        DMA14REQbits.FORCE = 1;
    }
}

void uart_queue_message(uart_message_t* m) {
    // wait for space in the queue
    while (n_uart_messages == UART_MESSAGE_BUFFER_LENGTH);

    if (m->status != UART_MSG_INIT_DONE) {
        return;
    }


    // queue message
    m->status = UART_MSG_QUEUED;
    uart_queue[uart_queue_idx] = m;
    uart_queue_idx = (uart_queue_idx + 1) % UART_MESSAGE_BUFFER_LENGTH;
    n_uart_messages++;

    process_uart_queue();
}

void __attribute__((__interrupt__, no_auto_psv)) _DMA14Interrupt(void) {
    // finish the current transfer
    uart_queue[uart_queue_valid]->status = UART_MSG_SENT;
    uart_queue_valid = (uart_queue_valid + 1) % UART_MESSAGE_BUFFER_LENGTH;
    n_uart_messages--;

    // check if another message is available
    uart_ongoing = 0;
    process_uart_queue();

    _DMA14IF = 0; // Clear the DMA0 Interrupt Flag
}

void uart_parse_to_buffer(uint8_t* data, uart_message_t* m, const size_t max_length) {
    uint16_t i;

    m->length = MIN(m->length, max_length - 8);

    data[0] = UART_CMD_START;
    data[1] = (uint8_t) (m->command >> 8);
    data[2] = (uint8_t) m->command;
    data[3] = m->id;
    data[4] = (uint8_t) (m->extended_id >> 8);
    data[5] = (uint8_t) m->extended_id;
    data[6] = (uint8_t) m->length;
    for (i = 0; i < m->length; i++) {
        data[7 + i] = m->data[i];
    }
    data[7 + m->length] = UART_CMD_STOP;
}

void uart_await_tx(uart_message_t* m) {
    while (m->status != UART_MSG_SENT);
}

void serial_can_update_address_cb(void) {
    // TODO
}

void uart_rx_callback() {
    can_message_t can_m;
    uint8_t can_data[CAN_MAX_N_BYTES], sensor_index;
    uart_message_t m = uart_rx_queue[uart_rx_read_idx];
    bool status = true;
    bool can_bus_active = true;
    // TODO: remove above line and fix missing variable

    uart_rx_read_idx = (uart_rx_read_idx + 1) % UART_RX_BUFFER_SIZE;

    if (m.status != UART_MSG_RECEIVED) {
        n_uart_rx_messages--;
        return;
        // TODO: handle errors over here
    }

    // prepare CAN message if needed
    can_init_message(&can_m, controller_address,
            CAN_NO_REMOTE_FRAME, CAN_EXTENDED_FRAME, m.extended_id, can_data, 0);

    switch (m.command) {
            // these messages cannot be sent from UART
        case SERIAL_CAN_REQUEST_ADDRESS_AVAILABLE:
        case SERIAL_CAN_ADDRESS_TAKEN:
            break;
        case SERIAL_CAN_UPDATE_ADDRESS:
            serial_can_update_address_cb();
            break;
        case SERIAL_CAN_DISCOVERY:
            can_init_message(&can_m, m.id,
                    CAN_NO_REMOTE_FRAME, CAN_EXTENDED_FRAME, CAN_HEADER(CAN_DISCOVERY, 0), can_data, 0);
            can_send_message_any_ch(&can_m);

            // send message back over UART with address of this node
            uart_init_message(&uart_discovery_m, SERIAL_CAN_DISCOVERY, controller_address, 0, NULL, 0);
            uart_queue_message(&uart_discovery_m);
            break;
        case SERIAL_START_MEAS_CMD:
            can_m.data_length = 1;
            can_data[0] = CAN_INFO_MSG_START_MEAS;
            can_send_message_any_ch(&can_m);
            delay_ms(1);
#ifdef __LOG__
            UART_DEBUG_PRINT("Sampling...");
#endif
            //start_sampling();
            break;
        case SERIAL_STOP_MEAS_CMD:
            // measurements stop immediately, reset device 
            asm("RESET");
            break;
        case SERIAL_SENSOR_ACTIVATE_CMD:
            can_m.data_length = 2;
            can_data[0] = CAN_MSG_SENSOR_STATUS;
            can_data[1] = 1;
            can_send_message_any_ch(&can_m);
            break;
        case SERIAL_SENSOR_DEACTIVATE_CMD:
            can_m.data_length = 2;
            can_data[0] = CAN_MSG_SENSOR_STATUS;
            can_data[1] = 0;
            can_send_message_any_ch(&can_m);
            break;
        case SERIAL_RESET_NODE_CMD:
            if (m.length == 1) {
                if (m.data[0] == controller_address) {
                    i2c_empty_queue();
                    asm("RESET");
                } else {
                    can_m.data_length = 2;
                    can_data[0] = CAN_INFO_MSG_RESET;
                    can_data[1] = m.data[0];
                    can_send_message_any_ch(&can_m);
                }
            }
            break;
        case SERIAL_RESET_SYSTEM_CMD:
            UART_DEBUG_PRINT("Resetting system...");

            // send reset message to other nodes

            if (can_bus_active) {
                can_m.data_length = 1;
                can_data[0] = CAN_INFO_MSG_RESET;
                while (can_send_message_any_ch(&can_m) != CAN_NO_ERROR);

                while (C1TR01CONbits.TXREQ0 == 1);
                while (C1TR01CONbits.TXREQ1 == 1);
                while (C1TR23CONbits.TXREQ2 == 1);
                while (C1TR23CONbits.TXREQ3 == 1);
                while (C1TR45CONbits.TXREQ4 == 1);
                while (C1TR45CONbits.TXREQ5 == 1);
                while (C1TR67CONbits.TXREQ6 == 1);
                while (C1TR67CONbits.TXREQ7 == 1);

                delay_us(100);
            }

            asm("RESET");

            break;
        case SERIAL_TEXT_MESSAGE_CMD:
            // this message can only be sent to the computer
            break;
        case SERIAL_SENSOR_DATA_CMD:
            // this message can only be sent to the computer
            break;
        case SERIAL_SENSOR_STATUS_CMD:
            // this message can only be sent to the computer
            break;
        case SERIAL_MEAS_PERIOD_CMD:
            if (m.length == 5) {
                uint8_t prescaler, i;
                uint32_t timer_period = 0;

                for (i = 0; i < 4; i++) {
                    timer_period = (timer_period << 8) | m.data[i];
                }
                prescaler = m.data[4] & 0b11;

                // TODO: add support for measurement period alterations
                //set_sample_period(timer_period, prescaler);
            }
            break;
        case SERIAL_ERR_MESSAGE_CMD:
            // this message can only be sent to the computer
            break;
        case SERIAL_LOOP_MESSAGE_CMD:
        {
            size_t i;

            uart_await_tx(&uart_loopback);

            for (i = 0; i < m.length; i++) {
                uart_loopback_data[i] = m.data[i];
            }
            uart_init_message(&uart_loopback,
                    SERIAL_LOOP_MESSAGE_CMD,
                    controller_address,
                    0,
                    uart_loopback_data,
                    m.length);

            uart_queue_message(&uart_loopback);

        }
            break;
        case SERIAL_ACTUATOR_STATUS:
            break;
        case SERIAL_HELLO_CMD:
            UART_DEBUG_PRINT("Broadcasting hello message...");

            // prepare data contents
            can_m.data_length = 1;
            can_data[0] = SERIAL_HELLO_CMD >> 8;
            can_data[0] = SERIAL_HELLO_CMD & 0xff;

            // broadcast over bus
            can_send_message_any_ch(&can_m);
            break;
        case SERIAL_INIT_SAMPLING_CMD:
            UART_DEBUG_PRINT("Initialising sampling event.");

            //init_sampling();
            break;
        case SERIAL_INIT_SENSORS_CMD:
            can_m.data_length = 1;
            can_data[0] = CAN_INFO_MSG_INIT_SENSORS;

            can_send_message_any_ch(&can_m);
            break;
        case SERIAL_LIA_GAIN_SET_CMD:
            can_m.data_length = 4;
            can_data[0] = CAN_INFO_LIA_GAIN_SET;
            can_data[1] = m.data[0];
            can_data[2] = m.data[1];
            can_data[3] = m.data[2];

            can_send_message_any_ch(&can_m);
            break;
        case SERIAL_START_INIT:
            break;
        case SERIAL_SENSOR_ACTUATOR_ENABLE:
            if (m.length < 2) {
                UART_DEBUG_PRINT("UART: sensor/actuator incomplete message.");
                break;
            }
            sensor_index = m.data[1];
            /* switch(m.data[0]){
                 case CAN_CONFIG_CMD_APDS9301:
                     if(sensor_index>=N_SENSOR_APDS9301){
                         UART_DEBUG_PRINT("UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                     } else {
                         apds9301_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                     }
                     break;
                 case CAN_CONFIG_CMD_OPT3001Q1:
                     if(sensor_index>=N_SENSOR_OPT3001Q1){
                         UART_DEBUG_PRINT("UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                     } else {
                         opt3001q1_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                     }
                     break;
                 case CAN_CONFIG_CMD_BH1721FVC:
                     if(sensor_index>=N_SENSOR_BH1721FVC){
                         UART_DEBUG_PRINT("UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                     } else {
                         bh1721fvc_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                     }
                     break;
                 case CAN_CONFIG_CMD_APDS9306:
                     if(sensor_index>=N_SENSOR_APDS9306){
                         UART_DEBUG_PRINT("UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                     } else {
                         apds9306_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                     }
                     break;
                 case CAN_CONFIG_CMD_SHT35:
                     if(sensor_index>=N_SENSOR_SHT35){
                         UART_DEBUG_PRINT("UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                     } else {
                         sht35_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                     }
                     break;
                 case CAN_CONFIG_CMD_SYLVATICA:
                     if(sensor_index>=N_SENSOR_SYLVATICA){
                         UART_DEBUG_PRINT("UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                     } else {
                         sylvatica_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                     }
                     break;
                 case CAN_CONFIG_CMD_SYLVATICA2:
                     if(sensor_index>=N_SENSOR_SYLVATICA2){
                         UART_DEBUG_PRINT("UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                     } else {
                         sylvatica2_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                     }
                     break;
                 case CAN_CONFIG_CMD_PLANALTA:
                     if(sensor_index>=N_SENSOR_PLANALTA){
                         UART_DEBUG_PRINT("UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                     } else {
                         planalta_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                     }
                     break;
                 case CAN_CONFIG_CMD_LICOR:
                     if(sensor_index>=N_SENSOR_LICOR){
                         UART_DEBUG_PRINT("UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                     } else {
                         licor_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                     }
                     break;
                 case CAN_CONFIG_CMD_GROWTH_CHAMBER:
                     growth_chamber_config.general.status = SENSOR_STATUS_IDLE;
                     break;
                 case CAN_CONFIG_CMD_RELAY_BOARD:
                     if(sensor_index>=N_ACTUATOR_RELAY_BOARD){
                         UART_DEBUG_PRINT("UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                     } else {
                         actuator_relay_board_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                     }
                     break;
                 case CAN_CONFIG_CMD_PLANALTA_FS:
                     if(sensor_index>=N_SENSOR_PLANALTA){
                         UART_DEBUG_PRINT("UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                     } else {
                         planalta_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                         planalta_config[sensor_index].op_mode = PLANALTA_FS;
                     }
                     break;
                 case CAN_CONFIG_CMD_NAU7802:
                     if(sensor_index>=N_SENSOR_NAU7802){
                         UART_DEBUG_PRINT("UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                     } else {
                         nau7802_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                     }
                     break;
                 case CAN_CONFIG_CMD_DS18B20:
                     if(sensor_index>=N_SENSOR_DS18B20){
                         UART_DEBUG_PRINT("UART: sensor/actuator `%x` index %x does not exist.", m.data[0], m.data[1]);
                     } else {
                         ds18b20_config[sensor_index].general.status = SENSOR_STATUS_IDLE;
                     }
                     break;
                 default:
                     UART_DEBUG_PRINT("UART: sensor/actuator type `%x` not supported.", m.data[0]);
                     break;
             }
             break;*/
        default:
            //status = process_actuator_serial_command(&m);
            // TODO: remove above line

            // TODO process_sensor_serial_command(&m);
            if (!status) {
                UART_DEBUG_PRINT("UART: serial command `%x` not supported.", m.command);
            }
            break;
    }
    m.status = UART_MSG_NONE;

    n_uart_rx_messages--;
}
