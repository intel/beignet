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
 * C interface for the Gen kernels and programs (either real Gen ISA or Gen
 * simulator). This is the only thing the run-time can see from the compiler
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
  GBE_ARG_GLOBAL_PTR = 1,       // __global
  GBE_ARG_CONSTANT_PTR = 2,     // __constant
  GBE_ARG_LOCAL_PTR = 3,        // __local
  GBE_ARG_IMAGE = 4,            // image2d_t, image3d_t
  GBE_ARG_SAMPLER = 5,          // sampler_t
  GBE_ARG_INVALID = 0xffffffff
};

/*! Constant buffer values (ie values to setup in the constant buffer) */
enum gbe_curbe_type {
  GBE_CURBE_LOCAL_ID_X = 0,
  GBE_CURBE_LOCAL_ID_Y,
  GBE_CURBE_LOCAL_ID_Z,
  GBE_CURBE_LOCAL_SIZE_X,
  GBE_CURBE_LOCAL_SIZE_Y,
  GBE_CURBE_LOCAL_SIZE_Z,
  GBE_CURBE_GLOBAL_SIZE_X,
  GBE_CURBE_GLOBAL_SIZE_Y,
  GBE_CURBE_GLOBAL_SIZE_Z,
  GBE_CURBE_GLOBAL_OFFSET_X,
  GBE_CURBE_GLOBAL_OFFSET_Y,
  GBE_CURBE_GLOBAL_OFFSET_Z,
  GBE_CURBE_GROUP_NUM_X,
  GBE_CURBE_GROUP_NUM_Y,
  GBE_CURBE_GROUP_NUM_Z,
  GBE_CURBE_WORK_DIM,
  GBE_CURBE_IMAGE_INFO,
  GBE_CURBE_STACK_POINTER,
  GBE_CURBE_KERNEL_ARGUMENT,
  GBE_CURBE_EXTRA_ARGUMENT,
  GBE_CURBE_BLOCK_IP,
  GBE_CURBE_THREAD_NUM,
  GBE_CURBE_ZERO,
  GBE_CURBE_ONE,
  GBE_CURBE_SLM_OFFSET,
};

/*! Extra arguments use the negative range of sub-values */
enum gbe_extra_argument {
  GBE_STACK_BUFFER = 0,   /* Give stack location in curbe */
  GBE_CONSTANT_BUFFER = 1 /* constant buffer argument location in curbe */
};

typedef struct ImageInfo {
    int32_t arg_idx;
    int32_t idx;
    int32_t wSlot;
    int32_t hSlot;
    int32_t depthSlot;
    int32_t dataTypeSlot;
    int32_t channelOrderSlot;
    int32_t dimOrderSlot;
} ImageInfo;

typedef void (gbe_set_image_base_index_cb)(uint32_t base_idx);
extern gbe_set_image_base_index_cb *gbe_set_image_base_index;

typedef uint32_t (gbe_get_image_base_index_cb)();
extern gbe_get_image_base_index_cb *gbe_get_image_base_index;

/*! Get the size of defined images */
typedef size_t (gbe_kernel_get_image_size_cb)(gbe_kernel gbeKernel);
extern gbe_kernel_get_image_size_cb *gbe_kernel_get_image_size;

/*! Get the content of defined images */
typedef void (gbe_kernel_get_image_data_cb)(gbe_kernel gbeKernel, ImageInfo *images);
extern gbe_kernel_get_image_data_cb *gbe_kernel_get_image_data;

/*! Create a new program from the given source code (zero terminated string) */
typedef gbe_program (gbe_program_new_from_source_cb)(uint32_t deviceID,
                                                     const char *source,
                                                     size_t stringSize,
                                                     const char *options,
                                                     char *err,
                                                     size_t *err_size);
extern gbe_program_new_from_source_cb *gbe_program_new_from_source;

/*! Create a new program from the given blob */
typedef gbe_program (gbe_program_new_from_binary_cb)(uint32_t deviceID, const char *binary, size_t size);
extern gbe_program_new_from_binary_cb *gbe_program_new_from_binary;

/*! Serialize a program to a bin */
typedef size_t (gbe_program_serialize_to_binary_cb)(gbe_program program, char **binary);
extern gbe_program_serialize_to_binary_cb *gbe_program_serialize_to_binary;

