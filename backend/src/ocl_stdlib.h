/* 
uint* Copyright © 2012 Intel Corporation
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

#ifndef __GEN_OCL_STDLIB_H__
#define __GEN_OCL_STDLIB_H__

#define INLINE __attribute__((always_inline)) inline
#define OVERLOADABLE __attribute__((overloadable))
#define PURE __attribute__((pure))
#define CONST __attribute__((const))

/////////////////////////////////////////////////////////////////////////////
// OpenCL basic types
/////////////////////////////////////////////////////////////////////////////
typedef unsigned int uint;
typedef unsigned int size_t;
typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef float float8 __attribute__((ext_vector_type(8)));
typedef float float16 __attribute__((ext_vector_type(16)));
typedef int int2 __attribute__((ext_vector_type(2)));
typedef int int3 __attribute__((ext_vector_type(3)));
typedef int int4 __attribute__((ext_vector_type(4)));
typedef int int8 __attribute__((ext_vector_type(8)));
typedef int int16 __attribute__((ext_vector_type(16)));
typedef unsigned int uint2 __attribute__((ext_vector_type(2)));
typedef unsigned uint3 __attribute__((ext_vector_type(3)));
typedef unsigned uint4 __attribute__((ext_vector_type(4)));
typedef unsigned uint8 __attribute__((ext_vector_type(8)));
typedef unsigned uint16 __attribute__((ext_vector_type(16)));
typedef bool bool2 __attribute__((ext_vector_type(2)));
typedef bool bool3 __attribute__((ext_vector_type(3)));
typedef bool bool4 __attribute__((ext_vector_type(4)));
typedef bool bool8 __attribute__((ext_vector_type(8)));
typedef bool bool16 __attribute__((ext_vector_type(16)));

/////////////////////////////////////////////////////////////////////////////
// OpenCL address space
/////////////////////////////////////////////////////////////////////////////
#define __private __attribute__((address_space(0)))
#define __global __attribute__((address_space(1)))
#define __constant __attribute__((address_space(2)))
//#define __local __attribute__((address_space(3)))
#define global __global
//#define local __local
#define constant __constant
#define private __private

/////////////////////////////////////////////////////////////////////////////
// Work Items functions (see 6.11.1 of OCL 1.1 spec)
/////////////////////////////////////////////////////////////////////////////
// TODO get_global_offset
// TODO get_work_dim

#define DECL_INTERNAL_WORK_ITEM_FN(NAME) \
PURE CONST unsigned int __gen_ocl_##NAME##0(void); \
PURE CONST unsigned int __gen_ocl_##NAME##1(void); \
PURE CONST unsigned int __gen_ocl_##NAME##2(void);
DECL_INTERNAL_WORK_ITEM_FN(get_group_id)
DECL_INTERNAL_WORK_ITEM_FN(get_local_id)
DECL_INTERNAL_WORK_ITEM_FN(get_local_size)
DECL_INTERNAL_WORK_ITEM_FN(get_global_size)
DECL_INTERNAL_WORK_ITEM_FN(get_num_groups)
#undef DECL_INTERNAL_WORK_ITEM_FN

#define DECL_PUBLIC_WORK_ITEM_FN(NAME) \
inline unsigned NAME(unsigned int dim) { \
  if (dim == 0) return __gen_ocl_##NAME##0(); \
  else if (dim == 1) return __gen_ocl_##NAME##1(); \
  else if (dim == 2) return __gen_ocl_##NAME##2(); \
  else return 0; \
}
DECL_PUBLIC_WORK_ITEM_FN(get_group_id)
DECL_PUBLIC_WORK_ITEM_FN(get_local_id)
DECL_PUBLIC_WORK_ITEM_FN(get_local_size)
DECL_PUBLIC_WORK_ITEM_FN(get_global_size)
DECL_PUBLIC_WORK_ITEM_FN(get_num_groups)
#undef DECL_PUBLIC_WORK_ITEM_FN

INLINE uint get_global_id(uint dim) {
  return get_local_id(dim) + get_local_size(dim) * get_group_id(dim);
}

/////////////////////////////////////////////////////////////////////////////
// Math Functions (see 6.11.2 of OCL 1.1 spec)
/////////////////////////////////////////////////////////////////////////////
PURE CONST float __gen_ocl_fabs(float x);
PURE CONST float __gen_ocl_sin(float x);
PURE CONST float __gen_ocl_cos(float x);
PURE CONST float __gen_ocl_sqrt(float x);
PURE CONST float __gen_ocl_rsqrt(float x);
PURE CONST float __gen_ocl_log(float x);
PURE CONST float __gen_ocl_pow(float x, float y);
PURE CONST float __gen_ocl_rcp(float x);
PURE CONST float __gen_ocl_rndz(float x);
PURE CONST float __gen_ocl_rnde(float x);
PURE CONST float __gen_ocl_rndu(float x);
PURE CONST float __gen_ocl_rndd(float x);
INLINE OVERLOADABLE float native_cos(float x) { return __gen_ocl_cos(x); }
INLINE OVERLOADABLE float native_sin(float x) { return __gen_ocl_sin(x); }
INLINE OVERLOADABLE float native_sqrt(float x) { return __gen_ocl_sqrt(x); }
INLINE OVERLOADABLE float native_rsqrt(float x) { return __gen_ocl_rsqrt(x); }
INLINE OVERLOADABLE float native_log2(float x) { return __gen_ocl_log(x); }
INLINE OVERLOADABLE float native_log(float x) {
  return native_log2(x) * 0.6931472002f;
}
INLINE OVERLOADABLE float native_log10(float x) {
  return native_log2(x) * 0.3010299956f;
}
INLINE OVERLOADABLE float native_powr(float x, float y) { return __gen_ocl_pow(x,y); }
INLINE OVERLOADABLE float native_recip(float x) { return __gen_ocl_rcp(x); }
INLINE OVERLOADABLE float native_tan(float x) {
  return native_sin(x) / native_cos(x);
}
#define E 2.71828182845904523536f
INLINE OVERLOADABLE float native_exp(float x) { return native_powr(E, x); }
#undef E

// TODO make them actually compliant precision-wise
#define sqrt native_sqrt // XXX work-around ptr profile: sin already defined
INLINE OVERLOADABLE float rsqrt(float x) { return native_rsqrt(x); }
INLINE OVERLOADABLE float fabs(float x) { return __gen_ocl_fabs(x); }
INLINE OVERLOADABLE float trunc(float x) { return __gen_ocl_rndz(x); }
INLINE OVERLOADABLE float round(float x) { return __gen_ocl_rnde(x); }
INLINE OVERLOADABLE float floor(float x) { return __gen_ocl_rndd(x); }
INLINE OVERLOADABLE float ceil(float x)  { return __gen_ocl_rndu(x); }
INLINE OVERLOADABLE float log(float x) { return native_log(x); }
INLINE OVERLOADABLE float log2(float x) { return native_log2(x); }
INLINE OVERLOADABLE float log10(float x) { return native_log10(x); }
INLINE OVERLOADABLE float powr(float x, float y) { return __gen_ocl_pow(x,y); }
INLINE OVERLOADABLE float exp(float x, float y) { return native_exp(x); }
INLINE OVERLOADABLE float fmod(float x, float y) { return x-y*trunc(x/y); }

// TODO use llvm intrinsics definitions
#define cos native_cos
#define sin native_sin
#define pow powr

PURE CONST OVERLOADABLE float mad(float a, float b, float c);

INLINE OVERLOADABLE uint select(uint src0, uint src1, uint cond) {
  return cond ? src1 : src0;
}
INLINE OVERLOADABLE int select(int src0, int src1, int cond) {
  return cond ? src1 : src0;
}
INLINE OVERLOADABLE float select(float src0, float src1, int cond) {
  return cond ? src1 : src0;
}

// This will be optimized out by LLVM and will output LLVM select instructions
#define DECL_SELECT4(TYPE4, TYPE, COND_TYPE4, MASK) \
INLINE OVERLOADABLE TYPE4 select(TYPE4 src0, TYPE4 src1, COND_TYPE4 cond) { \
  TYPE4 dst; \
  const TYPE x0 = src0.x; /* Fix performance issue with CLANG */ \
  const TYPE x1 = src1.x; \
  const TYPE y0 = src0.y; \
  const TYPE y1 = src1.y; \
  const TYPE z0 = src0.z; \
  const TYPE z1 = src1.z; \
  const TYPE w0 = src0.w; \
  const TYPE w1 = src1.w; \
  dst.x = (cond.x & MASK) ? x1 : x0; \
  dst.y = (cond.y & MASK) ? y1 : y0; \
  dst.z = (cond.z & MASK) ? z1 : z0; \
  dst.w = (cond.w & MASK) ? w1 : w0; \
  return dst; \
}
DECL_SELECT4(int4, int, int4, 0x80000000)
DECL_SELECT4(float4, float, int4, 0x80000000)
#undef DECL_SELECT4

