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

#include "cl_kernel.h"
#include "cl_program.h"
#include "cl_device_id.h"
#include "cl_event.h"
#include "cl_alloc.h"
#include "cl_sampler.h"
#include "cl_mem.h"
#include "cl_command_queue.h"
#include <string.h>

static void
cl_kernel_arg_delete(cl_argument arg)
{
  assert(arg);

  if (arg->is_set == CL_FALSE) {
    return;
  }

  if (arg->arg_type != ArgTypeValue && arg->arg_type != ArgTypeStruct) {
    return;
  }

  if (arg->arg_type == ArgTypeValue && arg->arg_size > sizeof(cl_double)) {
    CL_FREE(arg->val.val_ptr);
  } else if (arg->arg_type == ArgTypeStruct) {
    CL_FREE(arg->val.val_ptr);
  }

  arg->is_set = CL_FALSE;
  arg->use_svm = CL_FALSE;
  return;
}

LOCAL cl_int
cl_kernel_set_arg_svm_pointer(cl_kernel k, cl_uint index, const void *value)
{
  cl_int i;
  cl_argument arg = NULL;
  cl_mem svm = cl_context_get_svm_by_ptr(k->program->ctx, value, CL_FALSE);
  if (svm == NULL)
    return CL_INVALID_ARG_VALUE;

  if (index >= k->arg_n)
    return CL_INVALID_ARG_INDEX;

  for (i = 0; i < k->arg_n; i++) {
    if (k->args[i].arg_no == index) {
      arg = &k->args[i];
      break;
    }
  }
  assert(arg);

  if (arg->is_set) {
    cl_kernel_arg_delete(arg);
  }

  if (arg->arg_type != ArgTypePointer)
    return CL_INVALID_ARG_VALUE;

  if (arg->arg_addrspace != AddressSpaceGlobal &&
      arg->arg_addrspace != AddressSpaceConstant)
    return CL_INVALID_ARG_VALUE;

  arg->val.val_svm.svm = svm;
  arg->val.val_svm.ptr = (void *)value;
  arg->val_size = sizeof(cl_mem);
  arg->is_set = CL_TRUE;
  arg->use_svm = CL_TRUE;
  return CL_SUCCESS;
}

LOCAL cl_int
cl_kernel_set_arg(cl_kernel kernel, cl_uint index, size_t sz, const void *value)
{
  cl_int i;
  cl_argument arg = NULL;

  if (index >= kernel->arg_n)
    return CL_INVALID_ARG_INDEX;

  for (i = 0; i < kernel->arg_n; i++) {
    if (kernel->args[i].arg_no == index) {
      arg = &kernel->args[i];
      break;
    }
  }
  assert(arg);

  if (arg->is_set) {
    cl_kernel_arg_delete(arg);
  }

  /* Local mem is special, the size is the local mem's size to be allocated. */
  if (arg->arg_type == ArgTypePointer && arg->arg_addrspace == AddressSpaceLocal) {
    if (sz == 0)
      return CL_INVALID_ARG_SIZE;

    if (value != NULL)
      return CL_INVALID_ARG_VALUE;

    arg->val_size = sz;
    arg->is_set = CL_TRUE;
    return CL_SUCCESS;
  }

  if (sz != arg->arg_size)
    return CL_INVALID_ARG_SIZE;

  /* For constant and global mem, we should have a cl_mem object, and it is a buffer. */
  if (arg->arg_type == ArgTypePointer) {
    assert(arg->arg_addrspace != AddressSpaceLocal);
    if (value == NULL || *((cl_mem *)value) == NULL) {
      arg->val.val_mem = NULL;
    } else {
      if (!CL_OBJECT_IS_MEM(*(cl_mem *)value))
        return CL_INVALID_ARG_VALUE;

      arg->val.val_mem = *(cl_mem *)value;
    }

    arg->val_size = sizeof(cl_mem);
    arg->is_set = CL_TRUE;
    return CL_SUCCESS;
  }

  /* For image, we should have a cl_mem object, and it is a image. */
  if (arg->arg_type == ArgTypeImage) {
    if (!CL_OBJECT_IS_MEM(*(cl_mem *)value))
      return CL_INVALID_ARG_VALUE;

    arg->val.val_mem = *(cl_mem *)value;
    arg->val_size = sizeof(cl_mem);
    arg->is_set = CL_TRUE;
    return CL_SUCCESS;
  }

  if (arg->arg_type == ArgTypePipe) {
    if (!CL_OBJECT_IS_MEM(*(cl_mem *)value))
      return CL_INVALID_ARG_VALUE;

    arg->val.val_mem = *(cl_mem *)value;
    arg->val_size = sizeof(cl_mem);
    arg->is_set = CL_TRUE;
    return CL_SUCCESS;
  }

  /* For image, we should have a cl_mem object, and it is a image. */
  if (arg->arg_type == ArgTypeSampler) {
    if (!CL_OBJECT_IS_SAMPLER(*(cl_sampler *)value))
      return CL_INVALID_ARG_VALUE;

    arg->val.val_sampler = *(cl_sampler *)value;
    arg->val_size = sizeof(cl_sampler);
    arg->is_set = CL_TRUE;
    return CL_SUCCESS;
  }

  if (arg->arg_type == ArgTypeValue && arg->arg_size <= sizeof(cl_double)) {
    memcpy(&arg->val, value, sz);
    arg->is_set = CL_TRUE;
    arg->val_size = arg->arg_size;
    return CL_SUCCESS;
  }

  arg->val.val_ptr = CL_MALLOC(sz);
  if (arg->val.val_ptr == NULL)
    return CL_OUT_OF_HOST_MEMORY;

  memset(arg->val.val_ptr, 0, sz);
  memcpy(arg->val.val_ptr, value, sz);
  arg->val_size = arg->arg_size;
  arg->is_set = CL_TRUE;
  return CL_SUCCESS;
}

