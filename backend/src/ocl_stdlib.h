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

#define DECL_INTERNAL_WORK_ITEM_FN(NAME)                             \
__attribute__((pure,const)) unsigned int __gen_ocl_##NAME##0(void);  \
__attribute__((pure,const)) unsigned int __gen_ocl_##NAME##1(void);  \
__attribute__((pure,const)) unsigned int __gen_ocl_##NAME##2(void);
DECL_INTERNAL_WORK_ITEM_FN(get_group_id)
DECL_INTERNAL_WORK_ITEM_FN(get_local_id)
DECL_INTERNAL_WORK_ITEM_FN(get_local_size)
DECL_INTERNAL_WORK_ITEM_FN(get_global_size)
DECL_INTERNAL_WORK_ITEM_FN(get_num_groups)
#undef DECL_INTERNAL_WORK_ITEM_FN

#define DECL_PUBLIC_WORK_ITEM_FN(NAME)              \
inline unsigned NAME(unsigned int dim) {            \
  if (dim == 0) return __gen_ocl_##NAME##0();       \
  else if (dim == 1) return __gen_ocl_##NAME##1();  \
  else if (dim == 2) return __gen_ocl_##NAME##2();  \
  else return 0;                                    \
}
DECL_PUBLIC_WORK_ITEM_FN(get_group_id)
DECL_PUBLIC_WORK_ITEM_FN(get_local_id)
DECL_PUBLIC_WORK_ITEM_FN(get_local_size)
DECL_PUBLIC_WORK_ITEM_FN(get_global_size)
DECL_PUBLIC_WORK_ITEM_FN(get_num_groups)
#undef DECL_PUBLIC_WORK_ITEM_FN

inline unsigned int get_global_id(unsigned int dim) {
  return get_local_id(dim) + get_local_size(dim) * get_group_id(dim);
}

__attribute__ ((pure,const,overloadable)) float mad(float a, float b, float c);
__attribute__((overloadable)) inline unsigned select(unsigned src0, unsigned src1, unsigned cond) {
  return cond ? src0 : src1;
}
__attribute__((overloadable)) inline int select(int src0, int src1, int cond) {
  return cond ? src0 : src1;
}

typedef unsigned int uint;
typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef int int2 __attribute__((ext_vector_type(2)));
typedef int int3 __attribute__((ext_vector_type(3)));
typedef int int4 __attribute__((ext_vector_type(4)));
typedef unsigned int uint2 __attribute__((ext_vector_type(2)));
typedef unsigned uint3 __attribute__((ext_vector_type(3)));
typedef unsigned uint4 __attribute__((ext_vector_type(4)));
typedef bool bool2 __attribute__((ext_vector_type(2)));
typedef bool bool3 __attribute__((ext_vector_type(3)));
typedef bool bool4 __attribute__((ext_vector_type(4)));

// This will be optimized out by LLVM and will output LLVM select instructions
#define DECL_SELECT4(TYPE4, TYPE, COND_TYPE4, MASK)                       \
__attribute__((overloadable))                                             \
inline TYPE4 select(TYPE4 src0, TYPE4 src1, COND_TYPE4 cond) {            \
  TYPE4 dst;                                                              \
  const TYPE x0 = src0.x; /* Fix performance issue with CLANG */          \
  const TYPE x1 = src1.x;                                                 \
  const TYPE y0 = src0.y;                                                 \
  const TYPE y1 = src1.y;                                                 \
  const TYPE z0 = src0.z;                                                 \
  const TYPE z1 = src1.z;                                                 \
  const TYPE w0 = src0.w;                                                 \
  const TYPE w1 = src1.w;                                                 \
                                                                          \
  dst.x = (cond.x & MASK) ? x1 : x0;                                      \
  dst.y = (cond.y & MASK) ? y1 : y0;                                      \
  dst.z = (cond.z & MASK) ? z1 : z0;                                      \
  dst.w = (cond.w & MASK) ? w1 : w0;                                      \
  return dst;                                                             \
}
DECL_SELECT4(int4, int, int4, 0x80000000)
DECL_SELECT4(float4, float, int4, 0x80000000)
#undef DECL_SELECT4

__attribute__((overloadable,always_inline)) inline float2 mad(float2 a, float2 b, float2 c) {
  return (float2)(mad(a.x,b.x,c.x), mad(a.y,b.y,c.y));
}
__attribute__((overloadable,always_inline)) inline float3 mad(float3 a, float3 b, float3 c) {
  return (float3)(mad(a.x,b.x,c.x), mad(a.y,b.y,c.y), mad(a.z,b.z,c.z));
}
__attribute__((overloadable,always_inline)) inline float4 mad(float4 a, float4 b, float4 c) {
  return (float4)(mad(a.x,b.x,c.x), mad(a.y,b.y,c.y),
                  mad(a.z,b.z,c.z), mad(a.w,b.w,c.w));
}

#define __private __attribute__((address_space(0)))
#define __global __attribute__((address_space(1)))
#define __constant __attribute__((address_space(2)))
//#define __local __attribute__((address_space(3)))
#define global __global
//#define local __local
#define constant __constant
#define private __private

#define NULL ((void*)0)
