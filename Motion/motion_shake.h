/*
 *
 ****************************************************************************
 * Copyright (C) 2016 GlobalMEMS, Inc. <www.globalmems.com>
 * All rights reserved.
 *
 * File : motion_shake.h
 *
 * Usage: Shake motion detection
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

#ifndef __MOTION_SHAKE_H__
#define __MOTION_SHAKE_H__

#include "type_support.h"

#define MAX_DURATION                        0x7FFFFFFF
#define DEFAULT_SHAKE_THRESHOLD             0.8f           //threshold in g
#define DEFAULT_SHAKE_DURATION              2
#define DEFAULT_SHAKE_COUNT                 2
#define DEFAULT_SHAKE_RESET_COUNT_DURATION  MAX_DURATION

typedef enum {
  EVENT_SHAKE_NONE = 0,
  EVENT_SHAKE_X_POS = 1, EVENT_SHAKE_X_NEG = 2,
  EVENT_SHAKE_Y_POS = 4, EVENT_SHAKE_Y_NEG = 8,
  EVENT_SHAKE_Z_POS = 16, EVENT_SHAKE_Z_NEG = 32} shake_event_type;

typedef struct{

  float_xyzt_t shakeThresholdInG;
  raw_data_xyzt_t shakeDuration;
  raw_data_xyzt_t shakeCount;
  raw_data_xyzt_t shakeTimeOutDuration;
  raw_data_xyzt_t shakeEnable;
  int32_t shakeEvent;

  raw_data_xyzt_t stateDuration[2];
  raw_data_xyzt_t stateCount[2];
  raw_data_xyzt_t stateTimeOutDuration[2];
  raw_data_xyzt_t flagToCount[2];

} motion_shake_param_t;


/*!
 * @brief Initialize the shake detection
 *
 * @param pParam Pointer to the shake parameter struct
 *
 * @return None
 */
void shakeInit(motion_shake_param_t *pParam);

/*!
 * @brief Set shake detection threshold
 *        Only the axes specified in the "axesSel" will be updated, others will be kept 
 *        intact
 *
 * @param pParam Pointer to the shake parameter struct
 * @param thX X-axis thresold
 * @param thY Y-axis threshold
 * @param thz Z-axis threshold
 * @param axesSel An "or" (|) combination of X_AXIS, Y_AXIS, Z_AXIS
 *
 * @return None
 */
void setShakeThreshold(motion_shake_param_t *pParam,
		       float thX,
		       float thY,
		       float thZ,
		       int32_t axesSel);

/*!
 * @brief Set shake duration for detection that is above threhold
 *        Only the axes specified in the "axesSel" will be updated, others will be kept 
 *        intact
 *
 * @param pParam Pointer to the shake parameter struct
 * @param durX X-axis duration
 * @param durY Y-axis duration
 * @param durZ Z-axis duration
 * @param axesSel An "or" (|) combination of X_AXIS, Y_AXIS, Z_AXIS
 *
 * @return None
 */
void setShakeDuration(motion_shake_param_t *pParam,
		      int32_t durX,
		      int32_t durY,
		      int32_t durZ,
		      int32_t axesSel);

/*!
 * @brief Set successive shake count
 *        Only the axes specified in the "axesSel" will be updated, others will be kept 
 *        intact
 *
 * @param pParam Pointer to the shake parameter struct
 * @param cntX X-axis count
 * @param cntY Y-axis count
 * @param cntZ Z-axis count
 * @param axesSel An "or" (|) combination of X_AXIS, Y_AXIS, Z_AXIS
 *
 * @return None
 */
void setShakeCount(motion_shake_param_t *pParam,
		   int32_t cntX,
		   int32_t cntY,
		   int32_t cntZ,
		   int32_t axesSel);

/*!
 * @brief Set shake timeout duration
 *        Only the axes specified in the "axesSel" will be updated, others will be kept 
 *        intact
 *
 * @param pParam Pointer to the shake parameter struct
 * @param tmdX X-axis timeout duration
 * @param tmdY Y-axis timeout duration
 * @param tmdZ Z-axis timeout duration
 * @param axesSel An "or" (|) combination of X_AXIS, Y_AXIS, Z_AXIS
 *
 * @return None
 */
void setShakeTimeOutDuration(motion_shake_param_t *pParam,
			     int32_t tmdX,
			     int32_t tmdY,
			     int32_t tmdZ,
			     int32_t axesSel);

/*!
 * @brief Enable/Disable shake detection on specified axis
 *        Only the axes specified in the "axesSel" will be updated, others will be kept 
 *        intact
 *
 * @param pParam Pointer to the shake parameter struct
 * @param enableX X-axis, 1 to enable & 0 to disable
 * @param enableY Y-axis, 1 to enable & 0 to disable
 * @param enableZ Z-axis, 1 to enable & 0 to disable
 * @param axesSel An "or" (|) combination of X_AXIS, Y_AXIS, Z_AXIS
 *
 * @return None
 */
void setShakeEnable(motion_shake_param_t *pParam,
		    int32_t enableX,
		    int32_t enableY,
		    int32_t enableZ,
		    int32_t axesSel);

/*!
 * @brief Process the shake detection
 *
 * @param pParam Pointer to the shake parameter struct
 * @param gVal accelerometer reading in g
 *
 * @return An "or" (|) comnibation of shake_event_t;
 */
int32_t processShake(motion_shake_param_t *pParam, float_xyzt_t gVal);

/*!
 * @brief Get the current shake event
 *
 * @param pParam Pointer to the shake parameter struct
 * @param None
 *
 * @return An "or" (|) comnibation of shake_event_t;
 */
int32_t getShakeEvent(motion_shake_param_t *pParam);

#endif //__MOTION_SHAKE_H__