LOCAL int
cl_kernel_get_argument_info(cl_kernel k, cl_uint arg_index, cl_kernel_arg_info param_name,
                            size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
  assert(k != NULL);
  int str_len = 0;

  switch (param_name) {
  case CL_KERNEL_ARG_ADDRESS_QUALIFIER:
    if (param_value_size_ret)
      *param_value_size_ret = sizeof(cl_kernel_arg_address_qualifier);
    if (!param_value)
      return CL_SUCCESS;

    if (param_value_size < sizeof(cl_kernel_arg_address_qualifier))
      return CL_INVALID_VALUE;

    if (k->args[arg_index].arg_addrspace == AddressSpaceGlobal) {
      *(cl_kernel_arg_address_qualifier *)param_value = CL_KERNEL_ARG_ADDRESS_GLOBAL;
    } else if (k->args[arg_index].arg_addrspace == AddressSpaceConstant) {
      *(cl_kernel_arg_address_qualifier *)param_value = CL_KERNEL_ARG_ADDRESS_CONSTANT;
    } else if (k->args[arg_index].arg_addrspace == AddressSpaceLocal) {
      *(cl_kernel_arg_address_qualifier *)param_value = CL_KERNEL_ARG_ADDRESS_LOCAL;
    } else {
      /* If no address qualifier is specified, the default address qualifier
         which is CL_KERNEL_ARG_ADDRESS_PRIVATE is returned. */
      *(cl_kernel_arg_address_qualifier *)param_value = CL_KERNEL_ARG_ADDRESS_PRIVATE;
    }
    return CL_SUCCESS;

  case CL_KERNEL_ARG_ACCESS_QUALIFIER:
    if (k->args[arg_index].arg_access_qualifier == 0)
      return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
    if (param_value_size_ret)
      *param_value_size_ret = sizeof(cl_kernel_arg_access_qualifier);
    if (!param_value)
      return CL_SUCCESS;
    if (param_value_size < sizeof(cl_kernel_arg_access_qualifier))
      return CL_INVALID_VALUE;
    *(cl_kernel_arg_address_qualifier *)param_value = k->args[arg_index].arg_access_qualifier;
    return CL_SUCCESS;

  case CL_KERNEL_ARG_TYPE_NAME:
    if (k->args[arg_index].arg_type_name == NULL)
      return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
    str_len = strlen(k->args[arg_index].arg_type_name);
    if (param_value_size_ret)
      *param_value_size_ret = str_len + 1;
    if (!param_value)
      return CL_SUCCESS;
    if (param_value_size < str_len + 1)
      return CL_INVALID_VALUE;

    memcpy(param_value, k->args[arg_index].arg_type_name, str_len);
    ((char *)param_value)[str_len] = 0;
    return CL_SUCCESS;

  case CL_KERNEL_ARG_NAME:
    if (k->args[arg_index].arg_name == NULL)
      return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
    str_len = strlen(k->args[arg_index].arg_name);
    if (param_value_size_ret)
      *param_value_size_ret = str_len + 1;
    if (!param_value)
      return CL_SUCCESS;
    if (param_value_size < str_len + 1)
      return CL_INVALID_VALUE;

    memcpy(param_value, k->args[arg_index].arg_name, str_len);
    ((char *)param_value)[str_len] = 0;
    return CL_SUCCESS;

  case CL_KERNEL_ARG_TYPE_QUALIFIER:
    if ((k->args[arg_index].arg_type_qualifier &
         (~(CL_KERNEL_ARG_TYPE_NONE | CL_KERNEL_ARG_TYPE_CONST |
            CL_KERNEL_ARG_TYPE_RESTRICT | CL_KERNEL_ARG_TYPE_VOLATILE |
            CL_KERNEL_ARG_TYPE_PIPE))) != 0)
      return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
    if (param_value_size_ret)
      *param_value_size_ret = sizeof(cl_kernel_arg_type_qualifier);
    if (!param_value)
      return CL_SUCCESS;
    if (param_value_size < sizeof(cl_kernel_arg_type_qualifier))
      return CL_INVALID_VALUE;

    *(cl_kernel_arg_type_qualifier *)param_value = k->args[arg_index].arg_type_qualifier;
    return CL_SUCCESS;

  default:
    assert(0);
  }

  return CL_SUCCESS;
}

