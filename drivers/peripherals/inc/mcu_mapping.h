/*
 * mcu_mapping.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __MCU_MAPPING_H__
#define __MCU_MAPPING_H__

#include "adc.h"
#include "dsm_flags.h"
#include "gpio.h"
#ifndef MPMCM
#include "i2c.h"
#endif
#include "lpuart.h"
#ifndef MPMCM
#include "spi.h"
#endif
#include "tim.h"
#include "usart.h"

/*** MCU MAPPING macros ***/

#if ((defined LVRM) && (defined HW1_0))
#define ADC_CHANNEL_VIN             ADC_CHANNEL_IN6
#define ADC_CHANNEL_VOUT            ADC_CHANNEL_IN4
#define ADC_CHANNEL_IOUT            ADC_CHANNEL_IN0
#endif
#if ((defined LVRM) && (defined HW2_0))
#define ADC_CHANNEL_VIN             ADC_CHANNEL_IN0
#define ADC_CHANNEL_VOUT            ADC_CHANNEL_IN6
#define ADC_CHANNEL_IOUT            ADC_CHANNEL_IN1
#endif
#ifdef BPSM
#define ADC_CHANNEL_VSRC            ADC_CHANNEL_IN6
#define ADC_CHANNEL_VSTR            ADC_CHANNEL_IN4
#define ADC_CHANNEL_VBKP            ADC_CHANNEL_IN0
#endif
#ifdef DDRM
#define ADC_CHANNEL_VIN             ADC_CHANNEL_IN7
#define ADC_CHANNEL_VOUT            ADC_CHANNEL_IN4
#define ADC_CHANNEL_IOUT            ADC_CHANNEL_IN0
#endif
#ifdef UHFM
#define ADC_CHANNEL_VRF             ADC_CHANNEL_IN7
#endif
#ifdef GPSM
#define ADC_CHANNEL_VGPS            ADC_CHANNEL_IN0
#define ADC_CHANNEL_VANT            ADC_CHANNEL_IN1
#endif
#ifdef SM
#define ADC_CHANNEL_AIN0            ADC_CHANNEL_IN5
#define ADC_CHANNEL_AIN1            ADC_CHANNEL_IN6
#define ADC_CHANNEL_AIN2            ADC_CHANNEL_IN7
#define ADC_CHANNEL_AIN3            ADC_CHANNEL_IN8
#endif
#ifdef RRM
#define ADC_CHANNEL_VIN             ADC_CHANNEL_IN7
#define ADC_CHANNEL_VOUT            ADC_CHANNEL_IN6
#define ADC_CHANNEL_IOUT            ADC_CHANNEL_IN4
#endif
#ifdef MPMCM
#define ADC_INSTANCE_ACX_SAMPLING   ADC_INSTANCE_ADC1
#define ADC_INSTANCE_ANALOG         ADC_INSTANCE_ADC1
#define ADC_CHANNEL_ACV_SAMPLING    ADC_CHANNEL_IN1
#define ADC_CHANNEL_ACI1_SAMPLING   ADC_CHANNEL_IN2
#define ADC_CHANNEL_ACI2_SAMPLING   ADC_CHANNEL_IN17
#define ADC_CHANNEL_ACI3_SAMPLING   ADC_CHANNEL_IN3
#define ADC_CHANNEL_ACI4_SAMPLING   ADC_CHANNEL_IN4
#endif
#ifdef BCM
#define ADC_CHANNEL_VSRC            ADC_CHANNEL_IN8
#define ADC_CHANNEL_VSTR            ADC_CHANNEL_IN7
#define ADC_CHANNEL_ISTR            ADC_CHANNEL_IN1
#define ADC_CHANNEL_VBKP            ADC_CHANNEL_IN6
#endif

#ifdef MPMCM
#define DMA_INSTANCE_ACV_SAMPLING   DMA_INSTANCE_DMA1
#define DMA_CHANNEL_ACV_SAMPLING    DMA_CHANNEL_1
#define DMA_INSTANCE_ACI_SAMPLING   DMA_INSTANCE_DMA1
#define DMA_CHANNEL_ACI_SAMPLING    DMA_CHANNEL_2
#define DMA_INSTANCE_ACV_FREQUENCY  DMA_INSTANCE_DMA1
#define DMA_CHANNEL_ACV_FREQUENCY   DMA_CHANNEL_3
#define DMA_INSTANCE_TIC            DMA_INSTANCE_DMA1
#define DMA_CHANNEL_TIC             DMA_CHANNEL_4
#endif

