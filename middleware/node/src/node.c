/*
 * node.c
 *
 *  Created on: 28 may. 2023
 *      Author: Ludo
 */

#include "node.h"

#include "bcm.h"
#include "bcm_registers.h"
#include "bpsm.h"
#include "bpsm_registers.h"
#include "common.h"
#include "common_registers.h"
#include "ddrm.h"
#include "ddrm_registers.h"
#include "dsm_flags.h"
#include "error.h"
#include "error_base.h"
#include "gpsm.h"
#include "gpsm_registers.h"
#include "led.h"
#include "lvrm.h"
#include "lvrm_registers.h"
#include "mpmcm.h"
#include "mpmcm_registers.h"
#include "nvm.h"
#include "nvm_address.h"
#include "power.h"
#include "pwr.h"
#include "rtc.h"
#include "rrm.h"
#include "rrm_registers.h"
#include "sm.h"
#include "sm_registers.h"
#include "swreg.h"
#include "uhfm.h"
#include "uhfm_registers.h"
#include "una.h"

/*** NODE local macros ***/

#ifdef DSM_IOUT_INDICATOR
#ifdef BCM
#define NODE_IOUT_CHANNEL_INPUT_VOLTAGE         ANALOG_CHANNEL_VSRC_MV
#define NODE_IOUT_CHANNEL                       ANALOG_CHANNEL_ISTR_UA
#else
#define NODE_IOUT_CHANNEL_INPUT_VOLTAGE         ANALOG_CHANNEL_VIN_MV
#define NODE_IOUT_CHANNEL                       ANALOG_CHANNEL_IOUT_UA
#endif
#define NODE_IOUT_MEASUREMENTS_PERIOD_SECONDS   60
#define NODE_IOUT_INDICATOR_PERIOD_SECONDS      10
#define NODE_IOUT_INDICATOR_RANGE               7
#define NODE_IOUT_INDICATOR_POWER_THRESHOLD_MV  6000
#define NODE_IOUT_INDICATOR_BLINK_DURATION_MS   2000
#endif

/*** NODE local structures ***/

#ifdef DSM_IOUT_INDICATOR
/*******************************************************************/
typedef struct {
    int32_t threshold_ua;
    LED_color_t led_color;
} NODE_iout_indicator_t;
#endif

/*******************************************************************/
typedef struct {
    volatile uint32_t registers[NODE_REGISTER_ADDRESS_LAST];
#ifdef DSM_IOUT_INDICATOR
    uint32_t iout_measurements_next_time_seconds;
    uint32_t iout_indicator_next_time_seconds;
    int32_t input_voltage_mv;
    int32_t iout_ua;
#endif
} NODE_context_t;

/*** NODE local global variables ***/

#ifdef DSM_IOUT_INDICATOR
static const NODE_iout_indicator_t LVRM_IOUT_INDICATOR[NODE_IOUT_INDICATOR_RANGE] = {
    {0, LED_COLOR_GREEN},
    {50000, LED_COLOR_YELLOW},
    {500000, LED_COLOR_RED},
    {1000000, LED_COLOR_MAGENTA},
    {2000000, LED_COLOR_BLUE},
    {3000000, LED_COLOR_CYAN},
    {4000000, LED_COLOR_WHITE}
};
#endif

static NODE_context_t node_ctx = {
    .registers = { [0 ... (NODE_REGISTER_ADDRESS_LAST - 1)] = 0x00000000 },
#ifdef DSM_IOUT_INDICATOR
    .iout_measurements_next_time_seconds = 0,
    .iout_indicator_next_time_seconds = 0,
    .input_voltage_mv = 0,
    .iout_ua = 0,
#endif
};

/*** NODE local functions ***/

