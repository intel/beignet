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

int INLINE_OVERLOADABLE isequal(float x, float y) { return x == y; }
int INLINE_OVERLOADABLE isnotequal(float x, float y) { return x != y; }
int INLINE_OVERLOADABLE isgreater(float x, float y) { return x > y; }
int INLINE_OVERLOADABLE isgreaterequal(float x, float y) { return x >= y; }
int INLINE_OVERLOADABLE isless(float x, float y) { return x < y; }
int INLINE_OVERLOADABLE islessequal(float x, float y) { return x <= y; }

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

OVERLOADABLE int __gen_ocl_mul_hi(int x, int y);
OVERLOADABLE uint __gen_ocl_mul_hi(uint x, uint y);
INLINE_OVERLOADABLE char mul_hi(char x, char y) { return (x * y) >> 8; }
INLINE_OVERLOADABLE uchar mul_hi(uchar x, uchar y) { return (x * y) >> 8; }
INLINE_OVERLOADABLE short mul_hi(short x, short y) { return (x * y) >> 16; }
INLINE_OVERLOADABLE ushort mul_hi(ushort x, ushort y) { return (x * y) >> 16; }
INLINE_OVERLOADABLE int mul_hi(int x, int y) { return __gen_ocl_mul_hi(x, y); }
INLINE_OVERLOADABLE uint mul_hi(uint x, uint y) { return __gen_ocl_mul_hi(x, y); }

#define DEF(type) INLINE_OVERLOADABLE type mad_hi(type a, type b, type c) { return mul_hi(a, b) + c; }
DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(int)
DEF(uint)
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

/* XXX not implemented. */
INLINE_OVERLOADABLE int mad_sat(int a, int b, int c) {
  return 0;
}

INLINE_OVERLOADABLE uint mad_sat(uint a, uint b, uint c) {
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

OVERLOADABLE short __gen_ocl_upsample(short hi, short lo);
OVERLOADABLE int __gen_ocl_upsample(int hi, int lo);
INLINE_OVERLOADABLE short upsample(char hi, uchar lo) { return __gen_ocl_upsample((short)hi, (short)lo); }
INLINE_OVERLOADABLE ushort upsample(uchar hi, uchar lo) { return __gen_ocl_upsample((short)hi, (short)lo); }
INLINE_OVERLOADABLE int upsample(short hi, ushort lo) { return __gen_ocl_upsample((int)hi, (int)lo); }
INLINE_OVERLOADABLE uint upsample(ushort hi, ushort lo) { return __gen_ocl_upsample((int)hi, (int)lo); }

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

int __gen_ocl_abs(int x);
#define DEC(TYPE) INLINE_OVERLOADABLE u##TYPE abs(TYPE x) { return (u##TYPE) __gen_ocl_abs(x); }
DEC(int)
DEC(short)
DEC(char)
#undef DEC
/* For unsigned types, do nothing. */
#define DEC(TYPE) INLINE_OVERLOADABLE TYPE abs(TYPE x) { return x; }
DEC(uint)
DEC(ushort)
DEC(uchar)
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
INLINE_OVERLOADABLE float __gen_ocl_internal_sincos(float x, float *cosval) {
  *cosval = native_cos(x);
  return native_sin(x);
}
INLINE_OVERLOADABLE float sincos(float x, global float *cosval) { return __gen_ocl_internal_sincos(x, (float*)cosval); }
INLINE_OVERLOADABLE float sincos(float x, local float *cosval) { return __gen_ocl_internal_sincos(x, (float*)cosval); }
INLINE_OVERLOADABLE float sincos(float x, private float *cosval) { return __gen_ocl_internal_sincos(x, (float*)cosval); }
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
#undef DECL_MIN_MAX_CLAMP

INLINE_OVERLOADABLE float __gen_ocl_frexp(float x, int *exp) {
  uint u = as_uint(x);
  if ((u & 0x7FFFFFFFu) == 0) {
    *exp = 0;
    return x;
  }
  int e = (u >> 23) & 255;
  if (e == 255)
    return x;
  *exp = e - 126;
  u = (u & (0x807FFFFFu)) | 0x3F000000;
  return as_float(u);
}
INLINE_OVERLOADABLE float frexp(float x, global int *exp) { return __gen_ocl_frexp(x, (int *)exp); }
INLINE_OVERLOADABLE float frexp(float x, local int *exp) { return __gen_ocl_frexp(x, (int *)exp); }
INLINE_OVERLOADABLE float frexp(float x, private int *exp) { return __gen_ocl_frexp(x, (int *)exp); }
INLINE_OVERLOADABLE float2 frexp(float2 x, int2 *exp) {
  return (float2)(frexp(x.s0, (int *)exp), frexp(x.s1, 1 + (int *)exp));
}

INLINE_OVERLOADABLE float3 frexp(float3 x, int3 *exp) {
  return (float3)(frexp(x.s0, (int *)exp), frexp(x.s1, 1 + (int *)exp), frexp(x.s2, 2 + (int *)exp));
}

INLINE_OVERLOADABLE float4 frexp(float4 x, int4 *exp) {
  return (float4)(frexp(x.s0, (int *)exp), frexp(x.s1, 1 + (int *)exp), frexp(x.s2, 2 + (int *)exp), frexp(x.s3, 3 + (int *)exp));
}

INLINE_OVERLOADABLE float8 frexp(float8 x, int8 *exp) {
  return (float8)(frexp(x.s0, (int *)exp), frexp(x.s1, 1 + (int *)exp), frexp(x.s2, 2 + (int *)exp), frexp(x.s3, 3 + (int *)exp), frexp(x.s4, 4 + (int *)exp), frexp(x.s5, 5 + (int *)exp), frexp(x.s6, 6 + (int *)exp), frexp(x.s7, 7 + (int *)exp));
}

INLINE_OVERLOADABLE float16 frexp(float16 x, int16 *exp) {
  return (float16)(frexp(x.s0, (int *)exp), frexp(x.s1, 1 + (int *)exp), frexp(x.s2, 2 + (int *)exp), frexp(x.s3, 3 + (int *)exp), frexp(x.s4, 4 + (int *)exp), frexp(x.s5, 5 + (int *)exp), frexp(x.s6, 6 + (int *)exp), frexp(x.s7, 7 + (int *)exp), frexp(x.s8, 8 + (int *)exp), frexp(x.s9, 9 + (int *)exp), frexp(x.sa, 10 + (int *)exp), frexp(x.sb, 11 + (int *)exp), frexp(x.sc, 12 + (int *)exp), frexp(x.sd, 13 + (int *)exp), frexp(x.se, 14 + (int *)exp), frexp(x.sf, 15 + (int *)exp));
}

INLINE_OVERLOADABLE float degrees(float radians) { return (180 / M_PI_F) * radians; }
INLINE_OVERLOADABLE float radians(float degrees) { return (M_PI_F / 180) * degrees; }

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

INLINE_OVERLOADABLE float modf(float x, float *i) {
  uint hx = as_uint(x), ix = hx & 0x7FFFFFFF;
  if (ix > 0x7F800000) {
    *i = nan(0u);
    return nan(0u);
  }
  if (ix == 0x7F800000) {
    *i = x;
    return as_float(hx & 0x80000000u);
  }
  *i = __gen_ocl_rndz(x);
  return x - *i;
}

INLINE_OVERLOADABLE float2 modf(float2 x, float2 *i) {
  return (float2)(modf(x.s0, (float *)i), modf(x.s1, 1 + (float *)i));
}

INLINE_OVERLOADABLE float3 modf(float3 x, float3 *i) {
  return (float3)(modf(x.s0, (float *)i), modf(x.s1, 1 + (float *)i), modf(x.s2, 2 + (float *)i));
}

INLINE_OVERLOADABLE float4 modf(float4 x, float4 *i) {
  return (float4)(modf(x.s0, (float *)i), modf(x.s1, 1 + (float *)i), modf(x.s2, 2 + (float *)i), modf(x.s3, 3 + (float *)i));
}

INLINE_OVERLOADABLE float8 modf(float8 x, float8 *i) {
  return (float8)(modf(x.s0, (float *)i), modf(x.s1, 1 + (float *)i), modf(x.s2, 2 + (float *)i), modf(x.s3, 3 + (float *)i), modf(x.s4, 4 + (float *)i), modf(x.s5, 5 + (float *)i), modf(x.s6, 6 + (float *)i), modf(x.s7, 7 + (float *)i));
}

INLINE_OVERLOADABLE float16 modf(float16 x, float16 *i) {
  return (float16)(modf(x.s0, (float *)i), modf(x.s1, 1 + (float *)i), modf(x.s2, 2 + (float *)i), modf(x.s3, 3 + (float *)i), modf(x.s4, 4 + (float *)i), modf(x.s5, 5 + (float *)i), modf(x.s6, 6 + (float *)i), modf(x.s7, 7 + (float *)i), modf(x.s8, 8 + (float *)i), modf(x.s9, 9 + (float *)i), modf(x.sa, 10 + (float *)i), modf(x.sb, 11 + (float *)i), modf(x.sc, 12 + (float *)i), modf(x.sd, 13 + (float *)i), modf(x.se, 14 + (float *)i), modf(x.sf, 15 + (float *)i));
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
INLINE_OVERLOADABLE float __gen_ocl_fract(float x, float *p) {
  *p = __gen_ocl_internal_floor(x);
  return __gen_ocl_internal_fmin(x - *p, 0x1.FFFFFep-1F);
}
INLINE_OVERLOADABLE float fract(float x, global float *p) { return __gen_ocl_fract(x, (float *)p); }
INLINE_OVERLOADABLE float fract(float x, local float *p) { return __gen_ocl_fract(x, (float *)p); }
INLINE_OVERLOADABLE float fract(float x, private float *p) { return __gen_ocl_fract(x, (float *)p); }
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

INLINE_OVERLOADABLE float remquo(float x, float y, int *quo) {
  uint hx = as_uint(x), ix = hx & 0x7FFFFFFF, hy = as_uint(y), iy = hy & 0x7FFFFFFF;
  if (ix > 0x7F800000 || iy > 0x7F800000 || ix == 0x7F800000 || iy == 0)
    return nan(0u);
  float k = x / y;
  int q =  __gen_ocl_rnde(k);
  *quo = q >= 0 ? (q & 127) : (q | 0xFFFFFF80u);
  float r = x - q * y;
  uint hr = as_uint(r), ir = hr & 0x7FFFFFFF;
  if (ir == 0)
    hr = ir | (hx & 0x80000000u);
  return as_float(hr);
}

INLINE_OVERLOADABLE float2 remquo(float2 x, float2 y, int2 *i) {
  return (float2)(remquo(x.s0, y.s0, (int *)i), remquo(x.s1, y.s1, 1 + (int *)i));
}

INLINE_OVERLOADABLE float3 remquo(float3 x, float3 y, int3 *i) {
  return (float3)(remquo(x.s0, y.s0, (int *)i), remquo(x.s1, y.s1, 1 + (int *)i), remquo(x.s2, y.s2, 2 + (int *)i));
}

INLINE_OVERLOADABLE float4 remquo(float4 x, float4 y, int4 *i) {
  return (float4)(remquo(x.s0, y.s0, (int *)i), remquo(x.s1, y.s1, 1 + (int *)i), remquo(x.s2, y.s2, 2 + (int *)i), remquo(x.s3, y.s3, 3 + (int *)i));
}

INLINE_OVERLOADABLE float8 remquo(float8 x, float8 y, int8 *i) {
  return (float8)(remquo(x.s0, y.s0, (int *)i), remquo(x.s1, y.s1, 1 + (int *)i), remquo(x.s2, y.s2, 2 + (int *)i), remquo(x.s3, y.s3, 3 + (int *)i), remquo(x.s4, y.s4, 4 + (int *)i), remquo(x.s5, y.s5, 5 + (int *)i), remquo(x.s6, y.s6, 6 + (int *)i), remquo(x.s7, y.s7, 7 + (int *)i));
}

INLINE_OVERLOADABLE float16 remquo(float16 x, float16 y, int16 *i) {
  return (float16)(remquo(x.s0, y.s0, (int *)i), remquo(x.s1, y.s1, 1 + (int *)i), remquo(x.s2, y.s2, 2 + (int *)i), remquo(x.s3, y.s3, 3 + (int *)i), remquo(x.s4, y.s4, 4 + (int *)i), remquo(x.s5, y.s5, 5 + (int *)i), remquo(x.s6, y.s6, 6 + (int *)i), remquo(x.s7, y.s7, 7 + (int *)i), remquo(x.s8, y.s8, 8 + (int *)i), remquo(x.s9, y.s9, 9 + (int *)i), remquo(x.sa, y.sa, 10 + (int *)i), remquo(x.sb, y.sb, 11 + (int *)i), remquo(x.sc, y.sc, 12 + (int *)i), remquo(x.sd, y.sd, 13 + (int *)i), remquo(x.se, y.se, 14 + (int *)i), remquo(x.sf, y.sf, 15 + (int *)i));
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
// Miscellaneous Vector Functions (see 6.11.12 of OCL 1.1 spec)
/////////////////////////////////////////////////////////////////////////////
#define DEC2(TYPE, XTYPE) \
  INLINE_OVERLOADABLE TYPE##2 shuffle(XTYPE x, uint2 mask) { \
    TYPE##2 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & 1]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & 1]; \
    return y; \
  }

#define DEC3(TYPE, XTYPE) \
  INLINE_OVERLOADABLE TYPE##3 shuffle(XTYPE x, uint3 mask) { \
    TYPE##3 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & 3]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & 3]; \
    y.s2 = ((TYPE *) &x)[mask.s2 & 3]; \
    return y; \
  }

#define DEC4(TYPE, XTYPE) \
  INLINE_OVERLOADABLE TYPE##4 shuffle(XTYPE x, uint4 mask) { \
    TYPE##4 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & 3]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & 3]; \
    y.s2 = ((TYPE *) &x)[mask.s2 & 3]; \
    y.s3 = ((TYPE *) &x)[mask.s3 & 3]; \
    return y; \
  }

#define DEC8(TYPE, XTYPE) \
  INLINE_OVERLOADABLE TYPE##8 shuffle(XTYPE x, uint8 mask) { \
    TYPE##8 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & 7]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & 7]; \
    y.s2 = ((TYPE *) &x)[mask.s2 & 7]; \
    y.s3 = ((TYPE *) &x)[mask.s3 & 7]; \
    y.s4 = ((TYPE *) &x)[mask.s4 & 7]; \
    y.s5 = ((TYPE *) &x)[mask.s5 & 7]; \
    y.s6 = ((TYPE *) &x)[mask.s6 & 7]; \
    y.s7 = ((TYPE *) &x)[mask.s7 & 7]; \
    return y; \
  }

#define DEC16(TYPE, XTYPE) \
  INLINE_OVERLOADABLE TYPE##16 shuffle(XTYPE x, uint16 mask) { \
    TYPE##16 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & 15]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & 15]; \
    y.s2 = ((TYPE *) &x)[mask.s2 & 15]; \
    y.s3 = ((TYPE *) &x)[mask.s3 & 15]; \
    y.s4 = ((TYPE *) &x)[mask.s4 & 15]; \
    y.s5 = ((TYPE *) &x)[mask.s5 & 15]; \
    y.s6 = ((TYPE *) &x)[mask.s6 & 15]; \
    y.s7 = ((TYPE *) &x)[mask.s7 & 15]; \
    y.s8 = ((TYPE *) &x)[mask.s8 & 15]; \
    y.s9 = ((TYPE *) &x)[mask.s9 & 15]; \
    y.sa = ((TYPE *) &x)[mask.sa & 15]; \
    y.sb = ((TYPE *) &x)[mask.sb & 15]; \
    y.sc = ((TYPE *) &x)[mask.sc & 15]; \
    y.sd = ((TYPE *) &x)[mask.sd & 15]; \
    y.se = ((TYPE *) &x)[mask.se & 15]; \
    y.sf = ((TYPE *) &x)[mask.sf & 15]; \
    return y; \
  }

