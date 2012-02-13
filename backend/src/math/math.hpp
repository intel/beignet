/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

//////////////////////////////////////////////////////////////////////////////////////////
// Part of this file is taken from the Apache licensed Intel Embree project here:       //
// http://software.intel.com/en-us/articles/embree-photo-realistic-ray-tracing-kernels/ //
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef __GBE_MATH_HPP__
#define __GBE_MATH_HPP__

#include "sys/platform.hpp"

#include <cmath>
#include <cfloat>

namespace gbe
{
#if defined(__WIN32__)
  #undef min
  #undef max
#if defined(__MSVC__)
  INLINE bool finite (float x) {return _finite(x) != 0;}
#endif
#endif

  INLINE float sign (float x) {return x<0?-1.0f:1.0f;}
  INLINE float sqr  (float x) {return x*x;}
  INLINE float rcp  (float x) {return 1.0f/x;}
  INLINE float rsqrt(float x) {return 1.0f/::sqrtf(x);}
  INLINE float abs  (float x) {return ::fabsf(x);}
  INLINE float acos (float x) {return ::acosf (x);}
  INLINE float asin (float x) {return ::asinf (x);}
  INLINE float atan (float x) {return ::atanf (x);}
  INLINE float cos  (float x) {return ::cosf  (x);}
  INLINE float cosh (float x) {return ::coshf (x);}
  INLINE float exp  (float x) {return ::expf  (x);}
  INLINE float log  (float x) {return ::logf  (x);}
  INLINE float log2 (float x) {return ::log(x) / 0.69314718055994530941723212145818f;}
  INLINE float log10(float x) {return ::log10f(x);}
  INLINE float sin  (float x) {return ::sinf  (x);}
  INLINE float sinh (float x) {return ::sinhf (x);}
  INLINE float sqrt (float x) {return ::sqrtf (x);}
  INLINE float tan  (float x) {return ::tanf  (x);}
  INLINE float tanh (float x) {return ::tanhf (x);}
  INLINE float floor(float x) {return ::floorf (x);}
  INLINE float ceil (float x) {return ::ceilf (x);}
  INLINE float atan2(float y, float x) {return ::atan2f(y, x);}
  INLINE float fmod (float x, float y) {return ::fmodf (x, y);}
  INLINE float pow  (float x, float y) {return ::powf  (x, y);}

  INLINE double abs  (double x) {return ::fabs(x);}
  INLINE double sign (double x) {return x<0?-1.0:1.0;}
  INLINE double acos (double x) {return ::acos (x);}
  INLINE double asin (double x) {return ::asin (x);}
  INLINE double atan (double x) {return ::atan (x);}
  INLINE double cos  (double x) {return ::cos  (x);}
  INLINE double cosh (double x) {return ::cosh (x);}
  INLINE double exp  (double x) {return ::exp  (x);}
  INLINE double log  (double x) {return ::log  (x);}
  INLINE double log2 (double x) {return ::log(x) / 0.69314718055994530941723212145818;}
  INLINE double log10(double x) {return ::log10(x);}
  INLINE double rcp  (double x) {return 1.0/x;}
  INLINE double rsqrt(double x) {return 1.0/::sqrt(x);}
  INLINE double sin  (double x) {return ::sin  (x);}
  INLINE double sinh (double x) {return ::sinh (x);}
  INLINE double sqr  (double x) {return x*x;}
  INLINE double sqrt (double x) {return ::sqrt (x);}
  INLINE double tan  (double x) {return ::tan  (x);}
  INLINE double tanh (double x) {return ::tanh (x);}
  INLINE double floor(double x) {return ::floor (x);}
  INLINE double ceil (double x) {return ::ceil (x);}
  INLINE double atan2(double y, double x) {return ::atan2(y, x);}
  INLINE double fmod (double x, double y) {return ::fmod (x, y);}
  INLINE double pow  (double x, double y) {return ::pow  (x, y);}

#define DECL template <typename T> INLINE

  DECL T max (T a, T b) {return a<b? b:a;}
  DECL T min (T a, T b) {return a<b? a:b;}
  DECL T min (T a, T b, T c) {return min(min(a,b),c);}
  DECL T max (T a, T b, T c) {return max(max(a,b),c);}
  DECL T max (T a, T b, T c, T d) {return max(max(a,b),max(c,d));}
  DECL T min (T a, T b, T c, T d) {return min(min(a,b),min(c,d));}
  DECL T min (T a, T b, T c, T d, T e) {return min(min(min(a,b),min(c,d)),e);}
  DECL T max (T a, T b, T c, T d, T e) {return max(max(max(a,b),max(c,d)),e);}
  DECL T clamp (T x, T lower = T(zero), T upper = T(one)) {return max(lower, min(x,upper));}
  DECL T deg2rad (T x) {return x * T(1.74532925199432957692e-2f);}
  DECL T rad2deg (T x) {return x * T(5.72957795130823208768e1f);}
  DECL T sin2cos (T x) {return sqrt(max(T(zero),T(one)-x*x));}
  DECL T cos2sin (T x) {return sin2cos(x);}

#undef DECL
}

#endif /* __GBE_MATH_HPP__ */

