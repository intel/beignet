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
//#define local __local
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
union _type_cast_1_b {
  char _char;
  uchar _uchar;
};

INLINE OVERLOADABLE uchar as_uchar(char v) {
  union _type_cast_1_b u;
  u._char = v;
  return u._uchar;
}

INLINE OVERLOADABLE char as_char(uchar v) {
  union _type_cast_1_b u;
  u._uchar = v;
  return u._char;
}

union _type_cast_2_b {
  short _short;
  ushort _ushort;
  char2 _char2;
  uchar2 _uchar2;
};

INLINE OVERLOADABLE ushort as_ushort(short v) {
  union _type_cast_2_b u;
  u._short = v;
  return u._ushort;
}

INLINE OVERLOADABLE char2 as_char2(short v) {
  union _type_cast_2_b u;
  u._short = v;
  return u._char2;
}

INLINE OVERLOADABLE uchar2 as_uchar2(short v) {
  union _type_cast_2_b u;
  u._short = v;
  return u._uchar2;
}

INLINE OVERLOADABLE short as_short(ushort v) {
  union _type_cast_2_b u;
  u._ushort = v;
  return u._short;
}

INLINE OVERLOADABLE char2 as_char2(ushort v) {
  union _type_cast_2_b u;
  u._ushort = v;
  return u._char2;
}

INLINE OVERLOADABLE uchar2 as_uchar2(ushort v) {
  union _type_cast_2_b u;
  u._ushort = v;
  return u._uchar2;
}

INLINE OVERLOADABLE short as_short(char2 v) {
  union _type_cast_2_b u;
  u._char2 = v;
  return u._short;
}

INLINE OVERLOADABLE ushort as_ushort(char2 v) {
  union _type_cast_2_b u;
  u._char2 = v;
  return u._ushort;
}

INLINE OVERLOADABLE uchar2 as_uchar2(char2 v) {
  union _type_cast_2_b u;
  u._char2 = v;
  return u._uchar2;
}

INLINE OVERLOADABLE short as_short(uchar2 v) {
  union _type_cast_2_b u;
  u._uchar2 = v;
  return u._short;
}

INLINE OVERLOADABLE ushort as_ushort(uchar2 v) {
  union _type_cast_2_b u;
  u._uchar2 = v;
  return u._ushort;
}

INLINE OVERLOADABLE char2 as_char2(uchar2 v) {
  union _type_cast_2_b u;
  u._uchar2 = v;
  return u._char2;
}

union _type_cast_3_b {
  char3 _char3;
  uchar3 _uchar3;
};

INLINE OVERLOADABLE uchar3 as_uchar3(char3 v) {
  union _type_cast_3_b u;
  u._char3 = v;
  return u._uchar3;
}

INLINE OVERLOADABLE char3 as_char3(uchar3 v) {
  union _type_cast_3_b u;
  u._uchar3 = v;
  return u._char3;
}

union _type_cast_4_b {
  int _int;
  uint _uint;
  short2 _short2;
  ushort2 _ushort2;
  char4 _char4;
  uchar4 _uchar4;
  float _float;
};

INLINE OVERLOADABLE uint as_uint(int v) {
  union _type_cast_4_b u;
  u._int = v;
  return u._uint;
}

INLINE OVERLOADABLE short2 as_short2(int v) {
  union _type_cast_4_b u;
  u._int = v;
  return u._short2;
}

INLINE OVERLOADABLE ushort2 as_ushort2(int v) {
  union _type_cast_4_b u;
  u._int = v;
  return u._ushort2;
}

INLINE OVERLOADABLE char4 as_char4(int v) {
  union _type_cast_4_b u;
  u._int = v;
  return u._char4;
}

INLINE OVERLOADABLE uchar4 as_uchar4(int v) {
  union _type_cast_4_b u;
  u._int = v;
  return u._uchar4;
}

INLINE OVERLOADABLE float as_float(int v) {
  union _type_cast_4_b u;
  u._int = v;
  return u._float;
}

INLINE OVERLOADABLE int as_int(uint v) {
  union _type_cast_4_b u;
  u._uint = v;
  return u._int;
}

INLINE OVERLOADABLE short2 as_short2(uint v) {
  union _type_cast_4_b u;
  u._uint = v;
  return u._short2;
}

INLINE OVERLOADABLE ushort2 as_ushort2(uint v) {
  union _type_cast_4_b u;
  u._uint = v;
  return u._ushort2;
}

INLINE OVERLOADABLE char4 as_char4(uint v) {
  union _type_cast_4_b u;
  u._uint = v;
  return u._char4;
}

INLINE OVERLOADABLE uchar4 as_uchar4(uint v) {
  union _type_cast_4_b u;
  u._uint = v;
  return u._uchar4;
}

INLINE OVERLOADABLE float as_float(uint v) {
  union _type_cast_4_b u;
  u._uint = v;
  return u._float;
}

INLINE OVERLOADABLE int as_int(short2 v) {
  union _type_cast_4_b u;
  u._short2 = v;
  return u._int;
}

INLINE OVERLOADABLE uint as_uint(short2 v) {
  union _type_cast_4_b u;
  u._short2 = v;
  return u._uint;
}

INLINE OVERLOADABLE ushort2 as_ushort2(short2 v) {
  union _type_cast_4_b u;
  u._short2 = v;
  return u._ushort2;
}

INLINE OVERLOADABLE char4 as_char4(short2 v) {
  union _type_cast_4_b u;
  u._short2 = v;
  return u._char4;
}

INLINE OVERLOADABLE uchar4 as_uchar4(short2 v) {
  union _type_cast_4_b u;
  u._short2 = v;
  return u._uchar4;
}

INLINE OVERLOADABLE float as_float(short2 v) {
  union _type_cast_4_b u;
  u._short2 = v;
  return u._float;
}

INLINE OVERLOADABLE int as_int(ushort2 v) {
  union _type_cast_4_b u;
  u._ushort2 = v;
  return u._int;
}

INLINE OVERLOADABLE uint as_uint(ushort2 v) {
  union _type_cast_4_b u;
  u._ushort2 = v;
  return u._uint;
}

INLINE OVERLOADABLE short2 as_short2(ushort2 v) {
  union _type_cast_4_b u;
  u._ushort2 = v;
  return u._short2;
}

INLINE OVERLOADABLE char4 as_char4(ushort2 v) {
  union _type_cast_4_b u;
  u._ushort2 = v;
  return u._char4;
}

INLINE OVERLOADABLE uchar4 as_uchar4(ushort2 v) {
  union _type_cast_4_b u;
  u._ushort2 = v;
  return u._uchar4;
}

INLINE OVERLOADABLE float as_float(ushort2 v) {
  union _type_cast_4_b u;
  u._ushort2 = v;
  return u._float;
}

INLINE OVERLOADABLE int as_int(char4 v) {
  union _type_cast_4_b u;
  u._char4 = v;
  return u._int;
}

INLINE OVERLOADABLE uint as_uint(char4 v) {
  union _type_cast_4_b u;
  u._char4 = v;
  return u._uint;
}

INLINE OVERLOADABLE short2 as_short2(char4 v) {
  union _type_cast_4_b u;
  u._char4 = v;
  return u._short2;
}

INLINE OVERLOADABLE ushort2 as_ushort2(char4 v) {
  union _type_cast_4_b u;
  u._char4 = v;
  return u._ushort2;
}

INLINE OVERLOADABLE uchar4 as_uchar4(char4 v) {
  union _type_cast_4_b u;
  u._char4 = v;
  return u._uchar4;
}

INLINE OVERLOADABLE float as_float(char4 v) {
  union _type_cast_4_b u;
  u._char4 = v;
  return u._float;
}

INLINE OVERLOADABLE int as_int(uchar4 v) {
  union _type_cast_4_b u;
  u._uchar4 = v;
  return u._int;
}

INLINE OVERLOADABLE uint as_uint(uchar4 v) {
  union _type_cast_4_b u;
  u._uchar4 = v;
  return u._uint;
}

INLINE OVERLOADABLE short2 as_short2(uchar4 v) {
  union _type_cast_4_b u;
  u._uchar4 = v;
  return u._short2;
}

INLINE OVERLOADABLE ushort2 as_ushort2(uchar4 v) {
  union _type_cast_4_b u;
  u._uchar4 = v;
  return u._ushort2;
}

INLINE OVERLOADABLE char4 as_char4(uchar4 v) {
  union _type_cast_4_b u;
  u._uchar4 = v;
  return u._char4;
}

INLINE OVERLOADABLE float as_float(uchar4 v) {
  union _type_cast_4_b u;
  u._uchar4 = v;
  return u._float;
}

INLINE OVERLOADABLE int as_int(float v) {
  union _type_cast_4_b u;
  u._float = v;
  return u._int;
}

INLINE OVERLOADABLE uint as_uint(float v) {
  union _type_cast_4_b u;
  u._float = v;
  return u._uint;
}

INLINE OVERLOADABLE short2 as_short2(float v) {
  union _type_cast_4_b u;
  u._float = v;
  return u._short2;
}

INLINE OVERLOADABLE ushort2 as_ushort2(float v) {
  union _type_cast_4_b u;
  u._float = v;
  return u._ushort2;
}

INLINE OVERLOADABLE char4 as_char4(float v) {
  union _type_cast_4_b u;
  u._float = v;
  return u._char4;
}

INLINE OVERLOADABLE uchar4 as_uchar4(float v) {
  union _type_cast_4_b u;
  u._float = v;
  return u._uchar4;
}

union _type_cast_6_b {
  short3 _short3;
  ushort3 _ushort3;
};

INLINE OVERLOADABLE ushort3 as_ushort3(short3 v) {
  union _type_cast_6_b u;
  u._short3 = v;
  return u._ushort3;
}

INLINE OVERLOADABLE short3 as_short3(ushort3 v) {
  union _type_cast_6_b u;
  u._ushort3 = v;
  return u._short3;
}

union _type_cast_8_b {
  long _long;
  ulong _ulong;
  int2 _int2;
  uint2 _uint2;
  short4 _short4;
  ushort4 _ushort4;
  char8 _char8;
  uchar8 _uchar8;
  double _double;
  float2 _float2;
};

INLINE OVERLOADABLE ulong as_ulong(long v) {
  union _type_cast_8_b u;
  u._long = v;
  return u._ulong;
}

INLINE OVERLOADABLE int2 as_int2(long v) {
  union _type_cast_8_b u;
  u._long = v;
  return u._int2;
}

INLINE OVERLOADABLE uint2 as_uint2(long v) {
  union _type_cast_8_b u;
  u._long = v;
  return u._uint2;
}

INLINE OVERLOADABLE short4 as_short4(long v) {
  union _type_cast_8_b u;
  u._long = v;
  return u._short4;
}

INLINE OVERLOADABLE ushort4 as_ushort4(long v) {
  union _type_cast_8_b u;
  u._long = v;
  return u._ushort4;
}

INLINE OVERLOADABLE char8 as_char8(long v) {
  union _type_cast_8_b u;
  u._long = v;
  return u._char8;
}

INLINE OVERLOADABLE uchar8 as_uchar8(long v) {
  union _type_cast_8_b u;
  u._long = v;
  return u._uchar8;
}

INLINE OVERLOADABLE double as_double(long v) {
  union _type_cast_8_b u;
  u._long = v;
  return u._double;
}

INLINE OVERLOADABLE float2 as_float2(long v) {
  union _type_cast_8_b u;
  u._long = v;
  return u._float2;
}

INLINE OVERLOADABLE long as_long(ulong v) {
  union _type_cast_8_b u;
  u._ulong = v;
  return u._long;
}

INLINE OVERLOADABLE int2 as_int2(ulong v) {
  union _type_cast_8_b u;
  u._ulong = v;
  return u._int2;
}

INLINE OVERLOADABLE uint2 as_uint2(ulong v) {
  union _type_cast_8_b u;
  u._ulong = v;
  return u._uint2;
}

INLINE OVERLOADABLE short4 as_short4(ulong v) {
  union _type_cast_8_b u;
  u._ulong = v;
  return u._short4;
}

INLINE OVERLOADABLE ushort4 as_ushort4(ulong v) {
  union _type_cast_8_b u;
  u._ulong = v;
  return u._ushort4;
}

INLINE OVERLOADABLE char8 as_char8(ulong v) {
  union _type_cast_8_b u;
  u._ulong = v;
  return u._char8;
}

INLINE OVERLOADABLE uchar8 as_uchar8(ulong v) {
  union _type_cast_8_b u;
  u._ulong = v;
  return u._uchar8;
}

INLINE OVERLOADABLE double as_double(ulong v) {
  union _type_cast_8_b u;
  u._ulong = v;
  return u._double;
}

INLINE OVERLOADABLE float2 as_float2(ulong v) {
  union _type_cast_8_b u;
  u._ulong = v;
  return u._float2;
}

INLINE OVERLOADABLE long as_long(int2 v) {
  union _type_cast_8_b u;
  u._int2 = v;
  return u._long;
}

INLINE OVERLOADABLE ulong as_ulong(int2 v) {
  union _type_cast_8_b u;
  u._int2 = v;
  return u._ulong;
}

INLINE OVERLOADABLE uint2 as_uint2(int2 v) {
  union _type_cast_8_b u;
  u._int2 = v;
  return u._uint2;
}

INLINE OVERLOADABLE short4 as_short4(int2 v) {
  union _type_cast_8_b u;
  u._int2 = v;
  return u._short4;
}

INLINE OVERLOADABLE ushort4 as_ushort4(int2 v) {
  union _type_cast_8_b u;
  u._int2 = v;
  return u._ushort4;
}

INLINE OVERLOADABLE char8 as_char8(int2 v) {
  union _type_cast_8_b u;
  u._int2 = v;
  return u._char8;
}

INLINE OVERLOADABLE uchar8 as_uchar8(int2 v) {
  union _type_cast_8_b u;
  u._int2 = v;
  return u._uchar8;
}

INLINE OVERLOADABLE double as_double(int2 v) {
  union _type_cast_8_b u;
  u._int2 = v;
  return u._double;
}

INLINE OVERLOADABLE float2 as_float2(int2 v) {
  union _type_cast_8_b u;
  u._int2 = v;
  return u._float2;
}

INLINE OVERLOADABLE long as_long(uint2 v) {
  union _type_cast_8_b u;
  u._uint2 = v;
  return u._long;
}

INLINE OVERLOADABLE ulong as_ulong(uint2 v) {
  union _type_cast_8_b u;
  u._uint2 = v;
  return u._ulong;
}

INLINE OVERLOADABLE int2 as_int2(uint2 v) {
  union _type_cast_8_b u;
  u._uint2 = v;
  return u._int2;
}

INLINE OVERLOADABLE short4 as_short4(uint2 v) {
  union _type_cast_8_b u;
  u._uint2 = v;
  return u._short4;
}

INLINE OVERLOADABLE ushort4 as_ushort4(uint2 v) {
  union _type_cast_8_b u;
  u._uint2 = v;
  return u._ushort4;
}

INLINE OVERLOADABLE char8 as_char8(uint2 v) {
  union _type_cast_8_b u;
  u._uint2 = v;
  return u._char8;
}

INLINE OVERLOADABLE uchar8 as_uchar8(uint2 v) {
  union _type_cast_8_b u;
  u._uint2 = v;
  return u._uchar8;
}

INLINE OVERLOADABLE double as_double(uint2 v) {
  union _type_cast_8_b u;
  u._uint2 = v;
  return u._double;
}

INLINE OVERLOADABLE float2 as_float2(uint2 v) {
  union _type_cast_8_b u;
  u._uint2 = v;
  return u._float2;
}

INLINE OVERLOADABLE long as_long(short4 v) {
  union _type_cast_8_b u;
  u._short4 = v;
  return u._long;
}

INLINE OVERLOADABLE ulong as_ulong(short4 v) {
  union _type_cast_8_b u;
  u._short4 = v;
  return u._ulong;
}

INLINE OVERLOADABLE int2 as_int2(short4 v) {
  union _type_cast_8_b u;
  u._short4 = v;
  return u._int2;
}

INLINE OVERLOADABLE uint2 as_uint2(short4 v) {
  union _type_cast_8_b u;
  u._short4 = v;
  return u._uint2;
}

INLINE OVERLOADABLE ushort4 as_ushort4(short4 v) {
  union _type_cast_8_b u;
  u._short4 = v;
  return u._ushort4;
}

INLINE OVERLOADABLE char8 as_char8(short4 v) {
  union _type_cast_8_b u;
  u._short4 = v;
  return u._char8;
}

INLINE OVERLOADABLE uchar8 as_uchar8(short4 v) {
  union _type_cast_8_b u;
  u._short4 = v;
  return u._uchar8;
}

INLINE OVERLOADABLE double as_double(short4 v) {
  union _type_cast_8_b u;
  u._short4 = v;
  return u._double;
}

INLINE OVERLOADABLE float2 as_float2(short4 v) {
  union _type_cast_8_b u;
  u._short4 = v;
  return u._float2;
}

INLINE OVERLOADABLE long as_long(ushort4 v) {
  union _type_cast_8_b u;
  u._ushort4 = v;
  return u._long;
}

INLINE OVERLOADABLE ulong as_ulong(ushort4 v) {
  union _type_cast_8_b u;
  u._ushort4 = v;
  return u._ulong;
}

INLINE OVERLOADABLE int2 as_int2(ushort4 v) {
  union _type_cast_8_b u;
  u._ushort4 = v;
  return u._int2;
}

INLINE OVERLOADABLE uint2 as_uint2(ushort4 v) {
  union _type_cast_8_b u;
  u._ushort4 = v;
  return u._uint2;
}

INLINE OVERLOADABLE short4 as_short4(ushort4 v) {
  union _type_cast_8_b u;
  u._ushort4 = v;
  return u._short4;
}

INLINE OVERLOADABLE char8 as_char8(ushort4 v) {
  union _type_cast_8_b u;
  u._ushort4 = v;
  return u._char8;
}

INLINE OVERLOADABLE uchar8 as_uchar8(ushort4 v) {
  union _type_cast_8_b u;
  u._ushort4 = v;
  return u._uchar8;
}

INLINE OVERLOADABLE double as_double(ushort4 v) {
  union _type_cast_8_b u;
  u._ushort4 = v;
  return u._double;
}

INLINE OVERLOADABLE float2 as_float2(ushort4 v) {
  union _type_cast_8_b u;
  u._ushort4 = v;
  return u._float2;
}

INLINE OVERLOADABLE long as_long(char8 v) {
  union _type_cast_8_b u;
  u._char8 = v;
  return u._long;
}

INLINE OVERLOADABLE ulong as_ulong(char8 v) {
  union _type_cast_8_b u;
  u._char8 = v;
  return u._ulong;
}

INLINE OVERLOADABLE int2 as_int2(char8 v) {
  union _type_cast_8_b u;
  u._char8 = v;
  return u._int2;
}

INLINE OVERLOADABLE uint2 as_uint2(char8 v) {
  union _type_cast_8_b u;
  u._char8 = v;
  return u._uint2;
}

INLINE OVERLOADABLE short4 as_short4(char8 v) {
  union _type_cast_8_b u;
  u._char8 = v;
  return u._short4;
}

INLINE OVERLOADABLE ushort4 as_ushort4(char8 v) {
  union _type_cast_8_b u;
  u._char8 = v;
  return u._ushort4;
}

INLINE OVERLOADABLE uchar8 as_uchar8(char8 v) {
  union _type_cast_8_b u;
  u._char8 = v;
  return u._uchar8;
}

INLINE OVERLOADABLE double as_double(char8 v) {
  union _type_cast_8_b u;
  u._char8 = v;
  return u._double;
}

INLINE OVERLOADABLE float2 as_float2(char8 v) {
  union _type_cast_8_b u;
  u._char8 = v;
  return u._float2;
}

INLINE OVERLOADABLE long as_long(uchar8 v) {
  union _type_cast_8_b u;
  u._uchar8 = v;
  return u._long;
}

INLINE OVERLOADABLE ulong as_ulong(uchar8 v) {
  union _type_cast_8_b u;
  u._uchar8 = v;
  return u._ulong;
}

INLINE OVERLOADABLE int2 as_int2(uchar8 v) {
  union _type_cast_8_b u;
  u._uchar8 = v;
  return u._int2;
}

INLINE OVERLOADABLE uint2 as_uint2(uchar8 v) {
  union _type_cast_8_b u;
  u._uchar8 = v;
  return u._uint2;
}

INLINE OVERLOADABLE short4 as_short4(uchar8 v) {
  union _type_cast_8_b u;
  u._uchar8 = v;
  return u._short4;
}

INLINE OVERLOADABLE ushort4 as_ushort4(uchar8 v) {
  union _type_cast_8_b u;
  u._uchar8 = v;
  return u._ushort4;
}

INLINE OVERLOADABLE char8 as_char8(uchar8 v) {
  union _type_cast_8_b u;
  u._uchar8 = v;
  return u._char8;
}

INLINE OVERLOADABLE double as_double(uchar8 v) {
  union _type_cast_8_b u;
  u._uchar8 = v;
  return u._double;
}

INLINE OVERLOADABLE float2 as_float2(uchar8 v) {
  union _type_cast_8_b u;
  u._uchar8 = v;
  return u._float2;
}

INLINE OVERLOADABLE long as_long(double v) {
  union _type_cast_8_b u;
  u._double = v;
  return u._long;
}

INLINE OVERLOADABLE ulong as_ulong(double v) {
  union _type_cast_8_b u;
  u._double = v;
  return u._ulong;
}

INLINE OVERLOADABLE int2 as_int2(double v) {
  union _type_cast_8_b u;
  u._double = v;
  return u._int2;
}

INLINE OVERLOADABLE uint2 as_uint2(double v) {
  union _type_cast_8_b u;
  u._double = v;
  return u._uint2;
}

INLINE OVERLOADABLE short4 as_short4(double v) {
  union _type_cast_8_b u;
  u._double = v;
  return u._short4;
}

INLINE OVERLOADABLE ushort4 as_ushort4(double v) {
  union _type_cast_8_b u;
  u._double = v;
  return u._ushort4;
}

INLINE OVERLOADABLE char8 as_char8(double v) {
  union _type_cast_8_b u;
  u._double = v;
  return u._char8;
}

INLINE OVERLOADABLE uchar8 as_uchar8(double v) {
  union _type_cast_8_b u;
  u._double = v;
  return u._uchar8;
}

INLINE OVERLOADABLE float2 as_float2(double v) {
  union _type_cast_8_b u;
  u._double = v;
  return u._float2;
}

INLINE OVERLOADABLE long as_long(float2 v) {
  union _type_cast_8_b u;
  u._float2 = v;
  return u._long;
}

INLINE OVERLOADABLE ulong as_ulong(float2 v) {
  union _type_cast_8_b u;
  u._float2 = v;
  return u._ulong;
}

INLINE OVERLOADABLE int2 as_int2(float2 v) {
  union _type_cast_8_b u;
  u._float2 = v;
  return u._int2;
}

INLINE OVERLOADABLE uint2 as_uint2(float2 v) {
  union _type_cast_8_b u;
  u._float2 = v;
  return u._uint2;
}

INLINE OVERLOADABLE short4 as_short4(float2 v) {
  union _type_cast_8_b u;
  u._float2 = v;
  return u._short4;
}

INLINE OVERLOADABLE ushort4 as_ushort4(float2 v) {
  union _type_cast_8_b u;
  u._float2 = v;
  return u._ushort4;
}

