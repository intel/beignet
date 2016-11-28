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

#ifndef __OCL_ENQUEUE_H__
#define __OCL_ENQUEUE_H__

#include "ocl_types.h"
#define CLK_ENQUEUE_FLAGS_WAIT_KERNEL 0
#define CLK_ENQUEUE_FLAGS_NO_WAIT 1
#define CLK_ENQUEUE_FLAGS_WAIT_WORK_GROUP 2
#define CLK_SUCCESS 0
#define CL_COMPLETE 0
#define CLK_PROFILING_COMMAND_EXEC_TIME 0

struct ndrange_info_t {
  int type;
  int global_work_size[3];
  int local_work_size[3];
  int global_work_offset[3];
};

struct Block_literal {
  void *isa; // initialized to &_NSConcreteStackBlock or &_NSConcreteGlobalBlock
  int flags;
  int reserved;
  __global void (*invoke)(void *, ...);
  struct Block_descriptor_1 {
    unsigned long int reserved;         // NULL
    unsigned long int size;         // sizeof(struct Block_literal_1)
    // optional helper functions
    void (*copy_helper)(void *dst, void *src);     // IFF (1<<25)
    void (*dispose_helper)(void *src);             // IFF (1<<25)
    // required ABI.2010.3.16
    const char *signature;                         // IFF (1<<30)
  } *descriptor;
  // imported variables
};

clk_event_t create_user_event(void);
void retain_event(clk_event_t event);
void release_event(clk_event_t event);
void set_user_event_status(clk_event_t event, int status);
bool is_valid_event(clk_event_t event);
void capture_event_profiling_info(clk_event_t event, int name, global void *value);

uint __get_kernel_work_group_size_impl(__private void *block);
uint __get_kernel_preferred_work_group_multiple_impl(__private void *block);

OVERLOADABLE int enqueue_kernel(queue_t q, int flag, ndrange_t ndrange, void (^block)(void));
OVERLOADABLE int enqueue_kernel(queue_t q, int flag, ndrange_t ndrange,
                                uint num_events_in_wait_list, const clk_event_t *event_wait_list,
                                clk_event_t *event_ret, void (^block)(void));
OVERLOADABLE int enqueue_kernel(queue_t q, int flag, ndrange_t ndrange, __private void *block, uint size0, ...);
OVERLOADABLE int enqueue_kernel(queue_t q, int flag, ndrange_t ndrange,
                                uint num_events_in_wait_list, const clk_event_t *event_wait_list,
                                clk_event_t *event_ret,  __private void *block, uint size0, ...);

queue_t get_default_queue(void);
int __gen_enqueue_kernel(queue_t q, int flag, ndrange_t ndrange, void (^block)(void), int size);
int __gen_enqueue_kernel_slm(queue_t q, int flag, ndrange_t ndrange, __private void * block, int count, __private int* slm_sizes);

OVERLOADABLE ndrange_t ndrange_1D(size_t global_work_size);
OVERLOADABLE ndrange_t ndrange_1D(size_t global_work_size, size_t local_work_size);
OVERLOADABLE ndrange_t ndrange_1D(size_t global_work_offset, size_t global_work_size, size_t local_work_size);

OVERLOADABLE ndrange_t ndrange_2D(const size_t global_work_size[2]);
OVERLOADABLE ndrange_t ndrange_2D(const size_t global_work_size[2], const size_t local_work_size[2]);
OVERLOADABLE ndrange_t ndrange_2D(const size_t global_work_offset[2], const size_t global_work_size[2], const size_t local_work_size[2]);

OVERLOADABLE ndrange_t ndrange_3D(const size_t global_work_size[3]);
OVERLOADABLE ndrange_t ndrange_3D(const size_t global_work_size[3], const size_t local_work_size[3]);
OVERLOADABLE ndrange_t ndrange_3D(const size_t global_work_offset[3], const size_t global_work_size[3], const size_t local_work_size[3]);

int enqueue_marker (queue_t queue, uint num_events_in_wait_list, const clk_event_t *event_wait_list, clk_event_t *event_ret);
#endif
