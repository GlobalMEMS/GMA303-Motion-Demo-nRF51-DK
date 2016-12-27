/*
 *
 ****************************************************************************
 * Copyright (C) 2016 GlobalMEMS, Inc. <www.globalmems.com>
 * All rights reserved.
 *
 * File : motion_main_ctrl.c
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
#include <stdio.h>

#include "motion_main_ctrl.h"
#include "motion_pedo.h"
#include "motion_falldown.h"
#include "motion_shake.h"
#include "motion_orientation.h"
#include "motion_sleep_cycle.h"
#include "iir_filter.h"

#define alpha_pedo (0.8f)
#define alpha_fall (0.5f)
#define alpha_shake (0.4f)
#define alpha_seden (0.9f)
#define FLIP_INTERVAL_COUNT_THRESHOLD (1.0*MOTION_ALG_DATA_RATE_HZ)
#define SEDENTARY_THRESHOLD_G  (0.8)
#define SEDENTARY_DURATION     (2)
#define SEDENTARY_COUNT        (40)
#define SEDENTARY_TIME_OUT     (30*MOTION_ALG_DATA_RATE_HZ)

static MOTION_ALG_EVENT_HANDLER eventHandler = NULL;
static int32_t motionStates = 0, motionStates_pre = 0;
static uint32_t timeStep = 0;

//pedo states
static uint32_t ui32StepCount = 0, ui32StepCount_pre = 0;
static uint8_t ui8Activity = 0, ui8Activity_pre = 0;
static float fCal = 0., fCal_pre = 0.;
//Fall down states
static int32_t i32FallDown = 0;
//Shake states
static int32_t i32ShakeState = EVENT_SHAKE_NONE;
static motion_shake_param_t shakeParam;
//Raise hand states
static int32_t i32RaiseHandState = 0, i32RaiseHandState_pre = 0;
//Flip states
static int32_t i32FlipState = 0;
static int32_t i32FlipIntervalCount = 0;
//Sedentary states
static int32_t i32SedenState = 0, i32SedenState_pre = 0;
static int32_t i32SedenIntervalCount = 0, i32SedentaryDuration = 0;
static int32_t i32SedenSnoozeCount = 0, i32SedenSnoozeDuration = 0;
static motion_shake_param_t sedenShakeParam;
//Sleep cycle state
static motion_sleep_cycle_t sleepCycle = MOTION_SLEEP_CYCLE_NONE;
static motion_sleep_cycle_t sleepCycle_pre = MOTION_SLEEP_CYCLE_NONE;

//pedo high pass filter
static float histX_pedo[3];
static float histY_pedo[3];
static float coeffA_pedo[] = {alpha_pedo};
static float coeffB_pedo[] = {alpha_pedo, -alpha_pedo};
static  iir_filter_param_t iirPedo = {
  .dof = 3,
  .lenCoeffA = 1,
  .lenCoeffB = 2,
  .histX = histX_pedo,
  .histY = histY_pedo,
  .coeffA = coeffA_pedo,
  .coeffB = coeffB_pedo
};

//fall high pass filter
static float histX_fall[3];
static float histY_fall[3];
static float coeffA_fall[] = {alpha_fall};
static float coeffB_fall[] = {alpha_fall, -alpha_fall};
static  iir_filter_param_t iirFall = {
  .dof = 3,
  .lenCoeffA = 1,
  .lenCoeffB = 2,
  .histX = histX_fall,
  .histY = histY_fall,
  .coeffA = coeffA_fall,
  .coeffB = coeffB_fall
};

//shake high pass filter
static float histX_shake[3];
static float histY_shake[3];
static float coeffA_shake[] = {alpha_shake};
static float coeffB_shake[] = {alpha_shake, -alpha_shake};
static  iir_filter_param_t iirShake = {
  .dof = 3,
  .lenCoeffA = 1,
  .lenCoeffB = 2,
  .histX = histX_shake,
  .histY = histY_shake,
  .coeffA = coeffA_shake,
  .coeffB = coeffB_shake
};

//sedentary high pass filter
static float histX_seden[3];
static float histY_seden[3];
static float coeffA_seden[] = {alpha_seden};
static float coeffB_seden[] = {alpha_seden, -alpha_seden};
static  iir_filter_param_t iirSeden = {
  .dof = 3,
  .lenCoeffA = 1,
  .lenCoeffB = 2,
  .histX = histX_seden,
  .histY = histY_seden,
  .coeffA = coeffA_seden,
  .coeffB = coeffB_seden
};

/*!
 * @brief Initialize the motion algorithm main control
 *
 * @param[in] eventFcn Motion event handler call back function
 *
 * @return 
 *         0: success
 *         1: fail
 */