INLINE OVERLOADABLE char8 as_char8(float2 v) {
  union _type_cast_8_b u;
  u._float2 = v;
  return u._char8;
}

INLINE OVERLOADABLE uchar8 as_uchar8(float2 v) {
  union _type_cast_8_b u;
  u._float2 = v;
  return u._uchar8;
}

INLINE OVERLOADABLE double as_double(float2 v) {
  union _type_cast_8_b u;
  u._float2 = v;
  return u._double;
}

union _type_cast_12_b {
  int3 _int3;
  uint3 _uint3;
  float3 _float3;
};

INLINE OVERLOADABLE uint3 as_uint3(int3 v) {
  union _type_cast_12_b u;
  u._int3 = v;
  return u._uint3;
}

INLINE OVERLOADABLE float3 as_float3(int3 v) {
  union _type_cast_12_b u;
  u._int3 = v;
  return u._float3;
}

INLINE OVERLOADABLE int3 as_int3(uint3 v) {
  union _type_cast_12_b u;
  u._uint3 = v;
  return u._int3;
}

INLINE OVERLOADABLE float3 as_float3(uint3 v) {
  union _type_cast_12_b u;
  u._uint3 = v;
  return u._float3;
}

INLINE OVERLOADABLE int3 as_int3(float3 v) {
  union _type_cast_12_b u;
  u._float3 = v;
  return u._int3;
}

INLINE OVERLOADABLE uint3 as_uint3(float3 v) {
  union _type_cast_12_b u;
  u._float3 = v;
  return u._uint3;
}

union _type_cast_16_b {
  long2 _long2;
  ulong2 _ulong2;
  int4 _int4;
  uint4 _uint4;
  short8 _short8;
  ushort8 _ushort8;
  char16 _char16;
  uchar16 _uchar16;
  double2 _double2;
  float4 _float4;
};

INLINE OVERLOADABLE ulong2 as_ulong2(long2 v) {
  union _type_cast_16_b u;
  u._long2 = v;
  return u._ulong2;
}

INLINE OVERLOADABLE int4 as_int4(long2 v) {
  union _type_cast_16_b u;
  u._long2 = v;
  return u._int4;
}

INLINE OVERLOADABLE uint4 as_uint4(long2 v) {
  union _type_cast_16_b u;
  u._long2 = v;
  return u._uint4;
}

INLINE OVERLOADABLE short8 as_short8(long2 v) {
  union _type_cast_16_b u;
  u._long2 = v;
  return u._short8;
}

INLINE OVERLOADABLE ushort8 as_ushort8(long2 v) {
  union _type_cast_16_b u;
  u._long2 = v;
  return u._ushort8;
}

INLINE OVERLOADABLE char16 as_char16(long2 v) {
  union _type_cast_16_b u;
  u._long2 = v;
  return u._char16;
}

INLINE OVERLOADABLE uchar16 as_uchar16(long2 v) {
  union _type_cast_16_b u;
  u._long2 = v;
  return u._uchar16;
}

INLINE OVERLOADABLE double2 as_double2(long2 v) {
  union _type_cast_16_b u;
  u._long2 = v;
  return u._double2;
}

INLINE OVERLOADABLE float4 as_float4(long2 v) {
  union _type_cast_16_b u;
  u._long2 = v;
  return u._float4;
}

INLINE OVERLOADABLE long2 as_long2(ulong2 v) {
  union _type_cast_16_b u;
  u._ulong2 = v;
  return u._long2;
}

INLINE OVERLOADABLE int4 as_int4(ulong2 v) {
  union _type_cast_16_b u;
  u._ulong2 = v;
  return u._int4;
}

INLINE OVERLOADABLE uint4 as_uint4(ulong2 v) {
  union _type_cast_16_b u;
  u._ulong2 = v;
  return u._uint4;
}

INLINE OVERLOADABLE short8 as_short8(ulong2 v) {
  union _type_cast_16_b u;
  u._ulong2 = v;
  return u._short8;
}

INLINE OVERLOADABLE ushort8 as_ushort8(ulong2 v) {
  union _type_cast_16_b u;
  u._ulong2 = v;
  return u._ushort8;
}

INLINE OVERLOADABLE char16 as_char16(ulong2 v) {
  union _type_cast_16_b u;
  u._ulong2 = v;
  return u._char16;
}

INLINE OVERLOADABLE uchar16 as_uchar16(ulong2 v) {
  union _type_cast_16_b u;
  u._ulong2 = v;
  return u._uchar16;
}

INLINE OVERLOADABLE double2 as_double2(ulong2 v) {
  union _type_cast_16_b u;
  u._ulong2 = v;
  return u._double2;
}

INLINE OVERLOADABLE float4 as_float4(ulong2 v) {
  union _type_cast_16_b u;
  u._ulong2 = v;
  return u._float4;
}

INLINE OVERLOADABLE long2 as_long2(int4 v) {
  union _type_cast_16_b u;
  u._int4 = v;
  return u._long2;
}

INLINE OVERLOADABLE ulong2 as_ulong2(int4 v) {
  union _type_cast_16_b u;
  u._int4 = v;
  return u._ulong2;
}

INLINE OVERLOADABLE uint4 as_uint4(int4 v) {
  union _type_cast_16_b u;
  u._int4 = v;
  return u._uint4;
}

INLINE OVERLOADABLE short8 as_short8(int4 v) {
  union _type_cast_16_b u;
  u._int4 = v;
  return u._short8;
}

INLINE OVERLOADABLE ushort8 as_ushort8(int4 v) {
  union _type_cast_16_b u;
  u._int4 = v;
  return u._ushort8;
}

INLINE OVERLOADABLE char16 as_char16(int4 v) {
  union _type_cast_16_b u;
  u._int4 = v;
  return u._char16;
}

INLINE OVERLOADABLE uchar16 as_uchar16(int4 v) {
  union _type_cast_16_b u;
  u._int4 = v;
  return u._uchar16;
}

INLINE OVERLOADABLE double2 as_double2(int4 v) {
  union _type_cast_16_b u;
  u._int4 = v;
  return u._double2;
}

INLINE OVERLOADABLE float4 as_float4(int4 v) {
  union _type_cast_16_b u;
  u._int4 = v;
  return u._float4;
}

INLINE OVERLOADABLE long2 as_long2(uint4 v) {
  union _type_cast_16_b u;
  u._uint4 = v;
  return u._long2;
}

INLINE OVERLOADABLE ulong2 as_ulong2(uint4 v) {
  union _type_cast_16_b u;
  u._uint4 = v;
  return u._ulong2;
}

INLINE OVERLOADABLE int4 as_int4(uint4 v) {
  union _type_cast_16_b u;
  u._uint4 = v;
  return u._int4;
}

INLINE OVERLOADABLE short8 as_short8(uint4 v) {
  union _type_cast_16_b u;
  u._uint4 = v;
  return u._short8;
}

INLINE OVERLOADABLE ushort8 as_ushort8(uint4 v) {
  union _type_cast_16_b u;
  u._uint4 = v;
  return u._ushort8;
}

INLINE OVERLOADABLE char16 as_char16(uint4 v) {
  union _type_cast_16_b u;
  u._uint4 = v;
  return u._char16;
}

INLINE OVERLOADABLE uchar16 as_uchar16(uint4 v) {
  union _type_cast_16_b u;
  u._uint4 = v;
  return u._uchar16;
}

INLINE OVERLOADABLE double2 as_double2(uint4 v) {
  union _type_cast_16_b u;
  u._uint4 = v;
  return u._double2;
}

INLINE OVERLOADABLE float4 as_float4(uint4 v) {
  union _type_cast_16_b u;
  u._uint4 = v;
  return u._float4;
}

INLINE OVERLOADABLE long2 as_long2(short8 v) {
  union _type_cast_16_b u;
  u._short8 = v;
  return u._long2;
}

INLINE OVERLOADABLE ulong2 as_ulong2(short8 v) {
  union _type_cast_16_b u;
  u._short8 = v;
  return u._ulong2;
}

INLINE OVERLOADABLE int4 as_int4(short8 v) {
  union _type_cast_16_b u;
  u._short8 = v;
  return u._int4;
}

INLINE OVERLOADABLE uint4 as_uint4(short8 v) {
  union _type_cast_16_b u;
  u._short8 = v;
  return u._uint4;
}

INLINE OVERLOADABLE ushort8 as_ushort8(short8 v) {
  union _type_cast_16_b u;
  u._short8 = v;
  return u._ushort8;
}

INLINE OVERLOADABLE char16 as_char16(short8 v) {
  union _type_cast_16_b u;
  u._short8 = v;
  return u._char16;
}

INLINE OVERLOADABLE uchar16 as_uchar16(short8 v) {
  union _type_cast_16_b u;
  u._short8 = v;
  return u._uchar16;
}

INLINE OVERLOADABLE double2 as_double2(short8 v) {
  union _type_cast_16_b u;
  u._short8 = v;
  return u._double2;
}

INLINE OVERLOADABLE float4 as_float4(short8 v) {
  union _type_cast_16_b u;
  u._short8 = v;
  return u._float4;
}

INLINE OVERLOADABLE long2 as_long2(ushort8 v) {
  union _type_cast_16_b u;
  u._ushort8 = v;
  return u._long2;
}

INLINE OVERLOADABLE ulong2 as_ulong2(ushort8 v) {
  union _type_cast_16_b u;
  u._ushort8 = v;
  return u._ulong2;
}

INLINE OVERLOADABLE int4 as_int4(ushort8 v) {
  union _type_cast_16_b u;
  u._ushort8 = v;
  return u._int4;
}

INLINE OVERLOADABLE uint4 as_uint4(ushort8 v) {
  union _type_cast_16_b u;
  u._ushort8 = v;
  return u._uint4;
}

INLINE OVERLOADABLE short8 as_short8(ushort8 v) {
  union _type_cast_16_b u;
  u._ushort8 = v;
  return u._short8;
}

INLINE OVERLOADABLE char16 as_char16(ushort8 v) {
  union _type_cast_16_b u;
  u._ushort8 = v;
  return u._char16;
}

INLINE OVERLOADABLE uchar16 as_uchar16(ushort8 v) {
  union _type_cast_16_b u;
  u._ushort8 = v;
  return u._uchar16;
}

INLINE OVERLOADABLE double2 as_double2(ushort8 v) {
  union _type_cast_16_b u;
  u._ushort8 = v;
  return u._double2;
}

INLINE OVERLOADABLE float4 as_float4(ushort8 v) {
  union _type_cast_16_b u;
  u._ushort8 = v;
  return u._float4;
}

INLINE OVERLOADABLE long2 as_long2(char16 v) {
  union _type_cast_16_b u;
  u._char16 = v;
  return u._long2;
}

INLINE OVERLOADABLE ulong2 as_ulong2(char16 v) {
  union _type_cast_16_b u;
  u._char16 = v;
  return u._ulong2;
}

INLINE OVERLOADABLE int4 as_int4(char16 v) {
  union _type_cast_16_b u;
  u._char16 = v;
  return u._int4;
}

INLINE OVERLOADABLE uint4 as_uint4(char16 v) {
  union _type_cast_16_b u;
  u._char16 = v;
  return u._uint4;
}

INLINE OVERLOADABLE short8 as_short8(char16 v) {
  union _type_cast_16_b u;
  u._char16 = v;
  return u._short8;
}

INLINE OVERLOADABLE ushort8 as_ushort8(char16 v) {
  union _type_cast_16_b u;
  u._char16 = v;
  return u._ushort8;
}

INLINE OVERLOADABLE uchar16 as_uchar16(char16 v) {
  union _type_cast_16_b u;
  u._char16 = v;
  return u._uchar16;
}

INLINE OVERLOADABLE double2 as_double2(char16 v) {
  union _type_cast_16_b u;
  u._char16 = v;
  return u._double2;
}

INLINE OVERLOADABLE float4 as_float4(char16 v) {
  union _type_cast_16_b u;
  u._char16 = v;
  return u._float4;
}

INLINE OVERLOADABLE long2 as_long2(uchar16 v) {
  union _type_cast_16_b u;
  u._uchar16 = v;
  return u._long2;
}

INLINE OVERLOADABLE ulong2 as_ulong2(uchar16 v) {
  union _type_cast_16_b u;
  u._uchar16 = v;
  return u._ulong2;
}

INLINE OVERLOADABLE int4 as_int4(uchar16 v) {
  union _type_cast_16_b u;
  u._uchar16 = v;
  return u._int4;
}

INLINE OVERLOADABLE uint4 as_uint4(uchar16 v) {
  union _type_cast_16_b u;
  u._uchar16 = v;
  return u._uint4;
}

INLINE OVERLOADABLE short8 as_short8(uchar16 v) {
  union _type_cast_16_b u;
  u._uchar16 = v;
  return u._short8;
}

INLINE OVERLOADABLE ushort8 as_ushort8(uchar16 v) {
  union _type_cast_16_b u;
  u._uchar16 = v;
  return u._ushort8;
}

INLINE OVERLOADABLE char16 as_char16(uchar16 v) {
  union _type_cast_16_b u;
  u._uchar16 = v;
  return u._char16;
}

INLINE OVERLOADABLE double2 as_double2(uchar16 v) {
  union _type_cast_16_b u;
  u._uchar16 = v;
  return u._double2;
}

INLINE OVERLOADABLE float4 as_float4(uchar16 v) {
  union _type_cast_16_b u;
  u._uchar16 = v;
  return u._float4;
}

INLINE OVERLOADABLE long2 as_long2(double2 v) {
  union _type_cast_16_b u;
  u._double2 = v;
  return u._long2;
}

INLINE OVERLOADABLE ulong2 as_ulong2(double2 v) {
  union _type_cast_16_b u;
  u._double2 = v;
  return u._ulong2;
}

INLINE OVERLOADABLE int4 as_int4(double2 v) {
  union _type_cast_16_b u;
  u._double2 = v;
  return u._int4;
}

INLINE OVERLOADABLE uint4 as_uint4(double2 v) {
  union _type_cast_16_b u;
  u._double2 = v;
  return u._uint4;
}

INLINE OVERLOADABLE short8 as_short8(double2 v) {
  union _type_cast_16_b u;
  u._double2 = v;
  return u._short8;
}

INLINE OVERLOADABLE ushort8 as_ushort8(double2 v) {
  union _type_cast_16_b u;
  u._double2 = v;
  return u._ushort8;
}

INLINE OVERLOADABLE char16 as_char16(double2 v) {
  union _type_cast_16_b u;
  u._double2 = v;
  return u._char16;
}

INLINE OVERLOADABLE uchar16 as_uchar16(double2 v) {
  union _type_cast_16_b u;
  u._double2 = v;
  return u._uchar16;
}

INLINE OVERLOADABLE float4 as_float4(double2 v) {
  union _type_cast_16_b u;
  u._double2 = v;
  return u._float4;
}

INLINE OVERLOADABLE long2 as_long2(float4 v) {
  union _type_cast_16_b u;
  u._float4 = v;
  return u._long2;
}

INLINE OVERLOADABLE ulong2 as_ulong2(float4 v) {
  union _type_cast_16_b u;
  u._float4 = v;
  return u._ulong2;
}

INLINE OVERLOADABLE int4 as_int4(float4 v) {
  union _type_cast_16_b u;
  u._float4 = v;
  return u._int4;
}

INLINE OVERLOADABLE uint4 as_uint4(float4 v) {
  union _type_cast_16_b u;
  u._float4 = v;
  return u._uint4;
}

INLINE OVERLOADABLE short8 as_short8(float4 v) {
  union _type_cast_16_b u;
  u._float4 = v;
  return u._short8;
}

INLINE OVERLOADABLE ushort8 as_ushort8(float4 v) {
  union _type_cast_16_b u;
  u._float4 = v;
  return u._ushort8;
}

INLINE OVERLOADABLE char16 as_char16(float4 v) {
  union _type_cast_16_b u;
  u._float4 = v;
  return u._char16;
}

INLINE OVERLOADABLE uchar16 as_uchar16(float4 v) {
  union _type_cast_16_b u;
  u._float4 = v;
  return u._uchar16;
}

INLINE OVERLOADABLE double2 as_double2(float4 v) {
  union _type_cast_16_b u;
  u._float4 = v;
  return u._double2;
}

union _type_cast_24_b {
  long3 _long3;
  ulong3 _ulong3;
  double3 _double3;
};

INLINE OVERLOADABLE ulong3 as_ulong3(long3 v) {
  union _type_cast_24_b u;
  u._long3 = v;
  return u._ulong3;
}

INLINE OVERLOADABLE double3 as_double3(long3 v) {
  union _type_cast_24_b u;
  u._long3 = v;
  return u._double3;
}

INLINE OVERLOADABLE long3 as_long3(ulong3 v) {
  union _type_cast_24_b u;
  u._ulong3 = v;
  return u._long3;
}

INLINE OVERLOADABLE double3 as_double3(ulong3 v) {
  union _type_cast_24_b u;
  u._ulong3 = v;
  return u._double3;
}

INLINE OVERLOADABLE long3 as_long3(double3 v) {
  union _type_cast_24_b u;
  u._double3 = v;
  return u._long3;
}

INLINE OVERLOADABLE ulong3 as_ulong3(double3 v) {
  union _type_cast_24_b u;
  u._double3 = v;
  return u._ulong3;
}

union _type_cast_32_b {
  long4 _long4;
  ulong4 _ulong4;
  int8 _int8;
  uint8 _uint8;
  short16 _short16;
  ushort16 _ushort16;
  double4 _double4;
  float8 _float8;
};

INLINE OVERLOADABLE ulong4 as_ulong4(long4 v) {
  union _type_cast_32_b u;
  u._long4 = v;
  return u._ulong4;
}

INLINE OVERLOADABLE int8 as_int8(long4 v) {
  union _type_cast_32_b u;
  u._long4 = v;
  return u._int8;
}

INLINE OVERLOADABLE uint8 as_uint8(long4 v) {
  union _type_cast_32_b u;
  u._long4 = v;
  return u._uint8;
}

INLINE OVERLOADABLE short16 as_short16(long4 v) {
  union _type_cast_32_b u;
  u._long4 = v;
  return u._short16;
}

INLINE OVERLOADABLE ushort16 as_ushort16(long4 v) {
  union _type_cast_32_b u;
  u._long4 = v;
  return u._ushort16;
}

INLINE OVERLOADABLE double4 as_double4(long4 v) {
  union _type_cast_32_b u;
  u._long4 = v;
  return u._double4;
}

INLINE OVERLOADABLE float8 as_float8(long4 v) {
  union _type_cast_32_b u;
  u._long4 = v;
  return u._float8;
}

INLINE OVERLOADABLE long4 as_long4(ulong4 v) {
  union _type_cast_32_b u;
  u._ulong4 = v;
  return u._long4;
}

INLINE OVERLOADABLE int8 as_int8(ulong4 v) {
  union _type_cast_32_b u;
  u._ulong4 = v;
  return u._int8;
}

INLINE OVERLOADABLE uint8 as_uint8(ulong4 v) {
  union _type_cast_32_b u;
  u._ulong4 = v;
  return u._uint8;
}

INLINE OVERLOADABLE short16 as_short16(ulong4 v) {
  union _type_cast_32_b u;
  u._ulong4 = v;
  return u._short16;
}

INLINE OVERLOADABLE ushort16 as_ushort16(ulong4 v) {
  union _type_cast_32_b u;
  u._ulong4 = v;
  return u._ushort16;
}

INLINE OVERLOADABLE double4 as_double4(ulong4 v) {
  union _type_cast_32_b u;
  u._ulong4 = v;
  return u._double4;
}

INLINE OVERLOADABLE float8 as_float8(ulong4 v) {
  union _type_cast_32_b u;
  u._ulong4 = v;
  return u._float8;
}

INLINE OVERLOADABLE long4 as_long4(int8 v) {
  union _type_cast_32_b u;
  u._int8 = v;
  return u._long4;
}

INLINE OVERLOADABLE ulong4 as_ulong4(int8 v) {
  union _type_cast_32_b u;
  u._int8 = v;
  return u._ulong4;
}

INLINE OVERLOADABLE uint8 as_uint8(int8 v) {
  union _type_cast_32_b u;
  u._int8 = v;
  return u._uint8;
}

INLINE OVERLOADABLE short16 as_short16(int8 v) {
  union _type_cast_32_b u;
  u._int8 = v;
  return u._short16;
}

INLINE OVERLOADABLE ushort16 as_ushort16(int8 v) {
  union _type_cast_32_b u;
  u._int8 = v;
  return u._ushort16;
}

INLINE OVERLOADABLE double4 as_double4(int8 v) {
  union _type_cast_32_b u;
  u._int8 = v;
  return u._double4;
}

INLINE OVERLOADABLE float8 as_float8(int8 v) {
  union _type_cast_32_b u;
  u._int8 = v;
  return u._float8;
}

INLINE OVERLOADABLE long4 as_long4(uint8 v) {
  union _type_cast_32_b u;
  u._uint8 = v;
  return u._long4;
}

INLINE OVERLOADABLE ulong4 as_ulong4(uint8 v) {
  union _type_cast_32_b u;
  u._uint8 = v;
  return u._ulong4;
}

INLINE OVERLOADABLE int8 as_int8(uint8 v) {
  union _type_cast_32_b u;
  u._uint8 = v;
  return u._int8;
}

INLINE OVERLOADABLE short16 as_short16(uint8 v) {
  union _type_cast_32_b u;
  u._uint8 = v;
  return u._short16;
}

INLINE OVERLOADABLE ushort16 as_ushort16(uint8 v) {
  union _type_cast_32_b u;
  u._uint8 = v;
  return u._ushort16;
}

INLINE OVERLOADABLE double4 as_double4(uint8 v) {
  union _type_cast_32_b u;
  u._uint8 = v;
  return u._double4;
}

INLINE OVERLOADABLE float8 as_float8(uint8 v) {
  union _type_cast_32_b u;
  u._uint8 = v;
  return u._float8;
}

INLINE OVERLOADABLE long4 as_long4(short16 v) {
  union _type_cast_32_b u;
  u._short16 = v;
  return u._long4;
}

INLINE OVERLOADABLE ulong4 as_ulong4(short16 v) {
  union _type_cast_32_b u;
  u._short16 = v;
  return u._ulong4;
}

INLINE OVERLOADABLE int8 as_int8(short16 v) {
  union _type_cast_32_b u;
  u._short16 = v;
  return u._int8;
}

INLINE OVERLOADABLE uint8 as_uint8(short16 v) {
  union _type_cast_32_b u;
  u._short16 = v;
  return u._uint8;
}

INLINE OVERLOADABLE ushort16 as_ushort16(short16 v) {
  union _type_cast_32_b u;
  u._short16 = v;
  return u._ushort16;
}

INLINE OVERLOADABLE double4 as_double4(short16 v) {
  union _type_cast_32_b u;
  u._short16 = v;
  return u._double4;
}

INLINE OVERLOADABLE float8 as_float8(short16 v) {
  union _type_cast_32_b u;
  u._short16 = v;
  return u._float8;
}

INLINE OVERLOADABLE long4 as_long4(ushort16 v) {
  union _type_cast_32_b u;
  u._ushort16 = v;
  return u._long4;
}

INLINE OVERLOADABLE ulong4 as_ulong4(ushort16 v) {
  union _type_cast_32_b u;
  u._ushort16 = v;
  return u._ulong4;
}

INLINE OVERLOADABLE int8 as_int8(ushort16 v) {
  union _type_cast_32_b u;
  u._ushort16 = v;
  return u._int8;
}

