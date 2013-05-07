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

#ifndef __GEN_OCL_STDLIB_H__
#define __GEN_OCL_STDLIB_H__

#define INLINE __attribute__((always_inline)) inline
#define OVERLOADABLE __attribute__((overloadable))
#define PURE __attribute__((pure))
#define CONST __attribute__((const))
#define INLINE_OVERLOADABLE __attribute__((overloadable,always_inline))

/////////////////////////////////////////////////////////////////////////////
// OpenCL built-in scalar data types
/////////////////////////////////////////////////////////////////////////////
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef __typeof__(sizeof(int)) size_t;
typedef __typeof__((int *)0-(int *)0) ptrdiff_t;
typedef signed int intptr_t;
typedef unsigned int uintptr_t;

/////////////////////////////////////////////////////////////////////////////
// OpenCL address space
/////////////////////////////////////////////////////////////////////////////
#define __private __attribute__((address_space(0)))
#define __global __attribute__((address_space(1)))
#define __constant __attribute__((address_space(2)))
#define __local __attribute__((address_space(3)))
#define __texture __attribute__((address_space(4)))
#define global __global
//#define local __local
#define constant __constant
#define private __private

/////////////////////////////////////////////////////////////////////////////
// OpenCL built-in vector data types
/////////////////////////////////////////////////////////////////////////////
#define DEF(type) typedef type type##2 __attribute__((ext_vector_type(2)));\
                  typedef type type##3 __attribute__((ext_vector_type(3)));\
                  typedef type type##4 __attribute__((ext_vector_type(4)));\
                  typedef type type##8 __attribute__((ext_vector_type(8)));\
                  typedef type type##16 __attribute__((ext_vector_type(16)));
DEF(char);
DEF(uchar);
DEF(short);
DEF(ushort);
DEF(int);
DEF(uint);
DEF(long);
DEF(ulong);
DEF(float);
#undef DEF
/////////////////////////////////////////////////////////////////////////////
// OpenCL other built-in data types
/////////////////////////////////////////////////////////////////////////////
struct _image2d_t;
typedef __texture struct _image2d_t* image2d_t;
struct _image3d_t;
typedef __texture struct _image3d_t* image3d_t;
//typedef __sampler const uint* sampler_t;
typedef uint sampler_t;
typedef size_t event_t;
/////////////////////////////////////////////////////////////////////////////
// OpenCL conversions & type casting
/////////////////////////////////////////////////////////////////////////////
union type_cast_4_b {
  float f;
  uchar4 u4;
};
uchar4 INLINE_OVERLOADABLE as_uchar4(float f) {
    union type_cast_4_b u;
    u.f = f;
    return u.u4;
}
#define DEF(type, n, type2) type##n INLINE_OVERLOADABLE convert_##type##n(type2##n d) { \
    return (type##n)((type)(d.s0), (type)(d.s1), (type)(d.s2), (type)(d.s3)); \
 }
#define DEF2(type) DEF(type, 4, char); \
                   DEF(type, 4, uchar); \
                   DEF(type, 4, short); \
                   DEF(type, 4, ushort); \
                   DEF(type, 4, int); \
                   DEF(type, 4, uint); \
                   DEF(type, 4, long); \
                   DEF(type, 4, ulong); \
                   DEF(type, 4, float);
DEF2(char);
DEF2(uchar);
DEF2(short);
DEF2(ushort);
DEF2(int);
DEF2(uint);
DEF2(long);
DEF2(ulong);
DEF2(float);
#undef DEF2
#undef DEF
/////////////////////////////////////////////////////////////////////////////
// OpenCL preprocessor directives & macros
/////////////////////////////////////////////////////////////////////////////
#define __OPENCL_VERSION__ 110
#define __CL_VERSION_1_0__ 100
#define __CL_VERSION_1_1__ 110
#define __ENDIAN_LITTLE__ 1
#define __kernel_exec(X, TYPE) __kernel __attribute__((work_group_size_hint(X,1,1))) \
                                        __attribute__((vec_type_hint(TYPE)))
