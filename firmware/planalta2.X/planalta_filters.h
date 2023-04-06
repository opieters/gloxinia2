#ifndef __PLANALTA_FILTERING__
#define	__PLANALTA_FILTERING__

#include <xc.h>
#include <utilities.h>
#include <dsp.h>
#include <stdbool.h>
#include <sensor_adc12.h>
#include "planalta_definitions.h"

#define ADC_FS_BUFFER_LENGTH            200
#define ADC_FS_50KHZ_BUFFER_LENGTH      100
#define ADC_FS_20KHZ_10KHZ_5KHZ_BUFFER_LENGTH      100
#define ADC_FS_2KHZ_1KHZ_500HZ_BUFFER_LENGTH       200
#define ADC_FS_200HZ_100HZ_50HZ_BUFFER_LENGTH      200
#define ADC_FS_20HZ_10HZ_BUFFER_LENGTH       200

#define ADC_50KHZ_BUFFER_LENGTH         100
#define ADC_25KHZ_BUFFER_LENGTH         200
#define ADC_10KHZ_BUFFER_LENGTH         200
#define ADC_5KHZ_BUFFER_LENGTH          160

#define PLANALTA_FS_N_ADC_CHANNELS      1

#define PLANALTA_50KHZ_N_ADC_CHANNELS   1
#define PLANALTA_25KHZ_N_ADC_CHANNELS   2
#define PLANALTA_10KHZ_N_ADC_CHANNELS   4
#define PLANALTA_5KHZ_N_ADC_CHANNELS    8

#define FIR0_N_COPIES  8
#define FIR1_N_COPIES  8
#define FIR2_N_COPIES  8
#define FIR3_N_COPIES  8
#define FIR4_N_COPIES  8
#define FIR6_N_COPIES  2
#define FIR7_N_COPIES  4
#define FIR8_N_COPIES  8
#define FIR9_N_COPIES  1
#define FIR10_N_COPIES 2

#define N_FIR_COEFFS0     24
#define N_FIR_COEFFS1_I   23
#define N_FIR_COEFFS1_Q   23
#define N_FIR_COEFFS2     96
#define N_FIR_COEFFS3     96
#define N_FIR_COEFFS4     96
#define N_FIR_COEFFS6     200
#define N_FIR_SHARED     256
#define N_FIR_COEFFS7     80
#define N_FIR_COEFFS8     40
#define N_FIR_COEFFS9     160
#define N_FIR_COEFFS10    200
#define N_FIR_COEFFS11_I    80
#define N_FIR_COEFFS11_Q    80
#define N_FIR_COEFFS12_I    40
#define N_FIR_COEFFS12_Q    40

#define PLANALTA_5KHZ_DEC_FACT_F0         (1)
#define PLANALTA_5KHZ_F0_OUTPUT_SIZE      (ADC_5KHZ_BUFFER_LENGTH / PLANALTA_5KHZ_N_ADC_CHANNELS)
#define PLANALTA_5KHZ_DEC_FACT_F1         (5)
#define PLANALTA_5KHZ_F1_INPUT_SIZE       (PLANALTA_5KHZ_F0_OUTPUT_SIZE / 2)
#define PLANALTA_5KHZ_F1_OUTPUT_SIZE      (PLANALTA_5KHZ_F1_INPUT_SIZE / PLANALTA_5KHZ_DEC_FACT_F1)
#define PLANALTA_5KHZ_DEC_FACT_F2         (10)
#define PLANALTA_5KHZ_F2_INPUT_SIZE       (PLANALTA_5KHZ_DEC_FACT_F2)
#define PLANALTA_5KHZ_F2_OUTPUT_SIZE      (PLANALTA_5KHZ_F2_INPUT_SIZE / PLANALTA_5KHZ_DEC_FACT_F2)
#define PLANALTA_5KHZ_DEC_FACT_F3         (10)
#define PLANALTA_5KHZ_F3_INPUT_SIZE       (PLANALTA_5KHZ_DEC_FACT_F3)
#define PLANALTA_5KHZ_F3_OUTPUT_SIZE      (PLANALTA_5KHZ_F3_INPUT_SIZE / PLANALTA_5KHZ_DEC_FACT_F3)
#define PLANALTA_5KHZ_DEC_FACT_F4         (10)
#define PLANALTA_5KHZ_F4_INPUT_SIZE       (PLANALTA_5KHZ_DEC_FACT_F4)
#define PLANALTA_5KHZ_F4_OUTPUT_SIZE      (PLANALTA_5KHZ_F4_INPUT_SIZE / PLANALTA_5KHZ_DEC_FACT_F4)
#define PLANALTA_5KHZ_DEC_FACT_F5         (1)
#define PLANALTA_5KHZ_F5_INPUT_SIZE       (PLANALTA_5KHZ_DEC_FACT_F5)
#define PLANALTA_5KHZ_F5_OUTPUT_SIZE      (PLANALTA_5KHZ_F5_INPUT_SIZE / PLANALTA_5KHZ_DEC_FACT_F5)

