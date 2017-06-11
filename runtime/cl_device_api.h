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
 * Author: He Junyan <junyan.he@intel.com>
 */

#ifndef __CL_DEVICE_API_H__
#define __CL_DEVICE_API_H__

#include "CL/cl.h"

#define DEV_PRIVATE_DATA(PARENT, DEV, PRIV)                             \
  do {                                                                  \
    PRIV = NULL;                                                        \
    assert(PARENT->each_device_num > 0);                                \
    cl_uint eedev;                                                      \
    for (eedev = 0; eedev < PARENT->each_device_num; eedev++) {         \
      if (DEV == (PARENT->each_device[eedev])->device) {                \
        PRIV = (void *)PARENT->each_device[eedev];                      \
        break;                                                          \
      }                                                                 \
    }                                                                   \
    assert(PRIV != NULL);                                               \
  } while (0);

#define ASSIGN_DEV_PRIVATE_DATA(PARENT, DEV, PRIV)                      \
  do {                                                                  \
    assert(PARENT->each_device_num > 0);                                \
    cl_uint eedev;                                                      \
    for (eedev = 0; eedev < PARENT->each_device_num; eedev++) { \
      if (PARENT->each_device[eedev])                                   \
        assert(DEV != (PARENT->each_device[eedev])->device);            \
      else                                                              \
        PARENT->each_device[eedev] = PRIV;                              \
    }                                                                   \
  } while (0);

typedef struct _cl_device_api {
  cl_int (*compiler_unload)(cl_device_id device);

  cl_int (*context_create)(cl_device_id device, cl_context ctx);
  void (*context_delete)(cl_device_id device, cl_context ctx);

  cl_int (*event_create)(cl_device_id device, cl_event event);
  void (*event_delete)(cl_device_id device, cl_event event);
  void (*event_profiling)(cl_event event, cl_int status);

  cl_int (*command_queue_create)(cl_device_id device, cl_command_queue queue);
  void (*command_queue_delete)(cl_device_id device, cl_command_queue queue);

  cl_int (*sampler_create)(cl_device_id device, cl_sampler sampler);
  void (*sampler_delete)(cl_device_id device, cl_sampler sampler);

  cl_int (*program_create)(cl_device_id device, cl_program p);
  cl_int (*program_load_binary)(cl_device_id device, cl_program prog);
  void (*program_delete)(cl_device_id device, cl_program p);
  cl_int (*program_get_info)(cl_device_id device, cl_program program, cl_uint param_name, void *param_value);

  void (*kernel_delete)(cl_device_id device, cl_kernel kernel);
  cl_int (*kernel_create)(cl_device_id device, cl_kernel kernel);
  cl_int (*kernel_get_info)(cl_device_id device, cl_kernel kernel, cl_uint param_name, void *param_value);
  cl_int (*nd_range_kernel)(cl_event event, cl_int status);
  cl_int (*native_kernel)(cl_event event, cl_int status);

  cl_int (*svm_create)(cl_device_id device, cl_mem mem);
  void (*svm_delete)(cl_device_id device, cl_mem mem);
  cl_int (*svm_map)(cl_event event, cl_int status);
  cl_int (*svm_unmap)(cl_event event, cl_int status);
  cl_int (*svm_copy)(cl_event event, cl_int status);
  cl_int (*svm_fill)(cl_event event, cl_int status);

  cl_int (*image_format_support)(cl_device_id device, cl_mem_object_type image_type, cl_image_format *image_format);
  cl_int (*mem_allocate)(cl_device_id device, cl_mem mem);
  void (*mem_deallocate)(cl_device_id device, cl_mem mem);

  cl_int (*mem_map)(cl_event event, cl_int status);
  cl_int (*mem_unmap)(cl_event event, cl_int status);

  cl_int (*buffer_read)(cl_event event, cl_int status);
  cl_int (*buffer_write)(cl_event event, cl_int status);
  cl_int (*buffer_read_rect)(cl_event event, cl_int status);
  cl_int (*buffer_write_rect)(cl_event event, cl_int status);

  cl_int (*image_read)(cl_event event, cl_int status);
  cl_int (*image_write)(cl_event event, cl_int status);

  cl_int (*buffer_copy)(cl_event event, cl_int status);
  cl_int (*buffer_fill)(cl_event event, cl_int status);
  cl_int (*buffer_copy_rect)(cl_event event, cl_int status);

  cl_int (*image_fill)(cl_event event, cl_int status);
  cl_int (*image_copy)(cl_event event, cl_int status);

  cl_int (*copy_image_to_buffer)(cl_event event, cl_int status);
  cl_int (*copy_buffer_to_image)(cl_event event, cl_int status);
} _cl_device_api;
typedef _cl_device_api *cl_device_api;

#endif /* End of __CL_DEVICE_API_H__ */
