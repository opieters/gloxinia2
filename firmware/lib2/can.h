#ifndef __CAN_H__
#define __CAN_H__

#include <xc.h>
#include <stdint.h>
#include <utilities.h>
#include <uart.h>
#include <message.h>

/// @brief Maximum number of bytes that can be sent in a single CAN message.
#define CAN_MAX_N_BYTES 8U

/// @brief Number of ECAN buffers available for RX and TX.
#define NUM_OF_ECAN_BUFFERS 32

/// @brief Formats the header in the 18-bit ECAN extended identifier field.
#define CAN_HEADER(cmd, id) (((((uint16_t)(cmd)) & 0xFFU) << 8) | (((uint16_t)(id)) & 0xFFU))

/// @brief Extracts the command from the 18-bit ECAN extended identifier field.
#define CAN_EXTRACT_HEADER_CMD(x) ((uint8_t)(((x) >> 8) & 0xFFU))

/// @brief Extracts the sensor ID from the 18-bit ECAN extended identifier field.
#define CAN_EXTRACT_HEADER_ID(x) ((uint8_t)((x)&0xFFU))

/// @brief Macro to indicate that a CAN message is not a remote frame.
#define CAN_NO_REMOTE_FRAME 0

/// @brief Macro to indicate that a CAN message is a remote frame.
#define CAN_REMOTE_FRAME 1

/// @brief Macro to indicate that a CAN message is an extended frame.
#define CAN_EXTENDED_FRAME 1

/// @brief Macro to indicate that a CAN message is not an extended frame.
#define CAN_NO_EXTENDED_FRAME 0

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Fields associated with an ECAN message.
     *
     * @param identifier: 11-bit primary ECAN identifier
     * @param remote_frame: 0 if no remote frame, 1 if remote frame. Use
     * CAN_NO_REMOTE_FRAME and CAN_REMOTE_FRAME.
     * @param extended_frame: 0 if standard (11-bit address), 1 if 29 bit
     * identifier (11+18 bits). Use CAN_EXTENDED_FRAME and CAN_NO_EXTENDED_FRAME.
     * @param extended_identifier: 18 bit extended identifier field. Use
     * CAN_HEADER macro to format.
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

    /// @brief Symbolic representation of CAN message status

    typedef enum
    {
        CAN_NO_ERROR,        ///< No error
        CAN_TX_PENDING,      ///< Message is pending transmission
        CAN_RX_PENDING,      ///< Message is pending reception
        CAN_NOT_ENABLED,     ///< Error: CAN module is not enabled
        CAN_ADDRESS_NOT_SET, ///< Error: node address is not set
    } can_status_t;

    /// @brief Symbolic representation of CAN module status
    typedef enum
    {
        CAN_MODULE_LISTEN_ALL = 7,
        CAN_CONFIG_MODE = 4,
        CAN_MODULE_LISTEN_ONLY = 3,
        CAN_MODULE_LOOPBACK = 2,
        CAN_MODULE_DISABLE = 1,
        CAN_MODULE_ENABLE = 0,
    } can_module_status_t;

    /**
     * @brief Initialises ECAN module.
     *
     * @details Initialises ECAN module with the default configuration and
     * enables the module. DMA channels are also configured.
     */
    void can_init(void);

    /**
     * @brief Disables ECAN module.
     *
     * @details Disables ECAN module and disables the DMA channels.
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
     * @brief Initialises a CAN message.
     *
     * @details Initialises a CAN message with the given parameters.
     *
     * @param m: pointer to CAN message to initialise.
     * @param identifier: 11-bit primary ECAN identifier, should be the node address
     * @param remote_frame: CAN_NO_REMOTE_FRAME or CAN_REMOTE_FRAME.
     * @param extended_frame: CAN_EXTENDED_FRAME or CAN_NO_EXTENDED_FRAME.
     * @param *data: pointer to data bytes to transmit.
     * @param data_length: number of bytes to transmit (max. 8)
     */
    void can_init_message(can_message_t *m,
                          uint16_t identifier,
                          uint8_t remote_frame,
                          uint8_t extended_frame,
                          uint32_t extended_identifier,
                          uint8_t *data,
                          uint8_t data_length);

    /**
     * @brief Parses an ECAN message from the internal memory structure to a formatted
     * struct of type can_message_t.
     *
     * @details Messages received and transmitted by the ECAN peripheral are stored
     * in a specific format. To convert from this format to the more usable format
     * of can_message_t, this function is used.
     */
    void parse_from_can_buffer(can_message_t *m, uint16_t *raw_data);

    /// @brief Converts from can_message_t to generic message_t.
    void parse_from_can_message(message_t *m, can_message_t *cm);

    /// @brief Converts from generic message_t to can_message_t.
    void can_message_from_fmessage(can_message_t *cm, const message_t *m);

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
    can_status_t can_send_message(can_message_t *message, uint8_t can_channel);

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
    can_status_t can_send_message_any_ch(can_message_t *m);

    /**
     * @brief Sends a message on the CAN bus.
     *
     * @details Automatically converts from message_t to can_message_t and sends
     * it on the first available channel.
     *
     * @param m: pointer to message to send.
     *
     * @return CAN_NO_ERROR if message was sent successfully, CAN_TX_PENDING if
     * no channels are available, CAN_NOT_ENABLED if the CAN module is not
     * enabled, CAN_ADDRESS_NOT_SET if the node address is not set.
     */
    can_status_t can_send_fmessage_any_ch(const message_t *m);

    /**
     * @brief Detects if other devices are present on the CAN bus.
     *
     * @details Transmits a message on the CAN bus and monitors if it is sent
     * successfully by means of checking for the ACK in the ACK slot. If no ACK is
     * received, the CAN module is disabled since no devices are present.
     *
     * The system keeps track of the number of active CAN devices internally.
     */
    void can_detect_devices(void);

#ifdef __cplusplus
}
#endif

#endif