#define PLANALTA_10KHZ_F0_OUTPUT_SIZE      (ADC_10KHZ_BUFFER_LENGTH / PLANALTA_10KHZ_N_ADC_CHANNELS)
#define PLANALTA_10KHZ_F1_INPUT_SIZE       (PLANALTA_10KHZ_F0_OUTPUT_SIZE / 2)
#define PLANALTA_10KHZ_F1_OUTPUT_SIZE      (PLANALTA_10KHZ_F0_OUTPUT_SIZE / PLANALTA_DEC_FACT_F1 / 2)
#define PLANALTA_10KHZ_F2_INPUT_SIZE       (PLANALTA_DEC_FACT_F2)
#define PLANALTA_10KHZ_F2_OUTPUT_SIZE      (PLANALTA_10KHZ_F2_INPUT_SIZE / PLANALTA_DEC_FACT_F2)
#define PLANALTA_10KHZ_F3_INPUT_SIZE       (PLANALTA_DEC_FACT_F3)
#define PLANALTA_10KHZ_F3_OUTPUT_SIZE      (PLANALTA_10KHZ_F3_INPUT_SIZE / PLANALTA_DEC_FACT_F3)
#define PLANALTA_10KHZ_F4_INPUT_SIZE       (PLANALTA_DEC_FACT_F4)
#define PLANALTA_10KHZ_F4_OUTPUT_SIZE      (PLANALTA_10KHZ_F4_INPUT_SIZE / PLANALTA_DEC_FACT_F4)
#define PLANALTA_10KHZ_F5_INPUT_SIZE       (PLANALTA_DEC_FACT_F7)
#define PLANALTA_10KHZ_F5_OUTPUT_SIZE      (PLANALTA_10KHZ_F5_INPUT_SIZE / PLANALTA_DEC_FACT_F7)

#define PLANALTA_25KHZ_F0_OUTPUT_SIZE      (ADC_25KHZ_BUFFER_LENGTH / PLANALTA_25KHZ_N_ADC_CHANNELS)
#define PLANALTA_25KHZ_F1_INPUT_SIZE       (PLANALTA_25KHZ_F0_OUTPUT_SIZE / 2)
#define PLANALTA_25KHZ_F1_OUTPUT_SIZE      (PLANALTA_25KHZ_F0_OUTPUT_SIZE / PLANALTA_DEC_FACT_F1 / 2)
#define PLANALTA_25KHZ_F2_INPUT_SIZE       (PLANALTA_DEC_FACT_F2)
#define PLANALTA_25KHZ_F2_OUTPUT_SIZE      (PLANALTA_25KHZ_F2_INPUT_SIZE / PLANALTA_DEC_FACT_F2)
#define PLANALTA_25KHZ_F3_INPUT_SIZE       (PLANALTA_DEC_FACT_F3)
#define PLANALTA_25KHZ_F3_OUTPUT_SIZE      (PLANALTA_25KHZ_F3_INPUT_SIZE / PLANALTA_DEC_FACT_F3)
#define PLANALTA_25KHZ_F4_INPUT_SIZE       (PLANALTA_DEC_FACT_F4)
#define PLANALTA_25KHZ_F4_OUTPUT_SIZE      (PLANALTA_25KHZ_F4_INPUT_SIZE / PLANALTA_DEC_FACT_F4)
#define PLANALTA_25KHZ_F5_INPUT_SIZE       (PLANALTA_DEC_FACT_F6)
#define PLANALTA_25KHZ_F5_OUTPUT_SIZE      (PLANALTA_25KHZ_F5_INPUT_SIZE / PLANALTA_DEC_FACT_F6)

#define PLANALTA_50KHZ_F0_OUTPUT_SIZE      (ADC_50KHZ_BUFFER_LENGTH / PLANALTA_50KHZ_N_ADC_CHANNELS)
#define PLANALTA_50KHZ_F1_INPUT_SIZE       (PLANALTA_50KHZ_F0_OUTPUT_SIZE / 2)
#define PLANALTA_50KHZ_F1_OUTPUT_SIZE      (PLANALTA_50KHZ_F0_OUTPUT_SIZE / PLANALTA_DEC_FACT_F1 / 2)
#define PLANALTA_50KHZ_F2_INPUT_SIZE       (PLANALTA_DEC_FACT_F2)
#define PLANALTA_50KHZ_F2_OUTPUT_SIZE      (PLANALTA_50KHZ_F2_INPUT_SIZE / PLANALTA_DEC_FACT_F2)
#define PLANALTA_50KHZ_F3_INPUT_SIZE       (PLANALTA_DEC_FACT_F3)
#define PLANALTA_50KHZ_F3_OUTPUT_SIZE      (PLANALTA_50KHZ_F3_INPUT_SIZE / PLANALTA_DEC_FACT_F3)
#define PLANALTA_50KHZ_F4_INPUT_SIZE       (PLANALTA_DEC_FACT_F4)
#define PLANALTA_50KHZ_F4_OUTPUT_SIZE      (PLANALTA_50KHZ_F4_INPUT_SIZE / PLANALTA_DEC_FACT_F4)
#define PLANALTA_50KHZ_F5_INPUT_SIZE       (PLANALTA_DEC_FACT_F8 * PLANALTA_DEC_FACT_F4)
#define PLANALTA_50KHZ_F5_OUTPUT_SIZE      (PLANALTA_50KHZ_F5_INPUT_SIZE / PLANALTA_DEC_FACT_F4)
#define PLANALTA_50KHZ_F6_INPUT_SIZE       (PLANALTA_DEC_FACT_F8)
#define PLANALTA_50KHZ_F6_OUTPUT_SIZE      (PLANALTA_50KHZ_F6_INPUT_SIZE / PLANALTA_DEC_FACT_F8)

