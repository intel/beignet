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
#ifndef __OCL_MATH_H__
#define __OCL_MATH_H__

#include "ocl_types.h"
#include "ocl_math_common.h"


OVERLOADABLE float lgamma_r(float x, global int *signgamp);
OVERLOADABLE float lgamma_r(float x, local int *signgamp);
OVERLOADABLE float lgamma_r(float x, private int *signgamp);
OVERLOADABLE float sincos(float x, global float *cosval);
OVERLOADABLE float sincos(float x, local float *cosval);
OVERLOADABLE float sincos(float x, private float *cosval);
OVERLOADABLE float frexp(float x, global int *exp);
OVERLOADABLE float frexp(float x, local int *exp);
OVERLOADABLE float frexp(float x, private int *exp);
OVERLOADABLE float modf(float x, global float *i);
OVERLOADABLE float modf(float x, local float *i);
OVERLOADABLE float modf(float x, private float *i);
OVERLOADABLE float fract(float x, global float *p);
OVERLOADABLE float fract(float x, local float *p);
OVERLOADABLE float fract(float x, private float *p);
OVERLOADABLE float remquo(float x, float y, global int *quo);
OVERLOADABLE float remquo(float x, float y, local int *quo);
OVERLOADABLE float remquo(float x, float y, private int *quo);


OVERLOADABLE half lgamma_r(half x, global int *signgamp);
OVERLOADABLE half lgamma_r(half x, local int *signgamp);
OVERLOADABLE half lgamma_r(half x, private int *signgamp);
OVERLOADABLE half sincos(half x, global half *cosval);
OVERLOADABLE half sincos(half x, local half *cosval);
OVERLOADABLE half sincos(half x, private half *cosval);
OVERLOADABLE half frexp(half x, global int *exp);
OVERLOADABLE half frexp(half x, local int *exp);
OVERLOADABLE half frexp(half x, private int *exp);
OVERLOADABLE half modf(half x, global half *i);
OVERLOADABLE half modf(half x, local half *i);
OVERLOADABLE half modf(half x, private half *i);
OVERLOADABLE half fract(half x, global half *p);
OVERLOADABLE half fract(half x, local half *p);
OVERLOADABLE half fract(half x, private half *p);
OVERLOADABLE half remquo(half x, half y, global int *quo);
OVERLOADABLE half remquo(half x, half y, local int *quo);
OVERLOADABLE half remquo(half x, half y, private int *quo);



//------- double -----------
OVERLOADABLE double fract(double x, global double *p);
OVERLOADABLE double fract(double x, local double *p);
OVERLOADABLE double fract(double x, private double *p);
OVERLOADABLE double frexp(double x, global int *exp);
OVERLOADABLE double frexp(double x, local int *exp);
OVERLOADABLE double frexp(double x, private int *exp);
OVERLOADABLE double lgamma_r(double x, global int *signgamp);
OVERLOADABLE double lgamma_r(double x, local int *signgamp);
OVERLOADABLE double lgamma_r(double x, private int *signgamp);
OVERLOADABLE double modf(double x, global double *i);
OVERLOADABLE double modf(double x, local double *i);
OVERLOADABLE double modf(double x, private double *i);
OVERLOADABLE double remquo(double x, double y, global int *quo);
OVERLOADABLE double remquo(double x, double y, local int *quo);
OVERLOADABLE double remquo(double x, double y, private int *quo);
OVERLOADABLE double sincos(double x, global double *cosval);
OVERLOADABLE double sincos(double x, local double *cosval);
OVERLOADABLE double sincos(double x, private double *cosval);


