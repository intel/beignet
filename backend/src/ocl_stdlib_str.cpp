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

#include "string"
namespace gbe {
std::string ocl_stdlib_str = 
"#define DECL_INTERNAL_WORK_ITEM_FN(NAME)                             \\\n"
"__attribute__((pure,const)) unsigned int __gen_ocl_##NAME##0(void);  \\\n"
"__attribute__((pure,const)) unsigned int __gen_ocl_##NAME##1(void);  \\\n"
"__attribute__((pure,const)) unsigned int __gen_ocl_##NAME##2(void);\n"
"DECL_INTERNAL_WORK_ITEM_FN(get_group_id)\n"
"DECL_INTERNAL_WORK_ITEM_FN(get_local_id)\n"
"DECL_INTERNAL_WORK_ITEM_FN(get_local_size)\n"
"DECL_INTERNAL_WORK_ITEM_FN(get_global_size)\n"
"DECL_INTERNAL_WORK_ITEM_FN(get_num_groups)\n"
"#undef DECL_INTERNAL_WORK_ITEM_FN\n"
"\n"
"#define DECL_PUBLIC_WORK_ITEM_FN(NAME)              \\\n"
"inline unsigned NAME(unsigned int dim) {            \\\n"
"  if (dim == 0) return __gen_ocl_##NAME##0();       \\\n"
"  else if (dim == 1) return __gen_ocl_##NAME##1();  \\\n"
"  else if (dim == 2) return __gen_ocl_##NAME##2();  \\\n"
"  else return 0;                                    \\\n"
"}\n"
"DECL_PUBLIC_WORK_ITEM_FN(get_group_id)\n"
"DECL_PUBLIC_WORK_ITEM_FN(get_local_id)\n"
"DECL_PUBLIC_WORK_ITEM_FN(get_local_size)\n"
"DECL_PUBLIC_WORK_ITEM_FN(get_global_size)\n"
"DECL_PUBLIC_WORK_ITEM_FN(get_num_groups)\n"
"#undef DECL_PUBLIC_WORK_ITEM_FN\n"
"\n"
"inline unsigned int get_global_id(unsigned int dim) {\n"
"  return get_local_id(dim) + get_local_size(dim) * get_group_id(dim);\n"
"}\n"
"\n"
"__attribute__ ((pure,const,overloadable)) float mad(float a, float b, float c);\n"
"__attribute__((overloadable)) inline unsigned select(unsigned src0, unsigned src1, unsigned cond) {\n"
"  return cond ? src0 : src1;\n"
"}\n"
"__attribute__((overloadable)) inline int select(int src0, int src1, int cond) {\n"
"  return cond ? src0 : src1;\n"
"}\n"
"\n"
"typedef unsigned int uint;\n"
"typedef float float2 __attribute__((ext_vector_type(2)));\n"
"typedef float float3 __attribute__((ext_vector_type(3)));\n"
"typedef float float4 __attribute__((ext_vector_type(4)));\n"
"typedef int int2 __attribute__((ext_vector_type(2)));\n"
"typedef int int3 __attribute__((ext_vector_type(3)));\n"
"typedef int int4 __attribute__((ext_vector_type(4)));\n"
"typedef unsigned int uint2 __attribute__((ext_vector_type(2)));\n"
"typedef unsigned uint3 __attribute__((ext_vector_type(3)));\n"
"typedef unsigned uint4 __attribute__((ext_vector_type(4)));\n"
"typedef bool bool2 __attribute__((ext_vector_type(2)));\n"
"typedef bool bool3 __attribute__((ext_vector_type(3)));\n"
"typedef bool bool4 __attribute__((ext_vector_type(4)));\n"
"\n"
"// This will be optimized out by LLVM and will output LLVM select instructions\n"
"#define DECL_SELECT4(TYPE4, TYPE, COND_TYPE4, MASK)                       \\\n"
"__attribute__((overloadable))                                             \\\n"
"inline TYPE4 select(TYPE4 src0, TYPE4 src1, COND_TYPE4 cond) {            \\\n"
"  TYPE4 dst;                                                              \\\n"
"  const TYPE x0 = src0.x; /* Fix performance issue with CLANG */          \\\n"
"  const TYPE x1 = src1.x;                                                 \\\n"
"  const TYPE y0 = src0.y;                                                 \\\n"
"  const TYPE y1 = src1.y;                                                 \\\n"
"  const TYPE z0 = src0.z;                                                 \\\n"
"  const TYPE z1 = src1.z;                                                 \\\n"
"  const TYPE w0 = src0.w;                                                 \\\n"
"  const TYPE w1 = src1.w;                                                 \\\n"
"                                                                          \\\n"
"  dst.x = (cond.x & MASK) ? x1 : x0;                                      \\\n"
"  dst.y = (cond.y & MASK) ? y1 : y0;                                      \\\n"
"  dst.z = (cond.z & MASK) ? z1 : z0;                                      \\\n"
"  dst.w = (cond.w & MASK) ? w1 : w0;                                      \\\n"
"  return dst;                                                             \\\n"
"}\n"
"DECL_SELECT4(int4, int, int4, 0x80000000)\n"
"DECL_SELECT4(float4, float, int4, 0x80000000)\n"
"#undef DECL_SELECT4\n"
"\n"
"__attribute__((overloadable,always_inline)) inline float2 mad(float2 a, float2 b, float2 c) {\n"
"  return (float2)(mad(a.x,b.x,c.x), mad(a.y,b.y,c.y));\n"
"}\n"
"__attribute__((overloadable,always_inline)) inline float3 mad(float3 a, float3 b, float3 c) {\n"
"  return (float3)(mad(a.x,b.x,c.x), mad(a.y,b.y,c.y), mad(a.z,b.z,c.z));\n"
"}\n"
"__attribute__((overloadable,always_inline)) inline float4 mad(float4 a, float4 b, float4 c) {\n"
"  return (float4)(mad(a.x,b.x,c.x), mad(a.y,b.y,c.y),\n"
"                  mad(a.z,b.z,c.z), mad(a.w,b.w,c.w));\n"
"}\n"
"\n"
"#define __private __attribute__((address_space(0)))\n"
"#define __global __attribute__((address_space(1)))\n"
"#define __constant __attribute__((address_space(2)))\n"
"//#define __local __attribute__((address_space(3)))\n"
"#define global __global\n"
"//#define local __local\n"
"#define constant __constant\n"
"#define private __private\n"
"\n"
"#define NULL ((void*)0)\n"
;
}

