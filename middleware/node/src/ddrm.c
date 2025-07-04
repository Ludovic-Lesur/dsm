/*
 * ddrm.c
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#include "ddrm.h"

#ifdef DDRM

#include "adc.h"
#include "dsm_flags.h"
#include "error.h"
#include "load.h"
#include "ddrm_registers.h"
#include "node.h"
#include "swreg.h"
#include "types.h"
#include "una.h"

/*** DDRM local macros ***/

// Note: IOUT measurement uses LT6106 and OPA187 chips whose minimum operating voltage is 4.5V.
#define DDRM_IOUT_MEASUREMENT_VSH_MIN_MV    4500

/*** DDRM local structures ***/

/*******************************************************************/
typedef struct {
    UNA_bit_representation_t ddenst;
} DDRM_context_t;

/*** DDRM local global variables ***/

static DDRM_context_t ddrm_ctx = {
    .ddenst = UNA_BIT_ERROR
};

/*** DDRM local functions ***/

/*******************************************************************/
static void _DDRM_load_flags(void) {
    // Local variables.
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // DC-DC control mode.
#ifdef DDRM_DDEN_FORCED_HARDWARE
    SWREG_write_field(&reg_value, &reg_mask, 0b1, DDRM_REGISTER_FLAGS_1_MASK_DDFH);
#else
    SWREG_write_field(&reg_value, &reg_mask, 0b0, DDRM_REGISTER_FLAGS_1_MASK_DDFH);
#endif
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REGISTER_ADDRESS_FLAGS_1, reg_value, reg_mask);
}

/*******************************************************************/
static void _DDRM_load_configuration(void) {
    // Local variables.
    uint8_t reg_addr = 0;
    uint32_t reg_value = 0;
    // Load configuration registers from NVM.
    for (reg_addr = DDRM_REGISTER_ADDRESS_CONFIGURATION_0; reg_addr < DDRM_REGISTER_ADDRESS_STATUS_1; reg_addr++) {
        // Read NVM.
        NODE_read_nvm(reg_addr, &reg_value);
        // Write register.
        NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, UNA_REGISTER_MASK_ALL);
    }
}

/*******************************************************************/
static void _DDRM_reset_analog_data(void) {
    // Reset analog registers.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REGISTER_ADDRESS_ANALOG_DATA_1, DDRM_REGISTER_ERROR_VALUE[DDRM_REGISTER_ADDRESS_ANALOG_DATA_1], UNA_REGISTER_MASK_ALL);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REGISTER_ADDRESS_ANALOG_DATA_2, DDRM_REGISTER_ERROR_VALUE[DDRM_REGISTER_ADDRESS_ANALOG_DATA_2], UNA_REGISTER_MASK_ALL);
}

/*** DDRM functions ***/

/*******************************************************************/
NODE_status_t DDRM_init_registers(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#ifdef DSM_NVM_FACTORY_RESET
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
#endif
    // Init context.
    ddrm_ctx.ddenst = UNA_BIT_ERROR;
#ifdef DSM_NVM_FACTORY_RESET
    // IOUT offset.
    SWREG_write_field(&reg_value, &reg_mask, 0, DDRM_REGISTER_CONFIGURATION_0_MASK_IOUT_OFFSET);
    NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, DDRM_REGISTER_ADDRESS_CONFIGURATION_0, reg_value, reg_mask);
