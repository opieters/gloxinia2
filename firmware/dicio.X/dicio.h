#ifndef __DICIO_H__
#define __DICIO_H__

#include <xc.h>
#include <utilities.h>
#include <uart.h>
#include <message.h>

#define DICIO_CONFIG_ADDRESS 0x00000000U
#define DICIO_NODE_CONFIG_START_ADDRESS 0x00000020U
#define DICIO_DATA_START_ADDRESS 0x00000000U
#define DICIO_MAX_N_NODES 50U

/**
 * @brief Node configuration structure.
 *
 * @param sector_address The address of the sector where the node configuration
 * is stored on the SD card.
 * @param node_id The node ID of the node (1-0xFFFF).
 * @param node_type The type of node (see message.h).
 * @param n_interfaces The number of interfaces the node has.
 * @param stored_config Indicates if the node has a stored configuration at
 * sector_address.
 * @param v_hw The hardware version of the node.
 * @param v_sw_u The upper byte of the software version of the node.
 * @param v_sw_l The lower byte of the software version of the node.
 */
typedef struct
{
  uint32_t sector_address;
  uint16_t node_id;

  message_node_t node_type;
  uint8_t n_interfaces;
  bool stored_config;

  uint8_t v_hw;
  uint8_t v_sw_u;
  uint8_t v_sw_l;
} node_config_t;

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Initialises dicio hardware and peripherals.
   *
   * @details This routine is used to initialise all peripherals and hardware used by
   * dicio. This insures proper operation and also loads the necessary data from
   * memory to fetch esisting configurations on power-up.
   *
   * This function should only be called once during system initialization.
   *
   */
  void dicio_init(void);

  /**
   * @brief Ready message indicator.
   *
   * @details This function called when the system is ready to be configured. It
   * indicates to the main controller or GUI application that configuration
   * commands will be processed correctly. It should be used in conjunction with
   * the event scheduler and embedded into a task.
   */
  void dicio_send_ready_message(void *);

  /**
   * @brief Initialises pin configurations.
   *
   * @details This function is used to initialise pin configurations and peripheral connections thereto.
   *
   * It is called by dicio_init() and should not be called directly.
   */
  void dicio_init_pins(void);

  /**
   * @brief Loads initialisation from SD card if there is any.
   *
   *
   * @details Some configuration data is stored on the SD card. In case of an unextected
   * power interrupts this is useful to resume operation. However, no validation
   * is performed and if errors occur the configuration is not altered to reflect
   * these in case of future power interruptions or restarts.
   *
   * It is called by dicio_init() and should not be called directly.
   */
  void dicio_read_sdconfig_data(void);

  /**
   * @brief Clears all measurement data from the SD card.
   *
   * @details This clears data measured by the nodes from the SD card. This is useful
   * when the SD card is full and the data needs to be cleared to make room for
   * new data or when a new experiment is started and data is no longer needed.
   *
   * The SD card should not be used for long term data storage.
   *
   * Removed data cannot be recovered.
   */
  bool dicio_clear_data(void);

  /**
   * @brief Retreive data from SD card.
   *
   * @details Reads sector data from SD card and transmits it over the CAN bus or
   * UART/USB interface. This is useful when reading out experimental data.
   *
   * @param sector_start: The first sector to read from.
   * @param sector_stop: The last sector to read from.
   */
  void dicio_dump_sdcard_data(uint32_t sector_start, uint32_t sector_stop);

  /**
   * @brief Fill node configuration structure with dummy data.
   *
   * @details This function is used to fill the node configuration structure with dummy
   * data. This is necessary to make sure valid data is loaded and can later be
   * overwritten with actual configuration data.
   *
   */
  void dicio_init_node_configurations(void);

  /**
   * @brief Stores node configuration data on the SD card.
   *
   * @details This function is used to store node configuration data on the SD card.
   * Since a single message cannot comntain the necessary data, this function
   * keeps track of previous messages using an internal buffer.
   *
   * This buffer can be cleared by passing a NULL pointer as the message.
   *
   * The buffer is written to the SD card by passing a message with
   * M_SENSOR_CONFIG_END as command. Attention: this will not clear the buffer.
   *
   * Messages are stored in raw format, no error checking is performed or
   * redundancy removed. This is the simplest way to store the data and retrieve
   * it for later use.
   */
  void dicio_process_node_config(const message_t *m);

#ifdef __cplusplus
}
#endif

#endif /* __DICIO_H__ */
