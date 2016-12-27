/*
 *
 ****************************************************************************
 * Copyright (C) 2016 GlobalMEMS, Inc. <www.globalmems.com>
 * All rights reserved.
 *
 * File : motion_main_ctrl.h
 *
 * Usage: motion algorithm main control
 *
 ****************************************************************************
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 **************************************************************************/

#ifndef __MOTION_MAIN_CTRL_H__
#define __MOTION_MAIN_CTRL_H__

#include <stdint.h>
#include "type_support.h"

#define MOTION_ALG_DATA_RATE_HZ (25)
#define MOTION_ALG_COUNT (9)

typedef enum {
  MOTION_ALG_NONE = 0,
  MOTION_ALG_PEDO = 1,
  MOTION_ALG_CALORIE = 2,
  MOTION_ALG_ACTIVITY = 4,
  MOTION_ALG_FALL = 8,
  MOTION_ALG_SHAKE = 16,
  MOTION_ALG_RAISE_HAND = 32,
  MOTION_ALG_FLIP = 64,
  MOTION_ALG_SEDENTARY = 128,
  MOTION_ALG_SLEEP_CYCLE = 256
} motion_algorithm_t;

typedef void (*MOTION_ALG_EVENT_HANDLER)(motion_algorithm_t event, int32_t i32Data);

/*!
 * @brief Initialize the motion algorithm main control
 *
 * @param[in] eventFcn Motion event handler call back function
 *
 * @return 
 *         0: success
 *         1: fail
 */
int8_t motion_alg_init(MOTION_ALG_EVENT_HANDLER eventFcn);

/*!
 * @brief Enable/Disenable algorithms
 *
 * @param[in] algSelections A bit-or (|) combination of motion_algorithm_t
 * @param[in] enable 1 to enable, 0 to disenble the selected algorithms
 *
 * @return None
 */
void motion_alg_enable(int32_t algSelections, int8_t enable);

/*!
 * @brief Get algorithm state
 *
 * @param[in] alg Algorithm selected
 *
 * @return State of the selected algorithm
 */
int32_t motion_alg_get_state(motion_algorithm_t alg);

/*!
 * @brief Reset pedo step, activity and calories
 *
 * @param None
 *
 * @return None
 */
void motion_pedo_reset(void);

/*!
 * @brief Set the calorie parameter
 *
 * @param[in] ht_m Height in meter
 * @param[in] wt_kg Weight in kg
 *
 * @return None
 */
void motion_calorie_set_param(float ht_m, float wt_kg);


/*!
 * @brief Set the shake parameters
 *
 * @param[in] th_g threshold in g
 * @param[in] dur Peak duration, number of time steps
 * @param[in] cnt Peak count
 * @param[in] timeout_s Timeout (sec) for the peak count
 * @param[in] axes Select axes, a bit-or (|) combination of X_AXIS, Y_AXIS, Z_AXIS
 *
 * @return None
 */
void motion_shake_set_param(float th_g,
			    int32_t dur,
			    int32_t cnt,
			    float timeout_s,
			    int32_t axes);

/*!
 * @brief Set the sedentary parameters
 *
 * @param[in] sedentary_time_min Sedentary time in min
 * @param[in] snooze_interval_min Interval to alarm sendentary
 *
 * @return None
 */
void motion_sedentary_set_param(int32_t sedentary_time_min, int32_t snooze_interval_min);

/*!
 * @brief Run the motion algorithm, frequency should be 20Hz
 *
 * @param[in] gVal accelerometer reading in g
 *
 * @return None
 */
void motion_alg_process_data(float_xyzt_t gVal);


#endif //__MOTION_MAIN_CTRL_H__