INLINE OVERLOADABLE uint8 as_uint8(ushort16 v) {
  union _type_cast_32_b u;
  u._ushort16 = v;
  return u._uint8;
}

INLINE OVERLOADABLE short16 as_short16(ushort16 v) {
  union _type_cast_32_b u;
  u._ushort16 = v;
  return u._short16;
}

INLINE OVERLOADABLE double4 as_double4(ushort16 v) {
  union _type_cast_32_b u;
  u._ushort16 = v;
  return u._double4;
}

INLINE OVERLOADABLE float8 as_float8(ushort16 v) {
  union _type_cast_32_b u;
  u._ushort16 = v;
  return u._float8;
}

INLINE OVERLOADABLE long4 as_long4(double4 v) {
  union _type_cast_32_b u;
  u._double4 = v;
  return u._long4;
}

INLINE OVERLOADABLE ulong4 as_ulong4(double4 v) {
  union _type_cast_32_b u;
  u._double4 = v;
  return u._ulong4;
}

INLINE OVERLOADABLE int8 as_int8(double4 v) {
  union _type_cast_32_b u;
  u._double4 = v;
  return u._int8;
}

INLINE OVERLOADABLE uint8 as_uint8(double4 v) {
  union _type_cast_32_b u;
  u._double4 = v;
  return u._uint8;
}

INLINE OVERLOADABLE short16 as_short16(double4 v) {
  union _type_cast_32_b u;
  u._double4 = v;
  return u._short16;
}

INLINE OVERLOADABLE ushort16 as_ushort16(double4 v) {
  union _type_cast_32_b u;
  u._double4 = v;
  return u._ushort16;
}

INLINE OVERLOADABLE float8 as_float8(double4 v) {
  union _type_cast_32_b u;
  u._double4 = v;
  return u._float8;
}

INLINE OVERLOADABLE long4 as_long4(float8 v) {
  union _type_cast_32_b u;
  u._float8 = v;
  return u._long4;
}

INLINE OVERLOADABLE ulong4 as_ulong4(float8 v) {
  union _type_cast_32_b u;
  u._float8 = v;
  return u._ulong4;
}

INLINE OVERLOADABLE int8 as_int8(float8 v) {
  union _type_cast_32_b u;
  u._float8 = v;
  return u._int8;
}

INLINE OVERLOADABLE uint8 as_uint8(float8 v) {
  union _type_cast_32_b u;
  u._float8 = v;
  return u._uint8;
}

INLINE OVERLOADABLE short16 as_short16(float8 v) {
  union _type_cast_32_b u;
  u._float8 = v;
  return u._short16;
}

INLINE OVERLOADABLE ushort16 as_ushort16(float8 v) {
  union _type_cast_32_b u;
  u._float8 = v;
  return u._ushort16;
}

INLINE OVERLOADABLE double4 as_double4(float8 v) {
  union _type_cast_32_b u;
  u._float8 = v;
  return u._double4;
}

union _type_cast_64_b {
  long8 _long8;
  ulong8 _ulong8;
  int16 _int16;
  uint16 _uint16;
  double8 _double8;
  float16 _float16;
};

INLINE OVERLOADABLE ulong8 as_ulong8(long8 v) {
  union _type_cast_64_b u;
  u._long8 = v;
  return u._ulong8;
}

INLINE OVERLOADABLE int16 as_int16(long8 v) {
  union _type_cast_64_b u;
  u._long8 = v;
  return u._int16;
}

INLINE OVERLOADABLE uint16 as_uint16(long8 v) {
  union _type_cast_64_b u;
  u._long8 = v;
  return u._uint16;
}

INLINE OVERLOADABLE double8 as_double8(long8 v) {
  union _type_cast_64_b u;
  u._long8 = v;
  return u._double8;
}

INLINE OVERLOADABLE float16 as_float16(long8 v) {
  union _type_cast_64_b u;
  u._long8 = v;
  return u._float16;
}

INLINE OVERLOADABLE long8 as_long8(ulong8 v) {
  union _type_cast_64_b u;
  u._ulong8 = v;
  return u._long8;
}

INLINE OVERLOADABLE int16 as_int16(ulong8 v) {
  union _type_cast_64_b u;
  u._ulong8 = v;
  return u._int16;
}

INLINE OVERLOADABLE uint16 as_uint16(ulong8 v) {
  union _type_cast_64_b u;
  u._ulong8 = v;
  return u._uint16;
}

INLINE OVERLOADABLE double8 as_double8(ulong8 v) {
  union _type_cast_64_b u;
  u._ulong8 = v;
  return u._double8;
}

INLINE OVERLOADABLE float16 as_float16(ulong8 v) {
  union _type_cast_64_b u;
  u._ulong8 = v;
  return u._float16;
}

INLINE OVERLOADABLE long8 as_long8(int16 v) {
  union _type_cast_64_b u;
  u._int16 = v;
  return u._long8;
}

INLINE OVERLOADABLE ulong8 as_ulong8(int16 v) {
  union _type_cast_64_b u;
  u._int16 = v;
  return u._ulong8;
}

INLINE OVERLOADABLE uint16 as_uint16(int16 v) {
  union _type_cast_64_b u;
  u._int16 = v;
  return u._uint16;
}

INLINE OVERLOADABLE double8 as_double8(int16 v) {
  union _type_cast_64_b u;
  u._int16 = v;
  return u._double8;
}

INLINE OVERLOADABLE float16 as_float16(int16 v) {
  union _type_cast_64_b u;
  u._int16 = v;
  return u._float16;
}

INLINE OVERLOADABLE long8 as_long8(uint16 v) {
  union _type_cast_64_b u;
  u._uint16 = v;
  return u._long8;
}

INLINE OVERLOADABLE ulong8 as_ulong8(uint16 v) {
  union _type_cast_64_b u;
  u._uint16 = v;
  return u._ulong8;
}

INLINE OVERLOADABLE int16 as_int16(uint16 v) {
  union _type_cast_64_b u;
  u._uint16 = v;
  return u._int16;
}

INLINE OVERLOADABLE double8 as_double8(uint16 v) {
  union _type_cast_64_b u;
  u._uint16 = v;
  return u._double8;
}

INLINE OVERLOADABLE float16 as_float16(uint16 v) {
  union _type_cast_64_b u;
  u._uint16 = v;
  return u._float16;
}

INLINE OVERLOADABLE long8 as_long8(double8 v) {
  union _type_cast_64_b u;
  u._double8 = v;
  return u._long8;
}

INLINE OVERLOADABLE ulong8 as_ulong8(double8 v) {
  union _type_cast_64_b u;
  u._double8 = v;
  return u._ulong8;
}

INLINE OVERLOADABLE int16 as_int16(double8 v) {
  union _type_cast_64_b u;
  u._double8 = v;
  return u._int16;
}

INLINE OVERLOADABLE uint16 as_uint16(double8 v) {
  union _type_cast_64_b u;
  u._double8 = v;
  return u._uint16;
}

INLINE OVERLOADABLE float16 as_float16(double8 v) {
  union _type_cast_64_b u;
  u._double8 = v;
  return u._float16;
}

INLINE OVERLOADABLE long8 as_long8(float16 v) {
  union _type_cast_64_b u;
  u._float16 = v;
  return u._long8;
}

INLINE OVERLOADABLE ulong8 as_ulong8(float16 v) {
  union _type_cast_64_b u;
  u._float16 = v;
  return u._ulong8;
}

INLINE OVERLOADABLE int16 as_int16(float16 v) {
  union _type_cast_64_b u;
  u._float16 = v;
  return u._int16;
}

INLINE OVERLOADABLE uint16 as_uint16(float16 v) {
  union _type_cast_64_b u;
  u._float16 = v;
  return u._uint16;
}

INLINE OVERLOADABLE double8 as_double8(float16 v) {
  union _type_cast_64_b u;
  u._float16 = v;
  return u._double8;
}

union _type_cast_128_b {
  long16 _long16;
  ulong16 _ulong16;
  double16 _double16;
};

INLINE OVERLOADABLE ulong16 as_ulong16(long16 v) {
  union _type_cast_128_b u;
  u._long16 = v;
  return u._ulong16;
}

INLINE OVERLOADABLE double16 as_double16(long16 v) {
  union _type_cast_128_b u;
  u._long16 = v;
  return u._double16;
}

INLINE OVERLOADABLE long16 as_long16(ulong16 v) {
  union _type_cast_128_b u;
  u._ulong16 = v;
  return u._long16;
}

INLINE OVERLOADABLE double16 as_double16(ulong16 v) {
  union _type_cast_128_b u;
  u._ulong16 = v;
  return u._double16;
}

INLINE OVERLOADABLE long16 as_long16(double16 v) {
  union _type_cast_128_b u;
  u._double16 = v;
  return u._long16;
}

INLINE OVERLOADABLE ulong16 as_ulong16(double16 v) {
  union _type_cast_128_b u;
  u._double16 = v;
  return u._ulong16;
}

// ##END_AS##

// ##BEGIN_CONVERT##
INLINE OVERLOADABLE ulong2 convert_ulong2(long2 v) {
  return (ulong2)((ulong)(v.s0), (ulong)(v.s1));
}

INLINE OVERLOADABLE int2 convert_int2(long2 v) {
  return (int2)((int)(v.s0), (int)(v.s1));
}

INLINE OVERLOADABLE uint2 convert_uint2(long2 v) {
  return (uint2)((uint)(v.s0), (uint)(v.s1));
}

INLINE OVERLOADABLE short2 convert_short2(long2 v) {
  return (short2)((short)(v.s0), (short)(v.s1));
}

INLINE OVERLOADABLE ushort2 convert_ushort2(long2 v) {
  return (ushort2)((ushort)(v.s0), (ushort)(v.s1));
}

INLINE OVERLOADABLE char2 convert_char2(long2 v) {
  return (char2)((char)(v.s0), (char)(v.s1));
}

INLINE OVERLOADABLE uchar2 convert_uchar2(long2 v) {
  return (uchar2)((uchar)(v.s0), (uchar)(v.s1));
}

INLINE OVERLOADABLE double2 convert_double2(long2 v) {
  return (double2)((double)(v.s0), (double)(v.s1));
}

INLINE OVERLOADABLE float2 convert_float2(long2 v) {
  return (float2)((float)(v.s0), (float)(v.s1));
}

INLINE OVERLOADABLE long2 convert_long2(ulong2 v) {
  return (long2)((long)(v.s0), (long)(v.s1));
}

INLINE OVERLOADABLE int2 convert_int2(ulong2 v) {
  return (int2)((int)(v.s0), (int)(v.s1));
}

INLINE OVERLOADABLE uint2 convert_uint2(ulong2 v) {
  return (uint2)((uint)(v.s0), (uint)(v.s1));
}

INLINE OVERLOADABLE short2 convert_short2(ulong2 v) {
  return (short2)((short)(v.s0), (short)(v.s1));
}

INLINE OVERLOADABLE ushort2 convert_ushort2(ulong2 v) {
  return (ushort2)((ushort)(v.s0), (ushort)(v.s1));
}

INLINE OVERLOADABLE char2 convert_char2(ulong2 v) {
  return (char2)((char)(v.s0), (char)(v.s1));
}

INLINE OVERLOADABLE uchar2 convert_uchar2(ulong2 v) {
  return (uchar2)((uchar)(v.s0), (uchar)(v.s1));
}

INLINE OVERLOADABLE double2 convert_double2(ulong2 v) {
  return (double2)((double)(v.s0), (double)(v.s1));
}

INLINE OVERLOADABLE float2 convert_float2(ulong2 v) {
  return (float2)((float)(v.s0), (float)(v.s1));
}

INLINE OVERLOADABLE long2 convert_long2(int2 v) {
  return (long2)((long)(v.s0), (long)(v.s1));
}

INLINE OVERLOADABLE ulong2 convert_ulong2(int2 v) {
  return (ulong2)((ulong)(v.s0), (ulong)(v.s1));
}

INLINE OVERLOADABLE uint2 convert_uint2(int2 v) {
  return (uint2)((uint)(v.s0), (uint)(v.s1));
}

INLINE OVERLOADABLE short2 convert_short2(int2 v) {
  return (short2)((short)(v.s0), (short)(v.s1));
}

INLINE OVERLOADABLE ushort2 convert_ushort2(int2 v) {
  return (ushort2)((ushort)(v.s0), (ushort)(v.s1));
}

INLINE OVERLOADABLE char2 convert_char2(int2 v) {
  return (char2)((char)(v.s0), (char)(v.s1));
}

INLINE OVERLOADABLE uchar2 convert_uchar2(int2 v) {
  return (uchar2)((uchar)(v.s0), (uchar)(v.s1));
}

INLINE OVERLOADABLE double2 convert_double2(int2 v) {
  return (double2)((double)(v.s0), (double)(v.s1));
}

INLINE OVERLOADABLE float2 convert_float2(int2 v) {
  return (float2)((float)(v.s0), (float)(v.s1));
}

INLINE OVERLOADABLE long2 convert_long2(uint2 v) {
  return (long2)((long)(v.s0), (long)(v.s1));
}

INLINE OVERLOADABLE ulong2 convert_ulong2(uint2 v) {
  return (ulong2)((ulong)(v.s0), (ulong)(v.s1));
}

INLINE OVERLOADABLE int2 convert_int2(uint2 v) {
  return (int2)((int)(v.s0), (int)(v.s1));
}

INLINE OVERLOADABLE short2 convert_short2(uint2 v) {
  return (short2)((short)(v.s0), (short)(v.s1));
}

INLINE OVERLOADABLE ushort2 convert_ushort2(uint2 v) {
  return (ushort2)((ushort)(v.s0), (ushort)(v.s1));
}

INLINE OVERLOADABLE char2 convert_char2(uint2 v) {
  return (char2)((char)(v.s0), (char)(v.s1));
}

INLINE OVERLOADABLE uchar2 convert_uchar2(uint2 v) {
  return (uchar2)((uchar)(v.s0), (uchar)(v.s1));
}

INLINE OVERLOADABLE double2 convert_double2(uint2 v) {
  return (double2)((double)(v.s0), (double)(v.s1));
}

INLINE OVERLOADABLE float2 convert_float2(uint2 v) {
  return (float2)((float)(v.s0), (float)(v.s1));
}

INLINE OVERLOADABLE long2 convert_long2(short2 v) {
  return (long2)((long)(v.s0), (long)(v.s1));
}

INLINE OVERLOADABLE ulong2 convert_ulong2(short2 v) {
  return (ulong2)((ulong)(v.s0), (ulong)(v.s1));
}

INLINE OVERLOADABLE int2 convert_int2(short2 v) {
  return (int2)((int)(v.s0), (int)(v.s1));
}

INLINE OVERLOADABLE uint2 convert_uint2(short2 v) {
  return (uint2)((uint)(v.s0), (uint)(v.s1));
}

INLINE OVERLOADABLE ushort2 convert_ushort2(short2 v) {
  return (ushort2)((ushort)(v.s0), (ushort)(v.s1));
}

INLINE OVERLOADABLE char2 convert_char2(short2 v) {
  return (char2)((char)(v.s0), (char)(v.s1));
}

INLINE OVERLOADABLE uchar2 convert_uchar2(short2 v) {
  return (uchar2)((uchar)(v.s0), (uchar)(v.s1));
}

INLINE OVERLOADABLE double2 convert_double2(short2 v) {
  return (double2)((double)(v.s0), (double)(v.s1));
}

INLINE OVERLOADABLE float2 convert_float2(short2 v) {
  return (float2)((float)(v.s0), (float)(v.s1));
}

INLINE OVERLOADABLE long2 convert_long2(ushort2 v) {
  return (long2)((long)(v.s0), (long)(v.s1));
}

INLINE OVERLOADABLE ulong2 convert_ulong2(ushort2 v) {
  return (ulong2)((ulong)(v.s0), (ulong)(v.s1));
}

INLINE OVERLOADABLE int2 convert_int2(ushort2 v) {
  return (int2)((int)(v.s0), (int)(v.s1));
}

INLINE OVERLOADABLE uint2 convert_uint2(ushort2 v) {
  return (uint2)((uint)(v.s0), (uint)(v.s1));
}

INLINE OVERLOADABLE short2 convert_short2(ushort2 v) {
  return (short2)((short)(v.s0), (short)(v.s1));
}

INLINE OVERLOADABLE char2 convert_char2(ushort2 v) {
  return (char2)((char)(v.s0), (char)(v.s1));
}

INLINE OVERLOADABLE uchar2 convert_uchar2(ushort2 v) {
  return (uchar2)((uchar)(v.s0), (uchar)(v.s1));
}

INLINE OVERLOADABLE double2 convert_double2(ushort2 v) {
  return (double2)((double)(v.s0), (double)(v.s1));
}

INLINE OVERLOADABLE float2 convert_float2(ushort2 v) {
  return (float2)((float)(v.s0), (float)(v.s1));
}

INLINE OVERLOADABLE long2 convert_long2(char2 v) {
  return (long2)((long)(v.s0), (long)(v.s1));
}

INLINE OVERLOADABLE ulong2 convert_ulong2(char2 v) {
  return (ulong2)((ulong)(v.s0), (ulong)(v.s1));
}

INLINE OVERLOADABLE int2 convert_int2(char2 v) {
  return (int2)((int)(v.s0), (int)(v.s1));
}

INLINE OVERLOADABLE uint2 convert_uint2(char2 v) {
  return (uint2)((uint)(v.s0), (uint)(v.s1));
}

INLINE OVERLOADABLE short2 convert_short2(char2 v) {
  return (short2)((short)(v.s0), (short)(v.s1));
}

INLINE OVERLOADABLE ushort2 convert_ushort2(char2 v) {
  return (ushort2)((ushort)(v.s0), (ushort)(v.s1));
}

INLINE OVERLOADABLE uchar2 convert_uchar2(char2 v) {
  return (uchar2)((uchar)(v.s0), (uchar)(v.s1));
}

INLINE OVERLOADABLE double2 convert_double2(char2 v) {
  return (double2)((double)(v.s0), (double)(v.s1));
}

INLINE OVERLOADABLE float2 convert_float2(char2 v) {
  return (float2)((float)(v.s0), (float)(v.s1));
}

INLINE OVERLOADABLE long2 convert_long2(uchar2 v) {
  return (long2)((long)(v.s0), (long)(v.s1));
}

INLINE OVERLOADABLE ulong2 convert_ulong2(uchar2 v) {
  return (ulong2)((ulong)(v.s0), (ulong)(v.s1));
}

INLINE OVERLOADABLE int2 convert_int2(uchar2 v) {
  return (int2)((int)(v.s0), (int)(v.s1));
}

INLINE OVERLOADABLE uint2 convert_uint2(uchar2 v) {
  return (uint2)((uint)(v.s0), (uint)(v.s1));
}

INLINE OVERLOADABLE short2 convert_short2(uchar2 v) {
  return (short2)((short)(v.s0), (short)(v.s1));
}

INLINE OVERLOADABLE ushort2 convert_ushort2(uchar2 v) {
  return (ushort2)((ushort)(v.s0), (ushort)(v.s1));
}

INLINE OVERLOADABLE char2 convert_char2(uchar2 v) {
  return (char2)((char)(v.s0), (char)(v.s1));
}

INLINE OVERLOADABLE double2 convert_double2(uchar2 v) {
  return (double2)((double)(v.s0), (double)(v.s1));
}

INLINE OVERLOADABLE float2 convert_float2(uchar2 v) {
  return (float2)((float)(v.s0), (float)(v.s1));
}

INLINE OVERLOADABLE long2 convert_long2(double2 v) {
  return (long2)((long)(v.s0), (long)(v.s1));
}

INLINE OVERLOADABLE ulong2 convert_ulong2(double2 v) {
  return (ulong2)((ulong)(v.s0), (ulong)(v.s1));
}

INLINE OVERLOADABLE int2 convert_int2(double2 v) {
  return (int2)((int)(v.s0), (int)(v.s1));
}

INLINE OVERLOADABLE uint2 convert_uint2(double2 v) {
  return (uint2)((uint)(v.s0), (uint)(v.s1));
}

INLINE OVERLOADABLE short2 convert_short2(double2 v) {
  return (short2)((short)(v.s0), (short)(v.s1));
}

INLINE OVERLOADABLE ushort2 convert_ushort2(double2 v) {
  return (ushort2)((ushort)(v.s0), (ushort)(v.s1));
}

INLINE OVERLOADABLE char2 convert_char2(double2 v) {
  return (char2)((char)(v.s0), (char)(v.s1));
}

INLINE OVERLOADABLE uchar2 convert_uchar2(double2 v) {
  return (uchar2)((uchar)(v.s0), (uchar)(v.s1));
}

INLINE OVERLOADABLE float2 convert_float2(double2 v) {
  return (float2)((float)(v.s0), (float)(v.s1));
}

INLINE OVERLOADABLE long2 convert_long2(float2 v) {
  return (long2)((long)(v.s0), (long)(v.s1));
}

INLINE OVERLOADABLE ulong2 convert_ulong2(float2 v) {
  return (ulong2)((ulong)(v.s0), (ulong)(v.s1));
}

INLINE OVERLOADABLE int2 convert_int2(float2 v) {
  return (int2)((int)(v.s0), (int)(v.s1));
}

INLINE OVERLOADABLE uint2 convert_uint2(float2 v) {
  return (uint2)((uint)(v.s0), (uint)(v.s1));
}

INLINE OVERLOADABLE short2 convert_short2(float2 v) {
  return (short2)((short)(v.s0), (short)(v.s1));
}

INLINE OVERLOADABLE ushort2 convert_ushort2(float2 v) {
  return (ushort2)((ushort)(v.s0), (ushort)(v.s1));
}

INLINE OVERLOADABLE char2 convert_char2(float2 v) {
  return (char2)((char)(v.s0), (char)(v.s1));
}

INLINE OVERLOADABLE uchar2 convert_uchar2(float2 v) {
  return (uchar2)((uchar)(v.s0), (uchar)(v.s1));
}

INLINE OVERLOADABLE double2 convert_double2(float2 v) {
  return (double2)((double)(v.s0), (double)(v.s1));
}

INLINE OVERLOADABLE ulong3 convert_ulong3(long3 v) {
  return (ulong3)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2));
}

INLINE OVERLOADABLE int3 convert_int3(long3 v) {
  return (int3)((int)(v.s0), (int)(v.s1), (int)(v.s2));
}

INLINE OVERLOADABLE uint3 convert_uint3(long3 v) {
  return (uint3)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2));
}

INLINE OVERLOADABLE short3 convert_short3(long3 v) {
  return (short3)((short)(v.s0), (short)(v.s1), (short)(v.s2));
}

INLINE OVERLOADABLE ushort3 convert_ushort3(long3 v) {
  return (ushort3)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2));
}

INLINE OVERLOADABLE char3 convert_char3(long3 v) {
  return (char3)((char)(v.s0), (char)(v.s1), (char)(v.s2));
}

INLINE OVERLOADABLE uchar3 convert_uchar3(long3 v) {
  return (uchar3)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2));
}

INLINE OVERLOADABLE double3 convert_double3(long3 v) {
  return (double3)((double)(v.s0), (double)(v.s1), (double)(v.s2));
}

INLINE OVERLOADABLE float3 convert_float3(long3 v) {
  return (float3)((float)(v.s0), (float)(v.s1), (float)(v.s2));
}

