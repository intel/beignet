/* 
 * Copyright © 2012 Intel Corporation
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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __CL_PROGRAM_H__
#define __CL_PROGRAM_H__

#include "cl_internals.h"
#include "cl_gbe_loader.h"
#include "CL/cl.h"

#include <stdint.h>
#include <stdlib.h>

// This is the structure ouput by the compiler
struct _gbe_program;

enum {
  FROM_SOURCE = 0,
  FROM_LLVM = 1,
  FROM_BINARY = 2
};

/* This maps an OCL file containing some kernels */
struct _cl_program {
  DEFINE_ICD(dispatch)
  uint64_t magic;         /* To identify it as a program */
  volatile int ref_n;     /* We reference count this object */
  gbe_program opaque;     /* (Opaque) program as ouput by the compiler */
  cl_kernel *ker;         /* All kernels included by the OCL file */
  cl_program prev, next;  /* We chain the programs together */
  cl_context ctx;         /* Its parent context */
  char *bin;              /* The program copied verbatim */
  size_t bin_sz;          /* Its size in memory */
  char *source;           /* Program sources */
  char *binary;           /* Program binary. */
  size_t binary_sz;       /* The binary size. */
  uint32_t binary_type;   /* binary type: COMPILED_OBJECT(LLVM IR), LIBRARY(LLVM IR with option "-create-library"), or EXECUTABLE(GEN binary). */
  uint32_t ker_n;         /* Number of declared kernels */
  uint32_t source_type:2; /* Built from binary, source or LLVM */
  uint32_t is_built:1;    /* Did we call clBuildProgram on it? */
  int32_t build_status;   /* build status. */
  char *build_opts;       /* The build options for this program */
  size_t build_log_max_sz; /*build log maximum size in byte.*/
  char *build_log;         /* The build log for this program. */
  size_t build_log_sz;    /* The actual build log size.*/
};

/* Create a empty program */
extern cl_program cl_program_new(cl_context);

/* Destroy and deallocate an empty kernel */
extern void cl_program_delete(cl_program);

/* Add one more reference to the object (to defer its deletion) */
extern void cl_program_add_ref(cl_program);

/* Create a kernel for the OCL user */
extern cl_kernel cl_program_create_kernel(cl_program, const char*, cl_int*);

/* creates kernel objects for all kernel functions in program. */
extern cl_int cl_program_create_kernels_in_program(cl_program, cl_kernel*);

/* Create a program from OCL source */
extern cl_program
cl_program_create_from_source(cl_context ctx,
                              cl_uint count,
                              const char **strings,
                              const size_t *lengths,
                              cl_int *errcode_ret);

/* Directly create a program from a blob */
extern cl_program
cl_program_create_from_binary(cl_context             context,
                              cl_uint                num_devices,
                              const cl_device_id *   devices,
                              const size_t *         lengths,
                              const unsigned char ** binaries,
                              cl_int *               binary_status,
                              cl_int *               errcode_ret);

/* Create a program with built-in kernels*/
extern cl_program
cl_program_create_with_built_in_kernles(cl_context     context,
                                  cl_uint              num_devices,
                                  const cl_device_id * device_list,
                                  const char *         kernel_names,
                                  cl_int *             errcode_ret);
/* Directly create a program from a LLVM source file */
extern cl_program
cl_program_create_from_llvm(cl_context             context,
                            cl_uint                num_devices,
                            const cl_device_id *   devices,
                            const char *           fileName,
                            cl_int *               errcode_ret);

/* Build the program as specified by OCL */
extern cl_int
cl_program_build(cl_program p, const char* options);
/* Compile the program as specified by OCL */
extern cl_int
cl_program_compile(cl_program            p,
                   cl_uint               num_input_headers,
                   const cl_program *    input_headers,
                   const char **         header_include_names,
                   const char*           options);
/* link the program as specified by OCL */
extern cl_program
cl_program_link(cl_context            context,
                cl_uint               num_input_programs,
                const cl_program *    input_programs,
                const char *          options,
                cl_int*               errcode_ret);
/* Get the kernel names in program */
extern void
cl_program_get_kernel_names(cl_program p,
                            size_t size,
                            char *names,
                            size_t *size_ret);
#endif /* __CL_PROGRAM_H__ */