int8_t motion_alg_init(MOTION_ALG_EVENT_HANDLER eventFcn)
{

  if(eventFcn == NULL)
    return 0;

  eventHandler = eventFcn;
  motionStates = 0;
  timeStep = 0;

  return 1;
}

/*!
 * @brief Enable/Disenable algorithms
 *
 * @param[in] algSelections A bit-or (|) combination of motion_algorithm_t
 * @param[in] enable 1 to enable, 0 to disenble the selected algorithms
 *
 * @return None
 */
void motion_alg_enable(int32_t algSelections, int8_t enable)
{

  int i;

  motionStates_pre = motionStates;

  for(i = 0; i < MOTION_ALG_COUNT; ++i){

    if((algSelections >> i) & 0x01){
      if(enable) //set bit
	motionStates |= (1 << i);
      else //clear bit
	motionStates &= (~(1 << i));
    }
  }

  if(enable){ //enable

    //Pedo, calorie and activity
    if((motionStates_pre & (MOTION_ALG_PEDO | MOTION_ALG_CALORIE | MOTION_ALG_ACTIVITY)) == 0 &&
       (motionStates & (MOTION_ALG_PEDO | MOTION_ALG_CALORIE | MOTION_ALG_ACTIVITY))){

      //printf("Pedo init\n");
      
      ui32StepCount = ui32StepCount_pre = 0;
      ui8Activity = ui8Activity_pre = 0;
      fCal = fCal_pre = 0.;
      iirFilterInit(&iirPedo);  //Initialize pedo filter
      pedoInit();
    }

    //fall down detection
    if(motionStates & MOTION_ALG_FALL){

      //printf("Fall init\n");

      i32FallDown = 0;
      iirFilterInit(&iirFall); //Initialize fall filter
      fallDownInit();
    }

    //Shake
    if(motionStates & MOTION_ALG_SHAKE){
      
      //printf("Shake init\n");

      i32ShakeState = EVENT_SHAKE_NONE;
      iirFilterInit(&iirShake); //Initialize shake filter
      shakeInit(&shakeParam);

    }

    //Raise hand & flip
    if(motionStates & (MOTION_ALG_RAISE_HAND|MOTION_ALG_RAISE_HAND)){
      
      if((motionStates_pre & MOTION_ALG_RAISE_HAND) == 0 &&
	 (motionStates & MOTION_ALG_RAISE_HAND)){
	
	//printf("Raise hand init\n");

	i32RaiseHandState = i32RaiseHandState_pre = 0;
      }

      if((motionStates_pre & MOTION_ALG_FLIP) == 0 &&
	 (motionStates & MOTION_ALG_FLIP)){
	
	//printf("Flip init\n");

	i32FlipState = 0;
	i32FlipIntervalCount = 0;
      }

      if((motionStates_pre & (MOTION_ALG_RAISE_HAND|MOTION_ALG_RAISE_HAND)) == 0){
	//printf("Orient init\n");
	orientInit();
      }

    }

    //Sedentary
    if(motionStates & MOTION_ALG_SEDENTARY){

      //printf("Sedentary init\n");

      i32SedenState = i32SedenState_pre = i32SedenIntervalCount = 0;
      
      shakeInit(&sedenShakeParam);

      setShakeThreshold(&sedenShakeParam,
			SEDENTARY_THRESHOLD_G*SEDENTARY_THRESHOLD_G,
			0,
			0,
			X_AXIS | Y_AXIS | Z_AXIS);
      setShakeDuration(&sedenShakeParam,
		       SEDENTARY_DURATION,
		       0,
		       0,
		       X_AXIS | Y_AXIS | Z_AXIS);
      setShakeCount(&sedenShakeParam,
		    SEDENTARY_COUNT,
		    0,
		    0,
		    X_AXIS | Y_AXIS | Z_AXIS);
      setShakeTimeOutDuration(&sedenShakeParam,
			      SEDENTARY_TIME_OUT,
			      0,
			      0,
			      X_AXIS);
      setShakeEnable(&sedenShakeParam, 1, 0, 0, X_AXIS | Y_AXIS | Z_AXIS);

    }

    //Sleep cycle monitor
    if(motionStates & MOTION_ALG_SLEEP_CYCLE){

      //printf("Sleep cycle init\n");

      sleepCycle = sleepCycle_pre = MOTION_SLEEP_CYCLE_NONE;
      sleepCycleInit();
    }
  }
}

/*!
 * @brief Get algorithm state
 *
 * @param[in] alg Algorithm selected
 *
 * @return State of the selected algorithm
 */