#define PLANALTA_FS_50KHZ_F0_DEC_FACT         1
#define PLANALTA_FS_50KHZ_F0_INPUT_SIZE       (ADC_FS_50KHZ_BUFFER_LENGTH / PLANALTA_FS_N_ADC_CHANNELS)
#define PLANALTA_FS_50KHZ_F0_OUTPUT_SIZE      (PLANALTA_FS_50KHZ_F0_INPUT_SIZE / PLANALTA_FS_50KHZ_F0_DEC_FACT)
#define PLANALTA_FS_50KHZ_F1_DEC_FACT         5
#define PLANALTA_FS_50KHZ_F1_INPUT_SIZE       (PLANALTA_FS_50KHZ_F0_OUTPUT_SIZE / 2)
#define PLANALTA_FS_50KHZ_F1_OUTPUT_SIZE      (PLANALTA_FS_50KHZ_F1_INPUT_SIZE / PLANALTA_FS_50KHZ_F1_DEC_FACT)
#define PLANALTA_FS_50KHZ_F2_DEC_FACT         10
#define PLANALTA_FS_50KHZ_F2_INPUT_SIZE       (PLANALTA_FS_50KHZ_F2_DEC_FACT)
#define PLANALTA_FS_50KHZ_F2_OUTPUT_SIZE      (PLANALTA_FS_50KHZ_F2_INPUT_SIZE / PLANALTA_FS_50KHZ_F2_DEC_FACT)
#define PLANALTA_FS_50KHZ_F3_DEC_FACT         10
#define PLANALTA_FS_50KHZ_F3_INPUT_SIZE       (PLANALTA_FS_50KHZ_F3_DEC_FACT)
#define PLANALTA_FS_50KHZ_F3_OUTPUT_SIZE      (PLANALTA_FS_50KHZ_F3_INPUT_SIZE / PLANALTA_FS_50KHZ_F3_DEC_FACT)
#define PLANALTA_FS_50KHZ_F4_DEC_FACT         10
#define PLANALTA_FS_50KHZ_F4_INPUT_SIZE       (PLANALTA_FS_50KHZ_F4_DEC_FACT)
#define PLANALTA_FS_50KHZ_F4_OUTPUT_SIZE      (PLANALTA_FS_50KHZ_F4_INPUT_SIZE / PLANALTA_FS_50KHZ_F4_DEC_FACT)
#define PLANALTA_FS_50KHZ_F5_DEC_FACT         2
#define PLANALTA_FS_50KHZ_F5_INPUT_SIZE       (PLANALTA_FS_50KHZ_F5_DEC_FACT)
#define PLANALTA_FS_50KHZ_F5_OUTPUT_SIZE      (PLANALTA_FS_50KHZ_F5_INPUT_SIZE / PLANALTA_FS_50KHZ_F5_DEC_FACT)


#define PLANALTA_FS_20KHZ_10KHZ_5KHZ_F0_DEC_FACT         1
#define PLANALTA_FS_20KHZ_10KHZ_5KHZ_F0_INPUT_SIZE       (ADC_FS_20KHZ_10KHZ_5KHZ_BUFFER_LENGTH / PLANALTA_FS_N_ADC_CHANNELS)
#define PLANALTA_FS_20KHZ_10KHZ_5KHZ_F0_OUTPUT_SIZE      (PLANALTA_FS_20KHZ_10KHZ_5KHZ_F0_INPUT_SIZE / PLANALTA_FS_20KHZ_10KHZ_5KHZ_F0_DEC_FACT)
#define PLANALTA_FS_20KHZ_10KHZ_5KHZ_F1_DEC_FACT         5
#define PLANALTA_FS_20KHZ_10KHZ_5KHZ_F1_INPUT_SIZE       (PLANALTA_FS_20KHZ_10KHZ_5KHZ_F0_OUTPUT_SIZE / 2)
#define PLANALTA_FS_20KHZ_10KHZ_5KHZ_F1_OUTPUT_SIZE      (PLANALTA_FS_20KHZ_10KHZ_5KHZ_F1_INPUT_SIZE / PLANALTA_FS_20KHZ_10KHZ_5KHZ_F1_DEC_FACT)
#define PLANALTA_FS_20KHZ_10KHZ_5KHZ_F2_DEC_FACT         10
#define PLANALTA_FS_20KHZ_10KHZ_5KHZ_F2_INPUT_SIZE       (PLANALTA_FS_20KHZ_10KHZ_5KHZ_F2_DEC_FACT)
#define PLANALTA_FS_20KHZ_10KHZ_5KHZ_F2_OUTPUT_SIZE      (PLANALTA_FS_20KHZ_10KHZ_5KHZ_F2_INPUT_SIZE / PLANALTA_FS_20KHZ_10KHZ_5KHZ_F2_DEC_FACT)
#define PLANALTA_FS_20KHZ_10KHZ_5KHZ_F3_DEC_FACT         10
#define PLANALTA_FS_20KHZ_10KHZ_5KHZ_F3_INPUT_SIZE       (PLANALTA_FS_20KHZ_10KHZ_5KHZ_F3_DEC_FACT)
#define PLANALTA_FS_20KHZ_10KHZ_5KHZ_F3_OUTPUT_SIZE      (PLANALTA_FS_20KHZ_10KHZ_5KHZ_F3_INPUT_SIZE / PLANALTA_FS_20KHZ_10KHZ_5KHZ_F3_DEC_FACT)
#define PLANALTA_FS_20KHZ_F4_DEC_FACT         8
#define PLANALTA_FS_20KHZ_F4_INPUT_SIZE       PLANALTA_FS_20KHZ_F4_DEC_FACT
#define PLANALTA_FS_20KHZ_F4_OUTPUT_SIZE      (PLANALTA_FS_20KHZ_F4_INPUT_SIZE / PLANALTA_FS_20KHZ_F4_DEC_FACT)
#define PLANALTA_FS_10KHZ_F4_DEC_FACT         10
#define PLANALTA_FS_10KHZ_F4_INPUT_SIZE       (PLANALTA_FS_10KHZ_F4_DEC_FACT)
#define PLANALTA_FS_10KHZ_F4_OUTPUT_SIZE      (PLANALTA_FS_10KHZ_F4_INPUT_SIZE / PLANALTA_FS_10KHZ_F4_DEC_FACT)
#define PLANALTA_FS_5KHZ_F4_DEC_FACT         2
#define PLANALTA_FS_5KHZ_F4_INPUT_SIZE       (PLANALTA_FS_5KHZ_F4_DEC_FACT)
#define PLANALTA_FS_5KHZ_F4_OUTPUT_SIZE      (PLANALTA_FS_5KHZ_F4_INPUT_SIZE / PLANALTA_FS_5KHZ_F4_DEC_FACT)


