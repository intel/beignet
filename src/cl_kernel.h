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

#ifndef __CL_KERNEL_H__
#define __CL_KERNEL_H__

#include "cl_internals.h"
#include "cl_base_object.h"
#include "cl_driver.h"
#include "cl_gbe_loader.h"
#include "CL/cl.h"
#include "CL/cl_ext.h"

#include <stdint.h>
#include <stdlib.h>

/* This is the kernel as it is interfaced by the compiler */
struct _gbe_kernel;

/* We need to save buffer data for relocation and binding and we must figure out
 * if all arguments are properly set
 */
typedef struct cl_argument {
  cl_mem mem;           /* For image and regular buffers */
  cl_sampler sampler;   /* For sampler. */
  cl_accelerator_intel accel;
  unsigned char bti;
  void *ptr;            /* SVM ptr value. */
  uint32_t local_sz:30; /* For __local size specification */
  uint32_t is_set:1;    /* All args must be set before NDRange */
  uint32_t is_svm:1;    /* Indicate this argument is SVMPointer */
} cl_argument;

/* One OCL function */
struct _cl_kernel {
  _cl_base_object base;
  cl_buffer bo;               /* The code itself */
  cl_program program;         /* Owns this structure (and pointers) */
  gbe_kernel opaque;          /* (Opaque) compiler structure for the OCL kernel */
  cl_accelerator_intel accel;     /* accelerator */
  char *curbe;                /* One curbe per kernel */
  size_t curbe_sz;            /* Size of it */
  uint32_t samplers[GEN_MAX_SAMPLERS]; /* samplers defined in kernel & kernel args */
  size_t sampler_sz;          /* sampler size defined in kernel & kernel args. */
  struct ImageInfo *images;   /* images defined in kernel args */
  size_t image_sz;            /* image count in kernel args */
  cl_ulong local_mem_sz;      /* local memory size specified in kernel args. */
  size_t compile_wg_sz[3];    /* Required workgroup size by __attribute__((reqd_work_gro
                                 up_size(X, Y, Z))) qualifier.*/
  size_t global_work_sz[3];    /* maximum global size that can be used to execute a kernel
                                (i.e. global_work_size argument to clEnqueueNDRangeKernel.)*/
  size_t stack_size;          /* stack size per work item. */
  cl_argument *args;          /* To track argument setting */
  uint32_t arg_n:30;          /* Number of arguments */
  uint32_t ref_its_program:1; /* True only for the user kernel (created by clCreateKernel) */
  uint32_t vme:1;             /* True only if it is a built-in kernel for VME */

  void* cmrt_kernel;          /* CmKernel* */
  uint32_t exec_info_n;       /* The kernel's exec info count */
  void** exec_info;             /* The kernel's exec info */
  cl_bool useDeviceEnqueue;     /* kernel use device enqueue */
  void* device_enqueue_ptr;     /* device_enqueue buffer*/
  uint32_t device_enqueue_info_n; /* count of parent kernel's arguments buffers, as child enqueues' exec info */
  void** device_enqueue_infos;   /* parent kernel's arguments buffers, as child enqueues' exec info   */
};

#define CL_OBJECT_KERNEL_MAGIC 0x1234567890abedefLL
#define CL_OBJECT_IS_KERNEL(obj) ((obj &&                           \
         ((cl_base_object)obj)->magic == CL_OBJECT_KERNEL_MAGIC &&  \
         CL_OBJECT_GET_REF(obj) >= 1))

/* Allocate an empty kernel */
extern cl_kernel cl_kernel_new(cl_program);

/* Destroy and deallocate an empty kernel */
extern void cl_kernel_delete(cl_kernel);

/* Setup the kernel with the given GBE Kernel */
extern void cl_kernel_setup(cl_kernel k, gbe_kernel opaque);

/* Get the kernel name */
extern const char *cl_kernel_get_name(cl_kernel k);

/* Get the kernel attributes*/
extern const char *cl_kernel_get_attributes(cl_kernel k);

/* Get the simd width as used in the code */
extern uint32_t cl_kernel_get_simd_width(cl_kernel k);

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
extern int cl_kernel_set_arg_svm_pointer(cl_kernel,
                                            uint32_t arg_index,
                                            const void *arg_value);
extern cl_int cl_kernel_set_exec_info(cl_kernel k,
                                      size_t n,
                                      const void *value);

/* Get the argument information */
extern int cl_get_kernel_arg_info(cl_kernel k, cl_uint arg_index,
                                  cl_kernel_arg_info param_name,
                                  size_t param_value_size, void *param_value,
                                  size_t *param_value_size_ret);

/* Compute and check the work group size from the user provided local size */
extern cl_int
cl_kernel_work_group_sz(cl_kernel ker,
                        const size_t *local_wk_sz,
                        cl_uint wk_dim,
                        size_t *wk_grp_sz);

#endif /* __CL_KERNEL_H__ */

