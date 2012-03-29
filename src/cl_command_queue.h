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

#ifndef __CL_COMMAND_QUEUE_H__
#define __CL_COMMAND_QUEUE_H__

#include "cl_internals.h"
#include "cl_driver.h"
#include "CL/cl.h"
#include <stdint.h>

struct intel_gpgpu;

/* Basically, this is a (kind-of) batch buffer */
struct _cl_command_queue {
  uint64_t magic;               /* To identify it as a command queue */
  volatile int ref_n;           /* We reference count this object */
  cl_context ctx;               /* Its parent context */
  cl_command_queue prev, next;  /* We chain the command queues together */
  cl_gpgpu *gpgpu;              /* Setup all GEN commands */
  cl_mem perf;                  /* Where to put the perf counters */
  cl_mem fulsim_out;            /* Fulsim will output this buffer */
  cl_buffer *last_batch;        /* To synchronize using clFinish */
};

/* Allocate and initialize a new command queue. Also insert it in the list of
 * command queue in the associated context
 */
extern cl_command_queue cl_command_queue_new(cl_context);

/* Destroy and deallocate the command queue */
extern void cl_command_queue_delete(cl_command_queue);

/* Keep one more reference on the queue */
extern void cl_command_queue_add_ref(cl_command_queue);

/* Map ND range kernel from OCL API */
extern cl_int cl_command_queue_ND_range(cl_command_queue queue,
                                        cl_kernel ker,
                                        const size_t *global_work_offset,
                                        const size_t *global_work_size,
                                        const size_t *local_work_size);

/* The memory object where to report the performance */
extern cl_int cl_command_queue_set_report_buffer(cl_command_queue, cl_mem);

/* Fulsim will dump this buffer (mostly to check its consistency */
cl_int cl_command_queue_set_fulsim_buffer(cl_command_queue, cl_mem);

/* Wait for the completion of the command queue */
extern cl_int cl_command_queue_finish(cl_command_queue);

/* Bind all the surfaces in the GPGPU state */
extern cl_int cl_command_queue_bind_surface(cl_command_queue queue,
                                            cl_kernel k,
                                            char *curbe,
                                            cl_buffer **local,
                                            cl_buffer **priv,
                                            cl_buffer **scratch,
                                            uint32_t local_sz);

#endif /* __CL_COMMAND_QUEUE_H__ */

