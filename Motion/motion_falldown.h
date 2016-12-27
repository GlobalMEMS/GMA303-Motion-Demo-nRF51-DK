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

#ifndef __MOTION_FALLDOWN_H__
#define __MOTION_FALLDOWN_H__

#include "type_support.h"

/*!
 * @brief Initialize the fall down detection
 *
 * @param None
 *
 * @return None
 */
void fallDownInit(void);

/*!
 * @brief Process the fall down detection
 *
 * @param[in] gVal accelerometer reading in g
 *
 * @return fall down flag
 *         0: not detected
 *         1: fall down detected
 */
int32_t processFallDown(float_xyzt_t gVal);

#endif //__MOTION_FALLDOWN_H__
