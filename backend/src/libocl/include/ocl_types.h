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
#ifndef __OCL_TYPES_H__
#define __OCL_TYPES_H__

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#include "ocl_defines.h"

#define NULL ((void*)0)

/////////////////////////////////////////////////////////////////////////////
// OpenCL Common Defines
/////////////////////////////////////////////////////////////////////////////
#define INLINE inline __attribute__((always_inline))
#define OVERLOADABLE __attribute__((overloadable))
#define PURE __attribute__((pure))
#define CONST __attribute__((const))
#define INLINE_OVERLOADABLE inline __attribute__((overloadable,always_inline))
// FIXME, clang's opencl FE doesn't support static.
#define static

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
// OpenCL built-in image types
/////////////////////////////////////////////////////////////////////////////
// FIXME:
// This is a transitional hack to bypass the LLVM 3.3 built-in types.
// See the Khronos SPIR specification for handling of these types.
#define __texture __attribute__((address_space(4)))
struct _image1d_t;
typedef __texture struct _image1d_t* __image1d_t;
struct _image1d_buffer_t;
typedef __texture struct _image1d_buffer_t* __image1d_buffer_t;
struct _image1d_array_t;
typedef __texture struct _image1d_array_t* __image1d_array_t;
struct _image2d_t;
typedef __texture struct _image2d_t* __image2d_t;
struct _image2d_array_t;
typedef __texture struct _image2d_array_t* __image2d_array_t;
struct _image3d_t;
typedef __texture struct _image3d_t* __image3d_t;
typedef const ushort __sampler_t;
#define image1d_t __image1d_t
#define image1d_buffer_t __image1d_buffer_t
#define image1d_array_t __image1d_array_t
#define image2d_t __image2d_t
#define image2d_array_t __image2d_array_t
#define image3d_t __image3d_t
#define sampler_t __sampler_t

/////////////////////////////////////////////////////////////////////////////
// OpenCL built-in event types
/////////////////////////////////////////////////////////////////////////////
typedef size_t __event_t;
#define event_t __event_t

#endif /* __OCL_TYPES_H__ */
