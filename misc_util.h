/*
 *
 ****************************************************************************
 * Copyright (C) 2016 GlobalMEMS, Inc. <www.globalmems.com>
 * All rights reserved.
 *
 * File : misc_util.h
 *
 * Usage: Miscellaneous utility Function
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

/*! @file misc_util.h
 *  @brief Miscellaneous utility Function
 *  @author Joseph FC Tseng
 */

#ifndef __MISC_UTIL_H__
#define __MISC_UTIL_H__

#include "type_support.h"

/***** Layout pattern ********************************************************/
typedef enum {
  PAT_INVALID = 0,
  PAT1,	
  PAT2,	
  PAT3,	
  PAT4,	
  PAT5,	
  PAT6,	
  PAT7,	
  PAT8	
} GMEMS_PATNO;

/*!
 * @brief Rotate the axis according to layout pattern
 *
 * @param[in] pat Layout pattern number
 * @param[in/out] pi32Data pointer to raw int32_t data vectors
 *
 * @return 1 for Success
 * @return 0 for Error
 */
int16_t coord_rotate(const GMEMS_PATNO pat, raw_data_xyzt_t *pi32Data);

/*!
 * @brief Rotate the axis according to layout pattern
 *
 * @param[in] pat Layout pattern number
 * @param[in/out] pfData pointer to raw float data vectors
 *
 * @return 1 for Success
 * @return 0 for Error
 */
int16_t coord_rotate_f(const GMEMS_PATNO pat, float_xyzt_t *pfData); 

#endif //__MISC_UTIL_H__
