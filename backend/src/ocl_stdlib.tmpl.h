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

#define INLINE inline __attribute__((always_inline))
#define OVERLOADABLE __attribute__((overloadable))
#define PURE __attribute__((pure))
#define CONST __attribute__((const))
#define INLINE_OVERLOADABLE inline __attribute__((overloadable,always_inline))

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
// These are built-ins in LLVM 3.3.
#if 100*__clang_major__ + __clang_minor__ <= 302
#define __private __attribute__((address_space(0)))
#define __global __attribute__((address_space(1)))
#define __constant __attribute__((address_space(2)))
#define __local __attribute__((address_space(3)))
#define global __global
#define local __local
#define constant __constant
#define private __private
#endif

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

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
DEF(double);
#undef DEF
/////////////////////////////////////////////////////////////////////////////
// OpenCL other built-in data types
/////////////////////////////////////////////////////////////////////////////
// FIXME:
// This is a transitional hack to bypass the LLVM 3.3 built-in types.
// See the Khronos SPIR specification for handling of these types.
#define __texture __attribute__((address_space(4)))
struct _image2d_t;
typedef __texture struct _image2d_t* __image2d_t;
struct _image3d_t;
typedef __texture struct _image3d_t* __image3d_t;
typedef uint __sampler_t;
typedef size_t __event_t;
#define image2d_t __image2d_t
#define image3d_t __image3d_t
#define sampler_t __sampler_t
#define event_t __event_t
/////////////////////////////////////////////////////////////////////////////
// OpenCL conversions & type casting
/////////////////////////////////////////////////////////////////////////////

// ##BEGIN_AS##

// ##END_AS##

// ##BEGIN_CONVERT##

// ##END_CONVERT##

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
#define FLT_ONE 1.0000000000e+00         /* 0x3F800000 */
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

int INLINE_OVERLOADABLE isequal(float x, float y) { return x == y; }
int INLINE_OVERLOADABLE isnotequal(float x, float y) { return x != y; }
int INLINE_OVERLOADABLE isgreater(float x, float y) { return x > y; }
int INLINE_OVERLOADABLE isgreaterequal(float x, float y) { return x >= y; }
int INLINE_OVERLOADABLE isless(float x, float y) { return x < y; }
int INLINE_OVERLOADABLE islessequal(float x, float y) { return x <= y; }
int INLINE_OVERLOADABLE islessgreater(float x, float y) { return (x < y) || (x > y); }

#define SDEF(TYPE)                                                              \
OVERLOADABLE TYPE ocl_sadd_sat(TYPE x, TYPE y);                          \
OVERLOADABLE TYPE ocl_ssub_sat(TYPE x, TYPE y);                          \
INLINE_OVERLOADABLE TYPE add_sat(TYPE x, TYPE y) { return ocl_sadd_sat(x, y); } \
INLINE_OVERLOADABLE TYPE sub_sat(TYPE x, TYPE y) { return ocl_ssub_sat(x, y); }
SDEF(char);
SDEF(short);
SDEF(int);
SDEF(long);
#undef SDEF
#define UDEF(TYPE)                                                              \
OVERLOADABLE TYPE ocl_uadd_sat(TYPE x, TYPE y);                          \
OVERLOADABLE TYPE ocl_usub_sat(TYPE x, TYPE y);                          \
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

INLINE_OVERLOADABLE int isfinite(float x) { return __builtin_isfinite(x); }
INLINE_OVERLOADABLE int isinf(float x) { return __builtin_isinf(x); }
INLINE_OVERLOADABLE int isnan(float x) { return __builtin_isnan(x); }
INLINE_OVERLOADABLE int isnormal(float x) { return __builtin_isnormal(x); }
INLINE_OVERLOADABLE int isordered(float x, float y) { return isequal(x, x) && isequal(y, y); }
INLINE_OVERLOADABLE int isunordered(float x, float y) { return isnan(x) || isnan(y); }
INLINE_OVERLOADABLE int signbit(float x) { return __builtin_signbit(x); }

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

#define DEF(type) INLINE_OVERLOADABLE type bitselect(type a, type b, type c) { return (a & ~c) | (b & c); }
DEF(char); DEF(uchar); DEF(short); DEF(ushort); DEF(int); DEF(uint)
DEF(long); DEF(ulong)
#undef DEF
INLINE_OVERLOADABLE float bitselect(float a, float b, float c) {
  return as_float(bitselect(as_int(a), as_int(b), as_int(c)));
}

/////////////////////////////////////////////////////////////////////////////
// Integer built-in functions
/////////////////////////////////////////////////////////////////////////////
PURE CONST uint __gen_ocl_fbh(uint);
PURE CONST uint __gen_ocl_fbl(uint);

INLINE_OVERLOADABLE char clz(char x) {
  if (x < 0)
    return 0;
  if (x == 0)
    return 8;
  return __gen_ocl_fbl(x) - 24;
}

INLINE_OVERLOADABLE uchar clz(uchar x) {
  if (x == 0)
    return 8;
  return __gen_ocl_fbl(x) - 24;
}

INLINE_OVERLOADABLE short clz(short x) {
  if (x < 0)
    return 0;
  if (x == 0)
    return 16;
  return __gen_ocl_fbh(x) - 16;
}

INLINE_OVERLOADABLE ushort clz(ushort x) {
  if (x == 0)
    return 16;
  return __gen_ocl_fbh(x) - 16;
}

INLINE_OVERLOADABLE int clz(int x) {
  if (x < 0)
    return 0;
  if (x == 0)
    return 32;
  return __gen_ocl_fbh(x);
}

INLINE_OVERLOADABLE uint clz(uint x) {
  if (x == 0)
    return 32;
  return __gen_ocl_fbh(x);
}

INLINE_OVERLOADABLE long clz(long x) {
  return 0;
}

INLINE_OVERLOADABLE ulong clz(ulong x) {
  return 0;
}

OVERLOADABLE int __gen_ocl_mul_hi(int x, int y);
OVERLOADABLE uint __gen_ocl_mul_hi(uint x, uint y);
INLINE_OVERLOADABLE char mul_hi(char x, char y) { return (x * y) >> 8; }
INLINE_OVERLOADABLE uchar mul_hi(uchar x, uchar y) { return (x * y) >> 8; }
INLINE_OVERLOADABLE short mul_hi(short x, short y) { return (x * y) >> 16; }
INLINE_OVERLOADABLE ushort mul_hi(ushort x, ushort y) { return (x * y) >> 16; }
INLINE_OVERLOADABLE int mul_hi(int x, int y) { return __gen_ocl_mul_hi(x, y); }
INLINE_OVERLOADABLE uint mul_hi(uint x, uint y) { return __gen_ocl_mul_hi(x, y); }
INLINE_OVERLOADABLE long mul_hi(long x, long y) {
  return 0;
}
INLINE_OVERLOADABLE ulong mul_hi(ulong x, ulong y) {
  return 0;
}

#define DEF(type) INLINE_OVERLOADABLE type mad_hi(type a, type b, type c) { return mul_hi(a, b) + c; }
DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(int)
DEF(uint)
DEF(long)
DEF(ulong)
#undef DEF

INLINE_OVERLOADABLE int mul24(int a, int b) { return ((a << 8) >> 8) * ((b << 8) >> 8); }
INLINE_OVERLOADABLE uint mul24(uint a, uint b) { return (a & 0xFFFFFF) * (b & 0xFFFFFF); }

INLINE_OVERLOADABLE int mad24(int a, int b, int c) { return mul24(a, b) + c; }
INLINE_OVERLOADABLE uint mad24(uint a, uint b, uint c) { return mul24(a, b) + c; }

INLINE_OVERLOADABLE char mad_sat(char a, char b, char c) {
  int x = (int)a * (int)b + (int)c;
  if (x > 127)
    x = 127;
  if (x < -128)
    x = -128;
  return x;
}

INLINE_OVERLOADABLE uchar mad_sat(uchar a, uchar b, uchar c) {
  uint x = (uint)a * (uint)b + (uint)c;
  if (x > 255)
    x = 255;
  return x;
}

INLINE_OVERLOADABLE short mad_sat(short a, short b, short c) {
  int x = (int)a * (int)b + (int)c;
  if (x > 32767)
    x = 32767;
  if (x < -32768)
    x = -32768;
  return x;
}

INLINE_OVERLOADABLE ushort mad_sat(ushort a, ushort b, ushort c) {
  uint x = (uint)a * (uint)b + (uint)c;
  if (x > 65535)
    x = 65535;
  return x;
}

INLINE_OVERLOADABLE int mad_sat(int a, int b, int c) {
  long x = (long)a * (long)b + (long)c;
  if (x > 0x7FFFFFFF)
    x = 0x7FFFFFFF;
  else if (x < -0x7FFFFFFF-1)
    x = -0x7FFFFFFF-1;
  return (int)x;
}

INLINE_OVERLOADABLE uint mad_sat(uint a, uint b, uint c) {
  ulong x = (ulong)a * (ulong)b + (ulong)c;
  if (x > 0xFFFFFFFFu)
    x = 0xFFFFFFFFu;
  return (uint)x;
}

INLINE_OVERLOADABLE long mad_sat(long a, long b, long c) {
  return 0;
}

INLINE_OVERLOADABLE ulong mad_sat(ulong a, ulong b, ulong c) {
  return 0;
}

INLINE_OVERLOADABLE uchar __rotate_left(uchar x, uchar y) { return (x << y) | (x >> (8 - y)); }
INLINE_OVERLOADABLE char __rotate_left(char x, char y) { return __rotate_left((uchar)x, (uchar)y); }
INLINE_OVERLOADABLE ushort __rotate_left(ushort x, ushort y) { return (x << y) | (x >> (16 - y)); }
INLINE_OVERLOADABLE short __rotate_left(short x, short y) { return __rotate_left((ushort)x, (ushort)y); }
INLINE_OVERLOADABLE uint __rotate_left(uint x, uint y) { return (x << y) | (x >> (32 - y)); }
INLINE_OVERLOADABLE int __rotate_left(int x, int y) { return __rotate_left((uint)x, (uint)y); }
#define DEF(type, m) INLINE_OVERLOADABLE type rotate(type x, type y) { return __rotate_left(x, (type)(y & m)); }
DEF(char, 7)
DEF(uchar, 7)
DEF(short, 15)
DEF(ushort, 15)
DEF(int, 31)
DEF(uint, 31)
#undef DEF
INLINE_OVERLOADABLE long rotate(long x, long y) {
  return 0;
}
INLINE_OVERLOADABLE ulong rotate(ulong x, ulong y) {
  return 0;
}

OVERLOADABLE short __gen_ocl_upsample(short hi, short lo);
OVERLOADABLE int __gen_ocl_upsample(int hi, int lo);
OVERLOADABLE long __gen_ocl_upsample(long hi, long lo);
INLINE_OVERLOADABLE short upsample(char hi, uchar lo) { return __gen_ocl_upsample((short)hi, (short)lo); }
INLINE_OVERLOADABLE ushort upsample(uchar hi, uchar lo) { return __gen_ocl_upsample((short)hi, (short)lo); }
INLINE_OVERLOADABLE int upsample(short hi, ushort lo) { return __gen_ocl_upsample((int)hi, (int)lo); }
INLINE_OVERLOADABLE uint upsample(ushort hi, ushort lo) { return __gen_ocl_upsample((int)hi, (int)lo); }
INLINE_OVERLOADABLE long upsample(int hi, uint lo) {
  return __gen_ocl_upsample((long)hi, (long)lo);
}
INLINE_OVERLOADABLE ulong upsample(uint hi, uint lo) {
  return __gen_ocl_upsample((long)hi, (long)lo);
}

