/*
 * gps.h
 *
 *  Created on: 21 aug. 2024
 *      Author: Ludo
 */

#ifndef __GPS_H__
#define __GPS_H__

#include "error.h"
#include "led.h"
#include "neom8x.h"
#include "types.h"

/*** GPS structures ***/

/*!******************************************************************
 * \enum GPS_status_t
 * \brief GPS driver error codes.
 *******************************************************************/
typedef enum {
    // Driver errors.
    GPS_SUCCESS = 0,
    GPS_ERROR_NULL_PARAMETER,
    // Low level drivers errors.
    GPS_ERROR_BASE_NEOM8N = ERROR_BASE_STEP,
    GPS_ERROR_BASE_LED = (GPS_ERROR_BASE_NEOM8N + NEOM8X_ERROR_BASE_LAST),
    // Last base value.
    GPS_ERROR_BASE_LAST = (GPS_ERROR_BASE_LED + LED_ERROR_BASE_LAST)
} GPS_status_t;

#ifdef GPSM

/*!******************************************************************
 * \enum GPS_acquisition_status_t
 * \brief GPS acquisition status.
 *******************************************************************/
typedef enum {
    GPS_ACQUISITION_SUCCESS = 0,
    GPS_ACQUISITION_ERROR_TIMEOUT,
    GPS_ACQUISITION_ERROR_LAST
} GPS_acquisition_status_t;

/*!******************************************************************
 * \typedef GPS_time_t
 * \brief GPS time structure.
 *******************************************************************/
typedef NEOM8X_time_t GPS_time_t;

/*!******************************************************************
 * \typedef GPS_position_t
 * \brief GPS position structure.
 *******************************************************************/
typedef NEOM8X_position_t GPS_position_t;

/*!******************************************************************
 * \typedef GPS_timepulse_configuration_t
 * \brief GPS timepulse parameters structure.
 *******************************************************************/
typedef NEOM8X_timepulse_configuration_t GPS_timepulse_configuration_t;

/*** GPS functions ***/

/*!******************************************************************
 * \fn GPS_status_t GPS_init(void)
 * \brief Init GPS driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
GPS_status_t GPS_init(void);

/*!******************************************************************
 * \fn GPS_status_t GPS_de_init(void)
 * \brief Release GPS driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
GPS_status_t GPS_de_init(void);

/*!******************************************************************
 * \fn GPS_status_t GPS_get_time(GPS_time_t* gps_time, uint32_t timeout_seconds, uint32_t* acquisition_duration_seconds, GPS_acquisition_status_t* acquisition_status)
 * \brief Perform GPS position acquisition.
 * \param[in]   timeout_seconds: Fix timeout in seconds.
 * \param[out]  gps_time: Pointer to the GPS time if found.
 * \param[out]  acquisition_duration_seconds; Pointer to integer that will contain the GPS acquisition duration in seconds.
 * \param[out]  acquisition_success: Pointer to the acquisition success flag.
 * \retval      Function execution status.
 *******************************************************************/
GPS_status_t GPS_get_time(GPS_time_t* gps_time, uint32_t timeout_seconds, uint32_t* acquisition_duration_seconds, GPS_acquisition_status_t* acquisition_status);

/*!******************************************************************
 * \fn GPS_status_t GPS_get_position(GPS_position_t* gps_position, uint32_t timeout_seconds, uint32_t* acquisition_duration_seconds, GPS_acquisition_status_t* acquisition_status)
 * \brief Perform GPS position acquisition.
 * \param[in]   timeout_seconds: Fix timeout in seconds.
 * \param[out]  gps_position: Pointer to the GPS position if found.
 * \param[out]  acquisition_duration_seconds; Pointer to integer that will contain the GPS acquisition duration in seconds.
 * \param[out]  acquisition_success: Pointer to the acquisition success flag.
 * \retval      Function execution status.
 *******************************************************************/
GPS_status_t GPS_get_position(GPS_position_t* gps_position, uint32_t timeout_seconds, uint32_t* acquisition_duration_seconds, GPS_acquisition_status_t* acquisition_status);

/*!******************************************************************
 * \fn GPS_status_t GPS_set_backup_voltage(uint8_t state)
 * \brief Set GPS backup voltage state.
 * \param[in]   state: 0 to turn off, turn on otherwise.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
GPS_status_t GPS_set_backup_voltage(uint8_t state);

/*!******************************************************************
 * \fn uint8_t GPS_get_backup_voltage(void);
 * \brief Get GPS backup voltage state.
 * \param[in]   none
 * \param[out]  none
 * \retval      VBCKP pin state.
 *******************************************************************/
uint8_t GPS_get_backup_voltage(void);

/*!******************************************************************
 * \fn GPS_status_t GPS_set_timepulse(GPS_timepulse_configuration_t* configuration)
 * \brief Configure GPS timepulse output.
 * \param[in]   configuration: Pointer to the timepulse signal parameters.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
GPS_status_t GPS_set_timepulse(GPS_timepulse_configuration_t* configuration);

/*******************************************************************/
#define GPS_exit_error(base) { ERROR_check_exit(gps_status, GPS_SUCCESS, base) }

/*******************************************************************/
#define GPS_stack_error(base) { ERROR_check_stack(gps_status, GPS_SUCCESS, base) }

/*******************************************************************/
#define GPS_stack_exit_error(base, code) { ERROR_check_stack_exit(gps_status, GPS_SUCCESS, base, code) }

#endif /* GPSM */

#endif /* __GPS_H__ */
