/*
 * Copyright @2015 Intel Corporation
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
 * Author: Guo Yejun <yejun.guo@intel.com>
 */

#ifndef __CL_CMRT_H__
#define __CL_CMRT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cl_kernel.h"
#include "cl_program.h"

cl_int cmrt_build_program(cl_program p, const char *options);
cl_int cmrt_destroy_program(cl_program p);
cl_int cmrt_destroy_device(cl_device_id device);
void* cmrt_create_kernel(cl_program p, const char *name);
cl_int cmrt_destroy_kernel(cl_kernel k);
cl_int cmrt_enqueue(cl_command_queue cq, cl_kernel k, const size_t* global_work_size, const size_t* local_work_size);
cl_int cmrt_set_kernel_arg(cl_kernel k, cl_uint index, size_t sz, const void *value);
cl_int cmrt_destroy_memory(cl_mem mem);
cl_int cmrt_destroy_event(cl_command_queue cq);
cl_int cmrt_wait_for_task_finished(cl_command_queue cq);

#ifdef __cplusplus
}
#endif

#endif
