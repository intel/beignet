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

__attribute__((pure,const)) unsigned int __gen_ocl_get_global_id0(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_global_id1(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_global_id2(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_local_id0(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_local_id1(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_local_id2(void);

inline unsigned get_global_id(unsigned int dim) {
  if (dim == 0) return __gen_ocl_get_global_id0();
  else if (dim == 1) return __gen_ocl_get_global_id1();
  else if (dim == 2) return __gen_ocl_get_global_id2();
  else return 0;
}

inline unsigned get_local_id(unsigned int dim) {
  if (dim == 0) return __gen_ocl_get_local_id0();
  else if (dim == 1) return __gen_ocl_get_local_id1();
  else if (dim == 2) return __gen_ocl_get_local_id2();
  else return 0;
}

__attribute__((overloadable)) inline unsigned select(unsigned src0, unsigned src1, unsigned cond) {
  return cond ? src0 : src1;
}

__attribute__((overloadable)) inline int select(int src0, int src1, int cond) {
  return cond ? src0 : src1;
}

typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef int int2 __attribute__((ext_vector_type(2)));
typedef int int3 __attribute__((ext_vector_type(3)));
typedef int int4 __attribute__((ext_vector_type(4)));
typedef int uint2 __attribute__((ext_vector_type(2)));
typedef unsigned uint3 __attribute__((ext_vector_type(3)));
typedef unsigned uint4 __attribute__((ext_vector_type(4)));
typedef bool bool2 __attribute__((ext_vector_type(2)));
typedef bool bool3 __attribute__((ext_vector_type(3)));
typedef bool bool4 __attribute__((ext_vector_type(4)));

__attribute__((overloadable)) inline int4 select(int4 src0, int4 src1, int4 cond) {
  int4 dst;
  const int x0 = src0.x; // Fix performance issue with CLANG
  const int x1 = src1.x;
  const int y0 = src0.y;
  const int y1 = src1.y;
  const int z0 = src0.z;
  const int z1 = src1.z;
  const int w0 = src0.w;
  const int w1 = src1.w;

  dst.x = (cond.x & 0x80000000) ? x1 : x0;
  dst.y = (cond.y & 0x80000000) ? y1 : y0;
  dst.z = (cond.z & 0x80000000) ? z1 : z0;
  dst.w = (cond.w & 0x80000000) ? w1 : w0;
  return dst;
}

#define __global __attribute__((address_space(1)))
#define global __global

