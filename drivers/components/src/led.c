/*
 * led.c
 *
 *  Created on: 22 aug. 2020
 *      Author: Ludo
 */

#include "led.h"

#include "dsm_flags.h"
#include "error.h"
#include "error_base.h"
#include "gpio.h"
#include "maths.h"
#include "mcu_mapping.h"
#include "nvic_priority.h"
#include "tim.h"
#include "types.h"

#ifdef DSM_RGB_LED

/*** LED local macros ***/

#define LED_PWM_FREQUENCY_HZ        10000

#define LED_DIMMING_LUT_SIZE        100

/*** LED local structures ***/

#ifndef MPMCM
/*******************************************************************/
typedef struct {
    LED_color_t color;
    volatile uint8_t dimming_lut_direction;
    volatile uint32_t dimming_lut_index;
    volatile uint8_t single_blink_done;
} LED_context_t;
#endif

#ifdef GPSM
/*******************************************************************/
typedef enum {
    LED_COLOR_INDEX_RED = 0,
    LED_COLOR_INDEX_GREEN,
    LED_COLOR_INDEX_BLUE,
    LED_COLOR_INDEX_LAST
} LED_color_index_t;
#endif

/*** LED local global variables ***/

#ifndef MPMCM
static const uint8_t LED_DIMMING_LUT[LED_DIMMING_LUT_SIZE] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 3, 3, 3,
    3, 3, 3, 4, 4, 4, 4, 5, 5, 5,
    5, 6, 6, 6, 7, 7, 8, 8, 8, 9,
    9, 10, 10, 11, 11, 12, 13, 13, 14, 15,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 28, 29, 30, 32, 34, 35, 37, 39,
    41, 43, 45, 47, 49, 52, 54, 57, 59, 62,
    65, 69, 72, 75, 79, 83, 87, 91, 95, 100
};
#endif

#ifndef MPMCM
static LED_context_t led_ctx = {
    .color = LED_COLOR_OFF,
    .dimming_lut_direction = 0,
    .dimming_lut_index = 0,
    .single_blink_done = 1
};
#endif

/*** LED local functions ***/

#ifndef MPMCM
/*******************************************************************/
static LED_status_t _LED_turn_off(void) {
    // Local variables.
    LED_status_t status = LED_SUCCESS;
    TIM_status_t tim_status = TIM_SUCCESS;
#ifndef GPSM
    uint8_t idx = 0;
#endif
    // Stop timers.
#ifdef GPSM
    tim_status = TIM_PWM_set_waveform(TIM_INSTANCE_LED_RG, (TIM_GPIO_LED_RG.list[TIM_CHANNEL_INDEX_LED_RG_RED])->channel, (LED_PWM_FREQUENCY_HZ * 1000), 0);
    TIM_exit_error(LED_ERROR_BASE_TIM_PWM);
    tim_status = TIM_PWM_set_waveform(TIM_INSTANCE_LED_RG, (TIM_GPIO_LED_RG.list[TIM_CHANNEL_INDEX_LED_RG_GREEN])->channel, (LED_PWM_FREQUENCY_HZ * 1000), 0);
    TIM_exit_error(LED_ERROR_BASE_TIM_PWM);
    tim_status = TIM_PWM_set_waveform(TIM_INSTANCE_LED_B, (TIM_GPIO_LED_B.list[TIM_CHANNEL_INDEX_LED_B_BLUE])->channel, (LED_PWM_FREQUENCY_HZ * 1000), 0);
    TIM_exit_error(LED_ERROR_BASE_TIM_PWM);
#else
    for (idx = 0; idx < TIM_CHANNEL_INDEX_LED_LAST; idx++) {
        tim_status = TIM_PWM_set_waveform(TIM_INSTANCE_LED, (TIM_GPIO_LED.list[idx])->channel, (LED_PWM_FREQUENCY_HZ * 1000), 0);
        TIM_exit_error(LED_ERROR_BASE_TIM_PWM);
    }
#endif
errors:
    return status;
}
#endif

