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
#ifndef __MOTION_PEDO_H__
#define __MOTION_PEDO_H__

#include "type_support.h"

extern void PEDO_InitAlgo(unsigned char);
extern short PEDO_ProcessAccelarationData(short, short, short);
extern unsigned long PEDO_GetStepCount(void);
extern unsigned char PEDO_GetActivity(void);
extern void PEDO_ResetAlgo(void);

/*!
 * @brief Initialize the pedometer
 *
 * @return None
 */
void pedoInit(void);

/*!
 * @brief Set the pedometer parameters
 *
 * @param[in] ht_m Height in meter
 * @param[in] wt_kg Weight in kg
 *
 * @return None
 */
void pedoSetParam(float ht_m, float wt_kg);

/*!
 * @brief Reset the pedomters, step and calories will reset to 0
 *
 * @param None
 *
 * @return None
 */
void pedoReset(void);

/*!
 * @brief Process the pedometer
 *
 * @param[in] gVal accelerometer reading in g
 *
 * @return Pedometer steps
 */
uint32_t processPedo(float_xyzt_t gVal);

/*!
 * @brief Get the pedometer steps
 *
 * @param None
 *
 * @return Pedometer steps
 */
uint32_t getPedoStep(void);

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
uint8_t getPedoActivity(void);

/*!
 * @brief Get the pedometer calories
 *
 * @param None
 *
 * @return Pedometer calories
 */
float getPedoCalorie(void);


#endif //__MOTION_PEDO_H__