INLINE OVERLOADABLE long3 convert_long3(ulong3 v) {
  return (long3)((long)(v.s0), (long)(v.s1), (long)(v.s2));
}

INLINE OVERLOADABLE int3 convert_int3(ulong3 v) {
  return (int3)((int)(v.s0), (int)(v.s1), (int)(v.s2));
}

INLINE OVERLOADABLE uint3 convert_uint3(ulong3 v) {
  return (uint3)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2));
}

INLINE OVERLOADABLE short3 convert_short3(ulong3 v) {
  return (short3)((short)(v.s0), (short)(v.s1), (short)(v.s2));
}

INLINE OVERLOADABLE ushort3 convert_ushort3(ulong3 v) {
  return (ushort3)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2));
}

INLINE OVERLOADABLE char3 convert_char3(ulong3 v) {
  return (char3)((char)(v.s0), (char)(v.s1), (char)(v.s2));
}

INLINE OVERLOADABLE uchar3 convert_uchar3(ulong3 v) {
  return (uchar3)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2));
}

INLINE OVERLOADABLE double3 convert_double3(ulong3 v) {
  return (double3)((double)(v.s0), (double)(v.s1), (double)(v.s2));
}

INLINE OVERLOADABLE float3 convert_float3(ulong3 v) {
  return (float3)((float)(v.s0), (float)(v.s1), (float)(v.s2));
}

INLINE OVERLOADABLE long3 convert_long3(int3 v) {
  return (long3)((long)(v.s0), (long)(v.s1), (long)(v.s2));
}

INLINE OVERLOADABLE ulong3 convert_ulong3(int3 v) {
  return (ulong3)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2));
}

INLINE OVERLOADABLE uint3 convert_uint3(int3 v) {
  return (uint3)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2));
}

INLINE OVERLOADABLE short3 convert_short3(int3 v) {
  return (short3)((short)(v.s0), (short)(v.s1), (short)(v.s2));
}

INLINE OVERLOADABLE ushort3 convert_ushort3(int3 v) {
  return (ushort3)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2));
}

INLINE OVERLOADABLE char3 convert_char3(int3 v) {
  return (char3)((char)(v.s0), (char)(v.s1), (char)(v.s2));
}

INLINE OVERLOADABLE uchar3 convert_uchar3(int3 v) {
  return (uchar3)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2));
}

INLINE OVERLOADABLE double3 convert_double3(int3 v) {
  return (double3)((double)(v.s0), (double)(v.s1), (double)(v.s2));
}

INLINE OVERLOADABLE float3 convert_float3(int3 v) {
  return (float3)((float)(v.s0), (float)(v.s1), (float)(v.s2));
}

INLINE OVERLOADABLE long3 convert_long3(uint3 v) {
  return (long3)((long)(v.s0), (long)(v.s1), (long)(v.s2));
}

INLINE OVERLOADABLE ulong3 convert_ulong3(uint3 v) {
  return (ulong3)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2));
}

INLINE OVERLOADABLE int3 convert_int3(uint3 v) {
  return (int3)((int)(v.s0), (int)(v.s1), (int)(v.s2));
}

INLINE OVERLOADABLE short3 convert_short3(uint3 v) {
  return (short3)((short)(v.s0), (short)(v.s1), (short)(v.s2));
}

INLINE OVERLOADABLE ushort3 convert_ushort3(uint3 v) {
  return (ushort3)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2));
}

INLINE OVERLOADABLE char3 convert_char3(uint3 v) {
  return (char3)((char)(v.s0), (char)(v.s1), (char)(v.s2));
}

INLINE OVERLOADABLE uchar3 convert_uchar3(uint3 v) {
  return (uchar3)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2));
}

INLINE OVERLOADABLE double3 convert_double3(uint3 v) {
  return (double3)((double)(v.s0), (double)(v.s1), (double)(v.s2));
}

INLINE OVERLOADABLE float3 convert_float3(uint3 v) {
  return (float3)((float)(v.s0), (float)(v.s1), (float)(v.s2));
}

INLINE OVERLOADABLE long3 convert_long3(short3 v) {
  return (long3)((long)(v.s0), (long)(v.s1), (long)(v.s2));
}

INLINE OVERLOADABLE ulong3 convert_ulong3(short3 v) {
  return (ulong3)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2));
}

INLINE OVERLOADABLE int3 convert_int3(short3 v) {
  return (int3)((int)(v.s0), (int)(v.s1), (int)(v.s2));
}

INLINE OVERLOADABLE uint3 convert_uint3(short3 v) {
  return (uint3)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2));
}

INLINE OVERLOADABLE ushort3 convert_ushort3(short3 v) {
  return (ushort3)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2));
}

INLINE OVERLOADABLE char3 convert_char3(short3 v) {
  return (char3)((char)(v.s0), (char)(v.s1), (char)(v.s2));
}

INLINE OVERLOADABLE uchar3 convert_uchar3(short3 v) {
  return (uchar3)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2));
}

INLINE OVERLOADABLE double3 convert_double3(short3 v) {
  return (double3)((double)(v.s0), (double)(v.s1), (double)(v.s2));
}

INLINE OVERLOADABLE float3 convert_float3(short3 v) {
  return (float3)((float)(v.s0), (float)(v.s1), (float)(v.s2));
}

INLINE OVERLOADABLE long3 convert_long3(ushort3 v) {
  return (long3)((long)(v.s0), (long)(v.s1), (long)(v.s2));
}

INLINE OVERLOADABLE ulong3 convert_ulong3(ushort3 v) {
  return (ulong3)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2));
}

INLINE OVERLOADABLE int3 convert_int3(ushort3 v) {
  return (int3)((int)(v.s0), (int)(v.s1), (int)(v.s2));
}

INLINE OVERLOADABLE uint3 convert_uint3(ushort3 v) {
  return (uint3)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2));
}

INLINE OVERLOADABLE short3 convert_short3(ushort3 v) {
  return (short3)((short)(v.s0), (short)(v.s1), (short)(v.s2));
}

INLINE OVERLOADABLE char3 convert_char3(ushort3 v) {
  return (char3)((char)(v.s0), (char)(v.s1), (char)(v.s2));
}

INLINE OVERLOADABLE uchar3 convert_uchar3(ushort3 v) {
  return (uchar3)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2));
}

INLINE OVERLOADABLE double3 convert_double3(ushort3 v) {
  return (double3)((double)(v.s0), (double)(v.s1), (double)(v.s2));
}

INLINE OVERLOADABLE float3 convert_float3(ushort3 v) {
  return (float3)((float)(v.s0), (float)(v.s1), (float)(v.s2));
}

INLINE OVERLOADABLE long3 convert_long3(char3 v) {
  return (long3)((long)(v.s0), (long)(v.s1), (long)(v.s2));
}

INLINE OVERLOADABLE ulong3 convert_ulong3(char3 v) {
  return (ulong3)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2));
}

INLINE OVERLOADABLE int3 convert_int3(char3 v) {
  return (int3)((int)(v.s0), (int)(v.s1), (int)(v.s2));
}

INLINE OVERLOADABLE uint3 convert_uint3(char3 v) {
  return (uint3)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2));
}

INLINE OVERLOADABLE short3 convert_short3(char3 v) {
  return (short3)((short)(v.s0), (short)(v.s1), (short)(v.s2));
}

INLINE OVERLOADABLE ushort3 convert_ushort3(char3 v) {
  return (ushort3)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2));
}

INLINE OVERLOADABLE uchar3 convert_uchar3(char3 v) {
  return (uchar3)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2));
}

INLINE OVERLOADABLE double3 convert_double3(char3 v) {
  return (double3)((double)(v.s0), (double)(v.s1), (double)(v.s2));
}

INLINE OVERLOADABLE float3 convert_float3(char3 v) {
  return (float3)((float)(v.s0), (float)(v.s1), (float)(v.s2));
}

INLINE OVERLOADABLE long3 convert_long3(uchar3 v) {
  return (long3)((long)(v.s0), (long)(v.s1), (long)(v.s2));
}

INLINE OVERLOADABLE ulong3 convert_ulong3(uchar3 v) {
  return (ulong3)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2));
}

INLINE OVERLOADABLE int3 convert_int3(uchar3 v) {
  return (int3)((int)(v.s0), (int)(v.s1), (int)(v.s2));
}

INLINE OVERLOADABLE uint3 convert_uint3(uchar3 v) {
  return (uint3)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2));
}

INLINE OVERLOADABLE short3 convert_short3(uchar3 v) {
  return (short3)((short)(v.s0), (short)(v.s1), (short)(v.s2));
}

INLINE OVERLOADABLE ushort3 convert_ushort3(uchar3 v) {
  return (ushort3)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2));
}

INLINE OVERLOADABLE char3 convert_char3(uchar3 v) {
  return (char3)((char)(v.s0), (char)(v.s1), (char)(v.s2));
}

INLINE OVERLOADABLE double3 convert_double3(uchar3 v) {
  return (double3)((double)(v.s0), (double)(v.s1), (double)(v.s2));
}

INLINE OVERLOADABLE float3 convert_float3(uchar3 v) {
  return (float3)((float)(v.s0), (float)(v.s1), (float)(v.s2));
}

INLINE OVERLOADABLE long3 convert_long3(double3 v) {
  return (long3)((long)(v.s0), (long)(v.s1), (long)(v.s2));
}

INLINE OVERLOADABLE ulong3 convert_ulong3(double3 v) {
  return (ulong3)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2));
}

INLINE OVERLOADABLE int3 convert_int3(double3 v) {
  return (int3)((int)(v.s0), (int)(v.s1), (int)(v.s2));
}

INLINE OVERLOADABLE uint3 convert_uint3(double3 v) {
  return (uint3)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2));
}

INLINE OVERLOADABLE short3 convert_short3(double3 v) {
  return (short3)((short)(v.s0), (short)(v.s1), (short)(v.s2));
}

INLINE OVERLOADABLE ushort3 convert_ushort3(double3 v) {
  return (ushort3)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2));
}

INLINE OVERLOADABLE char3 convert_char3(double3 v) {
  return (char3)((char)(v.s0), (char)(v.s1), (char)(v.s2));
}

INLINE OVERLOADABLE uchar3 convert_uchar3(double3 v) {
  return (uchar3)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2));
}

INLINE OVERLOADABLE float3 convert_float3(double3 v) {
  return (float3)((float)(v.s0), (float)(v.s1), (float)(v.s2));
}

INLINE OVERLOADABLE long3 convert_long3(float3 v) {
  return (long3)((long)(v.s0), (long)(v.s1), (long)(v.s2));
}

INLINE OVERLOADABLE ulong3 convert_ulong3(float3 v) {
  return (ulong3)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2));
}

INLINE OVERLOADABLE int3 convert_int3(float3 v) {
  return (int3)((int)(v.s0), (int)(v.s1), (int)(v.s2));
}

INLINE OVERLOADABLE uint3 convert_uint3(float3 v) {
  return (uint3)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2));
}

INLINE OVERLOADABLE short3 convert_short3(float3 v) {
  return (short3)((short)(v.s0), (short)(v.s1), (short)(v.s2));
}

INLINE OVERLOADABLE ushort3 convert_ushort3(float3 v) {
  return (ushort3)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2));
}

INLINE OVERLOADABLE char3 convert_char3(float3 v) {
  return (char3)((char)(v.s0), (char)(v.s1), (char)(v.s2));
}

INLINE OVERLOADABLE uchar3 convert_uchar3(float3 v) {
  return (uchar3)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2));
}

INLINE OVERLOADABLE double3 convert_double3(float3 v) {
  return (double3)((double)(v.s0), (double)(v.s1), (double)(v.s2));
}

INLINE OVERLOADABLE ulong4 convert_ulong4(long4 v) {
  return (ulong4)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3));
}

INLINE OVERLOADABLE int4 convert_int4(long4 v) {
  return (int4)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3));
}

INLINE OVERLOADABLE uint4 convert_uint4(long4 v) {
  return (uint4)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3));
}

INLINE OVERLOADABLE short4 convert_short4(long4 v) {
  return (short4)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3));
}

INLINE OVERLOADABLE ushort4 convert_ushort4(long4 v) {
  return (ushort4)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3));
}

INLINE OVERLOADABLE char4 convert_char4(long4 v) {
  return (char4)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3));
}

INLINE OVERLOADABLE uchar4 convert_uchar4(long4 v) {
  return (uchar4)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3));
}

INLINE OVERLOADABLE double4 convert_double4(long4 v) {
  return (double4)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3));
}

INLINE OVERLOADABLE float4 convert_float4(long4 v) {
  return (float4)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3));
}

INLINE OVERLOADABLE long4 convert_long4(ulong4 v) {
  return (long4)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3));
}

INLINE OVERLOADABLE int4 convert_int4(ulong4 v) {
  return (int4)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3));
}

INLINE OVERLOADABLE uint4 convert_uint4(ulong4 v) {
  return (uint4)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3));
}

INLINE OVERLOADABLE short4 convert_short4(ulong4 v) {
  return (short4)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3));
}

INLINE OVERLOADABLE ushort4 convert_ushort4(ulong4 v) {
  return (ushort4)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3));
}

INLINE OVERLOADABLE char4 convert_char4(ulong4 v) {
  return (char4)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3));
}

INLINE OVERLOADABLE uchar4 convert_uchar4(ulong4 v) {
  return (uchar4)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3));
}

INLINE OVERLOADABLE double4 convert_double4(ulong4 v) {
  return (double4)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3));
}

INLINE OVERLOADABLE float4 convert_float4(ulong4 v) {
  return (float4)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3));
}

INLINE OVERLOADABLE long4 convert_long4(int4 v) {
  return (long4)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3));
}

INLINE OVERLOADABLE ulong4 convert_ulong4(int4 v) {
  return (ulong4)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3));
}

INLINE OVERLOADABLE uint4 convert_uint4(int4 v) {
  return (uint4)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3));
}

INLINE OVERLOADABLE short4 convert_short4(int4 v) {
  return (short4)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3));
}

INLINE OVERLOADABLE ushort4 convert_ushort4(int4 v) {
  return (ushort4)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3));
}

INLINE OVERLOADABLE char4 convert_char4(int4 v) {
  return (char4)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3));
}

INLINE OVERLOADABLE uchar4 convert_uchar4(int4 v) {
  return (uchar4)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3));
}

INLINE OVERLOADABLE double4 convert_double4(int4 v) {
  return (double4)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3));
}

INLINE OVERLOADABLE float4 convert_float4(int4 v) {
  return (float4)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3));
}

INLINE OVERLOADABLE long4 convert_long4(uint4 v) {
  return (long4)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3));
}

INLINE OVERLOADABLE ulong4 convert_ulong4(uint4 v) {
  return (ulong4)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3));
}

INLINE OVERLOADABLE int4 convert_int4(uint4 v) {
  return (int4)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3));
}

INLINE OVERLOADABLE short4 convert_short4(uint4 v) {
  return (short4)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3));
}

INLINE OVERLOADABLE ushort4 convert_ushort4(uint4 v) {
  return (ushort4)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3));
}

INLINE OVERLOADABLE char4 convert_char4(uint4 v) {
  return (char4)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3));
}

INLINE OVERLOADABLE uchar4 convert_uchar4(uint4 v) {
  return (uchar4)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3));
}

INLINE OVERLOADABLE double4 convert_double4(uint4 v) {
  return (double4)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3));
}

INLINE OVERLOADABLE float4 convert_float4(uint4 v) {
  return (float4)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3));
}

INLINE OVERLOADABLE long4 convert_long4(short4 v) {
  return (long4)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3));
}

INLINE OVERLOADABLE ulong4 convert_ulong4(short4 v) {
  return (ulong4)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3));
}

INLINE OVERLOADABLE int4 convert_int4(short4 v) {
  return (int4)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3));
}

INLINE OVERLOADABLE uint4 convert_uint4(short4 v) {
  return (uint4)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3));
}

INLINE OVERLOADABLE ushort4 convert_ushort4(short4 v) {
  return (ushort4)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3));
}

INLINE OVERLOADABLE char4 convert_char4(short4 v) {
  return (char4)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3));
}

INLINE OVERLOADABLE uchar4 convert_uchar4(short4 v) {
  return (uchar4)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3));
}

INLINE OVERLOADABLE double4 convert_double4(short4 v) {
  return (double4)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3));
}

INLINE OVERLOADABLE float4 convert_float4(short4 v) {
  return (float4)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3));
}

INLINE OVERLOADABLE long4 convert_long4(ushort4 v) {
  return (long4)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3));
}

INLINE OVERLOADABLE ulong4 convert_ulong4(ushort4 v) {
  return (ulong4)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3));
}

INLINE OVERLOADABLE int4 convert_int4(ushort4 v) {
  return (int4)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3));
}

INLINE OVERLOADABLE uint4 convert_uint4(ushort4 v) {
  return (uint4)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3));
}

INLINE OVERLOADABLE short4 convert_short4(ushort4 v) {
  return (short4)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3));
}

INLINE OVERLOADABLE char4 convert_char4(ushort4 v) {
  return (char4)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3));
}

INLINE OVERLOADABLE uchar4 convert_uchar4(ushort4 v) {
  return (uchar4)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3));
}

INLINE OVERLOADABLE double4 convert_double4(ushort4 v) {
  return (double4)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3));
}

INLINE OVERLOADABLE float4 convert_float4(ushort4 v) {
  return (float4)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3));
}

INLINE OVERLOADABLE long4 convert_long4(char4 v) {
  return (long4)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3));
}

INLINE OVERLOADABLE ulong4 convert_ulong4(char4 v) {
  return (ulong4)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3));
}

INLINE OVERLOADABLE int4 convert_int4(char4 v) {
  return (int4)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3));
}

INLINE OVERLOADABLE uint4 convert_uint4(char4 v) {
  return (uint4)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3));
}

INLINE OVERLOADABLE short4 convert_short4(char4 v) {
  return (short4)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3));
}

INLINE OVERLOADABLE ushort4 convert_ushort4(char4 v) {
  return (ushort4)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3));
}

INLINE OVERLOADABLE uchar4 convert_uchar4(char4 v) {
  return (uchar4)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3));
}

INLINE OVERLOADABLE double4 convert_double4(char4 v) {
  return (double4)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3));
}

INLINE OVERLOADABLE float4 convert_float4(char4 v) {
  return (float4)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3));
}

INLINE OVERLOADABLE long4 convert_long4(uchar4 v) {
  return (long4)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3));
}

INLINE OVERLOADABLE ulong4 convert_ulong4(uchar4 v) {
  return (ulong4)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3));
}

INLINE OVERLOADABLE int4 convert_int4(uchar4 v) {
  return (int4)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3));
}

INLINE OVERLOADABLE uint4 convert_uint4(uchar4 v) {
  return (uint4)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3));
}

INLINE OVERLOADABLE short4 convert_short4(uchar4 v) {
  return (short4)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3));
}

INLINE OVERLOADABLE ushort4 convert_ushort4(uchar4 v) {
  return (ushort4)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3));
}

INLINE OVERLOADABLE char4 convert_char4(uchar4 v) {
  return (char4)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3));
}

INLINE OVERLOADABLE double4 convert_double4(uchar4 v) {
  return (double4)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3));
}

INLINE OVERLOADABLE float4 convert_float4(uchar4 v) {
  return (float4)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3));
}

INLINE OVERLOADABLE long4 convert_long4(double4 v) {
  return (long4)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3));
}

INLINE OVERLOADABLE ulong4 convert_ulong4(double4 v) {
  return (ulong4)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3));
}

INLINE OVERLOADABLE int4 convert_int4(double4 v) {
  return (int4)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3));
}

INLINE OVERLOADABLE uint4 convert_uint4(double4 v) {
  return (uint4)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3));
}

INLINE OVERLOADABLE short4 convert_short4(double4 v) {
  return (short4)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3));
}

INLINE OVERLOADABLE ushort4 convert_ushort4(double4 v) {
  return (ushort4)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3));
}

INLINE OVERLOADABLE char4 convert_char4(double4 v) {
  return (char4)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3));
}

INLINE OVERLOADABLE uchar4 convert_uchar4(double4 v) {
  return (uchar4)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3));
}

INLINE OVERLOADABLE float4 convert_float4(double4 v) {
  return (float4)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3));
}

INLINE OVERLOADABLE long4 convert_long4(float4 v) {
  return (long4)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3));
}

INLINE OVERLOADABLE ulong4 convert_ulong4(float4 v) {
  return (ulong4)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3));
}

INLINE OVERLOADABLE int4 convert_int4(float4 v) {
  return (int4)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3));
}

INLINE OVERLOADABLE uint4 convert_uint4(float4 v) {
  return (uint4)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3));
}

INLINE OVERLOADABLE short4 convert_short4(float4 v) {
  return (short4)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3));
}

INLINE OVERLOADABLE ushort4 convert_ushort4(float4 v) {
  return (ushort4)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3));
}

INLINE OVERLOADABLE char4 convert_char4(float4 v) {
  return (char4)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3));
}

INLINE OVERLOADABLE uchar4 convert_uchar4(float4 v) {
  return (uchar4)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3));
}

INLINE OVERLOADABLE double4 convert_double4(float4 v) {
  return (double4)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3));
}

INLINE OVERLOADABLE ulong8 convert_ulong8(long8 v) {
  return (ulong8)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7));
}

INLINE OVERLOADABLE int8 convert_int8(long8 v) {
  return (int8)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7));
}

INLINE OVERLOADABLE uint8 convert_uint8(long8 v) {
  return (uint8)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7));
}

INLINE OVERLOADABLE short8 convert_short8(long8 v) {
  return (short8)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7));
}

INLINE OVERLOADABLE ushort8 convert_ushort8(long8 v) {
  return (ushort8)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7));
}

INLINE OVERLOADABLE char8 convert_char8(long8 v) {
  return (char8)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7));
}

INLINE OVERLOADABLE uchar8 convert_uchar8(long8 v) {
  return (uchar8)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7));
}

INLINE OVERLOADABLE double8 convert_double8(long8 v) {
  return (double8)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7));
}

INLINE OVERLOADABLE float8 convert_float8(long8 v) {
  return (float8)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7));
}

INLINE OVERLOADABLE long8 convert_long8(ulong8 v) {
  return (long8)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7));
}

INLINE OVERLOADABLE int8 convert_int8(ulong8 v) {
  return (int8)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7));
}

INLINE OVERLOADABLE uint8 convert_uint8(ulong8 v) {
  return (uint8)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7));
}

INLINE OVERLOADABLE short8 convert_short8(ulong8 v) {
  return (short8)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7));
}

INLINE OVERLOADABLE ushort8 convert_ushort8(ulong8 v) {
  return (ushort8)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7));
}

INLINE OVERLOADABLE char8 convert_char8(ulong8 v) {
  return (char8)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7));
}

INLINE OVERLOADABLE uchar8 convert_uchar8(ulong8 v) {
  return (uchar8)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7));
}

INLINE OVERLOADABLE double8 convert_double8(ulong8 v) {
  return (double8)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7));
}

INLINE OVERLOADABLE float8 convert_float8(ulong8 v) {
  return (float8)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7));
}

INLINE OVERLOADABLE long8 convert_long8(int8 v) {
  return (long8)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7));
}

INLINE OVERLOADABLE ulong8 convert_ulong8(int8 v) {
  return (ulong8)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7));
}

INLINE OVERLOADABLE uint8 convert_uint8(int8 v) {
  return (uint8)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7));
}

INLINE OVERLOADABLE short8 convert_short8(int8 v) {
  return (short8)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7));
}