#ifndef MPMCM
/*******************************************************************/
static void _LED_dimming_timer_irq_callback(void) {
    // Local variables.
    LED_status_t led_status = LED_SUCCESS;
    TIM_status_t tim_status = TIM_SUCCESS;
    uint8_t duty_cycle_percent = 0;
#ifndef GPSM
    uint8_t idx = 0;
#endif
    // Update duty cycles.
#ifdef GPSM
    // Apply color mask.
    duty_cycle_percent = ((led_ctx.color & (0b1 << LED_COLOR_INDEX_RED)) != 0) ? LED_DIMMING_LUT[led_ctx.dimming_lut_index] : 0;
    // Set duty cycle.
    tim_status = TIM_PWM_set_waveform(TIM_INSTANCE_LED_RG, (TIM_GPIO_LED_RG.list[TIM_CHANNEL_INDEX_LED_RG_RED])->channel, (LED_PWM_FREQUENCY_HZ * 1000), duty_cycle_percent);
    TIM_stack_error(ERROR_BASE_LED + LED_ERROR_BASE_TIM_PWM);
    // Apply color mask.
    duty_cycle_percent = ((led_ctx.color & (0b1 << LED_COLOR_INDEX_GREEN)) != 0) ? LED_DIMMING_LUT[led_ctx.dimming_lut_index] : 0;
    // Set duty cycle.
    tim_status = TIM_PWM_set_waveform(TIM_INSTANCE_LED_RG, (TIM_GPIO_LED_RG.list[TIM_CHANNEL_INDEX_LED_RG_GREEN])->channel, (LED_PWM_FREQUENCY_HZ * 1000), duty_cycle_percent);
    TIM_stack_error(ERROR_BASE_LED + LED_ERROR_BASE_TIM_PWM);
    // Apply color mask.
    duty_cycle_percent = ((led_ctx.color & (0b1 << LED_COLOR_INDEX_BLUE)) != 0) ? LED_DIMMING_LUT[led_ctx.dimming_lut_index] : 0;
    // Set duty cycle.
    tim_status = TIM_PWM_set_waveform(TIM_INSTANCE_LED_B, (TIM_GPIO_LED_B.list[TIM_CHANNEL_INDEX_LED_B_BLUE])->channel, (LED_PWM_FREQUENCY_HZ * 1000), duty_cycle_percent);
    TIM_stack_error(ERROR_BASE_LED + LED_ERROR_BASE_TIM_PWM);
#else
    for (idx = 0; idx < TIM_CHANNEL_INDEX_LED_LAST; idx++) {
        // Apply color mask.
        duty_cycle_percent = ((led_ctx.color & (0b1 << idx)) != 0) ? LED_DIMMING_LUT[led_ctx.dimming_lut_index] : 0;
        // Set duty cycle.
        tim_status = TIM_PWM_set_waveform(TIM_INSTANCE_LED, (TIM_GPIO_LED.list[idx])->channel, (LED_PWM_FREQUENCY_HZ * 1000), duty_cycle_percent);
        TIM_stack_error(ERROR_BASE_LED + LED_ERROR_BASE_TIM_PWM);
    }
#endif
    // Manage index and direction.
    if (led_ctx.dimming_lut_direction == 0) {
        // Increment index.
        led_ctx.dimming_lut_index++;
        // Invert direction at end of table.
        if (led_ctx.dimming_lut_index >= (LED_DIMMING_LUT_SIZE - 1)) {
            led_ctx.dimming_lut_direction = 1;
        }
    }
    else {
        // Decrement index.
        led_ctx.dimming_lut_index--;
        // Invert direction at the beginning of table.
        if (led_ctx.dimming_lut_index == 0) {
            // Stop timers.
            led_status = _LED_turn_off();
            LED_stack_error(ERROR_BASE_LED);
            tim_status = TIM_STD_stop(TIM_INSTANCE_LED_DIMMING);
            TIM_stack_error(ERROR_BASE_LED + LED_ERROR_BASE_TIM_DIMMING);
            // Single blink done.
            led_ctx.dimming_lut_direction = 0;
            led_ctx.single_blink_done = 1;
        }
    }
}
#endif

