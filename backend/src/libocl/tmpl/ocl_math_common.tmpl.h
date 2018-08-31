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
#ifndef __OCL_MATH_COMMON_H__
#define __OCL_MATH_COMMON_H__

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

OVERLOADABLE float log1p(float x);
OVERLOADABLE float logb(float x);
OVERLOADABLE int ilogb(float x);
OVERLOADABLE float nan(uint code);
OVERLOADABLE float sqrt(float x);
OVERLOADABLE float rsqrt(float x);
OVERLOADABLE float hypot(float x, float y);
OVERLOADABLE float nextafter(float x, float y);
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


// Half float version.
OVERLOADABLE half cospi(half x);
OVERLOADABLE half cosh(half x);
OVERLOADABLE half acos(half x);
OVERLOADABLE half acospi(half x);
OVERLOADABLE half acosh(half x);
OVERLOADABLE half sinpi(half x);
OVERLOADABLE half sinh(half x);
OVERLOADABLE half asin(half x);
OVERLOADABLE half asinpi(half x);
OVERLOADABLE half asinh(half x);
OVERLOADABLE half tanpi(half x);
OVERLOADABLE half tanh(half x);
OVERLOADABLE half atan(half x);
OVERLOADABLE half atan2(half y, half x);
OVERLOADABLE half atan2pi(half y, half x);
OVERLOADABLE half atanpi(half x);
OVERLOADABLE half atanh(half x);
OVERLOADABLE half cbrt(half x);
OVERLOADABLE half rint(half x);
OVERLOADABLE half copysign(half x, half y);
OVERLOADABLE half erf(half x);
OVERLOADABLE half erfc(half x);
OVERLOADABLE half fmod (half x, half y);
OVERLOADABLE half remainder(half x, half p);
OVERLOADABLE half ldexp(half x, int n);
OVERLOADABLE half powr(half x, half y);
OVERLOADABLE half pow(half x, half y);
//no pow, we use powr instead
OVERLOADABLE half fabs(half x);
OVERLOADABLE half trunc(half x);
OVERLOADABLE half round(half x);
OVERLOADABLE half floor(half x);
OVERLOADABLE half ceil(half x);
OVERLOADABLE half log(half x);
OVERLOADABLE half log2(half x);
OVERLOADABLE half log10(half x);
OVERLOADABLE half exp(half x);
OVERLOADABLE half exp10(half x);
OVERLOADABLE half expm1(half x);
OVERLOADABLE half fmin(half a, half b);
OVERLOADABLE half fmax(half a, half b);
OVERLOADABLE half fma(half a, half b, half c);
OVERLOADABLE half fdim(half x, half y);
OVERLOADABLE half maxmag(half x, half y);
OVERLOADABLE half minmag(half x, half y);
OVERLOADABLE half exp2(half x);
OVERLOADABLE half mad(half a, half b, half c);
OVERLOADABLE half sin(half x);
OVERLOADABLE half cos(half x);
OVERLOADABLE half tan(half x);
OVERLOADABLE half tgamma(half x);
OVERLOADABLE half lgamma(half x);
OVERLOADABLE half log1p(half x);
OVERLOADABLE half logb(half x);
OVERLOADABLE int ilogb(half x);
OVERLOADABLE half nan(ushort code);
OVERLOADABLE half pown(half x, int n);
OVERLOADABLE half rootn(half x, int n);
OVERLOADABLE half hypot(half x, half y);
OVERLOADABLE half nextafter(half x, half y);
OVERLOADABLE half sqrt(half x);
OVERLOADABLE half rsqrt(half x);


// native half
OVERLOADABLE half native_cos(half x);
OVERLOADABLE half native_divide(half x, half y);
OVERLOADABLE half native_exp(half x);
OVERLOADABLE half native_exp2(half x);
OVERLOADABLE half native_exp10(half x);
OVERLOADABLE half native_log(half x);
OVERLOADABLE half native_log2(half x);
OVERLOADABLE half native_log10(half x);
OVERLOADABLE half native_powr(half x, half y);
OVERLOADABLE half native_recip(half x);
OVERLOADABLE half native_rsqrt(half x);
OVERLOADABLE half native_sin(half x);
OVERLOADABLE half native_sqrt(half x);
OVERLOADABLE half native_tan(half x);

