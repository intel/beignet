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
 */

#ifndef __CL_PROGRAM_H__
#define __CL_PROGRAM_H__

#include "cl_base_object.h"
#include "CL/cl.h"

#define BUILD_LOG_MAX_SIZE (256 * 1024U)

typedef struct _cl_program_for_device {
  cl_device_id device;                /* Point to the device it belong to */
  char *binary;                       /* Program binary. */
  size_t binary_sz;                   /* The binary size. */
  cl_uint binary_type;                /* binary type: COMPILED_OBJECT(LLVM IR),
                                         LIBRARY(LLVM IR with option "-create-library"),
                                         or EXECUTABLE(GEN binary). */
  size_t build_log_max_sz;            /* build log maximum size in byte.*/
  char build_log[BUILD_LOG_MAX_SIZE]; /* The build log for this program. */
  size_t build_log_sz;                /* The actual build log size.*/
  cl_uint kernel_num;                 /* Kernel number */
  char **kernel_names;                /* All kernel names of this program */
} _cl_program_for_device;
typedef _cl_program_for_device *cl_program_for_device;

/* This maps an OCL file containing some kernels */
struct _cl_program {
  _cl_base_object base;
  cl_context ctx;                     /* Its parent context */
  char *source;                       /* Program sources */
  size_t source_sz;                   /* The source size. */
  cl_uint each_device_num;            /* Each device number */
  cl_program_for_device *each_device; /* Program content interpreted by device */
  char *build_opts;                   /* The build options for this program */
  cl_int build_status;                /* build status. */
  list_head kernels;                  /* All kernels belong to this program. */
  cl_uint ker_n;                      /* Number of declared kernels */
};

#define CL_OBJECT_PROGRAM_MAGIC 0x34562ab12789cdefLL
#define CL_OBJECT_IS_PROGRAM(obj) ((obj &&                                                     \
                                    ((cl_base_object)obj)->magic == CL_OBJECT_PROGRAM_MAGIC && \
                                    CL_OBJECT_GET_REF(obj) >= 1))

extern cl_program cl_program_new(cl_context);
extern void cl_program_delete(cl_program);
extern void cl_program_add_ref(cl_program);
extern cl_int cl_program_create_kernels_in_program(cl_program program, cl_uint num_kernels,
                                                   cl_kernel *kernels, cl_uint *num_kernels_ret);
extern cl_program cl_program_create_from_binary(cl_context ctx, cl_uint num_devices,
                                                const cl_device_id *devices, const size_t *lengths,
                                                const unsigned char **binaries, cl_int *binary_status,
                                                cl_int *errcode_ret);
extern cl_int cl_program_build(cl_program p, const char *options, cl_uint num_devices, const cl_device_id *device_list);
extern cl_int cl_program_compile(cl_program p, cl_uint num_input_headers, const cl_program *input_headers,
                                 const char **header_include_names, const char *options,
                                 cl_uint num_devices, const cl_device_id *device_list);
extern cl_program cl_program_link(cl_context context, cl_uint num_devices, const cl_device_id *device_list,
                                  cl_uint num_input_programs, const cl_program *input_programs,
                                  const char *options, cl_int *errcode_ret);
extern cl_int cl_program_get_kernel_names(cl_program p, size_t *kerne_num, size_t size, char *names, size_t *name_ret);
extern cl_program cl_program_create_with_built_in_kernles(cl_context context, cl_uint num_devices,
                                                          const cl_device_id *device_list, const char *kernel_names,
                                                          cl_int *errcode_ret);
extern void cl_program_take_out_of_context(cl_program p);

/* Create a program with built-in kernels*/
/* Directly create a program from a LLVM source file */
extern cl_program
cl_program_create_from_llvm(cl_context context,
                            cl_uint num_devices,
                            const cl_device_id *devices,
                            const char *fileName,
                            cl_int *errcode_ret);

/* Create a kernel for the OCL user */
extern cl_kernel cl_program_create_user_kernel(cl_program, const char *, cl_int *);

#endif /* __CL_PROGRAM_H__ */
