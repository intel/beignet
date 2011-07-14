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

#ifndef __CL_PROGRAM_H__
#define __CL_PROGRAM_H__

#include "cl_internals.h"
#include "CL/cl.h"

#include <stdint.h>
#include <stdlib.h>

/* This maps an OCL file containing some kernels */
struct _cl_program {
  uint64_t magic;           /* To identify it as a program */
  volatile int ref_n;       /* We reference count this object */
  cl_kernel *ker;           /* All kernels included by the OCL file */
  cl_program prev, next;    /* We chain the programs together */
  cl_context ctx;           /* Its parent context */
  char *bin;                /* The program copied verbatim */
  size_t bin_sz;            /* Its size in memory */
  uint32_t ker_n;           /* Number of declared kernels */
  uint32_t from_source:1;   /* Built from binary or source? */
  uint32_t is_built:1;      /* Did we call clBuildProgram on it? */
};

/* Create a program from */
extern cl_program cl_program_new(cl_context, const char*, size_t);

/* Destroy and deallocate an empty kernel */
extern void cl_program_delete(cl_program);

/* Add one more reference to the object (to defer its deletion) */
extern void cl_program_add_ref(cl_program);

/* Create a kernel for the OCL user */
extern cl_kernel cl_program_create_kernel(cl_program, const char*, cl_int*);

/* Directly create a program from a blob */
extern cl_program
cl_program_create_from_binary(cl_context             context,
                              cl_uint                num_devices,
                              const cl_device_id *   devices,
                              const size_t *         lengths,
                              const unsigned char ** binaries,
                              cl_int *               binary_status,
                              cl_int *               errcode_ret);

#endif /* __CL_PROGRAM_H__ */