/////////////////////////////////////////////////////////////////////////////
// Common Functions (see 6.11.4 of OCL 1.1 spec)
/////////////////////////////////////////////////////////////////////////////
#define DECL_MIN_MAX(TYPE) \
INLINE OVERLOADABLE TYPE max(TYPE a, TYPE b) { \
  return a > b ? a : b; \
} \
INLINE OVERLOADABLE TYPE min(TYPE a, TYPE b) { \
  return a < b ? a : b; \
}
DECL_MIN_MAX(float)
DECL_MIN_MAX(int)
DECL_MIN_MAX(short)
DECL_MIN_MAX(char)
DECL_MIN_MAX(uint)
DECL_MIN_MAX(unsigned short)
DECL_MIN_MAX(unsigned char)
#undef DECL_MIN_MAX

INLINE OVERLOADABLE float fmax(float a, float b) { return max(a,b); }
INLINE OVERLOADABLE float fmin(float a, float b) { return min(a,b); }
INLINE OVERLOADABLE float mix(float x, float y, float a) { return x + (y-x)*a;}

/////////////////////////////////////////////////////////////////////////////
// Geometric functions (see 6.11.5 of OCL 1.1 spec)
/////////////////////////////////////////////////////////////////////////////
INLINE OVERLOADABLE float dot(float2 p0, float2 p1) {
  return mad(p0.x,p1.x,p0.y*p1.y);
}
INLINE OVERLOADABLE float dot(float3 p0, float3 p1) {
  return mad(p0.x,p1.x,mad(p0.z,p1.z,p0.y*p1.y));
}
INLINE OVERLOADABLE float dot(float4 p0, float4 p1) {
  return mad(p0.x,p1.x,mad(p0.w,p1.w,mad(p0.z,p1.z,p0.y*p1.y)));
}