#define I2C_INSTANCE_SENSORS        I2C_INSTANCE_I2C1

#define SPI_INSTANCE_RADIO          SPI_INSTANCE_SPI1

#ifndef MPMCM
#define TIM_INSTANCE_LED_DIMMING    TIM_INSTANCE_TIM21
#endif

#if (defined LVRM) && (defined HW1_0)
#define TIM_INSTANCE_LED            TIM_INSTANCE_TIM2
#define TIM_CHANNEL_LED_RED         TIM_CHANNEL_3
#define TIM_CHANNEL_LED_GREEN       TIM_CHANNEL_2
#define TIM_CHANNEL_LED_BLUE        TIM_CHANNEL_1
#endif
#if (defined LVRM) && (defined HW2_0)
#define TIM_INSTANCE_LED            TIM_INSTANCE_TIM2
#define TIM_CHANNEL_LED_RED         TIM_CHANNEL_2
#define TIM_CHANNEL_LED_GREEN       TIM_CHANNEL_3
#define TIM_CHANNEL_LED_BLUE        TIM_CHANNEL_1
#endif
#ifdef DDRM
#define TIM_INSTANCE_LED            TIM_INSTANCE_TIM2
#define TIM_CHANNEL_LED_RED         TIM_CHANNEL_3
#define TIM_CHANNEL_LED_GREEN       TIM_CHANNEL_1
#define TIM_CHANNEL_LED_BLUE        TIM_CHANNEL_2
#endif
#ifdef GPSM
#define TIM_INSTANCE_LED_RG         TIM_INSTANCE_TIM22
#define TIM_INSTANCE_LED_B          TIM_INSTANCE_TIM2
#define TIM_CHANNEL_LED_RED         TIM_CHANNEL_1
#define TIM_CHANNEL_LED_GREEN       TIM_CHANNEL_2
#define TIM_CHANNEL_LED_BLUE        TIM_CHANNEL_3
#endif
#ifdef RRM
#define TIM_INSTANCE_LED            TIM_INSTANCE_TIM2
#define TIM_CHANNEL_LED_RED         TIM_CHANNEL_2
#define TIM_CHANNEL_LED_GREEN       TIM_CHANNEL_3
#define TIM_CHANNEL_LED_BLUE        TIM_CHANNEL_1
#endif
#ifdef MPMCM
#define TIM_INSTANCE_ADC_TRIGGER    TIM_INSTANCE_TIM6
#define TIM_INSTANCE_SIMULATION     TIM_INSTANCE_TIM15
#define TIM_INSTANCE_ACV_FREQUENCY  TIM_INSTANCE_TIM2
#define TIM_CHANNEL_ACV_FREQUENCY   TIM_CHANNEL_1
#define TIM_INSTANCE_LED            TIM_INSTANCE_TIM4
#define TIM_CHANNEL_LED_RED         TIM_CHANNEL_1
#define TIM_CHANNEL_LED_GREEN       TIM_CHANNEL_2
#define TIM_CHANNEL_LED_BLUE        TIM_CHANNEL_4
#endif
#ifdef BCM
#define TIM_INSTANCE_LED            TIM_INSTANCE_TIM2
#define TIM_CHANNEL_LED_RED         TIM_CHANNEL_1
#define TIM_CHANNEL_LED_GREEN       TIM_CHANNEL_2
#define TIM_CHANNEL_LED_BLUE        TIM_CHANNEL_4
#endif

#define TIM_INSTANCE_MCU_API        TIM_INSTANCE_TIM2

#define USART_INSTANCE_GPS          USART_INSTANCE_USART2

#define USART_INSTANCE_TIC          USART_INSTANCE_USART2

/*** MCU MAPPING structures ***/

/*!******************************************************************
 * \enum ADC_channel_index_t
 * \brief ADC channels index.
 *******************************************************************/