INLINE OVERLOADABLE ushort8 convert_ushort8(int8 v) {
  return (ushort8)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7));
}

INLINE OVERLOADABLE char8 convert_char8(int8 v) {
  return (char8)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7));
}

INLINE OVERLOADABLE uchar8 convert_uchar8(int8 v) {
  return (uchar8)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7));
}

INLINE OVERLOADABLE double8 convert_double8(int8 v) {
  return (double8)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7));
}

INLINE OVERLOADABLE float8 convert_float8(int8 v) {
  return (float8)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7));
}

INLINE OVERLOADABLE long8 convert_long8(uint8 v) {
  return (long8)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7));
}

INLINE OVERLOADABLE ulong8 convert_ulong8(uint8 v) {
  return (ulong8)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7));
}

INLINE OVERLOADABLE int8 convert_int8(uint8 v) {
  return (int8)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7));
}

INLINE OVERLOADABLE short8 convert_short8(uint8 v) {
  return (short8)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7));
}

INLINE OVERLOADABLE ushort8 convert_ushort8(uint8 v) {
  return (ushort8)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7));
}

INLINE OVERLOADABLE char8 convert_char8(uint8 v) {
  return (char8)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7));
}

INLINE OVERLOADABLE uchar8 convert_uchar8(uint8 v) {
  return (uchar8)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7));
}

INLINE OVERLOADABLE double8 convert_double8(uint8 v) {
  return (double8)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7));
}

INLINE OVERLOADABLE float8 convert_float8(uint8 v) {
  return (float8)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7));
}

INLINE OVERLOADABLE long8 convert_long8(short8 v) {
  return (long8)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7));
}

INLINE OVERLOADABLE ulong8 convert_ulong8(short8 v) {
  return (ulong8)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7));
}

INLINE OVERLOADABLE int8 convert_int8(short8 v) {
  return (int8)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7));
}

INLINE OVERLOADABLE uint8 convert_uint8(short8 v) {
  return (uint8)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7));
}

INLINE OVERLOADABLE ushort8 convert_ushort8(short8 v) {
  return (ushort8)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7));
}

INLINE OVERLOADABLE char8 convert_char8(short8 v) {
  return (char8)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7));
}

INLINE OVERLOADABLE uchar8 convert_uchar8(short8 v) {
  return (uchar8)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7));
}

INLINE OVERLOADABLE double8 convert_double8(short8 v) {
  return (double8)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7));
}

INLINE OVERLOADABLE float8 convert_float8(short8 v) {
  return (float8)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7));
}

INLINE OVERLOADABLE long8 convert_long8(ushort8 v) {
  return (long8)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7));
}

INLINE OVERLOADABLE ulong8 convert_ulong8(ushort8 v) {
  return (ulong8)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7));
}

INLINE OVERLOADABLE int8 convert_int8(ushort8 v) {
  return (int8)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7));
}

INLINE OVERLOADABLE uint8 convert_uint8(ushort8 v) {
  return (uint8)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7));
}

INLINE OVERLOADABLE short8 convert_short8(ushort8 v) {
  return (short8)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7));
}

INLINE OVERLOADABLE char8 convert_char8(ushort8 v) {
  return (char8)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7));
}

INLINE OVERLOADABLE uchar8 convert_uchar8(ushort8 v) {
  return (uchar8)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7));
}

INLINE OVERLOADABLE double8 convert_double8(ushort8 v) {
  return (double8)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7));
}

INLINE OVERLOADABLE float8 convert_float8(ushort8 v) {
  return (float8)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7));
}

INLINE OVERLOADABLE long8 convert_long8(char8 v) {
  return (long8)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7));
}

INLINE OVERLOADABLE ulong8 convert_ulong8(char8 v) {
  return (ulong8)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7));
}

INLINE OVERLOADABLE int8 convert_int8(char8 v) {
  return (int8)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7));
}

INLINE OVERLOADABLE uint8 convert_uint8(char8 v) {
  return (uint8)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7));
}

INLINE OVERLOADABLE short8 convert_short8(char8 v) {
  return (short8)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7));
}

INLINE OVERLOADABLE ushort8 convert_ushort8(char8 v) {
  return (ushort8)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7));
}

INLINE OVERLOADABLE uchar8 convert_uchar8(char8 v) {
  return (uchar8)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7));
}

INLINE OVERLOADABLE double8 convert_double8(char8 v) {
  return (double8)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7));
}

INLINE OVERLOADABLE float8 convert_float8(char8 v) {
  return (float8)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7));
}

INLINE OVERLOADABLE long8 convert_long8(uchar8 v) {
  return (long8)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7));
}

INLINE OVERLOADABLE ulong8 convert_ulong8(uchar8 v) {
  return (ulong8)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7));
}

INLINE OVERLOADABLE int8 convert_int8(uchar8 v) {
  return (int8)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7));
}

INLINE OVERLOADABLE uint8 convert_uint8(uchar8 v) {
  return (uint8)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7));
}

INLINE OVERLOADABLE short8 convert_short8(uchar8 v) {
  return (short8)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7));
}

INLINE OVERLOADABLE ushort8 convert_ushort8(uchar8 v) {
  return (ushort8)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7));
}

INLINE OVERLOADABLE char8 convert_char8(uchar8 v) {
  return (char8)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7));
}

INLINE OVERLOADABLE double8 convert_double8(uchar8 v) {
  return (double8)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7));
}

INLINE OVERLOADABLE float8 convert_float8(uchar8 v) {
  return (float8)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7));
}

INLINE OVERLOADABLE long8 convert_long8(double8 v) {
  return (long8)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7));
}

INLINE OVERLOADABLE ulong8 convert_ulong8(double8 v) {
  return (ulong8)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7));
}

INLINE OVERLOADABLE int8 convert_int8(double8 v) {
  return (int8)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7));
}

INLINE OVERLOADABLE uint8 convert_uint8(double8 v) {
  return (uint8)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7));
}

INLINE OVERLOADABLE short8 convert_short8(double8 v) {
  return (short8)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7));
}

INLINE OVERLOADABLE ushort8 convert_ushort8(double8 v) {
  return (ushort8)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7));
}

INLINE OVERLOADABLE char8 convert_char8(double8 v) {
  return (char8)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7));
}

INLINE OVERLOADABLE uchar8 convert_uchar8(double8 v) {
  return (uchar8)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7));
}

INLINE OVERLOADABLE float8 convert_float8(double8 v) {
  return (float8)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7));
}

INLINE OVERLOADABLE long8 convert_long8(float8 v) {
  return (long8)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7));
}

INLINE OVERLOADABLE ulong8 convert_ulong8(float8 v) {
  return (ulong8)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7));
}

INLINE OVERLOADABLE int8 convert_int8(float8 v) {
  return (int8)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7));
}

INLINE OVERLOADABLE uint8 convert_uint8(float8 v) {
  return (uint8)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7));
}

INLINE OVERLOADABLE short8 convert_short8(float8 v) {
  return (short8)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7));
}

INLINE OVERLOADABLE ushort8 convert_ushort8(float8 v) {
  return (ushort8)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7));
}

INLINE OVERLOADABLE char8 convert_char8(float8 v) {
  return (char8)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7));
}

INLINE OVERLOADABLE uchar8 convert_uchar8(float8 v) {
  return (uchar8)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7));
}

INLINE OVERLOADABLE double8 convert_double8(float8 v) {
  return (double8)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7));
}

INLINE OVERLOADABLE ulong16 convert_ulong16(long16 v) {
  return (ulong16)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7), (ulong)(v.s8), (ulong)(v.s9), (ulong)(v.sA), (ulong)(v.sB), (ulong)(v.sC), (ulong)(v.sD), (ulong)(v.sE), (ulong)(v.sF));
}

INLINE OVERLOADABLE int16 convert_int16(long16 v) {
  return (int16)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7), (int)(v.s8), (int)(v.s9), (int)(v.sA), (int)(v.sB), (int)(v.sC), (int)(v.sD), (int)(v.sE), (int)(v.sF));
}

INLINE OVERLOADABLE uint16 convert_uint16(long16 v) {
  return (uint16)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7), (uint)(v.s8), (uint)(v.s9), (uint)(v.sA), (uint)(v.sB), (uint)(v.sC), (uint)(v.sD), (uint)(v.sE), (uint)(v.sF));
}

INLINE OVERLOADABLE short16 convert_short16(long16 v) {
  return (short16)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7), (short)(v.s8), (short)(v.s9), (short)(v.sA), (short)(v.sB), (short)(v.sC), (short)(v.sD), (short)(v.sE), (short)(v.sF));
}

INLINE OVERLOADABLE ushort16 convert_ushort16(long16 v) {
  return (ushort16)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7), (ushort)(v.s8), (ushort)(v.s9), (ushort)(v.sA), (ushort)(v.sB), (ushort)(v.sC), (ushort)(v.sD), (ushort)(v.sE), (ushort)(v.sF));
}

INLINE OVERLOADABLE char16 convert_char16(long16 v) {
  return (char16)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7), (char)(v.s8), (char)(v.s9), (char)(v.sA), (char)(v.sB), (char)(v.sC), (char)(v.sD), (char)(v.sE), (char)(v.sF));
}

INLINE OVERLOADABLE uchar16 convert_uchar16(long16 v) {
  return (uchar16)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7), (uchar)(v.s8), (uchar)(v.s9), (uchar)(v.sA), (uchar)(v.sB), (uchar)(v.sC), (uchar)(v.sD), (uchar)(v.sE), (uchar)(v.sF));
}

INLINE OVERLOADABLE double16 convert_double16(long16 v) {
  return (double16)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7), (double)(v.s8), (double)(v.s9), (double)(v.sA), (double)(v.sB), (double)(v.sC), (double)(v.sD), (double)(v.sE), (double)(v.sF));
}

INLINE OVERLOADABLE float16 convert_float16(long16 v) {
  return (float16)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7), (float)(v.s8), (float)(v.s9), (float)(v.sA), (float)(v.sB), (float)(v.sC), (float)(v.sD), (float)(v.sE), (float)(v.sF));
}

INLINE OVERLOADABLE long16 convert_long16(ulong16 v) {
  return (long16)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7), (long)(v.s8), (long)(v.s9), (long)(v.sA), (long)(v.sB), (long)(v.sC), (long)(v.sD), (long)(v.sE), (long)(v.sF));
}

INLINE OVERLOADABLE int16 convert_int16(ulong16 v) {
  return (int16)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7), (int)(v.s8), (int)(v.s9), (int)(v.sA), (int)(v.sB), (int)(v.sC), (int)(v.sD), (int)(v.sE), (int)(v.sF));
}

INLINE OVERLOADABLE uint16 convert_uint16(ulong16 v) {
  return (uint16)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7), (uint)(v.s8), (uint)(v.s9), (uint)(v.sA), (uint)(v.sB), (uint)(v.sC), (uint)(v.sD), (uint)(v.sE), (uint)(v.sF));
}

INLINE OVERLOADABLE short16 convert_short16(ulong16 v) {
  return (short16)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7), (short)(v.s8), (short)(v.s9), (short)(v.sA), (short)(v.sB), (short)(v.sC), (short)(v.sD), (short)(v.sE), (short)(v.sF));
}

INLINE OVERLOADABLE ushort16 convert_ushort16(ulong16 v) {
  return (ushort16)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7), (ushort)(v.s8), (ushort)(v.s9), (ushort)(v.sA), (ushort)(v.sB), (ushort)(v.sC), (ushort)(v.sD), (ushort)(v.sE), (ushort)(v.sF));
}

INLINE OVERLOADABLE char16 convert_char16(ulong16 v) {
  return (char16)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7), (char)(v.s8), (char)(v.s9), (char)(v.sA), (char)(v.sB), (char)(v.sC), (char)(v.sD), (char)(v.sE), (char)(v.sF));
}

INLINE OVERLOADABLE uchar16 convert_uchar16(ulong16 v) {
  return (uchar16)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7), (uchar)(v.s8), (uchar)(v.s9), (uchar)(v.sA), (uchar)(v.sB), (uchar)(v.sC), (uchar)(v.sD), (uchar)(v.sE), (uchar)(v.sF));
}

INLINE OVERLOADABLE double16 convert_double16(ulong16 v) {
  return (double16)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7), (double)(v.s8), (double)(v.s9), (double)(v.sA), (double)(v.sB), (double)(v.sC), (double)(v.sD), (double)(v.sE), (double)(v.sF));
}

INLINE OVERLOADABLE float16 convert_float16(ulong16 v) {
  return (float16)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7), (float)(v.s8), (float)(v.s9), (float)(v.sA), (float)(v.sB), (float)(v.sC), (float)(v.sD), (float)(v.sE), (float)(v.sF));
}

INLINE OVERLOADABLE long16 convert_long16(int16 v) {
  return (long16)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7), (long)(v.s8), (long)(v.s9), (long)(v.sA), (long)(v.sB), (long)(v.sC), (long)(v.sD), (long)(v.sE), (long)(v.sF));
}

INLINE OVERLOADABLE ulong16 convert_ulong16(int16 v) {
  return (ulong16)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7), (ulong)(v.s8), (ulong)(v.s9), (ulong)(v.sA), (ulong)(v.sB), (ulong)(v.sC), (ulong)(v.sD), (ulong)(v.sE), (ulong)(v.sF));
}

INLINE OVERLOADABLE uint16 convert_uint16(int16 v) {
  return (uint16)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7), (uint)(v.s8), (uint)(v.s9), (uint)(v.sA), (uint)(v.sB), (uint)(v.sC), (uint)(v.sD), (uint)(v.sE), (uint)(v.sF));
}

INLINE OVERLOADABLE short16 convert_short16(int16 v) {
  return (short16)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7), (short)(v.s8), (short)(v.s9), (short)(v.sA), (short)(v.sB), (short)(v.sC), (short)(v.sD), (short)(v.sE), (short)(v.sF));
}

INLINE OVERLOADABLE ushort16 convert_ushort16(int16 v) {
  return (ushort16)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7), (ushort)(v.s8), (ushort)(v.s9), (ushort)(v.sA), (ushort)(v.sB), (ushort)(v.sC), (ushort)(v.sD), (ushort)(v.sE), (ushort)(v.sF));
}

INLINE OVERLOADABLE char16 convert_char16(int16 v) {
  return (char16)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7), (char)(v.s8), (char)(v.s9), (char)(v.sA), (char)(v.sB), (char)(v.sC), (char)(v.sD), (char)(v.sE), (char)(v.sF));
}

INLINE OVERLOADABLE uchar16 convert_uchar16(int16 v) {
  return (uchar16)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7), (uchar)(v.s8), (uchar)(v.s9), (uchar)(v.sA), (uchar)(v.sB), (uchar)(v.sC), (uchar)(v.sD), (uchar)(v.sE), (uchar)(v.sF));
}

INLINE OVERLOADABLE double16 convert_double16(int16 v) {
  return (double16)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7), (double)(v.s8), (double)(v.s9), (double)(v.sA), (double)(v.sB), (double)(v.sC), (double)(v.sD), (double)(v.sE), (double)(v.sF));
}

INLINE OVERLOADABLE float16 convert_float16(int16 v) {
  return (float16)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7), (float)(v.s8), (float)(v.s9), (float)(v.sA), (float)(v.sB), (float)(v.sC), (float)(v.sD), (float)(v.sE), (float)(v.sF));
}

INLINE OVERLOADABLE long16 convert_long16(uint16 v) {
  return (long16)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7), (long)(v.s8), (long)(v.s9), (long)(v.sA), (long)(v.sB), (long)(v.sC), (long)(v.sD), (long)(v.sE), (long)(v.sF));
}

INLINE OVERLOADABLE ulong16 convert_ulong16(uint16 v) {
  return (ulong16)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7), (ulong)(v.s8), (ulong)(v.s9), (ulong)(v.sA), (ulong)(v.sB), (ulong)(v.sC), (ulong)(v.sD), (ulong)(v.sE), (ulong)(v.sF));
}

INLINE OVERLOADABLE int16 convert_int16(uint16 v) {
  return (int16)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7), (int)(v.s8), (int)(v.s9), (int)(v.sA), (int)(v.sB), (int)(v.sC), (int)(v.sD), (int)(v.sE), (int)(v.sF));
}

INLINE OVERLOADABLE short16 convert_short16(uint16 v) {
  return (short16)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7), (short)(v.s8), (short)(v.s9), (short)(v.sA), (short)(v.sB), (short)(v.sC), (short)(v.sD), (short)(v.sE), (short)(v.sF));
}

INLINE OVERLOADABLE ushort16 convert_ushort16(uint16 v) {
  return (ushort16)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7), (ushort)(v.s8), (ushort)(v.s9), (ushort)(v.sA), (ushort)(v.sB), (ushort)(v.sC), (ushort)(v.sD), (ushort)(v.sE), (ushort)(v.sF));
}

INLINE OVERLOADABLE char16 convert_char16(uint16 v) {
  return (char16)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7), (char)(v.s8), (char)(v.s9), (char)(v.sA), (char)(v.sB), (char)(v.sC), (char)(v.sD), (char)(v.sE), (char)(v.sF));
}

INLINE OVERLOADABLE uchar16 convert_uchar16(uint16 v) {
  return (uchar16)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7), (uchar)(v.s8), (uchar)(v.s9), (uchar)(v.sA), (uchar)(v.sB), (uchar)(v.sC), (uchar)(v.sD), (uchar)(v.sE), (uchar)(v.sF));
}

INLINE OVERLOADABLE double16 convert_double16(uint16 v) {
  return (double16)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7), (double)(v.s8), (double)(v.s9), (double)(v.sA), (double)(v.sB), (double)(v.sC), (double)(v.sD), (double)(v.sE), (double)(v.sF));
}

INLINE OVERLOADABLE float16 convert_float16(uint16 v) {
  return (float16)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7), (float)(v.s8), (float)(v.s9), (float)(v.sA), (float)(v.sB), (float)(v.sC), (float)(v.sD), (float)(v.sE), (float)(v.sF));
}

INLINE OVERLOADABLE long16 convert_long16(short16 v) {
  return (long16)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7), (long)(v.s8), (long)(v.s9), (long)(v.sA), (long)(v.sB), (long)(v.sC), (long)(v.sD), (long)(v.sE), (long)(v.sF));
}

INLINE OVERLOADABLE ulong16 convert_ulong16(short16 v) {
  return (ulong16)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7), (ulong)(v.s8), (ulong)(v.s9), (ulong)(v.sA), (ulong)(v.sB), (ulong)(v.sC), (ulong)(v.sD), (ulong)(v.sE), (ulong)(v.sF));
}

INLINE OVERLOADABLE int16 convert_int16(short16 v) {
  return (int16)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7), (int)(v.s8), (int)(v.s9), (int)(v.sA), (int)(v.sB), (int)(v.sC), (int)(v.sD), (int)(v.sE), (int)(v.sF));
}

INLINE OVERLOADABLE uint16 convert_uint16(short16 v) {
  return (uint16)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7), (uint)(v.s8), (uint)(v.s9), (uint)(v.sA), (uint)(v.sB), (uint)(v.sC), (uint)(v.sD), (uint)(v.sE), (uint)(v.sF));
}

INLINE OVERLOADABLE ushort16 convert_ushort16(short16 v) {
  return (ushort16)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7), (ushort)(v.s8), (ushort)(v.s9), (ushort)(v.sA), (ushort)(v.sB), (ushort)(v.sC), (ushort)(v.sD), (ushort)(v.sE), (ushort)(v.sF));
}

INLINE OVERLOADABLE char16 convert_char16(short16 v) {
  return (char16)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7), (char)(v.s8), (char)(v.s9), (char)(v.sA), (char)(v.sB), (char)(v.sC), (char)(v.sD), (char)(v.sE), (char)(v.sF));
}

INLINE OVERLOADABLE uchar16 convert_uchar16(short16 v) {
  return (uchar16)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7), (uchar)(v.s8), (uchar)(v.s9), (uchar)(v.sA), (uchar)(v.sB), (uchar)(v.sC), (uchar)(v.sD), (uchar)(v.sE), (uchar)(v.sF));
}

INLINE OVERLOADABLE double16 convert_double16(short16 v) {
  return (double16)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7), (double)(v.s8), (double)(v.s9), (double)(v.sA), (double)(v.sB), (double)(v.sC), (double)(v.sD), (double)(v.sE), (double)(v.sF));
}

INLINE OVERLOADABLE float16 convert_float16(short16 v) {
  return (float16)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7), (float)(v.s8), (float)(v.s9), (float)(v.sA), (float)(v.sB), (float)(v.sC), (float)(v.sD), (float)(v.sE), (float)(v.sF));
}

INLINE OVERLOADABLE long16 convert_long16(ushort16 v) {
  return (long16)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7), (long)(v.s8), (long)(v.s9), (long)(v.sA), (long)(v.sB), (long)(v.sC), (long)(v.sD), (long)(v.sE), (long)(v.sF));
}

INLINE OVERLOADABLE ulong16 convert_ulong16(ushort16 v) {
  return (ulong16)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7), (ulong)(v.s8), (ulong)(v.s9), (ulong)(v.sA), (ulong)(v.sB), (ulong)(v.sC), (ulong)(v.sD), (ulong)(v.sE), (ulong)(v.sF));
}

INLINE OVERLOADABLE int16 convert_int16(ushort16 v) {
  return (int16)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7), (int)(v.s8), (int)(v.s9), (int)(v.sA), (int)(v.sB), (int)(v.sC), (int)(v.sD), (int)(v.sE), (int)(v.sF));
}

INLINE OVERLOADABLE uint16 convert_uint16(ushort16 v) {
  return (uint16)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7), (uint)(v.s8), (uint)(v.s9), (uint)(v.sA), (uint)(v.sB), (uint)(v.sC), (uint)(v.sD), (uint)(v.sE), (uint)(v.sF));
}

INLINE OVERLOADABLE short16 convert_short16(ushort16 v) {
  return (short16)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7), (short)(v.s8), (short)(v.s9), (short)(v.sA), (short)(v.sB), (short)(v.sC), (short)(v.sD), (short)(v.sE), (short)(v.sF));
}

INLINE OVERLOADABLE char16 convert_char16(ushort16 v) {
  return (char16)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7), (char)(v.s8), (char)(v.s9), (char)(v.sA), (char)(v.sB), (char)(v.sC), (char)(v.sD), (char)(v.sE), (char)(v.sF));
}

INLINE OVERLOADABLE uchar16 convert_uchar16(ushort16 v) {
  return (uchar16)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7), (uchar)(v.s8), (uchar)(v.s9), (uchar)(v.sA), (uchar)(v.sB), (uchar)(v.sC), (uchar)(v.sD), (uchar)(v.sE), (uchar)(v.sF));
}

INLINE OVERLOADABLE double16 convert_double16(ushort16 v) {
  return (double16)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7), (double)(v.s8), (double)(v.s9), (double)(v.sA), (double)(v.sB), (double)(v.sC), (double)(v.sD), (double)(v.sE), (double)(v.sF));
}

INLINE OVERLOADABLE float16 convert_float16(ushort16 v) {
  return (float16)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7), (float)(v.s8), (float)(v.s9), (float)(v.sA), (float)(v.sB), (float)(v.sC), (float)(v.sD), (float)(v.sE), (float)(v.sF));
}

INLINE OVERLOADABLE long16 convert_long16(char16 v) {
  return (long16)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7), (long)(v.s8), (long)(v.s9), (long)(v.sA), (long)(v.sB), (long)(v.sC), (long)(v.sD), (long)(v.sE), (long)(v.sF));
}

