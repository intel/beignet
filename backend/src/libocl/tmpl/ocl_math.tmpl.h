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

OVERLOADABLE float cospi(float x);
OVERLOADABLE float cosh(float x);
OVERLOADABLE float acos(float x);
OVERLOADABLE float acospi(float x);
OVERLOADABLE float acosh(float x);
OVERLOADABLE float sinpi(float x);
OVERLOADABLE float sinh(float x);
OVERLOADABLE float asin(float x);
OVERLOADABLE float asinpi(float x);
OVERLOADABLE float asinh(float x);
OVERLOADABLE float tanpi(float x);
OVERLOADABLE float tanh(float x);
OVERLOADABLE float atan(float x);
OVERLOADABLE float atan2(float y, float x);
OVERLOADABLE float atan2pi(float y, float x);
OVERLOADABLE float atanpi(float x);
OVERLOADABLE float atanh(float x);
OVERLOADABLE float cbrt(float x);
OVERLOADABLE float rint(float x);
OVERLOADABLE float copysign(float x, float y);
OVERLOADABLE float erf(float x);
OVERLOADABLE float erfc(float x);
OVERLOADABLE float fmod (float x, float y);
OVERLOADABLE float remainder(float x, float p);
OVERLOADABLE float ldexp(float x, int n);
OVERLOADABLE float powr(float x, float y);
OVERLOADABLE float pow(float x, float y);
//no pow, we use powr instead
OVERLOADABLE float fabs(float x);
OVERLOADABLE float trunc(float x);
OVERLOADABLE float round(float x);
OVERLOADABLE float floor(float x);
OVERLOADABLE float ceil(float x);
OVERLOADABLE float log(float x);
OVERLOADABLE float log2(float x);
OVERLOADABLE float log10(float x);
OVERLOADABLE float exp(float x);
OVERLOADABLE float exp10(float x);
OVERLOADABLE float expm1(float x);
OVERLOADABLE float fmin(float a, float b);
OVERLOADABLE float fmax(float a, float b);
OVERLOADABLE float fma(float a, float b, float c);
OVERLOADABLE float fdim(float x, float y);
OVERLOADABLE float maxmag(float x, float y);
OVERLOADABLE float minmag(float x, float y);
OVERLOADABLE float exp2(float x);
OVERLOADABLE float mad(float a, float b, float c);
OVERLOADABLE float sin(float x);
OVERLOADABLE float cos(float x);
OVERLOADABLE float tan(float x);
OVERLOADABLE float tgamma(float x);
OVERLOADABLE float lgamma(float x);
OVERLOADABLE float lgamma_r(float x, global int *signgamp);
OVERLOADABLE float lgamma_r(float x, local int *signgamp);
OVERLOADABLE float lgamma_r(float x, private int *signgamp);
OVERLOADABLE float log1p(float x);
OVERLOADABLE float logb(float x);
OVERLOADABLE int ilogb(float x);
OVERLOADABLE float nan(uint code);
OVERLOADABLE float sincos(float x, global float *cosval);
OVERLOADABLE float sincos(float x, local float *cosval);
OVERLOADABLE float sincos(float x, private float *cosval);
OVERLOADABLE float sqrt(float x);
OVERLOADABLE float rsqrt(float x);
OVERLOADABLE float frexp(float x, global int *exp);
OVERLOADABLE float frexp(float x, local int *exp);
OVERLOADABLE float frexp(float x, private int *exp);
OVERLOADABLE float nextafter(float x, float y);
OVERLOADABLE float modf(float x, global float *i);
OVERLOADABLE float modf(float x, local float *i);
OVERLOADABLE float modf(float x, private float *i);
OVERLOADABLE float hypot(float x, float y);
OVERLOADABLE float fract(float x, global float *p);
OVERLOADABLE float fract(float x, local float *p);
OVERLOADABLE float fract(float x, private float *p);
OVERLOADABLE float remquo(float x, float y, global int *quo);
OVERLOADABLE float remquo(float x, float y, local int *quo);
OVERLOADABLE float remquo(float x, float y, private int *quo);
OVERLOADABLE float pown(float x, int n);
OVERLOADABLE float rootn(float x, int n);

// native
OVERLOADABLE float native_cos(float x);
OVERLOADABLE float native_divide(float x, float y);
OVERLOADABLE float native_exp(float x);
OVERLOADABLE float native_exp2(float x);
OVERLOADABLE float native_exp10(float x);
OVERLOADABLE float native_log(float x);
OVERLOADABLE float native_log2(float x);
OVERLOADABLE float native_log10(float x);
OVERLOADABLE float native_powr(float x, float y);
OVERLOADABLE float native_recip(float x);
OVERLOADABLE float native_rsqrt(float x);
OVERLOADABLE float native_sin(float x);
OVERLOADABLE float native_sqrt(float x);
OVERLOADABLE float native_tan(float x);

// half  not supported now.
