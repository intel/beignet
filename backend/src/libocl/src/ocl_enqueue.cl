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
#include "ocl_types.h"
#include "ocl_enqueue.h"
#include "ocl_workitem.h"
#include "ocl_atom.h"

queue_t get_default_queue(void)
{
  queue_t queue;
  return queue; //return NULL queue
}

ndrange_t __gen_ocl_set_ndrange_info(__private struct ndrange_info_t *info);
__private struct ndrange_info_t* __gen_ocl_get_ndrange_info(ndrange_t info);
__global int* __gen_ocl_get_enqueue_info_addr(void);

OVERLOADABLE int enqueue_kernel(queue_t q, int flag, ndrange_t ndrange, void (^block)(void))
{
  int i;
  __private struct Block_literal *literal = (__private struct Block_literal *)block;
  __private uchar *data = (__private uchar *)block;
  int size = literal->descriptor->size;
  literal->descriptor->reserved = 0;
  __global int* start_addr = __gen_ocl_get_enqueue_info_addr();
  int offset = atomic_add(start_addr, size + sizeof(struct ndrange_info_t));
  __global uchar* addr = (__global uchar*)start_addr + offset + sizeof(int);
  __private struct ndrange_info_t *info = __gen_ocl_get_ndrange_info(ndrange);

  *((__global struct ndrange_info_t *)addr) = *info;
  addr += sizeof(*info);

  for(i=0; i< size; i++) {
    addr[i] = data[i];
  }
  return 0;
}

OVERLOADABLE int enqueue_kernel(queue_t q, int flag, ndrange_t ndrange,
                                uint num_events_in_wait_list, const clk_event_t *event_wait_list,
                                clk_event_t *event_ret, void (^block)(void))
{
  return enqueue_kernel(q, flag, ndrange, block);
}

int __gen_enqueue_kernel_slm(queue_t q, int flag, ndrange_t ndrange, __private void * block, int count, __private int* slm_sizes)
{
  int i;
  __private struct Block_literal* literal = (__private struct Block_literal *)block;
  __private uchar* data = (__private uchar *)block;
  int size = literal->descriptor->size;
  int slm_size = count * sizeof(int);
  literal->descriptor->reserved = slm_size;
  __global int* start_addr = __gen_ocl_get_enqueue_info_addr();
  int offset = atomic_add(start_addr, size + sizeof(struct ndrange_info_t) + slm_size);
  __global uchar* addr = (__global uchar*)start_addr + offset + sizeof(int);
  __private struct ndrange_info_t *info = __gen_ocl_get_ndrange_info(ndrange);

  *((__global struct ndrange_info_t *)addr) = *info;
  addr += sizeof(*info);

  for(i=0; i < size; i++) {
    addr[i] = data[i];
  }

  addr += size;
  for(i=0; i < count; i++) {
    ((__global int *)addr)[i] = slm_sizes[i];
  }
  return 0;
}

clk_event_t create_user_event(void)
{
  clk_event_t e;
  return e;
}

void retain_event(clk_event_t event)
{
  return;
}

void release_event(clk_event_t event)
{
  return;
}

void set_user_event_status(clk_event_t event, int status)
{
  return;
}

bool is_valid_event(clk_event_t event)
{
  return 1;
}

uint __get_kernel_work_group_size_impl(__private void *block)
{
  return 256;
}

uint __get_kernel_preferred_work_group_multiple_impl(__private  void *block)
{
  return 16;
}

void capture_event_profiling_info(clk_event_t event, int name, global void *value)
{
  //fake profiing data
  ((__global ulong *)value)[0] = 0x3000;
  ((__global ulong *)value)[1] = 0x6000;
}
OVERLOADABLE ndrange_t ndrange_1D(size_t global_work_size)
{
  struct ndrange_info_t info;
  info.type = 0x1;
  info.global_work_size[0] = global_work_size;
  return __gen_ocl_set_ndrange_info(&info);
  //return ndrange;
}