typedef enum {
#if ((defined DDRM) || (defined LVRM) || (defined RRM))
    ADC_CHANNEL_INDEX_VIN_MEASURE = 0,
    ADC_CHANNEL_INDEX_VOUT_MEASURE,
    ADC_CHANNEL_INDEX_IOUT_MEASURE,
#endif
#ifdef BPSM
    ADC_CHANNEL_INDEX_VSRC_MEASURE = 0,
    ADC_CHANNEL_INDEX_VSTR_MEASURE,
    ADC_CHANNEL_INDEX_VBKP_MEASURE,
#endif
#ifdef UHFM
    ADC_CHANNEL_INDEX_VRF = 0,
#endif
#ifdef GPSM
    ADC_CHANNEL_INDEX_VGPS_MEASURE = 0,
    ADC_CHANNEL_INDEX_VANT_MEASURE,
#endif
#ifdef SM
    ADC_CHANNEL_INDEX_AIN0 = 0,
    ADC_CHANNEL_INDEX_AIN1,
    ADC_CHANNEL_INDEX_AIN2,
    ADC_CHANNEL_INDEX_AIN3,
#endif
#ifdef MPMCM
    ADC_CHANNEL_INDEX_ACV_SAMPLING = 0,
    ADC_CHANNEL_INDEX_ACI1_SAMPLING,
    ADC_CHANNEL_INDEX_ACI2_SAMPLING,
    ADC_CHANNEL_INDEX_ACI3_SAMPLING,
    ADC_CHANNEL_INDEX_ACI4_SAMPLING,
#endif
#ifdef BCM
    ADC_CHANNEL_INDEX_VSRC_MEASURE = 0,
    ADC_CHANNEL_INDEX_VSTR_MEASURE,
    ADC_CHANNEL_INDEX_ISTR_MEASURE,
    ADC_CHANNEL_INDEX_VBKP_MEASURE,
#endif
    ADC_CHANNEL_INDEX_LAST
} ADC_channel_index_t;

#ifndef GPSM
/*!******************************************************************
 * \enum TIM_channel_channel_led_t
 * \brief TIM LED channels index.
 *******************************************************************/
typedef enum {
    TIM_CHANNEL_INDEX_LED_RED = 0,
    TIM_CHANNEL_INDEX_LED_GREEN,
    TIM_CHANNEL_INDEX_LED_BLUE,
    TIM_CHANNEL_INDEX_LED_LAST
} TIM_channel_index_led_t;
#endif

#ifdef GPSM
/*!******************************************************************
 * \enum TIM_channel_channel_led_t
 * \brief TIM LED channels index.
 *******************************************************************/
typedef enum {
    TIM_CHANNEL_INDEX_LED_RG_RED = 0,
    TIM_CHANNEL_INDEX_LED_RG_GREEN,
    TIM_CHANNEL_INDEX_LED_RG_LAST
} TIM_channel_index_led_rg_t;
#endif

#ifdef GPSM
/*!******************************************************************
 * \enum TIM_channel_channel_led_t
 * \brief TIM LED channels index.
 *******************************************************************/
typedef enum {
    TIM_CHANNEL_INDEX_LED_B_BLUE = 0,
    TIM_CHANNEL_INDEX_LED_B_LAST
} TIM_channel_index_led_b_t;
#endif

#ifdef MPMCM
/*!******************************************************************
 * \enum TIM_channel_index_acv_frequency_t
 * \brief TIM ACV frequency channels index.
 *******************************************************************/
typedef enum {
    TIM_CHANNEL_INDEX_ACV_FREQUENCY = 0,
    TIM_CHANNEL_INDEX_ACV_FREQUENCY_LAST
} TIM_channel_index_acv_frequency_t;
#endif

/*** MCU MAPPING global variables ***/