#ifdef DSM_IOUT_INDICATOR
/*******************************************************************/
static NODE_status_t _NODE_iout_measurement(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    // Turn analog front-end on.
    POWER_enable(POWER_REQUESTER_ID_NODE, POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
    // Check input voltage.
    analog_status = ANALOG_convert_channel(NODE_IOUT_CHANNEL_INPUT_VOLTAGE, &(node_ctx.input_voltage_mv));
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    // Enable RGB LED only if power input supplies the board.
    if (node_ctx.input_voltage_mv > NODE_IOUT_INDICATOR_POWER_THRESHOLD_MV) {
        // Read output current.
        analog_status = ANALOG_convert_channel(NODE_IOUT_CHANNEL, &(node_ctx.iout_ua));
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    }
errors:
    POWER_disable(POWER_REQUESTER_ID_NODE, POWER_DOMAIN_ANALOG);
    return status;
}
#endif

#ifdef DSM_IOUT_INDICATOR
/*******************************************************************/
static NODE_status_t _NODE_iout_indicator(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    LED_status_t led_status = LED_SUCCESS;
    LED_color_t led_color;
    uint8_t idx = NODE_IOUT_INDICATOR_RANGE;
    // Enable RGB LED only if power input supplies the board.
    if (node_ctx.input_voltage_mv > NODE_IOUT_INDICATOR_POWER_THRESHOLD_MV) {
        // Compute LED color according to output current..
        do {
            idx--;
            // Get range and corresponding color.
            led_color = LVRM_IOUT_INDICATOR[idx].led_color;
            if (node_ctx.iout_ua >= LVRM_IOUT_INDICATOR[idx].threshold_ua) break;
        }
        while (idx > 0);
        // Blink LED.
        led_status = LED_start_single_blink(NODE_IOUT_INDICATOR_BLINK_DURATION_MS, led_color);
        LED_exit_error(NODE_ERROR_BASE_LED);
    }
errors:
    return status;
}
#endif

/*******************************************************************/
static NODE_status_t _NODE_update_register(uint8_t reg_addr) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Update common registers.
    status = COMMON_update_register(reg_addr);
    if (status != NODE_SUCCESS) goto errors;
    // Update specific registers.
#ifdef LVRM
    status = LVRM_update_register(reg_addr);
#endif
#ifdef BPSM
    status = BPSM_update_register(reg_addr);
#endif
#ifdef DDRM
    status = DDRM_update_register(reg_addr);
#endif
#ifdef UHFM
    status = UHFM_update_register(reg_addr);
#endif
#ifdef GPSM
    status = GPSM_update_register(reg_addr);
#endif
#ifdef SM
    status = SM_update_register(reg_addr);
#endif
#ifdef RRM
    status = RRM_update_register(reg_addr);
#endif
#ifdef MPMCM
    status = MPMCM_update_register(reg_addr);
#endif
#ifdef BCM
    status = BCM_update_register(reg_addr);
#endif
errors:
    return status;
}

/*******************************************************************/
static NODE_status_t _NODE_check_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Check common registers.
    status = COMMON_check_register(reg_addr, reg_mask);
    if (status != NODE_SUCCESS) goto errors;
    // Check specific registers.
#ifdef LVRM
    status = LVRM_check_register(reg_addr, reg_mask);
#endif
#ifdef BPSM
    status = BPSM_check_register(reg_addr, reg_mask);
#endif
#ifdef DDRM
    status = DDRM_check_register(reg_addr, reg_mask);
#endif
#ifdef UHFM
    status = UHFM_check_register(reg_addr, reg_mask);
#endif
#ifdef GPSM
    status = GPSM_check_register(reg_addr, reg_mask);
#endif
#ifdef SM
    status = SM_check_register(reg_addr, reg_mask);
#endif
#ifdef RRM
    status = RRM_check_register(reg_addr, reg_mask);
#endif
#ifdef MPMCM
    status = MPMCM_check_register(reg_addr, reg_mask);
#endif
#ifdef BCM
    status = BCM_check_register(reg_addr, reg_mask);
#endif
errors:
    return status;
}

/*** NODE functions ***/

/*******************************************************************/
NODE_status_t NODE_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    NVM_status_t nvm_status = NVM_SUCCESS;
#ifdef DSM_RGB_LED
    LED_status_t led_status = LED_SUCCESS;
#endif
#ifdef MPMCM
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
    MEASURE_status_t measure_status = MEASURE_SUCCESS;
#endif
#ifdef MPMCM_LINKY_TIC_ENABLE
    TIC_status_t tic_status = TIC_SUCCESS;
#endif
    uint32_t tmp_u32 = 0;
#endif
    UNA_node_address_t self_address = 0;
    uint8_t idx = 0;
    // Init context.
    for (idx = 0; idx < NODE_REGISTER_ADDRESS_LAST; idx++) {
        node_ctx.registers[idx] = NODE_REGISTER_ERROR_VALUE[idx];
    }