PURE CONST uint __gen_ocl_hadd(uint x, uint y);
PURE CONST uint __gen_ocl_rhadd(uint x, uint y);
#define DEC DEF(char); DEF(uchar); DEF(short); DEF(ushort)
#define DEF(type) INLINE_OVERLOADABLE type hadd(type x, type y) { return (x + y) >> 1; }
DEC
#undef DEF
#define DEF(type) INLINE_OVERLOADABLE type rhadd(type x, type y) { return (x + y + 1) >> 1; }
DEC
#undef DEF
#undef DEC
INLINE_OVERLOADABLE int hadd(int x, int y) { return (x < 0 && y > 0) || (x > 0 && y < 0) ? ((x + y) >> 1) : __gen_ocl_hadd(x, y); }
INLINE_OVERLOADABLE uint hadd(uint x, uint y) { return __gen_ocl_hadd(x, y); }
INLINE_OVERLOADABLE int rhadd(int x, int y) { return (x < 0 && y > 0) || (x > 0 && y < 0) ? ((x + y + 1) >> 1) : __gen_ocl_rhadd(x, y); }
INLINE_OVERLOADABLE uint rhadd(uint x, uint y) { return __gen_ocl_rhadd(x, y); }
INLINE_OVERLOADABLE long hadd(long x, long y) {
  return 0;
}
INLINE_OVERLOADABLE ulong hadd(ulong x, ulong y) {
  return 0;
}
INLINE_OVERLOADABLE long rhadd(long x, long y) {
  return 0;
}
INLINE_OVERLOADABLE ulong rhadd(ulong x, ulong y) {
  return 0;
}

int __gen_ocl_abs(int x);
#define DEC(TYPE) INLINE_OVERLOADABLE u##TYPE abs(TYPE x) { return (u##TYPE) __gen_ocl_abs(x); }
DEC(int)
DEC(short)
DEC(char)
#undef DEC
INLINE_OVERLOADABLE ulong abs(long x) { return x < 0 ? -x : x; }
/* For unsigned types, do nothing. */
#define DEC(TYPE) INLINE_OVERLOADABLE TYPE abs(TYPE x) { return x; }
DEC(uint)
DEC(ushort)
DEC(uchar)
DEC(ulong)
#undef DEC

/* Char and short type abs diff */
/* promote char and short to int and will be no module overflow */
#define DEC(TYPE, UTYPE) INLINE_OVERLOADABLE UTYPE abs_diff(TYPE x, TYPE y) \
                         { return (UTYPE) (abs((int)x - (int)y)); }
DEC(char, uchar)
DEC(uchar, uchar)
DEC(short, ushort)
DEC(ushort, ushort)
#undef DEC

INLINE_OVERLOADABLE uint abs_diff (uint x, uint y) {
    /* same signed will never overflow. */
    return y > x ? (y -x) : (x - y);
}

INLINE_OVERLOADABLE uint abs_diff (int x, int y) {
    /* same signed will never module overflow. */
    if ((x >= 0 && y >= 0) || (x <= 0 && y <= 0))
        return abs(x - y);

    return (abs(x) + abs(y));
}

INLINE_OVERLOADABLE ulong abs_diff (long x, long y) {
  if ((x >= 0 && y >= 0) || (x <= 0 && y <= 0))
    return abs(x - y);
  return abs(x) + abs(y);
}
INLINE_OVERLOADABLE ulong abs_diff (ulong x, ulong y) {
  return y > x ? (y - x) : (x - y);
}

/////////////////////////////////////////////////////////////////////////////
// Work Items functions (see 6.11.1 of OCL 1.1 spec)
/////////////////////////////////////////////////////////////////////////////

PURE CONST uint __gen_ocl_get_work_dim(void);
INLINE uint get_work_dim(void) {
  return __gen_ocl_get_work_dim();
}

#define DECL_INTERNAL_WORK_ITEM_FN(NAME) \
PURE CONST unsigned int __gen_ocl_##NAME##0(void); \
PURE CONST unsigned int __gen_ocl_##NAME##1(void); \
PURE CONST unsigned int __gen_ocl_##NAME##2(void);
DECL_INTERNAL_WORK_ITEM_FN(get_group_id)
DECL_INTERNAL_WORK_ITEM_FN(get_local_id)
DECL_INTERNAL_WORK_ITEM_FN(get_local_size)
DECL_INTERNAL_WORK_ITEM_FN(get_global_size)
DECL_INTERNAL_WORK_ITEM_FN(get_global_offset)
DECL_INTERNAL_WORK_ITEM_FN(get_num_groups)
#undef DECL_INTERNAL_WORK_ITEM_FN

#define DECL_PUBLIC_WORK_ITEM_FN(NAME, OTHER_RET)    \
INLINE unsigned NAME(unsigned int dim) {             \
  if (dim == 0) return __gen_ocl_##NAME##0();        \
  else if (dim == 1) return __gen_ocl_##NAME##1();   \
  else if (dim == 2) return __gen_ocl_##NAME##2();   \
  else return OTHER_RET;                             \
}

DECL_PUBLIC_WORK_ITEM_FN(get_group_id, 0)
DECL_PUBLIC_WORK_ITEM_FN(get_local_id, 0)
DECL_PUBLIC_WORK_ITEM_FN(get_local_size, 1)
DECL_PUBLIC_WORK_ITEM_FN(get_global_size, 1)
DECL_PUBLIC_WORK_ITEM_FN(get_global_offset, 0)
DECL_PUBLIC_WORK_ITEM_FN(get_num_groups, 1)
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
INLINE_OVERLOADABLE float hypot(float x, float y) { return __gen_ocl_sqrt(x*x + y*y); }
INLINE_OVERLOADABLE float native_cos(float x) { return __gen_ocl_cos(x); }
INLINE_OVERLOADABLE float __gen_ocl_internal_cospi(float x) {
  return __gen_ocl_cos(x * M_PI_F);
}
INLINE_OVERLOADABLE float native_sin(float x) { return __gen_ocl_sin(x); }
INLINE_OVERLOADABLE float __gen_ocl_internal_sinpi(float x) {
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
  float y, z;
  int n, ix;
  ix = *(int *) (&x) & 0x7fffffff;
  if (ix < 0x3e800000)
    return __gen_ocl_sin(M_PI_F * x);
  y = -x;
  z = __gen_ocl_rndd(y);
  if (z != y) {
    y *= 0.5f;
    y = 2.f * (y - __gen_ocl_rndd(y));
    n = y * 4.f;
  } else {
    if (ix >= 0x4b800000) {
      y = 0;
      n = 0;
    } else {
      if (ix < 0x4b000000)
        z = y + 8.3886080000e+06f;
      int n = *(int *) (&z);
      n &= 1;
      y = n;
      n <<= 2;
    }
  }
  switch (n) {
  case 0:
    y = __gen_ocl_sin(M_PI_F * y);
    break;
  case 1:
  case 2:
    y = __gen_ocl_cos(M_PI_F * (0.5f - y));
    break;
  case 3:
  case 4:
    y = __gen_ocl_sin(M_PI_F * (1.f - y));
    break;
  case 5:
  case 6:
    y = -__gen_ocl_cos(M_PI_F * (y - 1.5f));
    break;
  default:
    y = __gen_ocl_sin(M_PI_F * (y - 2.f));
    break;
  }
  return -y;
}
INLINE_OVERLOADABLE float native_sqrt(float x) { return __gen_ocl_sqrt(x); }
INLINE_OVERLOADABLE float native_rsqrt(float x) { return __gen_ocl_rsqrt(x); }
INLINE_OVERLOADABLE float native_log2(float x) { return __gen_ocl_log(x); }
INLINE_OVERLOADABLE float native_log(float x) {
  return native_log2(x) * 0.6931472002f;
}
INLINE_OVERLOADABLE float tgamma(float x) {
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
  float pi = 3.1415927410e+00,
    a0 = 7.7215664089e-02,
    a1 = 3.2246702909e-01,
    a2 = 6.7352302372e-02,
    a3 = 2.0580807701e-02,
    a4 = 7.3855509982e-03,
    a5 = 2.8905137442e-03,
    a6 = 1.1927076848e-03,
    a7 = 5.1006977446e-04,
    a8 = 2.2086278477e-04,
    a9 = 1.0801156895e-04,
    a10 = 2.5214456400e-05,
    a11 = 4.4864096708e-05,
    tc = 1.4616321325e+00,
    tf = -1.2148628384e-01,
    tt = 6.6971006518e-09,
    t0 = 4.8383611441e-01,
    t1 = -1.4758771658e-01,
    t2 = 6.4624942839e-02,
    t3 = -3.2788541168e-02,
    t4 = 1.7970675603e-02,
    t5 = -1.0314224288e-02,
    t6 = 6.1005386524e-03,
    t7 = -3.6845202558e-03,
    t8 = 2.2596477065e-03,
    t9 = -1.4034647029e-03,
    t10 = 8.8108185446e-04,
    t11 = -5.3859531181e-04,
    t12 = 3.1563205994e-04,
    t13 = -3.1275415677e-04,
    t14 = 3.3552918467e-04,
    u0 = -7.7215664089e-02,
    u1 = 6.3282704353e-01,
    u2 = 1.4549225569e+00,
    u3 = 9.7771751881e-01,
    u4 = 2.2896373272e-01,
    u5 = 1.3381091878e-02,
    v1 = 2.4559779167e+00,
    v2 = 2.1284897327e+00,
    v3 = 7.6928514242e-01,
    v4 = 1.0422264785e-01,
    v5 = 3.2170924824e-03,
    s0 = -7.7215664089e-02,
    s1 = 2.1498242021e-01,
    s2 = 3.2577878237e-01,
    s3 = 1.4635047317e-01,
    s4 = 2.6642270386e-02,
    s5 = 1.8402845599e-03,
    s6 = 3.1947532989e-05,
    r1 = 1.3920053244e+00,
    r2 = 7.2193557024e-01,
    r3 = 1.7193385959e-01,
    r4 = 1.8645919859e-02,
    r5 = 7.7794247773e-04,
    r6 = 7.3266842264e-06,
    w0 = 4.1893854737e-01,
    w1 = 8.3333335817e-02,
    w2 = -2.7777778450e-03,
    w3 = 7.9365057172e-04,
    w4 = -5.9518753551e-04,
    w5 = 8.3633989561e-04,
    w6 = -1.6309292987e-03;
  float t, y, z, nadj, p, p1, p2, p3, q, r, w;
  int i, hx, ix;
  nadj = 0;
  hx = *(int *) (&x);
  ix = hx & 0x7fffffff;
  if (ix >= 0x7f800000)
    return x * x;
  if (ix == 0)
    return INFINITY;
  if (ix < 0x1c800000) {
    if (hx < 0) {
      return - native_log(-x);
    } else
      return - native_log(x);
  }
  if (hx < 0) {
    if (ix >= 0x4b000000)
      return INFINITY;
    t = __gen_ocl_internal_sinpi(x);
    if (__gen_ocl_fabs(t) < 1e-8f)
      return INFINITY;
    nadj = native_log(M_PI_F / __gen_ocl_fabs(t * x));
    x = -x;
  }

  if (ix == 0x3f800000 || ix == 0x40000000)
    r = 0;
  else if (ix < 0x40000000) {
    if (ix <= 0x3f666666) {
      r = - native_log(x);
      if (ix >= 0x3f3b4a20) {
        y = 1 - x;
        i = 0;
      } else if (ix >= 0x3e6d3308) {
        y = x - (tc - 1);
        i = 1;
      } else {
        y = x;
        i = 2;
      }
    } else {
      r = 0;
      if (ix >= 0x3fdda618) {
        y = 2 - x;
        i = 0;
      } else if (ix >= 0x3F9da620) {
        y = x - tc;
        i = 1;
      } else {
        y = x - 1;
        i = 2;
      }
    }
    switch (i) {
    case 0:
      z = y * y;
      p1 = a0 + z * (a2 + z * (a4 + z * (a6 + z * (a8 + z * a10))));
      p2 = z * (a1 + z * (a3 + z * (a5 + z * (a7 + z * (a9 + z * a11)))));
      p = y * p1 + p2;
      r += (p - .5f * y);
      break;
    case 1:
      z = y * y;
      w = z * y;
      p1 = t0 + w * (t3 + w * (t6 + w * (t9 + w * t12)));
      p2 = t1 + w * (t4 + w * (t7 + w * (t10 + w * t13)));
      p3 = t2 + w * (t5 + w * (t8 + w * (t11 + w * t14)));
      p = z * p1 - (tt - w * (p2 + y * p3));
      r += (tf + p);
      break;
    case 2:
      p1 = y * (u0 + y * (u1 + y * (u2 + y * (u3 + y * (u4 + y * u5)))));
      p2 = 1 + y * (v1 + y * (v2 + y * (v3 + y * (v4 + y * v5))));
      r += (-.5f * y + p1 / p2);
    }
  } else if (ix < 0x41000000) {
    i = x;
    t = 0;
    y = x - i;
    p = y*(s0+y*(s1+y*(s2+y*(s3+y*(s4+y*(s5+y*s6))))));
    q = 1 + y * (r1 + y * (r2 + y * (r3 + y * (r4 + y * (r5 + y * r6)))));
    r = .5f * y + p / q;
    z = 1;
    switch (i) {
    case 7:
      z *= (y + 6.f);
    case 6:
      z *= (y + 5.f);
    case 5:
      z *= (y + 4.f);
    case 4:
      z *= (y + 3.f);
    case 3:
      z *= (y + 2.f);
      r += native_log(z);
      break;
    }
  } else if (ix < 0x5c800000) {
    t = native_log(x);
    z = 1 / x;
    y = z * z;
    w = w0 + z * (w1 + y * (w2 + y * (w3 + y * (w4 + y * (w5 + y * w6)))));
    r = (x - .5f) * (t - 1) + w;
  } else
    r = x * (native_log(x) - 1);
  if (hx < 0)
    r = nadj - r;
  return r;
}