#define kernel_exec(X, TYPE) __kernel_exec(X, TYPE)
/////////////////////////////////////////////////////////////////////////////
// OpenCL floating-point macros and pragmas
/////////////////////////////////////////////////////////////////////////////
#define FLT_DIG 6
#define FLT_MANT_DIG 24
#define FLT_MAX_10_EXP +38
#define FLT_MAX_EXP +128
#define FLT_MIN_10_EXP -37
#define FLT_MIN_EXP -125
#define FLT_RADIX 2
#define FLT_MAX 0x1.fffffep127f
#define FLT_MIN 0x1.0p-126f
#define FLT_EPSILON 0x1.0p-23f

#define MAXFLOAT     3.40282347e38F
#define HUGE_VALF    (__builtin_huge_valf())
#define INFINITY     (__builtin_inff())
#define NAN          (__builtin_nanf(""))
#define M_E_F        2.718281828459045F
#define M_LOG2E_F    1.4426950408889634F
#define M_LOG10E_F   0.43429448190325176F
#define M_LN2_F      0.6931471805599453F
#define M_LN10_F     2.302585092994046F
#define M_PI_F       3.141592653589793F
#define M_PI_2_F     1.5707963267948966F
#define M_PI_4_F     0.7853981633974483F
#define M_1_PI_F     0.3183098861837907F
#define M_2_PI_F     0.6366197723675814F
#define M_2_SQRTPI_F 1.1283791670955126F
#define M_SQRT2_F    1.4142135623730951F
#define M_SQRT1_2_F  0.7071067811865476F
/////////////////////////////////////////////////////////////////////////////
// OpenCL integer built-in macros
/////////////////////////////////////////////////////////////////////////////
#define CHAR_BIT    8
#define CHAR_MAX    SCHAR_MAX
#define CHAR_MIN    SCHAR_MIN
#define INT_MAX     2147483647
#define INT_MIN     (-2147483647 - 1)
#define LONG_MAX    0x7fffffffffffffffL
#define LONG_MIN    (-0x7fffffffffffffffL - 1)
#define SCHAR_MAX   127
#define SCHAR_MIN   (-127 - 1)
#define SHRT_MAX    32767
#define SHRT_MIN    (-32767 - 1)
#define UCHAR_MAX   255
#define USHRT_MAX   65535
#define UINT_MAX    0xffffffff
#define ULONG_MAX   0xffffffffffffffffUL
/////////////////////////////////////////////////////////////////////////////
// OpenCL relational built-in functions
/////////////////////////////////////////////////////////////////////////////
#define DEF DECL(int, float); \
            DECL(int2, float2); \
            DECL(int3, float3); \
            DECL(int4, float4); \
            DECL(int8, float8); \
            DECL(int16, float16);
#define DECL(ret, type) ret INLINE_OVERLOADABLE isequal(type x, type y) { return x == y; }
DEF;
#undef DECL
#define DECL(ret, type) ret INLINE_OVERLOADABLE isnotequal(type x, type y) { return x != y; }
DEF;
#undef DECL
#define DECL(ret, type) ret INLINE_OVERLOADABLE isgreater(type x, type y) { return x > y; }
DEF;
#undef DECL
#define DECL(ret, type) ret INLINE_OVERLOADABLE isgreaterequal(type x, type y) { return x >= y; }
DEF;
#undef DECL
#define DECL(ret, type) ret INLINE_OVERLOADABLE isless(type x, type y) { return x < y; }
DEF;
#undef DECL
#define DECL(ret, type) ret INLINE_OVERLOADABLE islessequal(type x, type y) { return x <= y; }
DEF;
#undef DECL
#undef DEF