#ifdef DSM_IOUT_INDICATOR
    node_ctx.iout_measurements_next_time_seconds = 0;
    node_ctx.iout_indicator_next_time_seconds = 0;
    node_ctx.input_voltage_mv = 0;
    node_ctx.iout_ua = 0;
#endif
#ifdef DSM_NVM_FACTORY_RESET
#ifdef MPMCM
    nvm_status = NVM_write_word(NVM_ADDRESS_SELF_ADDRESS, (uint32_t) DSM_NODE_ADDRESS);
#else
    nvm_status = NVM_write_byte(NVM_ADDRESS_SELF_ADDRESS, DSM_NODE_ADDRESS);
#endif
    NVM_exit_error(NODE_ERROR_BASE_NVM);
#endif
    // Read self address in NVM.
#ifdef MPMCM
    nvm_status = NVM_read_word(NVM_ADDRESS_SELF_ADDRESS, &tmp_u32);
    self_address = (UNA_node_address_t) tmp_u32;
#else
    nvm_status = NVM_read_byte(NVM_ADDRESS_SELF_ADDRESS, &self_address);
#endif
    NVM_exit_error(NODE_ERROR_BASE_NVM);
#ifdef DSM_LOAD_CONTROL
    LOAD_init();
#endif
#ifdef DSM_RGB_LED
    led_status = LED_init();
    LED_stack_error(ERROR_BASE_LED);
#endif
#ifdef MPMCM
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
    measure_status = MEASURE_init();
    MEASURE_stack_error(ERROR_BASE_MEASURE);
#endif
#ifdef MPMCM_LINKY_TIC_ENABLE
    tic_status = TIC_init();
    TIC_stack_error(ERROR_BASE_TIC);
#endif
#endif
    // Init common registers.
    status = COMMON_init_registers(self_address);
    if (status != NODE_SUCCESS) goto errors;
    // Init specific registers.
#ifdef LVRM
    status = LVRM_init_registers();
#endif
#ifdef BPSM
    status = BPSM_init_registers();
#endif
#ifdef DDRM
    status = DDRM_init_registers();
#endif
#ifdef UHFM
    status = UHFM_init_registers();
#endif
#ifdef GPSM
    status = GPSM_init_registers();
#endif
#ifdef SM
    status = SM_init_registers();
#endif
#ifdef RRM
    status = RRM_init_registers();
#endif
#ifdef MPMCM
    status = MPMCM_init_registers();
#endif
#ifdef BCM
    status = BCM_init_registers();
#endif
    if (status != NODE_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
NODE_status_t NODE_de_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#ifdef MPMCM
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
    MEASURE_status_t measure_status = MEASURE_SUCCESS;
#endif
#ifdef MPMCM_LINKY_TIC_ENABLE
    TIC_status_t tic_status = TIC_SUCCESS;
#endif
#endif
#ifdef MPMCM
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
    measure_status = MEASURE_de_init();
    MEASURE_stack_error(ERROR_BASE_MEASURE);
#endif
#ifdef MPMCM_LINKY_TIC_ENABLE
    tic_status = TIC_de_init();
    TIC_stack_error(ERROR_BASE_TIC);
#endif
#endif
#ifdef DSM_RGB_LED
    LED_status_t led_status = LED_SUCCESS;
    led_status = LED_de_init();
    LED_stack_error(ERROR_BASE_NODE + NODE_ERROR_BASE_LED);
#endif
    return status;
}

