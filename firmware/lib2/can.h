#ifndef __CAN_H__
#define	__CAN_H__

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
#define MAX_CAN_TO_UART_SIZE (2+CAN_MAX_N_BYTES)

#define CAN_HEADER(cmd, id) (((((uint16_t) (cmd)) & 0xFFU) << 8) | (((uint16_t) (id)) &  0xFFU)) 
#define CAN_EXTRACT_HEADER_CMD(x) ((uint8_t) (((x) >> 8) & 0xFFU))
#define CAN_EXTRACT_HEADER_ID(x) ((uint8_t) ((x) & 0xFFU))
#define CAN_NO_REMOTE_FRAME 0
#define CAN_REMOTE_FRAME 1
#define CAN_EXTENDED_FRAME 1
#define CAN_NO_EXTENDED_FRAME 0

#ifdef	__cplusplus
extern "C" {
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
    typedef struct {
        uint16_t identifier;
        uint8_t remote_frame;
        uint8_t extended_frame;
        uint32_t extended_identifier;
        uint8_t data_length;
        uint8_t* data;
    } can_message_t;

    /**
     * @brief Symbolic representation of CAN module status
     */
    typedef enum {
        CAN_NO_ERROR,
        CAN_TX_PENDING,
        CAN_RX_PENDING,
        CAN_NOT_ENABLED,
        CAN_ADDRESS_NOT_SET,
    } can_status_t;

    typedef enum {
        CAN_MODULE_LISTEN_ALL = 7,
        CAN_CONFIG_MODE = 4,
        CAN_MODULE_LISTEN_ONLY = 3,
        CAN_MODULE_LOOPBACK = 2,
        CAN_MODULE_DISABLE = 1,
        CAN_MODULE_ENABLE = 0,
    } can_module_status_t;

    /**
     * @brief Initialises ECAN module.
     */
    void can_init(void);

    void can_disable(void);
    
    void can_reset(void);

    void can_init_message(can_message_t* m,
            uint16_t identifier,
            uint8_t remote_frame,
            uint8_t extended_frame,
            uint32_t extended_identifier,
            uint8_t* data,
            uint8_t data_length);
    
    void parse_from_can_buffer(can_message_t* m, uint16_t* raw_data);
    void parse_from_can_message(message_t* m, can_message_t* cm);
    
    can_status_t can_send_message(can_message_t* message, uint8_t can_channel);
    can_status_t can_send_message_any_ch(can_message_t* m);
    can_status_t can_send_fmessage_any_ch(message_t* m);
    
    void can_detect_devices(void);
    
    void can_message_from_fmessage(can_message_t* cm, message_t* m);


#ifdef	__cplusplus
}
#endif

#endif
