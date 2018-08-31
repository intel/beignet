/*
 * Copyright © 2012 - 2014 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __OCL_MATH_20_H__
#define __OCL_MATH_20_H__

#include "ocl_types.h"
#include "ocl_math_common.h"

OVERLOADABLE float lgamma_r(float x, int *signgamp);
OVERLOADABLE float sincos(float x, float *cosval);
OVERLOADABLE float modf(float x, float *i);
OVERLOADABLE float frexp(float x, int *exp);
OVERLOADABLE float fract(float x, float *p);
OVERLOADABLE float remquo(float x, float y, int *quo);

OVERLOADABLE half lgamma_r(half x, int *signgamp);
OVERLOADABLE half sincos(half x, half *cosval);
OVERLOADABLE half frexp(half x, int *exp);
OVERLOADABLE half modf(half x, half *i);
OVERLOADABLE half fract(half x, half *p);
OVERLOADABLE half remquo(half x, half y, int *quo);

//------- double -----------
OVERLOADABLE double fract(double x, double *p);
OVERLOADABLE double frexp(double x, int *exp);
OVERLOADABLE double lgamma_r(double x, int *signgamp);
OVERLOADABLE double modf(double x, double *i);
OVERLOADABLE double remquo(double x, double y, int *quo);
OVERLOADABLE double sincos(double x, double *cosval);