/*******************************************************************/
NODE_status_t NODE_process(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#if (((defined LVRM) && (defined LVRM_MODE_BMS)) || (defined BPSM) || (defined BCM))
    NODE_status_t node_status = NODE_SUCCESS;
#endif
#if ((defined MPMCM) && (defined MPMCM_LINKY_TIC_ENABLE))
    TIC_status_t tic_status = TIC_SUCCESS;
#endif
    // Read RTRG bit.
    if (SWREG_read_field(node_ctx.registers[COMMON_REGISTER_ADDRESS_CONTROL_0], COMMON_REGISTER_CONTROL_0_MASK_RTRG) != 0) {
        // Reset MCU.
        PWR_software_reset();
    }
#if ((defined LVRM) && (defined LVRM_MODE_BMS))
    status = LVRM_bms_process();
    NODE_stack_error(ERROR_BASE_NODE);
#endif
#ifdef BPSM
    node_status = BPSM_low_voltage_detector_process();
    NODE_stack_error(ERROR_BASE_NODE);
#ifndef BPSM_CHEN_FORCED_HARDWARE
    node_status = BPSM_charge_process();
    NODE_stack_error(ERROR_BASE_NODE);
#endif
#endif
#if ((defined MPMCM) && (defined MPMCM_LINKY_TIC_ENABLE))
    // Process TIC interface.
    tic_status = TIC_process();
    TIC_stack_error(ERROR_BASE_TIC);
#endif
#ifdef BCM
    node_status = BCM_low_voltage_detector_process();
    NODE_stack_error(ERROR_BASE_NODE);
#ifndef BCM_CHEN_FORCED_HARDWARE
    node_status = BCM_charge_process();
    NODE_stack_error(ERROR_BASE_NODE);
#endif
#endif
#ifdef DSM_IOUT_INDICATOR
    // Check measurements period.
    if (RTC_get_uptime_seconds() >= node_ctx.iout_measurements_next_time_seconds) {
        // Update next time.
        node_ctx.iout_measurements_next_time_seconds = RTC_get_uptime_seconds() + NODE_IOUT_MEASUREMENTS_PERIOD_SECONDS;
        // Perform measurements.
        status = _NODE_iout_measurement();
        if (status != NODE_SUCCESS) goto errors;
    }
    // Check LED period.
    if (RTC_get_uptime_seconds() >= node_ctx.iout_indicator_next_time_seconds) {
        // Update next time.
        node_ctx.iout_indicator_next_time_seconds = RTC_get_uptime_seconds() + NODE_IOUT_INDICATOR_PERIOD_SECONDS;
        // Perform LED task.
        status = _NODE_iout_indicator();
        if (status != NODE_SUCCESS) goto errors;
    }
errors:
#endif
    return status;
}

#ifdef MPMCM
/*******************************************************************/
NODE_status_t NODE_tick_second(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
    MEASURE_status_t measure_status = MEASURE_SUCCESS;
#endif
#ifdef MPMCM_LINKY_TIC_ENABLE
    // Call TIC interface tick.
    TIC_tick_second();
#endif
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
    // Call measure tick.
    measure_status = MEASURE_tick_second();
    MEASURE_exit_error(ERROR_BASE_MEASURE);
#endif
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
errors:
#endif
    return status;
}
#endif

/*******************************************************************/
NODE_state_t NODE_get_state(void) {
    // Local variables.
    NODE_state_t state = NODE_STATE_IDLE;
#ifdef MPMCM
    state = ((TIC_get_state() == TIC_STATE_OFF) && (MEASURE_get_state() == MEASURE_STATE_OFF) && (LED_get_state() == LED_STATE_OFF)) ? NODE_STATE_IDLE : NODE_STATE_RUNNING;
#else
#ifdef DSM_IOUT_INDICATOR
    state = (LED_get_state() == LED_STATE_OFF) ? NODE_STATE_IDLE : NODE_STATE_RUNNING;
#endif
#endif
    return state;
}

/*******************************************************************/
NODE_status_t NODE_write_register(NODE_request_source_t request_source, uint8_t reg_addr, uint32_t reg_value, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Check address.
    if (reg_addr >= NODE_REGISTER_ADDRESS_LAST) {
        status = NODE_ERROR_REGISTER_ADDRESS;
        goto errors;
    }
    // Check access.
    if ((request_source == NODE_REQUEST_SOURCE_EXTERNAL) && (NODE_REGISTER_ACCESS[reg_addr] == UNA_REGISTER_ACCESS_READ_ONLY)) {
        status = NODE_ERROR_REGISTER_READ_ONLY;
        goto errors;
    }
    // Write register.
    SWREG_modify_register((uint32_t*) &(node_ctx.registers[reg_addr]), reg_value, reg_mask);
    // Check actions.
    if (request_source == NODE_REQUEST_SOURCE_EXTERNAL) {
        // Check control bits.
        status = _NODE_check_register(reg_addr, reg_mask);
        if (status != NODE_SUCCESS) goto errors;
    }
errors:
    return status;
}

