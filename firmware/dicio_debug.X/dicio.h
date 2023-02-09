#ifndef __DICIO_H__
#define	__DICIO_H__ 

#include <xc.h>
#include <utilities.h>
#include <uart.h>
#include <message.h>

#define DICIO_CONFIG_ADDRESS 0x00000000U
#define DICIO_NODE_CONFIG_START_ADDRESS 0x00000020U
#define DICIO_DATA_START_ADDRESS 0x00000000U
#define DICIO_MAX_N_NODES 50U

typedef struct {
    uint32_t sector_address;
    uint16_t node_id;
    
    message_node_t node_type;
    uint8_t n_interfaces;
    bool stored_config;
    
    uint8_t v_hw;
    uint8_t v_sw_u;
    uint8_t v_sw_l;
} node_config_t;


#ifdef	__cplusplus
extern "C" {
#endif

    void dicio_init(void);
    
    void dicio_send_ready_message(void*);
    
    void dicio_init_pins(void);
    
    void dicio_read_sdconfig_data(void);
    void dicio_mount_fs(void);
    void dicio_unmount_fs(void);
    bool dicio_clear_data(void);
    void dicio_dump_sdcard_data(uint32_t sector_start, uint32_t sector_stop);
    void dicio_init_node_configurations(void);
    void dicio_process_node_config(const message_t* m);

#ifdef	__cplusplus
}
#endif

#endif	/* __DICIO_H__ */