INLINE_OVERLOADABLE float lgamma(float x) {
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
    const float
        zero=  0.,
        one =  1.0000000000e+00,
        pi  =  3.1415927410e+00,
        a0  =  7.7215664089e-02,
        a1  =  3.2246702909e-01,
        a2  =  6.7352302372e-02,
        a3  =  2.0580807701e-02,
        a4  =  7.3855509982e-03,
        a5  =  2.8905137442e-03,
        a6  =  1.1927076848e-03,
        a7  =  5.1006977446e-04,
        a8  =  2.2086278477e-04,
        a9  =  1.0801156895e-04,
        a10 =  2.5214456400e-05,
        a11 =  4.4864096708e-05,
        tc  =  1.4616321325e+00,
        tf  = -1.2148628384e-01,
        tt  =  6.6971006518e-09,
        t0  =  4.8383611441e-01,
        t1  = -1.4758771658e-01,
        t2  =  6.4624942839e-02,
        t3  = -3.2788541168e-02,
        t4  =  1.7970675603e-02,
        t5  = -1.0314224288e-02,
        t6  =  6.1005386524e-03,
        t7  = -3.6845202558e-03,
        t8  =  2.2596477065e-03,
        t9  = -1.4034647029e-03,
        t10 =  8.8108185446e-04,
        t11 = -5.3859531181e-04,
        t12 =  3.1563205994e-04,
        t13 = -3.1275415677e-04,
        t14 =  3.3552918467e-04,
        u0  = -7.7215664089e-02,
        u1  =  6.3282704353e-01,
        u2  =  1.4549225569e+00,
        u3  =  9.7771751881e-01,
        u4  =  2.2896373272e-01,
        u5  =  1.3381091878e-02,
        v1  =  2.4559779167e+00,
        v2  =  2.1284897327e+00,
        v3  =  7.6928514242e-01,
        v4  =  1.0422264785e-01,
        v5  =  3.2170924824e-03,
        s0  = -7.7215664089e-02,
        s1  =  2.1498242021e-01,
        s2  =  3.2577878237e-01,
        s3  =  1.4635047317e-01,
        s4  =  2.6642270386e-02,
        s5  =  1.8402845599e-03,
        s6  =  3.1947532989e-05,
        r1  =  1.3920053244e+00,
        r2  =  7.2193557024e-01,
        r3  =  1.7193385959e-01,
        r4  =  1.8645919859e-02,
        r5  =  7.7794247773e-04,
        r6  =  7.3266842264e-06,
        w0  =  4.1893854737e-01,
        w1  =  8.3333335817e-02,
        w2  = -2.7777778450e-03,
        w3  =  7.9365057172e-04,
        w4  = -5.9518753551e-04,
        w5  =  8.3633989561e-04,
        w6  = -1.6309292987e-03;
	float t, y, z, nadj, p, p1, p2, p3, q, r, w;
	int i, hx, ix;
	nadj = 0;
	hx = *(int *)&x;
	ix = hx & 0x7fffffff;
	if (ix >= 0x7f800000)
		return x * x;
	if (ix == 0)
		return ((x + one) / zero);
	if (ix < 0x1c800000) {
		if (hx < 0) {
			return -native_log(-x);
		} else
			return -native_log(x);
	}
	if (hx < 0) {
		if (ix >= 0x4b000000)
			return ((-x) / zero);
		t = __gen_ocl_internal_sinpi(x);
		if (t == zero)
			return ((-x) / zero);
		nadj = native_log(pi / __gen_ocl_fabs(t * x));
		x = -x;
	}
	if (ix == 0x3f800000 || ix == 0x40000000)
		r = 0;
	else if (ix < 0x40000000) {
		if (ix <= 0x3f666666) {
			r = -native_log(x);
			if (ix >= 0x3f3b4a20) {
				y = one - x;
				i = 0;
			} else if (ix >= 0x3e6d3308) {
				y = x - (tc - one);
				i = 1;
			} else {
				y = x;
				i = 2;
			}
		} else {
			r = zero;
			if (ix >= 0x3fdda618) {
				y = (float) 2.0 - x;
				i = 0;
			}
			else if (ix >= 0x3F9da620) {
				y = x - tc;
				i = 1;
			}
			else {
				y = x - one;
				i = 2;
			}
		}
		switch (i) {
		case 0:
			z = y * y;
			p1 = a0 + z * (a2 + z * (a4 + z * (a6 + z * (a8 + z * a10))));
			p2 = z * (a1 + z * (a3 + z * (a5 + z * (a7 + z * (a9 + z * a11)))));
			p = y * p1 + p2;
			r += (p - (float) 0.5 * y);
			break;
		case 1:
			z = y * y;
			w = z * y;
			p1 = t0 + w * (t3 + w * (t6 + w * (t9 + w * t12)));
			p2 = t1 + w * (t4 + w * (t7 + w * (t10 + w * t13)));
			p3 = t2 + w * (t5 + w * (t8 + w * (t11 + w * t14)));
			p = z * p1 - (tt - w * (p2 + y * p3));
			r += (tf + p);
			break;
		case 2:
			p1 = y * (u0 + y * (u1 + y * (u2 + y * (u3 + y * (u4 + y * u5)))));
			p2 = one + y * (v1 + y * (v2 + y * (v3 + y * (v4 + y * v5))));
			r += (-(float) 0.5 * y + p1 / p2);
		}
	} else if (ix < 0x41000000) {
		i = (int) x;
		t = zero;
		y = x - (float) i;
		p = y * (s0 + y * (s1 + y * (s2 + y * (s3 + y * (s4 + y * (s5 + y * s6))))));
		q = one + y * (r1 + y * (r2 + y * (r3 + y * (r4 + y * (r5 + y * r6)))));
		r = .5f * y + p / q;
		z = one;
		switch (i) {
		case 7:
			z *= (y + (float) 6.0);
		case 6:
			z *= (y + (float) 5.0);
		case 5:
			z *= (y + (float) 4.0);
		case 4:
			z *= (y + (float) 3.0);
		case 3:
			z *= (y + (float) 2.0);
			r += native_log(z);
			break;
		}

	} else if (ix < 0x5c800000) {
		t = native_log(x);
		z = one / x;
		y = z * z;
		w = w0 + z * (w1 + y * (w2 + y * (w3 + y * (w4 + y * (w5 + y * w6)))));
		r = (x - .5f) * (t - one) + w;
	} else
		r = x * (native_log(x) - one);
	if (hx < 0)
		r = nadj - r;
	return r;
}

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
#define BODY \
    const float  \
        zero=  0.,  \
        one =  1.0000000000e+00,  \
        pi  =  3.1415927410e+00,  \
        a0  =  7.7215664089e-02,  \
        a1  =  3.2246702909e-01,  \
        a2  =  6.7352302372e-02,  \
        a3  =  2.0580807701e-02,  \
        a4  =  7.3855509982e-03,  \
        a5  =  2.8905137442e-03,  \
        a6  =  1.1927076848e-03,  \
        a7  =  5.1006977446e-04,  \
        a8  =  2.2086278477e-04,  \
        a9  =  1.0801156895e-04,  \
        a10 =  2.5214456400e-05,  \
        a11 =  4.4864096708e-05,  \
        tc  =  1.4616321325e+00,  \
        tf  = -1.2148628384e-01,  \
        tt  =  6.6971006518e-09,  \
        t0  =  4.8383611441e-01,  \
        t1  = -1.4758771658e-01,  \
        t2  =  6.4624942839e-02,  \
        t3  = -3.2788541168e-02,  \
        t4  =  1.7970675603e-02,  \
        t5  = -1.0314224288e-02,  \
        t6  =  6.1005386524e-03,  \
        t7  = -3.6845202558e-03,  \
        t8  =  2.2596477065e-03,  \
        t9  = -1.4034647029e-03,  \
        t10 =  8.8108185446e-04,  \
        t11 = -5.3859531181e-04,  \
        t12 =  3.1563205994e-04,  \
        t13 = -3.1275415677e-04,  \
        t14 =  3.3552918467e-04,  \
        u0  = -7.7215664089e-02,  \
        u1  =  6.3282704353e-01,  \
        u2  =  1.4549225569e+00,  \
        u3  =  9.7771751881e-01,  \
        u4  =  2.2896373272e-01,  \
        u5  =  1.3381091878e-02,  \
        v1  =  2.4559779167e+00,  \
        v2  =  2.1284897327e+00,  \
        v3  =  7.6928514242e-01,  \
        v4  =  1.0422264785e-01,  \
        v5  =  3.2170924824e-03,  \
        s0  = -7.7215664089e-02,  \
        s1  =  2.1498242021e-01,  \
        s2  =  3.2577878237e-01,  \
        s3  =  1.4635047317e-01,  \
        s4  =  2.6642270386e-02,  \
        s5  =  1.8402845599e-03,  \
        s6  =  3.1947532989e-05,  \
        r1  =  1.3920053244e+00,  \
        r2  =  7.2193557024e-01,  \
        r3  =  1.7193385959e-01,  \
        r4  =  1.8645919859e-02,  \
        r5  =  7.7794247773e-04,  \
        r6  =  7.3266842264e-06,  \
        w0  =  4.1893854737e-01,  \
        w1  =  8.3333335817e-02,  \
        w2  = -2.7777778450e-03,  \
        w3  =  7.9365057172e-04,  \
        w4  = -5.9518753551e-04,  \
        w5  =  8.3633989561e-04,  \
        w6  = -1.6309292987e-03;  \
	float t, y, z, nadj, p, p1, p2, p3, q, r, w;  \
	int i, hx, ix;  \
	nadj = 0;  \
	hx = *(int *)&x;  \
	*signgamp = 1;  \
	ix = hx & 0x7fffffff;  \
	if (ix >= 0x7f800000)  \
		return x * x;  \
	if (ix == 0)  \
		return ((x + one) / zero);  \
	if (ix < 0x1c800000) {  \
		if (hx < 0) {  \
			*signgamp = -1;  \
			return -native_log(-x);  \
		} else  \
			return -native_log(x);  \
	}  \
	if (hx < 0) {  \
		if (ix >= 0x4b000000)  \
			return ((-x) / zero);  \
		t = __gen_ocl_internal_sinpi(x);  \
		if (t == zero)  \
			return ((-x) / zero);  \
		nadj = native_log(pi / __gen_ocl_fabs(t * x));  \
		if (t < zero)  \
			*signgamp = -1;  \
		x = -x;  \
	}  \
	if (ix == 0x3f800000 || ix == 0x40000000)  \
		r = 0;  \
	else if (ix < 0x40000000) {  \
		if (ix <= 0x3f666666) {  \
			r = -native_log(x);  \
			if (ix >= 0x3f3b4a20) {  \
				y = one - x;  \
				i = 0;  \
			} else if (ix >= 0x3e6d3308) {  \
				y = x - (tc - one);  \
				i = 1;  \
			} else {  \
				y = x;  \
				i = 2;  \
			}  \
		} else {  \
			r = zero;  \
			if (ix >= 0x3fdda618) {  \
				y = (float) 2.0 - x;  \
				i = 0;  \
			}  \
			else if (ix >= 0x3F9da620) {  \
				y = x - tc;  \
				i = 1;  \
			}  \
			else {  \
				y = x - one;  \
				i = 2;  \
			}  \
		}  \
		switch (i) {  \
		case 0:  \
			z = y * y;  \
			p1 = a0 + z * (a2 + z * (a4 + z * (a6 + z * (a8 + z * a10))));  \
			p2 = z * (a1 + z * (a3 + z * (a5 + z * (a7 + z * (a9 + z * a11)))));  \
			p = y * p1 + p2;  \
			r += (p - (float) 0.5 * y);  \
			break;  \
		case 1:  \
			z = y * y;  \
			w = z * y;  \
			p1 = t0 + w * (t3 + w * (t6 + w * (t9 + w * t12)));  \
			p2 = t1 + w * (t4 + w * (t7 + w * (t10 + w * t13)));  \
			p3 = t2 + w * (t5 + w * (t8 + w * (t11 + w * t14)));  \
			p = z * p1 - (tt - w * (p2 + y * p3));  \
			r += (tf + p);  \
			break;  \
		case 2:  \
			p1 = y * (u0 + y * (u1 + y * (u2 + y * (u3 + y * (u4 + y * u5)))));  \
			p2 = one + y * (v1 + y * (v2 + y * (v3 + y * (v4 + y * v5))));  \
			r += (-(float) 0.5 * y + p1 / p2);  \
		}  \
	} else if (ix < 0x41000000) {  \
		i = (int) x;  \
		t = zero;  \
		y = x - (float) i;  \
		p = y * (s0 + y * (s1 + y * (s2 + y * (s3 + y * (s4 + y * (s5 + y * s6))))));  \
		q = one + y * (r1 + y * (r2 + y * (r3 + y * (r4 + y * (r5 + y * r6)))));  \
		r = .5f * y + p / q;  \
		z = one;  \
		switch (i) {  \
		case 7:  \
			z *= (y + (float) 6.0);  \
		case 6:  \
			z *= (y + (float) 5.0);  \
		case 5:  \
			z *= (y + (float) 4.0);  \
		case 4:  \
			z *= (y + (float) 3.0);  \
		case 3:  \
			z *= (y + (float) 2.0);  \
			r += native_log(z);  \
			break;  \
		}  \
		  \
	} else if (ix < 0x5c800000) {  \
		t = native_log(x);  \
		z = one / x;  \
		y = z * z;  \
		w = w0 + z * (w1 + y * (w2 + y * (w3 + y * (w4 + y * (w5 + y * w6)))));  \
		r = (x - .5f) * (t - one) + w;  \
	} else  \
		r = x * (native_log(x) - one);  \
	if (hx < 0)  \
		r = nadj - r;  \
	return r;
