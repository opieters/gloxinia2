#ifndef __CAN2_H__
#define __CAN2_H__

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <utilities.h>
#include <message.h>
#include <can.h>

/// @brief Memory unlock sequence
#define CAN2_UNLOCK_SEQUENCE (0x5AU)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Fields associated with a bootloader ECAN message.
     *
     * @param command: CAN command
     * @param unlock: use unlock sequence or not
     * @param length: number of bytes to transmit (max. 8)
     * @param data: data bytes of ECAN message
     */
    typedef struct
    {
        uint8_t command;
        bool unlock;
        uint8_t length;
        uint8_t data[CAN_MAX_N_BYTES];
    } can2_message_t;

    /**
     * @brief Initialises ECAN module.
     *
     * @details Initialises ECAN module with the default configuration and
     * enables the module. DMA channels are also configured.
     */
    void can2_init(void);

    /**
     * @brief Disables ECAN module.
     *
     * @details Disables ECAN module and disables the DMA channels.
     */
    void can2_disable(void);

    /**
     * @brief Restarts ECAN module if an error occorred.
     *
     * @details This function can be used when it is unsure that the bus has
     * multiple devices to restart the bus. It does not reset the bus if it is
     * still operationg correctly.
     */
    void can2_reset(void);

    /**
     * @brief Parses an ECAN message from the internal memory structure to a formatted
     * struct of type can_message_t.
     *
     * @details Messages received and transmitted by the ECAN peripheral are stored
     * in a specific format. To convert from this format to the more usable format
     * of can_message_t, this function is used.
     */
    bool parse_from_can2_buffer(can2_message_t *m, uint16_t *raw_data);

    /**
     * @brief Sends a CAN message.
     *
     * @details Sends a CAN message on the specified channel. The channel
     * must be enabled and the CAN module must be enabled.
     *
     * @param message: pointer to CAN message to send.
     * @param can_channel: CAN channel to send message on.
     *
     * @return CAN_NO_ERROR if message was sent successfully, CAN_TX_PENDING if
     * message is pending transmission, CAN_NOT_ENABLED if the CAN module is not
     * enabled, CAN_ADDRESS_NOT_SET if the node address is not set.
     */
    can_status_t can2_send_message(can2_message_t *message, uint8_t can_channel);

    /**
     * @brief Sends a CAN message.
     *
     * @details Sends a CAN message on the first available channel.
     *
     * @param message: pointer to CAN message to send.
     *
     * @return CAN_NO_ERROR if message was sent successfully, CAN_TX_PENDING if
     * no channels are available, CAN_NOT_ENABLED if the CAN module is not
     * enabled, CAN_ADDRESS_NOT_SET if the node address is not set.
     */
    can_status_t can2_send_message_any_ch(can2_message_t *m);

    /**
     * @brief Detects if other devices are present on the CAN bus.
     *
     * @details Transmits a message on the CAN bus and monitors if it is sent
     * successfully by means of checking for the ACK in the ACK slot. If no ACK is
     * received, the CAN module is disabled since no devices are present.
     *
     * The system keeps track of the number of active CAN devices internally.
     */
    void can2_detect_devices(void);
    
    /// TODO
    void copy_can2_message(can2_message_t* m1, can2_message_t* m2);

#ifdef __cplusplus
}
#endif

#endif
