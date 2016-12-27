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

#ifndef __MOTION_ORIENTATION_H__
#define __MOTION_ORIENTATION_H__

#include "type_support.h"

typedef enum {
  ORIENT_NA,
  ORIENT_X_POS, ORIENT_X_NEG,
  ORIENT_Y_POS, ORIENT_Y_NEG,
  ORIENT_Z_POS, ORIENT_Z_NEG
} motion_orient_t;

/*!
 * @brief Initialize orientation detection
 *
 * @param None
 *
 * @return None
 */
void orientInit(void);


/*!
 * @brief Process the orientation detection
 *
 * @param[in] gVal accelerometer reading in g
 *
 * @return Orientation
 */
motion_orient_t processOrient(float_xyzt_t gVal);

/*!
 * @brief Get the orientation
 *
 * @param None
 *
 * @return Orientation
 */
motion_orient_t getOrient(void);

#endif //__MOTION_ORIENTATION_H__
