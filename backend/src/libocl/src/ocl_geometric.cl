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
#if (__OPENCL_C_VERSION__ >= 200)
#include "ocl_math_20.h"
#else
#include "ocl_math.h"
#endif
#include "ocl_float.h"
#include "ocl_printf.h"
#include "ocl_workitem.h"

CONST float __gen_ocl_fabs(float x) __asm("llvm.fabs" ".f32");

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
OVERLOADABLE half dot(half p0, half p1) {
  return p0 * p1;
}
OVERLOADABLE half dot(half2 p0, half2 p1) {
  return p0.x * p1.x + p0.y * p1.y;
}
OVERLOADABLE half dot(half3 p0, half3 p1) {
  return p0.x * p1.x + p0.y * p1.y + p0.z * p1.z;
}
OVERLOADABLE half dot(half4 p0, half4 p1) {
  return p0.x * p1.x + p0.y * p1.y + p0.z * p1.z + p0.w * p1.w;
}
OVERLOADABLE float length(float x) { return __gen_ocl_fabs(x); }

#define BODY \
  m = m==0.0f ? 1.0f : m; \
  m = isinf(m) ? 1.0f : m; \
  x = x/m; \
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
  float m = length(x);
  m = m == 0.0f ? 1.0f : m;
  return x / m;
}
OVERLOADABLE float2 normalize(float2 x) {
  float m = max(__gen_ocl_fabs(x.s0), __gen_ocl_fabs(x.s1));
  m = m == 0.0f ? 1.0f : m;
  m = isinf(m) ? 1.0f : m;
  x = x/m;
  return x*rsqrt(dot(x,x));
}
OVERLOADABLE float3 normalize(float3 x) {
 float m = max(__gen_ocl_fabs(x.s0), max(__gen_ocl_fabs(x.s1), __gen_ocl_fabs(x.s2)));
 m = m == 0.0f ? 1.0f : m;
 m = isinf(m) ? 1.0f : m;
 x = x/m;
 return x*rsqrt(dot(x,x));
}
OVERLOADABLE float4 normalize(float4 x) {
 float m = max(__gen_ocl_fabs(x.s0), max(__gen_ocl_fabs(x.s1), max(__gen_ocl_fabs(x.s2), __gen_ocl_fabs(x.s3))));
 m = m == 0.0f ? 1.0f : m;
 m = isinf(m) ? 1.0f : m;
 x = x/m;
 return x*rsqrt(dot(x,x));
}

OVERLOADABLE float fast_length(float x) { return __gen_ocl_fabs(x); }
OVERLOADABLE float fast_length(float2 x) { return sqrt(dot(x,x)); }
OVERLOADABLE float fast_length(float3 x) { return sqrt(dot(x,x)); }
OVERLOADABLE float fast_length(float4 x) { return sqrt(dot(x,x)); }
OVERLOADABLE float fast_distance(float x, float y) { return fast_length(x-y); }
OVERLOADABLE float fast_distance(float2 x, float2 y) { return fast_length(x-y); }
OVERLOADABLE float fast_distance(float3 x, float3 y) { return fast_length(x-y); }
OVERLOADABLE float fast_distance(float4 x, float4 y) { return fast_length(x-y); }
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

OVERLOADABLE double dot(double p0, double p1)
{
	return p0*p1;
}

OVERLOADABLE double dot(double2 p0, double2 p1)
{
	return p0.x*p1.x + p0.y*p1.y;
}

OVERLOADABLE double dot(double3 p0, double3 p1)
{
	return p0.x*p1.x + p0.y*p1.y + p0.z*p1.z;
}

OVERLOADABLE double dot(double4 p0, double4 p1)
{
	return p0.x * p1.x + p0.y * p1.y + p0.z * p1.z + p0.w * p1.w;
}

#define BODY \
  m = m==0.0 ? 1.0 : m; \
  m = isinf(m) ? 1.0 : m; \
  x = x/m; \
  return m * sqrt(dot(x,x));
OVERLOADABLE double length(double x)
{
	return fabs(x);
}

OVERLOADABLE double length(double2 x)
{
	double m = max(fabs(x.s0) , fabs(x.s1));
	BODY;
}

OVERLOADABLE double length(double3 x)
{
	double m = max(fabs(x.s0), max(fabs(x.s1), fabs(x.s2)));
	BODY;
}

OVERLOADABLE double length(double4 x)
{
	double m = max(fabs(x.s0), max(fabs(x.s1), max(fabs(x.s2), fabs(x.s3))));
	BODY;
}
#undef BODY

OVERLOADABLE double distance(double x, double y)
{
	return length(x-y);
}

OVERLOADABLE double distance(double2 x, double2 y)
{
	return length(x-y);
}

OVERLOADABLE double distance(double3 x, double3 y)
{
	return length(x-y);
}

OVERLOADABLE double distance(double4 x, double4 y)
{
	return length(x-y);
}

OVERLOADABLE double normalize(double x)
{
	double m = length(x);
	m = m == 0.0 ? 1.0 : m;
	return x / m;
}

OVERLOADABLE double2 normalize(double2 x)
{
	double m = max(fabs(x.s0) , fabs(x.s1));
         m = m==0.0 ? 1.0 : m;
         m = isinf(m) ? 1.0 : m;
         x = x/m;
         return x*rsqrt(dot(x,x));
}

OVERLOADABLE double3 normalize(double3 x)
{
	double m = max(fabs(x.s0), max(fabs(x.s1), fabs(x.s2)));
	m = m==0.0 ? 1.0 : m;
	m = isinf(m) ? 1.0 : m;
	x = x/m;
	return x*rsqrt(dot(x,x));
}

OVERLOADABLE double4 normalize(double4 x)
{
	double m = max(fabs(x.s0), max(fabs(x.s1), max(fabs(x.s2), fabs(x.s3))));
	m = m==0.0 ? 1.0 : m;
	m = isinf(m) ? 1.0 : m;
	x = x/m;
	return x*rsqrt(dot(x,x));
}

OVERLOADABLE double3 cross(double3 v0, double3 v1)
{
	return v0.yzx*v1.zxy-v0.zxy*v1.yzx;
}

OVERLOADABLE double4 cross(double4 v0, double4 v1)
{
	return (double4)(v0.yzx*v1.zxy-v0.zxy*v1.yzx, 0.0);
}