#define SDEF(TYPE)                                                              \
INLINE_OVERLOADABLE TYPE ocl_sadd_sat(TYPE x, TYPE y);                          \
INLINE_OVERLOADABLE TYPE ocl_ssub_sat(TYPE x, TYPE y);                          \
INLINE_OVERLOADABLE TYPE add_sat(TYPE x, TYPE y) { return ocl_sadd_sat(x, y); } \
INLINE_OVERLOADABLE TYPE sub_sat(TYPE x, TYPE y) { return ocl_ssub_sat(x, y); }
SDEF(char);
SDEF(short);
SDEF(int);
SDEF(long);
#undef SDEF
#define UDEF(TYPE)                                                              \
INLINE_OVERLOADABLE TYPE ocl_uadd_sat(TYPE x, TYPE y);                          \
INLINE_OVERLOADABLE TYPE ocl_usub_sat(TYPE x, TYPE y);                          \
INLINE_OVERLOADABLE TYPE add_sat(TYPE x, TYPE y) { return ocl_uadd_sat(x, y); } \
INLINE_OVERLOADABLE TYPE sub_sat(TYPE x, TYPE y) { return ocl_usub_sat(x, y); }
UDEF(uchar);
UDEF(ushort);
UDEF(uint);
UDEF(ulong);
#undef UDEF


uchar INLINE_OVERLOADABLE convert_uchar_sat(float x) {
    return add_sat((uchar)x, (uchar)0);
}

#define DEC2(name) INLINE_OVERLOADABLE int2 name(float2 x) { return (name(x.s0), name(x.s1)); }
#define DEC3(name) INLINE_OVERLOADABLE int3 name(float3 x) { return (name(x.s0), name(x.s1), name(x.s2)); }
#define DEC4(name) INLINE_OVERLOADABLE int4 name(float4 x) { return (name(x.s0), name(x.s1), name(x.s2), name(x.s3)); }
#define DEC8(name) INLINE_OVERLOADABLE int8 name(float8 x) { return (name(x.s0), name(x.s1), name(x.s2), name(x.s3), name(x.s4), name(x.s5), name(x.s6), name(x.s7)); }
#define DEC16(name) INLINE_OVERLOADABLE int16 name(float16 x) { return (name(x.s0), name(x.s1), name(x.s2), name(x.s3), name(x.s4), name(x.s5), name(x.s6), name(x.s7), name(x.s8), name(x.s9), name(x.sA), name(x.sB), name(x.sC), name(x.sD), name(x.sE), name(x.sF)); }
INLINE_OVERLOADABLE int isfinite(float x) { return __builtin_isfinite(x); }
DEC2(isfinite);
DEC3(isfinite);
DEC4(isfinite);
DEC8(isfinite);
DEC16(isfinite);
INLINE_OVERLOADABLE int isinf(float x) { return __builtin_isinf(x); }
DEC2(isinf);
DEC3(isinf);
DEC4(isinf);
DEC8(isinf);
DEC16(isinf);
INLINE_OVERLOADABLE int isnan(float x) { return __builtin_isnan(x); }
DEC2(isnan);
DEC3(isnan);
DEC4(isnan);
DEC8(isnan);
DEC16(isnan);
INLINE_OVERLOADABLE int isnormal(float x) { return __builtin_isnormal(x); }
DEC2(isnormal);
DEC3(isnormal);
DEC4(isnormal);
DEC8(isnormal);
DEC16(isnormal);
INLINE_OVERLOADABLE int signbit(float x) { return __builtin_signbit(x); }
DEC2(signbit);
DEC3(signbit);
DEC4(signbit);
DEC8(signbit);
DEC16(signbit);
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