INLINE OVERLOADABLE float dot(float8 p0, float8 p1) {
  return mad(p0.x,p1.x,mad(p0.s7,p1.s7, mad(p0.s6,p1.s6,mad(p0.s5,p1.s5,
         mad(p0.s4,p1.s4,mad(p0.w,p1.w, mad(p0.z,p1.z,p0.y*p1.y)))))));
}
INLINE OVERLOADABLE float dot(float16 p0, float16 p1) {
  return mad(p0.sc,p1.sc,mad(p0.sd,p1.sd,mad(p0.se,p1.se,mad(p0.sf,p1.sf,
         mad(p0.s8,p1.s8,mad(p0.s9,p1.s9,mad(p0.sa,p1.sa,mad(p0.sb,p1.sb,
         mad(p0.x,p1.x,mad(p0.s7,p1.s7, mad(p0.s6,p1.s6,mad(p0.s5,p1.s5,
         mad(p0.s4,p1.s4,mad(p0.w,p1.w, mad(p0.z,p1.z,p0.y*p1.y)))))))))))))));
}

INLINE OVERLOADABLE float length(float x) { return __gen_ocl_fabs(x); }
INLINE OVERLOADABLE float length(float2 x) { return sqrt(dot(x,x)); }
INLINE OVERLOADABLE float length(float3 x) { return sqrt(dot(x,x)); }
INLINE OVERLOADABLE float length(float4 x) { return sqrt(dot(x,x)); }
INLINE OVERLOADABLE float length(float8 x) { return sqrt(dot(x,x)); }
INLINE OVERLOADABLE float length(float16 x) { return sqrt(dot(x,x)); }
INLINE OVERLOADABLE float distance(float x, float y) { return length(x-y); }
INLINE OVERLOADABLE float distance(float2 x, float2 y) { return length(x-y); }
INLINE OVERLOADABLE float distance(float3 x, float3 y) { return length(x-y); }
INLINE OVERLOADABLE float distance(float4 x, float4 y) { return length(x-y); }
INLINE OVERLOADABLE float distance(float8 x, float8 y) { return length(x-y); }
INLINE OVERLOADABLE float distance(float16 x, float16 y) { return length(x-y); }
INLINE OVERLOADABLE float normalize(float x) { return 1.f; }
INLINE OVERLOADABLE float2 normalize(float2 x) { return x * rsqrt(dot(x, x)); }
INLINE OVERLOADABLE float3 normalize(float3 x) { return x * rsqrt(dot(x, x)); }
INLINE OVERLOADABLE float4 normalize(float4 x) { return x * rsqrt(dot(x, x)); }
INLINE OVERLOADABLE float8 normalize(float8 x) { return x * rsqrt(dot(x, x)); }
INLINE OVERLOADABLE float16 normalize(float16 x) { return x * rsqrt(dot(x, x)); }