OVERLOADABLE ndrange_t ndrange_1D(size_t global_work_size, size_t local_work_size)
{
  struct ndrange_info_t info;
  info.type = 0x2;
  info.global_work_size[0] = global_work_size;
  info.local_work_size[0] = local_work_size;
  return __gen_ocl_set_ndrange_info(&info);
 // return ndrange;
}


OVERLOADABLE ndrange_t ndrange_1D(size_t global_work_offset, size_t global_work_size, size_t local_work_size)
{
  struct ndrange_info_t info;
  info.type = 0x3;
  info.global_work_size[0] = global_work_size;
  info.local_work_size[0] = local_work_size;
  info.global_work_offset[0] = global_work_offset;
  return __gen_ocl_set_ndrange_info(&info);
  //return ndrange;
}

OVERLOADABLE ndrange_t ndrange_2D(const size_t global_work_size[2])
{
  struct ndrange_info_t info;
  info.type = 0x11;
  info.global_work_size[0] = global_work_size[0];
  info.global_work_size[1] = global_work_size[1];
  return __gen_ocl_set_ndrange_info(&info);
  //return ndrange;
}

OVERLOADABLE ndrange_t ndrange_2D(const size_t global_work_size[2], const size_t local_work_size[2])
{
  struct ndrange_info_t info;
  info.type = 0x12;
  info.global_work_size[0] = global_work_size[0];
  info.global_work_size[1] = global_work_size[1];
  info.local_work_size[0] = local_work_size[0];
  info.local_work_size[1] = local_work_size[1];
  return __gen_ocl_set_ndrange_info(&info);
}


OVERLOADABLE ndrange_t ndrange_2D(const size_t global_work_offset[2], const size_t global_work_size[2], const size_t local_work_size[2])
{
  struct ndrange_info_t info;
  info.type = 0x13;
  info.global_work_size[0] = global_work_size[0];
  info.global_work_size[1] = global_work_size[1];
  info.local_work_size[0] = local_work_size[0];
  info.local_work_size[1] = local_work_size[1];
  info.global_work_offset[0] = global_work_offset[0];
  info.global_work_offset[1] = global_work_offset[1];
  return __gen_ocl_set_ndrange_info(&info);
}

OVERLOADABLE ndrange_t ndrange_3D(const size_t global_work_size[3])
{
  struct ndrange_info_t info;
  info.type = 0x21;
  info.global_work_size[0] = global_work_size[0];
  info.global_work_size[1] = global_work_size[1];
  info.global_work_size[2] = global_work_size[2];
  return __gen_ocl_set_ndrange_info(&info);
}

OVERLOADABLE ndrange_t ndrange_3D(const size_t global_work_size[3], const size_t local_work_size[3])
{
  struct ndrange_info_t info;
  info.type = 0x22;
  info.global_work_size[0] = global_work_size[0];
  info.global_work_size[1] = global_work_size[1];
  info.global_work_size[2] = global_work_size[2];
  info.local_work_size[0] = local_work_size[0];
  info.local_work_size[1] = local_work_size[1];
  info.local_work_size[2] = local_work_size[2];
  return __gen_ocl_set_ndrange_info(&info);
}

OVERLOADABLE ndrange_t ndrange_3D(const size_t global_work_offset[3], const size_t global_work_size[3], const size_t local_work_size[3])
{
  struct ndrange_info_t info;
  info.type = 0x23;
  info.global_work_size[0] = global_work_size[0];
  info.global_work_size[1] = global_work_size[1];
  info.global_work_size[2] = global_work_size[2];
  info.local_work_size[0] = local_work_size[0];
  info.local_work_size[1] = local_work_size[1];
  info.local_work_size[2] = local_work_size[2];
  info.global_work_offset[0] = global_work_offset[0];
  info.global_work_offset[1] = global_work_offset[1];
  info.global_work_offset[2] = global_work_offset[2];
  return __gen_ocl_set_ndrange_info(&info);
}

int enqueue_marker (queue_t queue, uint num_events_in_wait_list, const clk_event_t *event_wait_list, clk_event_t *event_ret)
{
  return 0;
}
