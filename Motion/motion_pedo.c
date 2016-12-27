/*
 *
 ****************************************************************************
 * Copyright (C) 2016 GlobalMEMS, Inc. <www.globalmems.com>
 * All rights reserved.
 *
 * File : motion_pedo.h
 *
 * Usage: pedometer by accelerometer readings
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

#include "motion_pedo.h"

#define PEDO_SENSITIVITY {512.0f, 512.0f, 512.0f} //pedometer sensitivity, codes/g
#define CALORIE_UPDATE_TIME_INTERVAL_SEC 2
#define SAMPLING_RATE_HZ                 25
#define CALORIE_UPDATE_TIME_STEPS        (CALORIE_UPDATE_TIME_INTERVAL_SEC * SAMPLING_RATE_HZ)

static float height_m = 0.0;
static float weight_kg = 0.0;
static float calories = 0.0;
static uint32_t step_pre = 0;
static uint32_t time_step_interval = 0;

static const float pedoSensitivity[] = PEDO_SENSITIVITY;

float getStride(int32_t step_2)
{

  float sf = 1.2;

  if(step_2 < 2)
    sf = 0.2;
  else if(2 <= step_2 < 3)
    sf = 0.25;
  else if(3 <= step_2 < 4)
    sf = 1/3.0;
  else if(4 <= step_2 < 5)
    sf = 0.5;
  else if(5 <= step_2 < 6)
    sf = 1.0/1.2;
  else if(6 <= step_2 < 8)
    sf = 1.0;
  else
    sf = 1.2;

  return height_m * sf;

}

/*!
 * @brief Initialize the pedometer
 *
 * @return None
 */
void pedoInit()
{

  calories = 0.0;
  step_pre = 0;
  time_step_interval = 0;

  PEDO_InitAlgo(0);
}

/*!
 * @brief Set the pedometer parameters
 *
 * @param[in] ht_m Height in meter
 * @param[in] wt_kg Weight in kg
 *
 * @return None
 */
void pedoSetParam(float ht_m, float wt_kg){

  height_m = ht_m;
  weight_kg = wt_kg;

}

/*!
 * @brief Reset the pedomters, step and calories will reset to 0
 *
 * @param None
 *
 * @return None
 */
void pedoReset(void)
{

  calories = 0.0;
  step_pre = 0;
  time_step_interval = 0;

  PEDO_ResetAlgo();
}

/*!
 * @brief Process the pedometer
 *
 * @param[in] gVal accelerometer reading in g
 *
 * @return Pedometer steps
 */
uint32_t processPedo(float_xyzt_t gVal)
{
  int i;
  int16_t acc[3];
  int32_t step_cal;
  int32_t pedoStep;

  for(i = 0; i < 3; ++i)
    acc[i] = (int16_t)(gVal.v[i] * pedoSensitivity[i] + 0.5);

  //feed to algorithm
  PEDO_ProcessAccelarationData(acc[0], acc[1], acc[2]);

  //get the pedo steps
  pedoStep = PEDO_GetStepCount();

  //Calculate the carlories
  ++time_step_interval;
  if(time_step_interval > CALORIE_UPDATE_TIME_STEPS){

    step_cal = (pedoStep - step_pre);
    time_step_interval = 0;
    step_pre = pedoStep;

    if(step_cal == 0)
      calories += weight_kg / 1800.0;
    else
      calories += step_cal * getStride(step_cal) * weight_kg / 800.0;
  }

  return pedoStep;
}

/*!
 * @brief Get the pedometer steps
 *
 * @param None
 *
 * @return Pedometer steps
 */
uint32_t getPedoStep(void)
{

  return PEDO_GetStepCount();

}

/*!
 * @brief Get the pedometer activity
 *
 * @param None
 *
 * @return Pedometer acivity
 *         0: STATIONARY
 *         1: WALK
 *         3: RUN
 */
uint8_t getPedoActivity(void)
{

  return PEDO_GetActivity();

}

/*!
 * @brief Get the pedometer calories
 *
 * @param None
 *
 * @return Pedometer calories
 */
float getPedoCalorie(void)
{

  return calories;

}
