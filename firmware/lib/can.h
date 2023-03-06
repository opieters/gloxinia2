#ifndef __CAN_H__
#define __CAN_H__

#include <xc.h>
#include <stdint.h>
#include "utilities.h"
#include <uart.h>
#include "message.h"

/**
 * @brief Maximum number of bytes that can be sent in a single CAN message.
 */
#define CAN_MAX_N_BYTES 8U
#define NUM_OF_ECAN_BUFFERS 32
#define MAX_CAN_TO_UART_SIZE (2 + CAN_MAX_N_BYTES)

#define CAN_HEADER(cmd, id) (((((uint16_t)(cmd)) & 0xFFU) << 8) | (((uint16_t)(id)) & 0xFFU))
#define CAN_EXTRACT_HEADER_CMD(x) ((uint8_t)(((x) >> 8) & 0xFFU))
#define CAN_EXTRACT_HEADER_ID(x) ((uint8_t)((x)&0xFFU))
#define CAN_NO_REMOTE_FRAME 0
#define CAN_REMOTE_FRAME 1
#define CAN_EXTENDED_FRAME 1
#define CAN_NO_EXTENDED_FRAME 0

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Fields associated with an ECAN message.
     *
     * @param identifier: 11-bit primary ECAN identifier
     * @param remote_frame: 0 if no remote frame, 1 if remote frame
     * @param extended_frame: 0 if standard (11-bit address), 1 if 29-bit identifier
     * @param extended_identifier: 18 bit extended identifier field
     * @param data_length: number of bytes to transmit (max. 8)
     * @param data: data bytes of ECAN message
     */
    typedef struct
    {
        uint16_t identifier;
        uint8_t remote_frame;
        uint8_t extended_frame;
        uint32_t extended_identifier;
        uint8_t data_length;
        uint8_t *data;
    } can_message_t;

    /**
     * @brief Symbolic representation of CAN module status
     */
    typedef enum
    {
        CAN_NO_ERROR,
        CAN_TX_PENDING,
        CAN_RX_PENDING,
        CAN_NOT_ENABLED,
    } can_status_t;

    typedef enum
    {
        CAN_MODULE_LISTEN_ALL = 7,
        CAN_CONFIG_MODE = 4,
        CAN_MODULE_LISTEN_ONLY = 3,
        CAN_MODULE_LOOPBACK = 2,
        CAN_MODULE_DISABLE = 1,
        CAN_MODULE_ENABLE = 0,
    } can_module_status_t;

    extern volatile uint8_t __init_sensors;

    void can_cmd_info_rx(uint8_t cmd, uint8_t *data, uint8_t length);
    void can_cmd_info_tx(uint8_t cmd, uint8_t *data, uint8_t length);

    extern volatile uint8_t received_ecan_message;

    /**
     * @brief Initialises the DMA channel handling ECAN messages.
     *
     * @attention Should be executed only after initialising the ECAN module!
     */
    void can_init_dma_channel(void);

    /**
     * @brief Initialises ECAN module.
     */
    void can_init(void);

    /**
     * @brief Disables ECAN module and aborts transmissions.
     */
    void can_disable(void);

    /**
     * @brief Restarts ECAN module if an error occorred.
     *
     * @details This function can be used when it is unsure that the bus has
     * multiple devices to restart the bus. It does not reset the bus if it is
     * still operationg correctly.
     */
    void can_reset(void);

    /**
     * @brief Places a CAN message in the buffer of the corresponding channel.
     *
     * @details Does not actually handle CAN transfer. This is handled by the
     * DMA. For now, user configured priorities cannot be assigned to messages.
     *
     * @param message: the ECAN message to transmit
     * @param channel: the ECAN channel number (0-7) on which to transmit the
     * data.
     *
     * @return failure (message in buffer not yet sent) or success in the form
     * of a `can_status_t`.
     */
    __attribute__((always_inline)) can_status_t can_send_raw_message(can_message_t *message, uint8_t channel);
    __attribute__((always_inline)) can_status_t can_send_message(message_t *message, uint8_t channel);
    can_status_t can_send_message_any_ch(message_t *m);
    can_status_t can_send_raw_message_any_ch(can_message_t *m);

    void deactivate_can_bus(void);

    void can_parse_message(can_message_t *m, uint16_t *raw_data);

    void parse_can_to_uart_message(can_message_t *can_message, uart_message_t *uart_message);

    void can_init_message(can_message_t *m,
                          uint16_t identifier,
                          uint8_t remote_frame,
                          uint8_t extended_frame,
                          uint32_t extended_identifier,
                          uint8_t *data,
                          uint8_t data_length);

    void parse_from_can_buffer(message_t *m, uint16_t *raw_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CAN_H__ */