#define DEF(TYPE) \
  DEC2(TYPE, TYPE##2); DEC2(TYPE, TYPE##3); DEC2(TYPE, TYPE##4); DEC2(TYPE, TYPE##8); DEC2(TYPE, TYPE##16) \
  DEC3(TYPE, TYPE##2); DEC3(TYPE, TYPE##3); DEC3(TYPE, TYPE##4); DEC3(TYPE, TYPE##8); DEC3(TYPE, TYPE##16) \
  DEC4(TYPE, TYPE##2); DEC4(TYPE, TYPE##3); DEC4(TYPE, TYPE##4); DEC4(TYPE, TYPE##8); DEC4(TYPE, TYPE##16) \
  DEC8(TYPE, TYPE##2); DEC8(TYPE, TYPE##3); DEC8(TYPE, TYPE##4); DEC8(TYPE, TYPE##8); DEC8(TYPE, TYPE##16) \
  DEC16(TYPE, TYPE##2); DEC16(TYPE, TYPE##3); DEC16(TYPE, TYPE##4); DEC16(TYPE, TYPE##8); DEC16(TYPE, TYPE##16)
DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(int)
DEF(uint)
DEF(float)
#undef DEF
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16
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
// ##BEGIN_BUILTIN_VECTOR##
//gentype acos (gentype)

INLINE_OVERLOADABLE float2 acos (float2 param0)
{return (float2)(acos(param0.s0), acos(param0.s1)); }

INLINE_OVERLOADABLE float3 acos (float3 param0)
{return (float3)(acos(param0.s0), acos(param0.s1),
                 acos(param0.s2)); }

INLINE_OVERLOADABLE float4 acos (float4 param0)
{return (float4)(acos(param0.s0), acos(param0.s1),
                 acos(param0.s2), acos(param0.s3)); }

INLINE_OVERLOADABLE float8 acos (float8 param0)
{return (float8)(acos(param0.s0), acos(param0.s1),
                 acos(param0.s2), acos(param0.s3),
                 acos(param0.s4), acos(param0.s5),
                 acos(param0.s6), acos(param0.s7)); }

INLINE_OVERLOADABLE float16 acos (float16 param0)
{return (float16)(acos(param0.s0),  acos(param0.s1),
                  acos(param0.s2),  acos(param0.s3),
                  acos(param0.s4),  acos(param0.s5),
                  acos(param0.s6),  acos(param0.s7),
                  acos(param0.s8),  acos(param0.s9),
                  acos(param0.sa), acos(param0.sb),
                  acos(param0.sc), acos(param0.sd),
                  acos(param0.se), acos(param0.sf)); }


//gentype acosh (gentype)

INLINE_OVERLOADABLE float2 acosh (float2 param0)
{return (float2)(acosh(param0.s0), acosh(param0.s1)); }

INLINE_OVERLOADABLE float3 acosh (float3 param0)
{return (float3)(acosh(param0.s0), acosh(param0.s1),
                 acosh(param0.s2)); }

INLINE_OVERLOADABLE float4 acosh (float4 param0)
{return (float4)(acosh(param0.s0), acosh(param0.s1),
                 acosh(param0.s2), acosh(param0.s3)); }

INLINE_OVERLOADABLE float8 acosh (float8 param0)
{return (float8)(acosh(param0.s0), acosh(param0.s1),
                 acosh(param0.s2), acosh(param0.s3),
                 acosh(param0.s4), acosh(param0.s5),
                 acosh(param0.s6), acosh(param0.s7)); }

INLINE_OVERLOADABLE float16 acosh (float16 param0)
{return (float16)(acosh(param0.s0),  acosh(param0.s1),
                  acosh(param0.s2),  acosh(param0.s3),
                  acosh(param0.s4),  acosh(param0.s5),
                  acosh(param0.s6),  acosh(param0.s7),
                  acosh(param0.s8),  acosh(param0.s9),
                  acosh(param0.sa), acosh(param0.sb),
                  acosh(param0.sc), acosh(param0.sd),
                  acosh(param0.se), acosh(param0.sf)); }


//gentype acospi (gentype x)

INLINE_OVERLOADABLE float2 acospi (float2 param0)
{return (float2)(acospi(param0.s0), acospi(param0.s1)); }

INLINE_OVERLOADABLE float3 acospi (float3 param0)
{return (float3)(acospi(param0.s0), acospi(param0.s1),
                 acospi(param0.s2)); }

INLINE_OVERLOADABLE float4 acospi (float4 param0)
{return (float4)(acospi(param0.s0), acospi(param0.s1),
                 acospi(param0.s2), acospi(param0.s3)); }

INLINE_OVERLOADABLE float8 acospi (float8 param0)
{return (float8)(acospi(param0.s0), acospi(param0.s1),
                 acospi(param0.s2), acospi(param0.s3),
                 acospi(param0.s4), acospi(param0.s5),
                 acospi(param0.s6), acospi(param0.s7)); }

INLINE_OVERLOADABLE float16 acospi (float16 param0)
{return (float16)(acospi(param0.s0),  acospi(param0.s1),
                  acospi(param0.s2),  acospi(param0.s3),
                  acospi(param0.s4),  acospi(param0.s5),
                  acospi(param0.s6),  acospi(param0.s7),
                  acospi(param0.s8),  acospi(param0.s9),
                  acospi(param0.sa), acospi(param0.sb),
                  acospi(param0.sc), acospi(param0.sd),
                  acospi(param0.se), acospi(param0.sf)); }


//gentype asin (gentype)

INLINE_OVERLOADABLE float2 asin (float2 param0)
{return (float2)(asin(param0.s0), asin(param0.s1)); }

INLINE_OVERLOADABLE float3 asin (float3 param0)
{return (float3)(asin(param0.s0), asin(param0.s1),
                 asin(param0.s2)); }

INLINE_OVERLOADABLE float4 asin (float4 param0)
{return (float4)(asin(param0.s0), asin(param0.s1),
                 asin(param0.s2), asin(param0.s3)); }

INLINE_OVERLOADABLE float8 asin (float8 param0)
{return (float8)(asin(param0.s0), asin(param0.s1),
                 asin(param0.s2), asin(param0.s3),
                 asin(param0.s4), asin(param0.s5),
                 asin(param0.s6), asin(param0.s7)); }

INLINE_OVERLOADABLE float16 asin (float16 param0)
{return (float16)(asin(param0.s0),  asin(param0.s1),
                  asin(param0.s2),  asin(param0.s3),
                  asin(param0.s4),  asin(param0.s5),
                  asin(param0.s6),  asin(param0.s7),
                  asin(param0.s8),  asin(param0.s9),
                  asin(param0.sa), asin(param0.sb),
                  asin(param0.sc), asin(param0.sd),
                  asin(param0.se), asin(param0.sf)); }


//gentype asinh (gentype)

INLINE_OVERLOADABLE float2 asinh (float2 param0)
{return (float2)(asinh(param0.s0), asinh(param0.s1)); }

INLINE_OVERLOADABLE float3 asinh (float3 param0)
{return (float3)(asinh(param0.s0), asinh(param0.s1),
                 asinh(param0.s2)); }

INLINE_OVERLOADABLE float4 asinh (float4 param0)
{return (float4)(asinh(param0.s0), asinh(param0.s1),
                 asinh(param0.s2), asinh(param0.s3)); }

INLINE_OVERLOADABLE float8 asinh (float8 param0)
{return (float8)(asinh(param0.s0), asinh(param0.s1),
                 asinh(param0.s2), asinh(param0.s3),
                 asinh(param0.s4), asinh(param0.s5),
                 asinh(param0.s6), asinh(param0.s7)); }

INLINE_OVERLOADABLE float16 asinh (float16 param0)
{return (float16)(asinh(param0.s0),  asinh(param0.s1),
                  asinh(param0.s2),  asinh(param0.s3),
                  asinh(param0.s4),  asinh(param0.s5),
                  asinh(param0.s6),  asinh(param0.s7),
                  asinh(param0.s8),  asinh(param0.s9),
                  asinh(param0.sa), asinh(param0.sb),
                  asinh(param0.sc), asinh(param0.sd),
                  asinh(param0.se), asinh(param0.sf)); }


//gentype asinpi (gentype x)

INLINE_OVERLOADABLE float2 asinpi (float2 param0)
{return (float2)(asinpi(param0.s0), asinpi(param0.s1)); }

INLINE_OVERLOADABLE float3 asinpi (float3 param0)
{return (float3)(asinpi(param0.s0), asinpi(param0.s1),
                 asinpi(param0.s2)); }

INLINE_OVERLOADABLE float4 asinpi (float4 param0)
{return (float4)(asinpi(param0.s0), asinpi(param0.s1),
                 asinpi(param0.s2), asinpi(param0.s3)); }

INLINE_OVERLOADABLE float8 asinpi (float8 param0)
{return (float8)(asinpi(param0.s0), asinpi(param0.s1),
                 asinpi(param0.s2), asinpi(param0.s3),
                 asinpi(param0.s4), asinpi(param0.s5),
                 asinpi(param0.s6), asinpi(param0.s7)); }

INLINE_OVERLOADABLE float16 asinpi (float16 param0)
{return (float16)(asinpi(param0.s0),  asinpi(param0.s1),
                  asinpi(param0.s2),  asinpi(param0.s3),
                  asinpi(param0.s4),  asinpi(param0.s5),
                  asinpi(param0.s6),  asinpi(param0.s7),
                  asinpi(param0.s8),  asinpi(param0.s9),
                  asinpi(param0.sa), asinpi(param0.sb),
                  asinpi(param0.sc), asinpi(param0.sd),
                  asinpi(param0.se), asinpi(param0.sf)); }


//gentype atan (gentype y_over_x)

INLINE_OVERLOADABLE float2 atan (float2 param0)
{return (float2)(atan(param0.s0), atan(param0.s1)); }

INLINE_OVERLOADABLE float3 atan (float3 param0)
{return (float3)(atan(param0.s0), atan(param0.s1),
                 atan(param0.s2)); }

INLINE_OVERLOADABLE float4 atan (float4 param0)
{return (float4)(atan(param0.s0), atan(param0.s1),
                 atan(param0.s2), atan(param0.s3)); }

INLINE_OVERLOADABLE float8 atan (float8 param0)
{return (float8)(atan(param0.s0), atan(param0.s1),
                 atan(param0.s2), atan(param0.s3),
                 atan(param0.s4), atan(param0.s5),
                 atan(param0.s6), atan(param0.s7)); }

INLINE_OVERLOADABLE float16 atan (float16 param0)
{return (float16)(atan(param0.s0),  atan(param0.s1),
                  atan(param0.s2),  atan(param0.s3),
                  atan(param0.s4),  atan(param0.s5),
                  atan(param0.s6),  atan(param0.s7),
                  atan(param0.s8),  atan(param0.s9),
                  atan(param0.sa), atan(param0.sb),
                  atan(param0.sc), atan(param0.sd),
                  atan(param0.se), atan(param0.sf)); }


//gentype atanh (gentype)

INLINE_OVERLOADABLE float2 atanh (float2 param0)
{return (float2)(atanh(param0.s0), atanh(param0.s1)); }

INLINE_OVERLOADABLE float3 atanh (float3 param0)
{return (float3)(atanh(param0.s0), atanh(param0.s1),
                 atanh(param0.s2)); }

INLINE_OVERLOADABLE float4 atanh (float4 param0)
{return (float4)(atanh(param0.s0), atanh(param0.s1),
                 atanh(param0.s2), atanh(param0.s3)); }

INLINE_OVERLOADABLE float8 atanh (float8 param0)
{return (float8)(atanh(param0.s0), atanh(param0.s1),
                 atanh(param0.s2), atanh(param0.s3),
                 atanh(param0.s4), atanh(param0.s5),
                 atanh(param0.s6), atanh(param0.s7)); }

INLINE_OVERLOADABLE float16 atanh (float16 param0)
{return (float16)(atanh(param0.s0),  atanh(param0.s1),
                  atanh(param0.s2),  atanh(param0.s3),
                  atanh(param0.s4),  atanh(param0.s5),
                  atanh(param0.s6),  atanh(param0.s7),
                  atanh(param0.s8),  atanh(param0.s9),
                  atanh(param0.sa), atanh(param0.sb),
                  atanh(param0.sc), atanh(param0.sd),
                  atanh(param0.se), atanh(param0.sf)); }


//gentype atanpi (gentype x)

INLINE_OVERLOADABLE float2 atanpi (float2 param0)
{return (float2)(atanpi(param0.s0), atanpi(param0.s1)); }

INLINE_OVERLOADABLE float3 atanpi (float3 param0)
{return (float3)(atanpi(param0.s0), atanpi(param0.s1),
                 atanpi(param0.s2)); }

INLINE_OVERLOADABLE float4 atanpi (float4 param0)
{return (float4)(atanpi(param0.s0), atanpi(param0.s1),
                 atanpi(param0.s2), atanpi(param0.s3)); }

INLINE_OVERLOADABLE float8 atanpi (float8 param0)
{return (float8)(atanpi(param0.s0), atanpi(param0.s1),
                 atanpi(param0.s2), atanpi(param0.s3),
                 atanpi(param0.s4), atanpi(param0.s5),
                 atanpi(param0.s6), atanpi(param0.s7)); }

INLINE_OVERLOADABLE float16 atanpi (float16 param0)
{return (float16)(atanpi(param0.s0),  atanpi(param0.s1),
                  atanpi(param0.s2),  atanpi(param0.s3),
                  atanpi(param0.s4),  atanpi(param0.s5),
                  atanpi(param0.s6),  atanpi(param0.s7),
                  atanpi(param0.s8),  atanpi(param0.s9),
                  atanpi(param0.sa), atanpi(param0.sb),
                  atanpi(param0.sc), atanpi(param0.sd),
                  atanpi(param0.se), atanpi(param0.sf)); }


//gentype cbrt (gentype)

INLINE_OVERLOADABLE float2 cbrt (float2 param0)
{return (float2)(cbrt(param0.s0), cbrt(param0.s1)); }

INLINE_OVERLOADABLE float3 cbrt (float3 param0)
{return (float3)(cbrt(param0.s0), cbrt(param0.s1),
                 cbrt(param0.s2)); }

INLINE_OVERLOADABLE float4 cbrt (float4 param0)
{return (float4)(cbrt(param0.s0), cbrt(param0.s1),
                 cbrt(param0.s2), cbrt(param0.s3)); }

INLINE_OVERLOADABLE float8 cbrt (float8 param0)
{return (float8)(cbrt(param0.s0), cbrt(param0.s1),
                 cbrt(param0.s2), cbrt(param0.s3),
                 cbrt(param0.s4), cbrt(param0.s5),
                 cbrt(param0.s6), cbrt(param0.s7)); }

INLINE_OVERLOADABLE float16 cbrt (float16 param0)
{return (float16)(cbrt(param0.s0),  cbrt(param0.s1),
                  cbrt(param0.s2),  cbrt(param0.s3),
                  cbrt(param0.s4),  cbrt(param0.s5),
                  cbrt(param0.s6),  cbrt(param0.s7),
                  cbrt(param0.s8),  cbrt(param0.s9),
                  cbrt(param0.sa), cbrt(param0.sb),
                  cbrt(param0.sc), cbrt(param0.sd),
                  cbrt(param0.se), cbrt(param0.sf)); }


//gentype ceil (gentype)

INLINE_OVERLOADABLE float2 ceil (float2 param0)
{return (float2)(ceil(param0.s0), ceil(param0.s1)); }

INLINE_OVERLOADABLE float3 ceil (float3 param0)
{return (float3)(ceil(param0.s0), ceil(param0.s1),
                 ceil(param0.s2)); }

INLINE_OVERLOADABLE float4 ceil (float4 param0)
{return (float4)(ceil(param0.s0), ceil(param0.s1),
                 ceil(param0.s2), ceil(param0.s3)); }

INLINE_OVERLOADABLE float8 ceil (float8 param0)
{return (float8)(ceil(param0.s0), ceil(param0.s1),
                 ceil(param0.s2), ceil(param0.s3),
                 ceil(param0.s4), ceil(param0.s5),
                 ceil(param0.s6), ceil(param0.s7)); }

INLINE_OVERLOADABLE float16 ceil (float16 param0)
{return (float16)(ceil(param0.s0),  ceil(param0.s1),
                  ceil(param0.s2),  ceil(param0.s3),
                  ceil(param0.s4),  ceil(param0.s5),
                  ceil(param0.s6),  ceil(param0.s7),
                  ceil(param0.s8),  ceil(param0.s9),
                  ceil(param0.sa), ceil(param0.sb),
                  ceil(param0.sc), ceil(param0.sd),
                  ceil(param0.se), ceil(param0.sf)); }


//gentype copysign (gentype x, gentype y)

INLINE_OVERLOADABLE float2 copysign (float2 param0, float2 param1)
{return (float2)(copysign(param0.s0, param1.s0), copysign(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 copysign (float3 param0, float3 param1)
{return (float3)(copysign(param0.s0, param1.s0), copysign(param0.s1, param1.s1),
                 copysign(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 copysign (float4 param0, float4 param1)
{return (float4)(copysign(param0.s0, param1.s0), copysign(param0.s1, param1.s1),
                 copysign(param0.s2, param1.s2), copysign(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 copysign (float8 param0, float8 param1)
{return (float8)(copysign(param0.s0, param1.s0), copysign(param0.s1, param1.s1),
                 copysign(param0.s2, param1.s2), copysign(param0.s3, param1.s3),
                 copysign(param0.s4, param1.s4), copysign(param0.s5, param1.s5),
                 copysign(param0.s6, param1.s6), copysign(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 copysign (float16 param0, float16 param1)
{return (float16)(copysign(param0.s0,  param1.s0),  copysign(param0.s1,  param1.s1),
                  copysign(param0.s2,  param1.s2),  copysign(param0.s3,  param1.s3),
                  copysign(param0.s4,  param1.s4),  copysign(param0.s5,  param1.s5),
                  copysign(param0.s6,  param1.s6),  copysign(param0.s7,  param1.s7),
                  copysign(param0.s8,  param1.s8),  copysign(param0.s9,  param1.s9),
                  copysign(param0.sa, param1.sa), copysign(param0.sb, param1.sb),
                  copysign(param0.sc, param1.sc), copysign(param0.sd, param1.sd),
                  copysign(param0.se, param1.se), copysign(param0.sf, param1.sf)); }


//gentype cos (gentype)

INLINE_OVERLOADABLE float2 cos (float2 param0)
{return (float2)(cos(param0.s0), cos(param0.s1)); }

INLINE_OVERLOADABLE float3 cos (float3 param0)
{return (float3)(cos(param0.s0), cos(param0.s1),
                 cos(param0.s2)); }

INLINE_OVERLOADABLE float4 cos (float4 param0)
{return (float4)(cos(param0.s0), cos(param0.s1),
                 cos(param0.s2), cos(param0.s3)); }

INLINE_OVERLOADABLE float8 cos (float8 param0)
{return (float8)(cos(param0.s0), cos(param0.s1),
                 cos(param0.s2), cos(param0.s3),
                 cos(param0.s4), cos(param0.s5),
                 cos(param0.s6), cos(param0.s7)); }

INLINE_OVERLOADABLE float16 cos (float16 param0)
{return (float16)(cos(param0.s0),  cos(param0.s1),
                  cos(param0.s2),  cos(param0.s3),
                  cos(param0.s4),  cos(param0.s5),
                  cos(param0.s6),  cos(param0.s7),
                  cos(param0.s8),  cos(param0.s9),
                  cos(param0.sa), cos(param0.sb),
                  cos(param0.sc), cos(param0.sd),
                  cos(param0.se), cos(param0.sf)); }


//gentype cosh (gentype)

INLINE_OVERLOADABLE float2 cosh (float2 param0)
{return (float2)(cosh(param0.s0), cosh(param0.s1)); }

INLINE_OVERLOADABLE float3 cosh (float3 param0)
{return (float3)(cosh(param0.s0), cosh(param0.s1),
                 cosh(param0.s2)); }

INLINE_OVERLOADABLE float4 cosh (float4 param0)
{return (float4)(cosh(param0.s0), cosh(param0.s1),
                 cosh(param0.s2), cosh(param0.s3)); }

INLINE_OVERLOADABLE float8 cosh (float8 param0)
{return (float8)(cosh(param0.s0), cosh(param0.s1),
                 cosh(param0.s2), cosh(param0.s3),
                 cosh(param0.s4), cosh(param0.s5),
                 cosh(param0.s6), cosh(param0.s7)); }

INLINE_OVERLOADABLE float16 cosh (float16 param0)
{return (float16)(cosh(param0.s0),  cosh(param0.s1),
                  cosh(param0.s2),  cosh(param0.s3),
                  cosh(param0.s4),  cosh(param0.s5),
                  cosh(param0.s6),  cosh(param0.s7),
                  cosh(param0.s8),  cosh(param0.s9),
                  cosh(param0.sa), cosh(param0.sb),
                  cosh(param0.sc), cosh(param0.sd),
                  cosh(param0.se), cosh(param0.sf)); }


//gentype cospi (gentype x)

INLINE_OVERLOADABLE float2 cospi (float2 param0)
{return (float2)(cospi(param0.s0), cospi(param0.s1)); }

INLINE_OVERLOADABLE float3 cospi (float3 param0)
{return (float3)(cospi(param0.s0), cospi(param0.s1),
                 cospi(param0.s2)); }

INLINE_OVERLOADABLE float4 cospi (float4 param0)
{return (float4)(cospi(param0.s0), cospi(param0.s1),
                 cospi(param0.s2), cospi(param0.s3)); }

INLINE_OVERLOADABLE float8 cospi (float8 param0)
{return (float8)(cospi(param0.s0), cospi(param0.s1),
                 cospi(param0.s2), cospi(param0.s3),
                 cospi(param0.s4), cospi(param0.s5),
                 cospi(param0.s6), cospi(param0.s7)); }

INLINE_OVERLOADABLE float16 cospi (float16 param0)
{return (float16)(cospi(param0.s0),  cospi(param0.s1),
                  cospi(param0.s2),  cospi(param0.s3),
                  cospi(param0.s4),  cospi(param0.s5),
                  cospi(param0.s6),  cospi(param0.s7),
                  cospi(param0.s8),  cospi(param0.s9),
                  cospi(param0.sa), cospi(param0.sb),
                  cospi(param0.sc), cospi(param0.sd),
                  cospi(param0.se), cospi(param0.sf)); }


//gentype erfc (gentype)

INLINE_OVERLOADABLE float2 erfc (float2 param0)
{return (float2)(erfc(param0.s0), erfc(param0.s1)); }

INLINE_OVERLOADABLE float3 erfc (float3 param0)
{return (float3)(erfc(param0.s0), erfc(param0.s1),
                 erfc(param0.s2)); }

INLINE_OVERLOADABLE float4 erfc (float4 param0)
{return (float4)(erfc(param0.s0), erfc(param0.s1),
                 erfc(param0.s2), erfc(param0.s3)); }

INLINE_OVERLOADABLE float8 erfc (float8 param0)
{return (float8)(erfc(param0.s0), erfc(param0.s1),
                 erfc(param0.s2), erfc(param0.s3),
                 erfc(param0.s4), erfc(param0.s5),
                 erfc(param0.s6), erfc(param0.s7)); }

INLINE_OVERLOADABLE float16 erfc (float16 param0)
{return (float16)(erfc(param0.s0),  erfc(param0.s1),
                  erfc(param0.s2),  erfc(param0.s3),
                  erfc(param0.s4),  erfc(param0.s5),
                  erfc(param0.s6),  erfc(param0.s7),
                  erfc(param0.s8),  erfc(param0.s9),
                  erfc(param0.sa), erfc(param0.sb),
                  erfc(param0.sc), erfc(param0.sd),
                  erfc(param0.se), erfc(param0.sf)); }


//gentype erf (gentype)

INLINE_OVERLOADABLE float2 erf (float2 param0)
{return (float2)(erf(param0.s0), erf(param0.s1)); }

INLINE_OVERLOADABLE float3 erf (float3 param0)
{return (float3)(erf(param0.s0), erf(param0.s1),
                 erf(param0.s2)); }

INLINE_OVERLOADABLE float4 erf (float4 param0)
{return (float4)(erf(param0.s0), erf(param0.s1),
                 erf(param0.s2), erf(param0.s3)); }

INLINE_OVERLOADABLE float8 erf (float8 param0)
{return (float8)(erf(param0.s0), erf(param0.s1),
                 erf(param0.s2), erf(param0.s3),
                 erf(param0.s4), erf(param0.s5),
                 erf(param0.s6), erf(param0.s7)); }

INLINE_OVERLOADABLE float16 erf (float16 param0)
{return (float16)(erf(param0.s0),  erf(param0.s1),
                  erf(param0.s2),  erf(param0.s3),
                  erf(param0.s4),  erf(param0.s5),
                  erf(param0.s6),  erf(param0.s7),
                  erf(param0.s8),  erf(param0.s9),
                  erf(param0.sa), erf(param0.sb),
                  erf(param0.sc), erf(param0.sd),
                  erf(param0.se), erf(param0.sf)); }


//gentype exp (gentype x)

INLINE_OVERLOADABLE float2 exp (float2 param0)
{return (float2)(exp(param0.s0), exp(param0.s1)); }

INLINE_OVERLOADABLE float3 exp (float3 param0)
{return (float3)(exp(param0.s0), exp(param0.s1),
                 exp(param0.s2)); }

INLINE_OVERLOADABLE float4 exp (float4 param0)
{return (float4)(exp(param0.s0), exp(param0.s1),
                 exp(param0.s2), exp(param0.s3)); }

INLINE_OVERLOADABLE float8 exp (float8 param0)
{return (float8)(exp(param0.s0), exp(param0.s1),
                 exp(param0.s2), exp(param0.s3),
                 exp(param0.s4), exp(param0.s5),
                 exp(param0.s6), exp(param0.s7)); }

INLINE_OVERLOADABLE float16 exp (float16 param0)
{return (float16)(exp(param0.s0),  exp(param0.s1),
                  exp(param0.s2),  exp(param0.s3),
                  exp(param0.s4),  exp(param0.s5),
                  exp(param0.s6),  exp(param0.s7),
                  exp(param0.s8),  exp(param0.s9),
                  exp(param0.sa), exp(param0.sb),
                  exp(param0.sc), exp(param0.sd),
                  exp(param0.se), exp(param0.sf)); }


//gentype exp2 (gentype)

INLINE_OVERLOADABLE float2 exp2 (float2 param0)
{return (float2)(exp2(param0.s0), exp2(param0.s1)); }

INLINE_OVERLOADABLE float3 exp2 (float3 param0)
{return (float3)(exp2(param0.s0), exp2(param0.s1),
                 exp2(param0.s2)); }

INLINE_OVERLOADABLE float4 exp2 (float4 param0)
{return (float4)(exp2(param0.s0), exp2(param0.s1),
                 exp2(param0.s2), exp2(param0.s3)); }

INLINE_OVERLOADABLE float8 exp2 (float8 param0)
{return (float8)(exp2(param0.s0), exp2(param0.s1),
                 exp2(param0.s2), exp2(param0.s3),
                 exp2(param0.s4), exp2(param0.s5),
                 exp2(param0.s6), exp2(param0.s7)); }

INLINE_OVERLOADABLE float16 exp2 (float16 param0)
{return (float16)(exp2(param0.s0),  exp2(param0.s1),
                  exp2(param0.s2),  exp2(param0.s3),
                  exp2(param0.s4),  exp2(param0.s5),
                  exp2(param0.s6),  exp2(param0.s7),
                  exp2(param0.s8),  exp2(param0.s9),
                  exp2(param0.sa), exp2(param0.sb),
                  exp2(param0.sc), exp2(param0.sd),
                  exp2(param0.se), exp2(param0.sf)); }


//gentype exp10 (gentype)

INLINE_OVERLOADABLE float2 exp10 (float2 param0)
{return (float2)(exp10(param0.s0), exp10(param0.s1)); }

INLINE_OVERLOADABLE float3 exp10 (float3 param0)
{return (float3)(exp10(param0.s0), exp10(param0.s1),
                 exp10(param0.s2)); }

INLINE_OVERLOADABLE float4 exp10 (float4 param0)
{return (float4)(exp10(param0.s0), exp10(param0.s1),
                 exp10(param0.s2), exp10(param0.s3)); }

INLINE_OVERLOADABLE float8 exp10 (float8 param0)
{return (float8)(exp10(param0.s0), exp10(param0.s1),
                 exp10(param0.s2), exp10(param0.s3),
                 exp10(param0.s4), exp10(param0.s5),
                 exp10(param0.s6), exp10(param0.s7)); }

INLINE_OVERLOADABLE float16 exp10 (float16 param0)
{return (float16)(exp10(param0.s0),  exp10(param0.s1),
                  exp10(param0.s2),  exp10(param0.s3),
                  exp10(param0.s4),  exp10(param0.s5),
                  exp10(param0.s6),  exp10(param0.s7),
                  exp10(param0.s8),  exp10(param0.s9),
                  exp10(param0.sa), exp10(param0.sb),
                  exp10(param0.sc), exp10(param0.sd),
                  exp10(param0.se), exp10(param0.sf)); }


//gentype expm1 (gentype x)

INLINE_OVERLOADABLE float2 expm1 (float2 param0)
{return (float2)(expm1(param0.s0), expm1(param0.s1)); }

INLINE_OVERLOADABLE float3 expm1 (float3 param0)
{return (float3)(expm1(param0.s0), expm1(param0.s1),
                 expm1(param0.s2)); }

INLINE_OVERLOADABLE float4 expm1 (float4 param0)
{return (float4)(expm1(param0.s0), expm1(param0.s1),
                 expm1(param0.s2), expm1(param0.s3)); }

INLINE_OVERLOADABLE float8 expm1 (float8 param0)
{return (float8)(expm1(param0.s0), expm1(param0.s1),
                 expm1(param0.s2), expm1(param0.s3),
                 expm1(param0.s4), expm1(param0.s5),
                 expm1(param0.s6), expm1(param0.s7)); }

INLINE_OVERLOADABLE float16 expm1 (float16 param0)
{return (float16)(expm1(param0.s0),  expm1(param0.s1),
                  expm1(param0.s2),  expm1(param0.s3),
                  expm1(param0.s4),  expm1(param0.s5),
                  expm1(param0.s6),  expm1(param0.s7),
                  expm1(param0.s8),  expm1(param0.s9),
                  expm1(param0.sa), expm1(param0.sb),
                  expm1(param0.sc), expm1(param0.sd),
                  expm1(param0.se), expm1(param0.sf)); }


//gentype fabs (gentype)

INLINE_OVERLOADABLE float2 fabs (float2 param0)
{return (float2)(fabs(param0.s0), fabs(param0.s1)); }

INLINE_OVERLOADABLE float3 fabs (float3 param0)
{return (float3)(fabs(param0.s0), fabs(param0.s1),
                 fabs(param0.s2)); }

INLINE_OVERLOADABLE float4 fabs (float4 param0)
{return (float4)(fabs(param0.s0), fabs(param0.s1),
                 fabs(param0.s2), fabs(param0.s3)); }

INLINE_OVERLOADABLE float8 fabs (float8 param0)
{return (float8)(fabs(param0.s0), fabs(param0.s1),
                 fabs(param0.s2), fabs(param0.s3),
                 fabs(param0.s4), fabs(param0.s5),
                 fabs(param0.s6), fabs(param0.s7)); }

INLINE_OVERLOADABLE float16 fabs (float16 param0)
{return (float16)(fabs(param0.s0),  fabs(param0.s1),
                  fabs(param0.s2),  fabs(param0.s3),
                  fabs(param0.s4),  fabs(param0.s5),
                  fabs(param0.s6),  fabs(param0.s7),
                  fabs(param0.s8),  fabs(param0.s9),
                  fabs(param0.sa), fabs(param0.sb),
                  fabs(param0.sc), fabs(param0.sd),
                  fabs(param0.se), fabs(param0.sf)); }


//gentype fdim (gentype x, gentype y)

INLINE_OVERLOADABLE float2 fdim (float2 param0, float2 param1)
{return (float2)(fdim(param0.s0, param1.s0), fdim(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 fdim (float3 param0, float3 param1)
{return (float3)(fdim(param0.s0, param1.s0), fdim(param0.s1, param1.s1),
                 fdim(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 fdim (float4 param0, float4 param1)
{return (float4)(fdim(param0.s0, param1.s0), fdim(param0.s1, param1.s1),
                 fdim(param0.s2, param1.s2), fdim(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 fdim (float8 param0, float8 param1)
{return (float8)(fdim(param0.s0, param1.s0), fdim(param0.s1, param1.s1),
                 fdim(param0.s2, param1.s2), fdim(param0.s3, param1.s3),
                 fdim(param0.s4, param1.s4), fdim(param0.s5, param1.s5),
                 fdim(param0.s6, param1.s6), fdim(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 fdim (float16 param0, float16 param1)
{return (float16)(fdim(param0.s0,  param1.s0),  fdim(param0.s1,  param1.s1),
                  fdim(param0.s2,  param1.s2),  fdim(param0.s3,  param1.s3),
                  fdim(param0.s4,  param1.s4),  fdim(param0.s5,  param1.s5),
                  fdim(param0.s6,  param1.s6),  fdim(param0.s7,  param1.s7),
                  fdim(param0.s8,  param1.s8),  fdim(param0.s9,  param1.s9),
                  fdim(param0.sa, param1.sa), fdim(param0.sb, param1.sb),
                  fdim(param0.sc, param1.sc), fdim(param0.sd, param1.sd),
                  fdim(param0.se, param1.se), fdim(param0.sf, param1.sf)); }


//gentype floor (gentype)

INLINE_OVERLOADABLE float2 floor (float2 param0)
{return (float2)(floor(param0.s0), floor(param0.s1)); }

INLINE_OVERLOADABLE float3 floor (float3 param0)
{return (float3)(floor(param0.s0), floor(param0.s1),
                 floor(param0.s2)); }

INLINE_OVERLOADABLE float4 floor (float4 param0)
{return (float4)(floor(param0.s0), floor(param0.s1),
                 floor(param0.s2), floor(param0.s3)); }

INLINE_OVERLOADABLE float8 floor (float8 param0)
{return (float8)(floor(param0.s0), floor(param0.s1),
                 floor(param0.s2), floor(param0.s3),
                 floor(param0.s4), floor(param0.s5),
                 floor(param0.s6), floor(param0.s7)); }

INLINE_OVERLOADABLE float16 floor (float16 param0)
{return (float16)(floor(param0.s0),  floor(param0.s1),
                  floor(param0.s2),  floor(param0.s3),
                  floor(param0.s4),  floor(param0.s5),
                  floor(param0.s6),  floor(param0.s7),
                  floor(param0.s8),  floor(param0.s9),
                  floor(param0.sa), floor(param0.sb),
                  floor(param0.sc), floor(param0.sd),
                  floor(param0.se), floor(param0.sf)); }


//gentype fmax (gentype x, gentype y)

INLINE_OVERLOADABLE float2 fmax (float2 param0, float2 param1)
{return (float2)(fmax(param0.s0, param1.s0), fmax(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 fmax (float3 param0, float3 param1)
{return (float3)(fmax(param0.s0, param1.s0), fmax(param0.s1, param1.s1),
                 fmax(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 fmax (float4 param0, float4 param1)
{return (float4)(fmax(param0.s0, param1.s0), fmax(param0.s1, param1.s1),
                 fmax(param0.s2, param1.s2), fmax(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 fmax (float8 param0, float8 param1)
{return (float8)(fmax(param0.s0, param1.s0), fmax(param0.s1, param1.s1),
                 fmax(param0.s2, param1.s2), fmax(param0.s3, param1.s3),
                 fmax(param0.s4, param1.s4), fmax(param0.s5, param1.s5),
                 fmax(param0.s6, param1.s6), fmax(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 fmax (float16 param0, float16 param1)
{return (float16)(fmax(param0.s0,  param1.s0),  fmax(param0.s1,  param1.s1),
                  fmax(param0.s2,  param1.s2),  fmax(param0.s3,  param1.s3),
                  fmax(param0.s4,  param1.s4),  fmax(param0.s5,  param1.s5),
                  fmax(param0.s6,  param1.s6),  fmax(param0.s7,  param1.s7),
                  fmax(param0.s8,  param1.s8),  fmax(param0.s9,  param1.s9),
                  fmax(param0.sa, param1.sa), fmax(param0.sb, param1.sb),
                  fmax(param0.sc, param1.sc), fmax(param0.sd, param1.sd),
                  fmax(param0.se, param1.se), fmax(param0.sf, param1.sf)); }


//gentypef fmax (gentypef x, float y)

INLINE_OVERLOADABLE float2 fmax (float2 param0, float param1)
{return (float2)(fmax(param0.s0, param1), fmax(param0.s1, param1)); }

INLINE_OVERLOADABLE float3 fmax (float3 param0, float param1)
{return (float3)(fmax(param0.s0, param1), fmax(param0.s1, param1),
                 fmax(param0.s2, param1)); }

INLINE_OVERLOADABLE float4 fmax (float4 param0, float param1)
{return (float4)(fmax(param0.s0, param1), fmax(param0.s1, param1),
                 fmax(param0.s2, param1), fmax(param0.s3, param1)); }

INLINE_OVERLOADABLE float8 fmax (float8 param0, float param1)
{return (float8)(fmax(param0.s0, param1), fmax(param0.s1, param1),
                 fmax(param0.s2, param1), fmax(param0.s3, param1),
                 fmax(param0.s4, param1), fmax(param0.s5, param1),
                 fmax(param0.s6, param1), fmax(param0.s7, param1)); }

INLINE_OVERLOADABLE float16 fmax (float16 param0, float param1)
{return (float16)(fmax(param0.s0,  param1),  fmax(param0.s1,  param1),
                  fmax(param0.s2,  param1),  fmax(param0.s3,  param1),
                  fmax(param0.s4,  param1),  fmax(param0.s5,  param1),
                  fmax(param0.s6,  param1),  fmax(param0.s7,  param1),
                  fmax(param0.s8,  param1),  fmax(param0.s9,  param1),
                  fmax(param0.sa, param1), fmax(param0.sb, param1),
                  fmax(param0.sc, param1), fmax(param0.sd, param1),
                  fmax(param0.se, param1), fmax(param0.sf, param1)); }


//gentyped fmax (gentyped x, double y)


//gentype fmin (gentype x, gentype y)

INLINE_OVERLOADABLE float2 fmin (float2 param0, float2 param1)
{return (float2)(fmin(param0.s0, param1.s0), fmin(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 fmin (float3 param0, float3 param1)
{return (float3)(fmin(param0.s0, param1.s0), fmin(param0.s1, param1.s1),
                 fmin(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 fmin (float4 param0, float4 param1)
{return (float4)(fmin(param0.s0, param1.s0), fmin(param0.s1, param1.s1),
                 fmin(param0.s2, param1.s2), fmin(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 fmin (float8 param0, float8 param1)
{return (float8)(fmin(param0.s0, param1.s0), fmin(param0.s1, param1.s1),
                 fmin(param0.s2, param1.s2), fmin(param0.s3, param1.s3),
                 fmin(param0.s4, param1.s4), fmin(param0.s5, param1.s5),
                 fmin(param0.s6, param1.s6), fmin(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 fmin (float16 param0, float16 param1)
{return (float16)(fmin(param0.s0,  param1.s0),  fmin(param0.s1,  param1.s1),
                  fmin(param0.s2,  param1.s2),  fmin(param0.s3,  param1.s3),
                  fmin(param0.s4,  param1.s4),  fmin(param0.s5,  param1.s5),
                  fmin(param0.s6,  param1.s6),  fmin(param0.s7,  param1.s7),
                  fmin(param0.s8,  param1.s8),  fmin(param0.s9,  param1.s9),
                  fmin(param0.sa, param1.sa), fmin(param0.sb, param1.sb),
                  fmin(param0.sc, param1.sc), fmin(param0.sd, param1.sd),
                  fmin(param0.se, param1.se), fmin(param0.sf, param1.sf)); }


//gentypef fmin (gentypef x, float y)

INLINE_OVERLOADABLE float2 fmin (float2 param0, float param1)
{return (float2)(fmin(param0.s0, param1), fmin(param0.s1, param1)); }

INLINE_OVERLOADABLE float3 fmin (float3 param0, float param1)
{return (float3)(fmin(param0.s0, param1), fmin(param0.s1, param1),
                 fmin(param0.s2, param1)); }

INLINE_OVERLOADABLE float4 fmin (float4 param0, float param1)
{return (float4)(fmin(param0.s0, param1), fmin(param0.s1, param1),
                 fmin(param0.s2, param1), fmin(param0.s3, param1)); }

INLINE_OVERLOADABLE float8 fmin (float8 param0, float param1)
{return (float8)(fmin(param0.s0, param1), fmin(param0.s1, param1),
                 fmin(param0.s2, param1), fmin(param0.s3, param1),
                 fmin(param0.s4, param1), fmin(param0.s5, param1),
                 fmin(param0.s6, param1), fmin(param0.s7, param1)); }

INLINE_OVERLOADABLE float16 fmin (float16 param0, float param1)
{return (float16)(fmin(param0.s0,  param1),  fmin(param0.s1,  param1),
                  fmin(param0.s2,  param1),  fmin(param0.s3,  param1),
                  fmin(param0.s4,  param1),  fmin(param0.s5,  param1),
                  fmin(param0.s6,  param1),  fmin(param0.s7,  param1),
                  fmin(param0.s8,  param1),  fmin(param0.s9,  param1),
                  fmin(param0.sa, param1), fmin(param0.sb, param1),
                  fmin(param0.sc, param1), fmin(param0.sd, param1),
                  fmin(param0.se, param1), fmin(param0.sf, param1)); }


//gentyped fmin (gentyped x, double y)


//gentype fmod (gentype x, gentype y)

INLINE_OVERLOADABLE float2 fmod (float2 param0, float2 param1)
{return (float2)(fmod(param0.s0, param1.s0), fmod(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 fmod (float3 param0, float3 param1)
{return (float3)(fmod(param0.s0, param1.s0), fmod(param0.s1, param1.s1),
                 fmod(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 fmod (float4 param0, float4 param1)
{return (float4)(fmod(param0.s0, param1.s0), fmod(param0.s1, param1.s1),
                 fmod(param0.s2, param1.s2), fmod(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 fmod (float8 param0, float8 param1)
{return (float8)(fmod(param0.s0, param1.s0), fmod(param0.s1, param1.s1),
                 fmod(param0.s2, param1.s2), fmod(param0.s3, param1.s3),
                 fmod(param0.s4, param1.s4), fmod(param0.s5, param1.s5),
                 fmod(param0.s6, param1.s6), fmod(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 fmod (float16 param0, float16 param1)
{return (float16)(fmod(param0.s0,  param1.s0),  fmod(param0.s1,  param1.s1),
                  fmod(param0.s2,  param1.s2),  fmod(param0.s3,  param1.s3),
                  fmod(param0.s4,  param1.s4),  fmod(param0.s5,  param1.s5),
                  fmod(param0.s6,  param1.s6),  fmod(param0.s7,  param1.s7),
                  fmod(param0.s8,  param1.s8),  fmod(param0.s9,  param1.s9),
                  fmod(param0.sa, param1.sa), fmod(param0.sb, param1.sb),
                  fmod(param0.sc, param1.sc), fmod(param0.sd, param1.sd),
                  fmod(param0.se, param1.se), fmod(param0.sf, param1.sf)); }


//gentype hypot (gentype x, gentype y)

INLINE_OVERLOADABLE float2 hypot (float2 param0, float2 param1)
{return (float2)(hypot(param0.s0, param1.s0), hypot(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 hypot (float3 param0, float3 param1)
{return (float3)(hypot(param0.s0, param1.s0), hypot(param0.s1, param1.s1),
                 hypot(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 hypot (float4 param0, float4 param1)
{return (float4)(hypot(param0.s0, param1.s0), hypot(param0.s1, param1.s1),
                 hypot(param0.s2, param1.s2), hypot(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 hypot (float8 param0, float8 param1)
{return (float8)(hypot(param0.s0, param1.s0), hypot(param0.s1, param1.s1),
                 hypot(param0.s2, param1.s2), hypot(param0.s3, param1.s3),
                 hypot(param0.s4, param1.s4), hypot(param0.s5, param1.s5),
                 hypot(param0.s6, param1.s6), hypot(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 hypot (float16 param0, float16 param1)
{return (float16)(hypot(param0.s0,  param1.s0),  hypot(param0.s1,  param1.s1),
                  hypot(param0.s2,  param1.s2),  hypot(param0.s3,  param1.s3),
                  hypot(param0.s4,  param1.s4),  hypot(param0.s5,  param1.s5),
                  hypot(param0.s6,  param1.s6),  hypot(param0.s7,  param1.s7),
                  hypot(param0.s8,  param1.s8),  hypot(param0.s9,  param1.s9),
                  hypot(param0.sa, param1.sa), hypot(param0.sb, param1.sb),
                  hypot(param0.sc, param1.sc), hypot(param0.sd, param1.sd),
                  hypot(param0.se, param1.se), hypot(param0.sf, param1.sf)); }


//intn ilogb (floatn x)

INLINE_OVERLOADABLE int2 ilogb (float2 param0)
{return (int2)(ilogb(param0.s0), ilogb(param0.s1)); }

INLINE_OVERLOADABLE int3 ilogb (float3 param0)
{return (int3)(ilogb(param0.s0), ilogb(param0.s1),
               ilogb(param0.s2)); }

INLINE_OVERLOADABLE int4 ilogb (float4 param0)
{return (int4)(ilogb(param0.s0), ilogb(param0.s1),
               ilogb(param0.s2), ilogb(param0.s3)); }

INLINE_OVERLOADABLE int8 ilogb (float8 param0)
{return (int8)(ilogb(param0.s0), ilogb(param0.s1),
               ilogb(param0.s2), ilogb(param0.s3),
               ilogb(param0.s4), ilogb(param0.s5),
               ilogb(param0.s6), ilogb(param0.s7)); }

INLINE_OVERLOADABLE int16 ilogb (float16 param0)
{return (int16)(ilogb(param0.s0),  ilogb(param0.s1),
                ilogb(param0.s2),  ilogb(param0.s3),
                ilogb(param0.s4),  ilogb(param0.s5),
                ilogb(param0.s6),  ilogb(param0.s7),
                ilogb(param0.s8),  ilogb(param0.s9),
                ilogb(param0.sa), ilogb(param0.sb),
                ilogb(param0.sc), ilogb(param0.sd),
                ilogb(param0.se), ilogb(param0.sf)); }


//int ilogb (float x)


//intn ilogb (doublen x)


//int ilogb (double x)


//floatn ldexp (floatn x, intn k)

INLINE_OVERLOADABLE float2 ldexp (float2 param0, int2 param1)
{return (float2)(ldexp(param0.s0, param1.s0), ldexp(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 ldexp (float3 param0, int3 param1)
{return (float3)(ldexp(param0.s0, param1.s0), ldexp(param0.s1, param1.s1),
                 ldexp(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 ldexp (float4 param0, int4 param1)
{return (float4)(ldexp(param0.s0, param1.s0), ldexp(param0.s1, param1.s1),
                 ldexp(param0.s2, param1.s2), ldexp(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 ldexp (float8 param0, int8 param1)
{return (float8)(ldexp(param0.s0, param1.s0), ldexp(param0.s1, param1.s1),
                 ldexp(param0.s2, param1.s2), ldexp(param0.s3, param1.s3),
                 ldexp(param0.s4, param1.s4), ldexp(param0.s5, param1.s5),
                 ldexp(param0.s6, param1.s6), ldexp(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 ldexp (float16 param0, int16 param1)
{return (float16)(ldexp(param0.s0,  param1.s0),  ldexp(param0.s1,  param1.s1),
                  ldexp(param0.s2,  param1.s2),  ldexp(param0.s3,  param1.s3),
                  ldexp(param0.s4,  param1.s4),  ldexp(param0.s5,  param1.s5),
                  ldexp(param0.s6,  param1.s6),  ldexp(param0.s7,  param1.s7),
                  ldexp(param0.s8,  param1.s8),  ldexp(param0.s9,  param1.s9),
                  ldexp(param0.sa, param1.sa), ldexp(param0.sb, param1.sb),
                  ldexp(param0.sc, param1.sc), ldexp(param0.sd, param1.sd),
                  ldexp(param0.se, param1.se), ldexp(param0.sf, param1.sf)); }


//floatn ldexp (floatn x, int k)

INLINE_OVERLOADABLE float2 ldexp (float2 param0, int param1)
{return (float2)(ldexp(param0.s0, param1), ldexp(param0.s1, param1)); }

INLINE_OVERLOADABLE float3 ldexp (float3 param0, int param1)
{return (float3)(ldexp(param0.s0, param1), ldexp(param0.s1, param1),
                 ldexp(param0.s2, param1)); }

INLINE_OVERLOADABLE float4 ldexp (float4 param0, int param1)
{return (float4)(ldexp(param0.s0, param1), ldexp(param0.s1, param1),
                 ldexp(param0.s2, param1), ldexp(param0.s3, param1)); }

INLINE_OVERLOADABLE float8 ldexp (float8 param0, int param1)
{return (float8)(ldexp(param0.s0, param1), ldexp(param0.s1, param1),
                 ldexp(param0.s2, param1), ldexp(param0.s3, param1),
                 ldexp(param0.s4, param1), ldexp(param0.s5, param1),
                 ldexp(param0.s6, param1), ldexp(param0.s7, param1)); }

INLINE_OVERLOADABLE float16 ldexp (float16 param0, int param1)
{return (float16)(ldexp(param0.s0,  param1),  ldexp(param0.s1,  param1),
                  ldexp(param0.s2,  param1),  ldexp(param0.s3,  param1),
                  ldexp(param0.s4,  param1),  ldexp(param0.s5,  param1),
                  ldexp(param0.s6,  param1),  ldexp(param0.s7,  param1),
                  ldexp(param0.s8,  param1),  ldexp(param0.s9,  param1),
                  ldexp(param0.sa, param1), ldexp(param0.sb, param1),
                  ldexp(param0.sc, param1), ldexp(param0.sd, param1),
                  ldexp(param0.se, param1), ldexp(param0.sf, param1)); }


//float ldexp (float x, int k)


//doublen ldexp (doublen x, intn k)


//doublen ldexp (doublen x, int k)


//double ldexp (double x, int k)


//gentype log (gentype)

INLINE_OVERLOADABLE float2 log (float2 param0)
{return (float2)(log(param0.s0), log(param0.s1)); }

INLINE_OVERLOADABLE float3 log (float3 param0)
{return (float3)(log(param0.s0), log(param0.s1),
                 log(param0.s2)); }

INLINE_OVERLOADABLE float4 log (float4 param0)
{return (float4)(log(param0.s0), log(param0.s1),
                 log(param0.s2), log(param0.s3)); }

INLINE_OVERLOADABLE float8 log (float8 param0)
{return (float8)(log(param0.s0), log(param0.s1),
                 log(param0.s2), log(param0.s3),
                 log(param0.s4), log(param0.s5),
                 log(param0.s6), log(param0.s7)); }

INLINE_OVERLOADABLE float16 log (float16 param0)
{return (float16)(log(param0.s0),  log(param0.s1),
                  log(param0.s2),  log(param0.s3),
                  log(param0.s4),  log(param0.s5),
                  log(param0.s6),  log(param0.s7),
                  log(param0.s8),  log(param0.s9),
                  log(param0.sa), log(param0.sb),
                  log(param0.sc), log(param0.sd),
                  log(param0.se), log(param0.sf)); }


//gentype log2 (gentype)

INLINE_OVERLOADABLE float2 log2 (float2 param0)
{return (float2)(log2(param0.s0), log2(param0.s1)); }

INLINE_OVERLOADABLE float3 log2 (float3 param0)
{return (float3)(log2(param0.s0), log2(param0.s1),
                 log2(param0.s2)); }

INLINE_OVERLOADABLE float4 log2 (float4 param0)
{return (float4)(log2(param0.s0), log2(param0.s1),
                 log2(param0.s2), log2(param0.s3)); }

INLINE_OVERLOADABLE float8 log2 (float8 param0)
{return (float8)(log2(param0.s0), log2(param0.s1),
                 log2(param0.s2), log2(param0.s3),
                 log2(param0.s4), log2(param0.s5),
                 log2(param0.s6), log2(param0.s7)); }

INLINE_OVERLOADABLE float16 log2 (float16 param0)
{return (float16)(log2(param0.s0),  log2(param0.s1),
                  log2(param0.s2),  log2(param0.s3),
                  log2(param0.s4),  log2(param0.s5),
                  log2(param0.s6),  log2(param0.s7),
                  log2(param0.s8),  log2(param0.s9),
                  log2(param0.sa), log2(param0.sb),
                  log2(param0.sc), log2(param0.sd),
                  log2(param0.se), log2(param0.sf)); }


//gentype log10 (gentype)

INLINE_OVERLOADABLE float2 log10 (float2 param0)
{return (float2)(log10(param0.s0), log10(param0.s1)); }

INLINE_OVERLOADABLE float3 log10 (float3 param0)
{return (float3)(log10(param0.s0), log10(param0.s1),
                 log10(param0.s2)); }

INLINE_OVERLOADABLE float4 log10 (float4 param0)
{return (float4)(log10(param0.s0), log10(param0.s1),
                 log10(param0.s2), log10(param0.s3)); }

INLINE_OVERLOADABLE float8 log10 (float8 param0)
{return (float8)(log10(param0.s0), log10(param0.s1),
                 log10(param0.s2), log10(param0.s3),
                 log10(param0.s4), log10(param0.s5),
                 log10(param0.s6), log10(param0.s7)); }

INLINE_OVERLOADABLE float16 log10 (float16 param0)
{return (float16)(log10(param0.s0),  log10(param0.s1),
                  log10(param0.s2),  log10(param0.s3),
                  log10(param0.s4),  log10(param0.s5),
                  log10(param0.s6),  log10(param0.s7),
                  log10(param0.s8),  log10(param0.s9),
                  log10(param0.sa), log10(param0.sb),
                  log10(param0.sc), log10(param0.sd),
                  log10(param0.se), log10(param0.sf)); }


//gentype log1p (gentype x)

INLINE_OVERLOADABLE float2 log1p (float2 param0)
{return (float2)(log1p(param0.s0), log1p(param0.s1)); }

INLINE_OVERLOADABLE float3 log1p (float3 param0)
{return (float3)(log1p(param0.s0), log1p(param0.s1),
                 log1p(param0.s2)); }

INLINE_OVERLOADABLE float4 log1p (float4 param0)
{return (float4)(log1p(param0.s0), log1p(param0.s1),
                 log1p(param0.s2), log1p(param0.s3)); }

INLINE_OVERLOADABLE float8 log1p (float8 param0)
{return (float8)(log1p(param0.s0), log1p(param0.s1),
                 log1p(param0.s2), log1p(param0.s3),
                 log1p(param0.s4), log1p(param0.s5),
                 log1p(param0.s6), log1p(param0.s7)); }

INLINE_OVERLOADABLE float16 log1p (float16 param0)
{return (float16)(log1p(param0.s0),  log1p(param0.s1),
                  log1p(param0.s2),  log1p(param0.s3),
                  log1p(param0.s4),  log1p(param0.s5),
                  log1p(param0.s6),  log1p(param0.s7),
                  log1p(param0.s8),  log1p(param0.s9),
                  log1p(param0.sa), log1p(param0.sb),
                  log1p(param0.sc), log1p(param0.sd),
                  log1p(param0.se), log1p(param0.sf)); }


//gentype logb (gentype x)

INLINE_OVERLOADABLE float2 logb (float2 param0)
{return (float2)(logb(param0.s0), logb(param0.s1)); }

INLINE_OVERLOADABLE float3 logb (float3 param0)
{return (float3)(logb(param0.s0), logb(param0.s1),
                 logb(param0.s2)); }

INLINE_OVERLOADABLE float4 logb (float4 param0)
{return (float4)(logb(param0.s0), logb(param0.s1),
                 logb(param0.s2), logb(param0.s3)); }

INLINE_OVERLOADABLE float8 logb (float8 param0)
{return (float8)(logb(param0.s0), logb(param0.s1),
                 logb(param0.s2), logb(param0.s3),
                 logb(param0.s4), logb(param0.s5),
                 logb(param0.s6), logb(param0.s7)); }

INLINE_OVERLOADABLE float16 logb (float16 param0)
{return (float16)(logb(param0.s0),  logb(param0.s1),
                  logb(param0.s2),  logb(param0.s3),
                  logb(param0.s4),  logb(param0.s5),
                  logb(param0.s6),  logb(param0.s7),
                  logb(param0.s8),  logb(param0.s9),
                  logb(param0.sa), logb(param0.sb),
                  logb(param0.sc), logb(param0.sd),
                  logb(param0.se), logb(param0.sf)); }


//gentype mad (gentype a, gentype b, gentype c)

INLINE_OVERLOADABLE float2 mad (float2 param0, float2 param1, float2 param2)
{return (float2)(mad(param0.s0, param1.s0, param2.s0), mad(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE float3 mad (float3 param0, float3 param1, float3 param2)
{return (float3)(mad(param0.s0, param1.s0, param2.s0), mad(param0.s1, param1.s1, param2.s1),
                 mad(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE float4 mad (float4 param0, float4 param1, float4 param2)
{return (float4)(mad(param0.s0, param1.s0, param2.s0), mad(param0.s1, param1.s1, param2.s1),
                 mad(param0.s2, param1.s2, param2.s2), mad(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE float8 mad (float8 param0, float8 param1, float8 param2)
{return (float8)(mad(param0.s0, param1.s0, param2.s0), mad(param0.s1, param1.s1, param2.s1),
                 mad(param0.s2, param1.s2, param2.s2), mad(param0.s3, param1.s3, param2.s3),
                 mad(param0.s4, param1.s4, param2.s4), mad(param0.s5, param1.s5, param2.s5),
                 mad(param0.s6, param1.s6, param2.s6), mad(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE float16 mad (float16 param0, float16 param1, float16 param2)
{return (float16)(mad(param0.s0,  param1.s0,  param2.s0),  mad(param0.s1,  param1.s1,  param2.s1),
                  mad(param0.s2,  param1.s2,  param2.s2),  mad(param0.s3,  param1.s3,  param2.s3),
                  mad(param0.s4,  param1.s4,  param2.s4),  mad(param0.s5,  param1.s5,  param2.s5),
                  mad(param0.s6,  param1.s6,  param2.s6),  mad(param0.s7,  param1.s7,  param2.s7),
                  mad(param0.s8,  param1.s8,  param2.s8),  mad(param0.s9,  param1.s9,  param2.s9),
                  mad(param0.sa, param1.sa, param2.sa), mad(param0.sb, param1.sb, param2.sb),
                  mad(param0.sc, param1.sc, param2.sc), mad(param0.sd, param1.sd, param2.sd),
                  mad(param0.se, param1.se, param2.se), mad(param0.sf, param1.sf, param2.sf)); }


//gentype maxmag (gentype x, gentype y)

INLINE_OVERLOADABLE float2 maxmag (float2 param0, float2 param1)
{return (float2)(maxmag(param0.s0, param1.s0), maxmag(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 maxmag (float3 param0, float3 param1)
{return (float3)(maxmag(param0.s0, param1.s0), maxmag(param0.s1, param1.s1),
                 maxmag(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 maxmag (float4 param0, float4 param1)
{return (float4)(maxmag(param0.s0, param1.s0), maxmag(param0.s1, param1.s1),
                 maxmag(param0.s2, param1.s2), maxmag(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 maxmag (float8 param0, float8 param1)
{return (float8)(maxmag(param0.s0, param1.s0), maxmag(param0.s1, param1.s1),
                 maxmag(param0.s2, param1.s2), maxmag(param0.s3, param1.s3),
                 maxmag(param0.s4, param1.s4), maxmag(param0.s5, param1.s5),
                 maxmag(param0.s6, param1.s6), maxmag(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 maxmag (float16 param0, float16 param1)
{return (float16)(maxmag(param0.s0,  param1.s0),  maxmag(param0.s1,  param1.s1),
                  maxmag(param0.s2,  param1.s2),  maxmag(param0.s3,  param1.s3),
                  maxmag(param0.s4,  param1.s4),  maxmag(param0.s5,  param1.s5),
                  maxmag(param0.s6,  param1.s6),  maxmag(param0.s7,  param1.s7),
                  maxmag(param0.s8,  param1.s8),  maxmag(param0.s9,  param1.s9),
                  maxmag(param0.sa, param1.sa), maxmag(param0.sb, param1.sb),
                  maxmag(param0.sc, param1.sc), maxmag(param0.sd, param1.sd),
                  maxmag(param0.se, param1.se), maxmag(param0.sf, param1.sf)); }


//gentype minmag (gentype x, gentype y)

INLINE_OVERLOADABLE float2 minmag (float2 param0, float2 param1)
{return (float2)(minmag(param0.s0, param1.s0), minmag(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 minmag (float3 param0, float3 param1)
{return (float3)(minmag(param0.s0, param1.s0), minmag(param0.s1, param1.s1),
                 minmag(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 minmag (float4 param0, float4 param1)
{return (float4)(minmag(param0.s0, param1.s0), minmag(param0.s1, param1.s1),
                 minmag(param0.s2, param1.s2), minmag(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 minmag (float8 param0, float8 param1)
{return (float8)(minmag(param0.s0, param1.s0), minmag(param0.s1, param1.s1),
                 minmag(param0.s2, param1.s2), minmag(param0.s3, param1.s3),
                 minmag(param0.s4, param1.s4), minmag(param0.s5, param1.s5),
                 minmag(param0.s6, param1.s6), minmag(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 minmag (float16 param0, float16 param1)
{return (float16)(minmag(param0.s0,  param1.s0),  minmag(param0.s1,  param1.s1),
                  minmag(param0.s2,  param1.s2),  minmag(param0.s3,  param1.s3),
                  minmag(param0.s4,  param1.s4),  minmag(param0.s5,  param1.s5),
                  minmag(param0.s6,  param1.s6),  minmag(param0.s7,  param1.s7),
                  minmag(param0.s8,  param1.s8),  minmag(param0.s9,  param1.s9),
                  minmag(param0.sa, param1.sa), minmag(param0.sb, param1.sb),
                  minmag(param0.sc, param1.sc), minmag(param0.sd, param1.sd),
                  minmag(param0.se, param1.se), minmag(param0.sf, param1.sf)); }


//floatn nan (uintn nancode)

INLINE_OVERLOADABLE float2 nan (uint2 param0)
{return (float2)(nan(param0.s0), nan(param0.s1)); }

INLINE_OVERLOADABLE float3 nan (uint3 param0)
{return (float3)(nan(param0.s0), nan(param0.s1),
                 nan(param0.s2)); }

INLINE_OVERLOADABLE float4 nan (uint4 param0)
{return (float4)(nan(param0.s0), nan(param0.s1),
                 nan(param0.s2), nan(param0.s3)); }

INLINE_OVERLOADABLE float8 nan (uint8 param0)
{return (float8)(nan(param0.s0), nan(param0.s1),
                 nan(param0.s2), nan(param0.s3),
                 nan(param0.s4), nan(param0.s5),
                 nan(param0.s6), nan(param0.s7)); }

INLINE_OVERLOADABLE float16 nan (uint16 param0)
{return (float16)(nan(param0.s0),  nan(param0.s1),
                  nan(param0.s2),  nan(param0.s3),
                  nan(param0.s4),  nan(param0.s5),
                  nan(param0.s6),  nan(param0.s7),
                  nan(param0.s8),  nan(param0.s9),
                  nan(param0.sa), nan(param0.sb),
                  nan(param0.sc), nan(param0.sd),
                  nan(param0.se), nan(param0.sf)); }


//float nan (uint nancode)


//doublen nan (ulongn nancode)


//double nan (ulong nancode)


//gentype pow (gentype x, gentype y)

INLINE_OVERLOADABLE float2 pow (float2 param0, float2 param1)
{return (float2)(pow(param0.s0, param1.s0), pow(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 pow (float3 param0, float3 param1)
{return (float3)(pow(param0.s0, param1.s0), pow(param0.s1, param1.s1),
                 pow(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 pow (float4 param0, float4 param1)
{return (float4)(pow(param0.s0, param1.s0), pow(param0.s1, param1.s1),
                 pow(param0.s2, param1.s2), pow(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 pow (float8 param0, float8 param1)
{return (float8)(pow(param0.s0, param1.s0), pow(param0.s1, param1.s1),
                 pow(param0.s2, param1.s2), pow(param0.s3, param1.s3),
                 pow(param0.s4, param1.s4), pow(param0.s5, param1.s5),
                 pow(param0.s6, param1.s6), pow(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 pow (float16 param0, float16 param1)
{return (float16)(pow(param0.s0,  param1.s0),  pow(param0.s1,  param1.s1),
                  pow(param0.s2,  param1.s2),  pow(param0.s3,  param1.s3),
                  pow(param0.s4,  param1.s4),  pow(param0.s5,  param1.s5),
                  pow(param0.s6,  param1.s6),  pow(param0.s7,  param1.s7),
                  pow(param0.s8,  param1.s8),  pow(param0.s9,  param1.s9),
                  pow(param0.sa, param1.sa), pow(param0.sb, param1.sb),
                  pow(param0.sc, param1.sc), pow(param0.sd, param1.sd),
                  pow(param0.se, param1.se), pow(param0.sf, param1.sf)); }


//floatn pown (floatn x, intn y)

INLINE_OVERLOADABLE float2 pown (float2 param0, int2 param1)
{return (float2)(pown(param0.s0, param1.s0), pown(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 pown (float3 param0, int3 param1)
{return (float3)(pown(param0.s0, param1.s0), pown(param0.s1, param1.s1),
                 pown(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 pown (float4 param0, int4 param1)
{return (float4)(pown(param0.s0, param1.s0), pown(param0.s1, param1.s1),
                 pown(param0.s2, param1.s2), pown(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 pown (float8 param0, int8 param1)
{return (float8)(pown(param0.s0, param1.s0), pown(param0.s1, param1.s1),
                 pown(param0.s2, param1.s2), pown(param0.s3, param1.s3),
                 pown(param0.s4, param1.s4), pown(param0.s5, param1.s5),
                 pown(param0.s6, param1.s6), pown(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 pown (float16 param0, int16 param1)
{return (float16)(pown(param0.s0,  param1.s0),  pown(param0.s1,  param1.s1),
                  pown(param0.s2,  param1.s2),  pown(param0.s3,  param1.s3),
                  pown(param0.s4,  param1.s4),  pown(param0.s5,  param1.s5),
                  pown(param0.s6,  param1.s6),  pown(param0.s7,  param1.s7),
                  pown(param0.s8,  param1.s8),  pown(param0.s9,  param1.s9),
                  pown(param0.sa, param1.sa), pown(param0.sb, param1.sb),
                  pown(param0.sc, param1.sc), pown(param0.sd, param1.sd),
                  pown(param0.se, param1.se), pown(param0.sf, param1.sf)); }


//float pown (float x, int y)


//doublen pown (doublen x, intn y)


//double pown (double x, int y)


//gentype remainder (gentype x, gentype y)

INLINE_OVERLOADABLE float2 remainder (float2 param0, float2 param1)
{return (float2)(remainder(param0.s0, param1.s0), remainder(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 remainder (float3 param0, float3 param1)
{return (float3)(remainder(param0.s0, param1.s0), remainder(param0.s1, param1.s1),
                 remainder(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 remainder (float4 param0, float4 param1)
{return (float4)(remainder(param0.s0, param1.s0), remainder(param0.s1, param1.s1),
                 remainder(param0.s2, param1.s2), remainder(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 remainder (float8 param0, float8 param1)
{return (float8)(remainder(param0.s0, param1.s0), remainder(param0.s1, param1.s1),
                 remainder(param0.s2, param1.s2), remainder(param0.s3, param1.s3),
                 remainder(param0.s4, param1.s4), remainder(param0.s5, param1.s5),
                 remainder(param0.s6, param1.s6), remainder(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 remainder (float16 param0, float16 param1)
{return (float16)(remainder(param0.s0,  param1.s0),  remainder(param0.s1,  param1.s1),
                  remainder(param0.s2,  param1.s2),  remainder(param0.s3,  param1.s3),
                  remainder(param0.s4,  param1.s4),  remainder(param0.s5,  param1.s5),
                  remainder(param0.s6,  param1.s6),  remainder(param0.s7,  param1.s7),
                  remainder(param0.s8,  param1.s8),  remainder(param0.s9,  param1.s9),
                  remainder(param0.sa, param1.sa), remainder(param0.sb, param1.sb),
                  remainder(param0.sc, param1.sc), remainder(param0.sd, param1.sd),
                  remainder(param0.se, param1.se), remainder(param0.sf, param1.sf)); }


//gentype rint (gentype)

INLINE_OVERLOADABLE float2 rint (float2 param0)
{return (float2)(rint(param0.s0), rint(param0.s1)); }

INLINE_OVERLOADABLE float3 rint (float3 param0)
{return (float3)(rint(param0.s0), rint(param0.s1),
                 rint(param0.s2)); }

INLINE_OVERLOADABLE float4 rint (float4 param0)
{return (float4)(rint(param0.s0), rint(param0.s1),
                 rint(param0.s2), rint(param0.s3)); }

INLINE_OVERLOADABLE float8 rint (float8 param0)
{return (float8)(rint(param0.s0), rint(param0.s1),
                 rint(param0.s2), rint(param0.s3),
                 rint(param0.s4), rint(param0.s5),
                 rint(param0.s6), rint(param0.s7)); }

INLINE_OVERLOADABLE float16 rint (float16 param0)
{return (float16)(rint(param0.s0),  rint(param0.s1),
                  rint(param0.s2),  rint(param0.s3),
                  rint(param0.s4),  rint(param0.s5),
                  rint(param0.s6),  rint(param0.s7),
                  rint(param0.s8),  rint(param0.s9),
                  rint(param0.sa), rint(param0.sb),
                  rint(param0.sc), rint(param0.sd),
                  rint(param0.se), rint(param0.sf)); }


//floatn rootn (floatn x, intn y)

INLINE_OVERLOADABLE float2 rootn (float2 param0, int2 param1)
{return (float2)(rootn(param0.s0, param1.s0), rootn(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 rootn (float3 param0, int3 param1)
{return (float3)(rootn(param0.s0, param1.s0), rootn(param0.s1, param1.s1),
                 rootn(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 rootn (float4 param0, int4 param1)
{return (float4)(rootn(param0.s0, param1.s0), rootn(param0.s1, param1.s1),
                 rootn(param0.s2, param1.s2), rootn(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 rootn (float8 param0, int8 param1)
{return (float8)(rootn(param0.s0, param1.s0), rootn(param0.s1, param1.s1),
                 rootn(param0.s2, param1.s2), rootn(param0.s3, param1.s3),
                 rootn(param0.s4, param1.s4), rootn(param0.s5, param1.s5),
                 rootn(param0.s6, param1.s6), rootn(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 rootn (float16 param0, int16 param1)
{return (float16)(rootn(param0.s0,  param1.s0),  rootn(param0.s1,  param1.s1),
                  rootn(param0.s2,  param1.s2),  rootn(param0.s3,  param1.s3),
                  rootn(param0.s4,  param1.s4),  rootn(param0.s5,  param1.s5),
                  rootn(param0.s6,  param1.s6),  rootn(param0.s7,  param1.s7),
                  rootn(param0.s8,  param1.s8),  rootn(param0.s9,  param1.s9),
                  rootn(param0.sa, param1.sa), rootn(param0.sb, param1.sb),
                  rootn(param0.sc, param1.sc), rootn(param0.sd, param1.sd),
                  rootn(param0.se, param1.se), rootn(param0.sf, param1.sf)); }


//doublen rootn (doublen x, intn y)


//doublen rootn (double x, int y)


//gentype round (gentype x)

INLINE_OVERLOADABLE float2 round (float2 param0)
{return (float2)(round(param0.s0), round(param0.s1)); }

INLINE_OVERLOADABLE float3 round (float3 param0)
{return (float3)(round(param0.s0), round(param0.s1),
                 round(param0.s2)); }

INLINE_OVERLOADABLE float4 round (float4 param0)
{return (float4)(round(param0.s0), round(param0.s1),
                 round(param0.s2), round(param0.s3)); }

INLINE_OVERLOADABLE float8 round (float8 param0)
{return (float8)(round(param0.s0), round(param0.s1),
                 round(param0.s2), round(param0.s3),
                 round(param0.s4), round(param0.s5),
                 round(param0.s6), round(param0.s7)); }

INLINE_OVERLOADABLE float16 round (float16 param0)
{return (float16)(round(param0.s0),  round(param0.s1),
                  round(param0.s2),  round(param0.s3),
                  round(param0.s4),  round(param0.s5),
                  round(param0.s6),  round(param0.s7),
                  round(param0.s8),  round(param0.s9),
                  round(param0.sa), round(param0.sb),
                  round(param0.sc), round(param0.sd),
                  round(param0.se), round(param0.sf)); }


//gentype rsqrt (gentype)

INLINE_OVERLOADABLE float2 rsqrt (float2 param0)
{return (float2)(rsqrt(param0.s0), rsqrt(param0.s1)); }

INLINE_OVERLOADABLE float3 rsqrt (float3 param0)
{return (float3)(rsqrt(param0.s0), rsqrt(param0.s1),
                 rsqrt(param0.s2)); }

INLINE_OVERLOADABLE float4 rsqrt (float4 param0)
{return (float4)(rsqrt(param0.s0), rsqrt(param0.s1),
                 rsqrt(param0.s2), rsqrt(param0.s3)); }

INLINE_OVERLOADABLE float8 rsqrt (float8 param0)
{return (float8)(rsqrt(param0.s0), rsqrt(param0.s1),
                 rsqrt(param0.s2), rsqrt(param0.s3),
                 rsqrt(param0.s4), rsqrt(param0.s5),
                 rsqrt(param0.s6), rsqrt(param0.s7)); }

INLINE_OVERLOADABLE float16 rsqrt (float16 param0)
{return (float16)(rsqrt(param0.s0),  rsqrt(param0.s1),
                  rsqrt(param0.s2),  rsqrt(param0.s3),
                  rsqrt(param0.s4),  rsqrt(param0.s5),
                  rsqrt(param0.s6),  rsqrt(param0.s7),
                  rsqrt(param0.s8),  rsqrt(param0.s9),
                  rsqrt(param0.sa), rsqrt(param0.sb),
                  rsqrt(param0.sc), rsqrt(param0.sd),
                  rsqrt(param0.se), rsqrt(param0.sf)); }


//gentype sin (gentype)

INLINE_OVERLOADABLE float2 sin (float2 param0)
{return (float2)(sin(param0.s0), sin(param0.s1)); }

INLINE_OVERLOADABLE float3 sin (float3 param0)
{return (float3)(sin(param0.s0), sin(param0.s1),
                 sin(param0.s2)); }

INLINE_OVERLOADABLE float4 sin (float4 param0)
{return (float4)(sin(param0.s0), sin(param0.s1),
                 sin(param0.s2), sin(param0.s3)); }

INLINE_OVERLOADABLE float8 sin (float8 param0)
{return (float8)(sin(param0.s0), sin(param0.s1),
                 sin(param0.s2), sin(param0.s3),
                 sin(param0.s4), sin(param0.s5),
                 sin(param0.s6), sin(param0.s7)); }

INLINE_OVERLOADABLE float16 sin (float16 param0)
{return (float16)(sin(param0.s0),  sin(param0.s1),
                  sin(param0.s2),  sin(param0.s3),
                  sin(param0.s4),  sin(param0.s5),
                  sin(param0.s6),  sin(param0.s7),
                  sin(param0.s8),  sin(param0.s9),
                  sin(param0.sa), sin(param0.sb),
                  sin(param0.sc), sin(param0.sd),
                  sin(param0.se), sin(param0.sf)); }


//gentype sinh (gentype)

INLINE_OVERLOADABLE float2 sinh (float2 param0)
{return (float2)(sinh(param0.s0), sinh(param0.s1)); }

INLINE_OVERLOADABLE float3 sinh (float3 param0)
{return (float3)(sinh(param0.s0), sinh(param0.s1),
                 sinh(param0.s2)); }

INLINE_OVERLOADABLE float4 sinh (float4 param0)
{return (float4)(sinh(param0.s0), sinh(param0.s1),
                 sinh(param0.s2), sinh(param0.s3)); }

INLINE_OVERLOADABLE float8 sinh (float8 param0)
{return (float8)(sinh(param0.s0), sinh(param0.s1),
                 sinh(param0.s2), sinh(param0.s3),
                 sinh(param0.s4), sinh(param0.s5),
                 sinh(param0.s6), sinh(param0.s7)); }

INLINE_OVERLOADABLE float16 sinh (float16 param0)
{return (float16)(sinh(param0.s0),  sinh(param0.s1),
                  sinh(param0.s2),  sinh(param0.s3),
                  sinh(param0.s4),  sinh(param0.s5),
                  sinh(param0.s6),  sinh(param0.s7),
                  sinh(param0.s8),  sinh(param0.s9),
                  sinh(param0.sa), sinh(param0.sb),
                  sinh(param0.sc), sinh(param0.sd),
                  sinh(param0.se), sinh(param0.sf)); }


//gentype sinpi (gentype x)

INLINE_OVERLOADABLE float2 sinpi (float2 param0)
{return (float2)(sinpi(param0.s0), sinpi(param0.s1)); }

INLINE_OVERLOADABLE float3 sinpi (float3 param0)
{return (float3)(sinpi(param0.s0), sinpi(param0.s1),
                 sinpi(param0.s2)); }

INLINE_OVERLOADABLE float4 sinpi (float4 param0)
{return (float4)(sinpi(param0.s0), sinpi(param0.s1),
                 sinpi(param0.s2), sinpi(param0.s3)); }

INLINE_OVERLOADABLE float8 sinpi (float8 param0)
{return (float8)(sinpi(param0.s0), sinpi(param0.s1),
                 sinpi(param0.s2), sinpi(param0.s3),
                 sinpi(param0.s4), sinpi(param0.s5),
                 sinpi(param0.s6), sinpi(param0.s7)); }

INLINE_OVERLOADABLE float16 sinpi (float16 param0)
{return (float16)(sinpi(param0.s0),  sinpi(param0.s1),
                  sinpi(param0.s2),  sinpi(param0.s3),
                  sinpi(param0.s4),  sinpi(param0.s5),
                  sinpi(param0.s6),  sinpi(param0.s7),
                  sinpi(param0.s8),  sinpi(param0.s9),
                  sinpi(param0.sa), sinpi(param0.sb),
                  sinpi(param0.sc), sinpi(param0.sd),
                  sinpi(param0.se), sinpi(param0.sf)); }


//gentype sqrt (gentype)

INLINE_OVERLOADABLE float2 sqrt (float2 param0)
{return (float2)(sqrt(param0.s0), sqrt(param0.s1)); }

INLINE_OVERLOADABLE float3 sqrt (float3 param0)
{return (float3)(sqrt(param0.s0), sqrt(param0.s1),
                 sqrt(param0.s2)); }

INLINE_OVERLOADABLE float4 sqrt (float4 param0)
{return (float4)(sqrt(param0.s0), sqrt(param0.s1),
                 sqrt(param0.s2), sqrt(param0.s3)); }

INLINE_OVERLOADABLE float8 sqrt (float8 param0)
{return (float8)(sqrt(param0.s0), sqrt(param0.s1),
                 sqrt(param0.s2), sqrt(param0.s3),
                 sqrt(param0.s4), sqrt(param0.s5),
                 sqrt(param0.s6), sqrt(param0.s7)); }

INLINE_OVERLOADABLE float16 sqrt (float16 param0)
{return (float16)(sqrt(param0.s0),  sqrt(param0.s1),
                  sqrt(param0.s2),  sqrt(param0.s3),
                  sqrt(param0.s4),  sqrt(param0.s5),
                  sqrt(param0.s6),  sqrt(param0.s7),
                  sqrt(param0.s8),  sqrt(param0.s9),
                  sqrt(param0.sa), sqrt(param0.sb),
                  sqrt(param0.sc), sqrt(param0.sd),
                  sqrt(param0.se), sqrt(param0.sf)); }


//gentype tan (gentype)

INLINE_OVERLOADABLE float2 tan (float2 param0)
{return (float2)(tan(param0.s0), tan(param0.s1)); }

INLINE_OVERLOADABLE float3 tan (float3 param0)
{return (float3)(tan(param0.s0), tan(param0.s1),
                 tan(param0.s2)); }

INLINE_OVERLOADABLE float4 tan (float4 param0)
{return (float4)(tan(param0.s0), tan(param0.s1),
                 tan(param0.s2), tan(param0.s3)); }

INLINE_OVERLOADABLE float8 tan (float8 param0)
{return (float8)(tan(param0.s0), tan(param0.s1),
                 tan(param0.s2), tan(param0.s3),
                 tan(param0.s4), tan(param0.s5),
                 tan(param0.s6), tan(param0.s7)); }

INLINE_OVERLOADABLE float16 tan (float16 param0)
{return (float16)(tan(param0.s0),  tan(param0.s1),
                  tan(param0.s2),  tan(param0.s3),
                  tan(param0.s4),  tan(param0.s5),
                  tan(param0.s6),  tan(param0.s7),
                  tan(param0.s8),  tan(param0.s9),
                  tan(param0.sa), tan(param0.sb),
                  tan(param0.sc), tan(param0.sd),
                  tan(param0.se), tan(param0.sf)); }


//gentype tanh (gentype)

INLINE_OVERLOADABLE float2 tanh (float2 param0)
{return (float2)(tanh(param0.s0), tanh(param0.s1)); }

INLINE_OVERLOADABLE float3 tanh (float3 param0)
{return (float3)(tanh(param0.s0), tanh(param0.s1),
                 tanh(param0.s2)); }

INLINE_OVERLOADABLE float4 tanh (float4 param0)
{return (float4)(tanh(param0.s0), tanh(param0.s1),
                 tanh(param0.s2), tanh(param0.s3)); }

INLINE_OVERLOADABLE float8 tanh (float8 param0)
{return (float8)(tanh(param0.s0), tanh(param0.s1),
                 tanh(param0.s2), tanh(param0.s3),
                 tanh(param0.s4), tanh(param0.s5),
                 tanh(param0.s6), tanh(param0.s7)); }

INLINE_OVERLOADABLE float16 tanh (float16 param0)
{return (float16)(tanh(param0.s0),  tanh(param0.s1),
                  tanh(param0.s2),  tanh(param0.s3),
                  tanh(param0.s4),  tanh(param0.s5),
                  tanh(param0.s6),  tanh(param0.s7),
                  tanh(param0.s8),  tanh(param0.s9),
                  tanh(param0.sa), tanh(param0.sb),
                  tanh(param0.sc), tanh(param0.sd),
                  tanh(param0.se), tanh(param0.sf)); }


//gentype tanpi (gentype x)

INLINE_OVERLOADABLE float2 tanpi (float2 param0)
{return (float2)(tanpi(param0.s0), tanpi(param0.s1)); }

INLINE_OVERLOADABLE float3 tanpi (float3 param0)
{return (float3)(tanpi(param0.s0), tanpi(param0.s1),
                 tanpi(param0.s2)); }

INLINE_OVERLOADABLE float4 tanpi (float4 param0)
{return (float4)(tanpi(param0.s0), tanpi(param0.s1),
                 tanpi(param0.s2), tanpi(param0.s3)); }

INLINE_OVERLOADABLE float8 tanpi (float8 param0)
{return (float8)(tanpi(param0.s0), tanpi(param0.s1),
                 tanpi(param0.s2), tanpi(param0.s3),
                 tanpi(param0.s4), tanpi(param0.s5),
                 tanpi(param0.s6), tanpi(param0.s7)); }

INLINE_OVERLOADABLE float16 tanpi (float16 param0)
{return (float16)(tanpi(param0.s0),  tanpi(param0.s1),
                  tanpi(param0.s2),  tanpi(param0.s3),
                  tanpi(param0.s4),  tanpi(param0.s5),
                  tanpi(param0.s6),  tanpi(param0.s7),
                  tanpi(param0.s8),  tanpi(param0.s9),
                  tanpi(param0.sa), tanpi(param0.sb),
                  tanpi(param0.sc), tanpi(param0.sd),
                  tanpi(param0.se), tanpi(param0.sf)); }


//gentype trunc (gentype)

INLINE_OVERLOADABLE float2 trunc (float2 param0)
{return (float2)(trunc(param0.s0), trunc(param0.s1)); }

INLINE_OVERLOADABLE float3 trunc (float3 param0)
{return (float3)(trunc(param0.s0), trunc(param0.s1),
                 trunc(param0.s2)); }

INLINE_OVERLOADABLE float4 trunc (float4 param0)
{return (float4)(trunc(param0.s0), trunc(param0.s1),
                 trunc(param0.s2), trunc(param0.s3)); }

INLINE_OVERLOADABLE float8 trunc (float8 param0)
{return (float8)(trunc(param0.s0), trunc(param0.s1),
                 trunc(param0.s2), trunc(param0.s3),
                 trunc(param0.s4), trunc(param0.s5),
                 trunc(param0.s6), trunc(param0.s7)); }

INLINE_OVERLOADABLE float16 trunc (float16 param0)
{return (float16)(trunc(param0.s0),  trunc(param0.s1),
                  trunc(param0.s2),  trunc(param0.s3),
                  trunc(param0.s4),  trunc(param0.s5),
                  trunc(param0.s6),  trunc(param0.s7),
                  trunc(param0.s8),  trunc(param0.s9),
                  trunc(param0.sa), trunc(param0.sb),
                  trunc(param0.sc), trunc(param0.sd),
                  trunc(param0.se), trunc(param0.sf)); }


//gentype native_recip (gentype x)

INLINE_OVERLOADABLE float2 native_recip (float2 param0)
{return (float2)(native_recip(param0.s0), native_recip(param0.s1)); }

INLINE_OVERLOADABLE float3 native_recip (float3 param0)
{return (float3)(native_recip(param0.s0), native_recip(param0.s1),
                 native_recip(param0.s2)); }

INLINE_OVERLOADABLE float4 native_recip (float4 param0)
{return (float4)(native_recip(param0.s0), native_recip(param0.s1),
                 native_recip(param0.s2), native_recip(param0.s3)); }

INLINE_OVERLOADABLE float8 native_recip (float8 param0)
{return (float8)(native_recip(param0.s0), native_recip(param0.s1),
                 native_recip(param0.s2), native_recip(param0.s3),
                 native_recip(param0.s4), native_recip(param0.s5),
                 native_recip(param0.s6), native_recip(param0.s7)); }

INLINE_OVERLOADABLE float16 native_recip (float16 param0)
{return (float16)(native_recip(param0.s0),  native_recip(param0.s1),
                  native_recip(param0.s2),  native_recip(param0.s3),
                  native_recip(param0.s4),  native_recip(param0.s5),
                  native_recip(param0.s6),  native_recip(param0.s7),
                  native_recip(param0.s8),  native_recip(param0.s9),
                  native_recip(param0.sa), native_recip(param0.sb),
                  native_recip(param0.sc), native_recip(param0.sd),
                  native_recip(param0.se), native_recip(param0.sf)); }


//ugentype abs (gentype x)

INLINE_OVERLOADABLE uchar2 abs (char2 param0)
{return (uchar2)(abs(param0.s0), abs(param0.s1)); }

INLINE_OVERLOADABLE uchar3 abs (char3 param0)
{return (uchar3)(abs(param0.s0), abs(param0.s1),
                 abs(param0.s2)); }

INLINE_OVERLOADABLE uchar4 abs (char4 param0)
{return (uchar4)(abs(param0.s0), abs(param0.s1),
                 abs(param0.s2), abs(param0.s3)); }

INLINE_OVERLOADABLE uchar8 abs (char8 param0)
{return (uchar8)(abs(param0.s0), abs(param0.s1),
                 abs(param0.s2), abs(param0.s3),
                 abs(param0.s4), abs(param0.s5),
                 abs(param0.s6), abs(param0.s7)); }

INLINE_OVERLOADABLE uchar16 abs (char16 param0)
{return (uchar16)(abs(param0.s0),  abs(param0.s1),
                  abs(param0.s2),  abs(param0.s3),
                  abs(param0.s4),  abs(param0.s5),
                  abs(param0.s6),  abs(param0.s7),
                  abs(param0.s8),  abs(param0.s9),
                  abs(param0.sa), abs(param0.sb),
                  abs(param0.sc), abs(param0.sd),
                  abs(param0.se), abs(param0.sf)); }

INLINE_OVERLOADABLE ushort2 abs (short2 param0)
{return (ushort2)(abs(param0.s0), abs(param0.s1)); }

INLINE_OVERLOADABLE ushort3 abs (short3 param0)
{return (ushort3)(abs(param0.s0), abs(param0.s1),
                  abs(param0.s2)); }

INLINE_OVERLOADABLE ushort4 abs (short4 param0)
{return (ushort4)(abs(param0.s0), abs(param0.s1),
                  abs(param0.s2), abs(param0.s3)); }

INLINE_OVERLOADABLE ushort8 abs (short8 param0)
{return (ushort8)(abs(param0.s0), abs(param0.s1),
                  abs(param0.s2), abs(param0.s3),
                  abs(param0.s4), abs(param0.s5),
                  abs(param0.s6), abs(param0.s7)); }

INLINE_OVERLOADABLE ushort16 abs (short16 param0)
{return (ushort16)(abs(param0.s0),  abs(param0.s1),
                   abs(param0.s2),  abs(param0.s3),
                   abs(param0.s4),  abs(param0.s5),
                   abs(param0.s6),  abs(param0.s7),
                   abs(param0.s8),  abs(param0.s9),
                   abs(param0.sa), abs(param0.sb),
                   abs(param0.sc), abs(param0.sd),
                   abs(param0.se), abs(param0.sf)); }

INLINE_OVERLOADABLE uint2 abs (int2 param0)
{return (uint2)(abs(param0.s0), abs(param0.s1)); }

INLINE_OVERLOADABLE uint3 abs (int3 param0)
{return (uint3)(abs(param0.s0), abs(param0.s1),
                abs(param0.s2)); }

INLINE_OVERLOADABLE uint4 abs (int4 param0)
{return (uint4)(abs(param0.s0), abs(param0.s1),
                abs(param0.s2), abs(param0.s3)); }

INLINE_OVERLOADABLE uint8 abs (int8 param0)
{return (uint8)(abs(param0.s0), abs(param0.s1),
                abs(param0.s2), abs(param0.s3),
                abs(param0.s4), abs(param0.s5),
                abs(param0.s6), abs(param0.s7)); }

INLINE_OVERLOADABLE uint16 abs (int16 param0)
{return (uint16)(abs(param0.s0),  abs(param0.s1),
                 abs(param0.s2),  abs(param0.s3),
                 abs(param0.s4),  abs(param0.s5),
                 abs(param0.s6),  abs(param0.s7),
                 abs(param0.s8),  abs(param0.s9),
                 abs(param0.sa), abs(param0.sb),
                 abs(param0.sc), abs(param0.sd),
                 abs(param0.se), abs(param0.sf)); }

INLINE_OVERLOADABLE uchar2 abs (uchar2 param0)
{return (uchar2)(abs(param0.s0), abs(param0.s1)); }

INLINE_OVERLOADABLE uchar3 abs (uchar3 param0)
{return (uchar3)(abs(param0.s0), abs(param0.s1),
                 abs(param0.s2)); }

INLINE_OVERLOADABLE uchar4 abs (uchar4 param0)
{return (uchar4)(abs(param0.s0), abs(param0.s1),
                 abs(param0.s2), abs(param0.s3)); }

INLINE_OVERLOADABLE uchar8 abs (uchar8 param0)
{return (uchar8)(abs(param0.s0), abs(param0.s1),
                 abs(param0.s2), abs(param0.s3),
                 abs(param0.s4), abs(param0.s5),
                 abs(param0.s6), abs(param0.s7)); }

INLINE_OVERLOADABLE uchar16 abs (uchar16 param0)
{return (uchar16)(abs(param0.s0),  abs(param0.s1),
                  abs(param0.s2),  abs(param0.s3),
                  abs(param0.s4),  abs(param0.s5),
                  abs(param0.s6),  abs(param0.s7),
                  abs(param0.s8),  abs(param0.s9),
                  abs(param0.sa), abs(param0.sb),
                  abs(param0.sc), abs(param0.sd),
                  abs(param0.se), abs(param0.sf)); }

INLINE_OVERLOADABLE ushort2 abs (ushort2 param0)
{return (ushort2)(abs(param0.s0), abs(param0.s1)); }

INLINE_OVERLOADABLE ushort3 abs (ushort3 param0)
{return (ushort3)(abs(param0.s0), abs(param0.s1),
                  abs(param0.s2)); }

INLINE_OVERLOADABLE ushort4 abs (ushort4 param0)
{return (ushort4)(abs(param0.s0), abs(param0.s1),
                  abs(param0.s2), abs(param0.s3)); }

INLINE_OVERLOADABLE ushort8 abs (ushort8 param0)
{return (ushort8)(abs(param0.s0), abs(param0.s1),
                  abs(param0.s2), abs(param0.s3),
                  abs(param0.s4), abs(param0.s5),
                  abs(param0.s6), abs(param0.s7)); }

INLINE_OVERLOADABLE ushort16 abs (ushort16 param0)
{return (ushort16)(abs(param0.s0),  abs(param0.s1),
                   abs(param0.s2),  abs(param0.s3),
                   abs(param0.s4),  abs(param0.s5),
                   abs(param0.s6),  abs(param0.s7),
                   abs(param0.s8),  abs(param0.s9),
                   abs(param0.sa), abs(param0.sb),
                   abs(param0.sc), abs(param0.sd),
                   abs(param0.se), abs(param0.sf)); }

INLINE_OVERLOADABLE uint2 abs (uint2 param0)
{return (uint2)(abs(param0.s0), abs(param0.s1)); }

INLINE_OVERLOADABLE uint3 abs (uint3 param0)
{return (uint3)(abs(param0.s0), abs(param0.s1),
                abs(param0.s2)); }

INLINE_OVERLOADABLE uint4 abs (uint4 param0)
{return (uint4)(abs(param0.s0), abs(param0.s1),
                abs(param0.s2), abs(param0.s3)); }

INLINE_OVERLOADABLE uint8 abs (uint8 param0)
{return (uint8)(abs(param0.s0), abs(param0.s1),
                abs(param0.s2), abs(param0.s3),
                abs(param0.s4), abs(param0.s5),
                abs(param0.s6), abs(param0.s7)); }

INLINE_OVERLOADABLE uint16 abs (uint16 param0)
{return (uint16)(abs(param0.s0),  abs(param0.s1),
                 abs(param0.s2),  abs(param0.s3),
                 abs(param0.s4),  abs(param0.s5),
                 abs(param0.s6),  abs(param0.s7),
                 abs(param0.s8),  abs(param0.s9),
                 abs(param0.sa), abs(param0.sb),
                 abs(param0.sc), abs(param0.sd),
                 abs(param0.se), abs(param0.sf)); }


//ugentype abs_diff (gentype x, gentype y)

INLINE_OVERLOADABLE uchar2 abs_diff (char2 param0, char2 param1)
{return (uchar2)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uchar3 abs_diff (char3 param0, char3 param1)
{return (uchar3)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                 abs_diff(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uchar4 abs_diff (char4 param0, char4 param1)
{return (uchar4)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                 abs_diff(param0.s2, param1.s2), abs_diff(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uchar8 abs_diff (char8 param0, char8 param1)
{return (uchar8)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                 abs_diff(param0.s2, param1.s2), abs_diff(param0.s3, param1.s3),
                 abs_diff(param0.s4, param1.s4), abs_diff(param0.s5, param1.s5),
                 abs_diff(param0.s6, param1.s6), abs_diff(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uchar16 abs_diff (char16 param0, char16 param1)
{return (uchar16)(abs_diff(param0.s0,  param1.s0),  abs_diff(param0.s1,  param1.s1),
                  abs_diff(param0.s2,  param1.s2),  abs_diff(param0.s3,  param1.s3),
                  abs_diff(param0.s4,  param1.s4),  abs_diff(param0.s5,  param1.s5),
                  abs_diff(param0.s6,  param1.s6),  abs_diff(param0.s7,  param1.s7),
                  abs_diff(param0.s8,  param1.s8),  abs_diff(param0.s9,  param1.s9),
                  abs_diff(param0.sa, param1.sa), abs_diff(param0.sb, param1.sb),
                  abs_diff(param0.sc, param1.sc), abs_diff(param0.sd, param1.sd),
                  abs_diff(param0.se, param1.se), abs_diff(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE ushort2 abs_diff (short2 param0, short2 param1)
{return (ushort2)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE ushort3 abs_diff (short3 param0, short3 param1)
{return (ushort3)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                  abs_diff(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE ushort4 abs_diff (short4 param0, short4 param1)
{return (ushort4)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                  abs_diff(param0.s2, param1.s2), abs_diff(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE ushort8 abs_diff (short8 param0, short8 param1)
{return (ushort8)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                  abs_diff(param0.s2, param1.s2), abs_diff(param0.s3, param1.s3),
                  abs_diff(param0.s4, param1.s4), abs_diff(param0.s5, param1.s5),
                  abs_diff(param0.s6, param1.s6), abs_diff(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE ushort16 abs_diff (short16 param0, short16 param1)
{return (ushort16)(abs_diff(param0.s0,  param1.s0),  abs_diff(param0.s1,  param1.s1),
                   abs_diff(param0.s2,  param1.s2),  abs_diff(param0.s3,  param1.s3),
                   abs_diff(param0.s4,  param1.s4),  abs_diff(param0.s5,  param1.s5),
                   abs_diff(param0.s6,  param1.s6),  abs_diff(param0.s7,  param1.s7),
                   abs_diff(param0.s8,  param1.s8),  abs_diff(param0.s9,  param1.s9),
                   abs_diff(param0.sa, param1.sa), abs_diff(param0.sb, param1.sb),
                   abs_diff(param0.sc, param1.sc), abs_diff(param0.sd, param1.sd),
                   abs_diff(param0.se, param1.se), abs_diff(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uint2 abs_diff (int2 param0, int2 param1)
{return (uint2)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uint3 abs_diff (int3 param0, int3 param1)
{return (uint3)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                abs_diff(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uint4 abs_diff (int4 param0, int4 param1)
{return (uint4)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                abs_diff(param0.s2, param1.s2), abs_diff(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uint8 abs_diff (int8 param0, int8 param1)
{return (uint8)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                abs_diff(param0.s2, param1.s2), abs_diff(param0.s3, param1.s3),
                abs_diff(param0.s4, param1.s4), abs_diff(param0.s5, param1.s5),
                abs_diff(param0.s6, param1.s6), abs_diff(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uint16 abs_diff (int16 param0, int16 param1)
{return (uint16)(abs_diff(param0.s0,  param1.s0),  abs_diff(param0.s1,  param1.s1),
                 abs_diff(param0.s2,  param1.s2),  abs_diff(param0.s3,  param1.s3),
                 abs_diff(param0.s4,  param1.s4),  abs_diff(param0.s5,  param1.s5),
                 abs_diff(param0.s6,  param1.s6),  abs_diff(param0.s7,  param1.s7),
                 abs_diff(param0.s8,  param1.s8),  abs_diff(param0.s9,  param1.s9),
                 abs_diff(param0.sa, param1.sa), abs_diff(param0.sb, param1.sb),
                 abs_diff(param0.sc, param1.sc), abs_diff(param0.sd, param1.sd),
                 abs_diff(param0.se, param1.se), abs_diff(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uchar2 abs_diff (uchar2 param0, uchar2 param1)
{return (uchar2)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uchar3 abs_diff (uchar3 param0, uchar3 param1)
{return (uchar3)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                 abs_diff(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uchar4 abs_diff (uchar4 param0, uchar4 param1)
{return (uchar4)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                 abs_diff(param0.s2, param1.s2), abs_diff(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uchar8 abs_diff (uchar8 param0, uchar8 param1)
{return (uchar8)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                 abs_diff(param0.s2, param1.s2), abs_diff(param0.s3, param1.s3),
                 abs_diff(param0.s4, param1.s4), abs_diff(param0.s5, param1.s5),
                 abs_diff(param0.s6, param1.s6), abs_diff(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uchar16 abs_diff (uchar16 param0, uchar16 param1)
{return (uchar16)(abs_diff(param0.s0,  param1.s0),  abs_diff(param0.s1,  param1.s1),
                  abs_diff(param0.s2,  param1.s2),  abs_diff(param0.s3,  param1.s3),
                  abs_diff(param0.s4,  param1.s4),  abs_diff(param0.s5,  param1.s5),
                  abs_diff(param0.s6,  param1.s6),  abs_diff(param0.s7,  param1.s7),
                  abs_diff(param0.s8,  param1.s8),  abs_diff(param0.s9,  param1.s9),
                  abs_diff(param0.sa, param1.sa), abs_diff(param0.sb, param1.sb),
                  abs_diff(param0.sc, param1.sc), abs_diff(param0.sd, param1.sd),
                  abs_diff(param0.se, param1.se), abs_diff(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE ushort2 abs_diff (ushort2 param0, ushort2 param1)
{return (ushort2)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE ushort3 abs_diff (ushort3 param0, ushort3 param1)
{return (ushort3)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                  abs_diff(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE ushort4 abs_diff (ushort4 param0, ushort4 param1)
{return (ushort4)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                  abs_diff(param0.s2, param1.s2), abs_diff(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE ushort8 abs_diff (ushort8 param0, ushort8 param1)
{return (ushort8)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                  abs_diff(param0.s2, param1.s2), abs_diff(param0.s3, param1.s3),
                  abs_diff(param0.s4, param1.s4), abs_diff(param0.s5, param1.s5),
                  abs_diff(param0.s6, param1.s6), abs_diff(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE ushort16 abs_diff (ushort16 param0, ushort16 param1)
{return (ushort16)(abs_diff(param0.s0,  param1.s0),  abs_diff(param0.s1,  param1.s1),
                   abs_diff(param0.s2,  param1.s2),  abs_diff(param0.s3,  param1.s3),
                   abs_diff(param0.s4,  param1.s4),  abs_diff(param0.s5,  param1.s5),
                   abs_diff(param0.s6,  param1.s6),  abs_diff(param0.s7,  param1.s7),
                   abs_diff(param0.s8,  param1.s8),  abs_diff(param0.s9,  param1.s9),
                   abs_diff(param0.sa, param1.sa), abs_diff(param0.sb, param1.sb),
                   abs_diff(param0.sc, param1.sc), abs_diff(param0.sd, param1.sd),
                   abs_diff(param0.se, param1.se), abs_diff(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uint2 abs_diff (uint2 param0, uint2 param1)
{return (uint2)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uint3 abs_diff (uint3 param0, uint3 param1)
{return (uint3)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                abs_diff(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uint4 abs_diff (uint4 param0, uint4 param1)
{return (uint4)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                abs_diff(param0.s2, param1.s2), abs_diff(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uint8 abs_diff (uint8 param0, uint8 param1)
{return (uint8)(abs_diff(param0.s0, param1.s0), abs_diff(param0.s1, param1.s1),
                abs_diff(param0.s2, param1.s2), abs_diff(param0.s3, param1.s3),
                abs_diff(param0.s4, param1.s4), abs_diff(param0.s5, param1.s5),
                abs_diff(param0.s6, param1.s6), abs_diff(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uint16 abs_diff (uint16 param0, uint16 param1)
{return (uint16)(abs_diff(param0.s0,  param1.s0),  abs_diff(param0.s1,  param1.s1),
                 abs_diff(param0.s2,  param1.s2),  abs_diff(param0.s3,  param1.s3),
                 abs_diff(param0.s4,  param1.s4),  abs_diff(param0.s5,  param1.s5),
                 abs_diff(param0.s6,  param1.s6),  abs_diff(param0.s7,  param1.s7),
                 abs_diff(param0.s8,  param1.s8),  abs_diff(param0.s9,  param1.s9),
                 abs_diff(param0.sa, param1.sa), abs_diff(param0.sb, param1.sb),
                 abs_diff(param0.sc, param1.sc), abs_diff(param0.sd, param1.sd),
                 abs_diff(param0.se, param1.se), abs_diff(param0.sf, param1.sf)); }


//gentype add_sat (gentype x,  gentype y)

INLINE_OVERLOADABLE char2 add_sat (char2 param0, char2 param1)
{return (char2)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE char3 add_sat (char3 param0, char3 param1)
{return (char3)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                add_sat(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE char4 add_sat (char4 param0, char4 param1)
{return (char4)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                add_sat(param0.s2, param1.s2), add_sat(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE char8 add_sat (char8 param0, char8 param1)
{return (char8)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                add_sat(param0.s2, param1.s2), add_sat(param0.s3, param1.s3),
                add_sat(param0.s4, param1.s4), add_sat(param0.s5, param1.s5),
                add_sat(param0.s6, param1.s6), add_sat(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE char16 add_sat (char16 param0, char16 param1)
{return (char16)(add_sat(param0.s0,  param1.s0),  add_sat(param0.s1,  param1.s1),
                 add_sat(param0.s2,  param1.s2),  add_sat(param0.s3,  param1.s3),
                 add_sat(param0.s4,  param1.s4),  add_sat(param0.s5,  param1.s5),
                 add_sat(param0.s6,  param1.s6),  add_sat(param0.s7,  param1.s7),
                 add_sat(param0.s8,  param1.s8),  add_sat(param0.s9,  param1.s9),
                 add_sat(param0.sa, param1.sa), add_sat(param0.sb, param1.sb),
                 add_sat(param0.sc, param1.sc), add_sat(param0.sd, param1.sd),
                 add_sat(param0.se, param1.se), add_sat(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE short2 add_sat (short2 param0, short2 param1)
{return (short2)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE short3 add_sat (short3 param0, short3 param1)
{return (short3)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                 add_sat(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE short4 add_sat (short4 param0, short4 param1)
{return (short4)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                 add_sat(param0.s2, param1.s2), add_sat(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE short8 add_sat (short8 param0, short8 param1)
{return (short8)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                 add_sat(param0.s2, param1.s2), add_sat(param0.s3, param1.s3),
                 add_sat(param0.s4, param1.s4), add_sat(param0.s5, param1.s5),
                 add_sat(param0.s6, param1.s6), add_sat(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE short16 add_sat (short16 param0, short16 param1)
{return (short16)(add_sat(param0.s0,  param1.s0),  add_sat(param0.s1,  param1.s1),
                  add_sat(param0.s2,  param1.s2),  add_sat(param0.s3,  param1.s3),
                  add_sat(param0.s4,  param1.s4),  add_sat(param0.s5,  param1.s5),
                  add_sat(param0.s6,  param1.s6),  add_sat(param0.s7,  param1.s7),
                  add_sat(param0.s8,  param1.s8),  add_sat(param0.s9,  param1.s9),
                  add_sat(param0.sa, param1.sa), add_sat(param0.sb, param1.sb),
                  add_sat(param0.sc, param1.sc), add_sat(param0.sd, param1.sd),
                  add_sat(param0.se, param1.se), add_sat(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE int2 add_sat (int2 param0, int2 param1)
{return (int2)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 add_sat (int3 param0, int3 param1)
{return (int3)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
               add_sat(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 add_sat (int4 param0, int4 param1)
{return (int4)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
               add_sat(param0.s2, param1.s2), add_sat(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 add_sat (int8 param0, int8 param1)
{return (int8)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
               add_sat(param0.s2, param1.s2), add_sat(param0.s3, param1.s3),
               add_sat(param0.s4, param1.s4), add_sat(param0.s5, param1.s5),
               add_sat(param0.s6, param1.s6), add_sat(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 add_sat (int16 param0, int16 param1)
{return (int16)(add_sat(param0.s0,  param1.s0),  add_sat(param0.s1,  param1.s1),
                add_sat(param0.s2,  param1.s2),  add_sat(param0.s3,  param1.s3),
                add_sat(param0.s4,  param1.s4),  add_sat(param0.s5,  param1.s5),
                add_sat(param0.s6,  param1.s6),  add_sat(param0.s7,  param1.s7),
                add_sat(param0.s8,  param1.s8),  add_sat(param0.s9,  param1.s9),
                add_sat(param0.sa, param1.sa), add_sat(param0.sb, param1.sb),
                add_sat(param0.sc, param1.sc), add_sat(param0.sd, param1.sd),
                add_sat(param0.se, param1.se), add_sat(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uchar2 add_sat (uchar2 param0, uchar2 param1)
{return (uchar2)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uchar3 add_sat (uchar3 param0, uchar3 param1)
{return (uchar3)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                 add_sat(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uchar4 add_sat (uchar4 param0, uchar4 param1)
{return (uchar4)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                 add_sat(param0.s2, param1.s2), add_sat(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uchar8 add_sat (uchar8 param0, uchar8 param1)
{return (uchar8)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                 add_sat(param0.s2, param1.s2), add_sat(param0.s3, param1.s3),
                 add_sat(param0.s4, param1.s4), add_sat(param0.s5, param1.s5),
                 add_sat(param0.s6, param1.s6), add_sat(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uchar16 add_sat (uchar16 param0, uchar16 param1)
{return (uchar16)(add_sat(param0.s0,  param1.s0),  add_sat(param0.s1,  param1.s1),
                  add_sat(param0.s2,  param1.s2),  add_sat(param0.s3,  param1.s3),
                  add_sat(param0.s4,  param1.s4),  add_sat(param0.s5,  param1.s5),
                  add_sat(param0.s6,  param1.s6),  add_sat(param0.s7,  param1.s7),
                  add_sat(param0.s8,  param1.s8),  add_sat(param0.s9,  param1.s9),
                  add_sat(param0.sa, param1.sa), add_sat(param0.sb, param1.sb),
                  add_sat(param0.sc, param1.sc), add_sat(param0.sd, param1.sd),
                  add_sat(param0.se, param1.se), add_sat(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE ushort2 add_sat (ushort2 param0, ushort2 param1)
{return (ushort2)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE ushort3 add_sat (ushort3 param0, ushort3 param1)
{return (ushort3)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                  add_sat(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE ushort4 add_sat (ushort4 param0, ushort4 param1)
{return (ushort4)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                  add_sat(param0.s2, param1.s2), add_sat(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE ushort8 add_sat (ushort8 param0, ushort8 param1)
{return (ushort8)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                  add_sat(param0.s2, param1.s2), add_sat(param0.s3, param1.s3),
                  add_sat(param0.s4, param1.s4), add_sat(param0.s5, param1.s5),
                  add_sat(param0.s6, param1.s6), add_sat(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE ushort16 add_sat (ushort16 param0, ushort16 param1)
{return (ushort16)(add_sat(param0.s0,  param1.s0),  add_sat(param0.s1,  param1.s1),
                   add_sat(param0.s2,  param1.s2),  add_sat(param0.s3,  param1.s3),
                   add_sat(param0.s4,  param1.s4),  add_sat(param0.s5,  param1.s5),
                   add_sat(param0.s6,  param1.s6),  add_sat(param0.s7,  param1.s7),
                   add_sat(param0.s8,  param1.s8),  add_sat(param0.s9,  param1.s9),
                   add_sat(param0.sa, param1.sa), add_sat(param0.sb, param1.sb),
                   add_sat(param0.sc, param1.sc), add_sat(param0.sd, param1.sd),
                   add_sat(param0.se, param1.se), add_sat(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uint2 add_sat (uint2 param0, uint2 param1)
{return (uint2)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uint3 add_sat (uint3 param0, uint3 param1)
{return (uint3)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                add_sat(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uint4 add_sat (uint4 param0, uint4 param1)
{return (uint4)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                add_sat(param0.s2, param1.s2), add_sat(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uint8 add_sat (uint8 param0, uint8 param1)
{return (uint8)(add_sat(param0.s0, param1.s0), add_sat(param0.s1, param1.s1),
                add_sat(param0.s2, param1.s2), add_sat(param0.s3, param1.s3),
                add_sat(param0.s4, param1.s4), add_sat(param0.s5, param1.s5),
                add_sat(param0.s6, param1.s6), add_sat(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uint16 add_sat (uint16 param0, uint16 param1)
{return (uint16)(add_sat(param0.s0,  param1.s0),  add_sat(param0.s1,  param1.s1),
                 add_sat(param0.s2,  param1.s2),  add_sat(param0.s3,  param1.s3),
                 add_sat(param0.s4,  param1.s4),  add_sat(param0.s5,  param1.s5),
                 add_sat(param0.s6,  param1.s6),  add_sat(param0.s7,  param1.s7),
                 add_sat(param0.s8,  param1.s8),  add_sat(param0.s9,  param1.s9),
                 add_sat(param0.sa, param1.sa), add_sat(param0.sb, param1.sb),
                 add_sat(param0.sc, param1.sc), add_sat(param0.sd, param1.sd),
                 add_sat(param0.se, param1.se), add_sat(param0.sf, param1.sf)); }


//gentype hadd (gentype x,  gentype y)

INLINE_OVERLOADABLE char2 hadd (char2 param0, char2 param1)
{return (char2)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE char3 hadd (char3 param0, char3 param1)
{return (char3)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                hadd(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE char4 hadd (char4 param0, char4 param1)
{return (char4)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                hadd(param0.s2, param1.s2), hadd(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE char8 hadd (char8 param0, char8 param1)
{return (char8)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                hadd(param0.s2, param1.s2), hadd(param0.s3, param1.s3),
                hadd(param0.s4, param1.s4), hadd(param0.s5, param1.s5),
                hadd(param0.s6, param1.s6), hadd(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE char16 hadd (char16 param0, char16 param1)
{return (char16)(hadd(param0.s0,  param1.s0),  hadd(param0.s1,  param1.s1),
                 hadd(param0.s2,  param1.s2),  hadd(param0.s3,  param1.s3),
                 hadd(param0.s4,  param1.s4),  hadd(param0.s5,  param1.s5),
                 hadd(param0.s6,  param1.s6),  hadd(param0.s7,  param1.s7),
                 hadd(param0.s8,  param1.s8),  hadd(param0.s9,  param1.s9),
                 hadd(param0.sa, param1.sa), hadd(param0.sb, param1.sb),
                 hadd(param0.sc, param1.sc), hadd(param0.sd, param1.sd),
                 hadd(param0.se, param1.se), hadd(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE short2 hadd (short2 param0, short2 param1)
{return (short2)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE short3 hadd (short3 param0, short3 param1)
{return (short3)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                 hadd(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE short4 hadd (short4 param0, short4 param1)
{return (short4)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                 hadd(param0.s2, param1.s2), hadd(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE short8 hadd (short8 param0, short8 param1)
{return (short8)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                 hadd(param0.s2, param1.s2), hadd(param0.s3, param1.s3),
                 hadd(param0.s4, param1.s4), hadd(param0.s5, param1.s5),
                 hadd(param0.s6, param1.s6), hadd(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE short16 hadd (short16 param0, short16 param1)
{return (short16)(hadd(param0.s0,  param1.s0),  hadd(param0.s1,  param1.s1),
                  hadd(param0.s2,  param1.s2),  hadd(param0.s3,  param1.s3),
                  hadd(param0.s4,  param1.s4),  hadd(param0.s5,  param1.s5),
                  hadd(param0.s6,  param1.s6),  hadd(param0.s7,  param1.s7),
                  hadd(param0.s8,  param1.s8),  hadd(param0.s9,  param1.s9),
                  hadd(param0.sa, param1.sa), hadd(param0.sb, param1.sb),
                  hadd(param0.sc, param1.sc), hadd(param0.sd, param1.sd),
                  hadd(param0.se, param1.se), hadd(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE int2 hadd (int2 param0, int2 param1)
{return (int2)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 hadd (int3 param0, int3 param1)
{return (int3)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
               hadd(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 hadd (int4 param0, int4 param1)
{return (int4)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
               hadd(param0.s2, param1.s2), hadd(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 hadd (int8 param0, int8 param1)
{return (int8)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
               hadd(param0.s2, param1.s2), hadd(param0.s3, param1.s3),
               hadd(param0.s4, param1.s4), hadd(param0.s5, param1.s5),
               hadd(param0.s6, param1.s6), hadd(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 hadd (int16 param0, int16 param1)
{return (int16)(hadd(param0.s0,  param1.s0),  hadd(param0.s1,  param1.s1),
                hadd(param0.s2,  param1.s2),  hadd(param0.s3,  param1.s3),
                hadd(param0.s4,  param1.s4),  hadd(param0.s5,  param1.s5),
                hadd(param0.s6,  param1.s6),  hadd(param0.s7,  param1.s7),
                hadd(param0.s8,  param1.s8),  hadd(param0.s9,  param1.s9),
                hadd(param0.sa, param1.sa), hadd(param0.sb, param1.sb),
                hadd(param0.sc, param1.sc), hadd(param0.sd, param1.sd),
                hadd(param0.se, param1.se), hadd(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uchar2 hadd (uchar2 param0, uchar2 param1)
{return (uchar2)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uchar3 hadd (uchar3 param0, uchar3 param1)
{return (uchar3)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                 hadd(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uchar4 hadd (uchar4 param0, uchar4 param1)
{return (uchar4)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                 hadd(param0.s2, param1.s2), hadd(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uchar8 hadd (uchar8 param0, uchar8 param1)
{return (uchar8)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                 hadd(param0.s2, param1.s2), hadd(param0.s3, param1.s3),
                 hadd(param0.s4, param1.s4), hadd(param0.s5, param1.s5),
                 hadd(param0.s6, param1.s6), hadd(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uchar16 hadd (uchar16 param0, uchar16 param1)
{return (uchar16)(hadd(param0.s0,  param1.s0),  hadd(param0.s1,  param1.s1),
                  hadd(param0.s2,  param1.s2),  hadd(param0.s3,  param1.s3),
                  hadd(param0.s4,  param1.s4),  hadd(param0.s5,  param1.s5),
                  hadd(param0.s6,  param1.s6),  hadd(param0.s7,  param1.s7),
                  hadd(param0.s8,  param1.s8),  hadd(param0.s9,  param1.s9),
                  hadd(param0.sa, param1.sa), hadd(param0.sb, param1.sb),
                  hadd(param0.sc, param1.sc), hadd(param0.sd, param1.sd),
                  hadd(param0.se, param1.se), hadd(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE ushort2 hadd (ushort2 param0, ushort2 param1)
{return (ushort2)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE ushort3 hadd (ushort3 param0, ushort3 param1)
{return (ushort3)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                  hadd(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE ushort4 hadd (ushort4 param0, ushort4 param1)
{return (ushort4)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                  hadd(param0.s2, param1.s2), hadd(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE ushort8 hadd (ushort8 param0, ushort8 param1)
{return (ushort8)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                  hadd(param0.s2, param1.s2), hadd(param0.s3, param1.s3),
                  hadd(param0.s4, param1.s4), hadd(param0.s5, param1.s5),
                  hadd(param0.s6, param1.s6), hadd(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE ushort16 hadd (ushort16 param0, ushort16 param1)
{return (ushort16)(hadd(param0.s0,  param1.s0),  hadd(param0.s1,  param1.s1),
                   hadd(param0.s2,  param1.s2),  hadd(param0.s3,  param1.s3),
                   hadd(param0.s4,  param1.s4),  hadd(param0.s5,  param1.s5),
                   hadd(param0.s6,  param1.s6),  hadd(param0.s7,  param1.s7),
                   hadd(param0.s8,  param1.s8),  hadd(param0.s9,  param1.s9),
                   hadd(param0.sa, param1.sa), hadd(param0.sb, param1.sb),
                   hadd(param0.sc, param1.sc), hadd(param0.sd, param1.sd),
                   hadd(param0.se, param1.se), hadd(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uint2 hadd (uint2 param0, uint2 param1)
{return (uint2)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uint3 hadd (uint3 param0, uint3 param1)
{return (uint3)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                hadd(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uint4 hadd (uint4 param0, uint4 param1)
{return (uint4)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                hadd(param0.s2, param1.s2), hadd(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uint8 hadd (uint8 param0, uint8 param1)
{return (uint8)(hadd(param0.s0, param1.s0), hadd(param0.s1, param1.s1),
                hadd(param0.s2, param1.s2), hadd(param0.s3, param1.s3),
                hadd(param0.s4, param1.s4), hadd(param0.s5, param1.s5),
                hadd(param0.s6, param1.s6), hadd(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uint16 hadd (uint16 param0, uint16 param1)
{return (uint16)(hadd(param0.s0,  param1.s0),  hadd(param0.s1,  param1.s1),
                 hadd(param0.s2,  param1.s2),  hadd(param0.s3,  param1.s3),
                 hadd(param0.s4,  param1.s4),  hadd(param0.s5,  param1.s5),
                 hadd(param0.s6,  param1.s6),  hadd(param0.s7,  param1.s7),
                 hadd(param0.s8,  param1.s8),  hadd(param0.s9,  param1.s9),
                 hadd(param0.sa, param1.sa), hadd(param0.sb, param1.sb),
                 hadd(param0.sc, param1.sc), hadd(param0.sd, param1.sd),
                 hadd(param0.se, param1.se), hadd(param0.sf, param1.sf)); }


//gentype rhadd (gentype x, gentype y)

INLINE_OVERLOADABLE char2 rhadd (char2 param0, char2 param1)
{return (char2)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE char3 rhadd (char3 param0, char3 param1)
{return (char3)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                rhadd(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE char4 rhadd (char4 param0, char4 param1)
{return (char4)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                rhadd(param0.s2, param1.s2), rhadd(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE char8 rhadd (char8 param0, char8 param1)
{return (char8)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                rhadd(param0.s2, param1.s2), rhadd(param0.s3, param1.s3),
                rhadd(param0.s4, param1.s4), rhadd(param0.s5, param1.s5),
                rhadd(param0.s6, param1.s6), rhadd(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE char16 rhadd (char16 param0, char16 param1)
{return (char16)(rhadd(param0.s0,  param1.s0),  rhadd(param0.s1,  param1.s1),
                 rhadd(param0.s2,  param1.s2),  rhadd(param0.s3,  param1.s3),
                 rhadd(param0.s4,  param1.s4),  rhadd(param0.s5,  param1.s5),
                 rhadd(param0.s6,  param1.s6),  rhadd(param0.s7,  param1.s7),
                 rhadd(param0.s8,  param1.s8),  rhadd(param0.s9,  param1.s9),
                 rhadd(param0.sa, param1.sa), rhadd(param0.sb, param1.sb),
                 rhadd(param0.sc, param1.sc), rhadd(param0.sd, param1.sd),
                 rhadd(param0.se, param1.se), rhadd(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE short2 rhadd (short2 param0, short2 param1)
{return (short2)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE short3 rhadd (short3 param0, short3 param1)
{return (short3)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                 rhadd(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE short4 rhadd (short4 param0, short4 param1)
{return (short4)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                 rhadd(param0.s2, param1.s2), rhadd(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE short8 rhadd (short8 param0, short8 param1)
{return (short8)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                 rhadd(param0.s2, param1.s2), rhadd(param0.s3, param1.s3),
                 rhadd(param0.s4, param1.s4), rhadd(param0.s5, param1.s5),
                 rhadd(param0.s6, param1.s6), rhadd(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE short16 rhadd (short16 param0, short16 param1)
{return (short16)(rhadd(param0.s0,  param1.s0),  rhadd(param0.s1,  param1.s1),
                  rhadd(param0.s2,  param1.s2),  rhadd(param0.s3,  param1.s3),
                  rhadd(param0.s4,  param1.s4),  rhadd(param0.s5,  param1.s5),
                  rhadd(param0.s6,  param1.s6),  rhadd(param0.s7,  param1.s7),
                  rhadd(param0.s8,  param1.s8),  rhadd(param0.s9,  param1.s9),
                  rhadd(param0.sa, param1.sa), rhadd(param0.sb, param1.sb),
                  rhadd(param0.sc, param1.sc), rhadd(param0.sd, param1.sd),
                  rhadd(param0.se, param1.se), rhadd(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE int2 rhadd (int2 param0, int2 param1)
{return (int2)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 rhadd (int3 param0, int3 param1)
{return (int3)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
               rhadd(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 rhadd (int4 param0, int4 param1)
{return (int4)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
               rhadd(param0.s2, param1.s2), rhadd(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 rhadd (int8 param0, int8 param1)
{return (int8)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
               rhadd(param0.s2, param1.s2), rhadd(param0.s3, param1.s3),
               rhadd(param0.s4, param1.s4), rhadd(param0.s5, param1.s5),
               rhadd(param0.s6, param1.s6), rhadd(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 rhadd (int16 param0, int16 param1)
{return (int16)(rhadd(param0.s0,  param1.s0),  rhadd(param0.s1,  param1.s1),
                rhadd(param0.s2,  param1.s2),  rhadd(param0.s3,  param1.s3),
                rhadd(param0.s4,  param1.s4),  rhadd(param0.s5,  param1.s5),
                rhadd(param0.s6,  param1.s6),  rhadd(param0.s7,  param1.s7),
                rhadd(param0.s8,  param1.s8),  rhadd(param0.s9,  param1.s9),
                rhadd(param0.sa, param1.sa), rhadd(param0.sb, param1.sb),
                rhadd(param0.sc, param1.sc), rhadd(param0.sd, param1.sd),
                rhadd(param0.se, param1.se), rhadd(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uchar2 rhadd (uchar2 param0, uchar2 param1)
{return (uchar2)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uchar3 rhadd (uchar3 param0, uchar3 param1)
{return (uchar3)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                 rhadd(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uchar4 rhadd (uchar4 param0, uchar4 param1)
{return (uchar4)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                 rhadd(param0.s2, param1.s2), rhadd(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uchar8 rhadd (uchar8 param0, uchar8 param1)
{return (uchar8)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                 rhadd(param0.s2, param1.s2), rhadd(param0.s3, param1.s3),
                 rhadd(param0.s4, param1.s4), rhadd(param0.s5, param1.s5),
                 rhadd(param0.s6, param1.s6), rhadd(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uchar16 rhadd (uchar16 param0, uchar16 param1)
{return (uchar16)(rhadd(param0.s0,  param1.s0),  rhadd(param0.s1,  param1.s1),
                  rhadd(param0.s2,  param1.s2),  rhadd(param0.s3,  param1.s3),
                  rhadd(param0.s4,  param1.s4),  rhadd(param0.s5,  param1.s5),
                  rhadd(param0.s6,  param1.s6),  rhadd(param0.s7,  param1.s7),
                  rhadd(param0.s8,  param1.s8),  rhadd(param0.s9,  param1.s9),
                  rhadd(param0.sa, param1.sa), rhadd(param0.sb, param1.sb),
                  rhadd(param0.sc, param1.sc), rhadd(param0.sd, param1.sd),
                  rhadd(param0.se, param1.se), rhadd(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE ushort2 rhadd (ushort2 param0, ushort2 param1)
{return (ushort2)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE ushort3 rhadd (ushort3 param0, ushort3 param1)
{return (ushort3)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                  rhadd(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE ushort4 rhadd (ushort4 param0, ushort4 param1)
{return (ushort4)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                  rhadd(param0.s2, param1.s2), rhadd(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE ushort8 rhadd (ushort8 param0, ushort8 param1)
{return (ushort8)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                  rhadd(param0.s2, param1.s2), rhadd(param0.s3, param1.s3),
                  rhadd(param0.s4, param1.s4), rhadd(param0.s5, param1.s5),
                  rhadd(param0.s6, param1.s6), rhadd(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE ushort16 rhadd (ushort16 param0, ushort16 param1)
{return (ushort16)(rhadd(param0.s0,  param1.s0),  rhadd(param0.s1,  param1.s1),
                   rhadd(param0.s2,  param1.s2),  rhadd(param0.s3,  param1.s3),
                   rhadd(param0.s4,  param1.s4),  rhadd(param0.s5,  param1.s5),
                   rhadd(param0.s6,  param1.s6),  rhadd(param0.s7,  param1.s7),
                   rhadd(param0.s8,  param1.s8),  rhadd(param0.s9,  param1.s9),
                   rhadd(param0.sa, param1.sa), rhadd(param0.sb, param1.sb),
                   rhadd(param0.sc, param1.sc), rhadd(param0.sd, param1.sd),
                   rhadd(param0.se, param1.se), rhadd(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uint2 rhadd (uint2 param0, uint2 param1)
{return (uint2)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uint3 rhadd (uint3 param0, uint3 param1)
{return (uint3)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                rhadd(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uint4 rhadd (uint4 param0, uint4 param1)
{return (uint4)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                rhadd(param0.s2, param1.s2), rhadd(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uint8 rhadd (uint8 param0, uint8 param1)
{return (uint8)(rhadd(param0.s0, param1.s0), rhadd(param0.s1, param1.s1),
                rhadd(param0.s2, param1.s2), rhadd(param0.s3, param1.s3),
                rhadd(param0.s4, param1.s4), rhadd(param0.s5, param1.s5),
                rhadd(param0.s6, param1.s6), rhadd(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uint16 rhadd (uint16 param0, uint16 param1)
{return (uint16)(rhadd(param0.s0,  param1.s0),  rhadd(param0.s1,  param1.s1),
                 rhadd(param0.s2,  param1.s2),  rhadd(param0.s3,  param1.s3),
                 rhadd(param0.s4,  param1.s4),  rhadd(param0.s5,  param1.s5),
                 rhadd(param0.s6,  param1.s6),  rhadd(param0.s7,  param1.s7),
                 rhadd(param0.s8,  param1.s8),  rhadd(param0.s9,  param1.s9),
                 rhadd(param0.sa, param1.sa), rhadd(param0.sb, param1.sb),
                 rhadd(param0.sc, param1.sc), rhadd(param0.sd, param1.sd),
                 rhadd(param0.se, param1.se), rhadd(param0.sf, param1.sf)); }


//gentype clamp (gentype x, gentype minval, gentype maxval)

INLINE_OVERLOADABLE char2 clamp (char2 param0, char2 param1, char2 param2)
{return (char2)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE char3 clamp (char3 param0, char3 param1, char3 param2)
{return (char3)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                clamp(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE char4 clamp (char4 param0, char4 param1, char4 param2)
{return (char4)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE char8 clamp (char8 param0, char8 param1, char8 param2)
{return (char8)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3),
                clamp(param0.s4, param1.s4, param2.s4), clamp(param0.s5, param1.s5, param2.s5),
                clamp(param0.s6, param1.s6, param2.s6), clamp(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE char16 clamp (char16 param0, char16 param1, char16 param2)
{return (char16)(clamp(param0.s0,  param1.s0,  param2.s0),  clamp(param0.s1,  param1.s1,  param2.s1),
                 clamp(param0.s2,  param1.s2,  param2.s2),  clamp(param0.s3,  param1.s3,  param2.s3),
                 clamp(param0.s4,  param1.s4,  param2.s4),  clamp(param0.s5,  param1.s5,  param2.s5),
                 clamp(param0.s6,  param1.s6,  param2.s6),  clamp(param0.s7,  param1.s7,  param2.s7),
                 clamp(param0.s8,  param1.s8,  param2.s8),  clamp(param0.s9,  param1.s9,  param2.s9),
                 clamp(param0.sa, param1.sa, param2.sa), clamp(param0.sb, param1.sb, param2.sb),
                 clamp(param0.sc, param1.sc, param2.sc), clamp(param0.sd, param1.sd, param2.sd),
                 clamp(param0.se, param1.se, param2.se), clamp(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE short2 clamp (short2 param0, short2 param1, short2 param2)
{return (short2)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE short3 clamp (short3 param0, short3 param1, short3 param2)
{return (short3)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                 clamp(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE short4 clamp (short4 param0, short4 param1, short4 param2)
{return (short4)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                 clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE short8 clamp (short8 param0, short8 param1, short8 param2)
{return (short8)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                 clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3),
                 clamp(param0.s4, param1.s4, param2.s4), clamp(param0.s5, param1.s5, param2.s5),
                 clamp(param0.s6, param1.s6, param2.s6), clamp(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE short16 clamp (short16 param0, short16 param1, short16 param2)
{return (short16)(clamp(param0.s0,  param1.s0,  param2.s0),  clamp(param0.s1,  param1.s1,  param2.s1),
                  clamp(param0.s2,  param1.s2,  param2.s2),  clamp(param0.s3,  param1.s3,  param2.s3),
                  clamp(param0.s4,  param1.s4,  param2.s4),  clamp(param0.s5,  param1.s5,  param2.s5),
                  clamp(param0.s6,  param1.s6,  param2.s6),  clamp(param0.s7,  param1.s7,  param2.s7),
                  clamp(param0.s8,  param1.s8,  param2.s8),  clamp(param0.s9,  param1.s9,  param2.s9),
                  clamp(param0.sa, param1.sa, param2.sa), clamp(param0.sb, param1.sb, param2.sb),
                  clamp(param0.sc, param1.sc, param2.sc), clamp(param0.sd, param1.sd, param2.sd),
                  clamp(param0.se, param1.se, param2.se), clamp(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE int2 clamp (int2 param0, int2 param1, int2 param2)
{return (int2)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE int3 clamp (int3 param0, int3 param1, int3 param2)
{return (int3)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
               clamp(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE int4 clamp (int4 param0, int4 param1, int4 param2)
{return (int4)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
               clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE int8 clamp (int8 param0, int8 param1, int8 param2)
{return (int8)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
               clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3),
               clamp(param0.s4, param1.s4, param2.s4), clamp(param0.s5, param1.s5, param2.s5),
               clamp(param0.s6, param1.s6, param2.s6), clamp(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE int16 clamp (int16 param0, int16 param1, int16 param2)
{return (int16)(clamp(param0.s0,  param1.s0,  param2.s0),  clamp(param0.s1,  param1.s1,  param2.s1),
                clamp(param0.s2,  param1.s2,  param2.s2),  clamp(param0.s3,  param1.s3,  param2.s3),
                clamp(param0.s4,  param1.s4,  param2.s4),  clamp(param0.s5,  param1.s5,  param2.s5),
                clamp(param0.s6,  param1.s6,  param2.s6),  clamp(param0.s7,  param1.s7,  param2.s7),
                clamp(param0.s8,  param1.s8,  param2.s8),  clamp(param0.s9,  param1.s9,  param2.s9),
                clamp(param0.sa, param1.sa, param2.sa), clamp(param0.sb, param1.sb, param2.sb),
                clamp(param0.sc, param1.sc, param2.sc), clamp(param0.sd, param1.sd, param2.sd),
                clamp(param0.se, param1.se, param2.se), clamp(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE uchar2 clamp (uchar2 param0, uchar2 param1, uchar2 param2)
{return (uchar2)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE uchar3 clamp (uchar3 param0, uchar3 param1, uchar3 param2)
{return (uchar3)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                 clamp(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE uchar4 clamp (uchar4 param0, uchar4 param1, uchar4 param2)
{return (uchar4)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                 clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE uchar8 clamp (uchar8 param0, uchar8 param1, uchar8 param2)
{return (uchar8)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                 clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3),
                 clamp(param0.s4, param1.s4, param2.s4), clamp(param0.s5, param1.s5, param2.s5),
                 clamp(param0.s6, param1.s6, param2.s6), clamp(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE uchar16 clamp (uchar16 param0, uchar16 param1, uchar16 param2)
{return (uchar16)(clamp(param0.s0,  param1.s0,  param2.s0),  clamp(param0.s1,  param1.s1,  param2.s1),
                  clamp(param0.s2,  param1.s2,  param2.s2),  clamp(param0.s3,  param1.s3,  param2.s3),
                  clamp(param0.s4,  param1.s4,  param2.s4),  clamp(param0.s5,  param1.s5,  param2.s5),
                  clamp(param0.s6,  param1.s6,  param2.s6),  clamp(param0.s7,  param1.s7,  param2.s7),
                  clamp(param0.s8,  param1.s8,  param2.s8),  clamp(param0.s9,  param1.s9,  param2.s9),
                  clamp(param0.sa, param1.sa, param2.sa), clamp(param0.sb, param1.sb, param2.sb),
                  clamp(param0.sc, param1.sc, param2.sc), clamp(param0.sd, param1.sd, param2.sd),
                  clamp(param0.se, param1.se, param2.se), clamp(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE ushort2 clamp (ushort2 param0, ushort2 param1, ushort2 param2)
{return (ushort2)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE ushort3 clamp (ushort3 param0, ushort3 param1, ushort3 param2)
{return (ushort3)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                  clamp(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE ushort4 clamp (ushort4 param0, ushort4 param1, ushort4 param2)
{return (ushort4)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                  clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE ushort8 clamp (ushort8 param0, ushort8 param1, ushort8 param2)
{return (ushort8)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                  clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3),
                  clamp(param0.s4, param1.s4, param2.s4), clamp(param0.s5, param1.s5, param2.s5),
                  clamp(param0.s6, param1.s6, param2.s6), clamp(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE ushort16 clamp (ushort16 param0, ushort16 param1, ushort16 param2)
{return (ushort16)(clamp(param0.s0,  param1.s0,  param2.s0),  clamp(param0.s1,  param1.s1,  param2.s1),
                   clamp(param0.s2,  param1.s2,  param2.s2),  clamp(param0.s3,  param1.s3,  param2.s3),
                   clamp(param0.s4,  param1.s4,  param2.s4),  clamp(param0.s5,  param1.s5,  param2.s5),
                   clamp(param0.s6,  param1.s6,  param2.s6),  clamp(param0.s7,  param1.s7,  param2.s7),
                   clamp(param0.s8,  param1.s8,  param2.s8),  clamp(param0.s9,  param1.s9,  param2.s9),
                   clamp(param0.sa, param1.sa, param2.sa), clamp(param0.sb, param1.sb, param2.sb),
                   clamp(param0.sc, param1.sc, param2.sc), clamp(param0.sd, param1.sd, param2.sd),
                   clamp(param0.se, param1.se, param2.se), clamp(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE uint2 clamp (uint2 param0, uint2 param1, uint2 param2)
{return (uint2)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE uint3 clamp (uint3 param0, uint3 param1, uint3 param2)
{return (uint3)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                clamp(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE uint4 clamp (uint4 param0, uint4 param1, uint4 param2)
{return (uint4)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE uint8 clamp (uint8 param0, uint8 param1, uint8 param2)
{return (uint8)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3),
                clamp(param0.s4, param1.s4, param2.s4), clamp(param0.s5, param1.s5, param2.s5),
                clamp(param0.s6, param1.s6, param2.s6), clamp(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE uint16 clamp (uint16 param0, uint16 param1, uint16 param2)
{return (uint16)(clamp(param0.s0,  param1.s0,  param2.s0),  clamp(param0.s1,  param1.s1,  param2.s1),
                 clamp(param0.s2,  param1.s2,  param2.s2),  clamp(param0.s3,  param1.s3,  param2.s3),
                 clamp(param0.s4,  param1.s4,  param2.s4),  clamp(param0.s5,  param1.s5,  param2.s5),
                 clamp(param0.s6,  param1.s6,  param2.s6),  clamp(param0.s7,  param1.s7,  param2.s7),
                 clamp(param0.s8,  param1.s8,  param2.s8),  clamp(param0.s9,  param1.s9,  param2.s9),
                 clamp(param0.sa, param1.sa, param2.sa), clamp(param0.sb, param1.sb, param2.sb),
                 clamp(param0.sc, param1.sc, param2.sc), clamp(param0.sd, param1.sd, param2.sd),
                 clamp(param0.se, param1.se, param2.se), clamp(param0.sf, param1.sf, param2.sf)); }


//gentype clamp (gentype x, sgentype minval, sgentype maxval)

INLINE_OVERLOADABLE char2 clamp (char2 param0, char param1, char param2)
{return (char2)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2)); }

INLINE_OVERLOADABLE char3 clamp (char3 param0, char param1, char param2)
{return (char3)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                clamp(param0.s2, param1, param2)); }

INLINE_OVERLOADABLE char4 clamp (char4 param0, char param1, char param2)
{return (char4)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2)); }

INLINE_OVERLOADABLE char8 clamp (char8 param0, char param1, char param2)
{return (char8)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2),
                clamp(param0.s4, param1, param2), clamp(param0.s5, param1, param2),
                clamp(param0.s6, param1, param2), clamp(param0.s7, param1, param2)); }

INLINE_OVERLOADABLE char16 clamp (char16 param0, char param1, char param2)
{return (char16)(clamp(param0.s0,  param1,  param2),  clamp(param0.s1,  param1,  param2),
                 clamp(param0.s2,  param1,  param2),  clamp(param0.s3,  param1,  param2),
                 clamp(param0.s4,  param1,  param2),  clamp(param0.s5,  param1,  param2),
                 clamp(param0.s6,  param1,  param2),  clamp(param0.s7,  param1,  param2),
                 clamp(param0.s8,  param1,  param2),  clamp(param0.s9,  param1,  param2),
                 clamp(param0.sa, param1, param2), clamp(param0.sb, param1, param2),
                 clamp(param0.sc, param1, param2), clamp(param0.sd, param1, param2),
                 clamp(param0.se, param1, param2), clamp(param0.sf, param1, param2)); }

INLINE_OVERLOADABLE short2 clamp (short2 param0, short param1, short param2)
{return (short2)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2)); }

INLINE_OVERLOADABLE short3 clamp (short3 param0, short param1, short param2)
{return (short3)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                 clamp(param0.s2, param1, param2)); }

INLINE_OVERLOADABLE short4 clamp (short4 param0, short param1, short param2)
{return (short4)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                 clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2)); }

INLINE_OVERLOADABLE short8 clamp (short8 param0, short param1, short param2)
{return (short8)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                 clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2),
                 clamp(param0.s4, param1, param2), clamp(param0.s5, param1, param2),
                 clamp(param0.s6, param1, param2), clamp(param0.s7, param1, param2)); }

INLINE_OVERLOADABLE short16 clamp (short16 param0, short param1, short param2)
{return (short16)(clamp(param0.s0,  param1,  param2),  clamp(param0.s1,  param1,  param2),
                  clamp(param0.s2,  param1,  param2),  clamp(param0.s3,  param1,  param2),
                  clamp(param0.s4,  param1,  param2),  clamp(param0.s5,  param1,  param2),
                  clamp(param0.s6,  param1,  param2),  clamp(param0.s7,  param1,  param2),
                  clamp(param0.s8,  param1,  param2),  clamp(param0.s9,  param1,  param2),
                  clamp(param0.sa, param1, param2), clamp(param0.sb, param1, param2),
                  clamp(param0.sc, param1, param2), clamp(param0.sd, param1, param2),
                  clamp(param0.se, param1, param2), clamp(param0.sf, param1, param2)); }

INLINE_OVERLOADABLE int2 clamp (int2 param0, int param1, int param2)
{return (int2)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2)); }

INLINE_OVERLOADABLE int3 clamp (int3 param0, int param1, int param2)
{return (int3)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
               clamp(param0.s2, param1, param2)); }

INLINE_OVERLOADABLE int4 clamp (int4 param0, int param1, int param2)
{return (int4)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
               clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2)); }

INLINE_OVERLOADABLE int8 clamp (int8 param0, int param1, int param2)
{return (int8)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
               clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2),
               clamp(param0.s4, param1, param2), clamp(param0.s5, param1, param2),
               clamp(param0.s6, param1, param2), clamp(param0.s7, param1, param2)); }

INLINE_OVERLOADABLE int16 clamp (int16 param0, int param1, int param2)
{return (int16)(clamp(param0.s0,  param1,  param2),  clamp(param0.s1,  param1,  param2),
                clamp(param0.s2,  param1,  param2),  clamp(param0.s3,  param1,  param2),
                clamp(param0.s4,  param1,  param2),  clamp(param0.s5,  param1,  param2),
                clamp(param0.s6,  param1,  param2),  clamp(param0.s7,  param1,  param2),
                clamp(param0.s8,  param1,  param2),  clamp(param0.s9,  param1,  param2),
                clamp(param0.sa, param1, param2), clamp(param0.sb, param1, param2),
                clamp(param0.sc, param1, param2), clamp(param0.sd, param1, param2),
                clamp(param0.se, param1, param2), clamp(param0.sf, param1, param2)); }

INLINE_OVERLOADABLE uchar2 clamp (uchar2 param0, uchar param1, uchar param2)
{return (uchar2)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2)); }

INLINE_OVERLOADABLE uchar3 clamp (uchar3 param0, uchar param1, uchar param2)
{return (uchar3)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                 clamp(param0.s2, param1, param2)); }

INLINE_OVERLOADABLE uchar4 clamp (uchar4 param0, uchar param1, uchar param2)
{return (uchar4)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                 clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2)); }

INLINE_OVERLOADABLE uchar8 clamp (uchar8 param0, uchar param1, uchar param2)
{return (uchar8)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                 clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2),
                 clamp(param0.s4, param1, param2), clamp(param0.s5, param1, param2),
                 clamp(param0.s6, param1, param2), clamp(param0.s7, param1, param2)); }

INLINE_OVERLOADABLE uchar16 clamp (uchar16 param0, uchar param1, uchar param2)
{return (uchar16)(clamp(param0.s0,  param1,  param2),  clamp(param0.s1,  param1,  param2),
                  clamp(param0.s2,  param1,  param2),  clamp(param0.s3,  param1,  param2),
                  clamp(param0.s4,  param1,  param2),  clamp(param0.s5,  param1,  param2),
                  clamp(param0.s6,  param1,  param2),  clamp(param0.s7,  param1,  param2),
                  clamp(param0.s8,  param1,  param2),  clamp(param0.s9,  param1,  param2),
                  clamp(param0.sa, param1, param2), clamp(param0.sb, param1, param2),
                  clamp(param0.sc, param1, param2), clamp(param0.sd, param1, param2),
                  clamp(param0.se, param1, param2), clamp(param0.sf, param1, param2)); }

INLINE_OVERLOADABLE ushort2 clamp (ushort2 param0, ushort param1, ushort param2)
{return (ushort2)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2)); }

INLINE_OVERLOADABLE ushort3 clamp (ushort3 param0, ushort param1, ushort param2)
{return (ushort3)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                  clamp(param0.s2, param1, param2)); }

INLINE_OVERLOADABLE ushort4 clamp (ushort4 param0, ushort param1, ushort param2)
{return (ushort4)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                  clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2)); }

INLINE_OVERLOADABLE ushort8 clamp (ushort8 param0, ushort param1, ushort param2)
{return (ushort8)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                  clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2),
                  clamp(param0.s4, param1, param2), clamp(param0.s5, param1, param2),
                  clamp(param0.s6, param1, param2), clamp(param0.s7, param1, param2)); }

INLINE_OVERLOADABLE ushort16 clamp (ushort16 param0, ushort param1, ushort param2)
{return (ushort16)(clamp(param0.s0,  param1,  param2),  clamp(param0.s1,  param1,  param2),
                   clamp(param0.s2,  param1,  param2),  clamp(param0.s3,  param1,  param2),
                   clamp(param0.s4,  param1,  param2),  clamp(param0.s5,  param1,  param2),
                   clamp(param0.s6,  param1,  param2),  clamp(param0.s7,  param1,  param2),
                   clamp(param0.s8,  param1,  param2),  clamp(param0.s9,  param1,  param2),
                   clamp(param0.sa, param1, param2), clamp(param0.sb, param1, param2),
                   clamp(param0.sc, param1, param2), clamp(param0.sd, param1, param2),
                   clamp(param0.se, param1, param2), clamp(param0.sf, param1, param2)); }

INLINE_OVERLOADABLE uint2 clamp (uint2 param0, uint param1, uint param2)
{return (uint2)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2)); }

INLINE_OVERLOADABLE uint3 clamp (uint3 param0, uint param1, uint param2)
{return (uint3)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                clamp(param0.s2, param1, param2)); }

INLINE_OVERLOADABLE uint4 clamp (uint4 param0, uint param1, uint param2)
{return (uint4)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2)); }

INLINE_OVERLOADABLE uint8 clamp (uint8 param0, uint param1, uint param2)
{return (uint8)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2),
                clamp(param0.s4, param1, param2), clamp(param0.s5, param1, param2),
                clamp(param0.s6, param1, param2), clamp(param0.s7, param1, param2)); }

INLINE_OVERLOADABLE uint16 clamp (uint16 param0, uint param1, uint param2)
{return (uint16)(clamp(param0.s0,  param1,  param2),  clamp(param0.s1,  param1,  param2),
                 clamp(param0.s2,  param1,  param2),  clamp(param0.s3,  param1,  param2),
                 clamp(param0.s4,  param1,  param2),  clamp(param0.s5,  param1,  param2),
                 clamp(param0.s6,  param1,  param2),  clamp(param0.s7,  param1,  param2),
                 clamp(param0.s8,  param1,  param2),  clamp(param0.s9,  param1,  param2),
                 clamp(param0.sa, param1, param2), clamp(param0.sb, param1, param2),
                 clamp(param0.sc, param1, param2), clamp(param0.sd, param1, param2),
                 clamp(param0.se, param1, param2), clamp(param0.sf, param1, param2)); }


//gentype clz (gentype x)

INLINE_OVERLOADABLE char2 clz (char2 param0)
{return (char2)(clz(param0.s0), clz(param0.s1)); }

INLINE_OVERLOADABLE char3 clz (char3 param0)
{return (char3)(clz(param0.s0), clz(param0.s1),
                clz(param0.s2)); }

INLINE_OVERLOADABLE char4 clz (char4 param0)
{return (char4)(clz(param0.s0), clz(param0.s1),
                clz(param0.s2), clz(param0.s3)); }

INLINE_OVERLOADABLE char8 clz (char8 param0)
{return (char8)(clz(param0.s0), clz(param0.s1),
                clz(param0.s2), clz(param0.s3),
                clz(param0.s4), clz(param0.s5),
                clz(param0.s6), clz(param0.s7)); }

INLINE_OVERLOADABLE char16 clz (char16 param0)
{return (char16)(clz(param0.s0),  clz(param0.s1),
                 clz(param0.s2),  clz(param0.s3),
                 clz(param0.s4),  clz(param0.s5),
                 clz(param0.s6),  clz(param0.s7),
                 clz(param0.s8),  clz(param0.s9),
                 clz(param0.sa), clz(param0.sb),
                 clz(param0.sc), clz(param0.sd),
                 clz(param0.se), clz(param0.sf)); }

INLINE_OVERLOADABLE short2 clz (short2 param0)
{return (short2)(clz(param0.s0), clz(param0.s1)); }

INLINE_OVERLOADABLE short3 clz (short3 param0)
{return (short3)(clz(param0.s0), clz(param0.s1),
                 clz(param0.s2)); }

INLINE_OVERLOADABLE short4 clz (short4 param0)
{return (short4)(clz(param0.s0), clz(param0.s1),
                 clz(param0.s2), clz(param0.s3)); }

INLINE_OVERLOADABLE short8 clz (short8 param0)
{return (short8)(clz(param0.s0), clz(param0.s1),
                 clz(param0.s2), clz(param0.s3),
                 clz(param0.s4), clz(param0.s5),
                 clz(param0.s6), clz(param0.s7)); }

INLINE_OVERLOADABLE short16 clz (short16 param0)
{return (short16)(clz(param0.s0),  clz(param0.s1),
                  clz(param0.s2),  clz(param0.s3),
                  clz(param0.s4),  clz(param0.s5),
                  clz(param0.s6),  clz(param0.s7),
                  clz(param0.s8),  clz(param0.s9),
                  clz(param0.sa), clz(param0.sb),
                  clz(param0.sc), clz(param0.sd),
                  clz(param0.se), clz(param0.sf)); }

INLINE_OVERLOADABLE int2 clz (int2 param0)
{return (int2)(clz(param0.s0), clz(param0.s1)); }

INLINE_OVERLOADABLE int3 clz (int3 param0)
{return (int3)(clz(param0.s0), clz(param0.s1),
               clz(param0.s2)); }

INLINE_OVERLOADABLE int4 clz (int4 param0)
{return (int4)(clz(param0.s0), clz(param0.s1),
               clz(param0.s2), clz(param0.s3)); }

INLINE_OVERLOADABLE int8 clz (int8 param0)
{return (int8)(clz(param0.s0), clz(param0.s1),
               clz(param0.s2), clz(param0.s3),
               clz(param0.s4), clz(param0.s5),
               clz(param0.s6), clz(param0.s7)); }

INLINE_OVERLOADABLE int16 clz (int16 param0)
{return (int16)(clz(param0.s0),  clz(param0.s1),
                clz(param0.s2),  clz(param0.s3),
                clz(param0.s4),  clz(param0.s5),
                clz(param0.s6),  clz(param0.s7),
                clz(param0.s8),  clz(param0.s9),
                clz(param0.sa), clz(param0.sb),
                clz(param0.sc), clz(param0.sd),
                clz(param0.se), clz(param0.sf)); }

INLINE_OVERLOADABLE uchar2 clz (uchar2 param0)
{return (uchar2)(clz(param0.s0), clz(param0.s1)); }

INLINE_OVERLOADABLE uchar3 clz (uchar3 param0)
{return (uchar3)(clz(param0.s0), clz(param0.s1),
                 clz(param0.s2)); }

INLINE_OVERLOADABLE uchar4 clz (uchar4 param0)
{return (uchar4)(clz(param0.s0), clz(param0.s1),
                 clz(param0.s2), clz(param0.s3)); }

INLINE_OVERLOADABLE uchar8 clz (uchar8 param0)
{return (uchar8)(clz(param0.s0), clz(param0.s1),
                 clz(param0.s2), clz(param0.s3),
                 clz(param0.s4), clz(param0.s5),
                 clz(param0.s6), clz(param0.s7)); }

INLINE_OVERLOADABLE uchar16 clz (uchar16 param0)
{return (uchar16)(clz(param0.s0),  clz(param0.s1),
                  clz(param0.s2),  clz(param0.s3),
                  clz(param0.s4),  clz(param0.s5),
                  clz(param0.s6),  clz(param0.s7),
                  clz(param0.s8),  clz(param0.s9),
                  clz(param0.sa), clz(param0.sb),
                  clz(param0.sc), clz(param0.sd),
                  clz(param0.se), clz(param0.sf)); }

INLINE_OVERLOADABLE ushort2 clz (ushort2 param0)
{return (ushort2)(clz(param0.s0), clz(param0.s1)); }

INLINE_OVERLOADABLE ushort3 clz (ushort3 param0)
{return (ushort3)(clz(param0.s0), clz(param0.s1),
                  clz(param0.s2)); }

INLINE_OVERLOADABLE ushort4 clz (ushort4 param0)
{return (ushort4)(clz(param0.s0), clz(param0.s1),
                  clz(param0.s2), clz(param0.s3)); }

INLINE_OVERLOADABLE ushort8 clz (ushort8 param0)
{return (ushort8)(clz(param0.s0), clz(param0.s1),
                  clz(param0.s2), clz(param0.s3),
                  clz(param0.s4), clz(param0.s5),
                  clz(param0.s6), clz(param0.s7)); }

INLINE_OVERLOADABLE ushort16 clz (ushort16 param0)
{return (ushort16)(clz(param0.s0),  clz(param0.s1),
                   clz(param0.s2),  clz(param0.s3),
                   clz(param0.s4),  clz(param0.s5),
                   clz(param0.s6),  clz(param0.s7),
                   clz(param0.s8),  clz(param0.s9),
                   clz(param0.sa), clz(param0.sb),
                   clz(param0.sc), clz(param0.sd),
                   clz(param0.se), clz(param0.sf)); }

INLINE_OVERLOADABLE uint2 clz (uint2 param0)
{return (uint2)(clz(param0.s0), clz(param0.s1)); }

INLINE_OVERLOADABLE uint3 clz (uint3 param0)
{return (uint3)(clz(param0.s0), clz(param0.s1),
                clz(param0.s2)); }

INLINE_OVERLOADABLE uint4 clz (uint4 param0)
{return (uint4)(clz(param0.s0), clz(param0.s1),
                clz(param0.s2), clz(param0.s3)); }

INLINE_OVERLOADABLE uint8 clz (uint8 param0)
{return (uint8)(clz(param0.s0), clz(param0.s1),
                clz(param0.s2), clz(param0.s3),
                clz(param0.s4), clz(param0.s5),
                clz(param0.s6), clz(param0.s7)); }

INLINE_OVERLOADABLE uint16 clz (uint16 param0)
{return (uint16)(clz(param0.s0),  clz(param0.s1),
                 clz(param0.s2),  clz(param0.s3),
                 clz(param0.s4),  clz(param0.s5),
                 clz(param0.s6),  clz(param0.s7),
                 clz(param0.s8),  clz(param0.s9),
                 clz(param0.sa), clz(param0.sb),
                 clz(param0.sc), clz(param0.sd),
                 clz(param0.se), clz(param0.sf)); }


//gentype mad_hi (gentype a, gentype b, gentype c)

INLINE_OVERLOADABLE char2 mad_hi (char2 param0, char2 param1, char2 param2)
{return (char2)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE char3 mad_hi (char3 param0, char3 param1, char3 param2)
{return (char3)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                mad_hi(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE char4 mad_hi (char4 param0, char4 param1, char4 param2)
{return (char4)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                mad_hi(param0.s2, param1.s2, param2.s2), mad_hi(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE char8 mad_hi (char8 param0, char8 param1, char8 param2)
{return (char8)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                mad_hi(param0.s2, param1.s2, param2.s2), mad_hi(param0.s3, param1.s3, param2.s3),
                mad_hi(param0.s4, param1.s4, param2.s4), mad_hi(param0.s5, param1.s5, param2.s5),
                mad_hi(param0.s6, param1.s6, param2.s6), mad_hi(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE char16 mad_hi (char16 param0, char16 param1, char16 param2)
{return (char16)(mad_hi(param0.s0,  param1.s0,  param2.s0),  mad_hi(param0.s1,  param1.s1,  param2.s1),
                 mad_hi(param0.s2,  param1.s2,  param2.s2),  mad_hi(param0.s3,  param1.s3,  param2.s3),
                 mad_hi(param0.s4,  param1.s4,  param2.s4),  mad_hi(param0.s5,  param1.s5,  param2.s5),
                 mad_hi(param0.s6,  param1.s6,  param2.s6),  mad_hi(param0.s7,  param1.s7,  param2.s7),
                 mad_hi(param0.s8,  param1.s8,  param2.s8),  mad_hi(param0.s9,  param1.s9,  param2.s9),
                 mad_hi(param0.sa, param1.sa, param2.sa), mad_hi(param0.sb, param1.sb, param2.sb),
                 mad_hi(param0.sc, param1.sc, param2.sc), mad_hi(param0.sd, param1.sd, param2.sd),
                 mad_hi(param0.se, param1.se, param2.se), mad_hi(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE short2 mad_hi (short2 param0, short2 param1, short2 param2)
{return (short2)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE short3 mad_hi (short3 param0, short3 param1, short3 param2)
{return (short3)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                 mad_hi(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE short4 mad_hi (short4 param0, short4 param1, short4 param2)
{return (short4)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                 mad_hi(param0.s2, param1.s2, param2.s2), mad_hi(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE short8 mad_hi (short8 param0, short8 param1, short8 param2)
{return (short8)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                 mad_hi(param0.s2, param1.s2, param2.s2), mad_hi(param0.s3, param1.s3, param2.s3),
                 mad_hi(param0.s4, param1.s4, param2.s4), mad_hi(param0.s5, param1.s5, param2.s5),
                 mad_hi(param0.s6, param1.s6, param2.s6), mad_hi(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE short16 mad_hi (short16 param0, short16 param1, short16 param2)
{return (short16)(mad_hi(param0.s0,  param1.s0,  param2.s0),  mad_hi(param0.s1,  param1.s1,  param2.s1),
                  mad_hi(param0.s2,  param1.s2,  param2.s2),  mad_hi(param0.s3,  param1.s3,  param2.s3),
                  mad_hi(param0.s4,  param1.s4,  param2.s4),  mad_hi(param0.s5,  param1.s5,  param2.s5),
                  mad_hi(param0.s6,  param1.s6,  param2.s6),  mad_hi(param0.s7,  param1.s7,  param2.s7),
                  mad_hi(param0.s8,  param1.s8,  param2.s8),  mad_hi(param0.s9,  param1.s9,  param2.s9),
                  mad_hi(param0.sa, param1.sa, param2.sa), mad_hi(param0.sb, param1.sb, param2.sb),
                  mad_hi(param0.sc, param1.sc, param2.sc), mad_hi(param0.sd, param1.sd, param2.sd),
                  mad_hi(param0.se, param1.se, param2.se), mad_hi(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE int2 mad_hi (int2 param0, int2 param1, int2 param2)
{return (int2)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE int3 mad_hi (int3 param0, int3 param1, int3 param2)
{return (int3)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
               mad_hi(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE int4 mad_hi (int4 param0, int4 param1, int4 param2)
{return (int4)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
               mad_hi(param0.s2, param1.s2, param2.s2), mad_hi(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE int8 mad_hi (int8 param0, int8 param1, int8 param2)
{return (int8)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
               mad_hi(param0.s2, param1.s2, param2.s2), mad_hi(param0.s3, param1.s3, param2.s3),
               mad_hi(param0.s4, param1.s4, param2.s4), mad_hi(param0.s5, param1.s5, param2.s5),
               mad_hi(param0.s6, param1.s6, param2.s6), mad_hi(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE int16 mad_hi (int16 param0, int16 param1, int16 param2)
{return (int16)(mad_hi(param0.s0,  param1.s0,  param2.s0),  mad_hi(param0.s1,  param1.s1,  param2.s1),
                mad_hi(param0.s2,  param1.s2,  param2.s2),  mad_hi(param0.s3,  param1.s3,  param2.s3),
                mad_hi(param0.s4,  param1.s4,  param2.s4),  mad_hi(param0.s5,  param1.s5,  param2.s5),
                mad_hi(param0.s6,  param1.s6,  param2.s6),  mad_hi(param0.s7,  param1.s7,  param2.s7),
                mad_hi(param0.s8,  param1.s8,  param2.s8),  mad_hi(param0.s9,  param1.s9,  param2.s9),
                mad_hi(param0.sa, param1.sa, param2.sa), mad_hi(param0.sb, param1.sb, param2.sb),
                mad_hi(param0.sc, param1.sc, param2.sc), mad_hi(param0.sd, param1.sd, param2.sd),
                mad_hi(param0.se, param1.se, param2.se), mad_hi(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE uchar2 mad_hi (uchar2 param0, uchar2 param1, uchar2 param2)
{return (uchar2)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE uchar3 mad_hi (uchar3 param0, uchar3 param1, uchar3 param2)
{return (uchar3)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                 mad_hi(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE uchar4 mad_hi (uchar4 param0, uchar4 param1, uchar4 param2)
{return (uchar4)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                 mad_hi(param0.s2, param1.s2, param2.s2), mad_hi(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE uchar8 mad_hi (uchar8 param0, uchar8 param1, uchar8 param2)
{return (uchar8)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                 mad_hi(param0.s2, param1.s2, param2.s2), mad_hi(param0.s3, param1.s3, param2.s3),
                 mad_hi(param0.s4, param1.s4, param2.s4), mad_hi(param0.s5, param1.s5, param2.s5),
                 mad_hi(param0.s6, param1.s6, param2.s6), mad_hi(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE uchar16 mad_hi (uchar16 param0, uchar16 param1, uchar16 param2)
{return (uchar16)(mad_hi(param0.s0,  param1.s0,  param2.s0),  mad_hi(param0.s1,  param1.s1,  param2.s1),
                  mad_hi(param0.s2,  param1.s2,  param2.s2),  mad_hi(param0.s3,  param1.s3,  param2.s3),
                  mad_hi(param0.s4,  param1.s4,  param2.s4),  mad_hi(param0.s5,  param1.s5,  param2.s5),
                  mad_hi(param0.s6,  param1.s6,  param2.s6),  mad_hi(param0.s7,  param1.s7,  param2.s7),
                  mad_hi(param0.s8,  param1.s8,  param2.s8),  mad_hi(param0.s9,  param1.s9,  param2.s9),
                  mad_hi(param0.sa, param1.sa, param2.sa), mad_hi(param0.sb, param1.sb, param2.sb),
                  mad_hi(param0.sc, param1.sc, param2.sc), mad_hi(param0.sd, param1.sd, param2.sd),
                  mad_hi(param0.se, param1.se, param2.se), mad_hi(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE ushort2 mad_hi (ushort2 param0, ushort2 param1, ushort2 param2)
{return (ushort2)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE ushort3 mad_hi (ushort3 param0, ushort3 param1, ushort3 param2)
{return (ushort3)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                  mad_hi(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE ushort4 mad_hi (ushort4 param0, ushort4 param1, ushort4 param2)
{return (ushort4)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                  mad_hi(param0.s2, param1.s2, param2.s2), mad_hi(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE ushort8 mad_hi (ushort8 param0, ushort8 param1, ushort8 param2)
{return (ushort8)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                  mad_hi(param0.s2, param1.s2, param2.s2), mad_hi(param0.s3, param1.s3, param2.s3),
                  mad_hi(param0.s4, param1.s4, param2.s4), mad_hi(param0.s5, param1.s5, param2.s5),
                  mad_hi(param0.s6, param1.s6, param2.s6), mad_hi(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE ushort16 mad_hi (ushort16 param0, ushort16 param1, ushort16 param2)
{return (ushort16)(mad_hi(param0.s0,  param1.s0,  param2.s0),  mad_hi(param0.s1,  param1.s1,  param2.s1),
                   mad_hi(param0.s2,  param1.s2,  param2.s2),  mad_hi(param0.s3,  param1.s3,  param2.s3),
                   mad_hi(param0.s4,  param1.s4,  param2.s4),  mad_hi(param0.s5,  param1.s5,  param2.s5),
                   mad_hi(param0.s6,  param1.s6,  param2.s6),  mad_hi(param0.s7,  param1.s7,  param2.s7),
                   mad_hi(param0.s8,  param1.s8,  param2.s8),  mad_hi(param0.s9,  param1.s9,  param2.s9),
                   mad_hi(param0.sa, param1.sa, param2.sa), mad_hi(param0.sb, param1.sb, param2.sb),
                   mad_hi(param0.sc, param1.sc, param2.sc), mad_hi(param0.sd, param1.sd, param2.sd),
                   mad_hi(param0.se, param1.se, param2.se), mad_hi(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE uint2 mad_hi (uint2 param0, uint2 param1, uint2 param2)
{return (uint2)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE uint3 mad_hi (uint3 param0, uint3 param1, uint3 param2)
{return (uint3)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                mad_hi(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE uint4 mad_hi (uint4 param0, uint4 param1, uint4 param2)
{return (uint4)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                mad_hi(param0.s2, param1.s2, param2.s2), mad_hi(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE uint8 mad_hi (uint8 param0, uint8 param1, uint8 param2)
{return (uint8)(mad_hi(param0.s0, param1.s0, param2.s0), mad_hi(param0.s1, param1.s1, param2.s1),
                mad_hi(param0.s2, param1.s2, param2.s2), mad_hi(param0.s3, param1.s3, param2.s3),
                mad_hi(param0.s4, param1.s4, param2.s4), mad_hi(param0.s5, param1.s5, param2.s5),
                mad_hi(param0.s6, param1.s6, param2.s6), mad_hi(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE uint16 mad_hi (uint16 param0, uint16 param1, uint16 param2)
{return (uint16)(mad_hi(param0.s0,  param1.s0,  param2.s0),  mad_hi(param0.s1,  param1.s1,  param2.s1),
                 mad_hi(param0.s2,  param1.s2,  param2.s2),  mad_hi(param0.s3,  param1.s3,  param2.s3),
                 mad_hi(param0.s4,  param1.s4,  param2.s4),  mad_hi(param0.s5,  param1.s5,  param2.s5),
                 mad_hi(param0.s6,  param1.s6,  param2.s6),  mad_hi(param0.s7,  param1.s7,  param2.s7),
                 mad_hi(param0.s8,  param1.s8,  param2.s8),  mad_hi(param0.s9,  param1.s9,  param2.s9),
                 mad_hi(param0.sa, param1.sa, param2.sa), mad_hi(param0.sb, param1.sb, param2.sb),
                 mad_hi(param0.sc, param1.sc, param2.sc), mad_hi(param0.sd, param1.sd, param2.sd),
                 mad_hi(param0.se, param1.se, param2.se), mad_hi(param0.sf, param1.sf, param2.sf)); }


//gentype mad_sat (gentype a, gentype b, gentype c)

INLINE_OVERLOADABLE char2 mad_sat (char2 param0, char2 param1, char2 param2)
{return (char2)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE char3 mad_sat (char3 param0, char3 param1, char3 param2)
{return (char3)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                mad_sat(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE char4 mad_sat (char4 param0, char4 param1, char4 param2)
{return (char4)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                mad_sat(param0.s2, param1.s2, param2.s2), mad_sat(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE char8 mad_sat (char8 param0, char8 param1, char8 param2)
{return (char8)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                mad_sat(param0.s2, param1.s2, param2.s2), mad_sat(param0.s3, param1.s3, param2.s3),
                mad_sat(param0.s4, param1.s4, param2.s4), mad_sat(param0.s5, param1.s5, param2.s5),
                mad_sat(param0.s6, param1.s6, param2.s6), mad_sat(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE char16 mad_sat (char16 param0, char16 param1, char16 param2)
{return (char16)(mad_sat(param0.s0,  param1.s0,  param2.s0),  mad_sat(param0.s1,  param1.s1,  param2.s1),
                 mad_sat(param0.s2,  param1.s2,  param2.s2),  mad_sat(param0.s3,  param1.s3,  param2.s3),
                 mad_sat(param0.s4,  param1.s4,  param2.s4),  mad_sat(param0.s5,  param1.s5,  param2.s5),
                 mad_sat(param0.s6,  param1.s6,  param2.s6),  mad_sat(param0.s7,  param1.s7,  param2.s7),
                 mad_sat(param0.s8,  param1.s8,  param2.s8),  mad_sat(param0.s9,  param1.s9,  param2.s9),
                 mad_sat(param0.sa, param1.sa, param2.sa), mad_sat(param0.sb, param1.sb, param2.sb),
                 mad_sat(param0.sc, param1.sc, param2.sc), mad_sat(param0.sd, param1.sd, param2.sd),
                 mad_sat(param0.se, param1.se, param2.se), mad_sat(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE short2 mad_sat (short2 param0, short2 param1, short2 param2)
{return (short2)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE short3 mad_sat (short3 param0, short3 param1, short3 param2)
{return (short3)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                 mad_sat(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE short4 mad_sat (short4 param0, short4 param1, short4 param2)
{return (short4)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                 mad_sat(param0.s2, param1.s2, param2.s2), mad_sat(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE short8 mad_sat (short8 param0, short8 param1, short8 param2)
{return (short8)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                 mad_sat(param0.s2, param1.s2, param2.s2), mad_sat(param0.s3, param1.s3, param2.s3),
                 mad_sat(param0.s4, param1.s4, param2.s4), mad_sat(param0.s5, param1.s5, param2.s5),
                 mad_sat(param0.s6, param1.s6, param2.s6), mad_sat(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE short16 mad_sat (short16 param0, short16 param1, short16 param2)
{return (short16)(mad_sat(param0.s0,  param1.s0,  param2.s0),  mad_sat(param0.s1,  param1.s1,  param2.s1),
                  mad_sat(param0.s2,  param1.s2,  param2.s2),  mad_sat(param0.s3,  param1.s3,  param2.s3),
                  mad_sat(param0.s4,  param1.s4,  param2.s4),  mad_sat(param0.s5,  param1.s5,  param2.s5),
                  mad_sat(param0.s6,  param1.s6,  param2.s6),  mad_sat(param0.s7,  param1.s7,  param2.s7),
                  mad_sat(param0.s8,  param1.s8,  param2.s8),  mad_sat(param0.s9,  param1.s9,  param2.s9),
                  mad_sat(param0.sa, param1.sa, param2.sa), mad_sat(param0.sb, param1.sb, param2.sb),
                  mad_sat(param0.sc, param1.sc, param2.sc), mad_sat(param0.sd, param1.sd, param2.sd),
                  mad_sat(param0.se, param1.se, param2.se), mad_sat(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE int2 mad_sat (int2 param0, int2 param1, int2 param2)
{return (int2)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE int3 mad_sat (int3 param0, int3 param1, int3 param2)
{return (int3)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
               mad_sat(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE int4 mad_sat (int4 param0, int4 param1, int4 param2)
{return (int4)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
               mad_sat(param0.s2, param1.s2, param2.s2), mad_sat(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE int8 mad_sat (int8 param0, int8 param1, int8 param2)
{return (int8)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
               mad_sat(param0.s2, param1.s2, param2.s2), mad_sat(param0.s3, param1.s3, param2.s3),
               mad_sat(param0.s4, param1.s4, param2.s4), mad_sat(param0.s5, param1.s5, param2.s5),
               mad_sat(param0.s6, param1.s6, param2.s6), mad_sat(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE int16 mad_sat (int16 param0, int16 param1, int16 param2)
{return (int16)(mad_sat(param0.s0,  param1.s0,  param2.s0),  mad_sat(param0.s1,  param1.s1,  param2.s1),
                mad_sat(param0.s2,  param1.s2,  param2.s2),  mad_sat(param0.s3,  param1.s3,  param2.s3),
                mad_sat(param0.s4,  param1.s4,  param2.s4),  mad_sat(param0.s5,  param1.s5,  param2.s5),
                mad_sat(param0.s6,  param1.s6,  param2.s6),  mad_sat(param0.s7,  param1.s7,  param2.s7),
                mad_sat(param0.s8,  param1.s8,  param2.s8),  mad_sat(param0.s9,  param1.s9,  param2.s9),
                mad_sat(param0.sa, param1.sa, param2.sa), mad_sat(param0.sb, param1.sb, param2.sb),
                mad_sat(param0.sc, param1.sc, param2.sc), mad_sat(param0.sd, param1.sd, param2.sd),
                mad_sat(param0.se, param1.se, param2.se), mad_sat(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE uchar2 mad_sat (uchar2 param0, uchar2 param1, uchar2 param2)
{return (uchar2)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE uchar3 mad_sat (uchar3 param0, uchar3 param1, uchar3 param2)
{return (uchar3)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                 mad_sat(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE uchar4 mad_sat (uchar4 param0, uchar4 param1, uchar4 param2)
{return (uchar4)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                 mad_sat(param0.s2, param1.s2, param2.s2), mad_sat(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE uchar8 mad_sat (uchar8 param0, uchar8 param1, uchar8 param2)
{return (uchar8)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                 mad_sat(param0.s2, param1.s2, param2.s2), mad_sat(param0.s3, param1.s3, param2.s3),
                 mad_sat(param0.s4, param1.s4, param2.s4), mad_sat(param0.s5, param1.s5, param2.s5),
                 mad_sat(param0.s6, param1.s6, param2.s6), mad_sat(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE uchar16 mad_sat (uchar16 param0, uchar16 param1, uchar16 param2)
{return (uchar16)(mad_sat(param0.s0,  param1.s0,  param2.s0),  mad_sat(param0.s1,  param1.s1,  param2.s1),
                  mad_sat(param0.s2,  param1.s2,  param2.s2),  mad_sat(param0.s3,  param1.s3,  param2.s3),
                  mad_sat(param0.s4,  param1.s4,  param2.s4),  mad_sat(param0.s5,  param1.s5,  param2.s5),
                  mad_sat(param0.s6,  param1.s6,  param2.s6),  mad_sat(param0.s7,  param1.s7,  param2.s7),
                  mad_sat(param0.s8,  param1.s8,  param2.s8),  mad_sat(param0.s9,  param1.s9,  param2.s9),
                  mad_sat(param0.sa, param1.sa, param2.sa), mad_sat(param0.sb, param1.sb, param2.sb),
                  mad_sat(param0.sc, param1.sc, param2.sc), mad_sat(param0.sd, param1.sd, param2.sd),
                  mad_sat(param0.se, param1.se, param2.se), mad_sat(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE ushort2 mad_sat (ushort2 param0, ushort2 param1, ushort2 param2)
{return (ushort2)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE ushort3 mad_sat (ushort3 param0, ushort3 param1, ushort3 param2)
{return (ushort3)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                  mad_sat(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE ushort4 mad_sat (ushort4 param0, ushort4 param1, ushort4 param2)
{return (ushort4)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                  mad_sat(param0.s2, param1.s2, param2.s2), mad_sat(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE ushort8 mad_sat (ushort8 param0, ushort8 param1, ushort8 param2)
{return (ushort8)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                  mad_sat(param0.s2, param1.s2, param2.s2), mad_sat(param0.s3, param1.s3, param2.s3),
                  mad_sat(param0.s4, param1.s4, param2.s4), mad_sat(param0.s5, param1.s5, param2.s5),
                  mad_sat(param0.s6, param1.s6, param2.s6), mad_sat(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE ushort16 mad_sat (ushort16 param0, ushort16 param1, ushort16 param2)
{return (ushort16)(mad_sat(param0.s0,  param1.s0,  param2.s0),  mad_sat(param0.s1,  param1.s1,  param2.s1),
                   mad_sat(param0.s2,  param1.s2,  param2.s2),  mad_sat(param0.s3,  param1.s3,  param2.s3),
                   mad_sat(param0.s4,  param1.s4,  param2.s4),  mad_sat(param0.s5,  param1.s5,  param2.s5),
                   mad_sat(param0.s6,  param1.s6,  param2.s6),  mad_sat(param0.s7,  param1.s7,  param2.s7),
                   mad_sat(param0.s8,  param1.s8,  param2.s8),  mad_sat(param0.s9,  param1.s9,  param2.s9),
                   mad_sat(param0.sa, param1.sa, param2.sa), mad_sat(param0.sb, param1.sb, param2.sb),
                   mad_sat(param0.sc, param1.sc, param2.sc), mad_sat(param0.sd, param1.sd, param2.sd),
                   mad_sat(param0.se, param1.se, param2.se), mad_sat(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE uint2 mad_sat (uint2 param0, uint2 param1, uint2 param2)
{return (uint2)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE uint3 mad_sat (uint3 param0, uint3 param1, uint3 param2)
{return (uint3)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                mad_sat(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE uint4 mad_sat (uint4 param0, uint4 param1, uint4 param2)
{return (uint4)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                mad_sat(param0.s2, param1.s2, param2.s2), mad_sat(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE uint8 mad_sat (uint8 param0, uint8 param1, uint8 param2)
{return (uint8)(mad_sat(param0.s0, param1.s0, param2.s0), mad_sat(param0.s1, param1.s1, param2.s1),
                mad_sat(param0.s2, param1.s2, param2.s2), mad_sat(param0.s3, param1.s3, param2.s3),
                mad_sat(param0.s4, param1.s4, param2.s4), mad_sat(param0.s5, param1.s5, param2.s5),
                mad_sat(param0.s6, param1.s6, param2.s6), mad_sat(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE uint16 mad_sat (uint16 param0, uint16 param1, uint16 param2)
{return (uint16)(mad_sat(param0.s0,  param1.s0,  param2.s0),  mad_sat(param0.s1,  param1.s1,  param2.s1),
                 mad_sat(param0.s2,  param1.s2,  param2.s2),  mad_sat(param0.s3,  param1.s3,  param2.s3),
                 mad_sat(param0.s4,  param1.s4,  param2.s4),  mad_sat(param0.s5,  param1.s5,  param2.s5),
                 mad_sat(param0.s6,  param1.s6,  param2.s6),  mad_sat(param0.s7,  param1.s7,  param2.s7),
                 mad_sat(param0.s8,  param1.s8,  param2.s8),  mad_sat(param0.s9,  param1.s9,  param2.s9),
                 mad_sat(param0.sa, param1.sa, param2.sa), mad_sat(param0.sb, param1.sb, param2.sb),
                 mad_sat(param0.sc, param1.sc, param2.sc), mad_sat(param0.sd, param1.sd, param2.sd),
                 mad_sat(param0.se, param1.se, param2.se), mad_sat(param0.sf, param1.sf, param2.sf)); }


//gentype max (gentype x,  gentype y)

INLINE_OVERLOADABLE char2 max (char2 param0, char2 param1)
{return (char2)(max(param0.s0, param1.s0), max(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE char3 max (char3 param0, char3 param1)
{return (char3)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                max(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE char4 max (char4 param0, char4 param1)
{return (char4)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                max(param0.s2, param1.s2), max(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE char8 max (char8 param0, char8 param1)
{return (char8)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                max(param0.s2, param1.s2), max(param0.s3, param1.s3),
                max(param0.s4, param1.s4), max(param0.s5, param1.s5),
                max(param0.s6, param1.s6), max(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE char16 max (char16 param0, char16 param1)
{return (char16)(max(param0.s0,  param1.s0),  max(param0.s1,  param1.s1),
                 max(param0.s2,  param1.s2),  max(param0.s3,  param1.s3),
                 max(param0.s4,  param1.s4),  max(param0.s5,  param1.s5),
                 max(param0.s6,  param1.s6),  max(param0.s7,  param1.s7),
                 max(param0.s8,  param1.s8),  max(param0.s9,  param1.s9),
                 max(param0.sa, param1.sa), max(param0.sb, param1.sb),
                 max(param0.sc, param1.sc), max(param0.sd, param1.sd),
                 max(param0.se, param1.se), max(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE short2 max (short2 param0, short2 param1)
{return (short2)(max(param0.s0, param1.s0), max(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE short3 max (short3 param0, short3 param1)
{return (short3)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                 max(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE short4 max (short4 param0, short4 param1)
{return (short4)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                 max(param0.s2, param1.s2), max(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE short8 max (short8 param0, short8 param1)
{return (short8)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                 max(param0.s2, param1.s2), max(param0.s3, param1.s3),
                 max(param0.s4, param1.s4), max(param0.s5, param1.s5),
                 max(param0.s6, param1.s6), max(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE short16 max (short16 param0, short16 param1)
{return (short16)(max(param0.s0,  param1.s0),  max(param0.s1,  param1.s1),
                  max(param0.s2,  param1.s2),  max(param0.s3,  param1.s3),
                  max(param0.s4,  param1.s4),  max(param0.s5,  param1.s5),
                  max(param0.s6,  param1.s6),  max(param0.s7,  param1.s7),
                  max(param0.s8,  param1.s8),  max(param0.s9,  param1.s9),
                  max(param0.sa, param1.sa), max(param0.sb, param1.sb),
                  max(param0.sc, param1.sc), max(param0.sd, param1.sd),
                  max(param0.se, param1.se), max(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE int2 max (int2 param0, int2 param1)
{return (int2)(max(param0.s0, param1.s0), max(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 max (int3 param0, int3 param1)
{return (int3)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
               max(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 max (int4 param0, int4 param1)
{return (int4)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
               max(param0.s2, param1.s2), max(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 max (int8 param0, int8 param1)
{return (int8)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
               max(param0.s2, param1.s2), max(param0.s3, param1.s3),
               max(param0.s4, param1.s4), max(param0.s5, param1.s5),
               max(param0.s6, param1.s6), max(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 max (int16 param0, int16 param1)
{return (int16)(max(param0.s0,  param1.s0),  max(param0.s1,  param1.s1),
                max(param0.s2,  param1.s2),  max(param0.s3,  param1.s3),
                max(param0.s4,  param1.s4),  max(param0.s5,  param1.s5),
                max(param0.s6,  param1.s6),  max(param0.s7,  param1.s7),
                max(param0.s8,  param1.s8),  max(param0.s9,  param1.s9),
                max(param0.sa, param1.sa), max(param0.sb, param1.sb),
                max(param0.sc, param1.sc), max(param0.sd, param1.sd),
                max(param0.se, param1.se), max(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uchar2 max (uchar2 param0, uchar2 param1)
{return (uchar2)(max(param0.s0, param1.s0), max(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uchar3 max (uchar3 param0, uchar3 param1)
{return (uchar3)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                 max(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uchar4 max (uchar4 param0, uchar4 param1)
{return (uchar4)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                 max(param0.s2, param1.s2), max(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uchar8 max (uchar8 param0, uchar8 param1)
{return (uchar8)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                 max(param0.s2, param1.s2), max(param0.s3, param1.s3),
                 max(param0.s4, param1.s4), max(param0.s5, param1.s5),
                 max(param0.s6, param1.s6), max(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uchar16 max (uchar16 param0, uchar16 param1)
{return (uchar16)(max(param0.s0,  param1.s0),  max(param0.s1,  param1.s1),
                  max(param0.s2,  param1.s2),  max(param0.s3,  param1.s3),
                  max(param0.s4,  param1.s4),  max(param0.s5,  param1.s5),
                  max(param0.s6,  param1.s6),  max(param0.s7,  param1.s7),
                  max(param0.s8,  param1.s8),  max(param0.s9,  param1.s9),
                  max(param0.sa, param1.sa), max(param0.sb, param1.sb),
                  max(param0.sc, param1.sc), max(param0.sd, param1.sd),
                  max(param0.se, param1.se), max(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE ushort2 max (ushort2 param0, ushort2 param1)
{return (ushort2)(max(param0.s0, param1.s0), max(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE ushort3 max (ushort3 param0, ushort3 param1)
{return (ushort3)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                  max(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE ushort4 max (ushort4 param0, ushort4 param1)
{return (ushort4)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                  max(param0.s2, param1.s2), max(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE ushort8 max (ushort8 param0, ushort8 param1)
{return (ushort8)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                  max(param0.s2, param1.s2), max(param0.s3, param1.s3),
                  max(param0.s4, param1.s4), max(param0.s5, param1.s5),
                  max(param0.s6, param1.s6), max(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE ushort16 max (ushort16 param0, ushort16 param1)
{return (ushort16)(max(param0.s0,  param1.s0),  max(param0.s1,  param1.s1),
                   max(param0.s2,  param1.s2),  max(param0.s3,  param1.s3),
                   max(param0.s4,  param1.s4),  max(param0.s5,  param1.s5),
                   max(param0.s6,  param1.s6),  max(param0.s7,  param1.s7),
                   max(param0.s8,  param1.s8),  max(param0.s9,  param1.s9),
                   max(param0.sa, param1.sa), max(param0.sb, param1.sb),
                   max(param0.sc, param1.sc), max(param0.sd, param1.sd),
                   max(param0.se, param1.se), max(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uint2 max (uint2 param0, uint2 param1)
{return (uint2)(max(param0.s0, param1.s0), max(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uint3 max (uint3 param0, uint3 param1)
{return (uint3)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                max(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uint4 max (uint4 param0, uint4 param1)
{return (uint4)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                max(param0.s2, param1.s2), max(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uint8 max (uint8 param0, uint8 param1)
{return (uint8)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                max(param0.s2, param1.s2), max(param0.s3, param1.s3),
                max(param0.s4, param1.s4), max(param0.s5, param1.s5),
                max(param0.s6, param1.s6), max(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uint16 max (uint16 param0, uint16 param1)
{return (uint16)(max(param0.s0,  param1.s0),  max(param0.s1,  param1.s1),
                 max(param0.s2,  param1.s2),  max(param0.s3,  param1.s3),
                 max(param0.s4,  param1.s4),  max(param0.s5,  param1.s5),
                 max(param0.s6,  param1.s6),  max(param0.s7,  param1.s7),
                 max(param0.s8,  param1.s8),  max(param0.s9,  param1.s9),
                 max(param0.sa, param1.sa), max(param0.sb, param1.sb),
                 max(param0.sc, param1.sc), max(param0.sd, param1.sd),
                 max(param0.se, param1.se), max(param0.sf, param1.sf)); }


//gentype max (gentype x,  sgentype y)

INLINE_OVERLOADABLE char2 max (char2 param0, char param1)
{return (char2)(max(param0.s0, param1), max(param0.s1, param1)); }

INLINE_OVERLOADABLE char3 max (char3 param0, char param1)
{return (char3)(max(param0.s0, param1), max(param0.s1, param1),
                max(param0.s2, param1)); }

INLINE_OVERLOADABLE char4 max (char4 param0, char param1)
{return (char4)(max(param0.s0, param1), max(param0.s1, param1),
                max(param0.s2, param1), max(param0.s3, param1)); }

INLINE_OVERLOADABLE char8 max (char8 param0, char param1)
{return (char8)(max(param0.s0, param1), max(param0.s1, param1),
                max(param0.s2, param1), max(param0.s3, param1),
                max(param0.s4, param1), max(param0.s5, param1),
                max(param0.s6, param1), max(param0.s7, param1)); }

INLINE_OVERLOADABLE char16 max (char16 param0, char param1)
{return (char16)(max(param0.s0,  param1),  max(param0.s1,  param1),
                 max(param0.s2,  param1),  max(param0.s3,  param1),
                 max(param0.s4,  param1),  max(param0.s5,  param1),
                 max(param0.s6,  param1),  max(param0.s7,  param1),
                 max(param0.s8,  param1),  max(param0.s9,  param1),
                 max(param0.sa, param1), max(param0.sb, param1),
                 max(param0.sc, param1), max(param0.sd, param1),
                 max(param0.se, param1), max(param0.sf, param1)); }

INLINE_OVERLOADABLE short2 max (short2 param0, short param1)
{return (short2)(max(param0.s0, param1), max(param0.s1, param1)); }

INLINE_OVERLOADABLE short3 max (short3 param0, short param1)
{return (short3)(max(param0.s0, param1), max(param0.s1, param1),
                 max(param0.s2, param1)); }

INLINE_OVERLOADABLE short4 max (short4 param0, short param1)
{return (short4)(max(param0.s0, param1), max(param0.s1, param1),
                 max(param0.s2, param1), max(param0.s3, param1)); }

INLINE_OVERLOADABLE short8 max (short8 param0, short param1)
{return (short8)(max(param0.s0, param1), max(param0.s1, param1),
                 max(param0.s2, param1), max(param0.s3, param1),
                 max(param0.s4, param1), max(param0.s5, param1),
                 max(param0.s6, param1), max(param0.s7, param1)); }

INLINE_OVERLOADABLE short16 max (short16 param0, short param1)
{return (short16)(max(param0.s0,  param1),  max(param0.s1,  param1),
                  max(param0.s2,  param1),  max(param0.s3,  param1),
                  max(param0.s4,  param1),  max(param0.s5,  param1),
                  max(param0.s6,  param1),  max(param0.s7,  param1),
                  max(param0.s8,  param1),  max(param0.s9,  param1),
                  max(param0.sa, param1), max(param0.sb, param1),
                  max(param0.sc, param1), max(param0.sd, param1),
                  max(param0.se, param1), max(param0.sf, param1)); }

INLINE_OVERLOADABLE int2 max (int2 param0, int param1)
{return (int2)(max(param0.s0, param1), max(param0.s1, param1)); }

INLINE_OVERLOADABLE int3 max (int3 param0, int param1)
{return (int3)(max(param0.s0, param1), max(param0.s1, param1),
               max(param0.s2, param1)); }

INLINE_OVERLOADABLE int4 max (int4 param0, int param1)
{return (int4)(max(param0.s0, param1), max(param0.s1, param1),
               max(param0.s2, param1), max(param0.s3, param1)); }

INLINE_OVERLOADABLE int8 max (int8 param0, int param1)
{return (int8)(max(param0.s0, param1), max(param0.s1, param1),
               max(param0.s2, param1), max(param0.s3, param1),
               max(param0.s4, param1), max(param0.s5, param1),
               max(param0.s6, param1), max(param0.s7, param1)); }

INLINE_OVERLOADABLE int16 max (int16 param0, int param1)
{return (int16)(max(param0.s0,  param1),  max(param0.s1,  param1),
                max(param0.s2,  param1),  max(param0.s3,  param1),
                max(param0.s4,  param1),  max(param0.s5,  param1),
                max(param0.s6,  param1),  max(param0.s7,  param1),
                max(param0.s8,  param1),  max(param0.s9,  param1),
                max(param0.sa, param1), max(param0.sb, param1),
                max(param0.sc, param1), max(param0.sd, param1),
                max(param0.se, param1), max(param0.sf, param1)); }

INLINE_OVERLOADABLE uchar2 max (uchar2 param0, uchar param1)
{return (uchar2)(max(param0.s0, param1), max(param0.s1, param1)); }

INLINE_OVERLOADABLE uchar3 max (uchar3 param0, uchar param1)
{return (uchar3)(max(param0.s0, param1), max(param0.s1, param1),
                 max(param0.s2, param1)); }

INLINE_OVERLOADABLE uchar4 max (uchar4 param0, uchar param1)
{return (uchar4)(max(param0.s0, param1), max(param0.s1, param1),
                 max(param0.s2, param1), max(param0.s3, param1)); }

INLINE_OVERLOADABLE uchar8 max (uchar8 param0, uchar param1)
{return (uchar8)(max(param0.s0, param1), max(param0.s1, param1),
                 max(param0.s2, param1), max(param0.s3, param1),
                 max(param0.s4, param1), max(param0.s5, param1),
                 max(param0.s6, param1), max(param0.s7, param1)); }

INLINE_OVERLOADABLE uchar16 max (uchar16 param0, uchar param1)
{return (uchar16)(max(param0.s0,  param1),  max(param0.s1,  param1),
                  max(param0.s2,  param1),  max(param0.s3,  param1),
                  max(param0.s4,  param1),  max(param0.s5,  param1),
                  max(param0.s6,  param1),  max(param0.s7,  param1),
                  max(param0.s8,  param1),  max(param0.s9,  param1),
                  max(param0.sa, param1), max(param0.sb, param1),
                  max(param0.sc, param1), max(param0.sd, param1),
                  max(param0.se, param1), max(param0.sf, param1)); }

INLINE_OVERLOADABLE ushort2 max (ushort2 param0, ushort param1)
{return (ushort2)(max(param0.s0, param1), max(param0.s1, param1)); }

INLINE_OVERLOADABLE ushort3 max (ushort3 param0, ushort param1)
{return (ushort3)(max(param0.s0, param1), max(param0.s1, param1),
                  max(param0.s2, param1)); }

INLINE_OVERLOADABLE ushort4 max (ushort4 param0, ushort param1)
{return (ushort4)(max(param0.s0, param1), max(param0.s1, param1),
                  max(param0.s2, param1), max(param0.s3, param1)); }

INLINE_OVERLOADABLE ushort8 max (ushort8 param0, ushort param1)
{return (ushort8)(max(param0.s0, param1), max(param0.s1, param1),
                  max(param0.s2, param1), max(param0.s3, param1),
                  max(param0.s4, param1), max(param0.s5, param1),
                  max(param0.s6, param1), max(param0.s7, param1)); }

INLINE_OVERLOADABLE ushort16 max (ushort16 param0, ushort param1)
{return (ushort16)(max(param0.s0,  param1),  max(param0.s1,  param1),
                   max(param0.s2,  param1),  max(param0.s3,  param1),
                   max(param0.s4,  param1),  max(param0.s5,  param1),
                   max(param0.s6,  param1),  max(param0.s7,  param1),
                   max(param0.s8,  param1),  max(param0.s9,  param1),
                   max(param0.sa, param1), max(param0.sb, param1),
                   max(param0.sc, param1), max(param0.sd, param1),
                   max(param0.se, param1), max(param0.sf, param1)); }

INLINE_OVERLOADABLE uint2 max (uint2 param0, uint param1)
{return (uint2)(max(param0.s0, param1), max(param0.s1, param1)); }

INLINE_OVERLOADABLE uint3 max (uint3 param0, uint param1)
{return (uint3)(max(param0.s0, param1), max(param0.s1, param1),
                max(param0.s2, param1)); }

INLINE_OVERLOADABLE uint4 max (uint4 param0, uint param1)
{return (uint4)(max(param0.s0, param1), max(param0.s1, param1),
                max(param0.s2, param1), max(param0.s3, param1)); }

INLINE_OVERLOADABLE uint8 max (uint8 param0, uint param1)
{return (uint8)(max(param0.s0, param1), max(param0.s1, param1),
                max(param0.s2, param1), max(param0.s3, param1),
                max(param0.s4, param1), max(param0.s5, param1),
                max(param0.s6, param1), max(param0.s7, param1)); }

INLINE_OVERLOADABLE uint16 max (uint16 param0, uint param1)
{return (uint16)(max(param0.s0,  param1),  max(param0.s1,  param1),
                 max(param0.s2,  param1),  max(param0.s3,  param1),
                 max(param0.s4,  param1),  max(param0.s5,  param1),
                 max(param0.s6,  param1),  max(param0.s7,  param1),
                 max(param0.s8,  param1),  max(param0.s9,  param1),
                 max(param0.sa, param1), max(param0.sb, param1),
                 max(param0.sc, param1), max(param0.sd, param1),
                 max(param0.se, param1), max(param0.sf, param1)); }


//gentype min (gentype x,  gentype y)

INLINE_OVERLOADABLE char2 min (char2 param0, char2 param1)
{return (char2)(min(param0.s0, param1.s0), min(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE char3 min (char3 param0, char3 param1)
{return (char3)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                min(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE char4 min (char4 param0, char4 param1)
{return (char4)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                min(param0.s2, param1.s2), min(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE char8 min (char8 param0, char8 param1)
{return (char8)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                min(param0.s2, param1.s2), min(param0.s3, param1.s3),
                min(param0.s4, param1.s4), min(param0.s5, param1.s5),
                min(param0.s6, param1.s6), min(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE char16 min (char16 param0, char16 param1)
{return (char16)(min(param0.s0,  param1.s0),  min(param0.s1,  param1.s1),
                 min(param0.s2,  param1.s2),  min(param0.s3,  param1.s3),
                 min(param0.s4,  param1.s4),  min(param0.s5,  param1.s5),
                 min(param0.s6,  param1.s6),  min(param0.s7,  param1.s7),
                 min(param0.s8,  param1.s8),  min(param0.s9,  param1.s9),
                 min(param0.sa, param1.sa), min(param0.sb, param1.sb),
                 min(param0.sc, param1.sc), min(param0.sd, param1.sd),
                 min(param0.se, param1.se), min(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE short2 min (short2 param0, short2 param1)
{return (short2)(min(param0.s0, param1.s0), min(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE short3 min (short3 param0, short3 param1)
{return (short3)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                 min(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE short4 min (short4 param0, short4 param1)
{return (short4)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                 min(param0.s2, param1.s2), min(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE short8 min (short8 param0, short8 param1)
{return (short8)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                 min(param0.s2, param1.s2), min(param0.s3, param1.s3),
                 min(param0.s4, param1.s4), min(param0.s5, param1.s5),
                 min(param0.s6, param1.s6), min(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE short16 min (short16 param0, short16 param1)
{return (short16)(min(param0.s0,  param1.s0),  min(param0.s1,  param1.s1),
                  min(param0.s2,  param1.s2),  min(param0.s3,  param1.s3),
                  min(param0.s4,  param1.s4),  min(param0.s5,  param1.s5),
                  min(param0.s6,  param1.s6),  min(param0.s7,  param1.s7),
                  min(param0.s8,  param1.s8),  min(param0.s9,  param1.s9),
                  min(param0.sa, param1.sa), min(param0.sb, param1.sb),
                  min(param0.sc, param1.sc), min(param0.sd, param1.sd),
                  min(param0.se, param1.se), min(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE int2 min (int2 param0, int2 param1)
{return (int2)(min(param0.s0, param1.s0), min(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 min (int3 param0, int3 param1)
{return (int3)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
               min(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 min (int4 param0, int4 param1)
{return (int4)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
               min(param0.s2, param1.s2), min(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 min (int8 param0, int8 param1)
{return (int8)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
               min(param0.s2, param1.s2), min(param0.s3, param1.s3),
               min(param0.s4, param1.s4), min(param0.s5, param1.s5),
               min(param0.s6, param1.s6), min(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 min (int16 param0, int16 param1)
{return (int16)(min(param0.s0,  param1.s0),  min(param0.s1,  param1.s1),
                min(param0.s2,  param1.s2),  min(param0.s3,  param1.s3),
                min(param0.s4,  param1.s4),  min(param0.s5,  param1.s5),
                min(param0.s6,  param1.s6),  min(param0.s7,  param1.s7),
                min(param0.s8,  param1.s8),  min(param0.s9,  param1.s9),
                min(param0.sa, param1.sa), min(param0.sb, param1.sb),
                min(param0.sc, param1.sc), min(param0.sd, param1.sd),
                min(param0.se, param1.se), min(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uchar2 min (uchar2 param0, uchar2 param1)
{return (uchar2)(min(param0.s0, param1.s0), min(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uchar3 min (uchar3 param0, uchar3 param1)
{return (uchar3)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                 min(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uchar4 min (uchar4 param0, uchar4 param1)
{return (uchar4)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                 min(param0.s2, param1.s2), min(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uchar8 min (uchar8 param0, uchar8 param1)
{return (uchar8)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                 min(param0.s2, param1.s2), min(param0.s3, param1.s3),
                 min(param0.s4, param1.s4), min(param0.s5, param1.s5),
                 min(param0.s6, param1.s6), min(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uchar16 min (uchar16 param0, uchar16 param1)
{return (uchar16)(min(param0.s0,  param1.s0),  min(param0.s1,  param1.s1),
                  min(param0.s2,  param1.s2),  min(param0.s3,  param1.s3),
                  min(param0.s4,  param1.s4),  min(param0.s5,  param1.s5),
                  min(param0.s6,  param1.s6),  min(param0.s7,  param1.s7),
                  min(param0.s8,  param1.s8),  min(param0.s9,  param1.s9),
                  min(param0.sa, param1.sa), min(param0.sb, param1.sb),
                  min(param0.sc, param1.sc), min(param0.sd, param1.sd),
                  min(param0.se, param1.se), min(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE ushort2 min (ushort2 param0, ushort2 param1)
{return (ushort2)(min(param0.s0, param1.s0), min(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE ushort3 min (ushort3 param0, ushort3 param1)
{return (ushort3)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                  min(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE ushort4 min (ushort4 param0, ushort4 param1)
{return (ushort4)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                  min(param0.s2, param1.s2), min(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE ushort8 min (ushort8 param0, ushort8 param1)
{return (ushort8)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                  min(param0.s2, param1.s2), min(param0.s3, param1.s3),
                  min(param0.s4, param1.s4), min(param0.s5, param1.s5),
                  min(param0.s6, param1.s6), min(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE ushort16 min (ushort16 param0, ushort16 param1)
{return (ushort16)(min(param0.s0,  param1.s0),  min(param0.s1,  param1.s1),
                   min(param0.s2,  param1.s2),  min(param0.s3,  param1.s3),
                   min(param0.s4,  param1.s4),  min(param0.s5,  param1.s5),
                   min(param0.s6,  param1.s6),  min(param0.s7,  param1.s7),
                   min(param0.s8,  param1.s8),  min(param0.s9,  param1.s9),
                   min(param0.sa, param1.sa), min(param0.sb, param1.sb),
                   min(param0.sc, param1.sc), min(param0.sd, param1.sd),
                   min(param0.se, param1.se), min(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uint2 min (uint2 param0, uint2 param1)
{return (uint2)(min(param0.s0, param1.s0), min(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uint3 min (uint3 param0, uint3 param1)
{return (uint3)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                min(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uint4 min (uint4 param0, uint4 param1)
{return (uint4)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                min(param0.s2, param1.s2), min(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uint8 min (uint8 param0, uint8 param1)
{return (uint8)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                min(param0.s2, param1.s2), min(param0.s3, param1.s3),
                min(param0.s4, param1.s4), min(param0.s5, param1.s5),
                min(param0.s6, param1.s6), min(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uint16 min (uint16 param0, uint16 param1)
{return (uint16)(min(param0.s0,  param1.s0),  min(param0.s1,  param1.s1),
                 min(param0.s2,  param1.s2),  min(param0.s3,  param1.s3),
                 min(param0.s4,  param1.s4),  min(param0.s5,  param1.s5),
                 min(param0.s6,  param1.s6),  min(param0.s7,  param1.s7),
                 min(param0.s8,  param1.s8),  min(param0.s9,  param1.s9),
                 min(param0.sa, param1.sa), min(param0.sb, param1.sb),
                 min(param0.sc, param1.sc), min(param0.sd, param1.sd),
                 min(param0.se, param1.se), min(param0.sf, param1.sf)); }


//gentype min (gentype x,  sgentype y)

INLINE_OVERLOADABLE char2 min (char2 param0, char param1)
{return (char2)(min(param0.s0, param1), min(param0.s1, param1)); }

INLINE_OVERLOADABLE char3 min (char3 param0, char param1)
{return (char3)(min(param0.s0, param1), min(param0.s1, param1),
                min(param0.s2, param1)); }

INLINE_OVERLOADABLE char4 min (char4 param0, char param1)
{return (char4)(min(param0.s0, param1), min(param0.s1, param1),
                min(param0.s2, param1), min(param0.s3, param1)); }

INLINE_OVERLOADABLE char8 min (char8 param0, char param1)
{return (char8)(min(param0.s0, param1), min(param0.s1, param1),
                min(param0.s2, param1), min(param0.s3, param1),
                min(param0.s4, param1), min(param0.s5, param1),
                min(param0.s6, param1), min(param0.s7, param1)); }

INLINE_OVERLOADABLE char16 min (char16 param0, char param1)
{return (char16)(min(param0.s0,  param1),  min(param0.s1,  param1),
                 min(param0.s2,  param1),  min(param0.s3,  param1),
                 min(param0.s4,  param1),  min(param0.s5,  param1),
                 min(param0.s6,  param1),  min(param0.s7,  param1),
                 min(param0.s8,  param1),  min(param0.s9,  param1),
                 min(param0.sa, param1), min(param0.sb, param1),
                 min(param0.sc, param1), min(param0.sd, param1),
                 min(param0.se, param1), min(param0.sf, param1)); }

INLINE_OVERLOADABLE short2 min (short2 param0, short param1)
{return (short2)(min(param0.s0, param1), min(param0.s1, param1)); }

INLINE_OVERLOADABLE short3 min (short3 param0, short param1)
{return (short3)(min(param0.s0, param1), min(param0.s1, param1),
                 min(param0.s2, param1)); }

INLINE_OVERLOADABLE short4 min (short4 param0, short param1)
{return (short4)(min(param0.s0, param1), min(param0.s1, param1),
                 min(param0.s2, param1), min(param0.s3, param1)); }

INLINE_OVERLOADABLE short8 min (short8 param0, short param1)
{return (short8)(min(param0.s0, param1), min(param0.s1, param1),
                 min(param0.s2, param1), min(param0.s3, param1),
                 min(param0.s4, param1), min(param0.s5, param1),
                 min(param0.s6, param1), min(param0.s7, param1)); }

INLINE_OVERLOADABLE short16 min (short16 param0, short param1)
{return (short16)(min(param0.s0,  param1),  min(param0.s1,  param1),
                  min(param0.s2,  param1),  min(param0.s3,  param1),
                  min(param0.s4,  param1),  min(param0.s5,  param1),
                  min(param0.s6,  param1),  min(param0.s7,  param1),
                  min(param0.s8,  param1),  min(param0.s9,  param1),
                  min(param0.sa, param1), min(param0.sb, param1),
                  min(param0.sc, param1), min(param0.sd, param1),
                  min(param0.se, param1), min(param0.sf, param1)); }

INLINE_OVERLOADABLE int2 min (int2 param0, int param1)
{return (int2)(min(param0.s0, param1), min(param0.s1, param1)); }

INLINE_OVERLOADABLE int3 min (int3 param0, int param1)
{return (int3)(min(param0.s0, param1), min(param0.s1, param1),
               min(param0.s2, param1)); }

INLINE_OVERLOADABLE int4 min (int4 param0, int param1)
{return (int4)(min(param0.s0, param1), min(param0.s1, param1),
               min(param0.s2, param1), min(param0.s3, param1)); }

INLINE_OVERLOADABLE int8 min (int8 param0, int param1)
{return (int8)(min(param0.s0, param1), min(param0.s1, param1),
               min(param0.s2, param1), min(param0.s3, param1),
               min(param0.s4, param1), min(param0.s5, param1),
               min(param0.s6, param1), min(param0.s7, param1)); }

INLINE_OVERLOADABLE int16 min (int16 param0, int param1)
{return (int16)(min(param0.s0,  param1),  min(param0.s1,  param1),
                min(param0.s2,  param1),  min(param0.s3,  param1),
                min(param0.s4,  param1),  min(param0.s5,  param1),
                min(param0.s6,  param1),  min(param0.s7,  param1),
                min(param0.s8,  param1),  min(param0.s9,  param1),
                min(param0.sa, param1), min(param0.sb, param1),
                min(param0.sc, param1), min(param0.sd, param1),
                min(param0.se, param1), min(param0.sf, param1)); }

INLINE_OVERLOADABLE uchar2 min (uchar2 param0, uchar param1)
{return (uchar2)(min(param0.s0, param1), min(param0.s1, param1)); }

INLINE_OVERLOADABLE uchar3 min (uchar3 param0, uchar param1)
{return (uchar3)(min(param0.s0, param1), min(param0.s1, param1),
                 min(param0.s2, param1)); }

INLINE_OVERLOADABLE uchar4 min (uchar4 param0, uchar param1)
{return (uchar4)(min(param0.s0, param1), min(param0.s1, param1),
                 min(param0.s2, param1), min(param0.s3, param1)); }

INLINE_OVERLOADABLE uchar8 min (uchar8 param0, uchar param1)
{return (uchar8)(min(param0.s0, param1), min(param0.s1, param1),
                 min(param0.s2, param1), min(param0.s3, param1),
                 min(param0.s4, param1), min(param0.s5, param1),
                 min(param0.s6, param1), min(param0.s7, param1)); }

INLINE_OVERLOADABLE uchar16 min (uchar16 param0, uchar param1)
{return (uchar16)(min(param0.s0,  param1),  min(param0.s1,  param1),
                  min(param0.s2,  param1),  min(param0.s3,  param1),
                  min(param0.s4,  param1),  min(param0.s5,  param1),
                  min(param0.s6,  param1),  min(param0.s7,  param1),
                  min(param0.s8,  param1),  min(param0.s9,  param1),
                  min(param0.sa, param1), min(param0.sb, param1),
                  min(param0.sc, param1), min(param0.sd, param1),
                  min(param0.se, param1), min(param0.sf, param1)); }

INLINE_OVERLOADABLE ushort2 min (ushort2 param0, ushort param1)
{return (ushort2)(min(param0.s0, param1), min(param0.s1, param1)); }

INLINE_OVERLOADABLE ushort3 min (ushort3 param0, ushort param1)
{return (ushort3)(min(param0.s0, param1), min(param0.s1, param1),
                  min(param0.s2, param1)); }

INLINE_OVERLOADABLE ushort4 min (ushort4 param0, ushort param1)
{return (ushort4)(min(param0.s0, param1), min(param0.s1, param1),
                  min(param0.s2, param1), min(param0.s3, param1)); }

INLINE_OVERLOADABLE ushort8 min (ushort8 param0, ushort param1)
{return (ushort8)(min(param0.s0, param1), min(param0.s1, param1),
                  min(param0.s2, param1), min(param0.s3, param1),
                  min(param0.s4, param1), min(param0.s5, param1),
                  min(param0.s6, param1), min(param0.s7, param1)); }

INLINE_OVERLOADABLE ushort16 min (ushort16 param0, ushort param1)
{return (ushort16)(min(param0.s0,  param1),  min(param0.s1,  param1),
                   min(param0.s2,  param1),  min(param0.s3,  param1),
                   min(param0.s4,  param1),  min(param0.s5,  param1),
                   min(param0.s6,  param1),  min(param0.s7,  param1),
                   min(param0.s8,  param1),  min(param0.s9,  param1),
                   min(param0.sa, param1), min(param0.sb, param1),
                   min(param0.sc, param1), min(param0.sd, param1),
                   min(param0.se, param1), min(param0.sf, param1)); }

INLINE_OVERLOADABLE uint2 min (uint2 param0, uint param1)
{return (uint2)(min(param0.s0, param1), min(param0.s1, param1)); }

INLINE_OVERLOADABLE uint3 min (uint3 param0, uint param1)
{return (uint3)(min(param0.s0, param1), min(param0.s1, param1),
                min(param0.s2, param1)); }

INLINE_OVERLOADABLE uint4 min (uint4 param0, uint param1)
{return (uint4)(min(param0.s0, param1), min(param0.s1, param1),
                min(param0.s2, param1), min(param0.s3, param1)); }

INLINE_OVERLOADABLE uint8 min (uint8 param0, uint param1)
{return (uint8)(min(param0.s0, param1), min(param0.s1, param1),
                min(param0.s2, param1), min(param0.s3, param1),
                min(param0.s4, param1), min(param0.s5, param1),
                min(param0.s6, param1), min(param0.s7, param1)); }

INLINE_OVERLOADABLE uint16 min (uint16 param0, uint param1)
{return (uint16)(min(param0.s0,  param1),  min(param0.s1,  param1),
                 min(param0.s2,  param1),  min(param0.s3,  param1),
                 min(param0.s4,  param1),  min(param0.s5,  param1),
                 min(param0.s6,  param1),  min(param0.s7,  param1),
                 min(param0.s8,  param1),  min(param0.s9,  param1),
                 min(param0.sa, param1), min(param0.sb, param1),
                 min(param0.sc, param1), min(param0.sd, param1),
                 min(param0.se, param1), min(param0.sf, param1)); }


//gentype mul_hi (gentype x,  gentype y)

INLINE_OVERLOADABLE char2 mul_hi (char2 param0, char2 param1)
{return (char2)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE char3 mul_hi (char3 param0, char3 param1)
{return (char3)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                mul_hi(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE char4 mul_hi (char4 param0, char4 param1)
{return (char4)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                mul_hi(param0.s2, param1.s2), mul_hi(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE char8 mul_hi (char8 param0, char8 param1)
{return (char8)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                mul_hi(param0.s2, param1.s2), mul_hi(param0.s3, param1.s3),
                mul_hi(param0.s4, param1.s4), mul_hi(param0.s5, param1.s5),
                mul_hi(param0.s6, param1.s6), mul_hi(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE char16 mul_hi (char16 param0, char16 param1)
{return (char16)(mul_hi(param0.s0,  param1.s0),  mul_hi(param0.s1,  param1.s1),
                 mul_hi(param0.s2,  param1.s2),  mul_hi(param0.s3,  param1.s3),
                 mul_hi(param0.s4,  param1.s4),  mul_hi(param0.s5,  param1.s5),
                 mul_hi(param0.s6,  param1.s6),  mul_hi(param0.s7,  param1.s7),
                 mul_hi(param0.s8,  param1.s8),  mul_hi(param0.s9,  param1.s9),
                 mul_hi(param0.sa, param1.sa), mul_hi(param0.sb, param1.sb),
                 mul_hi(param0.sc, param1.sc), mul_hi(param0.sd, param1.sd),
                 mul_hi(param0.se, param1.se), mul_hi(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE short2 mul_hi (short2 param0, short2 param1)
{return (short2)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE short3 mul_hi (short3 param0, short3 param1)
{return (short3)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                 mul_hi(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE short4 mul_hi (short4 param0, short4 param1)
{return (short4)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                 mul_hi(param0.s2, param1.s2), mul_hi(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE short8 mul_hi (short8 param0, short8 param1)
{return (short8)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                 mul_hi(param0.s2, param1.s2), mul_hi(param0.s3, param1.s3),
                 mul_hi(param0.s4, param1.s4), mul_hi(param0.s5, param1.s5),
                 mul_hi(param0.s6, param1.s6), mul_hi(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE short16 mul_hi (short16 param0, short16 param1)
{return (short16)(mul_hi(param0.s0,  param1.s0),  mul_hi(param0.s1,  param1.s1),
                  mul_hi(param0.s2,  param1.s2),  mul_hi(param0.s3,  param1.s3),
                  mul_hi(param0.s4,  param1.s4),  mul_hi(param0.s5,  param1.s5),
                  mul_hi(param0.s6,  param1.s6),  mul_hi(param0.s7,  param1.s7),
                  mul_hi(param0.s8,  param1.s8),  mul_hi(param0.s9,  param1.s9),
                  mul_hi(param0.sa, param1.sa), mul_hi(param0.sb, param1.sb),
                  mul_hi(param0.sc, param1.sc), mul_hi(param0.sd, param1.sd),
                  mul_hi(param0.se, param1.se), mul_hi(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE int2 mul_hi (int2 param0, int2 param1)
{return (int2)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 mul_hi (int3 param0, int3 param1)
{return (int3)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
               mul_hi(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 mul_hi (int4 param0, int4 param1)
{return (int4)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
               mul_hi(param0.s2, param1.s2), mul_hi(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 mul_hi (int8 param0, int8 param1)
{return (int8)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
               mul_hi(param0.s2, param1.s2), mul_hi(param0.s3, param1.s3),
               mul_hi(param0.s4, param1.s4), mul_hi(param0.s5, param1.s5),
               mul_hi(param0.s6, param1.s6), mul_hi(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 mul_hi (int16 param0, int16 param1)
{return (int16)(mul_hi(param0.s0,  param1.s0),  mul_hi(param0.s1,  param1.s1),
                mul_hi(param0.s2,  param1.s2),  mul_hi(param0.s3,  param1.s3),
                mul_hi(param0.s4,  param1.s4),  mul_hi(param0.s5,  param1.s5),
                mul_hi(param0.s6,  param1.s6),  mul_hi(param0.s7,  param1.s7),
                mul_hi(param0.s8,  param1.s8),  mul_hi(param0.s9,  param1.s9),
                mul_hi(param0.sa, param1.sa), mul_hi(param0.sb, param1.sb),
                mul_hi(param0.sc, param1.sc), mul_hi(param0.sd, param1.sd),
                mul_hi(param0.se, param1.se), mul_hi(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uchar2 mul_hi (uchar2 param0, uchar2 param1)
{return (uchar2)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uchar3 mul_hi (uchar3 param0, uchar3 param1)
{return (uchar3)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                 mul_hi(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uchar4 mul_hi (uchar4 param0, uchar4 param1)
{return (uchar4)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                 mul_hi(param0.s2, param1.s2), mul_hi(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uchar8 mul_hi (uchar8 param0, uchar8 param1)
{return (uchar8)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                 mul_hi(param0.s2, param1.s2), mul_hi(param0.s3, param1.s3),
                 mul_hi(param0.s4, param1.s4), mul_hi(param0.s5, param1.s5),
                 mul_hi(param0.s6, param1.s6), mul_hi(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uchar16 mul_hi (uchar16 param0, uchar16 param1)
{return (uchar16)(mul_hi(param0.s0,  param1.s0),  mul_hi(param0.s1,  param1.s1),
                  mul_hi(param0.s2,  param1.s2),  mul_hi(param0.s3,  param1.s3),
                  mul_hi(param0.s4,  param1.s4),  mul_hi(param0.s5,  param1.s5),
                  mul_hi(param0.s6,  param1.s6),  mul_hi(param0.s7,  param1.s7),
                  mul_hi(param0.s8,  param1.s8),  mul_hi(param0.s9,  param1.s9),
                  mul_hi(param0.sa, param1.sa), mul_hi(param0.sb, param1.sb),
                  mul_hi(param0.sc, param1.sc), mul_hi(param0.sd, param1.sd),
                  mul_hi(param0.se, param1.se), mul_hi(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE ushort2 mul_hi (ushort2 param0, ushort2 param1)
{return (ushort2)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE ushort3 mul_hi (ushort3 param0, ushort3 param1)
{return (ushort3)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                  mul_hi(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE ushort4 mul_hi (ushort4 param0, ushort4 param1)
{return (ushort4)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                  mul_hi(param0.s2, param1.s2), mul_hi(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE ushort8 mul_hi (ushort8 param0, ushort8 param1)
{return (ushort8)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                  mul_hi(param0.s2, param1.s2), mul_hi(param0.s3, param1.s3),
                  mul_hi(param0.s4, param1.s4), mul_hi(param0.s5, param1.s5),
                  mul_hi(param0.s6, param1.s6), mul_hi(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE ushort16 mul_hi (ushort16 param0, ushort16 param1)
{return (ushort16)(mul_hi(param0.s0,  param1.s0),  mul_hi(param0.s1,  param1.s1),
                   mul_hi(param0.s2,  param1.s2),  mul_hi(param0.s3,  param1.s3),
                   mul_hi(param0.s4,  param1.s4),  mul_hi(param0.s5,  param1.s5),
                   mul_hi(param0.s6,  param1.s6),  mul_hi(param0.s7,  param1.s7),
                   mul_hi(param0.s8,  param1.s8),  mul_hi(param0.s9,  param1.s9),
                   mul_hi(param0.sa, param1.sa), mul_hi(param0.sb, param1.sb),
                   mul_hi(param0.sc, param1.sc), mul_hi(param0.sd, param1.sd),
                   mul_hi(param0.se, param1.se), mul_hi(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uint2 mul_hi (uint2 param0, uint2 param1)
{return (uint2)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uint3 mul_hi (uint3 param0, uint3 param1)
{return (uint3)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                mul_hi(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uint4 mul_hi (uint4 param0, uint4 param1)
{return (uint4)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                mul_hi(param0.s2, param1.s2), mul_hi(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uint8 mul_hi (uint8 param0, uint8 param1)
{return (uint8)(mul_hi(param0.s0, param1.s0), mul_hi(param0.s1, param1.s1),
                mul_hi(param0.s2, param1.s2), mul_hi(param0.s3, param1.s3),
                mul_hi(param0.s4, param1.s4), mul_hi(param0.s5, param1.s5),
                mul_hi(param0.s6, param1.s6), mul_hi(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uint16 mul_hi (uint16 param0, uint16 param1)
{return (uint16)(mul_hi(param0.s0,  param1.s0),  mul_hi(param0.s1,  param1.s1),
                 mul_hi(param0.s2,  param1.s2),  mul_hi(param0.s3,  param1.s3),
                 mul_hi(param0.s4,  param1.s4),  mul_hi(param0.s5,  param1.s5),
                 mul_hi(param0.s6,  param1.s6),  mul_hi(param0.s7,  param1.s7),
                 mul_hi(param0.s8,  param1.s8),  mul_hi(param0.s9,  param1.s9),
                 mul_hi(param0.sa, param1.sa), mul_hi(param0.sb, param1.sb),
                 mul_hi(param0.sc, param1.sc), mul_hi(param0.sd, param1.sd),
                 mul_hi(param0.se, param1.se), mul_hi(param0.sf, param1.sf)); }


//gentype rotate (gentype v,  gentype i)

INLINE_OVERLOADABLE char2 rotate (char2 param0, char2 param1)
{return (char2)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE char3 rotate (char3 param0, char3 param1)
{return (char3)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                rotate(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE char4 rotate (char4 param0, char4 param1)
{return (char4)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                rotate(param0.s2, param1.s2), rotate(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE char8 rotate (char8 param0, char8 param1)
{return (char8)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                rotate(param0.s2, param1.s2), rotate(param0.s3, param1.s3),
                rotate(param0.s4, param1.s4), rotate(param0.s5, param1.s5),
                rotate(param0.s6, param1.s6), rotate(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE char16 rotate (char16 param0, char16 param1)
{return (char16)(rotate(param0.s0,  param1.s0),  rotate(param0.s1,  param1.s1),
                 rotate(param0.s2,  param1.s2),  rotate(param0.s3,  param1.s3),
                 rotate(param0.s4,  param1.s4),  rotate(param0.s5,  param1.s5),
                 rotate(param0.s6,  param1.s6),  rotate(param0.s7,  param1.s7),
                 rotate(param0.s8,  param1.s8),  rotate(param0.s9,  param1.s9),
                 rotate(param0.sa, param1.sa), rotate(param0.sb, param1.sb),
                 rotate(param0.sc, param1.sc), rotate(param0.sd, param1.sd),
                 rotate(param0.se, param1.se), rotate(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE short2 rotate (short2 param0, short2 param1)
{return (short2)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE short3 rotate (short3 param0, short3 param1)
{return (short3)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                 rotate(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE short4 rotate (short4 param0, short4 param1)
{return (short4)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                 rotate(param0.s2, param1.s2), rotate(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE short8 rotate (short8 param0, short8 param1)
{return (short8)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                 rotate(param0.s2, param1.s2), rotate(param0.s3, param1.s3),
                 rotate(param0.s4, param1.s4), rotate(param0.s5, param1.s5),
                 rotate(param0.s6, param1.s6), rotate(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE short16 rotate (short16 param0, short16 param1)
{return (short16)(rotate(param0.s0,  param1.s0),  rotate(param0.s1,  param1.s1),
                  rotate(param0.s2,  param1.s2),  rotate(param0.s3,  param1.s3),
                  rotate(param0.s4,  param1.s4),  rotate(param0.s5,  param1.s5),
                  rotate(param0.s6,  param1.s6),  rotate(param0.s7,  param1.s7),
                  rotate(param0.s8,  param1.s8),  rotate(param0.s9,  param1.s9),
                  rotate(param0.sa, param1.sa), rotate(param0.sb, param1.sb),
                  rotate(param0.sc, param1.sc), rotate(param0.sd, param1.sd),
                  rotate(param0.se, param1.se), rotate(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE int2 rotate (int2 param0, int2 param1)
{return (int2)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 rotate (int3 param0, int3 param1)
{return (int3)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
               rotate(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 rotate (int4 param0, int4 param1)
{return (int4)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
               rotate(param0.s2, param1.s2), rotate(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 rotate (int8 param0, int8 param1)
{return (int8)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
               rotate(param0.s2, param1.s2), rotate(param0.s3, param1.s3),
               rotate(param0.s4, param1.s4), rotate(param0.s5, param1.s5),
               rotate(param0.s6, param1.s6), rotate(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 rotate (int16 param0, int16 param1)
{return (int16)(rotate(param0.s0,  param1.s0),  rotate(param0.s1,  param1.s1),
                rotate(param0.s2,  param1.s2),  rotate(param0.s3,  param1.s3),
                rotate(param0.s4,  param1.s4),  rotate(param0.s5,  param1.s5),
                rotate(param0.s6,  param1.s6),  rotate(param0.s7,  param1.s7),
                rotate(param0.s8,  param1.s8),  rotate(param0.s9,  param1.s9),
                rotate(param0.sa, param1.sa), rotate(param0.sb, param1.sb),
                rotate(param0.sc, param1.sc), rotate(param0.sd, param1.sd),
                rotate(param0.se, param1.se), rotate(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uchar2 rotate (uchar2 param0, uchar2 param1)
{return (uchar2)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uchar3 rotate (uchar3 param0, uchar3 param1)
{return (uchar3)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                 rotate(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uchar4 rotate (uchar4 param0, uchar4 param1)
{return (uchar4)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                 rotate(param0.s2, param1.s2), rotate(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uchar8 rotate (uchar8 param0, uchar8 param1)
{return (uchar8)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                 rotate(param0.s2, param1.s2), rotate(param0.s3, param1.s3),
                 rotate(param0.s4, param1.s4), rotate(param0.s5, param1.s5),
                 rotate(param0.s6, param1.s6), rotate(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uchar16 rotate (uchar16 param0, uchar16 param1)
{return (uchar16)(rotate(param0.s0,  param1.s0),  rotate(param0.s1,  param1.s1),
                  rotate(param0.s2,  param1.s2),  rotate(param0.s3,  param1.s3),
                  rotate(param0.s4,  param1.s4),  rotate(param0.s5,  param1.s5),
                  rotate(param0.s6,  param1.s6),  rotate(param0.s7,  param1.s7),
                  rotate(param0.s8,  param1.s8),  rotate(param0.s9,  param1.s9),
                  rotate(param0.sa, param1.sa), rotate(param0.sb, param1.sb),
                  rotate(param0.sc, param1.sc), rotate(param0.sd, param1.sd),
                  rotate(param0.se, param1.se), rotate(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE ushort2 rotate (ushort2 param0, ushort2 param1)
{return (ushort2)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE ushort3 rotate (ushort3 param0, ushort3 param1)
{return (ushort3)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                  rotate(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE ushort4 rotate (ushort4 param0, ushort4 param1)
{return (ushort4)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                  rotate(param0.s2, param1.s2), rotate(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE ushort8 rotate (ushort8 param0, ushort8 param1)
{return (ushort8)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                  rotate(param0.s2, param1.s2), rotate(param0.s3, param1.s3),
                  rotate(param0.s4, param1.s4), rotate(param0.s5, param1.s5),
                  rotate(param0.s6, param1.s6), rotate(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE ushort16 rotate (ushort16 param0, ushort16 param1)
{return (ushort16)(rotate(param0.s0,  param1.s0),  rotate(param0.s1,  param1.s1),
                   rotate(param0.s2,  param1.s2),  rotate(param0.s3,  param1.s3),
                   rotate(param0.s4,  param1.s4),  rotate(param0.s5,  param1.s5),
                   rotate(param0.s6,  param1.s6),  rotate(param0.s7,  param1.s7),
                   rotate(param0.s8,  param1.s8),  rotate(param0.s9,  param1.s9),
                   rotate(param0.sa, param1.sa), rotate(param0.sb, param1.sb),
                   rotate(param0.sc, param1.sc), rotate(param0.sd, param1.sd),
                   rotate(param0.se, param1.se), rotate(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uint2 rotate (uint2 param0, uint2 param1)
{return (uint2)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uint3 rotate (uint3 param0, uint3 param1)
{return (uint3)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                rotate(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uint4 rotate (uint4 param0, uint4 param1)
{return (uint4)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                rotate(param0.s2, param1.s2), rotate(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uint8 rotate (uint8 param0, uint8 param1)
{return (uint8)(rotate(param0.s0, param1.s0), rotate(param0.s1, param1.s1),
                rotate(param0.s2, param1.s2), rotate(param0.s3, param1.s3),
                rotate(param0.s4, param1.s4), rotate(param0.s5, param1.s5),
                rotate(param0.s6, param1.s6), rotate(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uint16 rotate (uint16 param0, uint16 param1)
{return (uint16)(rotate(param0.s0,  param1.s0),  rotate(param0.s1,  param1.s1),
                 rotate(param0.s2,  param1.s2),  rotate(param0.s3,  param1.s3),
                 rotate(param0.s4,  param1.s4),  rotate(param0.s5,  param1.s5),
                 rotate(param0.s6,  param1.s6),  rotate(param0.s7,  param1.s7),
                 rotate(param0.s8,  param1.s8),  rotate(param0.s9,  param1.s9),
                 rotate(param0.sa, param1.sa), rotate(param0.sb, param1.sb),
                 rotate(param0.sc, param1.sc), rotate(param0.sd, param1.sd),
                 rotate(param0.se, param1.se), rotate(param0.sf, param1.sf)); }


//gentype sub_sat (gentype x,  gentype y)

INLINE_OVERLOADABLE char2 sub_sat (char2 param0, char2 param1)
{return (char2)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE char3 sub_sat (char3 param0, char3 param1)
{return (char3)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                sub_sat(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE char4 sub_sat (char4 param0, char4 param1)
{return (char4)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                sub_sat(param0.s2, param1.s2), sub_sat(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE char8 sub_sat (char8 param0, char8 param1)
{return (char8)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                sub_sat(param0.s2, param1.s2), sub_sat(param0.s3, param1.s3),
                sub_sat(param0.s4, param1.s4), sub_sat(param0.s5, param1.s5),
                sub_sat(param0.s6, param1.s6), sub_sat(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE char16 sub_sat (char16 param0, char16 param1)
{return (char16)(sub_sat(param0.s0,  param1.s0),  sub_sat(param0.s1,  param1.s1),
                 sub_sat(param0.s2,  param1.s2),  sub_sat(param0.s3,  param1.s3),
                 sub_sat(param0.s4,  param1.s4),  sub_sat(param0.s5,  param1.s5),
                 sub_sat(param0.s6,  param1.s6),  sub_sat(param0.s7,  param1.s7),
                 sub_sat(param0.s8,  param1.s8),  sub_sat(param0.s9,  param1.s9),
                 sub_sat(param0.sa, param1.sa), sub_sat(param0.sb, param1.sb),
                 sub_sat(param0.sc, param1.sc), sub_sat(param0.sd, param1.sd),
                 sub_sat(param0.se, param1.se), sub_sat(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE short2 sub_sat (short2 param0, short2 param1)
{return (short2)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE short3 sub_sat (short3 param0, short3 param1)
{return (short3)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                 sub_sat(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE short4 sub_sat (short4 param0, short4 param1)
{return (short4)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                 sub_sat(param0.s2, param1.s2), sub_sat(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE short8 sub_sat (short8 param0, short8 param1)
{return (short8)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                 sub_sat(param0.s2, param1.s2), sub_sat(param0.s3, param1.s3),
                 sub_sat(param0.s4, param1.s4), sub_sat(param0.s5, param1.s5),
                 sub_sat(param0.s6, param1.s6), sub_sat(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE short16 sub_sat (short16 param0, short16 param1)
{return (short16)(sub_sat(param0.s0,  param1.s0),  sub_sat(param0.s1,  param1.s1),
                  sub_sat(param0.s2,  param1.s2),  sub_sat(param0.s3,  param1.s3),
                  sub_sat(param0.s4,  param1.s4),  sub_sat(param0.s5,  param1.s5),
                  sub_sat(param0.s6,  param1.s6),  sub_sat(param0.s7,  param1.s7),
                  sub_sat(param0.s8,  param1.s8),  sub_sat(param0.s9,  param1.s9),
                  sub_sat(param0.sa, param1.sa), sub_sat(param0.sb, param1.sb),
                  sub_sat(param0.sc, param1.sc), sub_sat(param0.sd, param1.sd),
                  sub_sat(param0.se, param1.se), sub_sat(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE int2 sub_sat (int2 param0, int2 param1)
{return (int2)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 sub_sat (int3 param0, int3 param1)
{return (int3)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
               sub_sat(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 sub_sat (int4 param0, int4 param1)
{return (int4)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
               sub_sat(param0.s2, param1.s2), sub_sat(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 sub_sat (int8 param0, int8 param1)
{return (int8)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
               sub_sat(param0.s2, param1.s2), sub_sat(param0.s3, param1.s3),
               sub_sat(param0.s4, param1.s4), sub_sat(param0.s5, param1.s5),
               sub_sat(param0.s6, param1.s6), sub_sat(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 sub_sat (int16 param0, int16 param1)
{return (int16)(sub_sat(param0.s0,  param1.s0),  sub_sat(param0.s1,  param1.s1),
                sub_sat(param0.s2,  param1.s2),  sub_sat(param0.s3,  param1.s3),
                sub_sat(param0.s4,  param1.s4),  sub_sat(param0.s5,  param1.s5),
                sub_sat(param0.s6,  param1.s6),  sub_sat(param0.s7,  param1.s7),
                sub_sat(param0.s8,  param1.s8),  sub_sat(param0.s9,  param1.s9),
                sub_sat(param0.sa, param1.sa), sub_sat(param0.sb, param1.sb),
                sub_sat(param0.sc, param1.sc), sub_sat(param0.sd, param1.sd),
                sub_sat(param0.se, param1.se), sub_sat(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uchar2 sub_sat (uchar2 param0, uchar2 param1)
{return (uchar2)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uchar3 sub_sat (uchar3 param0, uchar3 param1)
{return (uchar3)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                 sub_sat(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uchar4 sub_sat (uchar4 param0, uchar4 param1)
{return (uchar4)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                 sub_sat(param0.s2, param1.s2), sub_sat(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uchar8 sub_sat (uchar8 param0, uchar8 param1)
{return (uchar8)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                 sub_sat(param0.s2, param1.s2), sub_sat(param0.s3, param1.s3),
                 sub_sat(param0.s4, param1.s4), sub_sat(param0.s5, param1.s5),
                 sub_sat(param0.s6, param1.s6), sub_sat(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uchar16 sub_sat (uchar16 param0, uchar16 param1)
{return (uchar16)(sub_sat(param0.s0,  param1.s0),  sub_sat(param0.s1,  param1.s1),
                  sub_sat(param0.s2,  param1.s2),  sub_sat(param0.s3,  param1.s3),
                  sub_sat(param0.s4,  param1.s4),  sub_sat(param0.s5,  param1.s5),
                  sub_sat(param0.s6,  param1.s6),  sub_sat(param0.s7,  param1.s7),
                  sub_sat(param0.s8,  param1.s8),  sub_sat(param0.s9,  param1.s9),
                  sub_sat(param0.sa, param1.sa), sub_sat(param0.sb, param1.sb),
                  sub_sat(param0.sc, param1.sc), sub_sat(param0.sd, param1.sd),
                  sub_sat(param0.se, param1.se), sub_sat(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE ushort2 sub_sat (ushort2 param0, ushort2 param1)
{return (ushort2)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE ushort3 sub_sat (ushort3 param0, ushort3 param1)
{return (ushort3)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                  sub_sat(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE ushort4 sub_sat (ushort4 param0, ushort4 param1)
{return (ushort4)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                  sub_sat(param0.s2, param1.s2), sub_sat(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE ushort8 sub_sat (ushort8 param0, ushort8 param1)
{return (ushort8)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                  sub_sat(param0.s2, param1.s2), sub_sat(param0.s3, param1.s3),
                  sub_sat(param0.s4, param1.s4), sub_sat(param0.s5, param1.s5),
                  sub_sat(param0.s6, param1.s6), sub_sat(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE ushort16 sub_sat (ushort16 param0, ushort16 param1)
{return (ushort16)(sub_sat(param0.s0,  param1.s0),  sub_sat(param0.s1,  param1.s1),
                   sub_sat(param0.s2,  param1.s2),  sub_sat(param0.s3,  param1.s3),
                   sub_sat(param0.s4,  param1.s4),  sub_sat(param0.s5,  param1.s5),
                   sub_sat(param0.s6,  param1.s6),  sub_sat(param0.s7,  param1.s7),
                   sub_sat(param0.s8,  param1.s8),  sub_sat(param0.s9,  param1.s9),
                   sub_sat(param0.sa, param1.sa), sub_sat(param0.sb, param1.sb),
                   sub_sat(param0.sc, param1.sc), sub_sat(param0.sd, param1.sd),
                   sub_sat(param0.se, param1.se), sub_sat(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE uint2 sub_sat (uint2 param0, uint2 param1)
{return (uint2)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uint3 sub_sat (uint3 param0, uint3 param1)
{return (uint3)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                sub_sat(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uint4 sub_sat (uint4 param0, uint4 param1)
{return (uint4)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                sub_sat(param0.s2, param1.s2), sub_sat(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uint8 sub_sat (uint8 param0, uint8 param1)
{return (uint8)(sub_sat(param0.s0, param1.s0), sub_sat(param0.s1, param1.s1),
                sub_sat(param0.s2, param1.s2), sub_sat(param0.s3, param1.s3),
                sub_sat(param0.s4, param1.s4), sub_sat(param0.s5, param1.s5),
                sub_sat(param0.s6, param1.s6), sub_sat(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uint16 sub_sat (uint16 param0, uint16 param1)
{return (uint16)(sub_sat(param0.s0,  param1.s0),  sub_sat(param0.s1,  param1.s1),
                 sub_sat(param0.s2,  param1.s2),  sub_sat(param0.s3,  param1.s3),
                 sub_sat(param0.s4,  param1.s4),  sub_sat(param0.s5,  param1.s5),
                 sub_sat(param0.s6,  param1.s6),  sub_sat(param0.s7,  param1.s7),
                 sub_sat(param0.s8,  param1.s8),  sub_sat(param0.s9,  param1.s9),
                 sub_sat(param0.sa, param1.sa), sub_sat(param0.sb, param1.sb),
                 sub_sat(param0.sc, param1.sc), sub_sat(param0.sd, param1.sd),
                 sub_sat(param0.se, param1.se), sub_sat(param0.sf, param1.sf)); }


//shortn upsample (charn hi, ucharn lo)

INLINE_OVERLOADABLE short2 upsample (char2 param0, uchar2 param1)
{return (short2)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE short3 upsample (char3 param0, uchar3 param1)
{return (short3)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1),
                 upsample(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE short4 upsample (char4 param0, uchar4 param1)
{return (short4)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1),
                 upsample(param0.s2, param1.s2), upsample(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE short8 upsample (char8 param0, uchar8 param1)
{return (short8)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1),
                 upsample(param0.s2, param1.s2), upsample(param0.s3, param1.s3),
                 upsample(param0.s4, param1.s4), upsample(param0.s5, param1.s5),
                 upsample(param0.s6, param1.s6), upsample(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE short16 upsample (char16 param0, uchar16 param1)
{return (short16)(upsample(param0.s0,  param1.s0),  upsample(param0.s1,  param1.s1),
                  upsample(param0.s2,  param1.s2),  upsample(param0.s3,  param1.s3),
                  upsample(param0.s4,  param1.s4),  upsample(param0.s5,  param1.s5),
                  upsample(param0.s6,  param1.s6),  upsample(param0.s7,  param1.s7),
                  upsample(param0.s8,  param1.s8),  upsample(param0.s9,  param1.s9),
                  upsample(param0.sa, param1.sa), upsample(param0.sb, param1.sb),
                  upsample(param0.sc, param1.sc), upsample(param0.sd, param1.sd),
                  upsample(param0.se, param1.se), upsample(param0.sf, param1.sf)); }


//ushortn upsample (ucharn hi, ucharn lo)

INLINE_OVERLOADABLE ushort2 upsample (uchar2 param0, uchar2 param1)
{return (ushort2)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE ushort3 upsample (uchar3 param0, uchar3 param1)
{return (ushort3)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1),
                  upsample(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE ushort4 upsample (uchar4 param0, uchar4 param1)
{return (ushort4)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1),
                  upsample(param0.s2, param1.s2), upsample(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE ushort8 upsample (uchar8 param0, uchar8 param1)
{return (ushort8)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1),
                  upsample(param0.s2, param1.s2), upsample(param0.s3, param1.s3),
                  upsample(param0.s4, param1.s4), upsample(param0.s5, param1.s5),
                  upsample(param0.s6, param1.s6), upsample(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE ushort16 upsample (uchar16 param0, uchar16 param1)
{return (ushort16)(upsample(param0.s0,  param1.s0),  upsample(param0.s1,  param1.s1),
                   upsample(param0.s2,  param1.s2),  upsample(param0.s3,  param1.s3),
                   upsample(param0.s4,  param1.s4),  upsample(param0.s5,  param1.s5),
                   upsample(param0.s6,  param1.s6),  upsample(param0.s7,  param1.s7),
                   upsample(param0.s8,  param1.s8),  upsample(param0.s9,  param1.s9),
                   upsample(param0.sa, param1.sa), upsample(param0.sb, param1.sb),
                   upsample(param0.sc, param1.sc), upsample(param0.sd, param1.sd),
                   upsample(param0.se, param1.se), upsample(param0.sf, param1.sf)); }


//intn upsample (shortn hi, ushortn lo)

INLINE_OVERLOADABLE int2 upsample (short2 param0, ushort2 param1)
{return (int2)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 upsample (short3 param0, ushort3 param1)
{return (int3)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1),
               upsample(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 upsample (short4 param0, ushort4 param1)
{return (int4)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1),
               upsample(param0.s2, param1.s2), upsample(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 upsample (short8 param0, ushort8 param1)
{return (int8)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1),
               upsample(param0.s2, param1.s2), upsample(param0.s3, param1.s3),
               upsample(param0.s4, param1.s4), upsample(param0.s5, param1.s5),
               upsample(param0.s6, param1.s6), upsample(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 upsample (short16 param0, ushort16 param1)
{return (int16)(upsample(param0.s0,  param1.s0),  upsample(param0.s1,  param1.s1),
                upsample(param0.s2,  param1.s2),  upsample(param0.s3,  param1.s3),
                upsample(param0.s4,  param1.s4),  upsample(param0.s5,  param1.s5),
                upsample(param0.s6,  param1.s6),  upsample(param0.s7,  param1.s7),
                upsample(param0.s8,  param1.s8),  upsample(param0.s9,  param1.s9),
                upsample(param0.sa, param1.sa), upsample(param0.sb, param1.sb),
                upsample(param0.sc, param1.sc), upsample(param0.sd, param1.sd),
                upsample(param0.se, param1.se), upsample(param0.sf, param1.sf)); }


//uintn upsample (ushortn hi, ushortn lo)

INLINE_OVERLOADABLE uint2 upsample (ushort2 param0, ushort2 param1)
{return (uint2)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uint3 upsample (ushort3 param0, ushort3 param1)
{return (uint3)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1),
                upsample(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uint4 upsample (ushort4 param0, ushort4 param1)
{return (uint4)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1),
                upsample(param0.s2, param1.s2), upsample(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uint8 upsample (ushort8 param0, ushort8 param1)
{return (uint8)(upsample(param0.s0, param1.s0), upsample(param0.s1, param1.s1),
                upsample(param0.s2, param1.s2), upsample(param0.s3, param1.s3),
                upsample(param0.s4, param1.s4), upsample(param0.s5, param1.s5),
                upsample(param0.s6, param1.s6), upsample(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uint16 upsample (ushort16 param0, ushort16 param1)
{return (uint16)(upsample(param0.s0,  param1.s0),  upsample(param0.s1,  param1.s1),
                 upsample(param0.s2,  param1.s2),  upsample(param0.s3,  param1.s3),
                 upsample(param0.s4,  param1.s4),  upsample(param0.s5,  param1.s5),
                 upsample(param0.s6,  param1.s6),  upsample(param0.s7,  param1.s7),
                 upsample(param0.s8,  param1.s8),  upsample(param0.s9,  param1.s9),
                 upsample(param0.sa, param1.sa), upsample(param0.sb, param1.sb),
                 upsample(param0.sc, param1.sc), upsample(param0.sd, param1.sd),
                 upsample(param0.se, param1.se), upsample(param0.sf, param1.sf)); }


//longn upsample (intn hi, uintn lo)


//ulongn upsample (uintn hi, uintn lo)


//gentype mad24 (gentype x, gentype y, gentype z)

INLINE_OVERLOADABLE uint2 mad24 (uint2 param0, uint2 param1, uint2 param2)
{return (uint2)(mad24(param0.s0, param1.s0, param2.s0), mad24(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE uint3 mad24 (uint3 param0, uint3 param1, uint3 param2)
{return (uint3)(mad24(param0.s0, param1.s0, param2.s0), mad24(param0.s1, param1.s1, param2.s1),
                mad24(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE uint4 mad24 (uint4 param0, uint4 param1, uint4 param2)
{return (uint4)(mad24(param0.s0, param1.s0, param2.s0), mad24(param0.s1, param1.s1, param2.s1),
                mad24(param0.s2, param1.s2, param2.s2), mad24(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE uint8 mad24 (uint8 param0, uint8 param1, uint8 param2)
{return (uint8)(mad24(param0.s0, param1.s0, param2.s0), mad24(param0.s1, param1.s1, param2.s1),
                mad24(param0.s2, param1.s2, param2.s2), mad24(param0.s3, param1.s3, param2.s3),
                mad24(param0.s4, param1.s4, param2.s4), mad24(param0.s5, param1.s5, param2.s5),
                mad24(param0.s6, param1.s6, param2.s6), mad24(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE uint16 mad24 (uint16 param0, uint16 param1, uint16 param2)
{return (uint16)(mad24(param0.s0,  param1.s0,  param2.s0),  mad24(param0.s1,  param1.s1,  param2.s1),
                 mad24(param0.s2,  param1.s2,  param2.s2),  mad24(param0.s3,  param1.s3,  param2.s3),
                 mad24(param0.s4,  param1.s4,  param2.s4),  mad24(param0.s5,  param1.s5,  param2.s5),
                 mad24(param0.s6,  param1.s6,  param2.s6),  mad24(param0.s7,  param1.s7,  param2.s7),
                 mad24(param0.s8,  param1.s8,  param2.s8),  mad24(param0.s9,  param1.s9,  param2.s9),
                 mad24(param0.sa, param1.sa, param2.sa), mad24(param0.sb, param1.sb, param2.sb),
                 mad24(param0.sc, param1.sc, param2.sc), mad24(param0.sd, param1.sd, param2.sd),
                 mad24(param0.se, param1.se, param2.se), mad24(param0.sf, param1.sf, param2.sf)); }

INLINE_OVERLOADABLE int2 mad24 (int2 param0, int2 param1, int2 param2)
{return (int2)(mad24(param0.s0, param1.s0, param2.s0), mad24(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE int3 mad24 (int3 param0, int3 param1, int3 param2)
{return (int3)(mad24(param0.s0, param1.s0, param2.s0), mad24(param0.s1, param1.s1, param2.s1),
               mad24(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE int4 mad24 (int4 param0, int4 param1, int4 param2)
{return (int4)(mad24(param0.s0, param1.s0, param2.s0), mad24(param0.s1, param1.s1, param2.s1),
               mad24(param0.s2, param1.s2, param2.s2), mad24(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE int8 mad24 (int8 param0, int8 param1, int8 param2)
{return (int8)(mad24(param0.s0, param1.s0, param2.s0), mad24(param0.s1, param1.s1, param2.s1),
               mad24(param0.s2, param1.s2, param2.s2), mad24(param0.s3, param1.s3, param2.s3),
               mad24(param0.s4, param1.s4, param2.s4), mad24(param0.s5, param1.s5, param2.s5),
               mad24(param0.s6, param1.s6, param2.s6), mad24(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE int16 mad24 (int16 param0, int16 param1, int16 param2)
{return (int16)(mad24(param0.s0,  param1.s0,  param2.s0),  mad24(param0.s1,  param1.s1,  param2.s1),
                mad24(param0.s2,  param1.s2,  param2.s2),  mad24(param0.s3,  param1.s3,  param2.s3),
                mad24(param0.s4,  param1.s4,  param2.s4),  mad24(param0.s5,  param1.s5,  param2.s5),
                mad24(param0.s6,  param1.s6,  param2.s6),  mad24(param0.s7,  param1.s7,  param2.s7),
                mad24(param0.s8,  param1.s8,  param2.s8),  mad24(param0.s9,  param1.s9,  param2.s9),
                mad24(param0.sa, param1.sa, param2.sa), mad24(param0.sb, param1.sb, param2.sb),
                mad24(param0.sc, param1.sc, param2.sc), mad24(param0.sd, param1.sd, param2.sd),
                mad24(param0.se, param1.se, param2.se), mad24(param0.sf, param1.sf, param2.sf)); }


//gentype mul24 (gentype x, gentype y)

INLINE_OVERLOADABLE uint2 mul24 (uint2 param0, uint2 param1)
{return (uint2)(mul24(param0.s0, param1.s0), mul24(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE uint3 mul24 (uint3 param0, uint3 param1)
{return (uint3)(mul24(param0.s0, param1.s0), mul24(param0.s1, param1.s1),
                mul24(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE uint4 mul24 (uint4 param0, uint4 param1)
{return (uint4)(mul24(param0.s0, param1.s0), mul24(param0.s1, param1.s1),
                mul24(param0.s2, param1.s2), mul24(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE uint8 mul24 (uint8 param0, uint8 param1)
{return (uint8)(mul24(param0.s0, param1.s0), mul24(param0.s1, param1.s1),
                mul24(param0.s2, param1.s2), mul24(param0.s3, param1.s3),
                mul24(param0.s4, param1.s4), mul24(param0.s5, param1.s5),
                mul24(param0.s6, param1.s6), mul24(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE uint16 mul24 (uint16 param0, uint16 param1)
{return (uint16)(mul24(param0.s0,  param1.s0),  mul24(param0.s1,  param1.s1),
                 mul24(param0.s2,  param1.s2),  mul24(param0.s3,  param1.s3),
                 mul24(param0.s4,  param1.s4),  mul24(param0.s5,  param1.s5),
                 mul24(param0.s6,  param1.s6),  mul24(param0.s7,  param1.s7),
                 mul24(param0.s8,  param1.s8),  mul24(param0.s9,  param1.s9),
                 mul24(param0.sa, param1.sa), mul24(param0.sb, param1.sb),
                 mul24(param0.sc, param1.sc), mul24(param0.sd, param1.sd),
                 mul24(param0.se, param1.se), mul24(param0.sf, param1.sf)); }

INLINE_OVERLOADABLE int2 mul24 (int2 param0, int2 param1)
{return (int2)(mul24(param0.s0, param1.s0), mul24(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 mul24 (int3 param0, int3 param1)
{return (int3)(mul24(param0.s0, param1.s0), mul24(param0.s1, param1.s1),
               mul24(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 mul24 (int4 param0, int4 param1)
{return (int4)(mul24(param0.s0, param1.s0), mul24(param0.s1, param1.s1),
               mul24(param0.s2, param1.s2), mul24(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 mul24 (int8 param0, int8 param1)
{return (int8)(mul24(param0.s0, param1.s0), mul24(param0.s1, param1.s1),
               mul24(param0.s2, param1.s2), mul24(param0.s3, param1.s3),
               mul24(param0.s4, param1.s4), mul24(param0.s5, param1.s5),
               mul24(param0.s6, param1.s6), mul24(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 mul24 (int16 param0, int16 param1)
{return (int16)(mul24(param0.s0,  param1.s0),  mul24(param0.s1,  param1.s1),
                mul24(param0.s2,  param1.s2),  mul24(param0.s3,  param1.s3),
                mul24(param0.s4,  param1.s4),  mul24(param0.s5,  param1.s5),
                mul24(param0.s6,  param1.s6),  mul24(param0.s7,  param1.s7),
                mul24(param0.s8,  param1.s8),  mul24(param0.s9,  param1.s9),
                mul24(param0.sa, param1.sa), mul24(param0.sb, param1.sb),
                mul24(param0.sc, param1.sc), mul24(param0.sd, param1.sd),
                mul24(param0.se, param1.se), mul24(param0.sf, param1.sf)); }


//gentype clamp (gentype x, gentype minval, gentype maxval)

INLINE_OVERLOADABLE float2 clamp (float2 param0, float2 param1, float2 param2)
{return (float2)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE float3 clamp (float3 param0, float3 param1, float3 param2)
{return (float3)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                 clamp(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE float4 clamp (float4 param0, float4 param1, float4 param2)
{return (float4)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                 clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE float8 clamp (float8 param0, float8 param1, float8 param2)
{return (float8)(clamp(param0.s0, param1.s0, param2.s0), clamp(param0.s1, param1.s1, param2.s1),
                 clamp(param0.s2, param1.s2, param2.s2), clamp(param0.s3, param1.s3, param2.s3),
                 clamp(param0.s4, param1.s4, param2.s4), clamp(param0.s5, param1.s5, param2.s5),
                 clamp(param0.s6, param1.s6, param2.s6), clamp(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE float16 clamp (float16 param0, float16 param1, float16 param2)
{return (float16)(clamp(param0.s0,  param1.s0,  param2.s0),  clamp(param0.s1,  param1.s1,  param2.s1),
                  clamp(param0.s2,  param1.s2,  param2.s2),  clamp(param0.s3,  param1.s3,  param2.s3),
                  clamp(param0.s4,  param1.s4,  param2.s4),  clamp(param0.s5,  param1.s5,  param2.s5),
                  clamp(param0.s6,  param1.s6,  param2.s6),  clamp(param0.s7,  param1.s7,  param2.s7),
                  clamp(param0.s8,  param1.s8,  param2.s8),  clamp(param0.s9,  param1.s9,  param2.s9),
                  clamp(param0.sa, param1.sa, param2.sa), clamp(param0.sb, param1.sb, param2.sb),
                  clamp(param0.sc, param1.sc, param2.sc), clamp(param0.sd, param1.sd, param2.sd),
                  clamp(param0.se, param1.se, param2.se), clamp(param0.sf, param1.sf, param2.sf)); }


//gentypef clamp (gentypef x, float minval, float maxval)

INLINE_OVERLOADABLE float2 clamp (float2 param0, float param1, float param2)
{return (float2)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2)); }

INLINE_OVERLOADABLE float3 clamp (float3 param0, float param1, float param2)
{return (float3)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                 clamp(param0.s2, param1, param2)); }

INLINE_OVERLOADABLE float4 clamp (float4 param0, float param1, float param2)
{return (float4)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                 clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2)); }

INLINE_OVERLOADABLE float8 clamp (float8 param0, float param1, float param2)
{return (float8)(clamp(param0.s0, param1, param2), clamp(param0.s1, param1, param2),
                 clamp(param0.s2, param1, param2), clamp(param0.s3, param1, param2),
                 clamp(param0.s4, param1, param2), clamp(param0.s5, param1, param2),
                 clamp(param0.s6, param1, param2), clamp(param0.s7, param1, param2)); }

INLINE_OVERLOADABLE float16 clamp (float16 param0, float param1, float param2)
{return (float16)(clamp(param0.s0,  param1,  param2),  clamp(param0.s1,  param1,  param2),
                  clamp(param0.s2,  param1,  param2),  clamp(param0.s3,  param1,  param2),
                  clamp(param0.s4,  param1,  param2),  clamp(param0.s5,  param1,  param2),
                  clamp(param0.s6,  param1,  param2),  clamp(param0.s7,  param1,  param2),
                  clamp(param0.s8,  param1,  param2),  clamp(param0.s9,  param1,  param2),
                  clamp(param0.sa, param1, param2), clamp(param0.sb, param1, param2),
                  clamp(param0.sc, param1, param2), clamp(param0.sd, param1, param2),
                  clamp(param0.se, param1, param2), clamp(param0.sf, param1, param2)); }


//gentyped clamp (gentyped x, double minval, double maxval)


//gentype degrees (gentype radians)

INLINE_OVERLOADABLE float2 degrees (float2 param0)
{return (float2)(degrees(param0.s0), degrees(param0.s1)); }

INLINE_OVERLOADABLE float3 degrees (float3 param0)
{return (float3)(degrees(param0.s0), degrees(param0.s1),
                 degrees(param0.s2)); }

INLINE_OVERLOADABLE float4 degrees (float4 param0)
{return (float4)(degrees(param0.s0), degrees(param0.s1),
                 degrees(param0.s2), degrees(param0.s3)); }

INLINE_OVERLOADABLE float8 degrees (float8 param0)
{return (float8)(degrees(param0.s0), degrees(param0.s1),
                 degrees(param0.s2), degrees(param0.s3),
                 degrees(param0.s4), degrees(param0.s5),
                 degrees(param0.s6), degrees(param0.s7)); }

INLINE_OVERLOADABLE float16 degrees (float16 param0)
{return (float16)(degrees(param0.s0),  degrees(param0.s1),
                  degrees(param0.s2),  degrees(param0.s3),
                  degrees(param0.s4),  degrees(param0.s5),
                  degrees(param0.s6),  degrees(param0.s7),
                  degrees(param0.s8),  degrees(param0.s9),
                  degrees(param0.sa), degrees(param0.sb),
                  degrees(param0.sc), degrees(param0.sd),
                  degrees(param0.se), degrees(param0.sf)); }


//gentype max (gentype x,  gentype y)

INLINE_OVERLOADABLE float2 max (float2 param0, float2 param1)
{return (float2)(max(param0.s0, param1.s0), max(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 max (float3 param0, float3 param1)
{return (float3)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                 max(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 max (float4 param0, float4 param1)
{return (float4)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                 max(param0.s2, param1.s2), max(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 max (float8 param0, float8 param1)
{return (float8)(max(param0.s0, param1.s0), max(param0.s1, param1.s1),
                 max(param0.s2, param1.s2), max(param0.s3, param1.s3),
                 max(param0.s4, param1.s4), max(param0.s5, param1.s5),
                 max(param0.s6, param1.s6), max(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 max (float16 param0, float16 param1)
{return (float16)(max(param0.s0,  param1.s0),  max(param0.s1,  param1.s1),
                  max(param0.s2,  param1.s2),  max(param0.s3,  param1.s3),
                  max(param0.s4,  param1.s4),  max(param0.s5,  param1.s5),
                  max(param0.s6,  param1.s6),  max(param0.s7,  param1.s7),
                  max(param0.s8,  param1.s8),  max(param0.s9,  param1.s9),
                  max(param0.sa, param1.sa), max(param0.sb, param1.sb),
                  max(param0.sc, param1.sc), max(param0.sd, param1.sd),
                  max(param0.se, param1.se), max(param0.sf, param1.sf)); }


//gentypef max (gentypef x, float y)

INLINE_OVERLOADABLE float2 max (float2 param0, float param1)
{return (float2)(max(param0.s0, param1), max(param0.s1, param1)); }

INLINE_OVERLOADABLE float3 max (float3 param0, float param1)
{return (float3)(max(param0.s0, param1), max(param0.s1, param1),
                 max(param0.s2, param1)); }

INLINE_OVERLOADABLE float4 max (float4 param0, float param1)
{return (float4)(max(param0.s0, param1), max(param0.s1, param1),
                 max(param0.s2, param1), max(param0.s3, param1)); }

INLINE_OVERLOADABLE float8 max (float8 param0, float param1)
{return (float8)(max(param0.s0, param1), max(param0.s1, param1),
                 max(param0.s2, param1), max(param0.s3, param1),
                 max(param0.s4, param1), max(param0.s5, param1),
                 max(param0.s6, param1), max(param0.s7, param1)); }

INLINE_OVERLOADABLE float16 max (float16 param0, float param1)
{return (float16)(max(param0.s0,  param1),  max(param0.s1,  param1),
                  max(param0.s2,  param1),  max(param0.s3,  param1),
                  max(param0.s4,  param1),  max(param0.s5,  param1),
                  max(param0.s6,  param1),  max(param0.s7,  param1),
                  max(param0.s8,  param1),  max(param0.s9,  param1),
                  max(param0.sa, param1), max(param0.sb, param1),
                  max(param0.sc, param1), max(param0.sd, param1),
                  max(param0.se, param1), max(param0.sf, param1)); }


//gentyped max (gentyped x, double y)


//gentype min (gentype x,  gentype y)

INLINE_OVERLOADABLE float2 min (float2 param0, float2 param1)
{return (float2)(min(param0.s0, param1.s0), min(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 min (float3 param0, float3 param1)
{return (float3)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                 min(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 min (float4 param0, float4 param1)
{return (float4)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                 min(param0.s2, param1.s2), min(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 min (float8 param0, float8 param1)
{return (float8)(min(param0.s0, param1.s0), min(param0.s1, param1.s1),
                 min(param0.s2, param1.s2), min(param0.s3, param1.s3),
                 min(param0.s4, param1.s4), min(param0.s5, param1.s5),
                 min(param0.s6, param1.s6), min(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 min (float16 param0, float16 param1)
{return (float16)(min(param0.s0,  param1.s0),  min(param0.s1,  param1.s1),
                  min(param0.s2,  param1.s2),  min(param0.s3,  param1.s3),
                  min(param0.s4,  param1.s4),  min(param0.s5,  param1.s5),
                  min(param0.s6,  param1.s6),  min(param0.s7,  param1.s7),
                  min(param0.s8,  param1.s8),  min(param0.s9,  param1.s9),
                  min(param0.sa, param1.sa), min(param0.sb, param1.sb),
                  min(param0.sc, param1.sc), min(param0.sd, param1.sd),
                  min(param0.se, param1.se), min(param0.sf, param1.sf)); }


//gentypef min (gentypef x,  float y)

INLINE_OVERLOADABLE float2 min (float2 param0, float param1)
{return (float2)(min(param0.s0, param1), min(param0.s1, param1)); }

INLINE_OVERLOADABLE float3 min (float3 param0, float param1)
{return (float3)(min(param0.s0, param1), min(param0.s1, param1),
                 min(param0.s2, param1)); }

INLINE_OVERLOADABLE float4 min (float4 param0, float param1)
{return (float4)(min(param0.s0, param1), min(param0.s1, param1),
                 min(param0.s2, param1), min(param0.s3, param1)); }

INLINE_OVERLOADABLE float8 min (float8 param0, float param1)
{return (float8)(min(param0.s0, param1), min(param0.s1, param1),
                 min(param0.s2, param1), min(param0.s3, param1),
                 min(param0.s4, param1), min(param0.s5, param1),
                 min(param0.s6, param1), min(param0.s7, param1)); }

INLINE_OVERLOADABLE float16 min (float16 param0, float param1)
{return (float16)(min(param0.s0,  param1),  min(param0.s1,  param1),
                  min(param0.s2,  param1),  min(param0.s3,  param1),
                  min(param0.s4,  param1),  min(param0.s5,  param1),
                  min(param0.s6,  param1),  min(param0.s7,  param1),
                  min(param0.s8,  param1),  min(param0.s9,  param1),
                  min(param0.sa, param1), min(param0.sb, param1),
                  min(param0.sc, param1), min(param0.sd, param1),
                  min(param0.se, param1), min(param0.sf, param1)); }


//gentyped min (gentyped x,  double y)


//gentype mix (gentype x, gentype y, gentype a)

INLINE_OVERLOADABLE float2 mix (float2 param0, float2 param1, float2 param2)
{return (float2)(mix(param0.s0, param1.s0, param2.s0), mix(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE float3 mix (float3 param0, float3 param1, float3 param2)
{return (float3)(mix(param0.s0, param1.s0, param2.s0), mix(param0.s1, param1.s1, param2.s1),
                 mix(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE float4 mix (float4 param0, float4 param1, float4 param2)
{return (float4)(mix(param0.s0, param1.s0, param2.s0), mix(param0.s1, param1.s1, param2.s1),
                 mix(param0.s2, param1.s2, param2.s2), mix(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE float8 mix (float8 param0, float8 param1, float8 param2)
{return (float8)(mix(param0.s0, param1.s0, param2.s0), mix(param0.s1, param1.s1, param2.s1),
                 mix(param0.s2, param1.s2, param2.s2), mix(param0.s3, param1.s3, param2.s3),
                 mix(param0.s4, param1.s4, param2.s4), mix(param0.s5, param1.s5, param2.s5),
                 mix(param0.s6, param1.s6, param2.s6), mix(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE float16 mix (float16 param0, float16 param1, float16 param2)
{return (float16)(mix(param0.s0,  param1.s0,  param2.s0),  mix(param0.s1,  param1.s1,  param2.s1),
                  mix(param0.s2,  param1.s2,  param2.s2),  mix(param0.s3,  param1.s3,  param2.s3),
                  mix(param0.s4,  param1.s4,  param2.s4),  mix(param0.s5,  param1.s5,  param2.s5),
                  mix(param0.s6,  param1.s6,  param2.s6),  mix(param0.s7,  param1.s7,  param2.s7),
                  mix(param0.s8,  param1.s8,  param2.s8),  mix(param0.s9,  param1.s9,  param2.s9),
                  mix(param0.sa, param1.sa, param2.sa), mix(param0.sb, param1.sb, param2.sb),
                  mix(param0.sc, param1.sc, param2.sc), mix(param0.sd, param1.sd, param2.sd),
                  mix(param0.se, param1.se, param2.se), mix(param0.sf, param1.sf, param2.sf)); }


//gentypef mix (gentypef x, gentypef y, float a)

INLINE_OVERLOADABLE float2 mix (float2 param0, float2 param1, float param2)
{return (float2)(mix(param0.s0, param1.s0, param2), mix(param0.s1, param1.s1, param2)); }

INLINE_OVERLOADABLE float3 mix (float3 param0, float3 param1, float param2)
{return (float3)(mix(param0.s0, param1.s0, param2), mix(param0.s1, param1.s1, param2),
                 mix(param0.s2, param1.s2, param2)); }

INLINE_OVERLOADABLE float4 mix (float4 param0, float4 param1, float param2)
{return (float4)(mix(param0.s0, param1.s0, param2), mix(param0.s1, param1.s1, param2),
                 mix(param0.s2, param1.s2, param2), mix(param0.s3, param1.s3, param2)); }

INLINE_OVERLOADABLE float8 mix (float8 param0, float8 param1, float param2)
{return (float8)(mix(param0.s0, param1.s0, param2), mix(param0.s1, param1.s1, param2),
                 mix(param0.s2, param1.s2, param2), mix(param0.s3, param1.s3, param2),
                 mix(param0.s4, param1.s4, param2), mix(param0.s5, param1.s5, param2),
                 mix(param0.s6, param1.s6, param2), mix(param0.s7, param1.s7, param2)); }

INLINE_OVERLOADABLE float16 mix (float16 param0, float16 param1, float param2)
{return (float16)(mix(param0.s0,  param1.s0,  param2),  mix(param0.s1,  param1.s1,  param2),
                  mix(param0.s2,  param1.s2,  param2),  mix(param0.s3,  param1.s3,  param2),
                  mix(param0.s4,  param1.s4,  param2),  mix(param0.s5,  param1.s5,  param2),
                  mix(param0.s6,  param1.s6,  param2),  mix(param0.s7,  param1.s7,  param2),
                  mix(param0.s8,  param1.s8,  param2),  mix(param0.s9,  param1.s9,  param2),
                  mix(param0.sa, param1.sa, param2), mix(param0.sb, param1.sb, param2),
                  mix(param0.sc, param1.sc, param2), mix(param0.sd, param1.sd, param2),
                  mix(param0.se, param1.se, param2), mix(param0.sf, param1.sf, param2)); }


//gentyped mix (gentyped x, gentyped y, double a)


//gentype radians (gentype degrees)

INLINE_OVERLOADABLE float2 radians (float2 param0)
{return (float2)(radians(param0.s0), radians(param0.s1)); }

INLINE_OVERLOADABLE float3 radians (float3 param0)
{return (float3)(radians(param0.s0), radians(param0.s1),
                 radians(param0.s2)); }

INLINE_OVERLOADABLE float4 radians (float4 param0)
{return (float4)(radians(param0.s0), radians(param0.s1),
                 radians(param0.s2), radians(param0.s3)); }

INLINE_OVERLOADABLE float8 radians (float8 param0)
{return (float8)(radians(param0.s0), radians(param0.s1),
                 radians(param0.s2), radians(param0.s3),
                 radians(param0.s4), radians(param0.s5),
                 radians(param0.s6), radians(param0.s7)); }

INLINE_OVERLOADABLE float16 radians (float16 param0)
{return (float16)(radians(param0.s0),  radians(param0.s1),
                  radians(param0.s2),  radians(param0.s3),
                  radians(param0.s4),  radians(param0.s5),
                  radians(param0.s6),  radians(param0.s7),
                  radians(param0.s8),  radians(param0.s9),
                  radians(param0.sa), radians(param0.sb),
                  radians(param0.sc), radians(param0.sd),
                  radians(param0.se), radians(param0.sf)); }


//gentype step (gentype edge, gentype x)

INLINE_OVERLOADABLE float2 step (float2 param0, float2 param1)
{return (float2)(step(param0.s0, param1.s0), step(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE float3 step (float3 param0, float3 param1)
{return (float3)(step(param0.s0, param1.s0), step(param0.s1, param1.s1),
                 step(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE float4 step (float4 param0, float4 param1)
{return (float4)(step(param0.s0, param1.s0), step(param0.s1, param1.s1),
                 step(param0.s2, param1.s2), step(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE float8 step (float8 param0, float8 param1)
{return (float8)(step(param0.s0, param1.s0), step(param0.s1, param1.s1),
                 step(param0.s2, param1.s2), step(param0.s3, param1.s3),
                 step(param0.s4, param1.s4), step(param0.s5, param1.s5),
                 step(param0.s6, param1.s6), step(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE float16 step (float16 param0, float16 param1)
{return (float16)(step(param0.s0,  param1.s0),  step(param0.s1,  param1.s1),
                  step(param0.s2,  param1.s2),  step(param0.s3,  param1.s3),
                  step(param0.s4,  param1.s4),  step(param0.s5,  param1.s5),
                  step(param0.s6,  param1.s6),  step(param0.s7,  param1.s7),
                  step(param0.s8,  param1.s8),  step(param0.s9,  param1.s9),
                  step(param0.sa, param1.sa), step(param0.sb, param1.sb),
                  step(param0.sc, param1.sc), step(param0.sd, param1.sd),
                  step(param0.se, param1.se), step(param0.sf, param1.sf)); }


//gentypef step (float edge, gentypef x)

INLINE_OVERLOADABLE float2 step (float param0, float2 param1)
{return (float2)(step(param0, param1.s0), step(param0, param1.s1)); }

INLINE_OVERLOADABLE float3 step (float param0, float3 param1)
{return (float3)(step(param0, param1.s0), step(param0, param1.s1),
                 step(param0, param1.s2)); }

INLINE_OVERLOADABLE float4 step (float param0, float4 param1)
{return (float4)(step(param0, param1.s0), step(param0, param1.s1),
                 step(param0, param1.s2), step(param0, param1.s3)); }

INLINE_OVERLOADABLE float8 step (float param0, float8 param1)
{return (float8)(step(param0, param1.s0), step(param0, param1.s1),
                 step(param0, param1.s2), step(param0, param1.s3),
                 step(param0, param1.s4), step(param0, param1.s5),
                 step(param0, param1.s6), step(param0, param1.s7)); }

INLINE_OVERLOADABLE float16 step (float param0, float16 param1)
{return (float16)(step(param0,  param1.s0),  step(param0,  param1.s1),
                  step(param0,  param1.s2),  step(param0,  param1.s3),
                  step(param0,  param1.s4),  step(param0,  param1.s5),
                  step(param0,  param1.s6),  step(param0,  param1.s7),
                  step(param0,  param1.s8),  step(param0,  param1.s9),
                  step(param0, param1.sa), step(param0, param1.sb),
                  step(param0, param1.sc), step(param0, param1.sd),
                  step(param0, param1.se), step(param0, param1.sf)); }


//gentyped step (double edge, gentyped x)


//gentype smoothstep (gentype edge0, gentype edge1, gentype x)

INLINE_OVERLOADABLE float2 smoothstep (float2 param0, float2 param1, float2 param2)
{return (float2)(smoothstep(param0.s0, param1.s0, param2.s0), smoothstep(param0.s1, param1.s1, param2.s1)); }

INLINE_OVERLOADABLE float3 smoothstep (float3 param0, float3 param1, float3 param2)
{return (float3)(smoothstep(param0.s0, param1.s0, param2.s0), smoothstep(param0.s1, param1.s1, param2.s1),
                 smoothstep(param0.s2, param1.s2, param2.s2)); }

INLINE_OVERLOADABLE float4 smoothstep (float4 param0, float4 param1, float4 param2)
{return (float4)(smoothstep(param0.s0, param1.s0, param2.s0), smoothstep(param0.s1, param1.s1, param2.s1),
                 smoothstep(param0.s2, param1.s2, param2.s2), smoothstep(param0.s3, param1.s3, param2.s3)); }

INLINE_OVERLOADABLE float8 smoothstep (float8 param0, float8 param1, float8 param2)
{return (float8)(smoothstep(param0.s0, param1.s0, param2.s0), smoothstep(param0.s1, param1.s1, param2.s1),
                 smoothstep(param0.s2, param1.s2, param2.s2), smoothstep(param0.s3, param1.s3, param2.s3),
                 smoothstep(param0.s4, param1.s4, param2.s4), smoothstep(param0.s5, param1.s5, param2.s5),
                 smoothstep(param0.s6, param1.s6, param2.s6), smoothstep(param0.s7, param1.s7, param2.s7)); }

INLINE_OVERLOADABLE float16 smoothstep (float16 param0, float16 param1, float16 param2)
{return (float16)(smoothstep(param0.s0,  param1.s0,  param2.s0),  smoothstep(param0.s1,  param1.s1,  param2.s1),
                  smoothstep(param0.s2,  param1.s2,  param2.s2),  smoothstep(param0.s3,  param1.s3,  param2.s3),
                  smoothstep(param0.s4,  param1.s4,  param2.s4),  smoothstep(param0.s5,  param1.s5,  param2.s5),
                  smoothstep(param0.s6,  param1.s6,  param2.s6),  smoothstep(param0.s7,  param1.s7,  param2.s7),
                  smoothstep(param0.s8,  param1.s8,  param2.s8),  smoothstep(param0.s9,  param1.s9,  param2.s9),
                  smoothstep(param0.sa, param1.sa, param2.sa), smoothstep(param0.sb, param1.sb, param2.sb),
                  smoothstep(param0.sc, param1.sc, param2.sc), smoothstep(param0.sd, param1.sd, param2.sd),
                  smoothstep(param0.se, param1.se, param2.se), smoothstep(param0.sf, param1.sf, param2.sf)); }


//gentypef smoothstep (float edge0, float edge1, gentypef x)

INLINE_OVERLOADABLE float2 smoothstep (float param0, float param1, float2 param2)
{return (float2)(smoothstep(param0, param1, param2.s0), smoothstep(param0, param1, param2.s1)); }

INLINE_OVERLOADABLE float3 smoothstep (float param0, float param1, float3 param2)
{return (float3)(smoothstep(param0, param1, param2.s0), smoothstep(param0, param1, param2.s1),
                 smoothstep(param0, param1, param2.s2)); }

INLINE_OVERLOADABLE float4 smoothstep (float param0, float param1, float4 param2)
{return (float4)(smoothstep(param0, param1, param2.s0), smoothstep(param0, param1, param2.s1),
                 smoothstep(param0, param1, param2.s2), smoothstep(param0, param1, param2.s3)); }

INLINE_OVERLOADABLE float8 smoothstep (float param0, float param1, float8 param2)
{return (float8)(smoothstep(param0, param1, param2.s0), smoothstep(param0, param1, param2.s1),
                 smoothstep(param0, param1, param2.s2), smoothstep(param0, param1, param2.s3),
                 smoothstep(param0, param1, param2.s4), smoothstep(param0, param1, param2.s5),
                 smoothstep(param0, param1, param2.s6), smoothstep(param0, param1, param2.s7)); }

INLINE_OVERLOADABLE float16 smoothstep (float param0, float param1, float16 param2)
{return (float16)(smoothstep(param0,  param1,  param2.s0),  smoothstep(param0,  param1,  param2.s1),
                  smoothstep(param0,  param1,  param2.s2),  smoothstep(param0,  param1,  param2.s3),
                  smoothstep(param0,  param1,  param2.s4),  smoothstep(param0,  param1,  param2.s5),
                  smoothstep(param0,  param1,  param2.s6),  smoothstep(param0,  param1,  param2.s7),
                  smoothstep(param0,  param1,  param2.s8),  smoothstep(param0,  param1,  param2.s9),
                  smoothstep(param0, param1, param2.sa), smoothstep(param0, param1, param2.sb),
                  smoothstep(param0, param1, param2.sc), smoothstep(param0, param1, param2.sd),
                  smoothstep(param0, param1, param2.se), smoothstep(param0, param1, param2.sf)); }


//gentyped smoothstep (double edge0, double edge1, gentyped x)


//gentype sign (gentype x)

INLINE_OVERLOADABLE float2 sign (float2 param0)
{return (float2)(sign(param0.s0), sign(param0.s1)); }

INLINE_OVERLOADABLE float3 sign (float3 param0)
{return (float3)(sign(param0.s0), sign(param0.s1),
                 sign(param0.s2)); }

INLINE_OVERLOADABLE float4 sign (float4 param0)
{return (float4)(sign(param0.s0), sign(param0.s1),
                 sign(param0.s2), sign(param0.s3)); }

INLINE_OVERLOADABLE float8 sign (float8 param0)
{return (float8)(sign(param0.s0), sign(param0.s1),
                 sign(param0.s2), sign(param0.s3),
                 sign(param0.s4), sign(param0.s5),
                 sign(param0.s6), sign(param0.s7)); }

INLINE_OVERLOADABLE float16 sign (float16 param0)
{return (float16)(sign(param0.s0),  sign(param0.s1),
                  sign(param0.s2),  sign(param0.s3),
                  sign(param0.s4),  sign(param0.s5),
                  sign(param0.s6),  sign(param0.s7),
                  sign(param0.s8),  sign(param0.s9),
                  sign(param0.sa), sign(param0.sb),
                  sign(param0.sc), sign(param0.sd),
                  sign(param0.se), sign(param0.sf)); }


//intn isequal (floatn x, floatn y)

INLINE_OVERLOADABLE int2 isequal (float2 param0, float2 param1)
{return (int2)(isequal(param0.s0, param1.s0), isequal(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 isequal (float3 param0, float3 param1)
{return (int3)(isequal(param0.s0, param1.s0), isequal(param0.s1, param1.s1),
               isequal(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 isequal (float4 param0, float4 param1)
{return (int4)(isequal(param0.s0, param1.s0), isequal(param0.s1, param1.s1),
               isequal(param0.s2, param1.s2), isequal(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 isequal (float8 param0, float8 param1)
{return (int8)(isequal(param0.s0, param1.s0), isequal(param0.s1, param1.s1),
               isequal(param0.s2, param1.s2), isequal(param0.s3, param1.s3),
               isequal(param0.s4, param1.s4), isequal(param0.s5, param1.s5),
               isequal(param0.s6, param1.s6), isequal(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 isequal (float16 param0, float16 param1)
{return (int16)(isequal(param0.s0,  param1.s0),  isequal(param0.s1,  param1.s1),
                isequal(param0.s2,  param1.s2),  isequal(param0.s3,  param1.s3),
                isequal(param0.s4,  param1.s4),  isequal(param0.s5,  param1.s5),
                isequal(param0.s6,  param1.s6),  isequal(param0.s7,  param1.s7),
                isequal(param0.s8,  param1.s8),  isequal(param0.s9,  param1.s9),
                isequal(param0.sa, param1.sa), isequal(param0.sb, param1.sb),
                isequal(param0.sc, param1.sc), isequal(param0.sd, param1.sd),
                isequal(param0.se, param1.se), isequal(param0.sf, param1.sf)); }


//longn isequal (doublen x, doublen y)


//intn isnotequal (floatn x, floatn y)

INLINE_OVERLOADABLE int2 isnotequal (float2 param0, float2 param1)
{return (int2)(isnotequal(param0.s0, param1.s0), isnotequal(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 isnotequal (float3 param0, float3 param1)
{return (int3)(isnotequal(param0.s0, param1.s0), isnotequal(param0.s1, param1.s1),
               isnotequal(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 isnotequal (float4 param0, float4 param1)
{return (int4)(isnotequal(param0.s0, param1.s0), isnotequal(param0.s1, param1.s1),
               isnotequal(param0.s2, param1.s2), isnotequal(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 isnotequal (float8 param0, float8 param1)
{return (int8)(isnotequal(param0.s0, param1.s0), isnotequal(param0.s1, param1.s1),
               isnotequal(param0.s2, param1.s2), isnotequal(param0.s3, param1.s3),
               isnotequal(param0.s4, param1.s4), isnotequal(param0.s5, param1.s5),
               isnotequal(param0.s6, param1.s6), isnotequal(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 isnotequal (float16 param0, float16 param1)
{return (int16)(isnotequal(param0.s0,  param1.s0),  isnotequal(param0.s1,  param1.s1),
                isnotequal(param0.s2,  param1.s2),  isnotequal(param0.s3,  param1.s3),
                isnotequal(param0.s4,  param1.s4),  isnotequal(param0.s5,  param1.s5),
                isnotequal(param0.s6,  param1.s6),  isnotequal(param0.s7,  param1.s7),
                isnotequal(param0.s8,  param1.s8),  isnotequal(param0.s9,  param1.s9),
                isnotequal(param0.sa, param1.sa), isnotequal(param0.sb, param1.sb),
                isnotequal(param0.sc, param1.sc), isnotequal(param0.sd, param1.sd),
                isnotequal(param0.se, param1.se), isnotequal(param0.sf, param1.sf)); }


//longn isnotequal (doublen x, doublen y)


//intn isgreater (floatn x, floatn y)

INLINE_OVERLOADABLE int2 isgreater (float2 param0, float2 param1)
{return (int2)(isgreater(param0.s0, param1.s0), isgreater(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 isgreater (float3 param0, float3 param1)
{return (int3)(isgreater(param0.s0, param1.s0), isgreater(param0.s1, param1.s1),
               isgreater(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 isgreater (float4 param0, float4 param1)
{return (int4)(isgreater(param0.s0, param1.s0), isgreater(param0.s1, param1.s1),
               isgreater(param0.s2, param1.s2), isgreater(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 isgreater (float8 param0, float8 param1)
{return (int8)(isgreater(param0.s0, param1.s0), isgreater(param0.s1, param1.s1),
               isgreater(param0.s2, param1.s2), isgreater(param0.s3, param1.s3),
               isgreater(param0.s4, param1.s4), isgreater(param0.s5, param1.s5),
               isgreater(param0.s6, param1.s6), isgreater(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 isgreater (float16 param0, float16 param1)
{return (int16)(isgreater(param0.s0,  param1.s0),  isgreater(param0.s1,  param1.s1),
                isgreater(param0.s2,  param1.s2),  isgreater(param0.s3,  param1.s3),
                isgreater(param0.s4,  param1.s4),  isgreater(param0.s5,  param1.s5),
                isgreater(param0.s6,  param1.s6),  isgreater(param0.s7,  param1.s7),
                isgreater(param0.s8,  param1.s8),  isgreater(param0.s9,  param1.s9),
                isgreater(param0.sa, param1.sa), isgreater(param0.sb, param1.sb),
                isgreater(param0.sc, param1.sc), isgreater(param0.sd, param1.sd),
                isgreater(param0.se, param1.se), isgreater(param0.sf, param1.sf)); }


//longn isgreater (doublen x, doublen y)


//intn isgreaterequal (floatn x, floatn y)

INLINE_OVERLOADABLE int2 isgreaterequal (float2 param0, float2 param1)
{return (int2)(isgreaterequal(param0.s0, param1.s0), isgreaterequal(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 isgreaterequal (float3 param0, float3 param1)
{return (int3)(isgreaterequal(param0.s0, param1.s0), isgreaterequal(param0.s1, param1.s1),
               isgreaterequal(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 isgreaterequal (float4 param0, float4 param1)
{return (int4)(isgreaterequal(param0.s0, param1.s0), isgreaterequal(param0.s1, param1.s1),
               isgreaterequal(param0.s2, param1.s2), isgreaterequal(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 isgreaterequal (float8 param0, float8 param1)
{return (int8)(isgreaterequal(param0.s0, param1.s0), isgreaterequal(param0.s1, param1.s1),
               isgreaterequal(param0.s2, param1.s2), isgreaterequal(param0.s3, param1.s3),
               isgreaterequal(param0.s4, param1.s4), isgreaterequal(param0.s5, param1.s5),
               isgreaterequal(param0.s6, param1.s6), isgreaterequal(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 isgreaterequal (float16 param0, float16 param1)
{return (int16)(isgreaterequal(param0.s0,  param1.s0),  isgreaterequal(param0.s1,  param1.s1),
                isgreaterequal(param0.s2,  param1.s2),  isgreaterequal(param0.s3,  param1.s3),
                isgreaterequal(param0.s4,  param1.s4),  isgreaterequal(param0.s5,  param1.s5),
                isgreaterequal(param0.s6,  param1.s6),  isgreaterequal(param0.s7,  param1.s7),
                isgreaterequal(param0.s8,  param1.s8),  isgreaterequal(param0.s9,  param1.s9),
                isgreaterequal(param0.sa, param1.sa), isgreaterequal(param0.sb, param1.sb),
                isgreaterequal(param0.sc, param1.sc), isgreaterequal(param0.sd, param1.sd),
                isgreaterequal(param0.se, param1.se), isgreaterequal(param0.sf, param1.sf)); }


//longn isgreaterequal (doublen x, doublen y)


//intn isless (floatn x, floatn y)

INLINE_OVERLOADABLE int2 isless (float2 param0, float2 param1)
{return (int2)(isless(param0.s0, param1.s0), isless(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 isless (float3 param0, float3 param1)
{return (int3)(isless(param0.s0, param1.s0), isless(param0.s1, param1.s1),
               isless(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 isless (float4 param0, float4 param1)
{return (int4)(isless(param0.s0, param1.s0), isless(param0.s1, param1.s1),
               isless(param0.s2, param1.s2), isless(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 isless (float8 param0, float8 param1)
{return (int8)(isless(param0.s0, param1.s0), isless(param0.s1, param1.s1),
               isless(param0.s2, param1.s2), isless(param0.s3, param1.s3),
               isless(param0.s4, param1.s4), isless(param0.s5, param1.s5),
               isless(param0.s6, param1.s6), isless(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 isless (float16 param0, float16 param1)
{return (int16)(isless(param0.s0,  param1.s0),  isless(param0.s1,  param1.s1),
                isless(param0.s2,  param1.s2),  isless(param0.s3,  param1.s3),
                isless(param0.s4,  param1.s4),  isless(param0.s5,  param1.s5),
                isless(param0.s6,  param1.s6),  isless(param0.s7,  param1.s7),
                isless(param0.s8,  param1.s8),  isless(param0.s9,  param1.s9),
                isless(param0.sa, param1.sa), isless(param0.sb, param1.sb),
                isless(param0.sc, param1.sc), isless(param0.sd, param1.sd),
                isless(param0.se, param1.se), isless(param0.sf, param1.sf)); }


//longn isless (doublen x, doublen y)


//intn islessequal (floatn x, floatn y)

INLINE_OVERLOADABLE int2 islessequal (float2 param0, float2 param1)
{return (int2)(islessequal(param0.s0, param1.s0), islessequal(param0.s1, param1.s1)); }

INLINE_OVERLOADABLE int3 islessequal (float3 param0, float3 param1)
{return (int3)(islessequal(param0.s0, param1.s0), islessequal(param0.s1, param1.s1),
               islessequal(param0.s2, param1.s2)); }

INLINE_OVERLOADABLE int4 islessequal (float4 param0, float4 param1)
{return (int4)(islessequal(param0.s0, param1.s0), islessequal(param0.s1, param1.s1),
               islessequal(param0.s2, param1.s2), islessequal(param0.s3, param1.s3)); }

INLINE_OVERLOADABLE int8 islessequal (float8 param0, float8 param1)
{return (int8)(islessequal(param0.s0, param1.s0), islessequal(param0.s1, param1.s1),
               islessequal(param0.s2, param1.s2), islessequal(param0.s3, param1.s3),
               islessequal(param0.s4, param1.s4), islessequal(param0.s5, param1.s5),
               islessequal(param0.s6, param1.s6), islessequal(param0.s7, param1.s7)); }

INLINE_OVERLOADABLE int16 islessequal (float16 param0, float16 param1)
{return (int16)(islessequal(param0.s0,  param1.s0),  islessequal(param0.s1,  param1.s1),
                islessequal(param0.s2,  param1.s2),  islessequal(param0.s3,  param1.s3),
                islessequal(param0.s4,  param1.s4),  islessequal(param0.s5,  param1.s5),
                islessequal(param0.s6,  param1.s6),  islessequal(param0.s7,  param1.s7),
                islessequal(param0.s8,  param1.s8),  islessequal(param0.s9,  param1.s9),
                islessequal(param0.sa, param1.sa), islessequal(param0.sb, param1.sb),
                islessequal(param0.sc, param1.sc), islessequal(param0.sd, param1.sd),
                islessequal(param0.se, param1.se), islessequal(param0.sf, param1.sf)); }


//longn islessequal (doublen x, doublen y)


//longn islessgreater (doublen x, doublen y)


//intn isfinite (floatn

INLINE_OVERLOADABLE int2 isfinite (float2 param0)
{return (int2)(isfinite(param0.s0), isfinite(param0.s1)); }

INLINE_OVERLOADABLE int3 isfinite (float3 param0)
{return (int3)(isfinite(param0.s0), isfinite(param0.s1),
               isfinite(param0.s2)); }

INLINE_OVERLOADABLE int4 isfinite (float4 param0)
{return (int4)(isfinite(param0.s0), isfinite(param0.s1),
               isfinite(param0.s2), isfinite(param0.s3)); }

INLINE_OVERLOADABLE int8 isfinite (float8 param0)
{return (int8)(isfinite(param0.s0), isfinite(param0.s1),
               isfinite(param0.s2), isfinite(param0.s3),
               isfinite(param0.s4), isfinite(param0.s5),
               isfinite(param0.s6), isfinite(param0.s7)); }

INLINE_OVERLOADABLE int16 isfinite (float16 param0)
{return (int16)(isfinite(param0.s0),  isfinite(param0.s1),
                isfinite(param0.s2),  isfinite(param0.s3),
                isfinite(param0.s4),  isfinite(param0.s5),
                isfinite(param0.s6),  isfinite(param0.s7),
                isfinite(param0.s8),  isfinite(param0.s9),
                isfinite(param0.sa), isfinite(param0.sb),
                isfinite(param0.sc), isfinite(param0.sd),
                isfinite(param0.se), isfinite(param0.sf)); }


//longn isfinite (doublen)


//intn isinf (floatn)

INLINE_OVERLOADABLE int2 isinf (float2 param0)
{return (int2)(isinf(param0.s0), isinf(param0.s1)); }

INLINE_OVERLOADABLE int3 isinf (float3 param0)
{return (int3)(isinf(param0.s0), isinf(param0.s1),
               isinf(param0.s2)); }

INLINE_OVERLOADABLE int4 isinf (float4 param0)
{return (int4)(isinf(param0.s0), isinf(param0.s1),
               isinf(param0.s2), isinf(param0.s3)); }

INLINE_OVERLOADABLE int8 isinf (float8 param0)
{return (int8)(isinf(param0.s0), isinf(param0.s1),
               isinf(param0.s2), isinf(param0.s3),
               isinf(param0.s4), isinf(param0.s5),
               isinf(param0.s6), isinf(param0.s7)); }

INLINE_OVERLOADABLE int16 isinf (float16 param0)
{return (int16)(isinf(param0.s0),  isinf(param0.s1),
                isinf(param0.s2),  isinf(param0.s3),
                isinf(param0.s4),  isinf(param0.s5),
                isinf(param0.s6),  isinf(param0.s7),
                isinf(param0.s8),  isinf(param0.s9),
                isinf(param0.sa), isinf(param0.sb),
                isinf(param0.sc), isinf(param0.sd),
                isinf(param0.se), isinf(param0.sf)); }


//longn isinf (doublen)


//intn isnan (floatn)

INLINE_OVERLOADABLE int2 isnan (float2 param0)
{return (int2)(isnan(param0.s0), isnan(param0.s1)); }

INLINE_OVERLOADABLE int3 isnan (float3 param0)
{return (int3)(isnan(param0.s0), isnan(param0.s1),
               isnan(param0.s2)); }

INLINE_OVERLOADABLE int4 isnan (float4 param0)
{return (int4)(isnan(param0.s0), isnan(param0.s1),
               isnan(param0.s2), isnan(param0.s3)); }

INLINE_OVERLOADABLE int8 isnan (float8 param0)
{return (int8)(isnan(param0.s0), isnan(param0.s1),
               isnan(param0.s2), isnan(param0.s3),
               isnan(param0.s4), isnan(param0.s5),
               isnan(param0.s6), isnan(param0.s7)); }

INLINE_OVERLOADABLE int16 isnan (float16 param0)
{return (int16)(isnan(param0.s0),  isnan(param0.s1),
                isnan(param0.s2),  isnan(param0.s3),
                isnan(param0.s4),  isnan(param0.s5),
                isnan(param0.s6),  isnan(param0.s7),
                isnan(param0.s8),  isnan(param0.s9),
                isnan(param0.sa), isnan(param0.sb),
                isnan(param0.sc), isnan(param0.sd),
                isnan(param0.se), isnan(param0.sf)); }


//longn isnan (doublen)


//intn isnormal (floatn)

INLINE_OVERLOADABLE int2 isnormal (float2 param0)
{return (int2)(isnormal(param0.s0), isnormal(param0.s1)); }

INLINE_OVERLOADABLE int3 isnormal (float3 param0)
{return (int3)(isnormal(param0.s0), isnormal(param0.s1),
               isnormal(param0.s2)); }

INLINE_OVERLOADABLE int4 isnormal (float4 param0)
{return (int4)(isnormal(param0.s0), isnormal(param0.s1),
               isnormal(param0.s2), isnormal(param0.s3)); }

INLINE_OVERLOADABLE int8 isnormal (float8 param0)
{return (int8)(isnormal(param0.s0), isnormal(param0.s1),
               isnormal(param0.s2), isnormal(param0.s3),
               isnormal(param0.s4), isnormal(param0.s5),
               isnormal(param0.s6), isnormal(param0.s7)); }

INLINE_OVERLOADABLE int16 isnormal (float16 param0)
{return (int16)(isnormal(param0.s0),  isnormal(param0.s1),
                isnormal(param0.s2),  isnormal(param0.s3),
                isnormal(param0.s4),  isnormal(param0.s5),
                isnormal(param0.s6),  isnormal(param0.s7),
                isnormal(param0.s8),  isnormal(param0.s9),
                isnormal(param0.sa), isnormal(param0.sb),
                isnormal(param0.sc), isnormal(param0.sd),
                isnormal(param0.se), isnormal(param0.sf)); }


//longn isnormal (doublen)


//longn isordered (doublen x, doublen y)


//longn isunordered (doublen x, doublen y)


//intn signbit (floatn)

INLINE_OVERLOADABLE int2 signbit (float2 param0)
{return (int2)(signbit(param0.s0), signbit(param0.s1)); }

INLINE_OVERLOADABLE int3 signbit (float3 param0)
{return (int3)(signbit(param0.s0), signbit(param0.s1),
               signbit(param0.s2)); }

INLINE_OVERLOADABLE int4 signbit (float4 param0)
{return (int4)(signbit(param0.s0), signbit(param0.s1),
               signbit(param0.s2), signbit(param0.s3)); }

INLINE_OVERLOADABLE int8 signbit (float8 param0)
{return (int8)(signbit(param0.s0), signbit(param0.s1),
               signbit(param0.s2), signbit(param0.s3),
               signbit(param0.s4), signbit(param0.s5),
               signbit(param0.s6), signbit(param0.s7)); }

INLINE_OVERLOADABLE int16 signbit (float16 param0)
{return (int16)(signbit(param0.s0),  signbit(param0.s1),
                signbit(param0.s2),  signbit(param0.s3),
                signbit(param0.s4),  signbit(param0.s5),
                signbit(param0.s6),  signbit(param0.s7),
                signbit(param0.s8),  signbit(param0.s9),
                signbit(param0.sa), signbit(param0.sb),
                signbit(param0.sc), signbit(param0.sd),
                signbit(param0.se), signbit(param0.sf)); }


//longn signbit (doublen)


//int any (igentype x)


//int all (igentype x)


// ##END_BUILTIN_VECTOR##

#undef INLINE_OVERLOADABLE
#undef PURE
#undef CONST
#undef OVERLOADABLE
#undef INLINE
#endif /* __GEN_OCL_STDLIB_H__ */