INLINE_OVERLOADABLE float lgamma_r(float x, global int *signgamp) { BODY; }
INLINE_OVERLOADABLE float lgamma_r(float x, local int *signgamp) { BODY; }
INLINE_OVERLOADABLE float lgamma_r(float x, private int *signgamp) { BODY; }
#undef BODY

INLINE_OVERLOADABLE float native_log10(float x) {
  return native_log2(x) * 0.3010299956f;
}
INLINE_OVERLOADABLE float log1p(float x) { return native_log(x + 1); }
INLINE_OVERLOADABLE float logb(float x) { return __gen_ocl_rndd(native_log2(x)); }
INLINE_OVERLOADABLE int ilogb(float x) { return __gen_ocl_rndd(native_log2(x)); }
INLINE_OVERLOADABLE float nan(uint code) {
  return NAN;
}
INLINE_OVERLOADABLE float native_powr(float x, float y) { return __gen_ocl_pow(x,y); }
INLINE_OVERLOADABLE float native_recip(float x) { return __gen_ocl_rcp(x); }
INLINE_OVERLOADABLE float native_tan(float x) {
  return native_sin(x) / native_cos(x);
}
INLINE_OVERLOADABLE float __gen_ocl_internal_tanpi(float x) {
  return native_tan(x * M_PI_F);
}
INLINE_OVERLOADABLE float native_exp(float x) { return __gen_ocl_pow(M_E_F, x); }
INLINE_OVERLOADABLE float native_exp2(float x) { return __gen_ocl_pow(2, x); }
INLINE_OVERLOADABLE float native_exp10(float x) { return __gen_ocl_pow(10, x); }
INLINE_OVERLOADABLE float __gen_ocl_internal_expm1(float x) { return __gen_ocl_pow(M_E_F, x) - 1; }
INLINE_OVERLOADABLE float __gen_ocl_internal_cbrt(float x) {
  return __gen_ocl_pow(x, 0.3333333333f);
}
#define BODY \
  *cosval = native_cos(x); \
  return native_sin(x);
INLINE_OVERLOADABLE float sincos(float x, global float *cosval) { BODY; }
INLINE_OVERLOADABLE float sincos(float x, local float *cosval) { BODY; }
INLINE_OVERLOADABLE float sincos(float x, private float *cosval) { BODY; }
#undef BODY

INLINE_OVERLOADABLE float __gen_ocl_internal_sinh(float x) {
  return (1 - native_exp(-2 * x)) / (2 * native_exp(-x));
}
INLINE_OVERLOADABLE float __gen_ocl_internal_cosh(float x) {
  return (1 + native_exp(-2 * x)) / (2 * native_exp(-x));
}
INLINE_OVERLOADABLE float __gen_ocl_internal_tanh(float x) {
  float y = native_exp(-2 * x);
  return (1 - y) / (1 + y);
}

typedef union
{
  float value;
  int word;
} ieee_float_shape_type;

#ifndef GET_FLOAT_WORD
#define GET_FLOAT_WORD(i,d)         \
do {                                \
  ieee_float_shape_type gf_u;       \
  gf_u.value = (d);                 \
  (i) = gf_u.word;                  \
} while (0)
#endif

INLINE_OVERLOADABLE float __gen_ocl_internal_asin(float x) {
  int hx, ix;
  GET_FLOAT_WORD(hx,x);
  ix = hx&0x7fffffff;
  if(ix == 0x3f800000) {
    return x * M_PI_2_F;  /* asin(|1|)=+-pi/2 with inexact */
  }
  if(ix > 0x3f800000) {            /* |x|>= 1 */
    return (x-x) / (x-x);          /* asin(|x|>1) is NaN */
  }
  if(ix < 0x32000000) {            /* if |x| < 2**-27 */
    if(HUGE_VALF + x > FLT_ONE) return x;   /* return x with inexact if x!=0*/
  }
  /* 1 > |x| >= 2**-27 */
  float sum = x, c = x, m = 1.0;
  int n = 1;
  do
  {
    c *= (2 * n - 1) * x * x;
    m *= (2 * n);
    sum += ( c / m / (2 * n + 1));
    n++;
  }while( n < 30);
  return sum;
}
INLINE_OVERLOADABLE float __gen_ocl_internal_asinpi(float x) {
  return __gen_ocl_internal_asin(x) / M_PI_F;
}
INLINE_OVERLOADABLE float __gen_ocl_internal_acos(float x) {
  return M_PI_2_F - __gen_ocl_internal_asin(x);
}
INLINE_OVERLOADABLE float __gen_ocl_internal_acospi(float x) {
  return __gen_ocl_internal_acos(x) / M_PI_F;
}
INLINE_OVERLOADABLE float __gen_ocl_internal_atan(float x) {
  float a = 0, c = 1;
  if (x <= -1) {
    a = - M_PI_2_F;
    x = 1 / x;
    c = -1;
  }
  if (x >= 1) {
    a = M_PI_2_F;
    x = 1 / x;
    c = -1;
  }
  return a + c * (x - __gen_ocl_pow(x, 3) / 3 + __gen_ocl_pow(x, 5) / 5 - __gen_ocl_pow(x, 7) / 7 + __gen_ocl_pow(x, 9) / 9 - __gen_ocl_pow(x, 11) / 11);
}
INLINE_OVERLOADABLE float __gen_ocl_internal_atanpi(float x) {
  return __gen_ocl_internal_atan(x) / M_PI_F;
}
INLINE_OVERLOADABLE float __gen_ocl_internal_asinh(float x) {
  return native_log(x + native_sqrt(x * x + 1));
}
INLINE_OVERLOADABLE float __gen_ocl_internal_acosh(float x) {
  return native_log(x + native_sqrt(x + 1) * native_sqrt(x - 1));
}
INLINE_OVERLOADABLE float __gen_ocl_internal_atanh(float x) {
  return 0.5f * native_sqrt((1 + x) / (1 - x));
}
INLINE_OVERLOADABLE float __gen_ocl_internal_copysign(float x, float y) {
  return x * y < 0 ? -x : x;
}
INLINE_OVERLOADABLE float __gen_ocl_internal_erf(float x) {
  return M_2_SQRTPI_F * (x - __gen_ocl_pow(x, 3) / 3 + __gen_ocl_pow(x, 5) / 10 - __gen_ocl_pow(x, 7) / 42 + __gen_ocl_pow(x, 9) / 216);
}
INLINE_OVERLOADABLE float __gen_ocl_internal_erfc(float x) {
  return 1 - __gen_ocl_internal_erf(x);
}

