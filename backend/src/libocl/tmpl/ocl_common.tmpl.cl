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
#include "ocl_common.h"
#include "ocl_float.h"
#include "ocl_relational.h"

/////////////////////////////////////////////////////////////////////////////
// Common Functions
/////////////////////////////////////////////////////////////////////////////
PURE CONST float __gen_ocl_fmax(float a, float b);
PURE CONST float __gen_ocl_fmin(float a, float b);

OVERLOADABLE float step(float edge, float x) {
  return x < edge ? 0.0 : 1.0;
}

OVERLOADABLE float max(float a, float b) {
  return __gen_ocl_fmax(a, b);
}
OVERLOADABLE float min(float a, float b) {
  return __gen_ocl_fmin(a, b);
}
OVERLOADABLE float mix(float x, float y, float a) {
  return x + (y-x)*a;
}
OVERLOADABLE float clamp(float v, float l, float u) {
  return max(min(v, u), l);
}


OVERLOADABLE float degrees(float radians) {
  return (180 / M_PI_F) * radians;
}
OVERLOADABLE float radians(float degrees) {
  return (M_PI_F / 180) * degrees;
}

OVERLOADABLE float smoothstep(float e0, float e1, float x) {
  x = clamp((x - e0) / (e1 - e0), 0.f, 1.f);
  return x * x * (3 - 2 * x);
}

OVERLOADABLE float sign(float x) {
// TODO: the best form of implementation is below,
//      But I find it hard to implement in Beignet now,
//      So I would put it in the TODO list.

//      cmp.ne.f0  null    x:f  0.0:f
//      and        ret:ud  x:ud 0x80000000:ud
//(+f0) or         ret:ud  ret:ud 0x3f800000:ud
//      cmp.ne.f0  null    x:f  x:f
//(+f0) mov        ret:f   0.0f

  union {float f; unsigned u;} ieee;
  ieee.f = x;
  unsigned k = ieee.u;
  float r = (k&0x80000000) ? -1.0f : 1.0f;
  // differentiate +0.0f -0.0f
  float s = 0.0f * r;
  s = (x == 0.0f) ? s : r;
  return isnan(x) ? 0.0f : s;
}