/*** LED functions ***/

/*******************************************************************/
LED_status_t LED_init(void) {
    // Local variables.
    LED_status_t status = LED_SUCCESS;
    TIM_status_t tim_status = TIM_SUCCESS;
#ifndef MPMCM
    // Init context.
    led_ctx.color = LED_COLOR_OFF;
    led_ctx.dimming_lut_direction = 0;
    led_ctx.dimming_lut_index = 0;
    led_ctx.single_blink_done = 1;
#endif
    // Init timers.
#ifdef MPMCM
    tim_status = TIM_OPM_init(TIM_INSTANCE_LED, (TIM_gpio_t*) &TIM_GPIO_LED);
    TIM_exit_error(LED_ERROR_BASE_TIM_OPM);
#else
#ifdef GPSM
    tim_status = TIM_PWM_init(TIM_INSTANCE_LED_RG, (TIM_gpio_t*) &TIM_GPIO_LED_RG);
    TIM_exit_error(LED_ERROR_BASE_TIM_PWM);
    tim_status = TIM_PWM_init(TIM_INSTANCE_LED_B, (TIM_gpio_t*) &TIM_GPIO_LED_B);
    TIM_exit_error(LED_ERROR_BASE_TIM_PWM);
#else
    tim_status = TIM_PWM_init(TIM_INSTANCE_LED, (TIM_gpio_t*) &TIM_GPIO_LED);
    TIM_exit_error(LED_ERROR_BASE_TIM_PWM);
#endif
    tim_status = TIM_STD_init(TIM_INSTANCE_LED_DIMMING, NVIC_PRIORITY_LED);
    TIM_exit_error(LED_ERROR_BASE_TIM_DIMMING);
    // Turn LED off.
    status = _LED_turn_off();
    if (status != LED_SUCCESS) goto errors;
#endif
errors:
    return status;
}

/*******************************************************************/
LED_status_t LED_de_init(void) {
    // Local variables.
    LED_status_t status = LED_SUCCESS;
    TIM_status_t tim_status = TIM_SUCCESS;
#ifndef MPMCM
    LED_status_t led_status = LED_SUCCESS;
#endif
#ifndef MPMCM
    // Turn LED off.
    led_status = LED_stop_blink();
    LED_stack_error(ERROR_BASE_LED);
#endif
    // Release timers.
#ifdef MPMCM
    tim_status = TIM_OPM_de_init(TIM_INSTANCE_LED, (TIM_gpio_t*) &TIM_GPIO_LED);
    TIM_stack_error(ERROR_BASE_LED + LED_ERROR_BASE_TIM_OPM);
#else
#ifdef GPSM
    tim_status = TIM_PWM_de_init(TIM_INSTANCE_LED_RG, (TIM_gpio_t*) &TIM_GPIO_LED_RG);
    TIM_stack_error(ERROR_BASE_LED + LED_ERROR_BASE_TIM_PWM);
    tim_status = TIM_PWM_de_init(TIM_INSTANCE_LED_B, (TIM_gpio_t*) &TIM_GPIO_LED_B);
    TIM_stack_error(ERROR_BASE_LED + LED_ERROR_BASE_TIM_PWM);
#else
    tim_status = TIM_PWM_de_init(TIM_INSTANCE_LED, (TIM_gpio_t*) &TIM_GPIO_LED);
    TIM_stack_error(ERROR_BASE_LED + LED_ERROR_BASE_TIM_PWM);
#endif
    tim_status = TIM_STD_de_init(TIM_INSTANCE_LED_DIMMING);
    TIM_stack_error(ERROR_BASE_LED + LED_ERROR_BASE_TIM_DIMMING);
#endif
    return status;
}