// XXX work-around PTX profile
#define sqrt native_sqrt
INLINE_OVERLOADABLE float rsqrt(float x) { return native_rsqrt(x); }
INLINE_OVERLOADABLE float __gen_ocl_internal_fabs(float x)  { return __gen_ocl_fabs(x); }
INLINE_OVERLOADABLE float __gen_ocl_internal_trunc(float x) { return __gen_ocl_rndz(x); }
INLINE_OVERLOADABLE float __gen_ocl_internal_round(float x) { return __gen_ocl_rnde(x); }
INLINE_OVERLOADABLE float __gen_ocl_internal_floor(float x) { return __gen_ocl_rndd(x); }
INLINE_OVERLOADABLE float __gen_ocl_internal_ceil(float x)  { return __gen_ocl_rndu(x); }
INLINE_OVERLOADABLE float __gen_ocl_internal_log(float x)   { return native_log(x); }
INLINE_OVERLOADABLE float __gen_ocl_internal_log2(float x)  { return native_log2(x); }
INLINE_OVERLOADABLE float __gen_ocl_internal_log10(float x) { return native_log10(x); }
INLINE_OVERLOADABLE float __gen_ocl_internal_exp(float x)   { return native_exp(x); }
INLINE_OVERLOADABLE float powr(float x, float y) { return __gen_ocl_pow(x,y); }
INLINE_OVERLOADABLE float fmod(float x, float y) { return x-y*__gen_ocl_rndz(x/y); }
INLINE_OVERLOADABLE float remainder(float x, float y) { return x-y*__gen_ocl_rnde(x/y); }
INLINE_OVERLOADABLE float __gen_ocl_internal_rint(float x) {
  return 2 * __gen_ocl_internal_round(x / 2);
}
// TODO use llvm intrinsics definitions
#define cos native_cos
#define cospi __gen_ocl_internal_cospi
#define cosh __gen_ocl_internal_cosh
#define acos __gen_ocl_internal_acos
#define acospi __gen_ocl_internal_acospi
#define acosh __gen_ocl_internal_acosh
#define sin native_sin
#define sinpi __gen_ocl_internal_sinpi
#define sinh __gen_ocl_internal_sinh
#define asin __gen_ocl_internal_asin
#define asinpi __gen_ocl_internal_asinpi
#define asinh __gen_ocl_internal_asinh
#define tan native_tan
#define tanpi __gen_ocl_internal_tanpi
#define tanh __gen_ocl_internal_tanh
#define atan __gen_ocl_internal_atan
#define atanpi __gen_ocl_internal_atanpi
#define atanh __gen_ocl_internal_atanh
#define pow powr
#define cbrt __gen_ocl_internal_cbrt
#define rint __gen_ocl_internal_rint
#define copysign __gen_ocl_internal_copysign
#define erf __gen_ocl_internal_erf
#define erfc __gen_ocl_internal_erfc

INLINE_OVERLOADABLE float mad(float a, float b, float c) {
  return a*b+c;
}

INLINE_OVERLOADABLE uint select(uint src0, uint src1, int cond) {
  return cond ? src1 : src0;
}
INLINE_OVERLOADABLE uint select(uint src0, uint src1, uint cond) {
  return cond ? src1 : src0;
}
INLINE_OVERLOADABLE int select(int src0, int src1, int cond) {
  return cond ? src1 : src0;
}
INLINE_OVERLOADABLE int select(int src0, int src1, uint cond) {
  return cond ? src1 : src0;
}
INLINE_OVERLOADABLE float select(float src0, float src1, int cond) {
  return cond ? src1 : src0;
}
INLINE_OVERLOADABLE float select(float src0, float src1, uint cond) {
  return cond ? src1 : src0;
}

