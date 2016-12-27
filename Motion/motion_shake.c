/*
 *
 ****************************************************************************
 * Copyright (C) 2016 GlobalMEMS, Inc. <www.globalmems.com>
 * All rights reserved.
 *
 * File : motion_shake.c
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
#include "motion_shake.h"

static const m_axis_t allAxes[] = {X_AXIS, Y_AXIS, Z_AXIS};

void resetStates(motion_shake_param_t *pParam, int32_t face, int32_t axis)
{

  pParam->stateDuration[face].v[axis] = 0;
  pParam->stateCount[face].v[axis] = 0;
  pParam->stateTimeOutDuration[face].v[axis] = 0;
  pParam->flagToCount[face].v[axis] = 0;

}

/*!
 * @brief Initialize the shake detection
 *
 * @param pParam Pointer to the shake parameter struct
 *
 * @return None
 */
void shakeInit(motion_shake_param_t *pParam)
{

  int32_t i, j;

  pParam->shakeEvent = EVENT_SHAKE_NONE;

  //set threshold to default values
  setShakeThreshold(pParam,
		    DEFAULT_SHAKE_THRESHOLD,
		    DEFAULT_SHAKE_THRESHOLD,
		    DEFAULT_SHAKE_THRESHOLD,
		    X_AXIS | Y_AXIS | Z_AXIS);

  //set duration to default values
  setShakeDuration(pParam,
		   DEFAULT_SHAKE_DURATION,
		   DEFAULT_SHAKE_DURATION,
		   DEFAULT_SHAKE_DURATION,
		   X_AXIS | Y_AXIS | Z_AXIS);

  //set count to default values
  setShakeCount(pParam,
		DEFAULT_SHAKE_COUNT,
		DEFAULT_SHAKE_COUNT,
		DEFAULT_SHAKE_COUNT,
		X_AXIS | Y_AXIS | Z_AXIS);

  //set reset duration to default values
  setShakeTimeOutDuration(pParam,
			  DEFAULT_SHAKE_RESET_COUNT_DURATION,
			  DEFAULT_SHAKE_RESET_COUNT_DURATION,
			  DEFAULT_SHAKE_RESET_COUNT_DURATION,
			  X_AXIS | Y_AXIS | Z_AXIS);

  //Enable all axes
  setShakeEnable(pParam, 1, 1, 1, X_AXIS | Y_AXIS | Z_AXIS);

  //Reset all states and flags
  for(i = 0; i < 2; ++i)
    for(j = 0; j < 3; ++j){
      resetStates(pParam, i, j);
    }
}

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
		       int32_t axesSel)
{
  int i;
  float_xyzt_t thTmp = {thX, thY, thZ};

  for(i = 0; i < 3; ++i)
    if(axesSel & allAxes[i]) pParam->shakeThresholdInG.v[i] = thTmp.v[i];
}

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
		      int32_t axesSel)
{

  int i;
  raw_data_xyzt_t valTmp = {durX, durY, durZ};

  for(i = 0; i < 3; ++i)
    if(axesSel & allAxes[i]) pParam->shakeDuration.v[i] = valTmp.v[i];

}

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
		   int32_t axesSel)
{

  int i;
  raw_data_xyzt_t valTmp = {cntX, cntY, cntZ};

  for(i = 0; i < 3; ++i)
    if(axesSel & allAxes[i]) pParam->shakeCount.v[i] = valTmp.v[i];

}

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
			     int32_t axesSel)
{

  int i;
  raw_data_xyzt_t valTmp = {tmdX, tmdY, tmdZ};

  for(i = 0; i < 3; ++i)
    if(axesSel & allAxes[i]) pParam->shakeTimeOutDuration.v[i] = valTmp.v[i];

}

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
		    int32_t axesSel)
{

  int i;
  raw_data_xyzt_t valTmp = {enableX, enableY, enableZ};

  for(i = 0; i < 3; ++i)
    if(axesSel & allAxes[i]) pParam->shakeEnable.v[i] = (valTmp.v[i] == 0)? 0 : 1;

}

/*!
 * @brief Process the shake detection
 *
 * @param pParam Pointer to the shake parameter struct
 * @param gVal accelerometer reading in g
 *
 * @return An "or" (|) comnibation of shake_event_t;
 */
int32_t processShake(motion_shake_param_t *pParam, float_xyzt_t gVal)
{
  int32_t i, j;
  static const shake_event_type allShakeEvents[][3] = {
    {EVENT_SHAKE_X_POS, EVENT_SHAKE_Y_POS, EVENT_SHAKE_Z_POS},
    {EVENT_SHAKE_X_NEG, EVENT_SHAKE_Y_NEG, EVENT_SHAKE_Z_NEG}
  };

  pParam->shakeEvent = EVENT_SHAKE_NONE;

  for(i = 0; i < 3; ++i){

    //Check enable
    if(pParam->shakeEnable.v[i] == 0) continue;

    //Check against threshold
    if(gVal.v[i] >= pParam->shakeThresholdInG.v[i]) //positive
      pParam->stateDuration[0].v[i] += 1; //increase duration count
    else
      pParam->stateDuration[0].v[i] = 0; //reset duration count

    if(gVal.v[i] <= -pParam->shakeThresholdInG.v[i]) //negative
      pParam->stateDuration[1].v[i] += 1; //increase duratin count
    else
      pParam->stateDuration[1].v[i] = 0; //reset duration count
    
    for(j = 0; j < 2; ++j){

      //Init the time out counter
      if((pParam->stateDuration[j].v[i] == 1) && (pParam->stateCount[j].v[i] == 0))
	pParam->stateTimeOutDuration[j].v[i] = 0;

      //Reset the ToCount flag
      if(pParam->stateDuration[j].v[i] == 1)
	pParam->flagToCount[j].v[i] = 1;

      //Check duration
      if(pParam->stateDuration[j].v[i] >= pParam->shakeDuration.v[i] 
	 && pParam->flagToCount[j].v[i] == 1){
	
	pParam->stateCount[j].v[i] += 1;
	pParam->flagToCount[j].v[i] = 0;

      }

      //Check count for event
      if(pParam->stateCount[j].v[i] >= pParam->shakeCount.v[i]
	 && pParam->stateDuration[j].v[i] == 0){
	
	pParam->shakeEvent |= allShakeEvents[j][i];
	//reset the states and flags
	resetStates(pParam, j, i);
      }

      //Check time out
      if(pParam->stateTimeOutDuration[j].v[i] >= pParam->shakeTimeOutDuration.v[i])
	resetStates(pParam, j, i);
      else
	pParam->stateTimeOutDuration[j].v[i] += 1;

    }
  }

  return pParam->shakeEvent;
}

/*!
 * @brief Get the current shake event
 *
 * @param pParam Pointer to the shake parameter struct
 * @param None
 *
 * @return An "or" (|) comnibation of shake_event_t;
 */
int32_t getShakeEvent(motion_shake_param_t *pParam)
{

  return pParam->shakeEvent;

}