INLINE OVERLOADABLE ulong16 convert_ulong16(char16 v) {
  return (ulong16)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7), (ulong)(v.s8), (ulong)(v.s9), (ulong)(v.sA), (ulong)(v.sB), (ulong)(v.sC), (ulong)(v.sD), (ulong)(v.sE), (ulong)(v.sF));
}

INLINE OVERLOADABLE int16 convert_int16(char16 v) {
  return (int16)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7), (int)(v.s8), (int)(v.s9), (int)(v.sA), (int)(v.sB), (int)(v.sC), (int)(v.sD), (int)(v.sE), (int)(v.sF));
}

INLINE OVERLOADABLE uint16 convert_uint16(char16 v) {
  return (uint16)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7), (uint)(v.s8), (uint)(v.s9), (uint)(v.sA), (uint)(v.sB), (uint)(v.sC), (uint)(v.sD), (uint)(v.sE), (uint)(v.sF));
}

INLINE OVERLOADABLE short16 convert_short16(char16 v) {
  return (short16)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7), (short)(v.s8), (short)(v.s9), (short)(v.sA), (short)(v.sB), (short)(v.sC), (short)(v.sD), (short)(v.sE), (short)(v.sF));
}

INLINE OVERLOADABLE ushort16 convert_ushort16(char16 v) {
  return (ushort16)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7), (ushort)(v.s8), (ushort)(v.s9), (ushort)(v.sA), (ushort)(v.sB), (ushort)(v.sC), (ushort)(v.sD), (ushort)(v.sE), (ushort)(v.sF));
}

INLINE OVERLOADABLE uchar16 convert_uchar16(char16 v) {
  return (uchar16)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7), (uchar)(v.s8), (uchar)(v.s9), (uchar)(v.sA), (uchar)(v.sB), (uchar)(v.sC), (uchar)(v.sD), (uchar)(v.sE), (uchar)(v.sF));
}

INLINE OVERLOADABLE double16 convert_double16(char16 v) {
  return (double16)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7), (double)(v.s8), (double)(v.s9), (double)(v.sA), (double)(v.sB), (double)(v.sC), (double)(v.sD), (double)(v.sE), (double)(v.sF));
}

INLINE OVERLOADABLE float16 convert_float16(char16 v) {
  return (float16)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7), (float)(v.s8), (float)(v.s9), (float)(v.sA), (float)(v.sB), (float)(v.sC), (float)(v.sD), (float)(v.sE), (float)(v.sF));
}

INLINE OVERLOADABLE long16 convert_long16(uchar16 v) {
  return (long16)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7), (long)(v.s8), (long)(v.s9), (long)(v.sA), (long)(v.sB), (long)(v.sC), (long)(v.sD), (long)(v.sE), (long)(v.sF));
}

INLINE OVERLOADABLE ulong16 convert_ulong16(uchar16 v) {
  return (ulong16)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7), (ulong)(v.s8), (ulong)(v.s9), (ulong)(v.sA), (ulong)(v.sB), (ulong)(v.sC), (ulong)(v.sD), (ulong)(v.sE), (ulong)(v.sF));
}

INLINE OVERLOADABLE int16 convert_int16(uchar16 v) {
  return (int16)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7), (int)(v.s8), (int)(v.s9), (int)(v.sA), (int)(v.sB), (int)(v.sC), (int)(v.sD), (int)(v.sE), (int)(v.sF));
}

INLINE OVERLOADABLE uint16 convert_uint16(uchar16 v) {
  return (uint16)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7), (uint)(v.s8), (uint)(v.s9), (uint)(v.sA), (uint)(v.sB), (uint)(v.sC), (uint)(v.sD), (uint)(v.sE), (uint)(v.sF));
}

INLINE OVERLOADABLE short16 convert_short16(uchar16 v) {
  return (short16)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7), (short)(v.s8), (short)(v.s9), (short)(v.sA), (short)(v.sB), (short)(v.sC), (short)(v.sD), (short)(v.sE), (short)(v.sF));
}

INLINE OVERLOADABLE ushort16 convert_ushort16(uchar16 v) {
  return (ushort16)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7), (ushort)(v.s8), (ushort)(v.s9), (ushort)(v.sA), (ushort)(v.sB), (ushort)(v.sC), (ushort)(v.sD), (ushort)(v.sE), (ushort)(v.sF));
}

INLINE OVERLOADABLE char16 convert_char16(uchar16 v) {
  return (char16)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7), (char)(v.s8), (char)(v.s9), (char)(v.sA), (char)(v.sB), (char)(v.sC), (char)(v.sD), (char)(v.sE), (char)(v.sF));
}

INLINE OVERLOADABLE double16 convert_double16(uchar16 v) {
  return (double16)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7), (double)(v.s8), (double)(v.s9), (double)(v.sA), (double)(v.sB), (double)(v.sC), (double)(v.sD), (double)(v.sE), (double)(v.sF));
}

INLINE OVERLOADABLE float16 convert_float16(uchar16 v) {
  return (float16)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7), (float)(v.s8), (float)(v.s9), (float)(v.sA), (float)(v.sB), (float)(v.sC), (float)(v.sD), (float)(v.sE), (float)(v.sF));
}

INLINE OVERLOADABLE long16 convert_long16(double16 v) {
  return (long16)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7), (long)(v.s8), (long)(v.s9), (long)(v.sA), (long)(v.sB), (long)(v.sC), (long)(v.sD), (long)(v.sE), (long)(v.sF));
}

INLINE OVERLOADABLE ulong16 convert_ulong16(double16 v) {
  return (ulong16)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7), (ulong)(v.s8), (ulong)(v.s9), (ulong)(v.sA), (ulong)(v.sB), (ulong)(v.sC), (ulong)(v.sD), (ulong)(v.sE), (ulong)(v.sF));
}

INLINE OVERLOADABLE int16 convert_int16(double16 v) {
  return (int16)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7), (int)(v.s8), (int)(v.s9), (int)(v.sA), (int)(v.sB), (int)(v.sC), (int)(v.sD), (int)(v.sE), (int)(v.sF));
}

INLINE OVERLOADABLE uint16 convert_uint16(double16 v) {
  return (uint16)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7), (uint)(v.s8), (uint)(v.s9), (uint)(v.sA), (uint)(v.sB), (uint)(v.sC), (uint)(v.sD), (uint)(v.sE), (uint)(v.sF));
}

INLINE OVERLOADABLE short16 convert_short16(double16 v) {
  return (short16)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7), (short)(v.s8), (short)(v.s9), (short)(v.sA), (short)(v.sB), (short)(v.sC), (short)(v.sD), (short)(v.sE), (short)(v.sF));
}

INLINE OVERLOADABLE ushort16 convert_ushort16(double16 v) {
  return (ushort16)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7), (ushort)(v.s8), (ushort)(v.s9), (ushort)(v.sA), (ushort)(v.sB), (ushort)(v.sC), (ushort)(v.sD), (ushort)(v.sE), (ushort)(v.sF));
}

INLINE OVERLOADABLE char16 convert_char16(double16 v) {
  return (char16)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7), (char)(v.s8), (char)(v.s9), (char)(v.sA), (char)(v.sB), (char)(v.sC), (char)(v.sD), (char)(v.sE), (char)(v.sF));
}

INLINE OVERLOADABLE uchar16 convert_uchar16(double16 v) {
  return (uchar16)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7), (uchar)(v.s8), (uchar)(v.s9), (uchar)(v.sA), (uchar)(v.sB), (uchar)(v.sC), (uchar)(v.sD), (uchar)(v.sE), (uchar)(v.sF));
}

INLINE OVERLOADABLE float16 convert_float16(double16 v) {
  return (float16)((float)(v.s0), (float)(v.s1), (float)(v.s2), (float)(v.s3), (float)(v.s4), (float)(v.s5), (float)(v.s6), (float)(v.s7), (float)(v.s8), (float)(v.s9), (float)(v.sA), (float)(v.sB), (float)(v.sC), (float)(v.sD), (float)(v.sE), (float)(v.sF));
}

INLINE OVERLOADABLE long16 convert_long16(float16 v) {
  return (long16)((long)(v.s0), (long)(v.s1), (long)(v.s2), (long)(v.s3), (long)(v.s4), (long)(v.s5), (long)(v.s6), (long)(v.s7), (long)(v.s8), (long)(v.s9), (long)(v.sA), (long)(v.sB), (long)(v.sC), (long)(v.sD), (long)(v.sE), (long)(v.sF));
}

INLINE OVERLOADABLE ulong16 convert_ulong16(float16 v) {
  return (ulong16)((ulong)(v.s0), (ulong)(v.s1), (ulong)(v.s2), (ulong)(v.s3), (ulong)(v.s4), (ulong)(v.s5), (ulong)(v.s6), (ulong)(v.s7), (ulong)(v.s8), (ulong)(v.s9), (ulong)(v.sA), (ulong)(v.sB), (ulong)(v.sC), (ulong)(v.sD), (ulong)(v.sE), (ulong)(v.sF));
}

INLINE OVERLOADABLE int16 convert_int16(float16 v) {
  return (int16)((int)(v.s0), (int)(v.s1), (int)(v.s2), (int)(v.s3), (int)(v.s4), (int)(v.s5), (int)(v.s6), (int)(v.s7), (int)(v.s8), (int)(v.s9), (int)(v.sA), (int)(v.sB), (int)(v.sC), (int)(v.sD), (int)(v.sE), (int)(v.sF));
}

INLINE OVERLOADABLE uint16 convert_uint16(float16 v) {
  return (uint16)((uint)(v.s0), (uint)(v.s1), (uint)(v.s2), (uint)(v.s3), (uint)(v.s4), (uint)(v.s5), (uint)(v.s6), (uint)(v.s7), (uint)(v.s8), (uint)(v.s9), (uint)(v.sA), (uint)(v.sB), (uint)(v.sC), (uint)(v.sD), (uint)(v.sE), (uint)(v.sF));
}

INLINE OVERLOADABLE short16 convert_short16(float16 v) {
  return (short16)((short)(v.s0), (short)(v.s1), (short)(v.s2), (short)(v.s3), (short)(v.s4), (short)(v.s5), (short)(v.s6), (short)(v.s7), (short)(v.s8), (short)(v.s9), (short)(v.sA), (short)(v.sB), (short)(v.sC), (short)(v.sD), (short)(v.sE), (short)(v.sF));
}

INLINE OVERLOADABLE ushort16 convert_ushort16(float16 v) {
  return (ushort16)((ushort)(v.s0), (ushort)(v.s1), (ushort)(v.s2), (ushort)(v.s3), (ushort)(v.s4), (ushort)(v.s5), (ushort)(v.s6), (ushort)(v.s7), (ushort)(v.s8), (ushort)(v.s9), (ushort)(v.sA), (ushort)(v.sB), (ushort)(v.sC), (ushort)(v.sD), (ushort)(v.sE), (ushort)(v.sF));
}

INLINE OVERLOADABLE char16 convert_char16(float16 v) {
  return (char16)((char)(v.s0), (char)(v.s1), (char)(v.s2), (char)(v.s3), (char)(v.s4), (char)(v.s5), (char)(v.s6), (char)(v.s7), (char)(v.s8), (char)(v.s9), (char)(v.sA), (char)(v.sB), (char)(v.sC), (char)(v.sD), (char)(v.sE), (char)(v.sF));
}

INLINE OVERLOADABLE uchar16 convert_uchar16(float16 v) {
  return (uchar16)((uchar)(v.s0), (uchar)(v.s1), (uchar)(v.s2), (uchar)(v.s3), (uchar)(v.s4), (uchar)(v.s5), (uchar)(v.s6), (uchar)(v.s7), (uchar)(v.s8), (uchar)(v.s9), (uchar)(v.sA), (uchar)(v.sB), (uchar)(v.sC), (uchar)(v.sD), (uchar)(v.sE), (uchar)(v.sF));
}

INLINE OVERLOADABLE double16 convert_double16(float16 v) {
  return (double16)((double)(v.s0), (double)(v.s1), (double)(v.s2), (double)(v.s3), (double)(v.s4), (double)(v.s5), (double)(v.s6), (double)(v.s7), (double)(v.s8), (double)(v.s9), (double)(v.sA), (double)(v.sB), (double)(v.sC), (double)(v.sD), (double)(v.sE), (double)(v.sF));
}

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

#define DEC2(name) INLINE_OVERLOADABLE int2 name(float2 x) { return (int2)(name(x.s0), name(x.s1)); }
#define DEC3(name) INLINE_OVERLOADABLE int3 name(float3 x) { return (int3)(name(x.s0), name(x.s1), name(x.s2)); }
#define DEC4(name) INLINE_OVERLOADABLE int4 name(float4 x) { return (int4)(name(x.s0), name(x.s1), name(x.s2), name(x.s3)); }
#define DEC8(name) INLINE_OVERLOADABLE int8 name(float8 x) { return (int8)(name(x.s0), name(x.s1), name(x.s2), name(x.s3), name(x.s4), name(x.s5), name(x.s6), name(x.s7)); }
#define DEC16(name) INLINE_OVERLOADABLE int16 name(float16 x) { return (int16)(name(x.s0), name(x.s1), name(x.s2), name(x.s3), name(x.s4), name(x.s5), name(x.s6), name(x.s7), name(x.s8), name(x.s9), name(x.sA), name(x.sB), name(x.sC), name(x.sD), name(x.sE), name(x.sF)); }
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

#define DEC2(name) INLINE_OVERLOADABLE int2 name(float2 x, float2 y) { return (int2)(name(x.s0, y.s0), name(x.s1, y.s1)); }
#define DEC3(name) INLINE_OVERLOADABLE int3 name(float3 x, float3 y) { return (int3)(name(x.s0, y.s0), name(x.s1, y.s1), name(x.s2, y.s2)); }
#define DEC4(name) INLINE_OVERLOADABLE int4 name(float4 x, float4 y) { return (int4)(name(x.s0, y.s0), name(x.s1, y.s1), name(x.s2, y.s2), name(x.s3, y.s3)); }
#define DEC8(name) INLINE_OVERLOADABLE int8 name(float8 x, float8 y) { return (int8)(name(x.s0, y.s0), name(x.s1, y.s1), name(x.s2, y.s2), name(x.s3, y.s3), name(x.s4, y.s4), name(x.s5, y.s5), name(x.s6, y.s6), name(x.s7, y.s7)); }
#define DEC16(name) INLINE_OVERLOADABLE int16 name(float16 x, float16 y) { return (int16)(name(x.s0, y.s0), name(x.s1, y.s1), name(x.s2, y.s2), name(x.s3, y.s3), name(x.s4, y.s4), name(x.s5, y.s5), name(x.s6, y.s6), name(x.s7, y.s7), name(x.s8, y.s8), name(x.s9, y.s9), name(x.sA, y.sA), name(x.sB, y.sB), name(x.sC, y.sC), name(x.sD, y.sD), name(x.sE, y.sE), name(x.sF, y.sF)); }
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

#define DEF(type) INLINE_OVERLOADABLE type bitselect(type a, type b, type c) { return (a & ~c) | (b & c); }
DEF(char); DEF(uchar); DEF(short); DEF(ushort); DEF(int); DEF(uint)
#undef DEF
INLINE_OVERLOADABLE float bitselect(float a, float b, float c) {
  return as_float(bitselect(as_int(a), as_int(b), as_int(c)));
}
#define DEC2(type) INLINE_OVERLOADABLE type##2 bitselect(type##2 a, type##2 b, type##2 c) { return (type##2)(bitselect(a.s0, b.s0, c.s0), bitselect(a.s1, b.s1, c.s1)); }
#define DEC3(type) INLINE_OVERLOADABLE type##3 bitselect(type##3 a, type##3 b, type##3 c) { return (type##3)(bitselect(a.s0, b.s0, c.s0), bitselect(a.s1, b.s1, c.s1), bitselect(a.s2, b.s2, c.s2)); }
#define DEC4(type) INLINE_OVERLOADABLE type##4 bitselect(type##4 a, type##4 b, type##4 c) { return (type##4)(bitselect(a.s0, b.s0, c.s0), bitselect(a.s1, b.s1, c.s1), bitselect(a.s2, b.s2, c.s2), bitselect(a.s3, b.s3, c.s3)); }
#define DEC8(type) INLINE_OVERLOADABLE type##8 bitselect(type##8 a, type##8 b, type##8 c) { return (type##8)(bitselect(a.s0, b.s0, c.s0), bitselect(a.s1, b.s1, c.s1), bitselect(a.s2, b.s2, c.s2), bitselect(a.s3, b.s3, c.s3), bitselect(a.s4, b.s4, c.s4), bitselect(a.s5, b.s5, c.s5), bitselect(a.s6, b.s6, c.s6), bitselect(a.s7, b.s7, c.s7)); }
#define DEC16(type) INLINE_OVERLOADABLE type##16 bitselect(type##16 a, type##16 b, type##16 c) { return (type##16)(bitselect(a.s0, b.s0, c.s0), bitselect(a.s1, b.s1, c.s1), bitselect(a.s2, b.s2, c.s2), bitselect(a.s3, b.s3, c.s3), bitselect(a.s4, b.s4, c.s4), bitselect(a.s5, b.s5, c.s5), bitselect(a.s6, b.s6, c.s6), bitselect(a.s7, b.s7, c.s7), bitselect(a.s8, b.s8, c.s8), bitselect(a.s9, b.s9, c.s9), bitselect(a.sa, b.sa, c.sa), bitselect(a.sb, b.sb, c.sb), bitselect(a.sc, b.sc, c.sc), bitselect(a.sd, b.sd, c.sd), bitselect(a.se, b.se, c.se), bitselect(a.sf, b.sf, c.sf)); }
#define DEF(n) DEC##n(char); DEC##n(uchar); DEC##n(short); DEC##n(ushort); DEC##n(int); DEC##n(uint); DEC##n(float)
DEF(2)
DEF(3)
DEF(4)
DEF(8)
DEF(16)
#undef DEF
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

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

#define DEC2(type) INLINE_OVERLOADABLE type##2 clz(type##2 a) { return (type##2)(clz(a.s0), clz(a.s1)); }
#define DEC3(type) INLINE_OVERLOADABLE type##3 clz(type##3 a) { return (type##3)(clz(a.s0), clz(a.s1), clz(a.s2)); }
#define DEC4(type) INLINE_OVERLOADABLE type##4 clz(type##4 a) { return (type##4)(clz(a.s0), clz(a.s1), clz(a.s2), clz(a.s3)); }
#define DEC8(type) INLINE_OVERLOADABLE type##8 clz(type##8 a) { return (type##8)(clz(a.s0), clz(a.s1), clz(a.s2), clz(a.s3), clz(a.s4), clz(a.s5), clz(a.s6), clz(a.s7)); }
#define DEC16(type) INLINE_OVERLOADABLE type##16 clz(type##16 a) { return (type##16)(clz(a.s0), clz(a.s1), clz(a.s2), clz(a.s3), clz(a.s4), clz(a.s5), clz(a.s6), clz(a.s7), clz(a.s8), clz(a.s9), clz(a.sa), clz(a.sb), clz(a.sc), clz(a.sd), clz(a.se), clz(a.sf)); }
#define DEC(n) DEC##n(char); DEC##n(uchar); DEC##n(short); DEC##n(ushort); DEC##n(int); DEC##n(uint) 
DEC(2)
DEC(3)
DEC(4)
DEC(8)
DEC(16)
#undef DEC
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

OVERLOADABLE int __gen_ocl_mul_hi(int x, int y);
OVERLOADABLE uint __gen_ocl_mul_hi(uint x, uint y);
INLINE_OVERLOADABLE char mul_hi(char x, char y) { return (x * y) >> 8; }
INLINE_OVERLOADABLE uchar mul_hi(uchar x, uchar y) { return (x * y) >> 8; }
INLINE_OVERLOADABLE short mul_hi(short x, short y) { return (x * y) >> 16; }
INLINE_OVERLOADABLE ushort mul_hi(ushort x, ushort y) { return (x * y) >> 16; }
INLINE_OVERLOADABLE int mul_hi(int x, int y) { return __gen_ocl_mul_hi(x, y); }
INLINE_OVERLOADABLE uint mul_hi(uint x, uint y) { return __gen_ocl_mul_hi(x, y); }
#define DEC2(type) INLINE_OVERLOADABLE type##2 mul_hi(type##2 a, type##2 b) { return (type##2)(mul_hi(a.s0, b.s0), mul_hi(a.s1, b.s1)); }
#define DEC3(type) INLINE_OVERLOADABLE type##3 mul_hi(type##3 a, type##3 b) { return (type##3)(mul_hi(a.s0, b.s0), mul_hi(a.s1, b.s1), mul_hi(a.s2, b.s2)); }
#define DEC4(type) INLINE_OVERLOADABLE type##4 mul_hi(type##4 a, type##4 b) { return (type##4)(mul_hi(a.s0, b.s0), mul_hi(a.s1, b.s1), mul_hi(a.s2, b.s2), mul_hi(a.s3, b.s3)); }
#define DEC8(type) INLINE_OVERLOADABLE type##8 mul_hi(type##8 a, type##8 b) { return (type##8)(mul_hi(a.s0, b.s0), mul_hi(a.s1, b.s1), mul_hi(a.s2, b.s2), mul_hi(a.s3, b.s3), mul_hi(a.s4, b.s4), mul_hi(a.s5, b.s5), mul_hi(a.s6, b.s6), mul_hi(a.s7, b.s7)); }
#define DEC16(type) INLINE_OVERLOADABLE type##16 mul_hi(type##16 a, type##16 b) { return (type##16)(mul_hi(a.s0, b.s0), mul_hi(a.s1, b.s1), mul_hi(a.s2, b.s2), mul_hi(a.s3, b.s3), mul_hi(a.s4, b.s4), mul_hi(a.s5, b.s5), mul_hi(a.s6, b.s6), mul_hi(a.s7, b.s7), mul_hi(a.s8, b.s8), mul_hi(a.s9, b.s9), mul_hi(a.sa, b.sa), mul_hi(a.sb, b.sb), mul_hi(a.sc, b.sc), mul_hi(a.sd, b.sd), mul_hi(a.se, b.se), mul_hi(a.sf, b.sf)); }
#define DEF(n) DEC##n(char); DEC##n(uchar); DEC##n(short); DEC##n(ushort); DEC##n(int); DEC##n(uint)
DEF(2)
DEF(3)
DEF(4)
DEF(8)
DEF(16)
#undef DEF
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