#define DEC2(name) INLINE_OVERLOADABLE int2 name(float2 x, float2 y) { return (name(x.s0, y.s0), name(x.s1, y.s1)); }
#define DEC3(name) INLINE_OVERLOADABLE int3 name(float3 x, float3 y) { return (name(x.s0, y.s0), name(x.s1, y.s1), name(x.s2, y.s2)); }
#define DEC4(name) INLINE_OVERLOADABLE int4 name(float4 x, float4 y) { return (name(x.s0, y.s0), name(x.s1, y.s1), name(x.s2, y.s2), name(x.s3, y.s3)); }
#define DEC8(name) INLINE_OVERLOADABLE int8 name(float8 x, float8 y) { return (name(x.s0, y.s0), name(x.s1, y.s1), name(x.s2, y.s2), name(x.s3, y.s3), name(x.s4, y.s4), name(x.s5, y.s5), name(x.s6, y.s6), name(x.s7, y.s7)); }
#define DEC16(name) INLINE_OVERLOADABLE int16 name(float16 x, float16 y) { return (name(x.s0, y.s0), name(x.s1, y.s1), name(x.s2, y.s2), name(x.s3, y.s3), name(x.s4, y.s4), name(x.s5, y.s5), name(x.s6, y.s6), name(x.s7, y.s7), name(x.s8, y.s8), name(x.s9, y.s9), name(x.sA, y.sA), name(x.sB, y.sB), name(x.sC, y.sC), name(x.sD, y.sD), name(x.sE, y.sE), name(x.sF, y.sF)); }
INLINE_OVERLOADABLE int islessgreater(float x, float y) { return (x<y)||(x>y); }
DEC2(islessgreater);
DEC3(islessgreater);
DEC4(islessgreater);
DEC8(islessgreater);
DEC16(islessgreater);
INLINE_OVERLOADABLE int isordered(float x, float y) { return isequal(x,x) && isequal(y,y); }
DEC2(isordered);
DEC3(isordered);
DEC4(isordered);
DEC8(isordered);
DEC16(isordered);
INLINE_OVERLOADABLE int isunordered(float x, float y) { return isnan(x) || isnan(y); }
DEC2(isunordered);
DEC3(isunordered);
DEC4(isunordered);
DEC8(isunordered);
DEC16(isunordered);
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16
#define DEC1(type) INLINE_OVERLOADABLE int any(type a) { return a<0; }
#define DEC2(type) INLINE_OVERLOADABLE int any(type a) { return a.s0<0 || a.s1<0; }
#define DEC3(type) INLINE_OVERLOADABLE int any(type a) { return a.s0<0 || a.s1<0 || a.s2<0; }
#define DEC4(type) INLINE_OVERLOADABLE int any(type a) { return a.s0<0 || a.s1<0 || a.s2<0 || a.s3<0; }
#define DEC8(type) INLINE_OVERLOADABLE int any(type a) { return a.s0<0 || a.s1<0 || a.s2<0 || a.s3<0 || a.s4<0 || a.s5<0 || a.s6<0 || a.s7<0; }
#define DEC16(type) INLINE_OVERLOADABLE int any(type a) { return a.s0<0 || a.s1<0 || a.s2<0 || a.s3<0 || a.s4<0 || a.s5<0 || a.s6<0 || a.s7<0 || a.s8<0 || a.s9<0 || a.sA<0 || a.sB<0 || a.sC<0 || a.sD<0 || a.sE<0 || a.sF<0; }
DEC1(char);
DEC1(short);
DEC1(int);
DEC1(long);
#define DEC(n) DEC##n(char##n); DEC##n(short##n); DEC##n(int##n); DEC##n(long##n);
DEC(2);
DEC(3);
DEC(4);
DEC(8);
DEC(16);
#undef DEC
#undef DEC1
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16
#define DEC1(type) INLINE_OVERLOADABLE int all(type a) { return a<0; }
#define DEC2(type) INLINE_OVERLOADABLE int all(type a) { return a.s0<0 && a.s1<0; }
#define DEC3(type) INLINE_OVERLOADABLE int all(type a) { return a.s0<0 && a.s1<0 && a.s2<0; }
#define DEC4(type) INLINE_OVERLOADABLE int all(type a) { return a.s0<0 && a.s1<0 && a.s2<0 && a.s3<0; }
#define DEC8(type) INLINE_OVERLOADABLE int all(type a) { return a.s0<0 && a.s1<0 && a.s2<0 && a.s3<0 && a.s4<0 && a.s5<0 && a.s6<0 && a.s7<0; }
#define DEC16(type) INLINE_OVERLOADABLE int all(type a) { return a.s0<0 && a.s1<0 && a.s2<0 && a.s3<0 && a.s4<0 && a.s5<0 && a.s6<0 && a.s7<0 && a.s8<0 && a.s9<0 && a.sA<0 && a.sB<0 && a.sC<0 && a.sD<0 && a.sE<0 && a.sF<0; }
DEC1(char);
DEC1(short);
DEC1(int);
DEC1(long);
#define DEC(n) DEC##n(char##n); DEC##n(short##n); DEC##n(int##n); DEC##n(long##n);
DEC(2);
DEC(3);
DEC(4);
DEC(8);
DEC(16);
#undef DEC
#undef DEC1
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

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