#define PLANALTA_FS_2KHZ_1KHZ_500HZ_F0_DEC_FACT         10
#define PLANALTA_FS_2KHZ_1KHZ_500HZ_F0_INPUT_SIZE       (ADC_FS_2KHZ_1KHZ_500HZ_BUFFER_LENGTH / PLANALTA_FS_N_ADC_CHANNELS)
#define PLANALTA_FS_2KHZ_1KHZ_500HZ_F0_OUTPUT_SIZE      (PLANALTA_FS_2KHZ_1KHZ_500HZ_F0_INPUT_SIZE / PLANALTA_FS_2KHZ_1KHZ_500HZ_F0_DEC_FACT)
#define PLANALTA_FS_2KHZ_1KHZ_500HZ_F1_DEC_FACT         1
#define PLANALTA_FS_2KHZ_1KHZ_500HZ_F1_INPUT_SIZE       PLANALTA_FS_2KHZ_1KHZ_500HZ_F0_OUTPUT_SIZE
#define PLANALTA_FS_2KHZ_1KHZ_500HZ_F1_OUTPUT_SIZE      (PLANALTA_FS_2KHZ_1KHZ_500HZ_F1_INPUT_SIZE / PLANALTA_FS_2KHZ_1KHZ_500HZ_F1_DEC_FACT)
#define PLANALTA_FS_2KHZ_1KHZ_500HZ_F2_DEC_FACT         5
#define PLANALTA_FS_2KHZ_1KHZ_500HZ_F2_INPUT_SIZE       (PLANALTA_FS_2KHZ_1KHZ_500HZ_F1_OUTPUT_SIZE / 2)
#define PLANALTA_FS_2KHZ_1KHZ_500HZ_F2_OUTPUT_SIZE      (PLANALTA_FS_2KHZ_1KHZ_500HZ_F2_INPUT_SIZE / PLANALTA_FS_2KHZ_1KHZ_500HZ_F2_DEC_FACT)
#define PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_DEC_FACT         10
#define PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_INPUT_SIZE       (PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_DEC_FACT)
#define PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_OUTPUT_SIZE      (PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_INPUT_SIZE / PLANALTA_FS_2KHZ_1KHZ_500HZ_F3_DEC_FACT)
#define PLANALTA_FS_2KHZ_F4_DEC_FACT         8
#define PLANALTA_FS_2KHZ_F4_INPUT_SIZE       (PLANALTA_FS_2KHZ_F4_DEC_FACT)
#define PLANALTA_FS_2KHZ_F4_OUTPUT_SIZE      (PLANALTA_FS_2KHZ_F4_INPUT_SIZE / PLANALTA_FS_2KHZ_F4_DEC_FACT)
#define PLANALTA_FS_1KHZ_F4_DEC_FACT         4
#define PLANALTA_FS_1KHZ_F4_INPUT_SIZE       (PLANALTA_FS_1KHZ_F4_DEC_FACT)
#define PLANALTA_FS_1KHZ_F4_OUTPUT_SIZE      (PLANALTA_FS_1KHZ_F4_INPUT_SIZE / PLANALTA_FS_1KHZ_F4_DEC_FACT)
#define PLANALTA_FS_500HZ_F4_DEC_FACT         2
#define PLANALTA_FS_500HZ_F4_INPUT_SIZE       (PLANALTA_FS_500HZ_F4_DEC_FACT)
#define PLANALTA_FS_500HZ_F4_OUTPUT_SIZE      (PLANALTA_FS_500HZ_F4_INPUT_SIZE / PLANALTA_FS_500HZ_F4_DEC_FACT)


#define PLANALTA_FS_500HZ_F0_DEC_FACT         10
#define PLANALTA_FS_500HZ_F0_INPUT_SIZE       (ADC_FS_500HZ_BUFFER_LENGTH / PLANALTA_FS_N_ADC_CHANNELS)
#define PLANALTA_FS_500HZ_F0_OUTPUT_SIZE      (PLANALTA_FS_500HZ_F0_INPUT_SIZE / PLANALTA_FS_500HZ_F0_DEC_FACT)
#define PLANALTA_FS_500HZ_F1_DEC_FACT         1
#define PLANALTA_FS_500HZ_F1_INPUT_SIZE       (PLANALTA_FS_500HZ_F0_OUTPUT_SIZE)
#define PLANALTA_FS_500HZ_F1_OUTPUT_SIZE      (PLANALTA_FS_500HZ_F1_INPUT_SIZE / PLANALTA_FS_500HZ_F1_INPUT_SIZE)
#define PLANALTA_FS_500HZ_F2_DEC_FACT         5
#define PLANALTA_FS_500HZ_F2_INPUT_SIZE       (PLANALTA_FS_500HZ_F1_OUTPUT_SIZE / 2)
#define PLANALTA_FS_500HZ_F2_OUTPUT_SIZE      (PLANALTA_FS_500HZ_F2_INPUT_SIZE / PLANALTA_FS_500HZ_F2_DEC_FACT)
#define PLANALTA_FS_500HZ_F3_DEC_FACT         10
#define PLANALTA_FS_500HZ_F3_INPUT_SIZE       (PLANALTA_FS_500HZ_F3_DEC_FACT)
#define PLANALTA_FS_500HZ_F3_OUTPUT_SIZE      (PLANALTA_FS_500HZ_F3_INPUT_SIZE / PLANALTA_FS_500HZ_F3_DEC_FACT)


