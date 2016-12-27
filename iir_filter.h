/*
 *
 ****************************************************************************
 * Copyright (C) 2016 GlobalMEMS, Inc. <www.globalmems.com>
 * All rights reserved.
 *
 * File : iir_filter.h
 *
 * Usage: IIR filter
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

#ifndef __IIR_FILTER_H__
#define __IIR_FILTER_H__

typedef struct{

  int32_t isFirstX;
  int32_t isFirstY;
  int32_t dof;
  int32_t lenCoeffA;
  int32_t lenCoeffB;
  float *histX;  //dof*(lenCoeffB-1), {x_n-1, x_n-2, ..., x_n-(lenCoeffB-1)}
  float *histY;  //dof*lenCoeffA, {y_n-1, y_n-2, ..., y_n-(lenCoeffA)}
  float *coeffA; //{a1, a2, a3, ..., a(lenCoeffA)}
  float *coeffB; //{b0, b1, b2, ..., b(lenCoeffB-1)}


} iir_filter_param_t;

/*!
 * @brief Intialize IIR filter
 *
 * @param pParam Pointer to the IIR filter struct
 *
 * @return None
 */
void iirFilterInit(iir_filter_param_t *pParam);

/*!
 * @brief Filtering data
 *
 * @param pData_in data input to the filter
 * @param pData_out data output form the filter
 * @param pParam IIR filter parameter
 *
 * @return None
 */
void filterData(float *pData_in, float *pData_out, iir_filter_param_t *pParam);

#endif //__IIR_FILTER_H__