/*******************************************************************/
NODE_status_t NODE_write_byte_array(NODE_request_source_t request_source, uint8_t reg_addr_base, uint8_t* data, uint8_t data_size_byte) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint8_t idx = 0;
    uint8_t shift = 0;
    uint8_t reg_addr = 0;
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // Check parameters.
    if (data == NULL) {
        status = NODE_ERROR_NULL_PARAMETER;
        goto errors;
    }
    // Byte loop.
    for (idx = 0; idx < data_size_byte; idx++) {
        // Compute address, mask and value.
        reg_addr = (reg_addr_base + (idx >> 2));
        shift = ((idx % 4) << 3);
        reg_mask = (0xFF << shift);
        reg_value = (data[idx] << shift);
        // Write register.
        status = NODE_write_register(request_source, reg_addr, reg_value, reg_mask);
        if (status != NODE_SUCCESS) goto errors;
    }
errors:
    return status;
}

/*******************************************************************/
NODE_status_t NODE_write_nvm(uint8_t reg_addr, uint32_t reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    NVM_status_t nvm_status = NVM_SUCCESS;
#ifndef MPMCM
    uint8_t nvm_byte = 0;
    uint8_t idx = 0;
#endif
#ifdef MPMCM
    nvm_status = NVM_write_word((NVM_ADDRESS_REGISTERS + reg_addr), reg_value);
    NVM_exit_error(NODE_ERROR_BASE_NVM);
#else
    // Byte loop.
    for (idx = 0; idx < 4; idx++) {
        // Compute byte.
        nvm_byte = (uint8_t) (((reg_value) >> (idx << 3)) & 0x000000FF);
        // Write NVM.
        nvm_status = NVM_write_byte((NVM_ADDRESS_REGISTERS + (reg_addr << 2) + idx), nvm_byte);
        NVM_exit_error(NODE_ERROR_BASE_NVM);
    }
#endif
errors:
    return status;
}

/*******************************************************************/
NODE_status_t NODE_read_register(NODE_request_source_t request_source, uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Check parameters.
    if (reg_addr >= NODE_REGISTER_ADDRESS_LAST) {
        status = NODE_ERROR_REGISTER_ADDRESS;
        goto errors;
    }
    if (reg_value == NULL) {
        status = NODE_ERROR_NULL_PARAMETER;
        goto errors;
    }
    // Check update type.
    if (request_source == NODE_REQUEST_SOURCE_EXTERNAL) {
        // Update register.
        status = _NODE_update_register(reg_addr);
        if (status != NODE_SUCCESS) goto errors;
    }
    // Read register.
    (*reg_value) = node_ctx.registers[reg_addr];
errors:
    return status;
}

/*******************************************************************/
NODE_status_t NODE_read_byte_array(NODE_request_source_t request_source, uint8_t reg_addr_base, uint8_t* data, uint8_t data_size_byte) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint8_t idx = 0;
    uint8_t reg_addr = 0;
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // Check parameters.
    if (data == NULL) {
        status = NODE_ERROR_NULL_PARAMETER;
        goto errors;
    }
    // Byte loop.
    for (idx = 0; idx < data_size_byte; idx++) {
        // Compute address and mask.
        reg_addr = (reg_addr_base + (idx >> 2));
        reg_mask = (0xFF << ((idx % 4) << 3));
        // Read byte.
        status = NODE_read_register(request_source, reg_addr, &reg_value);
        // Fill data.
        data[idx] = (uint8_t) SWREG_read_field(reg_value, reg_mask);
    }
errors:
    return status;
}

/*******************************************************************/
NODE_status_t NODE_read_nvm(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    NVM_status_t nvm_status = NVM_SUCCESS;
#ifndef MPMCM
    uint8_t nvm_byte = 0;
    uint8_t idx = 0;
#endif
    // Check parameter.
    if (reg_value == NULL) {
        status = NODE_ERROR_NULL_PARAMETER;
        goto errors;
    }
    // Reset output value.
    (*reg_value) = 0;
#ifdef MPMCM
    nvm_status = NVM_read_word((NVM_ADDRESS_REGISTERS + reg_addr), reg_value);
    NVM_exit_error(NODE_ERROR_BASE_NVM);
#else
    // Byte loop.
    for (idx = 0; idx < 4; idx++) {
        // Read NVM.
        nvm_status = NVM_read_byte((NVM_ADDRESS_REGISTERS + (reg_addr << 2) + idx), &nvm_byte);
        NVM_exit_error(NODE_ERROR_BASE_NVM);
        // Update output value.
        (*reg_value) |= ((uint32_t) nvm_byte) << (idx << 3);
    }
#endif
errors:
    return status;
}
