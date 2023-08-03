#ifndef __UART_H__
#define __UART_H__

#include <xc.h>
#include <stdint.h>
#include <stddef.h>
#include <message.h>
#include <can.h>

/// @brief UART start byte
#define UART_CMD_START ('\x5B')

/// @brief UART stop byte
#define UART_CMD_STOP ('\x5D')

/// @brief UART FIFO TX queue size
#define UART_FIFO_TX_BUFFER_SIZE 16U
#define UART_FIFO_TX_PRINT_BUFFER_SIZE 4U
#define UART_FIFO_TX_PRINT_BUFFER_DATA_SIZE 32U
/// @brief UART FIFO TX message data size (preallocated)
#define UART_FIFO_TX_DATA_BUFFER_SIZE 16U

/// @brief UART FIFO RX queue size
#ifdef __DICIO__
#define UART_FIFO_RX_BUFFER_SIZE 512U
#else
#define UART_FIFO_RX_BUFFER_SIZE 64U
#endif
#define UART_FIFO_RX_DATA_BUFFER_SIZE CAN_MAX_N_BYTES

/// @brief UART header size (includes start and stop bytes)
#define UART_HEADER_SIZE 8U

/// @brief UART receive state enumeration
typedef enum
{
    UART_RX_STATE_FIND_START_BYTE, ///< Find start byte
    UART_RX_STATE_READ_IDH,        ///< Read ID high byte
    UART_RX_STATE_READ_IDL,        ///< Read ID low byte
    UART_RX_STATE_READ_CMD,        ///< Read command byte
    UART_RX_STATE_READ_REQUEST,    ///< Read request byte
    UART_RX_STATE_READ_SIDH,       ///< Read sensor ID high byte
    UART_RX_STATE_READ_SIDL,       ///< Read sensor ID low byte
    UART_RX_STATE_READ_LENGTH,     ///< Read data length byte
    UART_RX_STATE_READ_DATA,       ///< Read data (len: variable length)
    UART_RX_STATE_DETECT_STOP      ///< Detect stop byte
} uart_rx_state_t;

/// @brief UART transmit state enumeration
typedef enum
{
    UART_TX_STATE_SEND_START_BYTE, ///< Send start byte
    UART_TX_STATE_SEND_IDH,        ///< Send ID high byte
    UART_TX_STATE_SEND_IDL,        ///< Send ID low byte
    UART_TX_STATE_SEND_CMD,        ///< Send command byte
    UART_TX_STATE_SEND_REQUEST,    ///< Send request byte
    UART_TX_STATE_SEND_SIDH,       ///< Send sensor ID high byte
    UART_TX_STATE_SEND_SIDL,       ///< Send sensor ID low byte
    UART_TX_STATE_SEND_LENGTH,     ///< Send data length byte
    UART_TX_STATE_SEND_DATA,       ///< Send data (len: variable length)
    UART_TX_STATE_SEND_STOP,       ///< Send stop byte
    UART_TX_STATE_DONE,
} uart_tx_state_t;

typedef struct {
    uint8_t* data;
    uint8_t length;
} uart_message_t;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialise UART
     *
     * @param baudrate Baudrate
     */
    void uart_init(uint32_t baudrate);

    /**
     * @brief Reset message
     *
     * @param m Message
     */
    void message_reset(message_t *m);

    /**
     * @brief Add message to TX queue
     *
     * @details The message in the queue is a deep copy of the message passed to
     * the function. So the variable passed to the function can be freed after
     * the function returns. Data lengths of up to UART_FIFO_TX_DATA_BUFFER_SIZE
     * bytes are supported.
     *
     * @param m Message to add to queue
     */
    void uart_queue_message(message_t *m);

    /**
     * @brief Parse message to UART TX buffer for DMA transfer
     *
     * @details DMA transferis more efficient and can be used to make UART
     * transmissions more efficient.
     *
     * @param data Pointer to data buffer
     * @param m Message to parse
     * @param max_length Maximum length of data buffer
     */
    void uart_parse_to_raw_buffer(uint8_t *data, message_t *m, const size_t max_length);

    /**
     * @brief Send string over UART.
     *
     * @details The string length is also limited to
     * UART_FIFO_TX_DATA_BUFFER_SIZE bytes. If the string is longer than this,
     * it will be truncated.
     *
     * @param message String to send
     * @param lengh Length of string
     */
    void uart_print(const char *message, size_t lengh);

    /**
     * @brief Process messages in the UART TX queue
     *
     * @details This message will start the transmission of the next message in
     * the queue if the current one is done. It is automatically called by the
     * DMA interrupt handler and when a new message is queued, so it is not
     * necessary to call it manually in the main event loop.
     */
    void process_uart_tx_queue(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UART_H__ */
