/*
 *
 ****************************************************************************
 * Copyright (C) 2016 GlobalMEMS, Inc. <www.globalmems.com>
 * All rights reserved.
 *
 * File : motion_sleep_cycle.c
 *
 * Usage: motion sleep cycle monitor
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
#include "motion_main_ctrl.h"
#include "motion_sleep_cycle.h"
#include "motion_shake.h"
#include "iir_filter.h"
/*
 * The body movement rates are adapted from the article:
 * "Rate and distribution of body movements during sleep in humans, Johanna Wilde-Frenz and
 * Hartmut Schulz, Perceptual and Motor Skills, 1983, 56, 275-283"
 */
#define SLEEP_CYCLE_WAKE_LB (0.5)
#define SLEEP_CYCLE_S1_UB (0.5)
#define SLEEP_CYCLE_S1_LB (0.28)
#define SLEEP_CYCLE_REM_UB (0.28)
#define SLEEP_CYCLE_REM_LB (0.12)
#define SLEEP_CYCLE_S2_UB (0.12)
#define SLEEP_CYCLE_S2_LB (0.06)
#define SLEEP_CYCLE_S3_UB (0.06)
#define SLEEP_CYCLE_S3_LB (0.02)

#define SLEEP_CYCLE_NONE_REPEAT_COUNT (5)
#define SLEEP_CYCLE_CHECK_INTERVAL_SEC (60.f)
#define SLEEP_CYCLE_INTERVAL_DURATION \
  (SLEEP_CYCLE_CHECK_INTERVAL_SEC * MOTION_ALG_DATA_RATE_HZ)
#define alpha_sleep (0.9f)
#define SLEEP_THRESHOLD_G (0.4)
#define SLEEP_DURATION     (2)
#define SLEEP_COUNT        (1)
#define SLEEP_TIME_OUT     (MAX_DURATION)

//Sleep cycle state
static motion_sleep_cycle_t sleepCycleState = MOTION_SLEEP_CYCLE_NONE;
static int32_t i32SleepCycleIntervalCount = 0;
static int32_t i32SleepMovementCount = 0;
static int32_t i32SleepCycleNoneCount = 0;
static motion_shake_param_t sleepShakeParam;

//sleep cycle high pass filter
static float histX_sleep[3];
static float histY_sleep[3];
static float coeffA_sleep[] = {alpha_sleep};
static float coeffB_sleep[] = {alpha_sleep, -alpha_sleep};
static  iir_filter_param_t iirSleep = {
  .dof = 3,
  .lenCoeffA = 1,
  .lenCoeffB = 2,
  .histX = histX_sleep,
  .histY = histY_sleep,
  .coeffA = coeffA_sleep,
  .coeffB = coeffB_sleep
};

/*!
 * @brief Initialize the sleep cycle monitor
 *
 * @param None
 *
 * @return None
 */
void sleepCycleInit(void){

  sleepCycleState = MOTION_SLEEP_CYCLE_NONE;
  i32SleepCycleIntervalCount = 0;
  i32SleepMovementCount = 0;
  i32SleepCycleNoneCount = 0;
  iirFilterInit(&iirSleep);  //Initialize filter for sleep cycle monitor

  //Movement definition
  shakeInit(&sleepShakeParam);
  setShakeThreshold(&sleepShakeParam,
		    SLEEP_THRESHOLD_G * SLEEP_THRESHOLD_G,
		    0,
		    0,
		    X_AXIS | Y_AXIS | Z_AXIS);
  setShakeDuration(&sleepShakeParam,
		   SLEEP_DURATION,
		   0,
		   0,
		   X_AXIS | Y_AXIS | Z_AXIS);
  setShakeCount(&sleepShakeParam,
		SLEEP_COUNT,
		0,
		0,
		X_AXIS | Y_AXIS | Z_AXIS);
  setShakeTimeOutDuration(&sleepShakeParam,
			  SLEEP_TIME_OUT,
			  0,
			  0,
			  X_AXIS);
  setShakeEnable(&sleepShakeParam, 1, 0, 0, X_AXIS | Y_AXIS | Z_AXIS);

}

/*!
 * @brief Process the sleep cycle
 *
 * @param[in] gVal accelerometer reading in g
 *
 * @return Sleep cycle
 */
motion_sleep_cycle_t processSleepCycle(float_xyzt_t gVal){

  float_xyzt_t fData_out;
  int32_t i32Res, i;
  float fTmp;
  motion_sleep_cycle_t sleepCycle = sleepCycleState;

  //high-pass filter the data
  filterData((float *)&gVal, (float *)&fData_out, &iirSleep);
  //Calculate magnitude^2 in g and store in the X
  fTmp = 0.f;
  for(i = 0; i < 3; ++i)
    fTmp += fData_out.v[i] * fData_out.v[i];
  fData_out.v[0] = fTmp;
  fData_out.v[1] = fData_out.v[2] = 0.f;

  i32Res = processShake(&sleepShakeParam, fData_out);

  if(i32Res != EVENT_SHAKE_NONE){
    i32SleepMovementCount += 1;
  }

  ++i32SleepCycleIntervalCount;

  if(i32SleepCycleIntervalCount >= SLEEP_CYCLE_INTERVAL_DURATION){

    if(fTmp < SLEEP_CYCLE_S3_LB)
      i32SleepCycleNoneCount += 1;
    else
      i32SleepCycleNoneCount = 0;

    //calculate the body movement rate
    fTmp = ((float)i32SleepMovementCount) / SLEEP_CYCLE_CHECK_INTERVAL_SEC;

    if(fTmp >= SLEEP_CYCLE_WAKE_LB)
      sleepCycle = MOTION_SLEEP_CYCLE_WAKE;
    else if( fTmp < SLEEP_CYCLE_S1_UB && fTmp >= SLEEP_CYCLE_S1_LB)
      sleepCycle = MOTION_SLEEP_CYCLE_S1;
    else if( fTmp < SLEEP_CYCLE_REM_UB && fTmp >= SLEEP_CYCLE_REM_LB)
      sleepCycle = MOTION_SLEEP_CYCLE_REM;
    else if( fTmp < SLEEP_CYCLE_S2_UB && fTmp >= SLEEP_CYCLE_S2_LB)
      sleepCycle = MOTION_SLEEP_CYCLE_S2;
    else if( fTmp < SLEEP_CYCLE_S3_UB && fTmp >= SLEEP_CYCLE_S3_LB)
      sleepCycle = MOTION_SLEEP_CYCLE_S3;
    else if( i32SleepCycleNoneCount >= SLEEP_CYCLE_NONE_REPEAT_COUNT)
      sleepCycle = MOTION_SLEEP_CYCLE_NONE;

    //Update the state
    sleepCycleState = sleepCycle;

    //reset the counter
    i32SleepCycleIntervalCount = 0;
    i32SleepMovementCount = 0;

  }

  return sleepCycle;

}