INLINE OVERLOADABLE float fast_length(float x) { return __gen_ocl_fabs(x); }
INLINE OVERLOADABLE float fast_length(float2 x) { return sqrt(dot(x,x)); }
INLINE OVERLOADABLE float fast_length(float3 x) { return sqrt(dot(x,x)); }
INLINE OVERLOADABLE float fast_length(float4 x) { return sqrt(dot(x,x)); }
INLINE OVERLOADABLE float fast_length(float8 x) { return sqrt(dot(x,x)); }
INLINE OVERLOADABLE float fast_length(float16 x) { return sqrt(dot(x,x)); }
INLINE OVERLOADABLE float fast_distance(float x, float y) { return length(x-y); }
INLINE OVERLOADABLE float fast_distance(float2 x, float2 y) { return length(x-y); }
INLINE OVERLOADABLE float fast_distance(float3 x, float3 y) { return length(x-y); }
INLINE OVERLOADABLE float fast_distance(float4 x, float4 y) { return length(x-y); }
INLINE OVERLOADABLE float fast_distance(float8 x, float8 y) { return length(x-y); }
INLINE OVERLOADABLE float fast_distance(float16 x, float16 y) { return length(x-y); }
INLINE OVERLOADABLE float fast_normalize(float x) { return 1.f; }
INLINE OVERLOADABLE float2 fast_normalize(float2 x) { return x * rsqrt(dot(x, x)); }
INLINE OVERLOADABLE float3 fast_normalize(float3 x) { return x * rsqrt(dot(x, x)); }
INLINE OVERLOADABLE float4 fast_normalize(float4 x) { return x * rsqrt(dot(x, x)); }
INLINE OVERLOADABLE float8 fast_normalize(float8 x) { return x * rsqrt(dot(x, x)); }
INLINE OVERLOADABLE float16 fast_normalize(float16 x) { return x * rsqrt(dot(x, x)); }

INLINE OVERLOADABLE float3 cross(float3 v0, float3 v1) {
   return v0.yzx*v1.zxy-v0.zxy*v1.yzx;
}
INLINE OVERLOADABLE float4 cross(float4 v0, float4 v1) {
   return (float4)(v0.yzx*v1.zxy-v0.zxy*v1.yzx, 0.f);
}

/////////////////////////////////////////////////////////////////////////////
// Vector loads and stores
/////////////////////////////////////////////////////////////////////////////

// These loads and stores will use untyped reads and writes, so we can just
// cast to vector loads / stores. Not C99 compliant BTW due to aliasing issue.
// Well we do not care, we do not activate TBAA in the compiler
#define DECL_UNTYPED_RW_SPACE_N(TYPE, DIM, SPACE) \
INLINE OVERLOADABLE TYPE##DIM vload##DIM(size_t offset, const SPACE TYPE *p) { \
  return *(SPACE TYPE##DIM *) (p + DIM * offset); \
} \
INLINE OVERLOADABLE void vstore##DIM(TYPE##DIM v, size_t offset, SPACE TYPE *p) { \
  *(SPACE TYPE##DIM *) (p + DIM * offset) = v; \
}