// This will be optimized out by LLVM and will output LLVM select instructions
#define DECL_SELECT4(TYPE4, TYPE, COND_TYPE4, MASK) \
INLINE_OVERLOADABLE TYPE4 select(TYPE4 src0, TYPE4 src1, COND_TYPE4 cond) { \
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
INLINE_OVERLOADABLE float step(float edge, float x) {
  return x < edge ? 0.0 : 1.0;
}

#define DECL_MIN_MAX_CLAMP(TYPE) \
INLINE_OVERLOADABLE TYPE max(TYPE a, TYPE b) { \
  return a > b ? a : b; \
} \
INLINE_OVERLOADABLE TYPE min(TYPE a, TYPE b) { \
  return a < b ? a : b; \
} \
INLINE_OVERLOADABLE TYPE clamp(TYPE v, TYPE l, TYPE u) { \
  return max(min(v, u), l); \
}
DECL_MIN_MAX_CLAMP(float)
DECL_MIN_MAX_CLAMP(int)
DECL_MIN_MAX_CLAMP(short)
DECL_MIN_MAX_CLAMP(char)
DECL_MIN_MAX_CLAMP(uint)
DECL_MIN_MAX_CLAMP(unsigned short)
DECL_MIN_MAX_CLAMP(unsigned char)
DECL_MIN_MAX_CLAMP(long)
DECL_MIN_MAX_CLAMP(ulong)
#undef DECL_MIN_MAX_CLAMP

#define BODY \
  uint u = as_uint(x); \
  if ((u & 0x7FFFFFFFu) == 0) { \
    *exp = 0; \
    return x; \
  } \
  int e = (u >> 23) & 255; \
  if (e == 255) \
    return x; \
  *exp = e - 126; \
  u = (u & (0x807FFFFFu)) | 0x3F000000; \
  return as_float(u);
INLINE_OVERLOADABLE float frexp(float x, global int *exp) { BODY; }
INLINE_OVERLOADABLE float frexp(float x, local int *exp) { BODY; }
INLINE_OVERLOADABLE float frexp(float x, private int *exp) { BODY; }
#undef BODY

INLINE_OVERLOADABLE float nextafter(float x, float y) {
  uint hx = as_uint(x), ix = hx & 0x7FFFFFFF;
  uint hy = as_uint(y), iy = hy & 0x7FFFFFFF;
  if (ix > 0x7F800000 || iy > 0x7F800000)
    return nan(0u);
  if (hx == hy)
    return x;
  if (ix == 0)
    return as_float((hy & 0x80000000u) | 1);
  if (((0 == (hx & 0x80000000u)) && y > x) || ((hx & 0x80000000u) && y < x))
    hx ++;
  else
    hx --;
  return as_float(hx);
}

#define BODY \
  uint hx = as_uint(x), ix = hx & 0x7FFFFFFF; \
  if (ix > 0x7F800000) { \
    *i = nan(0u); \
    return nan(0u); \
  } \
  if (ix == 0x7F800000) { \
    *i = x; \
    return as_float(hx & 0x80000000u); \
  } \
  *i = __gen_ocl_rndz(x); \
  return x - *i;
INLINE_OVERLOADABLE float modf(float x, global float *i) { BODY; }
INLINE_OVERLOADABLE float modf(float x, local float *i) { BODY; }
INLINE_OVERLOADABLE float modf(float x, private float *i) { BODY; }
#undef BODY
INLINE_OVERLOADABLE float degrees(float radians) { return (180 / M_PI_F) * radians; }
INLINE_OVERLOADABLE float radians(float degrees) { return (M_PI_F / 180) * degrees; }

INLINE_OVERLOADABLE float smoothstep(float e0, float e1, float x) {
  x = clamp((x - e0) / (e1 - e0), 0.f, 1.f);
  return x * x * (3 - 2 * x);
}

INLINE_OVERLOADABLE float sign(float x) {
  if(x > 0)
    return 1;
  if(x < 0)
    return -1;
  if(x == -0.f)
    return -0.f;
  return 0.f;
}

INLINE_OVERLOADABLE float __gen_ocl_internal_fmax(float a, float b) { return max(a,b); }
INLINE_OVERLOADABLE float __gen_ocl_internal_fmin(float a, float b) { return min(a,b); }
INLINE_OVERLOADABLE float __gen_ocl_internal_maxmag(float x, float y) {
  float a = __gen_ocl_fabs(x), b = __gen_ocl_fabs(y);
  return a > b ? x : b > a ? y : max(x, y);
}
INLINE_OVERLOADABLE float __gen_ocl_internal_minmag(float x, float y) {
  float a = __gen_ocl_fabs(x), b = __gen_ocl_fabs(y);
  return a < b ? x : b < a ? y : min(x, y);
}
INLINE_OVERLOADABLE float mix(float x, float y, float a) { return x + (y-x)*a;}
INLINE_OVERLOADABLE float __gen_ocl_internal_fdim(float x, float y) {
  return __gen_ocl_internal_fmax(x, y) - y;
}
#define BODY \
  *p = __gen_ocl_internal_floor(x); \
  return __gen_ocl_internal_fmin(x - *p, 0x1.FFFFFep-1F);
INLINE_OVERLOADABLE float fract(float x, global float *p) { BODY; }
INLINE_OVERLOADABLE float fract(float x, local float *p) { BODY; }
INLINE_OVERLOADABLE float fract(float x, private float *p) { BODY; }
#undef BODY

#define BODY \
  uint hx = as_uint(x), ix = hx & 0x7FFFFFFF, hy = as_uint(y), iy = hy & 0x7FFFFFFF; \
  if (ix > 0x7F800000 || iy > 0x7F800000 || ix == 0x7F800000 || iy == 0) \
    return nan(0u); \
  float k = x / y; \
  int q =  __gen_ocl_rnde(k); \
  *quo = q >= 0 ? (q & 127) : (q | 0xFFFFFF80u); \
  float r = x - q * y; \
  uint hr = as_uint(r), ir = hr & 0x7FFFFFFF; \
  if (ir == 0) \
    hr = ir | (hx & 0x80000000u); \
  return as_float(hr);
INLINE_OVERLOADABLE float remquo(float x, float y, global int *quo) { BODY; }
INLINE_OVERLOADABLE float remquo(float x, float y, local int *quo) { BODY; }
INLINE_OVERLOADABLE float remquo(float x, float y, private int *quo) { BODY; }
#undef BODY
INLINE_OVERLOADABLE float native_divide(float x, float y) { return x/y; }
INLINE_OVERLOADABLE float ldexp(float x, int n) {
  return __gen_ocl_pow(2, n) * x;
}
INLINE_OVERLOADABLE float pown(float x, int n) {
  if (x == 0 && n == 0)
    return 1;
  return powr(x, n);
}
INLINE_OVERLOADABLE float rootn(float x, int n) {
  return powr(x, 1.f / n);
}

/////////////////////////////////////////////////////////////////////////////
// Geometric functions (see 6.11.5 of OCL 1.1 spec)
/////////////////////////////////////////////////////////////////////////////
INLINE_OVERLOADABLE float dot(float2 p0, float2 p1) {
  return mad(p0.x,p1.x,p0.y*p1.y);
}
INLINE_OVERLOADABLE float dot(float3 p0, float3 p1) {
  return mad(p0.x,p1.x,mad(p0.z,p1.z,p0.y*p1.y));
}
INLINE_OVERLOADABLE float dot(float4 p0, float4 p1) {
  return mad(p0.x,p1.x,mad(p0.w,p1.w,mad(p0.z,p1.z,p0.y*p1.y)));
}

INLINE_OVERLOADABLE float dot(float8 p0, float8 p1) {
  return mad(p0.x,p1.x,mad(p0.s7,p1.s7, mad(p0.s6,p1.s6,mad(p0.s5,p1.s5,
         mad(p0.s4,p1.s4,mad(p0.w,p1.w, mad(p0.z,p1.z,p0.y*p1.y)))))));
}
INLINE_OVERLOADABLE float dot(float16 p0, float16 p1) {
  return mad(p0.sc,p1.sc,mad(p0.sd,p1.sd,mad(p0.se,p1.se,mad(p0.sf,p1.sf,
         mad(p0.s8,p1.s8,mad(p0.s9,p1.s9,mad(p0.sa,p1.sa,mad(p0.sb,p1.sb,
         mad(p0.x,p1.x,mad(p0.s7,p1.s7, mad(p0.s6,p1.s6,mad(p0.s5,p1.s5,
         mad(p0.s4,p1.s4,mad(p0.w,p1.w, mad(p0.z,p1.z,p0.y*p1.y)))))))))))))));
}

INLINE_OVERLOADABLE float length(float x) { return __gen_ocl_fabs(x); }
INLINE_OVERLOADABLE float length(float2 x) { return sqrt(dot(x,x)); }
INLINE_OVERLOADABLE float length(float3 x) { return sqrt(dot(x,x)); }
INLINE_OVERLOADABLE float length(float4 x) { return sqrt(dot(x,x)); }
INLINE_OVERLOADABLE float length(float8 x) { return sqrt(dot(x,x)); }
INLINE_OVERLOADABLE float length(float16 x) { return sqrt(dot(x,x)); }
INLINE_OVERLOADABLE float distance(float x, float y) { return length(x-y); }
INLINE_OVERLOADABLE float distance(float2 x, float2 y) { return length(x-y); }
INLINE_OVERLOADABLE float distance(float3 x, float3 y) { return length(x-y); }
INLINE_OVERLOADABLE float distance(float4 x, float4 y) { return length(x-y); }
INLINE_OVERLOADABLE float distance(float8 x, float8 y) { return length(x-y); }
INLINE_OVERLOADABLE float distance(float16 x, float16 y) { return length(x-y); }
INLINE_OVERLOADABLE float normalize(float x) { return 1.f; }
INLINE_OVERLOADABLE float2 normalize(float2 x) { return x * rsqrt(dot(x, x)); }
INLINE_OVERLOADABLE float3 normalize(float3 x) { return x * rsqrt(dot(x, x)); }
INLINE_OVERLOADABLE float4 normalize(float4 x) { return x * rsqrt(dot(x, x)); }
INLINE_OVERLOADABLE float8 normalize(float8 x) { return x * rsqrt(dot(x, x)); }
INLINE_OVERLOADABLE float16 normalize(float16 x) { return x * rsqrt(dot(x, x)); }

INLINE_OVERLOADABLE float fast_length(float x) { return __gen_ocl_fabs(x); }
INLINE_OVERLOADABLE float fast_length(float2 x) { return sqrt(dot(x,x)); }
INLINE_OVERLOADABLE float fast_length(float3 x) { return sqrt(dot(x,x)); }
INLINE_OVERLOADABLE float fast_length(float4 x) { return sqrt(dot(x,x)); }
INLINE_OVERLOADABLE float fast_length(float8 x) { return sqrt(dot(x,x)); }
INLINE_OVERLOADABLE float fast_length(float16 x) { return sqrt(dot(x,x)); }
INLINE_OVERLOADABLE float fast_distance(float x, float y) { return length(x-y); }
INLINE_OVERLOADABLE float fast_distance(float2 x, float2 y) { return length(x-y); }
INLINE_OVERLOADABLE float fast_distance(float3 x, float3 y) { return length(x-y); }
INLINE_OVERLOADABLE float fast_distance(float4 x, float4 y) { return length(x-y); }
INLINE_OVERLOADABLE float fast_distance(float8 x, float8 y) { return length(x-y); }
INLINE_OVERLOADABLE float fast_distance(float16 x, float16 y) { return length(x-y); }
INLINE_OVERLOADABLE float fast_normalize(float x) { return 1.f; }
INLINE_OVERLOADABLE float2 fast_normalize(float2 x) { return x * rsqrt(dot(x, x)); }
INLINE_OVERLOADABLE float3 fast_normalize(float3 x) { return x * rsqrt(dot(x, x)); }
INLINE_OVERLOADABLE float4 fast_normalize(float4 x) { return x * rsqrt(dot(x, x)); }
INLINE_OVERLOADABLE float8 fast_normalize(float8 x) { return x * rsqrt(dot(x, x)); }
INLINE_OVERLOADABLE float16 fast_normalize(float16 x) { return x * rsqrt(dot(x, x)); }

INLINE_OVERLOADABLE float3 cross(float3 v0, float3 v1) {
   return v0.yzx*v1.zxy-v0.zxy*v1.yzx;
}
INLINE_OVERLOADABLE float4 cross(float4 v0, float4 v1) {
   return (float4)(v0.yzx*v1.zxy-v0.zxy*v1.yzx, 0.f);
}

/////////////////////////////////////////////////////////////////////////////
// Vector loads and stores
/////////////////////////////////////////////////////////////////////////////

// These loads and stores will use untyped reads and writes, so we can just
// cast to vector loads / stores. Not C99 compliant BTW due to aliasing issue.
// Well we do not care, we do not activate TBAA in the compiler
#define DECL_UNTYPED_RW_SPACE_N(TYPE, DIM, SPACE) \
INLINE_OVERLOADABLE TYPE##DIM vload##DIM(size_t offset, const SPACE TYPE *p) { \
  return *(SPACE TYPE##DIM *) (p + DIM * offset); \
} \
INLINE_OVERLOADABLE void vstore##DIM(TYPE##DIM v, size_t offset, SPACE TYPE *p) { \
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
DECL_UNTYPED_RW_ALL(double)

#undef DECL_UNTYPED_RW_ALL
#undef DECL_UNTYPED_RW_ALL_SPACE
#undef DECL_UNTYPED_RW_SPACE_N

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
#define exp2 native_exp2
#define exp10 native_exp10
#define expm1 __gen_ocl_internal_expm1
#define fmin __gen_ocl_internal_fmin
#define fmax __gen_ocl_internal_fmax
#define fma mad
#define fdim __gen_ocl_internal_fdim
#define maxmag __gen_ocl_internal_maxmag
#define minmag __gen_ocl_internal_minmag

/////////////////////////////////////////////////////////////////////////////
// Miscellaneous Vector Functions (see 6.11.12 of OCL 1.1 spec)
/////////////////////////////////////////////////////////////////////////////
#define DEC2(TYPE, XTYPE) \
  INLINE_OVERLOADABLE TYPE##2 shuffle(XTYPE x, uint2 mask) { \
    TYPE##2 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & (vec_step(x) - 1)]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & (vec_step(x) - 1)]; \
    return y; \
  }

#define DEC4(TYPE, XTYPE) \
  INLINE_OVERLOADABLE TYPE##4 shuffle(XTYPE x, uint4 mask) { \
    TYPE##4 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & (vec_step(x) - 1)]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & (vec_step(x) - 1)]; \
    y.s2 = ((TYPE *) &x)[mask.s2 & (vec_step(x) - 1)]; \
    y.s3 = ((TYPE *) &x)[mask.s3 & (vec_step(x) - 1)]; \
    return y; \
  }

#define DEC8(TYPE, XTYPE) \
  INLINE_OVERLOADABLE TYPE##8 shuffle(XTYPE x, uint8 mask) { \
    TYPE##8 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & (vec_step(x) - 1)]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & (vec_step(x) - 1)]; \
    y.s2 = ((TYPE *) &x)[mask.s2 & (vec_step(x) - 1)]; \
    y.s3 = ((TYPE *) &x)[mask.s3 & (vec_step(x) - 1)]; \
    y.s4 = ((TYPE *) &x)[mask.s4 & (vec_step(x) - 1)]; \
    y.s5 = ((TYPE *) &x)[mask.s5 & (vec_step(x) - 1)]; \
    y.s6 = ((TYPE *) &x)[mask.s6 & (vec_step(x) - 1)]; \
    y.s7 = ((TYPE *) &x)[mask.s7 & (vec_step(x) - 1)]; \
    return y; \
  }

#define DEC16(TYPE, XTYPE) \
  INLINE_OVERLOADABLE TYPE##16 shuffle(XTYPE x, uint16 mask) { \
    TYPE##16 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & (vec_step(x) - 1)]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & (vec_step(x) - 1)]; \
    y.s2 = ((TYPE *) &x)[mask.s2 & (vec_step(x) - 1)]; \
    y.s3 = ((TYPE *) &x)[mask.s3 & (vec_step(x) - 1)]; \
    y.s4 = ((TYPE *) &x)[mask.s4 & (vec_step(x) - 1)]; \
    y.s5 = ((TYPE *) &x)[mask.s5 & (vec_step(x) - 1)]; \
    y.s6 = ((TYPE *) &x)[mask.s6 & (vec_step(x) - 1)]; \
    y.s7 = ((TYPE *) &x)[mask.s7 & (vec_step(x) - 1)]; \
    y.s8 = ((TYPE *) &x)[mask.s8 & (vec_step(x) - 1)]; \
    y.s9 = ((TYPE *) &x)[mask.s9 & (vec_step(x) - 1)]; \
    y.sa = ((TYPE *) &x)[mask.sa & (vec_step(x) - 1)]; \
    y.sb = ((TYPE *) &x)[mask.sb & (vec_step(x) - 1)]; \
    y.sc = ((TYPE *) &x)[mask.sc & (vec_step(x) - 1)]; \
    y.sd = ((TYPE *) &x)[mask.sd & (vec_step(x) - 1)]; \
    y.se = ((TYPE *) &x)[mask.se & (vec_step(x) - 1)]; \
    y.sf = ((TYPE *) &x)[mask.sf & (vec_step(x) - 1)]; \
    return y; \
  }