#define PLANALTA_FS_200HZ_100HZ_50HZ_F0_DEC_FACT         10
#define PLANALTA_FS_200HZ_100HZ_50HZ_F0_INPUT_SIZE       (ADC_FS_200HZ_100HZ_50HZ_BUFFER_LENGTH / PLANALTA_FS_N_ADC_CHANNELS)
#define PLANALTA_FS_200HZ_100HZ_50HZ_F0_OUTPUT_SIZE      (PLANALTA_FS_200HZ_100HZ_50HZ_F0_INPUT_SIZE / PLANALTA_FS_200HZ_100HZ_50HZ_F0_DEC_FACT)
#define PLANALTA_FS_200HZ_100HZ_50HZ_F1_DEC_FACT         10
#define PLANALTA_FS_200HZ_100HZ_50HZ_F1_INPUT_SIZE       (PLANALTA_FS_200HZ_100HZ_50HZ_F0_OUTPUT_SIZE)
#define PLANALTA_FS_200HZ_100HZ_50HZ_F1_OUTPUT_SIZE      (PLANALTA_FS_200HZ_100HZ_50HZ_F1_INPUT_SIZE / PLANALTA_FS_200HZ_100HZ_50HZ_F1_DEC_FACT)
#define PLANALTA_FS_200HZ_100HZ_50HZ_F2_DEC_FACT         1
#define PLANALTA_FS_200HZ_100HZ_50HZ_F2_INPUT_SIZE       PLANALTA_FS_200HZ_100HZ_50HZ_F1_OUTPUT_SIZE
#define PLANALTA_FS_200HZ_100HZ_50HZ_F2_OUTPUT_SIZE      (PLANALTA_FS_200HZ_100HZ_50HZ_F1_OUTPUT_SIZE / PLANALTA_FS_200HZ_100HZ_50HZ_F2_DEC_FACT)
#define PLANALTA_FS_200HZ_100HZ_50HZ_F3_DEC_FACT         5
#define PLANALTA_FS_200HZ_100HZ_50HZ_F3_INPUT_SIZE       (4*PLANALTA_FS_200HZ_100HZ_50HZ_F3_DEC_FACT)
#define PLANALTA_FS_200HZ_100HZ_50HZ_F3_OUTPUT_SIZE      (PLANALTA_FS_200HZ_100HZ_50HZ_F3_INPUT_SIZE / 2 / PLANALTA_FS_200HZ_100HZ_50HZ_F3_DEC_FACT)
#define PLANALTA_FS_200HZ_F4_DEC_FACT                    8
#define PLANALTA_FS_200HZ_F4_INPUT_SIZE                  (PLANALTA_FS_200HZ_F4_DEC_FACT)
#define PLANALTA_FS_200HZ_F4_OUTPUT_SIZE                 (PLANALTA_FS_200HZ_F4_INPUT_SIZE / PLANALTA_FS_200HZ_F4_DEC_FACT)
#define PLANALTA_FS_100HZ_F4_DEC_FACT                    4
#define PLANALTA_FS_100HZ_F4_INPUT_SIZE                  (PLANALTA_FS_100HZ_F4_DEC_FACT)
#define PLANALTA_FS_100HZ_F4_OUTPUT_SIZE                 (PLANALTA_FS_100HZ_F4_INPUT_SIZE / PLANALTA_FS_100HZ_F4_DEC_FACT)
#define PLANALTA_FS_50HZ_F4_DEC_FACT                     2
#define PLANALTA_FS_50HZ_F4_INPUT_SIZE                   (PLANALTA_FS_50HZ_F4_DEC_FACT)
#define PLANALTA_FS_50HZ_F4_OUTPUT_SIZE                  (PLANALTA_FS_50HZ_F4_INPUT_SIZE / PLANALTA_FS_50HZ_F4_DEC_FACT)

#define PLANALTA_FS_20HZ_10HZ_F0_DEC_FACT         10
#define PLANALTA_FS_20HZ_10HZ_F0_INPUT_SIZE       (ADC_FS_20HZ_10HZ_BUFFER_LENGTH / PLANALTA_FS_N_ADC_CHANNELS)
#define PLANALTA_FS_20HZ_10HZ_F0_OUTPUT_SIZE      (PLANALTA_FS_20HZ_10HZ_F0_INPUT_SIZE / PLANALTA_FS_20HZ_10HZ_F0_DEC_FACT)
#define PLANALTA_FS_20HZ_10HZ_F1_DEC_FACT         10
#define PLANALTA_FS_20HZ_10HZ_F1_INPUT_SIZE       (PLANALTA_FS_20HZ_10HZ_F0_OUTPUT_SIZE)
#define PLANALTA_FS_20HZ_10HZ_F1_OUTPUT_SIZE      (PLANALTA_FS_20HZ_10HZ_F1_INPUT_SIZE / PLANALTA_FS_20HZ_10HZ_F1_DEC_FACT)
#define PLANALTA_FS_20HZ_10HZ_F2_DEC_FACT         10
#define PLANALTA_FS_20HZ_10HZ_F2_INPUT_SIZE       (PLANALTA_FS_20HZ_10HZ_F2_DEC_FACT)
#define PLANALTA_FS_20HZ_10HZ_F2_OUTPUT_SIZE      (PLANALTA_FS_20HZ_10HZ_F2_INPUT_SIZE / PLANALTA_FS_20HZ_10HZ_F2_DEC_FACT)
#define PLANALTA_FS_20HZ_10HZ_F3_DEC_FACT         1
#define PLANALTA_FS_20HZ_10HZ_F3_INPUT_SIZE       (PLANALTA_FS_20HZ_10HZ_F3_DEC_FACT)
#define PLANALTA_FS_20HZ_10HZ_F3_OUTPUT_SIZE      (PLANALTA_FS_20HZ_10HZ_F3_INPUT_SIZE / PLANALTA_FS_20HZ_10HZ_F3_DEC_FACT)
#define PLANALTA_FS_20HZ_F4_DEC_FACT              4
#define PLANALTA_FS_20HZ_F4_INPUT_SIZE            (2*PLANALTA_FS_20HZ_F4_DEC_FACT)
#define PLANALTA_FS_20HZ_F4_OUTPUT_SIZE           (PLANALTA_FS_20HZ_F4_INPUT_SIZE / 2 / PLANALTA_FS_20HZ_F4_DEC_FACT)
#define PLANALTA_FS_10HZ_F4_DEC_FACT              2
#define PLANALTA_FS_10HZ_F4_INPUT_SIZE            (2*PLANALTA_FS_10HZ_F4_DEC_FACT)
#define PLANALTA_FS_10HZ_F4_OUTPUT_SIZE           (PLANALTA_FS_10HZ_F4_INPUT_SIZE / 2 / PLANALTA_FS_10HZ_F4_DEC_FACT)