// XXX work-around PTX profile
#define sqrt native_sqrt
INLINE OVERLOADABLE float rsqrt(float x) { return native_rsqrt(x); }
INLINE OVERLOADABLE float __gen_ocl_internal_fabs(float x)  { return __gen_ocl_fabs(x); }
INLINE OVERLOADABLE float __gen_ocl_internal_trunc(float x) { return __gen_ocl_rndz(x); }
INLINE OVERLOADABLE float __gen_ocl_internal_round(float x) { return __gen_ocl_rnde(x); }
INLINE OVERLOADABLE float __gen_ocl_internal_floor(float x) { return __gen_ocl_rndd(x); }
INLINE OVERLOADABLE float __gen_ocl_internal_ceil(float x)  { return __gen_ocl_rndu(x); }
INLINE OVERLOADABLE float __gen_ocl_internal_log(float x)   { return native_log(x); }
INLINE OVERLOADABLE float __gen_ocl_internal_log2(float x)  { return native_log2(x); }
INLINE OVERLOADABLE float __gen_ocl_internal_log10(float x) { return native_log10(x); }
INLINE OVERLOADABLE float __gen_ocl_internal_exp(float x)   { return native_exp(x); }
INLINE OVERLOADABLE float powr(float x, float y) { return __gen_ocl_pow(x,y); }
INLINE OVERLOADABLE float fmod(float x, float y) { return x-y*__gen_ocl_rndz(x/y); }

// TODO use llvm intrinsics definitions
#define cos native_cos
#define sin native_sin
#define pow powr

INLINE OVERLOADABLE float mad(float a, float b, float c) {
  return a*b+c;
}

