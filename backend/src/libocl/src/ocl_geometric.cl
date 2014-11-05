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
#include "ocl_geometric.h"
#include "ocl_common.h"
#include "ocl_relational.h"
#include "ocl_math.h"
#include "ocl_float.h"

PURE CONST float __gen_ocl_fabs(float x);

OVERLOADABLE float dot(float p0, float p1) {
  return p0 * p1;
}
OVERLOADABLE float dot(float2 p0, float2 p1) {
  return p0.x * p1.x + p0.y * p1.y;
}
OVERLOADABLE float dot(float3 p0, float3 p1) {
  return p0.x * p1.x + p0.y * p1.y + p0.z * p1.z;
}
OVERLOADABLE float dot(float4 p0, float4 p1) {
  return p0.x * p1.x + p0.y * p1.y + p0.z * p1.z + p0.w * p1.w;
}
OVERLOADABLE float length(float x) { return __gen_ocl_fabs(x); }

#define BODY \
  if(m == 0) \
    return 0; \
  if(isinf(m)) \
    return INFINITY; \
  if(m < 1) \
    m = 1; \
  x /= m; \
  return m * sqrt(dot(x,x));
OVERLOADABLE float length(float2 x) {
  float m = max(__gen_ocl_fabs(x.s0), __gen_ocl_fabs(x.s1));
  BODY;
}
OVERLOADABLE float length(float3 x) {
  float m = max(__gen_ocl_fabs(x.s0), max(__gen_ocl_fabs(x.s1), __gen_ocl_fabs(x.s2)));
  BODY;
}
OVERLOADABLE float length(float4 x) {
  float m = max(__gen_ocl_fabs(x.s0), max(__gen_ocl_fabs(x.s1), max(__gen_ocl_fabs(x.s2), __gen_ocl_fabs(x.s3))));
  BODY;
}
#undef BODY
OVERLOADABLE float distance(float x, float y) { return length(x-y); }
OVERLOADABLE float distance(float2 x, float2 y) { return length(x-y); }
OVERLOADABLE float distance(float3 x, float3 y) { return length(x-y); }
OVERLOADABLE float distance(float4 x, float4 y) { return length(x-y); }
OVERLOADABLE float normalize(float x) {
  union { float f; unsigned u; } u;
  u.f = x;
  if(u.u == 0)
    return 0.f;
  if(isnan(x))
    return NAN;
  return u.u < 0x7fffffff ? 1.f : -1.f;
}
OVERLOADABLE float2 normalize(float2 x) {
  float m = length(x);
  if(m == 0)
    return 0;
  return x / m;
}
OVERLOADABLE float3 normalize(float3 x) {
  float m = length(x);
  if(m == 0)
    return 0;
  return x / m;
}
OVERLOADABLE float4 normalize(float4 x) {
  float m = length(x);
  if(m == 0)
    return 0;
  return x / m;
}

OVERLOADABLE float fast_length(float x) { return __gen_ocl_fabs(x); }
OVERLOADABLE float fast_length(float2 x) { return sqrt(dot(x,x)); }
OVERLOADABLE float fast_length(float3 x) { return sqrt(dot(x,x)); }
OVERLOADABLE float fast_length(float4 x) { return sqrt(dot(x,x)); }
OVERLOADABLE float fast_distance(float x, float y) { return length(x-y); }
OVERLOADABLE float fast_distance(float2 x, float2 y) { return length(x-y); }
OVERLOADABLE float fast_distance(float3 x, float3 y) { return length(x-y); }
OVERLOADABLE float fast_distance(float4 x, float4 y) { return length(x-y); }
OVERLOADABLE float fast_normalize(float x) { return x > 0 ? 1.f : (x < 0 ? -1.f : 0.f); }
OVERLOADABLE float2 fast_normalize(float2 x) { return x * rsqrt(dot(x, x)); }
OVERLOADABLE float3 fast_normalize(float3 x) { return x * rsqrt(dot(x, x)); }
OVERLOADABLE float4 fast_normalize(float4 x) { return x * rsqrt(dot(x, x)); }

OVERLOADABLE float3 cross(float3 v0, float3 v1) {
   return v0.yzx*v1.zxy-v0.zxy*v1.yzx;
}
OVERLOADABLE float4 cross(float4 v0, float4 v1) {
   return (float4)(v0.yzx*v1.zxy-v0.zxy*v1.yzx, 0.f);
}
