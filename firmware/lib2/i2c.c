#include <i2c.h>
#include <utilities.h>

// i2c queue variables
volatile uint8_t transfer_done = 0;
i2c_message_t* m = NULL;
i2c_message_t* i2c_message_queue [I2C_MESSAGE_BUFFER_LENGTH];
volatile uint16_t i2c_queue_idx = 0;
volatile uint16_t i2c_queue_valid = 0;
volatile uint16_t n_i2c_queued_messages = 0;
volatile uint8_t i2c_transfer_status = 0;


void i2c1_init(i2c_config_t* config) {
    uint16_t frequency;

    //i2c_reset_callback_init();
    
    transfer_done = 1;

    I2C1CONbits.A10M = 0; // 7-bit slave address
    I2C1CONbits.SCLREL = 1; // release clock

    frequency = FCY / (2 * FREQUENCY_SCL) - DELAY_I2C * (FCY / 2) - 2;
    if (frequency > 0x1FF) {
        frequency = 0x1FF; // max allowed value for this register
    }
    I2C1BRG = frequency;

    I2C1ADD = 0x0; // no slave address, this is I2C master
    I2C1MSK = 0x0; // disable address masking for this bit position

    I2C1CONbits.I2CEN = 1; // enable I2C module and configure pins as serial port pins
    IEC1bits.MI2C1IE = 1; // enable I2C interrupt
    IFS1bits.MI2C1IF = 0; // clear I2C interrupt flag
    
    T2CONbits.TON = 0;
    T2CONbits.TCS = 0; // use internal instruction cycle as clock source
    T2CONbits.TGATE = 0; // disable gated timer
    T2CONbits.TCKPS = 0b11; // prescaler 1:256
    TMR2 = 0; // clear timer register
    PR2 = I2C_TIMER_PERIOD - 1; // set period of ADC_SAMPLE_FREQUENCY
    _T2IF = 0; // clear interrupt flag
    _T2IE = 1; // enable interrupt

    /*T3CONbits.TON = 0;
    T3CONbits.TCS = 0; // use internal instruction cycle as clock source
    T3CONbits.TGATE = 0; // disable gated timer
    T3CONbits.TCKPS = 0b11; // prescaler 1:256
    TMR3 = 0; // clear timer register
    PR3 = 0xffff;
    _T3IF = 0; // clear interrupt flag
    _T3IE = 1; // enable interrupt
    
    T3CONbits.TON = 1;*/
}

void i2c1_disable(void){
    I2C1CONbits.I2CEN = 0;
}

void i2c2_disable(void){
    I2C2CONbits.I2CEN = 0;
}

void i2c2_init(i2c_config_t* config) {
    uint16_t frequency;

    //i2c_reset_callback_init();

    transfer_done = 1;

    I2C2CONbits.A10M = 0; // 7-bit slave address
    I2C2CONbits.SCLREL = 1; // release clock

    frequency = FCY / (2 * FREQUENCY_SCL) - DELAY_I2C * (FCY / 2) - 2;
    if (frequency > 0x1FF) {
        frequency = 0x1FF; // max allowed value for this register
    }
    I2C2BRG = frequency;

    I2C2ADD = 0x0; // no slave address, this is I2C master
    I2C2MSK = 0x0; // disable address masking for this bit position

    I2C2CONbits.I2CEN = 1; // enable I2C module and configure pins as serial port pins
    _MI2C2IE = 1; // enable I2C interrupt
    _MI2C2IF = 0; // clear I2C interrupt flag

    T2CONbits.TON = 0;
    T2CONbits.TCS = 0; // use internal instruction cycle as clock source
    T2CONbits.TGATE = 0; // disable gated timer
    T2CONbits.TCKPS = 0b11; // prescaler 1:256
    TMR2 = 0; // clear timer register
    PR2 = I2C_TIMER_PERIOD - 1; // set timeout value
    _T2IF = 0; // clear interrupt flag
    _T2IE = 1; // enable interrupt

    /*T3CONbits.TON = 0;
    T3CONbits.TCS = 0; // use internal instruction cycle as clock source
    T3CONbits.TGATE = 0; // disable gated timer
    T3CONbits.TCKPS = 0b11; // prescaler 1:256
    TMR3 = 0; // clear timer register
    PR3 = 0xffff;
    _T3IF = 0; // clear interrupt flag
    _T3IE = 1; // enable interrupt
    
    T3CONbits.TON = 1;*/
}