int32_t motion_alg_get_state(motion_algorithm_t alg)
{
  switch(alg){
  case MOTION_ALG_PEDO:
    return (int32_t)ui32StepCount;
  case MOTION_ALG_CALORIE:
    return (int32_t)fCal;
  case MOTION_ALG_ACTIVITY:
    return (int32_t)ui8Activity;
  case MOTION_ALG_FALL:
    return i32FallDown;
  case MOTION_ALG_SHAKE:
    return i32ShakeState;
  case MOTION_ALG_RAISE_HAND:
    return i32RaiseHandState;
  case MOTION_ALG_FLIP:
    return i32FlipState;
  case MOTION_ALG_SEDENTARY:
    return i32SedenState;
  case MOTION_ALG_SLEEP_CYCLE:
    return sleepCycle;
  default:
    return 0;
  }
}

/*!
 * @brief Reset pedo step, activity and calories
 *
 * @param None
 *
 * @return None
 */
void motion_pedo_reset(void)
{
  ui32StepCount = ui32StepCount_pre = 0;
  ui8Activity = ui8Activity_pre = 0;
  fCal = fCal_pre = 0.;
  pedoReset();
}

/*!
 * @brief Set the calorie parameter
 *
 * @param[in] ht_m Height in meter
 * @param[in] wt_kg Weight in kg
 *
 * @return None
 */
void motion_calorie_set_param(float ht_m, float wt_kg)
{
  pedoSetParam(ht_m, wt_kg);
}

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
			    int32_t axes)
{
  int32_t tm = (int32_t)(timeout_s * MOTION_ALG_DATA_RATE_HZ + 0.5f);

  setShakeThreshold(&shakeParam, th_g, th_g, th_g, X_AXIS|Y_AXIS|Z_AXIS);
  setShakeDuration(&shakeParam, dur, dur, dur, X_AXIS|Y_AXIS|Z_AXIS);
  setShakeCount(&shakeParam, cnt, cnt, cnt, X_AXIS|Y_AXIS|Z_AXIS);
  setShakeTimeOutDuration(&shakeParam, tm, tm, tm, X_AXIS|Y_AXIS|Z_AXIS);

  //Reset axes enable, then set selected axes enable
  setShakeEnable(&shakeParam, 0, 0, 0, X_AXIS|Y_AXIS|Z_AXIS);
  setShakeEnable(&shakeParam, 1, 1, 1, axes);
}

/*!
 * @brief Set the sedentary parameters
 *
 * @param[in] sedentary_time_min Sedentary time in min
 * @param[in] snooze_interval_min Interval to alarm sendentary
 *
 * @return None
 */
void motion_sedentary_set_param(int32_t sedentary_time_min, int32_t snooze_interval_min)
{

  i32SedentaryDuration = sedentary_time_min * 60 * MOTION_ALG_DATA_RATE_HZ;
  i32SedenIntervalCount = i32SedentaryDuration;
  i32SedenSnoozeDuration = snooze_interval_min * 60 * MOTION_ALG_DATA_RATE_HZ;
  i32SedenSnoozeCount = 0;

}


void motion_alg_process_pedo(float_xyzt_t gVal)
{

  float_xyzt_t fData_out;

  //high-pass filter the data
  filterData((float *)&gVal, (float *)&fData_out, &iirPedo);

  if(timeStep < 2*MOTION_ALG_DATA_RATE_HZ) return; //wait for the filter to settle down
    
  ui32StepCount = processPedo(fData_out);
  ui8Activity = getPedoActivity();
  fCal = getPedoCalorie();

  if(ui32StepCount != ui32StepCount_pre){
    ui32StepCount_pre = ui32StepCount;
    if(motionStates & MOTION_ALG_PEDO)
      eventHandler(MOTION_ALG_PEDO, (int32_t) ui32StepCount);
  }

  if(ui8Activity != ui8Activity_pre){
    ui8Activity_pre = ui8Activity;
    if(motionStates & MOTION_ALG_ACTIVITY)
      eventHandler(MOTION_ALG_ACTIVITY, (int32_t) ui8Activity);
  }

  if(fCal - fCal_pre > 1.){
    fCal_pre = fCal;
    if(motionStates & MOTION_ALG_CALORIE)
      eventHandler(MOTION_ALG_CALORIE, (int32_t) fCal);
  }
}

void motion_alg_process_fall(float_xyzt_t gVal)
{

  float_xyzt_t fData_out;

  //high-pass filter the data
  filterData((float *)&gVal, (float *)&fData_out, &iirFall);
  
  i32FallDown = processFallDown(fData_out);

  if(i32FallDown != 0){
    eventHandler(MOTION_ALG_FALL, (int32_t) i32FallDown);
  }

}