INLINE OVERLOADABLE uint select(uint src0, uint src1, int cond) {
  return cond ? src1 : src0;
}
INLINE OVERLOADABLE uint select(uint src0, uint src1, uint cond) {
  return cond ? src1 : src0;
}
INLINE OVERLOADABLE int select(int src0, int src1, int cond) {
  return cond ? src1 : src0;
}
INLINE OVERLOADABLE int select(int src0, int src1, uint cond) {
  return cond ? src1 : src0;
}
INLINE OVERLOADABLE float select(float src0, float src1, int cond) {
  return cond ? src1 : src0;
}
INLINE OVERLOADABLE float select(float src0, float src1, uint cond) {
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
DECL_SELECT4(int4, int, uint4, 0x80000000)
DECL_SELECT4(float4, float, int4, 0x80000000)
DECL_SELECT4(float4, float, uint4, 0x80000000)
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

INLINE OVERLOADABLE float __gen_ocl_internal_fmax(float a, float b) { return max(a,b); }
INLINE OVERLOADABLE float __gen_ocl_internal_fmin(float a, float b) { return min(a,b); }
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

DECL_UNTYPED_RW_ALL(char)
DECL_UNTYPED_RW_ALL(uchar)
DECL_UNTYPED_RW_ALL(short)
DECL_UNTYPED_RW_ALL(ushort)
DECL_UNTYPED_RW_ALL(int)
DECL_UNTYPED_RW_ALL(uint)
DECL_UNTYPED_RW_ALL(long)
DECL_UNTYPED_RW_ALL(ulong)
DECL_UNTYPED_RW_ALL(float)

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
DECL_VECTOR_1OP(__gen_ocl_internal_fabs, float);
DECL_VECTOR_1OP(__gen_ocl_internal_trunc, float);
DECL_VECTOR_1OP(__gen_ocl_internal_round, float);
DECL_VECTOR_1OP(__gen_ocl_internal_floor, float);
DECL_VECTOR_1OP(__gen_ocl_internal_ceil, float);
DECL_VECTOR_1OP(__gen_ocl_internal_log, float);
DECL_VECTOR_1OP(__gen_ocl_internal_log2, float);
DECL_VECTOR_1OP(__gen_ocl_internal_log10, float);
#undef DECL_VECTOR_1OP
/////////////////////////////////////////////////////////////////////////////
// Arithmetic functions
/////////////////////////////////////////////////////////////////////////////

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
DECL_VECTOR_2OP(__gen_ocl_internal_fmin, float);
DECL_VECTOR_2OP(__gen_ocl_internal_fmax, float);
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

// XXX workaround ptx profile
#define fabs __gen_ocl_internal_fabs
#define trunc __gen_ocl_internal_trunc
#define round __gen_ocl_internal_round
#define floor __gen_ocl_internal_floor
#define ceil __gen_ocl_internal_ceil
#define log __gen_ocl_internal_log
#define log2 __gen_ocl_internal_log2
#define log10 __gen_ocl_internal_log10
#define exp __gen_ocl_internal_exp
#define fmin __gen_ocl_internal_fmin
#define fmax __gen_ocl_internal_fmax

/////////////////////////////////////////////////////////////////////////////
// Synchronization functions
/////////////////////////////////////////////////////////////////////////////
#define CLK_LOCAL_MEM_FENCE  (1 << 0)
#define CLK_GLOBAL_MEM_FENCE (1 << 1)

void __gen_ocl_barrier_local(void);
void __gen_ocl_barrier_global(void);
void __gen_ocl_barrier_local_and_global(void);

typedef uint cl_mem_fence_flags;
INLINE void barrier(cl_mem_fence_flags flags) {
  if (flags == (CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE))
    __gen_ocl_barrier_local_and_global();
  else if (flags == CLK_LOCAL_MEM_FENCE)
    __gen_ocl_barrier_local();
  else if (flags == CLK_GLOBAL_MEM_FENCE)
    __gen_ocl_barrier_global();
}

/////////////////////////////////////////////////////////////////////////////
// Force the compilation to SIMD8 or SIMD16
/////////////////////////////////////////////////////////////////////////////

int __gen_ocl_force_simd8(void);
int __gen_ocl_force_simd16(void);

#define NULL ((void*)0)

/////////////////////////////////////////////////////////////////////////////
// Image access functions
/////////////////////////////////////////////////////////////////////////////

OVERLOADABLE int4 __gen_ocl_read_imagei(uint surface_id, uint sampler, int u, int v);
OVERLOADABLE int4 __gen_ocl_read_imagei(uint surface_id, uint sampler, float u, float v);
OVERLOADABLE uint4 __gen_ocl_read_imageui(uint surface_id, uint sampler, int u, int v);
OVERLOADABLE uint4 __gen_ocl_read_imageui(uint surface_id, uint sampler, float u, float v);
OVERLOADABLE float4 __gen_ocl_read_imagef(uint surface_id, uint sampler, int u, int v);
OVERLOADABLE float4 __gen_ocl_read_imagef(uint surface_id, uint sampler, float u, float v);

OVERLOADABLE int4 __gen_ocl_read_imagei(uint surface_id, uint sampler, int u, int v, int w);
OVERLOADABLE int4 __gen_ocl_read_imagei(uint surface_id, uint sampler, float u, float v, float w);
OVERLOADABLE uint4 __gen_ocl_read_imageui(uint surface_id, uint sampler, int u, int v, int w);
OVERLOADABLE uint4 __gen_ocl_read_imageui(uint surface_id, uint sampler, float u, float v, float w);
OVERLOADABLE float4 __gen_ocl_read_imagef(uint surface_id, uint sampler, int u, int v, int w);
OVERLOADABLE float4 __gen_ocl_read_imagef(uint surface_id, uint sampler, float u, float v, float w);

OVERLOADABLE void __gen_ocl_write_imagei(uint surface_id, int u, int v, int4 color);
OVERLOADABLE void __gen_ocl_write_imagei(uint surface_id, float u, float v, int4 color);
OVERLOADABLE void __gen_ocl_write_imageui(uint surface_id, int u, int v, uint4 color);
OVERLOADABLE void __gen_ocl_write_imageui(uint surface_id, float u, float v, uint4 color);
OVERLOADABLE void __gen_ocl_write_imagef(uint surface_id, int u, int v, float4 color);
OVERLOADABLE void __gen_ocl_write_imagef(uint surface_id, float u, float v, float4 color);

OVERLOADABLE void __gen_ocl_write_imagei(uint surface_id, int u, int v, int w, int4 color);
OVERLOADABLE void __gen_ocl_write_imagei(uint surface_id, float u, float v, float w, int4 color);
OVERLOADABLE void __gen_ocl_write_imageui(uint surface_id, int u, int v, int w, uint4 color);
OVERLOADABLE void __gen_ocl_write_imageui(uint surface_id, float u, float v, float w, uint4 color);
OVERLOADABLE void __gen_ocl_write_imagef(uint surface_id, int u, int v, int w, float4 color);
OVERLOADABLE void __gen_ocl_write_imagef(uint surface_id, float u, float v, float w, float4 color);

#define GET_IMAGE(cl_image, surface_id) \
    uint surface_id = (uint)cl_image

#define DECL_READ_IMAGE(type, suffix, coord_type) \
  INLINE_OVERLOADABLE type read_image ##suffix(image2d_t cl_image, sampler_t sampler, coord_type coord) \
  {\
    GET_IMAGE(cl_image, surface_id);\
    return __gen_ocl_read_image ##suffix(surface_id, sampler, coord.s0, coord.s1);\
  }

#define DECL_WRITE_IMAGE(type, suffix, coord_type) \
  INLINE_OVERLOADABLE void write_image ##suffix(image2d_t cl_image, coord_type coord, type color)\
  {\
    GET_IMAGE(cl_image, surface_id);\
    __gen_ocl_write_image ##suffix(surface_id, coord.s0, coord.s1, color);\
  }

#define DECL_IMAGE(type, suffix)        \
  DECL_READ_IMAGE(type, suffix, int2)   \
  DECL_READ_IMAGE(type, suffix, float2) \
  DECL_WRITE_IMAGE(type, suffix, int2)   \
  DECL_WRITE_IMAGE(type, suffix, float2)

DECL_IMAGE(int4, i)
DECL_IMAGE(uint4, ui)
DECL_IMAGE(float4, f)

#undef DECL_IMAGE
#undef DECL_READ_IMAGE
#undef DECL_WRITE_IMAGE

#define DECL_READ_IMAGE(type, suffix, coord_type) \
  INLINE_OVERLOADABLE type read_image ## suffix(image3d_t cl_image, sampler_t sampler, coord_type coord) \
  {\
    GET_IMAGE(cl_image, surface_id);\
    return __gen_ocl_read_image ## suffix(surface_id, (uint)sampler, coord.s0, coord.s1, coord.s2);\
  }

#define DECL_WRITE_IMAGE(type, suffix, coord_type) \
  INLINE_OVERLOADABLE void write_image ## suffix(image3d_t cl_image, coord_type coord, type color)\
  {\
    GET_IMAGE(cl_image, surface_id);\
    __gen_ocl_write_image ## suffix(surface_id, coord.s0, coord.s1, coord.s2, color);\
  }

#define DECL_IMAGE(type, suffix)        \
  DECL_READ_IMAGE(type, suffix, int4)   \
  DECL_READ_IMAGE(type, suffix, float4) \
  DECL_WRITE_IMAGE(type, suffix, int4)   \
  DECL_WRITE_IMAGE(type, suffix, float4)

DECL_IMAGE(int4, i)
DECL_IMAGE(uint4, ui)
DECL_IMAGE(float4, f)

#undef DECL_IMAGE
#undef DECL_READ_IMAGE
#undef DECL_WRITE_IMAGE

#undef GET_IMAGE
#undef INLINE_OVERLOADABLE

#undef PURE
#undef CONST
#undef OVERLOADABLE
#undef INLINE
#endif /* __GEN_OCL_STDLIB_H__ */