LOCAL cl_kernel
cl_kernel_new(cl_program p, const char *name)
{
  cl_kernel k = NULL;

  k = CL_CALLOC(1, sizeof(struct _cl_kernel));
  if (k == NULL)
    return NULL;

  CL_OBJECT_INIT_BASE(k, CL_OBJECT_KERNEL_MAGIC);
  k->program = p;

  k->name = CL_CALLOC(1, strlen(name) + 1);
  if (k->name == NULL) {
    CL_FREE(k);
    return NULL;
  }
  memcpy(k->name, name, strlen(name) + 1);

  k->each_device = CL_CALLOC(p->each_device_num, sizeof(cl_kernel_for_device));
  if (k->each_device == NULL) {
    CL_FREE(k->name);
    CL_FREE(k);
    return NULL;
  }
  k->each_device_num = p->each_device_num;

  /* Add it to program's user kernels list. */
  cl_program_add_ref(p);
  CL_OBJECT_LOCK(p);
  list_add_tail(&p->kernels, &k->base.node);
  p->ker_n++;
  CL_OBJECT_UNLOCK(p);
  return k;
}

LOCAL void
cl_kernel_delete(cl_kernel k)
{
  cl_uint i;
  if (k == NULL)
    return;

  /* We are not done with the kernel */
  if (CL_OBJECT_DEC_REF(k) > 1)
    return;

  CL_OBJECT_LOCK(k->program);
  list_node_del(&k->base.node);
  k->program->ker_n--;
  CL_OBJECT_UNLOCK(k->program);
  cl_program_delete(k->program);

  if (k->name)
    CL_FREE(k->name);
  k->name = NULL;

  if (k->kernel_attr)
    CL_FREE(k->kernel_attr);
  k->kernel_attr = NULL;

  if (k->exec_info) {
    assert(k->exec_info_n > 0);
    CL_FREE(k->exec_info);
    k->exec_info = NULL;
    k->exec_info_n = 0;
  }

  for (i = 0; i < k->each_device_num; i++) {
    if (k->each_device[i])
      (k->each_device[i]->device->api.kernel_delete)(k->each_device[i]->device, k);
  }
  CL_FREE(k->each_device);

  if (k->args) {
    for (i = 0; i < k->arg_n; i++) {
      if (k->args[i].arg_name)
        CL_FREE(k->args[i].arg_name);
      if (k->args[i].arg_type_name)
        CL_FREE(k->args[i].arg_type_name);
      cl_kernel_arg_delete(&k->args[i]);
    }

    CL_FREE(k->args);
    k->args = NULL;
  }

  CL_OBJECT_DESTROY_BASE(k);
  CL_FREE(k);
}

