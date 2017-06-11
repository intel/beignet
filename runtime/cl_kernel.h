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

#include "cl_base_object.h"
#include "CL/cl.h"

typedef enum cl_address_space_type {
  AddressSpacePrivate = 0,
  AddressSpaceGlobal = 1,
  AddressSpaceConstant = 2,
  AddressSpaceLocal = 3,
} cl_address_space_type;

typedef enum cl_arg_type {
  ArgTypeInvalid = 0,
  ArgTypeValue, // int, float, double, long, etc
  ArgTypeStruct,
  ArgTypePointer,
  ArgTypeImage,
  ArgTypeSampler,
  ArgTypePipe,
} cl_arg_type;

typedef struct _cl_argument {
  cl_arg_type arg_type;
  cl_uint arg_no;
  cl_uint arg_size; // size in bytes
  cl_address_space_type arg_addrspace;
  cl_uint arg_type_qualifier;
  cl_uint arg_access_qualifier;
  char *arg_name;
  char *arg_type_name;
  cl_bool use_svm;

  union {
    cl_char val_char;
    cl_short val_short;
    cl_int val_int;
    cl_long val_long;
    cl_half val_half;
    cl_float val_float;
    cl_double val_double;
    cl_sampler val_sampler;
    cl_mem val_mem;
    struct {
      cl_mem svm;
      void *ptr;
    } val_svm;
    void *val_ptr;
  } val;
  cl_uint val_size;
  cl_bool is_set; /* All args must be set before NDRange */
} _cl_argument;
typedef _cl_argument *cl_argument;

typedef struct _cl_kernel_for_device {
  cl_device_id device;
  void *exec_code;      /* The binary for exec */
  cl_uint exec_code_sz; /* The binary for exec size */
} _cl_kernel_for_device;
typedef _cl_kernel_for_device *cl_kernel_for_device;

typedef struct _cl_kernel_exec_svm_info {
  cl_mem svm;
  size_t offset;
} _cl_kernel_exec_svm_info;
typedef _cl_kernel_exec_svm_info *cl_kernel_exec_svm_info;

/* One OCL function */
typedef struct _cl_kernel {
  _cl_base_object base;
  cl_program program;                /* Point back to program */
  char *name;                        /* The kernel name */
  cl_argument args;                  /* All the arguments */
  cl_uint arg_n;                     /* Number of arguments */
  size_t compile_wg_sz[3];           /* Required workgroup size by
                                        __attribute__((reqd_work_group_size(X, Y, Z))) qualifier */
  char *kernel_attr;                 /* The kernel attribute */
  cl_uint each_device_num;           /* Each device number */
  cl_kernel_for_device *each_device; /* Program content interpreted by device */
  cl_kernel_exec_svm_info exec_info; /* The kernel's exec info */
  cl_uint exec_info_n;               /* The kernel's exec info count */
} _cl_kernel;

#define CL_OBJECT_KERNEL_MAGIC 0x1234567890abedefLL
#define CL_OBJECT_IS_KERNEL(obj) ((obj &&                                                    \
                                   ((cl_base_object)obj)->magic == CL_OBJECT_KERNEL_MAGIC && \
                                   CL_OBJECT_GET_REF(obj) >= 1))

extern cl_kernel cl_kernel_create(cl_program p, const char *name, cl_int *errcode_ret);
extern cl_kernel cl_kernel_new(cl_program, const char *name);
extern void cl_kernel_delete(cl_kernel);
extern void cl_kernel_add_ref(cl_kernel);
extern int cl_kernel_set_arg(cl_kernel, uint32_t arg_index, size_t arg_size, const void *arg_value);
extern int cl_kernel_get_argument_info(cl_kernel k, cl_uint arg_index, cl_kernel_arg_info param_name,
                                       size_t param_value_size, void *param_value, size_t *param_value_size_ret);
extern cl_int cl_kernel_get_workgroup_info(cl_kernel kernel, cl_device_id device,
                                           cl_kernel_work_group_info param_name, size_t param_value_size,
                                           void *param_value, size_t *param_value_size_ret);
extern cl_int cl_kernel_get_subgroup_info(cl_kernel kernel, cl_device_id device,
                                          cl_kernel_work_group_info param_name, size_t input_value_size,
                                          const void *input_value, size_t param_value_size,
                                          void *param_value, size_t *param_value_size_ret);

/* Set the argument before kernel execution */
extern int cl_kernel_set_arg_svm_pointer(cl_kernel, uint32_t arg_index, const void *arg_value);
extern cl_int cl_kernel_set_exec_info(cl_kernel k, size_t n, const void *value);
extern cl_int cl_kernel_work_group_sz(cl_kernel ker, const size_t *local_wk_sz,
                                      cl_uint wk_dim, size_t *wk_grp_sz);
extern cl_int cl_enqueue_handle_kernel_ndrange(cl_event e, cl_int status);
#endif /* __CL_KERNEL_H__ */