#define DEF(TYPE) \
  DEC2(TYPE, TYPE##2); DEC2(TYPE, TYPE##4); DEC2(TYPE, TYPE##8); DEC2(TYPE, TYPE##16) \
  DEC4(TYPE, TYPE##2); DEC4(TYPE, TYPE##4); DEC4(TYPE, TYPE##8); DEC4(TYPE, TYPE##16) \
  DEC8(TYPE, TYPE##2); DEC8(TYPE, TYPE##4); DEC8(TYPE, TYPE##8); DEC8(TYPE, TYPE##16) \
  DEC16(TYPE, TYPE##2); DEC16(TYPE, TYPE##4); DEC16(TYPE, TYPE##8); DEC16(TYPE, TYPE##16)
DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(int)
DEF(uint)
DEF(float)
#undef DEF
#undef DEC2
#undef DEC4
#undef DEC8
#undef DEC16

#define DEC2(TYPE, ARGTYPE, TEMPTYPE) \
  INLINE_OVERLOADABLE TYPE##2 shuffle2(ARGTYPE x, ARGTYPE y, uint2 mask) { \
    return shuffle((TEMPTYPE)(x, y), mask); \
  }

#define DEC2X(TYPE) \
  INLINE_OVERLOADABLE TYPE##2 shuffle2(TYPE##16 x, TYPE##16 y, uint2 mask) { \
    TYPE##2 z; \
    z.s0 = mask.s0 < 16 ? ((TYPE *)&x)[mask.s0] : ((TYPE *)&y)[mask.s0 & 15]; \
    z.s1 = mask.s1 < 16 ? ((TYPE *)&x)[mask.s1] : ((TYPE *)&y)[mask.s1 & 15]; \
    return z; \
  }

#define DEC4(TYPE, ARGTYPE, TEMPTYPE) \
  INLINE_OVERLOADABLE TYPE##4 shuffle2(ARGTYPE x, ARGTYPE y, uint4 mask) { \
    return shuffle((TEMPTYPE)(x, y), mask); \
  }

#define DEC4X(TYPE) \
  INLINE_OVERLOADABLE TYPE##4 shuffle2(TYPE##16 x, TYPE##16 y, uint4 mask) { \
    TYPE##4 z; \
    z.s0 = mask.s0 < 16 ? ((TYPE *)&x)[mask.s0] : ((TYPE *)&y)[mask.s0 & 15]; \
    z.s1 = mask.s1 < 16 ? ((TYPE *)&x)[mask.s1] : ((TYPE *)&y)[mask.s1 & 15]; \
    z.s2 = mask.s2 < 16 ? ((TYPE *)&x)[mask.s2] : ((TYPE *)&y)[mask.s2 & 15]; \
    z.s3 = mask.s3 < 16 ? ((TYPE *)&x)[mask.s3] : ((TYPE *)&y)[mask.s3 & 15]; \
    return z; \
  }

#define DEC8(TYPE, ARGTYPE, TEMPTYPE) \
  INLINE_OVERLOADABLE TYPE##8 shuffle2(ARGTYPE x, ARGTYPE y, uint8 mask) { \
    return shuffle((TEMPTYPE)(x, y), mask); \
  }

#define DEC8X(TYPE) \
  INLINE_OVERLOADABLE TYPE##8 shuffle2(TYPE##16 x, TYPE##16 y, uint8 mask) { \
    TYPE##8 z; \
    z.s0 = mask.s0 < 16 ? ((TYPE *)&x)[mask.s0] : ((TYPE *)&y)[mask.s0 & 15]; \
    z.s1 = mask.s1 < 16 ? ((TYPE *)&x)[mask.s1] : ((TYPE *)&y)[mask.s1 & 15]; \
    z.s2 = mask.s2 < 16 ? ((TYPE *)&x)[mask.s2] : ((TYPE *)&y)[mask.s2 & 15]; \
    z.s3 = mask.s3 < 16 ? ((TYPE *)&x)[mask.s3] : ((TYPE *)&y)[mask.s3 & 15]; \
    z.s4 = mask.s4 < 16 ? ((TYPE *)&x)[mask.s4] : ((TYPE *)&y)[mask.s4 & 15]; \
    z.s5 = mask.s5 < 16 ? ((TYPE *)&x)[mask.s5] : ((TYPE *)&y)[mask.s5 & 15]; \
    z.s6 = mask.s6 < 16 ? ((TYPE *)&x)[mask.s6] : ((TYPE *)&y)[mask.s6 & 15]; \
    z.s7 = mask.s7 < 16 ? ((TYPE *)&x)[mask.s7] : ((TYPE *)&y)[mask.s7 & 15]; \
    return z; \
  }

#define DEC16(TYPE, ARGTYPE, TEMPTYPE) \
  INLINE_OVERLOADABLE TYPE##16 shuffle2(ARGTYPE x, ARGTYPE y, uint16 mask) { \
    return shuffle((TEMPTYPE)(x, y), mask); \
  }

#define DEC16X(TYPE) \
  INLINE_OVERLOADABLE TYPE##16 shuffle2(TYPE##16 x, TYPE##16 y, uint16 mask) { \
    TYPE##16 z; \
    z.s0 = mask.s0 < 16 ? ((TYPE *)&x)[mask.s0] : ((TYPE *)&y)[mask.s0 & 15]; \
    z.s1 = mask.s1 < 16 ? ((TYPE *)&x)[mask.s1] : ((TYPE *)&y)[mask.s1 & 15]; \
    z.s2 = mask.s2 < 16 ? ((TYPE *)&x)[mask.s2] : ((TYPE *)&y)[mask.s2 & 15]; \
    z.s3 = mask.s3 < 16 ? ((TYPE *)&x)[mask.s3] : ((TYPE *)&y)[mask.s3 & 15]; \
    z.s4 = mask.s4 < 16 ? ((TYPE *)&x)[mask.s4] : ((TYPE *)&y)[mask.s4 & 15]; \
    z.s5 = mask.s5 < 16 ? ((TYPE *)&x)[mask.s5] : ((TYPE *)&y)[mask.s5 & 15]; \
    z.s6 = mask.s6 < 16 ? ((TYPE *)&x)[mask.s6] : ((TYPE *)&y)[mask.s6 & 15]; \
    z.s7 = mask.s7 < 16 ? ((TYPE *)&x)[mask.s7] : ((TYPE *)&y)[mask.s7 & 15]; \
    z.s8 = mask.s8 < 16 ? ((TYPE *)&x)[mask.s8] : ((TYPE *)&y)[mask.s8 & 15]; \
    z.s9 = mask.s9 < 16 ? ((TYPE *)&x)[mask.s9] : ((TYPE *)&y)[mask.s9 & 15]; \
    z.sa = mask.sa < 16 ? ((TYPE *)&x)[mask.sa] : ((TYPE *)&y)[mask.sa & 15]; \
    z.sb = mask.sb < 16 ? ((TYPE *)&x)[mask.sb] : ((TYPE *)&y)[mask.sb & 15]; \
    z.sc = mask.sc < 16 ? ((TYPE *)&x)[mask.sc] : ((TYPE *)&y)[mask.sc & 15]; \
    z.sd = mask.sd < 16 ? ((TYPE *)&x)[mask.sd] : ((TYPE *)&y)[mask.sd & 15]; \
    z.se = mask.se < 16 ? ((TYPE *)&x)[mask.se] : ((TYPE *)&y)[mask.se & 15]; \
    z.sf = mask.sf < 16 ? ((TYPE *)&x)[mask.sf] : ((TYPE *)&y)[mask.sf & 15]; \
    return z; \
  }

