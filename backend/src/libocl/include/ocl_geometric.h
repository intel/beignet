#ifndef __OCL_GEOMETRIC_H__
#define __OCL_GEOMETRIC_H__

#include "ocl_types.h"

OVERLOADABLE float dot(float p0, float p1);
OVERLOADABLE float dot(float2 p0, float2 p1);
OVERLOADABLE float dot(float3 p0, float3 p1);
OVERLOADABLE float dot(float4 p0, float4 p1);
OVERLOADABLE float length(float x);
OVERLOADABLE float length(float2 x);
OVERLOADABLE float length(float3 x);
OVERLOADABLE float length(float4 x);
OVERLOADABLE float distance(float x, float y);
OVERLOADABLE float distance(float2 x, float2 y);
OVERLOADABLE float distance(float3 x, float3 y);
OVERLOADABLE float distance(float4 x, float4 y);
OVERLOADABLE float normalize(float x);
OVERLOADABLE float2 normalize(float2 x);
OVERLOADABLE float3 normalize(float3 x);
OVERLOADABLE float4 normalize(float4 x);

OVERLOADABLE float fast_length(float x);
OVERLOADABLE float fast_length(float2 x);
OVERLOADABLE float fast_length(float3 x);
OVERLOADABLE float fast_length(float4 x);
OVERLOADABLE float fast_distance(float x, float y);
OVERLOADABLE float fast_distance(float2 x, float2 y);
OVERLOADABLE float fast_distance(float3 x, float3 y);
OVERLOADABLE float fast_distance(float4 x, float4 y);
OVERLOADABLE float fast_normalize(float x);
OVERLOADABLE float2 fast_normalize(float2 x);
OVERLOADABLE float3 fast_normalize(float3 x);
OVERLOADABLE float4 fast_normalize(float4 x);

OVERLOADABLE float3 cross(float3 v0, float3 v1);
OVERLOADABLE float4 cross(float4 v0, float4 v1);

#endif
