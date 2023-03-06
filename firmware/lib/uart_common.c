#include <uart_common.h>

void uart_init_message(uart_message_t* m,
        serial_cmd_t command,
        uint8_t id,
        uint16_t extended_id,
        uint8_t* data,
        uint16_t length) {
    m->command = command;
    m->data = data;
    m->id = id;
    m->extended_id = extended_id;
    m->length = length;
    m->status = UART_MSG_INIT_DONE;
}

void uart_reset_message(uart_message_t* m) {
    if ((m->status != UART_MSG_QUEUED) && (m->status != UART_MSG_TRANSFERRED)) {
        m->status = UART_MSG_INIT_DONE;
    }
}
