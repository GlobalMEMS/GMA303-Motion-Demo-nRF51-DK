/*
 *
 ****************************************************************************
 * Copyright (C) 2016 GlobalMEMS, Inc. <www.globalmems.com>
 * All rights reserved.
 *
 * File : motion_sleep_cycle.h
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

#ifndef __MOTION_SLEEP_CYCLE_H__
#define __MOTION_SLEEP_CYCLE_H__

/*!
 * For NREM sleep cycles (S1, S2, and S3) description, see
 *  https://en.wikipedia.org/wiki/Non-rapid_eye_movement_sleep
 *
 * For REM sleep cycle description, see 
 *  https://en.wikipedia.org/wiki/Rapid_eye_movement_sleep 
 */
typedef enum {
  MOTION_SLEEP_CYCLE_WAKE,
  MOTION_SLEEP_CYCLE_S1, // somnolence or drowsy sleep stage
  MOTION_SLEEP_CYCLE_REM,// Rapid-Eye-Movement stage
  MOTION_SLEEP_CYCLE_S2, // "sleep spindle stage
  MOTION_SLEEP_CYCLE_S3, //Deep or slow-wave sleep stage
  MOTION_SLEEP_CYCLE_NONE
} motion_sleep_cycle_t;

/*!
 * @brief Initialize the sleep cycle monitor
 *
 * @param None
 *
 * @return None
 */
void sleepCycleInit(void);

/*!
 * @brief Process the sleep cycle monitor
 *
 * @param[in] gVal accelerometer reading in g
 *
 * @return current sleep cycle
 */
motion_sleep_cycle_t processSleepCycle(float_xyzt_t gVal);

#endif //__MOTION_SLEEP_CYCLE_H__
