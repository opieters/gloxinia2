#ifndef __PLANALTA_DEFINITIONS_H__
#define	__PLANALTA_DEFINITIONS_H__

#include <xc.h>
#include <sensor_adc12.h>
#include <sensor_adc16.h>
#include <pga.h>
#include <i2c.h>

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




#ifdef	__cplusplus
}
#endif

#endif