// RS485.
extern const LPUART_gpio_t LPUART_GPIO_RS485;
// Analog inputs.
#if ((defined BCM) || (defined BPSM) || ((defined LVRM) && (defined HW2_0)))
extern const GPIO_pin_t GPIO_MNTR_EN;
#endif
#if ((defined SM) || (defined MPMCM))
extern const GPIO_pin_t GPIO_ANALOG_POWER_ENABLE;
#endif
extern const ADC_gpio_t ADC_GPIO;
#ifdef DSM_RGB_LED
// RGB LED.
#ifdef GPSM
extern const TIM_gpio_t TIM_GPIO_LED_RG;
extern const TIM_gpio_t TIM_GPIO_LED_B;
#else
extern const TIM_gpio_t TIM_GPIO_LED;
#endif
#endif
#if (((defined LVRM) && (defined HW1_0)) || (defined BCM) || (defined BPSM) || (defined DDRM) || (defined RRM))
// Load control.
extern const GPIO_pin_t GPIO_OUT_EN;
#endif
#if (defined LVRM) && (defined HW2_0)
// Bistable relay control.
extern const GPIO_pin_t GPIO_DC_DC_POWER_ENABLE;
extern const GPIO_pin_t GPIO_COIL_POWER_ENABLE;
extern const GPIO_pin_t GPIO_OUT_SELECT;
extern const GPIO_pin_t GPIO_OUT_CONTROL;
extern const GPIO_pin_t GPIO_MUX_SEL;
extern const GPIO_pin_t GPIO_MUX_EN;
#endif
#ifdef BPSM
// Charge control and status.
extern const GPIO_pin_t GPIO_CHRG_ST;
extern const GPIO_pin_t GPIO_CHRG_EN;
#endif
#ifdef UHFM
// RF power enable.
extern const GPIO_pin_t GPIO_RF_POWER_ENABLE;
extern const GPIO_pin_t GPIO_RF_TX_ENABLE;
extern const GPIO_pin_t GPIO_RF_RX_ENABLE;
// TCXO power control.
extern const GPIO_pin_t GPIO_TCXO_POWER_ENABLE;
// Radio SPI.
extern const GPIO_pin_t GPIO_S2LP_CS;
extern const SPI_gpio_t SPI_GPIO_S2LP;
// S2LP GPIOs.
extern const GPIO_pin_t GPIO_S2LP_SDN;
extern const GPIO_pin_t GPIO_S2LP_GPIO0;
// Test points.
extern const GPIO_pin_t GPIO_TP1;
extern const GPIO_pin_t GPIO_TP2;
extern const GPIO_pin_t GPIO_TP3;
#endif
#ifdef GPSM
// GPS.
extern const GPIO_pin_t GPIO_GPS_POWER_ENABLE;
extern const GPIO_pin_t GPIO_GPS_RESET;
extern const GPIO_pin_t GPIO_GPS_VBCKP;
extern const GPIO_pin_t GPIO_GPS_TIMEPULSE;
extern const GPIO_pin_t GPIO_ANT_POWER_ENABLE;
// USART.
extern const USART_gpio_t USART_GPIO_GPS;
// Test point.
extern const GPIO_pin_t GPIO_TP1;
#endif
#ifdef SM
// Digital front-end.
extern const GPIO_pin_t GPIO_DIGITAL_POWER_ENABLE;
extern const GPIO_pin_t GPIO_DIO0;
extern const GPIO_pin_t GPIO_DIO1;
extern const GPIO_pin_t GPIO_DIO2;
extern const GPIO_pin_t GPIO_DIO3;
// Digital sensors.
extern const GPIO_pin_t GPIO_SENSORS_POWER_ENABLE;
extern const I2C_gpio_t I2C_GPIO_SENSORS;
#endif
#ifdef MPMCM
// TCXO.
extern const GPIO_pin_t GPIO_TCXO_POWER_ENABLE;
// Current sensors detectors.
extern const GPIO_pin_t GPIO_ACI1_DETECT;
extern const GPIO_pin_t GPIO_ACI2_DETECT;
extern const GPIO_pin_t GPIO_ACI3_DETECT;
extern const GPIO_pin_t GPIO_ACI4_DETECT;
// Zero cross detector.
extern const GPIO_pin_t GPIO_ZERO_CROSS_RAW;
extern const GPIO_pin_t GPIO_ZERO_CROSS_PULSE;
// Frequency measure.
extern const TIM_gpio_t TIM_GPIO_ACV_FREQUENCY;
// TIC interface.
extern const GPIO_pin_t GPIO_TIC_POWER_ENABLE;
extern const USART_gpio_t USART_GPIO_TIC;
// Test points.
extern const GPIO_pin_t GPIO_TP1;
extern const GPIO_pin_t GPIO_MCO;
#endif
#ifdef BCM
// Charge control and status.
extern const GPIO_pin_t GPIO_CHRG_ST0;
extern const GPIO_pin_t GPIO_CHRG_ST1;
extern const GPIO_pin_t GPIO_CHRG_EN;
#endif

#endif /* __MCU_MAPPING_H__ */