#define DEF(TYPE) \
  DEC2(TYPE, TYPE##2, TYPE##4) \
  DEC2(TYPE, TYPE##4, TYPE##8) \
  DEC2(TYPE, TYPE##8, TYPE##16) \
  DEC2X(TYPE) \
  DEC4(TYPE, TYPE##2, TYPE##4) \
  DEC4(TYPE, TYPE##4, TYPE##8) \
  DEC4(TYPE, TYPE##8, TYPE##16) \
  DEC4X(TYPE) \
  DEC8(TYPE, TYPE##2, TYPE##4) \
  DEC8(TYPE, TYPE##4, TYPE##8) \
  DEC8(TYPE, TYPE##8, TYPE##16) \
  DEC8X(TYPE) \
  DEC16(TYPE, TYPE##2, TYPE##4) \
  DEC16(TYPE, TYPE##4, TYPE##8) \
  DEC16(TYPE, TYPE##8, TYPE##16) \
  DEC16X(TYPE)

DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(int)
DEF(uint)
DEF(float)
#undef DEF
#undef DEC2
#undef DEC2X
#undef DEC4
#undef DEC4X
#undef DEC8
#undef DEC8X
#undef DEC16
#undef DEC16X
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

INLINE void mem_fence(cl_mem_fence_flags flags) {
}
INLINE void read_mem_fence(cl_mem_fence_flags flags) {
}
INLINE void write_mem_fence(cl_mem_fence_flags flags) {
}

/////////////////////////////////////////////////////////////////////////////
// Async Copies and prefetch
/////////////////////////////////////////////////////////////////////////////
#define BODY(SRC_STRIDE, DST_STRIDE) \
  uint size = get_local_size(2) * get_local_size(1) * get_local_size(0); \
  uint count = num / size;  \
  uint offset = get_local_id(2) * get_local_size(1) + get_local_id(1);  \
  offset = offset * get_local_size(0) + get_local_id(0); \
  for(uint i=0; i<count; i+=1) { \
    *(dst + offset * DST_STRIDE) = *(src + offset * SRC_STRIDE); \
    offset += size;                                 \
  } \
  if(offset < num) \
    *(dst + offset * DST_STRIDE) = *(src + offset * SRC_STRIDE); \
  return 0;

#define DEFN(TYPE) \
INLINE_OVERLOADABLE event_t async_work_group_copy (local TYPE *dst,  const global TYPE *src, \
										    size_t num, event_t event) { \
  BODY(1, 1); \
} \
INLINE_OVERLOADABLE event_t async_work_group_copy (global TYPE *dst,  const local TYPE *src, \
										    size_t num, event_t event) { \
  BODY(1, 1); \
} \
INLINE_OVERLOADABLE event_t async_work_group_strided_copy (local TYPE *dst,  const global TYPE *src, \
										            size_t num, size_t src_stride, event_t event) { \
  BODY(src_stride, 1); \
} \
INLINE_OVERLOADABLE event_t async_work_group_strided_copy (global TYPE *dst,  const local TYPE *src, \
										            size_t num, size_t dst_stride, event_t event) { \
  BODY(1, dst_stride); \
}
#define DEF(TYPE) \
  DEFN(TYPE); DEFN(TYPE##2); DEFN(TYPE##3); DEFN(TYPE##4); DEFN(TYPE##8); DEFN(TYPE##16);
DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(int)
DEF(uint)
DEF(float)
#undef BODY
#undef DEFN
#undef DEF

INLINE void wait_group_events (int num_events, event_t *event_list) {
  barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
}

#define DEFN(TYPE) \
INLINE_OVERLOADABLE void prefetch(const global TYPE *p, size_t num) { }
#define DEF(TYPE) \
DEFN(TYPE); DEFN(TYPE##2); DEFN(TYPE##3); DEFN(TYPE##4); DEFN(TYPE##8); DEFN(TYPE##16)
DEF(char);
DEF(uchar);
DEF(short);
DEF(ushort);
DEF(int);
DEF(uint);
DEF(long);
DEF(ulong);
DEF(float);
#undef DEFN
#undef DEF

/////////////////////////////////////////////////////////////////////////////
// Atomic functions
/////////////////////////////////////////////////////////////////////////////
OVERLOADABLE uint __gen_ocl_atomic_add(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_add(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_sub(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_sub(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_and(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_and(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_or(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_or(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_xor(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_xor(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_xchg(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_xchg(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_inc(__global uint *p);
OVERLOADABLE uint __gen_ocl_atomic_inc(__local uint *p);
OVERLOADABLE uint __gen_ocl_atomic_dec(__global uint *p);
OVERLOADABLE uint __gen_ocl_atomic_dec(__local uint *p);
OVERLOADABLE uint __gen_ocl_atomic_cmpxchg(__global uint *p, uint cmp, uint val);
OVERLOADABLE uint __gen_ocl_atomic_cmpxchg(__local uint *p, uint cmp, uint val);
OVERLOADABLE uint __gen_ocl_atomic_imin(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_imin(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_imax(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_imax(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_umin(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_umin(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_umax(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_umax(__local uint *p, uint val);

#define DECL_ATOMIC_OP_SPACE(NAME, TYPE, SPACE, PREFIX)                        \
  INLINE_OVERLOADABLE TYPE atomic_##NAME (volatile SPACE TYPE *p, TYPE val) { \
    return (TYPE)__gen_ocl_##PREFIX##NAME((SPACE uint *)p, val);            \
  }

#define DECL_ATOMIC_OP_TYPE(NAME, TYPE, PREFIX) \
  DECL_ATOMIC_OP_SPACE(NAME, TYPE, __global, PREFIX) \
  DECL_ATOMIC_OP_SPACE(NAME, TYPE, __local, PREFIX) \

#define DECL_ATOMIC_OP(NAME) \
  DECL_ATOMIC_OP_TYPE(NAME, uint, atomic_)              \
  DECL_ATOMIC_OP_TYPE(NAME, int, atomic_)

DECL_ATOMIC_OP(add)
DECL_ATOMIC_OP(sub)
DECL_ATOMIC_OP(and)
DECL_ATOMIC_OP(or)
DECL_ATOMIC_OP(xor)
DECL_ATOMIC_OP(xchg)
DECL_ATOMIC_OP_TYPE(xchg, float, atomic_)
DECL_ATOMIC_OP_TYPE(min, int, atomic_i)
DECL_ATOMIC_OP_TYPE(max, int, atomic_i)
DECL_ATOMIC_OP_TYPE(min, uint, atomic_u)
DECL_ATOMIC_OP_TYPE(max, uint, atomic_u)

#undef DECL_ATOMIC_OP
#undef DECL_ATOMIC_OP_TYPE
#undef DECL_ATOMIC_OP_SPACE

#define DECL_ATOMIC_OP_SPACE(NAME, TYPE, SPACE) \
  INLINE_OVERLOADABLE TYPE atomic_##NAME (volatile SPACE TYPE *p) { \
    return (TYPE)__gen_ocl_atomic_##NAME((SPACE uint *)p); \
  }

#define DECL_ATOMIC_OP_TYPE(NAME, TYPE) \
  DECL_ATOMIC_OP_SPACE(NAME, TYPE, __global) \
  DECL_ATOMIC_OP_SPACE(NAME, TYPE, __local)

#define DECL_ATOMIC_OP(NAME) \
  DECL_ATOMIC_OP_TYPE(NAME, uint) \
  DECL_ATOMIC_OP_TYPE(NAME, int)

DECL_ATOMIC_OP(inc)
DECL_ATOMIC_OP(dec)

#undef DECL_ATOMIC_OP
#undef DECL_ATOMIC_OP_TYPE
#undef DECL_ATOMIC_OP_SPACE

#define DECL_ATOMIC_OP_SPACE(NAME, TYPE, SPACE)  \
  INLINE_OVERLOADABLE TYPE atomic_##NAME (volatile SPACE TYPE *p, TYPE cmp, TYPE val) { \
    return (TYPE)__gen_ocl_atomic_##NAME((SPACE uint *)p, (uint)cmp, (uint)val); \
  }

#define DECL_ATOMIC_OP_TYPE(NAME, TYPE) \
  DECL_ATOMIC_OP_SPACE(NAME, TYPE, __global) \
  DECL_ATOMIC_OP_SPACE(NAME, TYPE, __local)

#define DECL_ATOMIC_OP(NAME) \
  DECL_ATOMIC_OP_TYPE(NAME, uint) \
  DECL_ATOMIC_OP_TYPE(NAME, int)

DECL_ATOMIC_OP(cmpxchg)

#undef DECL_ATOMIC_OP
#undef DECL_ATOMIC_OP_TYPE
#undef DECL_ATOMIC_OP_SPACE

// XXX for conformance test
// The following atom_xxx api is on OpenCL spec 1.0.
// But the conformance test suite will test them anyway.
#define atom_add atomic_add
#define atom_sub atomic_sub
#define atom_and atomic_and
#define atom_or atomic_or
#define atom_xor atomic_xor
#define atom_xchg atomic_xchg
#define atom_min atomic_min
#define atom_max atomic_max
#define atom_inc atomic_inc
#define atom_dec atomic_dec
#define atom_cmpxchg atomic_cmpxchg

/////////////////////////////////////////////////////////////////////////////
// Force the compilation to SIMD8 or SIMD16
/////////////////////////////////////////////////////////////////////////////

int __gen_ocl_force_simd8(void);
int __gen_ocl_force_simd16(void);

#define NULL ((void*)0)

// ##BEGIN_COMMON_DEFINES##
// ##END_COMMON_DEFINES##

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
int __gen_ocl_get_image_width(uint surface_id);
int __gen_ocl_get_image_height(uint surface_id);
int __gen_ocl_get_image_channel_data_type(uint surface_id);
int __gen_ocl_get_image_channel_order(uint surface_id);
int __gen_ocl_get_image_depth(uint surface_id);

#define GET_IMAGE(cl_image, surface_id) \
    uint surface_id = (uint)cl_image

#define DECL_READ_IMAGE(type, suffix, coord_type) \
  INLINE_OVERLOADABLE type read_image ##suffix(image2d_t cl_image, sampler_t sampler, coord_type coord) \
  {\
    GET_IMAGE(cl_image, surface_id);\
    return __gen_ocl_read_image ##suffix(surface_id, sampler, coord.s0, coord.s1);\
  }

#define DECL_READ_IMAGE_NOSAMPLER(type, suffix, coord_type) \
  INLINE_OVERLOADABLE type read_image ##suffix(image2d_t cl_image, coord_type coord) \
  {\
    GET_IMAGE(cl_image, surface_id);\
    return __gen_ocl_read_image ##suffix(surface_id, CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_NONE|CLK_FILTER_NEAREST, coord.s0, coord.s1);\
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
  DECL_READ_IMAGE_NOSAMPLER(type, suffix, int2) \
  DECL_WRITE_IMAGE(type, suffix, int2)   \
  DECL_WRITE_IMAGE(type, suffix, float2)

DECL_IMAGE(int4, i)
DECL_IMAGE(uint4, ui)
DECL_IMAGE(float4, f)

#undef DECL_IMAGE
#undef DECL_READ_IMAGE
#undef DECL_READ_IMAGE_NOSAMPLER
#undef DECL_WRITE_IMAGE

#define DECL_IMAGE_INFO(image_type)    \
  INLINE_OVERLOADABLE  int get_image_width(image_type image) \
  { \
    GET_IMAGE(image, surface_id);\
    return __gen_ocl_get_image_width(surface_id);\
  } \
  INLINE_OVERLOADABLE  int get_image_height(image_type image)\
  { \
    GET_IMAGE(image, surface_id);\
    return __gen_ocl_get_image_height(surface_id); \
  } \
  INLINE_OVERLOADABLE  int get_image_channel_data_type(image_type image)\
  { \
    GET_IMAGE(image, surface_id);\
    return __gen_ocl_get_image_channel_data_type(surface_id); \
  }\
  INLINE_OVERLOADABLE  int get_image_channel_order(image_type image)\
  { \
    GET_IMAGE(image, surface_id);\
    return __gen_ocl_get_image_channel_order(surface_id); \
  }

DECL_IMAGE_INFO(image2d_t)
DECL_IMAGE_INFO(image3d_t)

INLINE_OVERLOADABLE  int get_image_depth(image3d_t image)
  {
   GET_IMAGE(image, surface_id);
   return __gen_ocl_get_image_depth(surface_id);
  }

INLINE_OVERLOADABLE  int2 get_image_dim(image2d_t image)
  { return (int2){get_image_width(image), get_image_height(image)}; }

INLINE_OVERLOADABLE  int4 get_image_dim(image3d_t image)
  { return (int4){get_image_width(image), get_image_height(image), get_image_depth(image), 0}; }
#if 0
/* The following functions are not implemented yet. */
DECL_IMAGE_INFO(image1d_t)
DECL_IMAGE_INFO(image1d_buffer_t)
DECL_IMAGE_INFO(image1d_array_t)
DECL_IMAGE_INFO(image2d_array_t)

INLINE_OVERLOADABLE  int2 get_image_dim(image2d_array_t image)
  { return __gen_ocl_get_image_dim(image); }

INLINE_OVERLOADABLE  int4 get_image_dim(image2d_array_t image)
  { return __gen_ocl_get_image_dim(image); }

INLINE_OVERLOADABLE  size_t get_image_array_size(image2d_array_t image)
  { return __gen_ocl_get_image_array_size(image); }

INLINE_OVERLOADABLE  size_t get_image_array_size(image1d_array_t image)
  { return __gen_ocl_get_image_array_size(image); }
#endif

#define DECL_READ_IMAGE(type, suffix, coord_type) \
  INLINE_OVERLOADABLE type read_image ## suffix(image3d_t cl_image, sampler_t sampler, coord_type coord) \
  {\
    GET_IMAGE(cl_image, surface_id);\
    return __gen_ocl_read_image ## suffix(surface_id, (uint)sampler, coord.s0, coord.s1, coord.s2);\
  }

#define DECL_READ_IMAGE_NOSAMPLER(type, suffix, coord_type) \
  INLINE_OVERLOADABLE type read_image ## suffix(image3d_t cl_image, coord_type coord) \
  {\
    GET_IMAGE(cl_image, surface_id);\
    return __gen_ocl_read_image ## suffix(surface_id, CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_NONE|CLK_FILTER_NEAREST, coord.s0, coord.s1, coord.s2);\
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
  DECL_READ_IMAGE_NOSAMPLER(type, suffix, int4) \
  DECL_WRITE_IMAGE(type, suffix, int4)   \
  DECL_WRITE_IMAGE(type, suffix, float4)

DECL_IMAGE(int4, i)
DECL_IMAGE(uint4, ui)
DECL_IMAGE(float4, f)



#pragma OPENCL EXTENSION cl_khr_fp64 : disable

#undef DECL_IMAGE
#undef DECL_READ_IMAGE
#undef DECL_READ_IMAGE_NOSAMPLER
#undef DECL_WRITE_IMAGE

#undef GET_IMAGE
// ##BEGIN_VECTOR##
// ##END_VECTOR##

#undef INLINE_OVERLOADABLE
#undef PURE
#undef CONST
#undef OVERLOADABLE
#undef INLINE
#endif /* __GEN_OCL_STDLIB_H__ */