#define PLANALTA_COPY_BUFFER_SIZE         (50*PLANALTA_DEC_FACT_F0)
#define PLANALTA_BLOCK1_INPUT_SIZE        (10*PLANALTA_DEC_FACT_F1)
#define PLANALTA_BLOCK2_INPUT_SIZE        (10*PLANALTA_DEC_FACT_F2)
#define PLANALTA_BLOCK3_INPUT_SIZE        (PLANALTA_DEC_FACT_F3)
#define PLANALTA_BLOCK4_INPUT_SIZE        (PLANALTA_DEC_FACT_F4)
#define PLANALTA_BLOCK0_OUTPUT_SIZE       (PLANALTA_COPY_BUFFER_SIZE  / PLANALTA_DEC_FACT_F0)
#define PLANALTA_BLOCK1_OUTPUT_SIZE       (PLANALTA_BLOCK1_INPUT_SIZE / PLANALTA_DEC_FACT_F1)
#define PLANALTA_BLOCK2_OUTPUT_SIZE       (PLANALTA_BLOCK2_INPUT_SIZE / PLANALTA_DEC_FACT_F2)
#define PLANALTA_BLOCK3_OUTPUT_SIZE       (PLANALTA_BLOCK3_INPUT_SIZE / PLANALTA_DEC_FACT_F3)
#define PLANALTA_BLOCK4_OUTPUT_SIZE       (PLANALTA_BLOCK4_INPUT_SIZE / PLANALTA_DEC_FACT_F4)

#define PLANALTA_F2_INPUT_SIZE 10
#define PLANALTA_F3_INPUT_SIZE 10
#define PLANALTA_F4_INPUT_SIZE 10
#define PLANALTA_F5_INPUT_SIZE 20


#define PLANALTA_DEC_FACT_F0 1
// F1 has a decimation factor of 10, but due to the mixing optimisation, the 
// effective rate becomes 5
#define PLANALTA_DEC_FACT_F1 5
#define PLANALTA_DEC_FACT_F2 10
#define PLANALTA_DEC_FACT_F3 10
#define PLANALTA_DEC_FACT_F4 10
#define PLANALTA_DEC_FACT_F6 10
#define PLANALTA_DEC_FACT_F7 4
#define PLANALTA_DEC_FACT_F8 2
#define PLANALTA_DEC_FACT_F9 8
#define PLANALTA_DEC_FACT_F10 10