// half accuracy
OVERLOADABLE float half_cos(float x);
OVERLOADABLE float half_divide(float x, float y);
OVERLOADABLE float half_exp(float x);
OVERLOADABLE float half_exp2(float x);
OVERLOADABLE float half_exp10(float x);
OVERLOADABLE float half_log(float x);
OVERLOADABLE float half_log2(float x);
OVERLOADABLE float half_log10(float x);
OVERLOADABLE float half_powr(float x, float y);
OVERLOADABLE float half_recip(float x);
OVERLOADABLE float half_rsqrt(float x);
OVERLOADABLE float half_sin(float x);
OVERLOADABLE float half_sqrt(float x);
OVERLOADABLE float half_tan(float x);


OVERLOADABLE double acos(double x);
OVERLOADABLE double acospi(double x);
OVERLOADABLE double acosh(double x);
OVERLOADABLE double asin(double x);
OVERLOADABLE double asinpi(double x);
OVERLOADABLE double asinh(double x);
OVERLOADABLE double atan(double x);
OVERLOADABLE double atan2(double x, double y);
OVERLOADABLE double atanpi(double x);
OVERLOADABLE double atan2pi(double x, double y);
OVERLOADABLE double atanh(double x);
OVERLOADABLE double exp(double x);
OVERLOADABLE double expm1(double x);
OVERLOADABLE double exp2(double x);
OVERLOADABLE double exp10(double x);
OVERLOADABLE double erf(double x);
OVERLOADABLE double erfc(double x);
OVERLOADABLE double cbrt(double x);
OVERLOADABLE double ceil(double x);
OVERLOADABLE double copysign(double x, double y);
OVERLOADABLE double cos(double x);
OVERLOADABLE double cosh(double x);
OVERLOADABLE double cospi(double x);
OVERLOADABLE double fabs(double x);
OVERLOADABLE double fdim(double x, double y);
OVERLOADABLE double floor(double x);
OVERLOADABLE double fmax(double a, double b);
OVERLOADABLE double fmin(double a, double b);
OVERLOADABLE double fmod (double x, double y);
OVERLOADABLE double fma(double x, double y, double z);
OVERLOADABLE double maxmag(double x, double y);
OVERLOADABLE double minmag(double x, double y);
OVERLOADABLE double hypot(double x, double y);
OVERLOADABLE double ldexp(double x, int n);
OVERLOADABLE double log(double x);
OVERLOADABLE double log2(double x);
OVERLOADABLE double log10(double x);
OVERLOADABLE double log1p(double x);
OVERLOADABLE double logb(double x);
OVERLOADABLE int ilogb(double x);
OVERLOADABLE double lgamma(double x);
OVERLOADABLE double mad(double a, double b, double c);
OVERLOADABLE double nan(ulong code);
OVERLOADABLE double nextafter(double x, double y);
OVERLOADABLE double pow(double x, double y);
OVERLOADABLE double pown(double x, int n);
OVERLOADABLE double powr(double x, double y);
OVERLOADABLE double remainder(double x, double p);
OVERLOADABLE double rint(double x);
OVERLOADABLE double round(double x);
OVERLOADABLE double rootn(double x, int n);
OVERLOADABLE double rsqrt(double x);
OVERLOADABLE double sin(double x);
OVERLOADABLE double sinh(double x);
OVERLOADABLE double sinpi(double x);
OVERLOADABLE double sqrt(double x);
OVERLOADABLE double tan(double x);
OVERLOADABLE double tanh(double x);
OVERLOADABLE double tanpi(double x);
OVERLOADABLE double tgamma(double x);
OVERLOADABLE double trunc(double x);