#endif
    // Load default values.
    _DDRM_load_flags();
    _DDRM_load_configuration();
    _DDRM_reset_analog_data();
    // Read init state.
    status = DDRM_update_register(DDRM_REGISTER_ADDRESS_STATUS_1);
    if (status != NODE_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
NODE_status_t DDRM_update_register(uint8_t reg_addr) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // Check address.
    switch (reg_addr) {
    case DDRM_REGISTER_ADDRESS_STATUS_1:
        // DC-DC state.
#ifdef DDRM_DDEN_FORCED_HARDWARE
        ddrm_ctx.ddenst = UNA_BIT_FORCED_HARDWARE;
#else
        switch (LOAD_get_output_state()) {
        case 0:
            ddrm_ctx.ddenst = UNA_BIT_0;
            break;
        case 1:
            ddrm_ctx.ddenst = UNA_BIT_1;
            break;
        default:
            ddrm_ctx.ddenst = UNA_BIT_ERROR;
            break;
        }
#endif
        SWREG_write_field(&reg_value, &reg_mask, ((uint32_t) ddrm_ctx.ddenst), DDRM_REGISTER_STATUS_1_MASK_DDENST);
        break;
    default:
        // Nothing to do for other registers.
        break;
    }
    // Write register.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, reg_mask);
    return status;
}

/*******************************************************************/
NODE_status_t DDRM_check_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t reg_value = 0;
#ifndef DDRM_DDEN_FORCED_HARDWARE
    LOAD_status_t load_status = LOAD_SUCCESS;
    UNA_bit_representation_t dden = UNA_BIT_ERROR;
#endif
    // Read register.
    status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
    if (status != NODE_SUCCESS) goto errors;
    // Check address.
    switch (reg_addr) {
    case DDRM_REGISTER_ADDRESS_CONFIGURATION_0:
        // Store new value in NVM.
        if (reg_mask != 0) {
            NODE_write_nvm(reg_addr, reg_value);
        }
        break;
    case DDRM_REGISTER_ADDRESS_CONTROL_1:
        // DDEN.
        if ((reg_mask & DDRM_REGISTER_CONTROL_1_MASK_DDEN) != 0) {
            // Check pin mode.
#ifdef DDRM_DDEN_FORCED_HARDWARE
            status = NODE_ERROR_FORCED_HARDWARE;
            goto errors;
#else
            // Read bit.
            dden = SWREG_read_field(reg_value, DDRM_REGISTER_CONTROL_1_MASK_DDEN);
            // Compare to current state.
            if (dden != ddrm_ctx.ddenst) {
                // Set DC-DC state.
                load_status = LOAD_set_output_state(dden);
                LOAD_exit_error(NODE_ERROR_BASE_LOAD);
            }
#endif
        }
        break;
    default:
        // Nothing to do for other registers.
        break;
    }
errors:
    POWER_disable(POWER_REQUESTER_ID_DDRM, POWER_DOMAIN_ANALOG);
    DDRM_update_register(DDRM_REGISTER_ADDRESS_STATUS_1);
    return status;
}

/*******************************************************************/
NODE_status_t DDRM_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    int32_t vin_mv = 0;
    int32_t vout_mv = 0;
    int32_t iout_ua = 0;
    uint32_t reg_analog_data_1 = 0;
    uint32_t reg_analog_data_1_mask = 0;
    uint32_t reg_analog_data_2 = 0;
    uint32_t reg_analog_data_2_mask = 0;
    // Reset results.
    _DDRM_reset_analog_data();
    // DC-DC input voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VIN_MV, &vin_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_convert_mv(vin_mv), DDRM_REGISTER_ANALOG_DATA_1_MASK_VIN);
    // DC-DC output voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VOUT_MV, &vout_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_convert_mv(vout_mv), DDRM_REGISTER_ANALOG_DATA_1_MASK_VOUT);
    // Check IOUT measurement validity.
    if (vout_mv >= DDRM_IOUT_MEASUREMENT_VSH_MIN_MV) {
        // DC-DC output current.
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_IOUT_UA, &iout_ua);
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    }
    SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, UNA_convert_ua(iout_ua), DDRM_REGISTER_ANALOG_DATA_2_MASK_IOUT);
    // Write registers.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REGISTER_ADDRESS_ANALOG_DATA_2, reg_analog_data_2, reg_analog_data_2_mask);
errors:
    return status;
}

#endif /* DDRM */
