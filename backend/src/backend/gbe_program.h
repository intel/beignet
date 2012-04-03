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

/**
 * \file program.h
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * C-like interface for the gen kernels and programs (either real Gen ISA or Gen
 * simulator)
 */

#ifndef __GBE_PROGRAM_H__
#define __GBE_PROGRAM_H__

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! Opaque structure that interfaces a GBE program */
typedef struct _gbe_program *gbe_program;

/*! Opaque structure that interfaces a GBE kernel (ie one OCL function) */
typedef struct _gbe_kernel *gbe_kernel;

/*! Argument type for each function call */
enum gbe_arg_type {
  GBE_ARG_VALUE = 0,            // int, float and so on
  GBE_ARG_GLOBAL_PTR = 1,       // __global, __constant
  GBE_ARG_STRUCTURE = 2,        // By value structure
  GBE_ARG_IMAGE = 3,            // image2d_t, image3d_t
  GBE_ARG_INVALID = 0xffffffff
};

/*! Create a new program from the given source code (zero terminated string) */
typedef gbe_program (gbe_program_new_from_source_cb)(const char *source);
extern gbe_program_new_from_source_cb *gbe_program_new_from_source;

/*! Create a new program from the given blob */
typedef gbe_program (gbe_program_new_from_binary_cb)(const char *binary, size_t size);
extern gbe_program_new_from_binary_cb *gbe_program_new_from_binary;

/*! Create a new program from the given LLVM file */
typedef gbe_program (gbe_program_new_from_llvm_cb)(const char *fileName,
                                                   size_t string_size,
                                                   char *err,
                                                   size_t *err_size);
extern gbe_program_new_from_llvm_cb *gbe_program_new_from_llvm;

/*! Destroy and deallocate the given program */
typedef void (gbe_program_delete_cb)(gbe_program);
extern gbe_program_delete_cb *gbe_program_delete;

/*! Get the number of functions in the program */
typedef uint32_t (gbe_program_get_kernel_num_cb)(gbe_program);
extern gbe_program_get_kernel_num_cb *gbe_program_get_kernel_num;

/*! Get the kernel from its name */
typedef gbe_kernel (gbe_program_get_kernel_by_name_cb)(gbe_program, const char *name);
extern gbe_program_get_kernel_by_name_cb *gbe_program_get_kernel_by_name;

/*! Get the kernel from its ID */
typedef gbe_kernel (gbe_program_get_kernel_cb)(gbe_program, uint32_t ID);
extern gbe_program_get_kernel_cb *gbe_program_get_kernel;

/*! Get the GBE kernel name */
typedef const char *(gbe_kernel_get_name_cb)(gbe_kernel);
extern gbe_kernel_get_name_cb *gbe_kernel_get_name;

/*! Get the GBE kernel source code */
typedef const char *(gbe_kernel_get_code_cb)(gbe_kernel);
extern gbe_kernel_get_code_cb *gbe_kernel_get_code;

/*! Get the size of the source code */
typedef size_t (gbe_kernel_get_code_size_cb)(gbe_kernel);
extern gbe_kernel_get_code_size_cb *gbe_kernel_get_code_size;

/*! Get the total number of arguments */
typedef uint32_t (gbe_kernel_get_arg_num_cb)(gbe_kernel);
extern gbe_kernel_get_arg_num_cb *gbe_kernel_get_arg_num;

/*! Get the size of the given argument */
typedef uint32_t (gbe_kernel_get_arg_size_cb)(gbe_kernel, uint32_t argID);
extern gbe_kernel_get_arg_size_cb *gbe_kernel_get_arg_size;

/*! Get the type of the given argument */
typedef enum gbe_arg_type (gbe_kernel_get_arg_type_cb)(gbe_kernel, uint32_t argID);
extern gbe_kernel_get_arg_type_cb *gbe_kernel_get_arg_type;

/*! Get the simd width for the kernel */
typedef uint32_t (gbe_kernel_get_simd_width_cb)(gbe_kernel);
extern gbe_kernel_get_simd_width_cb *gbe_kernel_get_simd_width;

/*! Indicates if a work group size is required. Return the required width or 0
 *  if none
 */
typedef uint32_t (gbe_kernel_get_required_work_group_size_cb)(gbe_kernel, uint32_t dim);
extern gbe_kernel_get_required_work_group_size_cb *gbe_kernel_get_required_work_group_size;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GBE_PROGRAM_H__ */

