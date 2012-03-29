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

#ifndef __CL_KERNEL_H__
#define __CL_KERNEL_H__

#include "cl_defs.h"
#include "cl_internals.h"
#include "cl_driver.h"
#include "CL/cl.h"

#include <stdint.h>
#include <stdlib.h>

/* This is the kernel as it is interfaced by the compiler */
struct GenKernel;

/*! One OCL function */
struct _cl_kernel {
  uint64_t magic;                     /* To identify it as a kernel */
  volatile int ref_n;                 /* We reference count this object */
  cl_buffer bo;                       /* The code itself */
  cl_buffer const_bo;                 /* Buffer for all __constants values in the OCL program */
  cl_program program;                 /* Owns this structure (and pointers) */
  const struct GenKernel *gen_kernel; /* (Opaque) compiler structure for the OCL kernel */
  uint8_t ref_its_program;            /* True only for the user kernel (created by clCreateKernel) */
};

/* Allocate an empty kernel */
extern cl_kernel cl_kernel_new(const cl_program);

/* Destroy and deallocate an empty kernel */
extern void cl_kernel_delete(cl_kernel);

/* Setup the kernel with the given Gen Kernel */
extern void cl_kernel_setup(cl_kernel k, const struct GenKernel *gen_kernel);

/* Get the kernel name */
extern const char *cl_kernel_get_name(const cl_kernel k);

/* Get the simd width as used in the code */
extern uint32_t cl_kernel_get_simd_width(const cl_kernel k);

/* When a kernel is created from outside, we just duplicate the structure we
 * have internally and give it back to the user
 */
extern cl_kernel cl_kernel_dup(cl_kernel);

/* Add one more reference on the kernel object */
extern void cl_kernel_add_ref(cl_kernel);

/* Set the argument before kernel execution */
extern int cl_kernel_set_arg(cl_kernel,
                             uint32_t    arg_index,
                             size_t      arg_size,
                             const void *arg_value);

/* Compute and check the work group size from the user provided local size */
extern cl_int
cl_kernel_work_group_sz(cl_kernel ker,
                        const size_t *local_wk_sz,
                        cl_uint wk_dim,
                        size_t *wk_grp_sz);

#endif /* __CL_KERNEL_H__ */

