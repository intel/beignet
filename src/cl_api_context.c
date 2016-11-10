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

#include "cl_context.h"
#include "cl_device_id.h"
#include "cl_alloc.h"

cl_context
clCreateContext(const cl_context_properties *properties,
                cl_uint num_devices,
                const cl_device_id *devices,
                void (*pfn_notify)(const char *, const void *, size_t, void *),
                void *user_data,
                cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_context context = NULL;

  do {
    /* Assure parameters correctness */
    if (devices == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (num_devices == 0) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (pfn_notify == NULL && user_data != NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    err = cl_devices_list_check(num_devices, devices);
    if (err != CL_SUCCESS)
      break;

    context = cl_create_context(properties, num_devices, devices, pfn_notify, user_data, &err);
  } while (0);

  if (errcode_ret)
    *errcode_ret = err;
  return context;
}

cl_context
clCreateContextFromType(const cl_context_properties *properties,
                        cl_device_type device_type,
                        void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                        void *user_data,
                        cl_int *errcode_ret)
{
  cl_context context = NULL;
  cl_int err = CL_SUCCESS;
  cl_device_id *devices = NULL;
  cl_uint num_devices = 0;
  const cl_device_type valid_type = CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_ACCELERATOR |
                                    CL_DEVICE_TYPE_DEFAULT | CL_DEVICE_TYPE_CUSTOM;

  do {
    /* Assure parameters correctness */
    if (pfn_notify == NULL && user_data != NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if ((device_type & valid_type) == 0) {
      err = CL_INVALID_DEVICE_TYPE;
      break;
    }

    /* Get the devices num first. */
    err = cl_get_device_ids(NULL, device_type, 0, NULL, &num_devices);
    if (err != CL_SUCCESS)
      break;

    assert(num_devices > 0);
    devices = cl_malloc(num_devices * sizeof(cl_device_id));
    err = cl_get_device_ids(NULL, device_type, num_devices, &devices[0], &num_devices);
    if (err != CL_SUCCESS)
      break;

    context = cl_create_context(properties, num_devices, devices, pfn_notify, user_data, &err);
  } while (0);

  if (devices)
    cl_free(devices);
  if (errcode_ret)
    *errcode_ret = err;
  return context;
}

cl_int
clRetainContext(cl_context context)
{
  if (!CL_OBJECT_IS_CONTEXT(context)) {
    return CL_INVALID_CONTEXT;
  }

  cl_context_add_ref(context);
  return CL_SUCCESS;
}

cl_int
clReleaseContext(cl_context context)
{
  if (!CL_OBJECT_IS_CONTEXT(context)) {
    return CL_INVALID_CONTEXT;
  }

  cl_context_delete(context);
  return CL_SUCCESS;
}

cl_int
clGetContextInfo(cl_context context,
                 cl_context_info param_name,
                 size_t param_value_size,
                 void *param_value,
                 size_t *param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;
  cl_uint n, ref;
  cl_context_properties p;

  if (!CL_OBJECT_IS_CONTEXT(context)) {
    return CL_INVALID_CONTEXT;
  }

  if (param_name == CL_CONTEXT_DEVICES) {
    src_ptr = context->devices;
    src_size = sizeof(cl_device_id) * context->device_num;
  } else if (param_name == CL_CONTEXT_NUM_DEVICES) {
    n = context->device_num;
    src_ptr = &n;
    src_size = sizeof(cl_uint);
  } else if (param_name == CL_CONTEXT_REFERENCE_COUNT) {
    ref = CL_OBJECT_GET_REF(context);
    src_ptr = &ref;
    src_size = sizeof(cl_uint);
  } else if (param_name == CL_CONTEXT_PROPERTIES) {
    if (context->prop_len > 0) {
      src_ptr = context->prop_user;
      src_size = sizeof(cl_context_properties) * context->prop_len;
    } else {
      p = 0;
      src_ptr = &p;
      src_size = sizeof(cl_context_properties);
    }
  } else {
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}