LOCAL void
cl_kernel_add_ref(cl_kernel k)
{
  CL_OBJECT_INC_REF(k);
}

LOCAL cl_kernel
cl_kernel_create(cl_program p, const char *name, cl_int *errcode_ret)
{
  cl_kernel kernel = NULL;
  cl_uint i, j;
  cl_int err = CL_SUCCESS;
  int someone_created;
  cl_bool find;

  assert(p->each_device);
  assert(name);

  if (CL_OBJECT_TAKE_OWNERSHIP(p, CL_FALSE) == CL_FALSE) {
    *errcode_ret = CL_INVALID_OPERATION;
    return NULL;
  }

  if (p->build_status != CL_BUILD_SUCCESS) {
    *errcode_ret = CL_INVALID_PROGRAM_EXECUTABLE;
    return NULL;
  }

  /* Need to find it in at least one device's program */
  find = CL_FALSE;
  for (i = 0; i < p->each_device_num; i++) {
    for (j = 0; j < p->each_device[i]->kernel_num; j++) {
      if (strcmp(p->each_device[i]->kernel_names[j], name) == 0)
        find = CL_TRUE;
    }
  }
  if (find == CL_FALSE) {
    *errcode_ret = CL_INVALID_KERNEL_NAME;
    CL_OBJECT_RELEASE_OWNERSHIP(p);
    return NULL;
  }

  kernel = cl_kernel_new(p, name);
  CL_OBJECT_RELEASE_OWNERSHIP(p);
  if (kernel == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }

  someone_created = 0;
  for (i = 0; i < p->each_device_num; i++) {
    err = (p->each_device[i]->device->api.kernel_create)(p->each_device[i]->device, kernel);
    if (err == CL_INVALID_KERNEL_DEFINITION) { // Conflict kernel define, can not go on
      *errcode_ret = CL_INVALID_KERNEL_DEFINITION;
      break;
    }

    if (err == CL_SUCCESS) { // Once success, this kernel can be created
      someone_created = 1;
    }
  }

  if (*errcode_ret != CL_SUCCESS) {
    cl_kernel_delete(kernel);
    return NULL;
  }
  if (someone_created == 0) {
    assert(err != CL_SUCCESS);
    *errcode_ret = err;
    cl_kernel_delete(kernel);
    return NULL;
  }

  *errcode_ret = CL_SUCCESS;
  return kernel;
}

