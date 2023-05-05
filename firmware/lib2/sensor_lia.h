#ifndef __SENSOR_LIA_H__
#define	__SENSOR_LIA_H__

#include <xc.h>
#include <sensor_common.h>
#include <sensor_adc16.h>
#include <pga.h>
    
    typedef enum
    {
        sensor_lia_gloxinia_register_general = 0x00,
        sensor_lia_gloxinia_register_pga_config = 0x01,
    } sensor_lia_gloxinia_register_t;
    
    typedef enum {
        PLANALTA_FS_FREQ_10HZ,  // 11
        PLANALTA_FS_FREQ_20HZ,  // 10
        PLANALTA_FS_FREQ_50HZ,  // 9
        PLANALTA_FS_FREQ_100HZ, // 8
        PLANALTA_FS_FREQ_200HZ, // 7
        PLANALTA_FS_FREQ_500HZ, // 6
        PLANALTA_FS_FREQ_1KHZ,  // 5
        PLANALTA_FS_FREQ_2KHZ,  // 4
        PLANALTA_FS_FREQ_5KHZ,  // 3
        PLANALTA_FS_FREQ_10KHZ, // 2
        PLANALTA_FS_FREQ_20KHZ, // 1
        PLANALTA_FS_FREQ_50KHZ, // 0
    } sensor_lia_fs_freq_t;
    
#define LIA_FS_FREQ_N 12

    typedef enum {
        PLANALTA_LIA_F_50KHZ,
        PLANALTA_LIA_F_25KHZ,
        PLANALTA_LIA_F_10KHZ,
        PLANALTA_LIA_F_5KHZ,
        PLANALTA_FS,
    } sensor_lia_op_mode_t;
    
    typedef struct sensor_lia_config_s
    {
        pga_config_t* pga_config;
        sensor_lia_fs_freq_t fs_low;
        sensor_lia_fs_freq_t fs_high;
        bool ouput_enable;
        sensor_lia_op_mode_t mode;
        
        fractional sample_i;
        fractional sample_q;
    } sensor_lia_config_t;
#ifdef	__cplusplus
extern "C" {
#endif

    // forward declaration of sensor_interface_s since cyclic dependency is not
    // allowed.
    struct sensor_gconfig_s;
    
    void sensor_lia_get_config(struct sensor_gconfig_s* intf, uint8_t reg, uint8_t* buffer, uint8_t* length);
    sensor_status_t sensor_lia_config(struct sensor_gconfig_s *intf, uint8_t *buffer, uint8_t length);
    void sensor_lia_measure(void *data);
    void lia_init_sensor(struct sensor_gconfig_s *intf);
    void sensor_lia_activate(struct sensor_gconfig_s* intf);
    bool validate_lia_config(sensor_lia_config_t *config);


#ifdef	__cplusplus
}
#endif

#endif