#ifdef	__cplusplus
extern "C" {
#endif
    
extern fractional fir_coeffs_0[N_FIR_COEFFS0];
extern fractional fir_coeffs_1_i[N_FIR_COEFFS1_I];
extern fractional fir_coeffs_1_q[N_FIR_COEFFS1_Q];
extern fractional fir_coeffs_2[N_FIR_COEFFS2];
extern fractional fir_coeffs_3[N_FIR_COEFFS3];
extern fractional fir_coeffs_4[N_FIR_COEFFS4];
extern fractional fir_coeffs_6[N_FIR_COEFFS6];
extern fractional fir_coeffs_7[N_FIR_COEFFS7];
extern fractional fir_coeffs_8[N_FIR_COEFFS8];
extern fractional fir_coeffs_9[N_FIR_COEFFS9];
extern fractional fir_coeffs_10[N_FIR_COEFFS10];
extern fractional fir_coeffs_11_i[N_FIR_COEFFS11_I];
extern fractional fir_coeffs_11_q[N_FIR_COEFFS11_Q];
extern fractional fir_coeffs_12_i[N_FIR_COEFFS12_I];
extern fractional fir_coeffs_12_q[N_FIR_COEFFS12_Q];


extern FIRStruct planalta_lia_filters_0[PLANALTA_N_ADC_CHANNELS];
extern FIRStruct planalta_lia_filters_1_q[PLANALTA_N_ADC_CHANNELS];
extern FIRStruct planalta_lia_filters_1_i[PLANALTA_N_ADC_CHANNELS];
extern FIRStruct planalta_lia_filters_2_q[PLANALTA_N_ADC_CHANNELS];
extern FIRStruct planalta_lia_filters_2_i[PLANALTA_N_ADC_CHANNELS];
extern FIRStruct planalta_lia_filters_3_q[PLANALTA_N_ADC_CHANNELS];
extern FIRStruct planalta_lia_filters_3_i[PLANALTA_N_ADC_CHANNELS];
extern FIRStruct planalta_lia_filters_4_q[PLANALTA_N_ADC_CHANNELS];
extern FIRStruct planalta_lia_filters_4_i[PLANALTA_N_ADC_CHANNELS];
extern FIRStruct planalta_lia_filters_5_q[PLANALTA_N_ADC_CHANNELS];
extern FIRStruct planalta_lia_filters_5_i[PLANALTA_N_ADC_CHANNELS];
extern FIRStruct* planalta_lia_filters_6_q;
extern FIRStruct* planalta_lia_filters_6_i;

extern FIRStruct planalta_fs_filters_0;
extern FIRStruct planalta_fs_filters_1;
extern FIRStruct planalta_fs_filters_1_i;
extern FIRStruct planalta_fs_filters_1_q;
extern FIRStruct planalta_fs_filters_2;
extern FIRStruct planalta_fs_filters_2_i;
extern FIRStruct planalta_fs_filters_2_q;
extern FIRStruct planalta_fs_filters_3;
extern FIRStruct planalta_fs_filters_3_i;
extern FIRStruct planalta_fs_filters_3_q;
extern FIRStruct planalta_fs_filters_4_i;
extern FIRStruct planalta_fs_filters_4_q;
extern FIRStruct planalta_fs_filters_5_i;
extern FIRStruct planalta_fs_filters_5_q;

extern FIRStruct* const planalta_fs_filter_0;
extern FIRStruct* const planalta_fs_filter_1;
extern FIRStruct* const planalta_fs_filter_1_q;
extern FIRStruct* const planalta_fs_filter_1_i;
extern FIRStruct* const planalta_fs_filter_2;
extern FIRStruct* const planalta_fs_filter_2_q;
extern FIRStruct* const planalta_fs_filter_2_i;
extern FIRStruct* const planalta_fs_filter_3;
extern FIRStruct* const planalta_fs_filter_3_q;
extern FIRStruct* const planalta_fs_filter_3_i;
extern FIRStruct* const planalta_fs_filter_4_q;
extern FIRStruct* const planalta_fs_filter_4_i;
extern FIRStruct* const planalta_fs_filter_6_q;
extern FIRStruct* const planalta_fs_filter_6_i;
extern FIRStruct* const planalta_fs_filter_7_q;
extern FIRStruct* const planalta_fs_filter_7_i;
extern FIRStruct* const planalta_fs_filter_8_q;
extern FIRStruct* const planalta_fs_filter_8_i;
extern FIRStruct* const planalta_fs_filter_9_q;
extern FIRStruct* const planalta_fs_filter_9_i;
extern FIRStruct* const planalta_fs_filter_10_q;
extern FIRStruct* const planalta_fs_filter_10_i;
extern FIRStruct* const planalta_fs_filter_11;
extern FIRStruct* const planalta_fs_filter_12;

extern uint8_t start_filter_block1, start_filter_block2, start_filter_block3, start_filter_block4, start_filter_block5;


extern fractional* fo1_buffer_i_write[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo1_buffer_q_write[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo2_buffer_i_write[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo2_buffer_q_write[PLANALTA_N_ADC_CHANNELS];

extern fractional* fo2_buffer_i_write[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo2_buffer_q_write[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo3_buffer_i_write[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo3_buffer_q_write[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo4_buffer_i_write[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo4_buffer_q_write[PLANALTA_N_ADC_CHANNELS];

extern fractional* fo2_buffer_i_read[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo2_buffer_q_read[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo3_buffer_i_read[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo3_buffer_q_read[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo4_buffer_i_read[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo4_buffer_q_read[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo5_buffer_i_read[PLANALTA_N_ADC_CHANNELS];
extern fractional* fo5_buffer_q_read[PLANALTA_N_ADC_CHANNELS];

extern fractional f1_to_f2_buffer_i_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F2_INPUT_SIZE];
extern fractional f1_to_f2_buffer_q_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F2_INPUT_SIZE];
extern fractional f1_to_f2_buffer_i_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F2_INPUT_SIZE];
extern fractional f1_to_f2_buffer_q_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F2_INPUT_SIZE];

extern fractional f2_to_f3_buffer_i_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F3_INPUT_SIZE];
extern fractional f2_to_f3_buffer_q_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F3_INPUT_SIZE];
extern fractional f2_to_f3_buffer_i_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F3_INPUT_SIZE];
extern fractional f2_to_f3_buffer_q_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F3_INPUT_SIZE];
extern fractional f3_to_f4_buffer_i_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F4_INPUT_SIZE];
extern fractional f3_to_f4_buffer_q_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F4_INPUT_SIZE];
extern fractional f3_to_f4_buffer_i_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F4_INPUT_SIZE];
extern fractional f3_to_f4_buffer_q_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F4_INPUT_SIZE];
extern fractional f4_to_f5_buffer_i_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F5_INPUT_SIZE];
extern fractional f4_to_f5_buffer_q_a[PLANALTA_N_ADC_CHANNELS][PLANALTA_F5_INPUT_SIZE];
extern fractional f4_to_f5_buffer_i_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F5_INPUT_SIZE];
extern fractional f4_to_f5_buffer_q_b[PLANALTA_N_ADC_CHANNELS][PLANALTA_F5_INPUT_SIZE];

extern bool planalta_lia_obuffer_selector;
extern fractional planalta_lia_obuffer_a_i[PLANALTA_N_ADC_CHANNELS];
extern fractional planalta_lia_obuffer_a_q[PLANALTA_N_ADC_CHANNELS];
extern fractional planalta_lia_obuffer_b_i[PLANALTA_N_ADC_CHANNELS];
extern fractional planalta_lia_obuffer_b_q[PLANALTA_N_ADC_CHANNELS];

extern uint8_t select_f0_to_f1, select_f1_to_f2, select_f2_to_f3, select_f3_to_f4, select_f4_to_f5;
extern uint8_t adc_buffer_selector;

extern volatile uint16_t planalta_fs_n_coeffs_written;
extern volatile bool planalta_fs_all_coeffs_written;

extern fractional fs_output_buffer_i;
extern fractional fs_output_buffer_q;

extern fractional* fs_fo0_buffer_write;
extern fractional* fs_fo1_buffer_write;
extern fractional* fs_fo2_buffer_write;

extern fractional* fs_fo0_buffer_read;
extern fractional* fs_fo1_buffer_read;
extern fractional* fs_fo2_buffer_read;
extern fractional* fs_fo3_buffer_read;

extern fractional* fs_fo1_buffer_i_write;
extern fractional* fs_fo1_buffer_q_write;
extern fractional* fs_fo2_buffer_i_write;
extern fractional* fs_fo2_buffer_q_write;
extern fractional* fs_fo3_buffer_i_write;
extern fractional* fs_fo3_buffer_q_write;
extern fractional* fs_fo4_buffer_i_write;
extern fractional* fs_fo4_buffer_q_write;

extern fractional* fs_fo2_buffer_i_read;
extern fractional* fs_fo2_buffer_q_read;
extern fractional* fs_fo3_buffer_i_read;
extern fractional* fs_fo3_buffer_q_read;
extern fractional* fs_fo4_buffer_i_read;
extern fractional* fs_fo4_buffer_q_read;
extern fractional* fs_fo5_buffer_i_read;
extern fractional* fs_fo5_buffer_q_read;

extern fractional conversion_buffer[];
extern fractional sample_buffer[];
extern uint16_t copy_buffer[PLANALTA_5KHZ_N_ADC_CHANNELS][ADC_5KHZ_BUFFER_LENGTH/PLANALTA_5KHZ_N_ADC_CHANNELS];

extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_0[FIR0_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_1_q[FIR1_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_1_i[FIR1_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_2_q[FIR2_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_2_i[FIR2_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_3_q[FIR3_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_3_i[FIR3_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_4_q[FIR4_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_4_i[FIR4_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_5_q[FIR6_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_5_i[FIR6_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_5_q[FIR6_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_5_i[FIR6_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_7_q[FIR7_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_7_i[FIR7_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_8_q[FIR8_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_8_i[FIR8_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_9_q[FIR9_N_COPIES];
extern fractional* const __attribute__((space(xmemory), eds)) delay_buffers_9_i[FIR9_N_COPIES];

extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_0[N_FIR_COEFFS0];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_1[N_FIR_COEFFS0];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_2[N_FIR_COEFFS0];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_3[N_FIR_COEFFS0];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_4[N_FIR_COEFFS0];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_5[N_FIR_COEFFS0];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_6[N_FIR_COEFFS0];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_0_7[N_FIR_COEFFS0];

extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_0[N_FIR_COEFFS1_Q];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_0[N_FIR_COEFFS1_I];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_1[N_FIR_COEFFS1_Q];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_1[N_FIR_COEFFS1_I];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_2[N_FIR_COEFFS1_Q];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_2[N_FIR_COEFFS1_I];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_3[N_FIR_COEFFS1_Q];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_3[N_FIR_COEFFS1_I];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_4[N_FIR_COEFFS1_Q];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_4[N_FIR_COEFFS1_I];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_5[N_FIR_COEFFS1_Q];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_5[N_FIR_COEFFS1_I];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_6[N_FIR_COEFFS1_Q];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_6[N_FIR_COEFFS1_I];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_q_7[N_FIR_COEFFS1_Q];
extern fractional __attribute__((space(xmemory), aligned(64), eds)) delay_buffers_1_i_7[N_FIR_COEFFS1_I];

extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_0[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_0[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_1[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_1[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_2[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_2[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_3[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_3[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_4[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_4[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_5[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_5[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_6[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_6[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_q_7[N_FIR_COEFFS2];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_2_i_7[N_FIR_COEFFS2];

extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_0[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_0[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_1[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_1[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_2[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_2[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_3[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_3[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_4[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_4[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_5[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_5[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_6[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_6[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_q_7[N_FIR_COEFFS3];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_3_i_7[N_FIR_COEFFS3];

extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_0[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_0[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_1[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_1[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_2[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_2[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_3[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_3[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_4[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_4[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_5[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_5[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_6[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_6[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_q_7[N_FIR_COEFFS4];
extern fractional __attribute__((space(xmemory), aligned(256), eds)) delay_buffers_4_i_7[N_FIR_COEFFS4];

extern fractional __attribute__((space(xmemory), aligned(512), eds)) delay_buffers_5_q_0[N_FIR_SHARED];
extern fractional __attribute__((space(xmemory), aligned(512), eds)) delay_buffers_5_i_0[N_FIR_SHARED];
extern fractional __attribute__((space(xmemory), aligned(512), eds)) delay_buffers_5_q_1[N_FIR_SHARED];
extern fractional __attribute__((space(xmemory), aligned(512), eds)) delay_buffers_5_i_1[N_FIR_SHARED];

void init_filters(void);
    
void adc_rx_callback_50khz(void);
void adc_rx_callback_25khz(void);
void adc_rx_callback_10khz(void);
void adc_rx_callback_5khz(void);

void run_filter2_5khz(void);
void run_filter3_5khz(void);
void run_filter4_5khz(void);
void run_filter5_5khz(void);
void run_filter6_5khz(void);

void run_filter2_10khz(void);
void run_filter3_10khz(void);
void run_filter4_10khz(void);
void run_filter5_10khz(void);
void run_filter6_10khz(void);

void run_filter2_25khz(void);
void run_filter3_25khz(void);
void run_filter4_25khz(void);
void run_filter5_25khz(void);
void run_filter6_25khz(void);

void run_filter2_50khz(void);
void run_filter3_50khz(void);
void run_filter4_50khz(void);
void run_filter5_50khz(void);
void run_filter6_50khz(void);

void init_filtering(void);

void planalta_clear_filter_buffers(void);

void planalta_filter_50khz(void);
void planalta_filter_25khz(void);
void planalta_filter_10khz(void);
void planalta_filter_5khz(void);

void planalta_fs_sample_50khz(void);
void planalta_fs_sample_20khz_10khz_5khz(planalta_fs_freq_t freq);
void planalta_fs_sample_2khz_1khz_500hz(planalta_fs_freq_t freq);
void planalta_fs_sample_200hz_100hz_50hz(planalta_fs_freq_t freq);
void planalta_fs_sample_20hz_10hz(planalta_fs_freq_t freq);

void adc_rx_callback_fs_50khz(void);
void adc_rx_callback_fs_20khz_10khz_5khz(void);
void adc_rx_callback_fs_2khz_1khz_500hz(void);
void adc_rx_callback_fs_200hz_100hz_50hz(void);
void adc_rx_callback_fs_20hz_10hz(void);
    
#ifdef	__cplusplus
}
#endif

#endif

