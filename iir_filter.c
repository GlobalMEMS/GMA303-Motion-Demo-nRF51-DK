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

#include <stdint.h>
#include "iir_filter.h"

/*!
 * @brief Intialize IIR filter
 *
 * @param pParam Pointer to the IIR filter struct
 *
 * @return None
 */
void iirFilterInit(iir_filter_param_t *pParam)
{

  int32_t i;

  pParam->isFirstY = pParam->lenCoeffA;
  pParam->isFirstX = pParam->lenCoeffB - 1;

  //Initialize the history
  for(i = 0; i < pParam->dof * (pParam->lenCoeffB - 1); ++i)
    pParam->histX[i] = 0.0;

  for(i = 0; i < pParam->dof * pParam->lenCoeffA; ++i)
    pParam->histY[i] = 0.0;
}

/*!
 * @brief Filtering data
 *
 * @param pData_in data input to the filter
 * @param pData_out data output form the filter
 * @param pParam IIR filter parameter
 *
 * @return None
 */
void filterData(float *X_n, float *Y_n, iir_filter_param_t *pParam)
{

  int32_t i, j;
  int32_t lenA = pParam->lenCoeffA;
  int32_t lenB = pParam->lenCoeffB;
  int32_t colhY = lenA;
  int32_t colhX = lenB - 1;

  //Initialize the first colhX datas to histX
  if(pParam->isFirstX > 0){

    for(i = 0; i < pParam->dof; ++i)
      pParam->histX[i * colhX] = X_n[i];

    pParam->isFirstX -= 1;
  }

  //Initialize the first colhY datas to histY
  if(pParam->isFirstY > 0){

    for(i = 0; i < pParam->dof; ++i)
      pParam->histY[i * colhY] = X_n[i];

    pParam->isFirstY -= 1;
  }

  // Data filtering
  for(i = 0; i < pParam->dof; ++i){

    //
    // y_n = b_0 * x_n +
    //       b_1 * x_n-1 + b_2 * x_n-2 + .... +
    //       a_1 * y_n-1 + a_2 * y_n-2 +....
    //
    Y_n[i] = pParam->coeffB[0] * X_n[i];

    for(j = 1; j < lenB; ++j)
      Y_n[i] += pParam->coeffB[j] * pParam->histX[i * colhX + j - 1];

    for(j = 0; j < lenA; ++j)
      Y_n[i] += pParam->coeffA[j] * pParam->histY[i * colhY + j];

    //Update the history value
    for(j = 1; j < colhX; ++j)
      pParam->histX[i * colhX + j] = pParam->histX[i * colhX + j - 1];
    if(colhX > 0) pParam->histX[i * colhX] = X_n[i];

    for(j = 1; j < colhY; ++j)
      pParam->histY[i * colhY + j] = pParam->histY[i * colhY + j - 1];
    if(colhY > 0) pParam->histY[i * colhY] = Y_n[i];
  }
}