#define DEF(type) INLINE_OVERLOADABLE type mad_hi(type a, type b, type c) { return mul_hi(a, b) + c; }
DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(int)
DEF(uint)
#undef DEF
#define DEC2(type) INLINE_OVERLOADABLE type##2 mad_hi(type##2 a, type##2 b, type##2 c) { return (type##2)(mad_hi(a.s0, b.s0, c.s0), mad_hi(a.s1, b.s1, c.s1)); }
#define DEC3(type) INLINE_OVERLOADABLE type##3 mad_hi(type##3 a, type##3 b, type##3 c) { return (type##3)(mad_hi(a.s0, b.s0, c.s0), mad_hi(a.s1, b.s1, c.s1), mad_hi(a.s2, b.s2, c.s2)); }
#define DEC4(type) INLINE_OVERLOADABLE type##4 mad_hi(type##4 a, type##4 b, type##4 c) { return (type##4)(mad_hi(a.s0, b.s0, c.s0), mad_hi(a.s1, b.s1, c.s1), mad_hi(a.s2, b.s2, c.s2), mad_hi(a.s3, b.s3, c.s3)); }
#define DEC8(type) INLINE_OVERLOADABLE type##8 mad_hi(type##8 a, type##8 b, type##8 c) { return (type##8)(mad_hi(a.s0, b.s0, c.s0), mad_hi(a.s1, b.s1, c.s1), mad_hi(a.s2, b.s2, c.s2), mad_hi(a.s3, b.s3, c.s3), mad_hi(a.s4, b.s4, c.s4), mad_hi(a.s5, b.s5, c.s5), mad_hi(a.s6, b.s6, c.s6), mad_hi(a.s7, b.s7, c.s7)); }
#define DEC16(type) INLINE_OVERLOADABLE type##16 mad_hi(type##16 a, type##16 b, type##16 c) { return (type##16)(mad_hi(a.s0, b.s0, c.s0), mad_hi(a.s1, b.s1, c.s1), mad_hi(a.s2, b.s2, c.s2), mad_hi(a.s3, b.s3, c.s3), mad_hi(a.s4, b.s4, c.s4), mad_hi(a.s5, b.s5, c.s5), mad_hi(a.s6, b.s6, c.s6), mad_hi(a.s7, b.s7, c.s7), mad_hi(a.s8, b.s8, c.s8), mad_hi(a.s9, b.s9, c.s9), mad_hi(a.sa, b.sa, c.sa), mad_hi(a.sb, b.sb, c.sb), mad_hi(a.sc, b.sc, c.sc), mad_hi(a.sd, b.sd, c.sd), mad_hi(a.se, b.se, c.se), mad_hi(a.sf, b.sf, c.sf)); }
#define DEF(n) DEC##n(char); DEC##n(uchar); DEC##n(short); DEC##n(ushort); DEC##n(int); DEC##n(uint)
DEF(2)
DEF(3)
DEF(4)
DEF(8)
DEF(16)
#undef DEF
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

INLINE_OVERLOADABLE int mul24(int a, int b) { return ((a << 8) >> 8) * ((b << 8) >> 8); }
INLINE_OVERLOADABLE uint mul24(uint a, uint b) { return (a & 0xFFFFFF) * (b & 0xFFFFFF); }
#define DEC2(type) INLINE_OVERLOADABLE type##2 mul24(type##2 a, type##2 b) { return (type##2)(mul24(a.s0, b.s0), mul24(a.s1, b.s1)); }
#define DEC3(type) INLINE_OVERLOADABLE type##3 mul24(type##3 a, type##3 b) { return (type##3)(mul24(a.s0, b.s0), mul24(a.s1, b.s1), mul24(a.s2, b.s2)); }
#define DEC4(type) INLINE_OVERLOADABLE type##4 mul24(type##4 a, type##4 b) { return (type##4)(mul24(a.s0, b.s0), mul24(a.s1, b.s1), mul24(a.s2, b.s2), mul24(a.s3, b.s3)); }
#define DEC8(type) INLINE_OVERLOADABLE type##8 mul24(type##8 a, type##8 b) { return (type##8)(mul24(a.s0, b.s0), mul24(a.s1, b.s1), mul24(a.s2, b.s2), mul24(a.s3, b.s3), mul24(a.s4, b.s4), mul24(a.s5, b.s5), mul24(a.s6, b.s6), mul24(a.s7, b.s7)); }
#define DEC16(type) INLINE_OVERLOADABLE type##16 mul24(type##16 a, type##16 b) { return (type##16)(mul24(a.s0, b.s0), mul24(a.s1, b.s1), mul24(a.s2, b.s2), mul24(a.s3, b.s3), mul24(a.s4, b.s4), mul24(a.s5, b.s5), mul24(a.s6, b.s6), mul24(a.s7, b.s7), mul24(a.s8, b.s8), mul24(a.s9, b.s9), mul24(a.sa, b.sa), mul24(a.sb, b.sb), mul24(a.sc, b.sc), mul24(a.sd, b.sd), mul24(a.se, b.se), mul24(a.sf, b.sf)); }
#define DEF(n) DEC##n(int); DEC##n(uint)
DEF(2)
DEF(3)
DEF(4)
DEF(8)
DEF(16)
#undef DEF
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

INLINE_OVERLOADABLE int mad24(int a, int b, int c) { return mul24(a, b) + c; }
INLINE_OVERLOADABLE uint mad24(uint a, uint b, uint c) { return mul24(a, b) + c; }
#define DEC2(type) INLINE_OVERLOADABLE type##2 mad24(type##2 a, type##2 b, type##2 c) { return (type##2)(mad24(a.s0, b.s0, c.s0), mad24(a.s1, b.s1, c.s1)); }
#define DEC3(type) INLINE_OVERLOADABLE type##3 mad24(type##3 a, type##3 b, type##3 c) { return (type##3)(mad24(a.s0, b.s0, c.s0), mad24(a.s1, b.s1, c.s1), mad24(a.s2, b.s2, c.s2)); }
#define DEC4(type) INLINE_OVERLOADABLE type##4 mad24(type##4 a, type##4 b, type##4 c) { return (type##4)(mad24(a.s0, b.s0, c.s0), mad24(a.s1, b.s1, c.s1), mad24(a.s2, b.s2, c.s2), mad24(a.s3, b.s3, c.s3)); }
#define DEC8(type) INLINE_OVERLOADABLE type##8 mad24(type##8 a, type##8 b, type##8 c) { return (type##8)(mad24(a.s0, b.s0, c.s0), mad24(a.s1, b.s1, c.s1), mad24(a.s2, b.s2, c.s2), mad24(a.s3, b.s3, c.s3), mad24(a.s4, b.s4, c.s4), mad24(a.s5, b.s5, c.s5), mad24(a.s6, b.s6, c.s6), mad24(a.s7, b.s7, c.s7)); }
#define DEC16(type) INLINE_OVERLOADABLE type##16 mad24(type##16 a, type##16 b, type##16 c) { return (type##16)(mad24(a.s0, b.s0, c.s0), mad24(a.s1, b.s1, c.s1), mad24(a.s2, b.s2, c.s2), mad24(a.s3, b.s3, c.s3), mad24(a.s4, b.s4, c.s4), mad24(a.s5, b.s5, c.s5), mad24(a.s6, b.s6, c.s6), mad24(a.s7, b.s7, c.s7), mad24(a.s8, b.s8, c.s8), mad24(a.s9, b.s9, c.s9), mad24(a.sa, b.sa, c.sa), mad24(a.sb, b.sb, c.sb), mad24(a.sc, b.sc, c.sc), mad24(a.sd, b.sd, c.sd), mad24(a.se, b.se, c.se), mad24(a.sf, b.sf, c.sf)); }
#define DEF(n) DEC##n(int); DEC##n(uint)
DEF(2)
DEF(3)
DEF(4)
DEF(8)
DEF(16)
#undef DEF
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

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
#define DEC2(type) INLINE_OVERLOADABLE type##2 rotate(type##2 a, type##2 b) { return (type##2)(rotate(a.s0, b.s0), rotate(a.s1, b.s1)); }
#define DEC3(type) INLINE_OVERLOADABLE type##3 rotate(type##3 a, type##3 b) { return (type##3)(rotate(a.s0, b.s0), rotate(a.s1, b.s1), rotate(a.s2, b.s2)); }
#define DEC4(type) INLINE_OVERLOADABLE type##4 rotate(type##4 a, type##4 b) { return (type##4)(rotate(a.s0, b.s0), rotate(a.s1, b.s1), rotate(a.s2, b.s2), rotate(a.s3, b.s3)); }
#define DEC8(type) INLINE_OVERLOADABLE type##8 rotate(type##8 a, type##8 b) { return (type##8)(rotate(a.s0, b.s0), rotate(a.s1, b.s1), rotate(a.s2, b.s2), rotate(a.s3, b.s3), rotate(a.s4, b.s4), rotate(a.s5, b.s5), rotate(a.s6, b.s6), rotate(a.s7, b.s7)); }
#define DEC16(type) INLINE_OVERLOADABLE type##16 rotate(type##16 a, type##16 b) { return (type##16)(rotate(a.s0, b.s0), rotate(a.s1, b.s1), rotate(a.s2, b.s2), rotate(a.s3, b.s3), rotate(a.s4, b.s4), rotate(a.s5, b.s5), rotate(a.s6, b.s6), rotate(a.s7, b.s7), rotate(a.s8, b.s8), rotate(a.s9, b.s9), rotate(a.sa, b.sa), rotate(a.sb, b.sb), rotate(a.sc, b.sc), rotate(a.sd, b.sd), rotate(a.se, b.se), rotate(a.sf, b.sf)); }
#define DEF(n) DEC##n(char); DEC##n(uchar); DEC##n(short); DEC##n(ushort); DEC##n(int); DEC##n(uint)
DEF(2)
DEF(3)
DEF(4)
DEF(8)
DEF(16)
#undef DEF
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

OVERLOADABLE short __gen_ocl_upsample(short hi, short lo);
OVERLOADABLE int __gen_ocl_upsample(int hi, int lo);
INLINE_OVERLOADABLE short upsample(char hi, uchar lo) { return __gen_ocl_upsample((short)hi, (short)lo); }
INLINE_OVERLOADABLE ushort upsample(uchar hi, uchar lo) { return __gen_ocl_upsample((short)hi, (short)lo); }
INLINE_OVERLOADABLE int upsample(short hi, ushort lo) { return __gen_ocl_upsample((int)hi, (int)lo); }
INLINE_OVERLOADABLE uint upsample(ushort hi, ushort lo) { return __gen_ocl_upsample((int)hi, (int)lo); }
#define DEC2(type, type2) INLINE_OVERLOADABLE type2##2 upsample(type##2 a, type##2 b) { return (type2##2)(upsample(a.s0, b.s0), upsample(a.s1, b.s1)); }
#define DEC3(type, type2) INLINE_OVERLOADABLE type2##3 upsample(type##3 a, type##3 b) { return (type2##3)(upsample(a.s0, b.s0), upsample(a.s1, b.s1), upsample(a.s2, b.s2)); }
#define DEC4(type, type2) INLINE_OVERLOADABLE type2##4 upsample(type##4 a, type##4 b) { return (type2##4)(upsample(a.s0, b.s0), upsample(a.s1, b.s1), upsample(a.s2, b.s2), upsample(a.s3, b.s3)); }
#define DEC8(type, type2) INLINE_OVERLOADABLE type2##8 upsample(type##8 a, type##8 b) { return (type2##8)(upsample(a.s0, b.s0), upsample(a.s1, b.s1), upsample(a.s2, b.s2), upsample(a.s3, b.s3), upsample(a.s4, b.s4), upsample(a.s5, b.s5), upsample(a.s6, b.s6), upsample(a.s7, b.s7)); }
#define DEC16(type, type2) INLINE_OVERLOADABLE type2##16 upsample(type##16 a, type##16 b) { return (type2##16)(upsample(a.s0, b.s0), upsample(a.s1, b.s1), upsample(a.s2, b.s2), upsample(a.s3, b.s3), upsample(a.s4, b.s4), upsample(a.s5, b.s5), upsample(a.s6, b.s6), upsample(a.s7, b.s7), upsample(a.s8, b.s8), upsample(a.s9, b.s9), upsample(a.sa, b.sa), upsample(a.sb, b.sb), upsample(a.sc, b.sc), upsample(a.sd, b.sd), upsample(a.se, b.se), upsample(a.sf, b.sf)); }
#define DEF(n) DEC##n(uchar, ushort); DEC##n(ushort, uint)
DEF(2)
DEF(3)
DEF(4)
DEF(8)
DEF(16)
#undef DEF
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16
#define DEC2(type, type2) INLINE_OVERLOADABLE type2##2 upsample(type##2 a, u##type##2 b) { return (type2##2)(upsample(a.s0, b.s0), upsample(a.s1, b.s1)); }
#define DEC3(type, type2) INLINE_OVERLOADABLE type2##3 upsample(type##3 a, u##type##3 b) { return (type2##3)(upsample(a.s0, b.s0), upsample(a.s1, b.s1), upsample(a.s2, b.s2)); }
#define DEC4(type, type2) INLINE_OVERLOADABLE type2##4 upsample(type##4 a, u##type##4 b) { return (type2##4)(upsample(a.s0, b.s0), upsample(a.s1, b.s1), upsample(a.s2, b.s2), upsample(a.s3, b.s3)); }
#define DEC8(type, type2) INLINE_OVERLOADABLE type2##8 upsample(type##8 a, u##type##8 b) { return (type2##8)(upsample(a.s0, b.s0), upsample(a.s1, b.s1), upsample(a.s2, b.s2), upsample(a.s3, b.s3), upsample(a.s4, b.s4), upsample(a.s5, b.s5), upsample(a.s6, b.s6), upsample(a.s7, b.s7)); }
#define DEC16(type, type2) INLINE_OVERLOADABLE type2##16 upsample(type##16 a, u##type##16 b) { return (type2##16)(upsample(a.s0, b.s0), upsample(a.s1, b.s1), upsample(a.s2, b.s2), upsample(a.s3, b.s3), upsample(a.s4, b.s4), upsample(a.s5, b.s5), upsample(a.s6, b.s6), upsample(a.s7, b.s7), upsample(a.s8, b.s8), upsample(a.s9, b.s9), upsample(a.sa, b.sa), upsample(a.sb, b.sb), upsample(a.sc, b.sc), upsample(a.sd, b.sd), upsample(a.se, b.se), upsample(a.sf, b.sf)); }
#define DEF(n) DEC##n(char, short); DEC##n(short, int)
DEF(2)
DEF(3)
DEF(4)
DEF(8)
DEF(16)
#undef DEF
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

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
#define DEC2(func, type) INLINE_OVERLOADABLE type##2 func(type##2 a, type##2 b) { return (type##2)(func(a.s0, b.s0), func(a.s1, b.s1)); }
#define DEC3(func, type) INLINE_OVERLOADABLE type##3 func(type##3 a, type##3 b) { return (type##3)(func(a.s0, b.s0), func(a.s1, b.s1), func(a.s2, b.s2)); }
#define DEC4(func, type) INLINE_OVERLOADABLE type##4 func(type##4 a, type##4 b) { return (type##4)(func(a.s0, b.s0), func(a.s1, b.s1), func(a.s2, b.s2), func(a.s3, b.s3)); }
#define DEC8(func, type) INLINE_OVERLOADABLE type##8 func(type##8 a, type##8 b) { return (type##8)(func(a.s0, b.s0), func(a.s1, b.s1), func(a.s2, b.s2), func(a.s3, b.s3), func(a.s4, b.s4), func(a.s5, b.s5), func(a.s6, b.s6), func(a.s7, b.s7)); }
#define DEC16(func, type) INLINE_OVERLOADABLE type##16 func(type##16 a, type##16 b) { return (type##16)(func(a.s0, b.s0), func(a.s1, b.s1), func(a.s2, b.s2), func(a.s3, b.s3), func(a.s4, b.s4), func(a.s5, b.s5), func(a.s6, b.s6), func(a.s7, b.s7), func(a.s8, b.s8), func(a.s9, b.s9), func(a.sa, b.sa), func(a.sb, b.sb), func(a.sc, b.sc), func(a.sd, b.sd), func(a.se, b.se), func(a.sf, b.sf)); }
#define DEF(func, n) DEC##n(func, char); DEC##n(func, uchar); DEC##n(func, short); DEC##n(func, ushort); DEC##n(func, int); DEC##n(func, uint)
DEF(hadd, 2)
DEF(hadd, 3)
DEF(hadd, 4)
DEF(hadd, 8)
DEF(hadd, 16)
DEF(rhadd, 2)
DEF(rhadd, 3)
DEF(rhadd, 4)
DEF(rhadd, 8)
DEF(rhadd, 16)
#undef DEF
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

int __gen_ocl_abs(int x);
#define ABS_I(I, CVT)  (CVT)__gen_ocl_abs(x.s##I)
#define ABS_VEC1(CVT)  (CVT)__gen_ocl_abs(x)
#define ABS_VEC2(CVT)  ABS_I(0, CVT), ABS_I(1, CVT)
#define ABS_VEC3(CVT)  ABS_I(0, CVT), ABS_I(1, CVT), ABS_I(2, CVT)
#define ABS_VEC4(CVT)  ABS_VEC2(CVT), ABS_I(2, CVT), ABS_I(3, CVT)
#define ABS_VEC8(CVT)  ABS_VEC4(CVT), ABS_I(4, CVT), ABS_I(5, CVT),\
	               ABS_I(6, CVT), ABS_I(7, CVT)
#define ABS_VEC16(CVT) ABS_VEC8(CVT), ABS_I(8, CVT), ABS_I(9, CVT), \
	               ABS_I(A, CVT), ABS_I(B, CVT), ABS_I(C, CVT), \
	               ABS_I(D, CVT), ABS_I(E, CVT), ABS_I(F, CVT)

#define DEC_1(TYPE) INLINE_OVERLOADABLE u##TYPE abs(TYPE x) { return ABS_VEC1(u##TYPE); }
#define DEC_N(TYPE, N) INLINE_OVERLOADABLE u##TYPE##N abs(TYPE##N x) { return (u##TYPE##N)(ABS_VEC##N(u##TYPE)); };
#define DEC(TYPE) DEC_1(TYPE) DEC_N(TYPE, 2) DEC_N(TYPE, 3) DEC_N(TYPE, 4) DEC_N(TYPE, 8) DEC_N(TYPE, 16)

DEC(int)
DEC(short)
DEC(char)
#undef DEC_1
#undef DEC_N
/* For unsigned types, do nothing. */
#define DEC_1(TYPE) INLINE_OVERLOADABLE TYPE abs(TYPE x) { return x; }
#define DEC_N(TYPE, N) INLINE_OVERLOADABLE TYPE##N abs(TYPE##N x) { return x; }
DEC(uint)
DEC(ushort)
DEC(uchar)
#undef DEC
#undef DEC_1
#undef DEC_N
#undef ABS_I
#undef ABS_VEC1
#undef ABS_VEC2
#undef ABS_VEC3
#undef ABS_VEC4
#undef ABS_VEC8
#undef ABS_VEC16


/* Char and short type abs diff */
/* promote char and short to int and will be no module overflow */
#define ABS_DIFF(CVT) (CVT)(abs((int)x - (int)y))
#define ABS_DIFF_I(CVT, I)  (CVT)(abs((int)x.s##I - (int)y.s##I))

#define ABS_DIFF_VEC1(CVT)  ABS_DIFF(CVT)
#define ABS_DIFF_VEC2(CVT)  ABS_DIFF_I(CVT, 0), ABS_DIFF_I(CVT, 1)
#define ABS_DIFF_VEC3(CVT)  ABS_DIFF_I(CVT, 0), ABS_DIFF_I(CVT, 1), ABS_DIFF_I(CVT, 2)
#define ABS_DIFF_VEC4(CVT)  ABS_DIFF_VEC2(CVT), ABS_DIFF_I(CVT, 2), ABS_DIFF_I(CVT, 3)
#define ABS_DIFF_VEC8(CVT)  ABS_DIFF_VEC4(CVT), ABS_DIFF_I(CVT, 4), ABS_DIFF_I(CVT, 5), \
                            ABS_DIFF_I(CVT, 6), ABS_DIFF_I(CVT, 7)
#define ABS_DIFF_VEC16(CVT)  ABS_DIFF_VEC8(CVT), ABS_DIFF_I(CVT, 8), ABS_DIFF_I(CVT, 9), \
                            ABS_DIFF_I(CVT, A), ABS_DIFF_I(CVT, B), \
                            ABS_DIFF_I(CVT, C), ABS_DIFF_I(CVT, D), \
                            ABS_DIFF_I(CVT, E), ABS_DIFF_I(CVT, F)

#define DEC_1(TYPE, UTYPE) INLINE_OVERLOADABLE UTYPE abs_diff(TYPE x, TYPE y) \
                           { return ABS_DIFF_VEC1(UTYPE); }
