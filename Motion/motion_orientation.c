/*
 *
 ****************************************************************************
 * Copyright (C) 2016 GlobalMEMS, Inc. <www.globalmems.com>
 * All rights reserved.
 *
 * File : motion_orientation.h
 *
 * Usage: Orientataion detection
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
#include "motion_orientation.h"

#define Rad2Deg (57.29578)
#define SWITCH_THRESHOLD_DEG 45.f
#define SWITCH_HYSTERESIS_DEG 15.f

static motion_orient_t myOrientation = ORIENT_NA;

/*!
 * @brief Initialize orientation detection
 *
 * @param None
 *
 * @return None
 */
void orientInit(void)
{

  myOrientation = ORIENT_NA;

}


/*!
 * @brief Process the orientation detection
 *
 * @param[in] gVal accelerometer reading in g
 *
 * @return Orientation
 */
motion_orient_t processOrient(float_xyzt_t gVal)
{

  float gMag = sqrt(gVal.u.x * gVal.u.x + gVal.u.y * gVal.u.y + gVal.u.z * gVal.u.z);
  float xi = acos(gVal.u.z / gMag) * Rad2Deg;
  float xyMag = sqrt(gVal.u.x * gVal.u.x + gVal.u.y * gVal.u.y);
  float psi = acos(gVal.u.x / xyMag) * Rad2Deg;
  float thZ, thX;
  
  switch(myOrientation){
  case ORIENT_NA:
    thZ = SWITCH_THRESHOLD_DEG;
    thX = SWITCH_THRESHOLD_DEG;
    break;
  case ORIENT_Z_POS:
  case ORIENT_Z_NEG: //Check for switch
    thZ = SWITCH_THRESHOLD_DEG + SWITCH_HYSTERESIS_DEG;
    thX = SWITCH_THRESHOLD_DEG;
    break;
  case ORIENT_Y_POS:
  case ORIENT_Y_NEG: //Check for switch
    thZ = SWITCH_THRESHOLD_DEG - SWITCH_HYSTERESIS_DEG;
    thX = SWITCH_THRESHOLD_DEG - SWITCH_HYSTERESIS_DEG;
    break;
  default: //X+/X-
    thZ = SWITCH_THRESHOLD_DEG - SWITCH_HYSTERESIS_DEG;
    thX = SWITCH_THRESHOLD_DEG + SWITCH_HYSTERESIS_DEG;
    break;
  }

  if(xi < thZ || xi > 180 - thZ){ //Tilt
    myOrientation = gVal.u.z > 0 ? ORIENT_Z_POS : ORIENT_Z_NEG;
  }
  else{
    if(psi < thX || psi > 180 - thX){ //X
      myOrientation = gVal.u.x > 0 ? ORIENT_X_POS : ORIENT_X_NEG;
    }
    else{ //Y
      myOrientation = gVal.u.y > 0 ? ORIENT_Y_POS : ORIENT_Y_NEG;
    }
  }

  return myOrientation;
}

/*!
 * @brief Get the orientation
 *
 * @param None
 *
 * @return Orientation
 */
motion_orient_t getOrient(void)
{

  return myOrientation;

}