void i2c_init_message(i2c_message_t* m,
        uint8_t address,
        i2c_bus_t i2c_bus,
        uint8_t* write_data,
        size_t write_length,
        uint8_t* read_data,
        uint8_t read_length,
        void (*controller)(i2c_message_t* m),
        int8_t n_attempts,
        void (*callback)(i2c_message_t* m),
        void* callback_data,
        uint8_t callback_data_length) {
    m->address = address;
    m->write_data = write_data;
    m->write_length = write_length;
    m->read_data = read_data;
    m->read_length = read_length;
    m->controller = controller;
    m->status = I2C_MESSAGE_HANDLED;
    m->error = I2C_NO_ERROR;
    m->callback = callback;
    m->callback_data = callback_data;
    m->callback_data_length = callback_data_length;
    m->i2c_bus = i2c_bus;
    i2c_reset_message(m, n_attempts);
}

void i2c_reset_message(i2c_message_t* m, uint8_t n_attempts) {
    m->n_attempts = n_attempts;
    m->status = I2C_MESSAGE_HANDLED;
    m->error = I2C_NO_ERROR;
}


void i2c_queue_message(i2c_message_t* message) {
    if (n_i2c_queued_messages == I2C_MESSAGE_BUFFER_LENGTH) {
        message->status = I2C_MESSAGE_CANCELED;
        message->error = I2C_QUEUE_FULL;
        UART_DEBUG_PRINT("I2C queue full.");
    } else {
        if (message->n_attempts < 1) {
            message->status = I2C_MESSAGE_HANDLED;
            message->error = I2C_ZERO_ATTEMPTS;
        } else {
            if((message->i2c_bus == I2C1_BUS) && (I2C1CONbits.I2CEN == 0)){
                message->status = I2C_MESSAGE_HANDLED;
                message->error = I2C_MESSAGE_BUS_INACTIVE;
                return;
            }
            if((message->i2c_bus == I2C2_BUS) && (I2C2CONbits.I2CEN == 0)){
                message->status = I2C_MESSAGE_HANDLED;
                message->error = I2C_MESSAGE_BUS_INACTIVE;
                return;
            }
            message->status = I2C_MESSAGE_QUEUED;
            i2c_message_queue[i2c_queue_valid] = message;
            i2c_queue_valid = (i2c_queue_valid + 1) % I2C_MESSAGE_BUFFER_LENGTH;
            message->error = I2C_NO_ERROR;
            n_i2c_queued_messages++;
            //UART_DEBUG_PRINT("I2C added message to %x on bus %x.", message->address, message->i2c_bus);
        }
    }
}

