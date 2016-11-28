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
#include "ocl_memset.h"

#define DECL_MEMSET_FN(NAME, DST_SPACE) \
void __gen_memset_ ##NAME## _align (DST_SPACE uchar* dst, uchar val, size_t size) { \
  size_t index = 0; \
  uint v = (val << 24) | (val << 16) | (val << 8) | val; \
  while((index + 4) <= size) { \
    *((DST_SPACE uint *)(dst + index)) = v; \
    index += 4; \
  } \
  while(index < size) { \
    dst[index] = val; \
    index++; \
 } \
} \
void __gen_memset_ ##NAME (DST_SPACE uchar* dst, uchar val, size_t size) { \
  size_t index = 0; \
  while(index < size) { \
    dst[index] = val; \
    index++; \
 } \
}

DECL_MEMSET_FN(g, __global)
DECL_MEMSET_FN(l, __local)
DECL_MEMSET_FN(p, __private)
#if (__OPENCL_C_VERSION__ >= 200)
DECL_MEMSET_FN(n, __generic)
#endif