/*! Create a new program from the given LLVM file */
typedef gbe_program (gbe_program_new_from_llvm_cb)(uint32_t deviceID,
                                                   const char *fileName,
                                                   size_t string_size,
                                                   char *err,
                                                   size_t *err_size,
                                                   int optLevel);
extern gbe_program_new_from_llvm_cb *gbe_program_new_from_llvm;

/*! Get the size of global constants */
typedef size_t (gbe_program_get_global_constant_size_cb)(gbe_program gbeProgram);
extern gbe_program_get_global_constant_size_cb *gbe_program_get_global_constant_size;

/*! Get the content of global constants */
typedef void (gbe_program_get_global_constant_data_cb)(gbe_program gbeProgram, char *mem);
extern gbe_program_get_global_constant_data_cb *gbe_program_get_global_constant_data;

/*! Get the size of defined samplers */
typedef size_t (gbe_kernel_get_sampler_size_cb)(gbe_kernel gbeKernel);
extern gbe_kernel_get_sampler_size_cb *gbe_kernel_get_sampler_size;

/*! Get the content of defined samplers */
typedef void (gbe_kernel_get_sampler_data_cb)(gbe_kernel gbeKernel, uint32_t *samplers);
extern gbe_kernel_get_sampler_data_cb *gbe_kernel_get_sampler_data;

/*! Get the content of defined samplers */
typedef void (gbe_kernel_get_compile_wg_size_cb)(gbe_kernel gbeKernel, size_t wg_sz[3]);
extern gbe_kernel_get_compile_wg_size_cb *gbe_kernel_get_compile_wg_size;

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

/*! Get the kernel name */
typedef const char *(gbe_kernel_get_name_cb)(gbe_kernel);
extern gbe_kernel_get_name_cb *gbe_kernel_get_name;

/*! Get the kernel source code */
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

/*! Get the align of the given argument */
typedef uint32_t (gbe_kernel_get_arg_align_cb)(gbe_kernel, uint32_t argID);
extern gbe_kernel_get_arg_align_cb *gbe_kernel_get_arg_align;

/*! Get the simd width for the kernel */
typedef uint32_t (gbe_kernel_get_simd_width_cb)(gbe_kernel);
extern gbe_kernel_get_simd_width_cb *gbe_kernel_get_simd_width;

/*! Get the curbe size required by the kernel */
typedef int32_t (gbe_kernel_get_curbe_size_cb)(gbe_kernel);
extern gbe_kernel_get_curbe_size_cb *gbe_kernel_get_curbe_size;

/*! Get the stack size (zero if no stack is required) */
typedef int32_t (gbe_kernel_get_stack_size_cb)(gbe_kernel);
extern gbe_kernel_get_stack_size_cb *gbe_kernel_get_stack_size;

/*! Get the scratch size (zero if no scratch is required) */
typedef int32_t (gbe_kernel_get_scratch_size_cb)(gbe_kernel);
extern gbe_kernel_get_scratch_size_cb *gbe_kernel_get_scratch_size;

/*! Get the curbe offset where to put the data. Returns -1 if not required */
typedef int32_t (gbe_kernel_get_curbe_offset_cb)(gbe_kernel, enum gbe_curbe_type type, uint32_t sub_type);
extern gbe_kernel_get_curbe_offset_cb *gbe_kernel_get_curbe_offset;

/*! Set the constant pointer arg size and return the cb offset in curbe */
typedef int32_t (gbe_kernel_set_const_buffer_size_cb)(gbe_kernel, uint32_t argID, size_t sz);
extern gbe_kernel_set_const_buffer_size_cb *gbe_kernel_set_const_buffer_size;

/*! Indicates if a work group size is required. Return the required width or 0
 *  if none
 */
typedef uint32_t (gbe_kernel_get_required_work_group_size_cb)(gbe_kernel, uint32_t dim);
extern gbe_kernel_get_required_work_group_size_cb *gbe_kernel_get_required_work_group_size;

/*! Says if SLM is used. Required to reconfigure the L3 complex */
typedef int32_t (gbe_kernel_use_slm_cb)(gbe_kernel);
extern gbe_kernel_use_slm_cb *gbe_kernel_use_slm;
/*! Get slm size needed for kernel local variables */
typedef int32_t (gbe_kernel_get_slm_size_cb)(gbe_kernel);
extern gbe_kernel_get_slm_size_cb *gbe_kernel_get_slm_size;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GBE_PROGRAM_H__ */