#define DEC_N(TYPE, UTYPE, N) INLINE_OVERLOADABLE UTYPE##N abs_diff(TYPE##N x, TYPE##N y) \
                              { return (UTYPE##N)(ABS_DIFF_VEC##N(UTYPE)); };
#define DEC(TYPE, UTYPE)  DEC_1(TYPE, UTYPE) DEC_N(TYPE, UTYPE, 2)  DEC_N(TYPE, UTYPE, 3 ) \
                          DEC_N(TYPE, UTYPE, 4) DEC_N(TYPE, UTYPE, 8) DEC_N(TYPE, UTYPE, 16)
DEC(char, uchar)
DEC(uchar, uchar)
DEC(short, ushort)
DEC(ushort, ushort)

#undef DEC
#undef DEC_1
#undef DEC_N
#undef ABS_DIFF
#undef ABS_DIFF_I
#undef ABS_DIFF_VEC1
#undef ABS_DIFF_VEC2
#undef ABS_DIFF_VEC3
#undef ABS_DIFF_VEC4
#undef ABS_DIFF_VEC8
#undef ABS_DIFF_VEC16

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

#define ABS_DIFF_I(I)  abs_diff(x.s##I, y.s##I)

#define ABS_DIFF_VEC2  ABS_DIFF_I(0), ABS_DIFF_I(1)
#define ABS_DIFF_VEC3  ABS_DIFF_I(0), ABS_DIFF_I(1), ABS_DIFF_I(2)
#define ABS_DIFF_VEC4  ABS_DIFF_VEC2, ABS_DIFF_I(2), ABS_DIFF_I(3)
#define ABS_DIFF_VEC8  ABS_DIFF_VEC4, ABS_DIFF_I(4), ABS_DIFF_I(5), \
                       ABS_DIFF_I(6), ABS_DIFF_I(7)
#define ABS_DIFF_VEC16  ABS_DIFF_VEC8, ABS_DIFF_I(8), ABS_DIFF_I(9), \
                            ABS_DIFF_I(A), ABS_DIFF_I(B), \
                            ABS_DIFF_I(C), ABS_DIFF_I(D), \
                            ABS_DIFF_I(E), ABS_DIFF_I(F)

#define DEC_N(TYPE, N) INLINE_OVERLOADABLE uint##N abs_diff(TYPE##N x, TYPE##N y) \
				      { return (uint##N)(ABS_DIFF_VEC##N); };
#define DEC(TYPE)   DEC_N(TYPE, 2)  DEC_N(TYPE, 3 ) \
                           DEC_N(TYPE, 4) DEC_N(TYPE, 8) DEC_N(TYPE, 16)
DEC(int)
DEC(uint)

#undef DEC
#undef DEC_1
#undef DEC_N
#undef ABS_DIFF
#undef ABS_DIFF_I
#undef ABS_DIFF_VEC1
#undef ABS_DIFF_VEC2
#undef ABS_DIFF_VEC3
#undef ABS_DIFF_VEC4
#undef ABS_DIFF_VEC8
#undef ABS_DIFF_VEC16

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
  return __gen_ocl_sin(x * M_PI_F);
}
INLINE_OVERLOADABLE float native_sqrt(float x) { return __gen_ocl_sqrt(x); }
INLINE_OVERLOADABLE float native_rsqrt(float x) { return __gen_ocl_rsqrt(x); }
INLINE_OVERLOADABLE float native_log2(float x) { return __gen_ocl_log(x); }
INLINE_OVERLOADABLE float native_log(float x) {
  return native_log2(x) * 0.6931472002f;
}
INLINE_OVERLOADABLE float native_log10(float x) {
  return native_log2(x) * 0.3010299956f;
}
INLINE_OVERLOADABLE float log1p(float x) { return native_log(x + 1); }
INLINE_OVERLOADABLE float logb(float x) { return __gen_ocl_rndd(native_log2(x)); }
INLINE_OVERLOADABLE int ilogb(float x) { return __gen_ocl_rndd(native_log2(x)); }
INLINE_OVERLOADABLE int2 ilogb(float2 x) {
  return (int2)(ilogb(x.s0), ilogb(x.s1));
}
INLINE_OVERLOADABLE int4 ilogb(float4 x) {
  return (int4)(ilogb(x.s01), ilogb(x.s23));
}
INLINE_OVERLOADABLE int8 ilogb(float8 x) {
  return (int8)(ilogb(x.s0123), ilogb(x.s4567));
}
INLINE_OVERLOADABLE int16 ilogb(float16 x) {
  return (int16)(ilogb(x.s01234567), ilogb(x.s89abcdef));
}
INLINE_OVERLOADABLE float nan(uint code) {
  return NAN;
}
INLINE_OVERLOADABLE float2 nan(uint2 code) {
  return (float2)(nan(code.s0), nan(code.s1));
}
INLINE_OVERLOADABLE float4 nan(uint4 code) {
  return (float4)(nan(code.s01), nan(code.s23));
}
INLINE_OVERLOADABLE float8 nan(uint8 code) {
  return (float8)(nan(code.s0123), nan(code.s4567));
}
INLINE_OVERLOADABLE float16 nan(uint16 code) {
  return (float16)(nan(code.s01234567), nan(code.s89abcdef));
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
INLINE_OVERLOADABLE float __gen_ocl_internal_sincos(float x, float *cosval) {
  *cosval = native_cos(x);
  return native_sin(x);
}
INLINE_OVERLOADABLE float2 __gen_ocl_internal_sincos(float2 x, float2 *cosval) {
  return (float2)(__gen_ocl_internal_sincos(x.s0, (float *)cosval),
                  __gen_ocl_internal_sincos(x.s1, 1 + (float *)cosval));
}
INLINE_OVERLOADABLE float4 __gen_ocl_internal_sincos(float4 x, float4 *cosval) {
  return (float4)(__gen_ocl_internal_sincos(x.s0, (float *)cosval),
                  __gen_ocl_internal_sincos(x.s1, 1 + (float *)cosval),
                  __gen_ocl_internal_sincos(x.s2, 2 + (float *)cosval),
                  __gen_ocl_internal_sincos(x.s3, 3 + (float *)cosval));
}
INLINE_OVERLOADABLE float8 __gen_ocl_internal_sincos(float8 x, float8 *cosval) {
  return (float8)(__gen_ocl_internal_sincos(x.s0, (float *)cosval),
                  __gen_ocl_internal_sincos(x.s1, 1 + (float *)cosval),
                  __gen_ocl_internal_sincos(x.s2, 2 + (float *)cosval),
                  __gen_ocl_internal_sincos(x.s3, 3 + (float *)cosval),
                  __gen_ocl_internal_sincos(x.s4, 4 + (float *)cosval),
                  __gen_ocl_internal_sincos(x.s5, 5 + (float *)cosval),
                  __gen_ocl_internal_sincos(x.s6, 6 + (float *)cosval),
                  __gen_ocl_internal_sincos(x.s7, 7 + (float *)cosval));
}
INLINE_OVERLOADABLE float16 __gen_ocl_internal_sincos(float16 x, float16 *cosval) {
  return (float16)(__gen_ocl_internal_sincos(x.s0, (float *)cosval),
                   __gen_ocl_internal_sincos(x.s1, 1 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.s2, 2 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.s3, 3 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.s4, 4 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.s5, 5 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.s6, 6 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.s7, 7 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.s8, 8 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.s9, 9 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.sa, 10 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.sb, 11 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.sc, 12 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.sd, 13 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.se, 14 + (float *)cosval),
                   __gen_ocl_internal_sincos(x.sf, 15 + (float *)cosval));
}
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
INLINE_OVERLOADABLE float __gen_ocl_internal_asin(float x) {
  return x + __gen_ocl_pow(x, 3) / 6 + __gen_ocl_pow(x, 5) * 3 / 40 + __gen_ocl_pow(x, 7) * 5 / 112;
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
#define sincos __gen_ocl_internal_sincos
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
#define STEP(I)  x.s##I < edge.s##I ? 0.0 : 1.0
INLINE_OVERLOADABLE float2 step(float2 edge, float2 x) {
  return (float2)(STEP(0), STEP(1));
}
INLINE_OVERLOADABLE float3 step(float3 edge, float3 x) {
  return (float3)(STEP(0), STEP(1), STEP(2));
}
INLINE_OVERLOADABLE float4 step(float4 edge, float4 x) {
  return (float4)(STEP(0), STEP(1), STEP(2), STEP(3));
}
INLINE_OVERLOADABLE float8 step(float8 edge, float8 x) {
  return (float8)(STEP(0), STEP(1), STEP(2), STEP(3),
                  STEP(4), STEP(5), STEP(6), STEP(7));
}
INLINE_OVERLOADABLE float16 step(float16 edge, float16 x) {
  return (float16)(STEP(0), STEP(1), STEP(2), STEP(3),
                   STEP(4), STEP(5), STEP(6), STEP(7),
                   STEP(8), STEP(9), STEP(A), STEP(B),
                   STEP(C), STEP(D), STEP(E), STEP(F));
}
#undef STEP
#define STEP(I)  x.s##I < edge ? 0.0 : 1.0
INLINE_OVERLOADABLE float2 step(float edge, float2 x) {
  return (float2)(STEP(0), STEP(1));
}
INLINE_OVERLOADABLE float3 step(float edge, float3 x) {
  return (float3)(STEP(0), STEP(1), STEP(2));
}
INLINE_OVERLOADABLE float4 step(float edge, float4 x) {
  return (float4)(STEP(0), STEP(1), STEP(2), STEP(3));
}
INLINE_OVERLOADABLE float8 step(float edge, float8 x) {
  return (float8)(STEP(0), STEP(1), STEP(2), STEP(3),
                  STEP(4), STEP(5), STEP(6), STEP(7));
}
INLINE_OVERLOADABLE float16 step(float edge, float16 x) {
  return (float16)(STEP(0), STEP(1), STEP(2), STEP(3),
                   STEP(4), STEP(5), STEP(6), STEP(7),
                   STEP(8), STEP(9), STEP(A), STEP(B),
                   STEP(C), STEP(D), STEP(E), STEP(F));
}
#undef STEP

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
#undef DECL_MIN_MAX_CLAMP

INLINE_OVERLOADABLE float degrees(float radians) { return (180 / M_PI_F) * radians; }
INLINE_OVERLOADABLE float2 degrees(float2 r) { return (float2)(degrees(r.s0), degrees(r.s1)); }
INLINE_OVERLOADABLE float3 degrees(float3 r) { return (float3)(degrees(r.s0), degrees(r.s1), degrees(r.s2)); }
INLINE_OVERLOADABLE float4 degrees(float4 r) { return (float4)(degrees(r.s0), degrees(r.s1), degrees(r.s2), degrees(r.s3)); }
INLINE_OVERLOADABLE float8 degrees(float8 r) { return (float8)(degrees(r.s0), degrees(r.s1), degrees(r.s2), degrees(r.s3), degrees(r.s4), degrees(r.s5), degrees(r.s6), degrees(r.s7)); }
INLINE_OVERLOADABLE float16 degrees(float16 r) { return (float16)(degrees(r.s0), degrees(r.s1), degrees(r.s2), degrees(r.s3), degrees(r.s4), degrees(r.s5), degrees(r.s6), degrees(r.s7), degrees(r.s8), degrees(r.s9), degrees(r.sa), degrees(r.sb), degrees(r.sc), degrees(r.sd), degrees(r.se), degrees(r.sf)); }
INLINE_OVERLOADABLE float radians(float degrees) { return (M_PI_F / 180) * degrees; }
INLINE_OVERLOADABLE float2 radians(float2 r) { return (float2)(radians(r.s0), radians(r.s1)); }
INLINE_OVERLOADABLE float3 radians(float3 r) { return (float3)(radians(r.s0), radians(r.s1), radians(r.s2)); }
INLINE_OVERLOADABLE float4 radians(float4 r) { return (float4)(radians(r.s0), radians(r.s1), radians(r.s2), radians(r.s3)); }
INLINE_OVERLOADABLE float8 radians(float8 r) { return (float8)(radians(r.s0), radians(r.s1), radians(r.s2), radians(r.s3), radians(r.s4), radians(r.s5), radians(r.s6), radians(r.s7)); }
INLINE_OVERLOADABLE float16 radians(float16 r) { return (float16)(radians(r.s0), radians(r.s1), radians(r.s2), radians(r.s3), radians(r.s4), radians(r.s5), radians(r.s6), radians(r.s7), radians(r.s8), radians(r.s9), radians(r.sa), radians(r.sb), radians(r.sc), radians(r.sd), radians(r.se), radians(r.sf)); }

INLINE_OVERLOADABLE float smoothstep(float e0, float e1, float x) {
  x = clamp((x - e0) / (e1 - e0), 0.f, 1.f);
  return x * x * (3 - 2 * x);
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
INLINE_OVERLOADABLE float fract(float x, float *p) {
  *p = __gen_ocl_internal_floor(x);
  return __gen_ocl_internal_fmin(x - *p, 0x1.FFFFFep-1F);
}
INLINE_OVERLOADABLE float2 fract(float2 x, float2 *p) {
  return (float2)(fract(x.s0, (float *)p),
                  fract(x.s1, 1 + (float *)p));
}
INLINE_OVERLOADABLE float4 fract(float4 x, float4 *p) {
  return (float4)(fract(x.s0, (float *)p),
                  fract(x.s1, 1 + (float *)p),
                  fract(x.s2, 2 + (float *)p),
                  fract(x.s3, 3 + (float *)p));
}
INLINE_OVERLOADABLE float8 fract(float8 x, float8 *p) {
  return (float8)(fract(x.s0, (float *)p),
                  fract(x.s1, 1 + (float *)p),
                  fract(x.s2, 2 + (float *)p),
                  fract(x.s3, 3 + (float *)p),
                  fract(x.s4, 4 + (float *)p),
                  fract(x.s5, 5 + (float *)p),
                  fract(x.s6, 6 + (float *)p),
                  fract(x.s7, 7 + (float *)p));
}
INLINE_OVERLOADABLE float16 fract(float16 x, float16 *p) {
  return (float16)(fract(x.s0, (float *)p),
                   fract(x.s1, 1 + (float *)p),
                   fract(x.s2, 2 + (float *)p),
                   fract(x.s3, 3 + (float *)p),
                   fract(x.s4, 4 + (float *)p),
                   fract(x.s5, 5 + (float *)p),
                   fract(x.s6, 6 + (float *)p),
                   fract(x.s7, 7 + (float *)p),
                   fract(x.s8, 8 + (float *)p),
                   fract(x.s9, 9 + (float *)p),
                   fract(x.sa, 10 + (float *)p),
                   fract(x.sb, 11 + (float *)p),
                   fract(x.sc, 12 + (float *)p),
                   fract(x.sd, 13 + (float *)p),
                   fract(x.se, 14 + (float *)p),
                   fract(x.sf, 15 + (float *)p));
}
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

#undef DECL_UNTYPED_RW_ALL
#undef DECL_UNTYPED_RW_ALL_SPACE
#undef DECL_UNTYPED_RW_SPACE_N

/////////////////////////////////////////////////////////////////////////////
// Declare functions for vector types which are derived from scalar ones
/////////////////////////////////////////////////////////////////////////////
#define DECL_VECTOR_1OP(NAME, TYPE) \
  INLINE_OVERLOADABLE TYPE##2 NAME(TYPE##2 v) { \
    return (TYPE##2)(NAME(v.x), NAME(v.y)); \
  }\
  INLINE_OVERLOADABLE TYPE##3 NAME(TYPE##3 v) { \
    return (TYPE##3)(NAME(v.x), NAME(v.y), NAME(v.z)); \
  }\
  INLINE_OVERLOADABLE TYPE##4 NAME(TYPE##4 v) { \
    return (TYPE##4)(NAME(v.x), NAME(v.y), NAME(v.z), NAME(v.w)); \
  }\
  INLINE_OVERLOADABLE TYPE##8 NAME(TYPE##8 v) { \
    TYPE##8 dst;\
    dst.s0123 = NAME(v.s0123);\
    dst.s4567 = NAME(v.s4567);\
    return dst;\
  }\
  INLINE_OVERLOADABLE TYPE##16 NAME(TYPE##16 v) { \
    TYPE##16 dst;\
    dst.s01234567 = NAME(v.s01234567);\
    dst.s89abcdef = NAME(v.s89abcdef);\
    return dst;\
  }
DECL_VECTOR_1OP(native_cos, float);
DECL_VECTOR_1OP(__gen_ocl_internal_cospi, float);
DECL_VECTOR_1OP(__gen_ocl_internal_cosh, float);
DECL_VECTOR_1OP(__gen_ocl_internal_acos, float);
DECL_VECTOR_1OP(__gen_ocl_internal_acospi, float);
DECL_VECTOR_1OP(__gen_ocl_internal_acosh, float);
DECL_VECTOR_1OP(native_sin, float);
DECL_VECTOR_1OP(__gen_ocl_internal_sinpi, float);
DECL_VECTOR_1OP(__gen_ocl_internal_sinh, float);
DECL_VECTOR_1OP(__gen_ocl_internal_asin, float);
DECL_VECTOR_1OP(__gen_ocl_internal_asinpi, float);
DECL_VECTOR_1OP(__gen_ocl_internal_asinh, float);
DECL_VECTOR_1OP(native_tan, float);
DECL_VECTOR_1OP(__gen_ocl_internal_tanpi, float);
DECL_VECTOR_1OP(__gen_ocl_internal_tanh, float);
DECL_VECTOR_1OP(__gen_ocl_internal_atan, float);
DECL_VECTOR_1OP(__gen_ocl_internal_atanpi, float);
DECL_VECTOR_1OP(__gen_ocl_internal_atanh, float);
DECL_VECTOR_1OP(native_sqrt, float);
DECL_VECTOR_1OP(native_rsqrt, float);
DECL_VECTOR_1OP(native_log2, float);
DECL_VECTOR_1OP(log1p, float);
DECL_VECTOR_1OP(logb, float);
DECL_VECTOR_1OP(native_recip, float);
DECL_VECTOR_1OP(native_exp2, float);
DECL_VECTOR_1OP(native_exp10, float);
DECL_VECTOR_1OP(__gen_ocl_internal_expm1, float);
DECL_VECTOR_1OP(__gen_ocl_internal_cbrt, float);
DECL_VECTOR_1OP(__gen_ocl_internal_fabs, float);
DECL_VECTOR_1OP(__gen_ocl_internal_trunc, float);
DECL_VECTOR_1OP(__gen_ocl_internal_round, float);
DECL_VECTOR_1OP(__gen_ocl_internal_floor, float);
DECL_VECTOR_1OP(__gen_ocl_internal_ceil, float);
DECL_VECTOR_1OP(__gen_ocl_internal_log, float);
DECL_VECTOR_1OP(__gen_ocl_internal_log2, float);
DECL_VECTOR_1OP(__gen_ocl_internal_log10, float);
DECL_VECTOR_1OP(__gen_ocl_internal_rint, float);
DECL_VECTOR_1OP(__gen_ocl_internal_erf, float);
DECL_VECTOR_1OP(__gen_ocl_internal_erfc, float);
#undef DECL_VECTOR_1OP
/////////////////////////////////////////////////////////////////////////////
// Arithmetic functions
/////////////////////////////////////////////////////////////////////////////

#define DECL_VECTOR_2OP(NAME, TYPE) \
  INLINE_OVERLOADABLE TYPE##2 NAME(TYPE##2 v0, TYPE##2 v1) { \
    return (TYPE##2)(NAME(v0.x, v1.x), NAME(v0.y, v1.y)); \
  }\
  INLINE_OVERLOADABLE TYPE##3 NAME(TYPE##3 v0, TYPE##3 v1) { \
    return (TYPE##3)(NAME(v0.x, v1.x), NAME(v0.y, v1.y), NAME(v0.z, v1.z)); \
  }\
  INLINE_OVERLOADABLE TYPE##4 NAME(TYPE##4 v0, TYPE##4 v1) { \
    return (TYPE##4)(NAME(v0.x, v1.x), NAME(v0.y, v1.y), NAME(v0.z, v1.z), NAME(v0.w, v1.w)); \
  }\
  INLINE_OVERLOADABLE TYPE##8 NAME(TYPE##8 v0, TYPE##8 v1) { \
    TYPE##8 dst;\
    dst.s0123 = NAME(v0.s0123, v1.s0123);\
    dst.s4567 = NAME(v0.s4567, v1.s4567);\
    return dst;\
  }\
  INLINE_OVERLOADABLE TYPE##16 NAME(TYPE##16 v0, TYPE##16 v1) { \
    TYPE##16 dst;\
    dst.s01234567 = NAME(v0.s01234567, v1.s01234567);\
    dst.s89abcdef = NAME(v0.s89abcdef, v1.s89abcdef);\
    return dst;\
  }
DECL_VECTOR_2OP(hypot, float);
DECL_VECTOR_2OP(min, float);
DECL_VECTOR_2OP(max, float);
DECL_VECTOR_2OP(__gen_ocl_internal_fmin, float);
DECL_VECTOR_2OP(__gen_ocl_internal_fmax, float);
DECL_VECTOR_2OP(__gen_ocl_internal_fdim, float);
DECL_VECTOR_2OP(fmod, float);
DECL_VECTOR_2OP(remainder, float);
DECL_VECTOR_2OP(powr, float);
DECL_VECTOR_2OP(native_divide, float);
DECL_VECTOR_2OP(copysign, float);
DECL_VECTOR_2OP(__gen_ocl_internal_maxmag, float);
DECL_VECTOR_2OP(__gen_ocl_internal_minmag, float);

#define DECL_VECTOR_NOP_ALL_INT_TYPES(NOP, NAME) \
NOP(NAME, char)   \
NOP(NAME, uchar)  \
NOP(NAME, short)  \
NOP(NAME, ushort) \
NOP(NAME, int)    \
NOP(NAME, uint)   \
NOP(NAME, long)   \
NOP(NAME, ulong)

DECL_VECTOR_NOP_ALL_INT_TYPES(DECL_VECTOR_2OP, add_sat)
DECL_VECTOR_NOP_ALL_INT_TYPES(DECL_VECTOR_2OP, sub_sat)
#undef DECL_VECTOR_NOP_ALL_INT_TYPES
#undef DECL_VECTOR_2OP

#define DECL_VECTOR_2OP(NAME, TYPE, TYPE2) \
  INLINE_OVERLOADABLE TYPE##2 NAME(TYPE##2 v0, TYPE2##2 v1) { \
    return (TYPE##2)(NAME(v0.x, v1.x), NAME(v0.y, v1.y)); \
  }\
  INLINE_OVERLOADABLE TYPE##3 NAME(TYPE##3 v0, TYPE2##3 v1) { \
    return (TYPE##3)(NAME(v0.x, v1.x), NAME(v0.y, v1.y), NAME(v0.z, v1.z)); \
  }\
  INLINE_OVERLOADABLE TYPE##4 NAME(TYPE##4 v0, TYPE2##4 v1) { \
    return (TYPE##4)(NAME(v0.x, v1.x), NAME(v0.y, v1.y), NAME(v0.z, v1.z), NAME(v0.w, v1.w)); \
  }\
  INLINE_OVERLOADABLE TYPE##8 NAME(TYPE##8 v0, TYPE2##8 v1) { \
    TYPE##8 dst;\
    dst.s0123 = NAME(v0.s0123, v1.s0123);\
    dst.s4567 = NAME(v0.s4567, v1.s4567);\
    return dst;\
  }\
  INLINE_OVERLOADABLE TYPE##16 NAME(TYPE##16 v0, TYPE2##16 v1) { \
    TYPE##16 dst;\
    dst.s01234567 = NAME(v0.s01234567, v1.s01234567);\
    dst.s89abcdef = NAME(v0.s89abcdef, v1.s89abcdef);\
    return dst;\
  }
DECL_VECTOR_2OP(ldexp, float, int);
DECL_VECTOR_2OP(pown, float, int);
DECL_VECTOR_2OP(rootn, float, int);
#undef DECL_VECTOR_2OP

#define DECL_VECTOR_3OP(NAME, TYPE) \
  INLINE_OVERLOADABLE TYPE##2 NAME(TYPE##2 v0, TYPE##2 v1, TYPE##2 v2) { \
    return (TYPE##2)(NAME(v0.x, v1.x, v2.x), NAME(v0.y, v1.y, v2.y)); \
  }\
  INLINE_OVERLOADABLE TYPE##3 NAME(TYPE##3 v0, TYPE##3 v1, TYPE##3 v2) { \
    return (TYPE##3)(NAME(v0.x, v1.x, v2.x), NAME(v0.y, v1.y, v2.y), NAME(v0.z, v1.z, v2.z)); \
  }\
  INLINE_OVERLOADABLE TYPE##4 NAME(TYPE##4 v0, TYPE##4 v1, TYPE##4 v2) { \
    return (TYPE##4)(NAME(v0.x, v1.x, v2.x), NAME(v0.y, v1.y, v2.y), NAME(v0.z, v1.z, v2.z), NAME(v0.w, v1.w, v2.w)); \
  }\
  INLINE_OVERLOADABLE TYPE##8 NAME(TYPE##8 v0, TYPE##8 v1, TYPE##8 v2) { \
    TYPE##8 dst;\
    dst.s0123 = NAME(v0.s0123, v1.s0123, v2.s0123);\
    dst.s4567 = NAME(v0.s4567, v1.s4567, v2.s4567);\
    return dst;\
  }\
  INLINE_OVERLOADABLE TYPE##16 NAME(TYPE##16 v0, TYPE##16 v1, TYPE##16 v2) { \
    TYPE##16 dst;\
    dst.s01234567 = NAME(v0.s01234567, v1.s01234567, v2.s01234567);\
    dst.s89abcdef = NAME(v0.s89abcdef, v1.s89abcdef, v2.s89abcdef);\
    return dst;\
  }
DECL_VECTOR_3OP(mad, float);
DECL_VECTOR_3OP(mix, float);
DECL_VECTOR_3OP(smoothstep, float);
#undef DECL_VECTOR_3OP

// mix requires more variants
INLINE_OVERLOADABLE float2 mix(float2 x, float2 y, float a) { return mix(x,y,(float2)(a));}
INLINE_OVERLOADABLE float3 mix(float3 x, float3 y, float a) { return mix(x,y,(float3)(a));}
INLINE_OVERLOADABLE float4 mix(float4 x, float4 y, float a) { return mix(x,y,(float4)(a));}
INLINE_OVERLOADABLE float8 mix(float8 x, float8 y, float a) { return mix(x,y,(float8)(a));}
INLINE_OVERLOADABLE float16 mix(float16 x, float16 y, float a) { return mix(x,y,(float16)(a));}

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
#undef INLINE_OVERLOADABLE

#undef PURE
#undef CONST
#undef OVERLOADABLE
#undef INLINE
#endif /* __GEN_OCL_STDLIB_H__ */