LOCAL cl_int
cl_kernel_get_workgroup_info(cl_kernel kernel, cl_device_id device, cl_kernel_work_group_info param_name,
                             size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;
  size_t parameter_data = 0;
  size_t wk_size[3] = {0, 0, 0};
  cl_int err = CL_SUCCESS;

  if (device == NULL) {
    assert(kernel->each_device_num == 1);
    device = kernel->each_device[0]->device;
  }

  switch (param_name) {
  case CL_KERNEL_WORK_GROUP_SIZE:
  case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
  case CL_KERNEL_LOCAL_MEM_SIZE:
  case CL_KERNEL_PRIVATE_MEM_SIZE: {
    err = device->api.kernel_get_info(device, kernel, param_name, &parameter_data);
    src_ptr = &parameter_data;
    src_size = sizeof(size_t);
    break;
  }
  case CL_KERNEL_COMPILE_WORK_GROUP_SIZE: {
    src_ptr = kernel->compile_wg_sz;
    src_size = sizeof(size_t) * 3;
    break;
  }
  case CL_KERNEL_GLOBAL_WORK_SIZE: {
    err = device->api.kernel_get_info(device, kernel, param_name, wk_size);
    src_ptr = wk_size;
    src_size = sizeof(size_t) * 3;
    break;
  }
  default:
    return CL_INVALID_VALUE;
  }

  if (err != CL_SUCCESS)
    return err;

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

LOCAL cl_int
cl_kernel_get_subgroup_info(cl_kernel kernel, cl_device_id device, cl_kernel_work_group_info param_name,
                            size_t input_value_size, const void *input_value, size_t param_value_size,
                            void *param_value, size_t *param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;
  size_t parameter_data = 0;
  cl_int err = CL_SUCCESS;

  switch (param_name) {
  case CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE_KHR: {
    int i, dim = 0;
    size_t local_sz = 1;
    size_t prefer_sz = 0;

    switch (input_value_size) {
    case sizeof(size_t) * 1:
    case sizeof(size_t) * 2:
    case sizeof(size_t) * 3:
      dim = input_value_size / sizeof(size_t);
      break;
    default:
      return CL_INVALID_VALUE;
    }

    if (input_value == NULL)
      return CL_INVALID_VALUE;

    for (i = 0; i < dim; i++)
      local_sz *= ((size_t *)input_value)[i];

    err = device->api.kernel_get_info(device, kernel, param_name, &prefer_sz);
    if (err != CL_SUCCESS)
      return err;

    parameter_data = local_sz >= prefer_sz ? prefer_sz : local_sz;
    src_ptr = &parameter_data;
    src_size = sizeof(size_t);
    break;
  }

  case CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE_KHR: {
    int i, dim = 0;
    size_t local_sz = 1;
    size_t prefer_sz = 0;

    switch (input_value_size) {
    case sizeof(size_t) * 1:
    case sizeof(size_t) * 2:
    case sizeof(size_t) * 3:
      dim = input_value_size / sizeof(size_t);
      break;
    default:
      return CL_INVALID_VALUE;
    }

    if (input_value == NULL)
      return CL_INVALID_VALUE;

    for (i = 0; i < dim; i++)
      local_sz *= ((size_t *)input_value)[i];

    err = device->api.kernel_get_info(device, kernel,
                                      CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE_KHR, &prefer_sz);
    if (err != CL_SUCCESS)
      return err;
    parameter_data = (local_sz + prefer_sz - 1) / prefer_sz;
    src_ptr = &parameter_data;
    src_size = sizeof(size_t);
    break;
  }
  default:
    return CL_INVALID_VALUE;
  };

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

LOCAL cl_int
cl_kernel_set_exec_info(cl_kernel k, size_t n, const void *value)
{
  size_t i;
  cl_mem svm;

  assert(k != NULL);

  if (n == 0)
    return CL_SUCCESS;

  if (k->exec_info) { // Already set
    assert(k->exec_info_n > 0);
    CL_FREE(k->exec_info);
    k->exec_info_n = 0;
  }

  k->exec_info = CL_CALLOC(n, sizeof(_cl_kernel_exec_svm_info));
  if (k->exec_info == NULL)
    return CL_OUT_OF_HOST_MEMORY;

  for (i = 0; i < n / sizeof(void *); i++) { // Assure all the ptr are svm allocated */
    svm = cl_context_get_svm_by_ptr(k->program->ctx, ((void **)value)[i], CL_FALSE);
    if (svm == NULL) {
      CL_FREE(k->exec_info);
      k->exec_info = NULL;
      return CL_INVALID_OPERATION;
    }

    k->exec_info[i].svm = svm;
    assert(svm->host_ptr);
    k->exec_info[i].offset = svm->host_ptr - ((void **)value)[i];
  }

  k->exec_info_n = n / sizeof(void *);
  return CL_SUCCESS;
}

LOCAL cl_int
cl_enqueue_handle_kernel_ndrange(cl_event e, cl_int status)
{
  cl_int err = CL_SUCCESS;

  if (status == CL_QUEUED) {
    cl_uint i;
    cl_kernel k = e->exec_data.nd_range.kernel;
    assert(k);

    /* Check that the user did not forget any argument */
    for (i = 0; i < k->arg_n; ++i) {
      if (k->args[i].is_set == CL_FALSE)
        return CL_INVALID_KERNEL_ARGS;

      if ((k->args[i].arg_type == ArgTypePointer && k->args[i].arg_addrspace != AddressSpaceLocal) ||
          k->args[i].arg_type == ArgTypeImage ||
          k->args[i].arg_type == ArgTypePipe) {
        if (k->args[i].val.val_mem) {
          err = cl_mem_assure_allocated(e->queue->device, k->args[i].val.val_mem);
          if (err != CL_SUCCESS)
            return err;
        }
      }
    }
  }

  err = e->queue->device->api.nd_range_kernel(e, status);
  return err;
}
