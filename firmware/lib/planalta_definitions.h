#ifndef __PLANALTA_DEFINITIONS_H__
#define	__PLANALTA_DEFINITIONS_H__

#include <xc.h> // include processor files - each processor file is guarded.  
#include <adc.h>
#include <pga.h>
#include <i2c.h>
#include "dac.h"

#ifdef	__cplusplus
extern "C" {
#endif
    
#define PLANALTA_CHANNEL_BUFFER_SIZE 16
#define PLANALTA_N_ADC_CHANNELS         8
#define PLANALTA_FS_CHANNELS 5

#define PLANALTA_I2C_BASE_ADDRESS  0b1011000
#define PLANALTA_I2C_READ_CH_BUFFER_LENGTH 5

#define PLANALTA_ADC_SAMPLE_FREQUENCY     160000

// length in bytes of the I2C read buffer
#define PLANALTA_I2C_READ_LIA_BUFFER_LENGTH    (4*PLANALTA_N_ADC_CHANNELS+1)
#define PLANALTA_I2C_READ_FS_BUFFER_LENGTH (5*PLANALTA_FS_FREQ_N*4+1)
#define PLANALTA_I2C_READ_BUFFER_LENGTH    MAX(PLANALTA_I2C_READ_LIA_BUFFER_LENGTH, PLANALTA_I2C_READ_FS_BUFFER_LENGTH)

// length in bytes of the I2C (master) write buffer, i.e. the max length a
// write message to this slave can be
#define PLANALTA_I2C_WRITE_BUFFER_LENGTH  (5)

#define PLANALTA_PRINT_VALUES_BUFFER_SIZE PLANALTA_BLOCK4_INPUT_SIZE // must be at least the size of PLANALTA_BLOCK5_INPUT_SIZE

#define PLANALTA_STATUS_ON           (0 << 7)
#define PLANALTA_STATUS_OFF          (1 << 7)
#define PLANALTA_STATUS_RESET_BUFFER (1 << 6)
#define PLANALTA_STATUS_RESET        (1 << 5)
#define PLANALTA_READ_STATE_MASK          (0b11 << 3)

#define PLANALTA_ADC_ON (1<<7)

#define PLANALTA_CH_CONFIG_ON (1<<7)
#define PLANALTA_CH_CONFIG_SET_GAIN(X) (((X)-PGA_GAIN_1) << 4)
#define PLANALTA_CH_CONFIG_GAIN (0b111 << 4)

#define PLANALTA_N_ADDRESS_SEL_PINS 8

typedef enum {
    PLANALTA_REG_STATUS      = 0x00,
    PLANALTA_REG_ADC         = 0x01,
    PLANALTA_REG_START_STOP  = 0x02,
    PLANALTA_REG_MODE        = 0x03,
    PLANALTA_REG_CONFIG_CH0  = 0x10,
    PLANALTA_REG_CONFIG_CH1  = 0x11,
    PLANALTA_REG_CONFIG_CH2  = 0x12,
    PLANALTA_REG_CONFIG_CH3  = 0x13,
    PLANALTA_REG_CONFIG_CH4  = 0x14,
    PLANALTA_REG_CONFIG_CH5  = 0x15,
    PLANALTA_REG_CONFIG_CH6  = 0x16,
    PLANALTA_REG_CONFIG_CH7  = 0x17,
    PLANALTA_REG_DATA_LIA    = 0x20,
    PLANALTA_REG_CONFIG_T0   = 0x30,
    PLANALTA_REG_CONFIG_T1   = 0x31,
    PLANALTA_REG_DATA_FS     = 0x40,
} planalta_reg_t;

typedef enum {
    PLANALTA_FS_FREQ_50KHZ, // 0
    PLANALTA_FS_FREQ_20KHZ, // 1
    PLANALTA_FS_FREQ_10KHZ, // 2
    PLANALTA_FS_FREQ_5KHZ,  // 3
    PLANALTA_FS_FREQ_2KHZ,  // 4
    PLANALTA_FS_FREQ_1KHZ,  // 5
    PLANALTA_FS_FREQ_500HZ, // 6
    PLANALTA_FS_FREQ_200HZ, // 7
    PLANALTA_FS_FREQ_100HZ, // 8
    PLANALTA_FS_FREQ_50HZ,  // 9
    PLANALTA_FS_FREQ_20HZ,  // 10
    PLANALTA_FS_FREQ_10HZ,  // 11
    PLANALTA_FS_FREQ_N,     // 12
} planalta_fs_freq_t;

typedef enum {
    PLANALTA_LIA_F_50KHZ,
    PLANALTA_LIA_F_25KHZ,
    PLANALTA_LIA_F_10KHZ,
    PLANALTA_LIA_F_5KHZ,
    PLANALTA_FS,
    PLANALTA_N_OP_MODES,
} planalta_op_mode_t;

typedef enum {
    PLANALTA_CHANNEL_DISABLED,
    PLANALTA_CHANNEL_ENABLED,
} planalta_channel_status_t;

typedef enum {
    PLANALTA_STATUS_INITIALISING,
    PLANALTA_STATUS_READY,
    PLANALTA_STATUS_RUNNING
} planalta_status_t;

typedef enum {
    PLANALTA_CHANNEL_DATA_STATUS_READ,
    PLANALTA_CHANNEL_DATA_STATUS_NEW,
} planalta_channel_data_status_t;

typedef struct planalta_config_s {
    adc_config_t adc_config;
    dac_config_t dac_config;
    pga_config_t pga_config[PLANALTA_N_ADC_CHANNELS];
    planalta_channel_status_t channel_status[PLANALTA_N_ADC_CHANNELS];
    planalta_channel_data_status_t channel_data_status[PLANALTA_N_ADC_CHANNELS];
    i2c_config_t i2c_config;
    const pin_t address_selection[PLANALTA_N_ADDRESS_SEL_PINS];
    pin_t blinky_pin;
    planalta_op_mode_t operation_mode;
    planalta_fs_freq_t sweep_frequency;
    pin_t filter_selection_pins[2];
    pin_t int_pin;
    planalta_status_t status;
} planalta_config_t;


#ifdef	__cplusplus
}
#endif

#endif

