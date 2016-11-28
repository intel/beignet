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
#include "ocl_memcpy.h"

#define DECL_TWO_SPACE_MEMCOPY_FN(NAME, DST_SPACE, SRC_SPACE) \
void __gen_memcpy_ ##NAME## _align (DST_SPACE uchar* dst, SRC_SPACE uchar* src, size_t size) { \
  size_t index = 0; \
  while((index + 4) <= size) { \
    *((DST_SPACE uint *)(dst + index)) = *((SRC_SPACE uint *)(src + index)); \
    index += 4; \
  } \
  while(index < size) { \
    dst[index] = src[index]; \
    index++; \
  } \
} \
void __gen_memcpy_ ##NAME (DST_SPACE uchar* dst, SRC_SPACE uchar* src, size_t size) { \
  size_t index = 0; \
  while(index < size) { \
    dst[index] = src[index]; \
    index++; \
  } \
}

#if (__OPENCL_C_VERSION__ >= 200)
#define DECL_ONE_SPACE_MEMCOPY_FN(NAME, DST_SPACE) \
  DECL_TWO_SPACE_MEMCOPY_FN( NAME## g, DST_SPACE, __global) \
  DECL_TWO_SPACE_MEMCOPY_FN( NAME## l, DST_SPACE, __local) \
  DECL_TWO_SPACE_MEMCOPY_FN( NAME## p, DST_SPACE, __private) \
  DECL_TWO_SPACE_MEMCOPY_FN( NAME## n, DST_SPACE, __generic) \
  DECL_TWO_SPACE_MEMCOPY_FN( NAME## c, DST_SPACE, __constant)

DECL_ONE_SPACE_MEMCOPY_FN(g, __global)
DECL_ONE_SPACE_MEMCOPY_FN(l, __local)
DECL_ONE_SPACE_MEMCOPY_FN(p, __private)
DECL_ONE_SPACE_MEMCOPY_FN(n, __generic)
#else
#define DECL_ONE_SPACE_MEMCOPY_FN(NAME, DST_SPACE) \
  DECL_TWO_SPACE_MEMCOPY_FN( NAME## g, DST_SPACE, __global) \
  DECL_TWO_SPACE_MEMCOPY_FN( NAME## l, DST_SPACE, __local) \
  DECL_TWO_SPACE_MEMCOPY_FN( NAME## p, DST_SPACE, __private) \
  DECL_TWO_SPACE_MEMCOPY_FN( NAME## c, DST_SPACE, __constant)

DECL_ONE_SPACE_MEMCOPY_FN(g, __global)
DECL_ONE_SPACE_MEMCOPY_FN(l, __local)
DECL_ONE_SPACE_MEMCOPY_FN(p, __private)

#endif

