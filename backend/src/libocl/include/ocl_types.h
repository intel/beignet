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
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#define DISABLE_ATOMIC_INT64
#ifndef DISABLE_ATOMIC_INT64
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
#endif
#include "ocl_defines.h"

#define NULL 0

/////////////////////////////////////////////////////////////////////////////
// OpenCL Common Defines
/////////////////////////////////////////////////////////////////////////////
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
#define __int_t_type(a,b,c) a##b##c
#define __int_type(type,n) __int_t_type(type,n,_TYPE__)
typedef __int_type(__INT,__INTPTR_WIDTH__) intptr_t;
typedef __int_type(__UINT,__INTPTR_WIDTH__) uintptr_t;
#undef __int_type
#undef __int_t_type

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
DEF(half);
#undef DEF

/////////////////////////////////////////////////////////////////////////////
// OpenCL atomic related types
/////////////////////////////////////////////////////////////////////////////
//atomic flags
#define CLK_LOCAL_MEM_FENCE  (1 << 0)
#define CLK_GLOBAL_MEM_FENCE (1 << 1)
#define  CLK_IMAGE_MEM_FENCE (1 << 2)

typedef uint cl_mem_fence_flags;

//memory order
typedef enum {
	memory_order_relaxed,
	memory_order_acquire,
	memory_order_release,
	memory_order_acq_rel,
	memory_order_seq_cst
} memory_order;

//memory scope
typedef enum {
	memory_scope_work_item,
	memory_scope_work_group,
	memory_scope_device,
	memory_scope_all_svm_devices,
	memory_scope_sub_group,
} memory_scope;

/////////////////////////////////////////////////////////////////////////////
// OpenCL built-in event types
/////////////////////////////////////////////////////////////////////////////
// FIXME:
// This is a transitional hack to bypass the LLVM 3.3 built-in types.
// See the Khronos SPIR specification for handling of these types.

#endif /* __OCL_TYPES_H__ */
