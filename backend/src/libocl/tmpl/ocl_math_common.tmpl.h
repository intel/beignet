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
OVERLOADABLE double remainder(double x, double p);
OVERLOADABLE double rint(double x);
OVERLOADABLE double round(double x);
OVERLOADABLE double sin(double x);
OVERLOADABLE double sinh(double x);
OVERLOADABLE double sinpi(double x);
OVERLOADABLE double sqrt(double x);
OVERLOADABLE double tan(double x);
OVERLOADABLE double tanh(double x);
OVERLOADABLE double tanpi(double x);
OVERLOADABLE double trunc(double x);