#define DECL_UNTYPED_RW_ALL_SPACE(TYPE, SPACE) \
  DECL_UNTYPED_RW_SPACE_N(TYPE, 2, SPACE) \
  DECL_UNTYPED_RW_SPACE_N(TYPE, 3, SPACE) \
  DECL_UNTYPED_RW_SPACE_N(TYPE, 4, SPACE) \
  DECL_UNTYPED_RW_SPACE_N(TYPE, 8, SPACE) \
  DECL_UNTYPED_RW_SPACE_N(TYPE, 16, SPACE)

#define DECL_UNTYPED_RW_ALL(TYPE) \
  DECL_UNTYPED_RW_ALL_SPACE(TYPE, __global) \
  DECL_UNTYPED_RW_ALL_SPACE(TYPE, __local) \
  DECL_UNTYPED_RW_ALL_SPACE(TYPE, __constant) \
  DECL_UNTYPED_RW_ALL_SPACE(TYPE, __private)

DECL_UNTYPED_RW_ALL(float)
DECL_UNTYPED_RW_ALL(uint)
DECL_UNTYPED_RW_ALL(int)

#undef DECL_UNTYPED_RW_ALL
#undef DECL_UNTYPED_RW_ALL_SPACE
#undef DECL_UNTYPED_RW_SPACE_N

/////////////////////////////////////////////////////////////////////////////
// Declare functions for vector types which are derived from scalar ones
/////////////////////////////////////////////////////////////////////////////
#define DECL_VECTOR_1OP(NAME, TYPE) \
  INLINE OVERLOADABLE TYPE##2 NAME(TYPE##2 v) { \
    return (TYPE##2)(NAME(v.x), NAME(v.y)); \
  }\
  INLINE OVERLOADABLE TYPE##3 NAME(TYPE##3 v) { \
    return (TYPE##3)(NAME(v.x), NAME(v.y), NAME(v.z)); \
  }\
  INLINE OVERLOADABLE TYPE##4 NAME(TYPE##4 v) { \
    return (TYPE##4)(NAME(v.x), NAME(v.y), NAME(v.z), NAME(v.w)); \
  }\
  INLINE OVERLOADABLE TYPE##8 NAME(TYPE##8 v) { \
    TYPE##8 dst;\
    dst.s0123 = NAME(v.s0123);\
    dst.s4567 = NAME(v.s4567);\
    return dst;\
  }\
  INLINE OVERLOADABLE TYPE##16 NAME(TYPE##16 v) { \
    TYPE##16 dst;\
    dst.s01234567 = NAME(v.s01234567);\
    dst.s89abcdef = NAME(v.s89abcdef);\
    return dst;\
  }
DECL_VECTOR_1OP(native_cos, float);
DECL_VECTOR_1OP(native_sin, float);
DECL_VECTOR_1OP(native_tan, float);
DECL_VECTOR_1OP(native_sqrt, float);
DECL_VECTOR_1OP(native_rsqrt, float);
DECL_VECTOR_1OP(native_log2, float);
DECL_VECTOR_1OP(native_recip, float);
DECL_VECTOR_1OP(fabs, float);
DECL_VECTOR_1OP(trunc, float);
DECL_VECTOR_1OP(round, float);
DECL_VECTOR_1OP(floor, float);
DECL_VECTOR_1OP(ceil, float);
DECL_VECTOR_1OP(log, float);
DECL_VECTOR_1OP(log2, float);
DECL_VECTOR_1OP(log10, float);
#undef DECL_VECTOR_1OP

