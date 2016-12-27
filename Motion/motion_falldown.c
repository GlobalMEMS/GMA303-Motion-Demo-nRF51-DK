/*
 *
 ****************************************************************************
 * Copyright (C) 2016 GlobalMEMS, Inc. <www.globalmems.com>
 * All rights reserved.
 *
 * File : motion_falldown.h
 *
 * Usage: Fall down detection
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
#include <math.h>
#include "nrf_delay.h"
#include "motion_falldown.h"

static int falling_down_flag;
static int falling_down_once_flag;
static int falling_still_counter;

/*!
 * @brief Initialize the fall down detection
 *
 * @param None
 *
 * @return None
 */
void fallDownInit(void){

  falling_down_flag = 0;
  falling_down_once_flag = 0;
  falling_still_counter = 0;

}

/*!
 * @brief Process the fall down detection
 *
 * @param[in] gVal accelerometer reading in g
 *
 * @return fall down flag
 *         0: not detected
 *         1: fall down detected
 */
int32_t processFallDown(float_xyzt_t gVal)
{

  if (!falling_down_once_flag){
    // X+Y > 4G and Z < 4G
    if ((fabsf(gVal.u.x) + fabsf(gVal.u.y)) > 4. && (fabsf(gVal.u.z) < 4.)){
      falling_down_once_flag = 1;
      falling_down_flag = 0;
      nrf_delay_ms(1000);
    }
  }
  else{
    if ((fabsf(gVal.u.x) < 0.25) && (fabsf(gVal.u.y) < 0.25) && (fabsf(gVal.u.z) < 0.5)){
      falling_still_counter ++;
      if (falling_still_counter >= 7){
	falling_down_flag = 1;
	falling_still_counter = 7;
      }
    }
    else{
      falling_still_counter = 0;
      falling_down_once_flag = 0;
    }
  }
	
  return falling_down_flag;
}