void motion_alg_process_shake(float_xyzt_t gVal)
{

  float_xyzt_t fData_out;

  //high-pass filter the data
  filterData((float *)&gVal, (float *)&fData_out, &iirShake);
  
  i32ShakeState = processShake(&shakeParam, fData_out);

  if(i32ShakeState != EVENT_SHAKE_NONE){
    eventHandler(MOTION_ALG_SHAKE, (int32_t) i32ShakeState);
  }

}

void motion_alg_process_raise_hand_flip(float_xyzt_t gVal)
{

  motion_orient_t orient = processOrient(gVal);

  //Raise hand
  if(motionStates & MOTION_ALG_RAISE_HAND){

    i32RaiseHandState = (orient == ORIENT_Z_POS) ? 1 : 0;

    if(i32RaiseHandState != i32RaiseHandState_pre){
    
      i32RaiseHandState_pre = i32RaiseHandState;
      eventHandler(MOTION_ALG_RAISE_HAND, (int32_t) i32RaiseHandState);
    }
  }

  //Flip
  if(motionStates & MOTION_ALG_FLIP){

    i32FlipState = 0;
    --i32FlipIntervalCount;

    if(orient == ORIENT_Z_POS)
      i32FlipIntervalCount = FLIP_INTERVAL_COUNT_THRESHOLD; //start count down
    else if(orient == ORIENT_Z_NEG){

      if(i32FlipIntervalCount >= 0){

	i32FlipState = 1;
	i32FlipIntervalCount = 0;
	eventHandler(MOTION_ALG_FLIP, (int32_t) i32FlipState);
      }

    }

    if(i32FlipIntervalCount < 0)
      i32FlipIntervalCount = 0;
  }
}

void motion_alg_process_sedentary(float_xyzt_t gVal)
{

  float_xyzt_t fData_out;
  int32_t i32Res, i;
  float fTmp;

  //high-pass filter the data
  filterData((float *)&gVal, (float *)&fData_out, &iirSeden);
  //Calculate magnitude^2 in g and store in the X
  fTmp = 0.f;
  for(i = 0; i < 3; ++i)
    fTmp += fData_out.v[i] * fData_out.v[i];
  fData_out.v[0] = fTmp;
  fData_out.v[1] = fData_out.v[2] = 0.f;
  
  i32Res = processShake(&sedenShakeParam, fData_out);

  --i32SedenIntervalCount;
  --i32SedenSnoozeCount;
  
  if(i32Res != EVENT_SHAKE_NONE){
    i32SedenIntervalCount = i32SedentaryDuration;
    i32SedenState = 0;
  }

  if(i32SedenIntervalCount == 0){
    i32SedenState = 1;
    i32SedenSnoozeCount = i32SedenSnoozeDuration;
  }
  else if(i32SedenIntervalCount < 0)
    i32SedenIntervalCount = 0;

  if(i32SedenState != i32SedenState_pre){

    i32SedenState_pre = i32SedenState;
    eventHandler(MOTION_ALG_SEDENTARY, (int32_t) i32SedenState);

  }
  else{
    //check snooze
    if(i32SedenState && (i32SedenSnoozeCount == 0)){
      i32SedenIntervalCount = i32SedentaryDuration;
      eventHandler(MOTION_ALG_SEDENTARY, (int32_t) i32SedenState);
    }
  }

  if(i32SedenSnoozeCount < 0) i32SedenSnoozeCount = 0;
}


void motion_alg_process_sleep_cycle(float_xyzt_t gVal)
{

  sleepCycle = processSleepCycle(gVal);

  if(sleepCycle != sleepCycle_pre){
    sleepCycle_pre = sleepCycle;
    eventHandler(MOTION_ALG_SLEEP_CYCLE, (int32_t) sleepCycle);
  }

}

/*!
 * @brief Run the motion algorithm, frequency should be 20Hz
 *
 * @param[in] gVal accelerometer reading in g
 *
 * @return None
 */
void motion_alg_process_data(float_xyzt_t gVal)
{

  timeStep += 1;

  if(motionStates & (MOTION_ALG_PEDO | MOTION_ALG_CALORIE | MOTION_ALG_ACTIVITY))
    motion_alg_process_pedo(gVal);
  
  if(motionStates & MOTION_ALG_FALL)
    motion_alg_process_fall(gVal);

  if(motionStates & MOTION_ALG_SHAKE)
    motion_alg_process_shake(gVal);

  if(motionStates & (MOTION_ALG_RAISE_HAND|MOTION_ALG_FLIP))
    motion_alg_process_raise_hand_flip(gVal);

  if(motionStates & MOTION_ALG_SEDENTARY)
    motion_alg_process_sedentary(gVal);

  if(motionStates & MOTION_ALG_SLEEP_CYCLE)
    motion_alg_process_sleep_cycle(gVal);
}

