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
// Work groups and work items functions
/////////////////////////////////////////////////////////////////////////////
#define DECL_INTERNAL_WORK_ITEM_FN(NAME) \
__attribute__((pure,const)) unsigned int __gen_ocl_##NAME##0(void); \
__attribute__((pure,const)) unsigned int __gen_ocl_##NAME##1(void); \
__attribute__((pure,const)) unsigned int __gen_ocl_##NAME##2(void);
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

/////////////////////////////////////////////////////////////////////////////
// Vector loads and stores
/////////////////////////////////////////////////////////////////////////////

// These loads and stores will use untyped reads and writes, so we can just
// cast to vector loads / stores. Not C99 compliant BTW due to aliasing issue.
// Well we do not care, we do not activate TBAA in the compiler
#define DECL_UNTYPED_RW_SPACE_N(TYPE, DIM, SPACE) \
__attribute__((always_inline, overloadable)) \
inline TYPE##DIM vload##DIM(size_t offset, const SPACE TYPE *p) { \
  return *(SPACE TYPE##DIM *) (p + DIM * offset); \
} \
__attribute__((always_inline, overloadable)) \
inline void vstore##DIM(TYPE##DIM v, size_t offset, SPACE TYPE *p) { \
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
// Arithmetic functions
/////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline))
inline uint get_global_id(uint dim) {
  return get_local_id(dim) + get_local_size(dim) * get_group_id(dim);
}

__attribute__ ((pure, const, overloadable)) float mad(float a, float b, float c);
__attribute__((overloadable, always_inline))
inline uint select(uint src0, uint src1, uint cond) {
  return cond ? src0 : src1;
}
__attribute__((overloadable, always_inline))
inline int select(int src0, int src1, int cond) {
  return cond ? src0 : src1;
}

// This will be optimized out by LLVM and will output LLVM select instructions
#define DECL_SELECT4(TYPE4, TYPE, COND_TYPE4, MASK) \
__attribute__((overloadable)) \
inline TYPE4 select(TYPE4 src0, TYPE4 src1, COND_TYPE4 cond) { \
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

INLINE_OVERLOADABLE inline float2 mad(float2 a, float2 b, float2 c) {
  return (float2)(mad(a.x,b.x,c.x), mad(a.y,b.y,c.y));
}
INLINE_OVERLOADABLE inline float3 mad(float3 a, float3 b, float3 c) {
  return (float3)(mad(a.x,b.x,c.x), mad(a.y,b.y,c.y), mad(a.z,b.z,c.z));
}
INLINE_OVERLOADABLE inline float4 mad(float4 a, float4 b, float4 c) {
  return (float4)(mad(a.x,b.x,c.x), mad(a.y,b.y,c.y),
                  mad(a.z,b.z,c.z), mad(a.w,b.w,c.w));
}

/////////////////////////////////////////////////////////////////////////////
// Extensions to manipulate the register file
/////////////////////////////////////////////////////////////////////////////

// Direct addressing register regions
OVERLOADABLE int __gen_ocl_region(int offset, int vstride, int width, int hstride, int);
OVERLOADABLE int __gen_ocl_region(int offset, int vstride, int width, int hstride, int, int);
OVERLOADABLE int __gen_ocl_region(int offset, int vstride, int width, int hstride, int, int, int);
OVERLOADABLE int __gen_ocl_region(int offset, int vstride, int width, int hstride, int, int, int, int);
OVERLOADABLE int __gen_ocl_region(int offset, int vstride, int width, int hstride, int, int, int, int, int);
OVERLOADABLE int __gen_ocl_region(int offset, int vstride, int width, int hstride, int, int, int, int, int, int);
OVERLOADABLE int __gen_ocl_region(int offset, int vstride, int width, int hstride, int, int, int, int, int, int, int);
OVERLOADABLE int __gen_ocl_region(int offset, int vstride, int width, int hstride, int, int, int, int, int, int, int, int);

// Gather from register file
OVERLOADABLE int __gen_ocl_rgather(unsigned short index, int);
OVERLOADABLE int __gen_ocl_rgather(unsigned short index, int, int);
OVERLOADABLE int __gen_ocl_rgather(unsigned short index, int, int, int);
OVERLOADABLE int __gen_ocl_rgather(unsigned short index, int, int, int, int);
OVERLOADABLE int __gen_ocl_rgather(unsigned short index, int, int, int, int, int);
OVERLOADABLE int __gen_ocl_rgather(unsigned short index, int, int, int, int, int, int);
OVERLOADABLE int __gen_ocl_rgather(unsigned short index, int, int, int, int, int, int, int);
OVERLOADABLE int __gen_ocl_rgather(unsigned short index, int, int, int, int, int, int, int, int);

/////////////////////////////////////////////////////////////////////////////
// Extension to have uniform condition per hardware thread
/////////////////////////////////////////////////////////////////////////////

OVERLOADABLE unsigned short __gen_ocl_any(unsigned short cond);
OVERLOADABLE unsigned short __gen_ocl_all(unsigned short cond);

/////////////////////////////////////////////////////////////////////////////
// Extension to support OBlock reads / writes
/////////////////////////////////////////////////////////////////////////////

OVERLOADABLE int  __gen_ocl_obread(const __global void *address);
OVERLOADABLE int  __gen_ocl_obread(const __constant void *address);
OVERLOADABLE int  __gen_ocl_obread(const __local void *address);
OVERLOADABLE void  __gen_ocl_obwrite(const __global void *address, int);
OVERLOADABLE void  __gen_ocl_obwrite(const __local void *address, int);

/////////////////////////////////////////////////////////////////////////////
// Force the compilation to SIMD8 or SIMD16
/////////////////////////////////////////////////////////////////////////////

int __gen_ocl_force_simd8(void);
int __gen_ocl_force_simd16(void);

#define DECL_VOTE(TYPE) \
__attribute__((overloadable,always_inline)) \
TYPE __gen_ocl_any(TYPE cond) { \
  return (TYPE) __gen_ocl_any((unsigned short) cond); \
} \
__attribute__((overloadable,always_inline)) \
TYPE __gen_ocl_all(TYPE cond) { \
  return (TYPE) __gen_ocl_all((unsigned short) cond); \
}
DECL_VOTE(unsigned int)
DECL_VOTE(unsigned char)
DECL_VOTE(int)
DECL_VOTE(char)
DECL_VOTE(short)
DECL_VOTE(bool)
#undef DECL_VOTE

#define NULL ((void*)0)
#undef INLINE_OVERLOADABLE