#define DECL_VECTOR_2OP(NAME, TYPE) \
  INLINE OVERLOADABLE TYPE##2 NAME(TYPE##2 v0, TYPE##2 v1) { \
    return (TYPE##2)(NAME(v0.x, v1.x), NAME(v1.y, v1.y)); \
  }\
  INLINE OVERLOADABLE TYPE##3 NAME(TYPE##3 v0, TYPE##3 v1) { \
    return (TYPE##3)(NAME(v0.x, v1.x), NAME(v0.y, v1.y), NAME(v0.z, v1.z)); \
  }\
  INLINE OVERLOADABLE TYPE##4 NAME(TYPE##4 v0, TYPE##4 v1) { \
    return (TYPE##4)(NAME(v0.x, v1.x), NAME(v0.y, v1.y), NAME(v0.z, v1.z), NAME(v0.w, v1.w)); \
  }\
  INLINE OVERLOADABLE TYPE##8 NAME(TYPE##8 v0, TYPE##8 v1) { \
    TYPE##8 dst;\
    dst.s0123 = NAME(v0.s0123, v1.s0123);\
    dst.s4567 = NAME(v0.s4567, v1.s4567);\
    return dst;\
  }\
  INLINE OVERLOADABLE TYPE##16 NAME(TYPE##16 v0, TYPE##16 v1) { \
    TYPE##16 dst;\
    dst.s01234567 = NAME(v0.s01234567, v1.s01234567);\
    dst.s89abcdef = NAME(v0.s89abcdef, v1.s89abcdef);\
    return dst;\
  }
DECL_VECTOR_2OP(min, float);
DECL_VECTOR_2OP(max, float);
DECL_VECTOR_2OP(fmod, float);
DECL_VECTOR_2OP(powr, float);
#undef DECL_VECTOR_2OP

#define DECL_VECTOR_3OP(NAME, TYPE) \
  INLINE OVERLOADABLE TYPE##2 NAME(TYPE##2 v0, TYPE##2 v1, TYPE##2 v2) { \
    return (TYPE##2)(NAME(v0.x, v1.x, v2.x), NAME(v1.y, v1.y, v2.y)); \
  }\
  INLINE OVERLOADABLE TYPE##3 NAME(TYPE##3 v0, TYPE##3 v1, TYPE##3 v2) { \
    return (TYPE##3)(NAME(v0.x, v1.x, v2.x), NAME(v0.y, v1.y, v2.y), NAME(v0.z, v1.z, v2.z)); \
  }\
  INLINE OVERLOADABLE TYPE##4 NAME(TYPE##4 v0, TYPE##4 v1, TYPE##4 v2) { \
    return (TYPE##4)(NAME(v0.x, v1.x, v2.x), NAME(v0.y, v1.y, v2.y), NAME(v0.z, v1.z, v2.z), NAME(v0.w, v1.w, v2.w)); \
  }\
  INLINE OVERLOADABLE TYPE##8 NAME(TYPE##8 v0, TYPE##8 v1, TYPE##8 v2) { \
    TYPE##8 dst;\
    dst.s0123 = NAME(v0.s0123, v1.s0123, v2.s0123);\
    dst.s4567 = NAME(v0.s4567, v1.s4567, v2.s4567);\
    return dst;\
  }\
  INLINE OVERLOADABLE TYPE##16 NAME(TYPE##16 v0, TYPE##16 v1, TYPE##16 v2) { \
    TYPE##16 dst;\
    dst.s01234567 = NAME(v0.s01234567, v1.s01234567, v2.s01234567);\
    dst.s89abcdef = NAME(v0.s89abcdef, v1.s89abcdef, v2.s89abcdef);\
    return dst;\
  }
DECL_VECTOR_3OP(mad, float);
DECL_VECTOR_3OP(mix, float);
#undef DECL_VECTOR_3OP

// mix requires more variants
INLINE OVERLOADABLE float2 mix(float2 x, float2 y, float a) { return mix(x,y,(float2)(a));}
INLINE OVERLOADABLE float3 mix(float3 x, float3 y, float a) { return mix(x,y,(float3)(a));}
INLINE OVERLOADABLE float4 mix(float4 x, float4 y, float a) { return mix(x,y,(float4)(a));}
INLINE OVERLOADABLE float8 mix(float8 x, float8 y, float a) { return mix(x,y,(float8)(a));}
INLINE OVERLOADABLE float16 mix(float16 x, float16 y, float a) { return mix(x,y,(float16)(a));}

/////////////////////////////////////////////////////////////////////////////
// Force the compilation to SIMD8 or SIMD16
/////////////////////////////////////////////////////////////////////////////

int __gen_ocl_force_simd8(void);
int __gen_ocl_force_simd16(void);

#define NULL ((void*)0)
#undef PURE
#undef CONST
#undef OVERLOADABLE
#undef INLINE
#endif /* __GEN_OCL_STDLIB_H__ */