void i2c_process_queue(void) {
    if (transfer_done == 1) {
        if (m != NULL) {
            switch (m->status) {
                case I2C_MESSAGE_PROCESSING:
                    if(m->callback != NULL){
                        m->callback(m); // execute callback
                    }
                    m->status = I2C_MESSAGE_HANDLED;

                    //UART_DEBUG_PRINT("I2C callback.");
                case I2C_MESSAGE_HANDLED:
                    // check if transfer went OK or max attempts reached
                    if ((m->error == I2C_NO_ERROR) || (m->n_attempts <= 0)) {

                        //UART_DEBUG_PRINT("I2C message handled.");
                        T2CONbits.TON = 0; // disable timer (everything OK!)

                        i2c_queue_idx = (i2c_queue_idx + 1) % I2C_MESSAGE_BUFFER_LENGTH;
                        m = NULL;
                        n_i2c_queued_messages--;

                       // if (m->callback == i2c_free_callback) {
                            // TODO when upgraded to new message structure
                            //free(m->read_data);
                            //free(m->write_data);
                        //    free(m);
                        //}
                    } else {


                        //UART_DEBUG_PRINT("I2C ERROR, restarting.");
                        m->n_attempts--;
                        m->status = I2C_MESSAGE_TRANSFERRING;
                        m->error = I2C_NO_ERROR;

                        // reset the transfer status (just in case)
                        i2c_transfer_status = 0;
                    }

                    break;
                case I2C_MESSAGE_QUEUED:
                    m->status = I2C_MESSAGE_TRANSFERRING;
                    TMR2 = 0; // clear timer register
                    // start timer: transfer must finish within approx. 25ms
                    T2CONbits.TON = 1;

                    //UART_DEBUG_PRINT("I2C message queued.");

                case I2C_MESSAGE_TRANSFERRING:

                    //UART_DEBUG_PRINT("I2C message transferring.");

                    m->controller(m);
                    break;
                case I2C_MESSAGE_CANCELED:
                    if (m->i2c_bus == I2C1_BUS) {
                        I2C1CONbits.PEN = 1;
                    } else {
                        I2C2CONbits.PEN = 1;
                    }

                    if (m->cancelled_callback != NULL) {
                        m->cancelled_callback(m);
                    }

                    // move to next message
                    i2c_queue_idx = (i2c_queue_idx + 1) % I2C_MESSAGE_BUFFER_LENGTH;
                    m = NULL;
                    n_i2c_queued_messages--;

                    break;
                default:
                    T2CONbits.TON = 0; // disable timer (everything OK!)

                    i2c_queue_idx = (i2c_queue_idx + 1) % I2C_MESSAGE_BUFFER_LENGTH;
                    m = NULL;
                    n_i2c_queued_messages--;
                    m->status = I2C_MESSAGE_CANCELED;
            }
        } else {
            if (i2c_queue_idx != i2c_queue_valid) {
                m = i2c_message_queue[i2c_queue_idx];

                //UART_DEBUG_PRINT("Fetched message from queue: %x on bus %x.", m->address, m->i2c_bus);

            }
        }
    }
}

i2c_controller_t i2c_get_write_controller(i2c_bus_t bus) {
    switch (bus) {
        case I2C1_BUS:
            return i2c1_write_controller;
            break;
        case I2C2_BUS:
            return i2c2_write_controller;
            break;
        default:
            UART_DEBUG_PRINT("I2C module not supported.");
    }

    return NULL;
}

i2c_controller_t i2c_get_read_controller(i2c_bus_t bus) {
    // configure read message
    switch (bus) {
        case I2C1_BUS:
            return i2c1_read_controller;
            break;
        case I2C2_BUS:
            return i2c2_read_controller;
            break;
        default:
            UART_DEBUG_PRINT("I2C module not supported.");
    }

    return NULL;
}

i2c_controller_t i2c_get_write_read_controller(i2c_bus_t bus){
    switch (bus) {
        case I2C1_BUS:
            return i2c1_write_read_controller;
            break;
        case I2C2_BUS:
            return i2c2_write_read_controller;
            break;
        default:
            UART_DEBUG_PRINT("I2C module not supported.");
            break;
    }
    
    return NULL;
}

bool i2c_check_message_sent(i2c_message_t* m){
    return (m->status == I2C_MESSAGE_HANDLED) || (m->status == I2C_MESSAGE_CANCELED);
}

void __attribute__((interrupt, no_auto_psv)) _MI2C1Interrupt(void) {
    transfer_done = 1;
    
    _MI2C1IF = 0;
    
    i2c_process_queue();
}

void __attribute__((interrupt, no_auto_psv)) _MI2C2Interrupt(void) {
    transfer_done = 1;
    
    _MI2C2IF = 0;
    
    i2c_process_queue();
}

void __attribute__((no_auto_psv)) __T2Interrupt(void) {
    UART_DEBUG_PRINT("I2C error, cancelling message (%02x) on bus %x.", m->address, m->i2c_bus);

    if ((I2C1CONbits.I2CEN == 1) && (m->i2c_bus == I2C1_BUS)) {
        I2C1CONbits.PEN = 1;
        UART_DEBUG_PRINT("Sending stop on bus 0.");
    }

    if ((I2C2CONbits.I2CEN == 1) && (m->i2c_bus == I2C2_BUS)) {
        I2C2CONbits.PEN = 1;
        UART_DEBUG_PRINT("Sending stop on bus 1.");
    }
    transfer_done = 1;

    m->status = I2C_MESSAGE_CANCELED;
    m->n_attempts = 0;

    // disable timer
    T2CONbits.TON = 0;

    _T2IF = 0;
}