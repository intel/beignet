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
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#include "ocl_async.h"
#include "ocl_sync.h"
#include "ocl_workitem.h"

#define BODY(SRC_STRIDE, DST_STRIDE) \
  uint size = get_local_size(2) * get_local_size(1) * get_local_size(0); \
  uint count = num / size;  \
  uint offset = get_local_id(2) * get_local_size(1) + get_local_id(1);  \
  offset = offset * get_local_size(0) + get_local_id(0); \
  for(uint i=0; i<count; i+=1) { \
    *(dst + offset * DST_STRIDE) = *(src + offset * SRC_STRIDE); \
    offset += size;                                 \
  } \
  if(offset < num) \
    *(dst + offset * DST_STRIDE) = *(src + offset * SRC_STRIDE); \
  return 0;

#define DEFN(TYPE) \
OVERLOADABLE event_t async_work_group_copy (local TYPE *dst,  const global TYPE *src, \
							 size_t num, event_t event) { \
  BODY(1, 1); \
} \
OVERLOADABLE event_t async_work_group_copy (global TYPE *dst,  const local TYPE *src, \
							  size_t num, event_t event) { \
  BODY(1, 1); \
} \
OVERLOADABLE event_t async_work_group_strided_copy (local TYPE *dst,  const global TYPE *src, \
								 size_t num, size_t src_stride, event_t event) { \
  BODY(src_stride, 1); \
} \
OVERLOADABLE event_t async_work_group_strided_copy (global TYPE *dst,  const local TYPE *src, \
								  size_t num, size_t dst_stride, event_t event) { \
  BODY(1, dst_stride); \
}
#define DEF(TYPE) \
  DEFN(TYPE); DEFN(TYPE##2); DEFN(TYPE##3); DEFN(TYPE##4); DEFN(TYPE##8); DEFN(TYPE##16);
DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(int)
DEF(uint)
DEF(long)
DEF(ulong)
DEF(float)
DEF(double)
#undef BODY
#undef DEFN
#undef DEF

void wait_group_events (int num_events, event_t *event_list) {
  barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
}

#define DEFN(TYPE) \
OVERLOADABLE void prefetch(const global TYPE *p, size_t num) { }
#define DEF(TYPE) \
DEFN(TYPE); DEFN(TYPE##2); DEFN(TYPE##3); DEFN(TYPE##4); DEFN(TYPE##8); DEFN(TYPE##16)
DEF(char);
DEF(uchar);
DEF(short);
DEF(ushort);
DEF(int);
DEF(uint);
DEF(long);
DEF(ulong);
DEF(float);
#undef DEFN
#undef DEF