#ifndef MPMCM
/*******************************************************************/
LED_status_t LED_start_single_blink(uint32_t blink_duration_ms, LED_color_t color) {
    // Local variables.
    LED_status_t status = LED_SUCCESS;
    TIM_status_t tim_status = TIM_SUCCESS;
    // Check parameters.
    if (blink_duration_ms == 0) {
        status = LED_ERROR_NULL_DURATION;
        goto errors;
    }
    if (color >= LED_COLOR_LAST) {
        status = LED_ERROR_COLOR;
        goto errors;
    }
    // Update context.
    led_ctx.color = color;
    led_ctx.dimming_lut_direction = 0;
    led_ctx.dimming_lut_index = 0;
    led_ctx.single_blink_done = 0;
    // Start blink.
    tim_status = TIM_STD_start(TIM_INSTANCE_LED_DIMMING, ((blink_duration_ms) / (LED_DIMMING_LUT_SIZE << 1)), TIM_UNIT_MS, &_LED_dimming_timer_irq_callback);
    TIM_exit_error(LED_ERROR_BASE_TIM_DIMMING);
errors:
    return status;
}
#endif

#ifndef MPMCM
/*******************************************************************/
LED_status_t LED_stop_blink(void) {
    // Local variables.
    LED_status_t status = LED_SUCCESS;
    TIM_status_t tim_status = TIM_SUCCESS;
    // Turn LED off.
    status = _LED_turn_off();
    if (status != LED_SUCCESS) goto errors;
    // Stop dimming timer.
    tim_status = TIM_STD_stop(TIM_INSTANCE_LED_DIMMING);
    TIM_exit_error(LED_ERROR_BASE_TIM_DIMMING);
errors:
    return status;
}
#endif

#ifdef MPMCM
/*******************************************************************/
LED_status_t LED_single_pulse(uint32_t pulse_duration_ms, LED_color_t color, uint8_t pulse_completion_event) {
    // Local variables.
    LED_status_t status = LED_SUCCESS;
    TIM_status_t tim_status = TIM_SUCCESS;
    uint8_t channels_mask = 0;
    uint8_t idx = 0;
    // Check parameters.
    if (pulse_duration_ms == 0) {
        status = LED_ERROR_NULL_DURATION;
        goto errors;
    }
    if (color >= LED_COLOR_LAST) {
        status = LED_ERROR_COLOR;
        goto errors;
    }
    // Make pulse on required channels.
    for (idx = 0; idx < TIM_CHANNEL_INDEX_LED_LAST; idx++) {
        // Apply color mask.
        channels_mask |= (((color >> idx) & 0x01) << ((TIM_GPIO_LED.list[idx])->channel));
    }
    // Make pulse on channel.
    tim_status = TIM_OPM_make_pulse(TIM_INSTANCE_LED, channels_mask, 0, (pulse_duration_ms * MATH_POWER_10[6]), pulse_completion_event);
    TIM_exit_error(LED_ERROR_BASE_TIM_OPM);
errors:
    return status;
}
#endif

/*******************************************************************/
LED_state_t LED_get_state(void) {
    // Local variables.
    LED_state_t state = LED_STATE_OFF;
#ifdef MPMCM
    TIM_status_t tim_status = TIM_SUCCESS;
#endif
    uint8_t pulse_is_done = 0;
    // Get status.
#ifdef MPMCM
    tim_status = TIM_OPM_get_pulse_status(TIM_INSTANCE_LED, &pulse_is_done);
    TIM_stack_error(ERROR_BASE_LED + LED_ERROR_BASE_TIM_OPM);
#else
    pulse_is_done = led_ctx.single_blink_done;
#endif
    // Update state.
    state = (pulse_is_done == 0) ? LED_STATE_ACTIVE : LED_STATE_OFF;
    return state;
}

#endif /* DSM_RGB_LED */
