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

#include "cl_platform_id.h"
#include "cl_device_id.h"
#include "cl_utils.h"
#include "CL/cl_ext.h"
#include <string.h>

#ifndef CL_VERSION_1_2
#define CL_DEVICE_BUILT_IN_KERNELS 0x103F
#endif

LOCAL cl_int
cl_device_get_ids(cl_platform_id platform, cl_device_type device_type, cl_uint num_entries,
                  cl_device_id *devices, cl_uint *num_devices)
{
  cl_device_id device = NULL;

  if (device_type & (CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT))
    device = cl_device_get_id_gen(platform);

  /* Do we have a usable device? */
  if (device == NULL)
    return CL_DEVICE_NOT_FOUND;

  if (devices)
    devices[0] = device;
  if (num_devices)
    *num_devices = 1;
  return CL_SUCCESS;
}

LOCAL cl_int
cl_device_get_info(cl_device_id device, cl_device_info param_name, size_t param_value_size,
                   void *param_value, size_t *param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;
  cl_int dev_ref;

  /* Find the correct parameter */
  switch (param_name) {
  case CL_DEVICE_TYPE:
    src_ptr = &device->device_type;
    src_size = sizeof(device->device_type);
    break;
  case CL_DEVICE_VENDOR_ID:
    src_ptr = &device->vendor_id;
    src_size = sizeof(device->vendor_id);
    break;
  case CL_DEVICE_MAX_COMPUTE_UNITS:
    src_ptr = &device->max_compute_unit;
    src_size = sizeof(device->max_compute_unit);
    break;
  case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
    src_ptr = &device->max_work_item_dimensions;
    src_size = sizeof(device->max_work_item_dimensions);
    break;
  case CL_DEVICE_MAX_WORK_ITEM_SIZES:
    src_ptr = &device->max_work_item_sizes;
    src_size = sizeof(device->max_work_item_sizes);
    break;
  case CL_DEVICE_MAX_WORK_GROUP_SIZE:
    src_ptr = &device->max_work_group_size;
    src_size = sizeof(device->max_work_group_size);
    break;
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
    src_ptr = &device->preferred_vector_width_char;
    src_size = sizeof(device->preferred_vector_width_char);
    break;
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
    src_ptr = &device->preferred_vector_width_short;
    src_size = sizeof(device->preferred_vector_width_short);
    break;
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
    src_ptr = &device->preferred_vector_width_int;
    src_size = sizeof(device->preferred_vector_width_int);
    break;
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
    src_ptr = &device->preferred_vector_width_long;
    src_size = sizeof(device->preferred_vector_width_long);
    break;
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
    src_ptr = &device->preferred_vector_width_float;
    src_size = sizeof(device->preferred_vector_width_float);
    break;
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
    src_ptr = &device->preferred_vector_width_double;
    src_size = sizeof(device->preferred_vector_width_double);
    break;
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
    src_ptr = &device->preferred_vector_width_half;
    src_size = sizeof(device->preferred_vector_width_half);
    break;
  case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
    src_ptr = &device->native_vector_width_char;
    src_size = sizeof(device->native_vector_width_char);
    break;
  case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
    src_ptr = &device->native_vector_width_short;
    src_size = sizeof(device->native_vector_width_short);
    break;
  case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
    src_ptr = &device->native_vector_width_int;
    src_size = sizeof(device->native_vector_width_int);
    break;
  case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
    src_ptr = &device->native_vector_width_long;
    src_size = sizeof(device->native_vector_width_long);
    break;
  case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
    src_ptr = &device->native_vector_width_float;
    src_size = sizeof(device->native_vector_width_float);
    break;
  case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
    src_ptr = &device->native_vector_width_double;
    src_size = sizeof(device->native_vector_width_double);
    break;
  case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
    src_ptr = &device->native_vector_width_half;
    src_size = sizeof(device->native_vector_width_half);
    break;
  case CL_DEVICE_MAX_CLOCK_FREQUENCY:
    src_ptr = &device->max_clock_frequency;
    src_size = sizeof(device->max_clock_frequency);
    break;
  case CL_DEVICE_ADDRESS_BITS:
    src_ptr = &device->address_bits;
    src_size = sizeof(device->address_bits);
    break;
  case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
    src_ptr = &device->max_mem_alloc_size;
    src_size = sizeof(device->max_mem_alloc_size);
    break;
  case CL_DEVICE_IMAGE_SUPPORT:
    src_ptr = &device->image_support;
    src_size = sizeof(device->image_support);
    break;
  case CL_DEVICE_MAX_READ_IMAGE_ARGS:
    src_ptr = &device->max_read_image_args;
    src_size = sizeof(device->max_read_image_args);
    break;
  case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
    src_ptr = &device->max_write_image_args;
    src_size = sizeof(device->max_write_image_args);
    break;
  case CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS:
    src_ptr = &device->max_read_write_image_args;
    src_size = sizeof(device->max_read_write_image_args);
    break;
  case CL_DEVICE_IMAGE_MAX_ARRAY_SIZE:
    src_ptr = &device->image_max_array_size;
    src_size = sizeof(device->image_max_array_size);
    break;
  case CL_DEVICE_IMAGE2D_MAX_WIDTH:
  case CL_DEVICE_PLANAR_YUV_MAX_WIDTH_INTEL:
    src_ptr = &device->image2d_max_width;
    src_size = sizeof(device->image2d_max_width);
    break;
  case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
  case CL_DEVICE_PLANAR_YUV_MAX_HEIGHT_INTEL:
    src_ptr = &device->image2d_max_height;
    src_size = sizeof(device->image2d_max_height);
    break;
  case CL_DEVICE_IMAGE3D_MAX_WIDTH:
    src_ptr = &device->image3d_max_width;
    src_size = sizeof(device->image3d_max_width);
    break;
  case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
    src_ptr = &device->image3d_max_height;
    src_size = sizeof(device->image3d_max_height);
    break;
  case CL_DEVICE_IMAGE3D_MAX_DEPTH:
    src_ptr = &device->image3d_max_depth;
    src_size = sizeof(device->image3d_max_depth);
    break;
  case CL_DEVICE_MAX_SAMPLERS:
    src_ptr = &device->max_samplers;
    src_size = sizeof(device->max_samplers);
    break;
  case CL_DEVICE_MAX_PARAMETER_SIZE:
    src_ptr = &device->max_parameter_size;
    src_size = sizeof(device->max_parameter_size);
    break;
  case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
    src_ptr = &device->mem_base_addr_align;
    src_size = sizeof(device->mem_base_addr_align);
    break;
  case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
    src_ptr = &device->min_data_type_align_size;
    src_size = sizeof(device->min_data_type_align_size);
    break;
  case CL_DEVICE_MAX_PIPE_ARGS:
    src_ptr = &device->max_pipe_args;
    src_size = sizeof(device->max_pipe_args);
    break;
  case CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS:
    src_ptr = &device->pipe_max_active_reservations;
    src_size = sizeof(device->pipe_max_active_reservations);
    break;
  case CL_DEVICE_PIPE_MAX_PACKET_SIZE:
    src_ptr = &device->pipe_max_packet_siz;
    src_size = sizeof(device->pipe_max_packet_siz);
    break;
  case CL_DEVICE_SINGLE_FP_CONFIG:
    src_ptr = &device->single_fp_config;
    src_size = sizeof(device->single_fp_config);
    break;
  case CL_DEVICE_HALF_FP_CONFIG:
    src_ptr = &device->half_fp_config;
    src_size = sizeof(device->half_fp_config);
    break;
  case CL_DEVICE_DOUBLE_FP_CONFIG:
    src_ptr = &device->double_fp_config;
    src_size = sizeof(device->double_fp_config);
    break;
  case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
    src_ptr = &device->global_mem_cache_type;
    src_size = sizeof(device->global_mem_cache_type);
    break;
  case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
    src_ptr = &device->global_mem_cache_line_size;
    src_size = sizeof(device->global_mem_cache_line_size);
    break;
  case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
    src_ptr = &device->global_mem_cache_size;
    src_size = sizeof(device->global_mem_cache_size);
    break;
  case CL_DEVICE_GLOBAL_MEM_SIZE:
    src_ptr = &device->global_mem_size;
    src_size = sizeof(device->global_mem_size);
    break;
  case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
    src_ptr = &device->max_constant_buffer_size;
    src_size = sizeof(device->max_constant_buffer_size);
    break;
  case CL_DEVICE_IMAGE_MAX_BUFFER_SIZE:
    src_ptr = &device->image_mem_size;
    src_size = sizeof(device->image_mem_size);
    break;
  case CL_DEVICE_MAX_CONSTANT_ARGS:
    src_ptr = &device->max_constant_args;
    src_size = sizeof(device->max_constant_args);
    break;
  case CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE:
    src_ptr = &device->max_global_variable_size;
    src_size = sizeof(device->max_global_variable_size);
    break;
  case CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE:
    src_ptr = &device->global_variable_preferred_total_size;
    src_size = sizeof(device->global_variable_preferred_total_size);
    break;
  case CL_DEVICE_LOCAL_MEM_TYPE:
    src_ptr = &device->local_mem_type;
    src_size = sizeof(device->local_mem_type);
    break;
  case CL_DEVICE_LOCAL_MEM_SIZE:
    src_ptr = &device->local_mem_size;
    src_size = sizeof(device->local_mem_size);
    break;
  case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
    src_ptr = &device->error_correction_support;
    src_size = sizeof(device->error_correction_support);
    break;
  case CL_DEVICE_HOST_UNIFIED_MEMORY:
    src_ptr = &device->host_unified_memory;
    src_size = sizeof(device->host_unified_memory);
    break;
  case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
    src_ptr = &device->profiling_timer_resolution;
    src_size = sizeof(device->profiling_timer_resolution);
    break;
  case CL_DEVICE_ENDIAN_LITTLE:
    src_ptr = &device->endian_little;
    src_size = sizeof(device->endian_little);
    break;
  case CL_DEVICE_AVAILABLE:
    src_ptr = &device->available;
    src_size = sizeof(device->available);
    break;
  case CL_DEVICE_COMPILER_AVAILABLE:
    src_ptr = &device->compiler_available;
    src_size = sizeof(device->compiler_available);
    break;
  case CL_DEVICE_LINKER_AVAILABLE:
    src_ptr = &device->linker_available;
    src_size = sizeof(device->linker_available);
    break;
  case CL_DEVICE_EXECUTION_CAPABILITIES:
    src_ptr = &device->execution_capabilities;
    src_size = sizeof(device->execution_capabilities);
    break;
  case CL_DEVICE_QUEUE_PROPERTIES:
    src_ptr = &device->queue_properties;
    src_size = sizeof(device->queue_properties);
    break;
  case CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES:
    src_ptr = &device->queue_on_device_properties;
    src_size = sizeof(device->queue_on_device_properties);
    break;
  case CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE:
    src_ptr = &device->queue_on_device_preferred_size;
    src_size = sizeof(device->queue_on_device_preferred_size);
    break;
  case CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE:
    src_ptr = &device->queue_on_device_max_size;
    src_size = sizeof(device->queue_on_device_max_size);
    break;
  case CL_DEVICE_MAX_ON_DEVICE_QUEUES:
    src_ptr = &device->max_on_device_queues;
    src_size = sizeof(device->max_on_device_queues);
    break;
  case CL_DEVICE_MAX_ON_DEVICE_EVENTS:
    src_ptr = &device->max_on_device_events;
    src_size = sizeof(device->max_on_device_events);
    break;
  case CL_DEVICE_PLATFORM:
    src_ptr = &device->platform;
    src_size = sizeof(device->platform);
    break;
  case CL_DEVICE_PRINTF_BUFFER_SIZE:
    src_ptr = &device->printf_buffer_size;
    src_size = sizeof(device->printf_buffer_size);
    break;
  case CL_DEVICE_PREFERRED_INTEROP_USER_SYNC:
    src_ptr = &device->interop_user_sync;
    src_size = sizeof(device->interop_user_sync);
    break;
  case CL_DEVICE_NAME:
    src_ptr = device->name;
    src_size = device->name_sz;
    break;
  case CL_DEVICE_VENDOR:
    src_ptr = device->vendor;
    src_size = device->vendor_sz;
    break;
  case CL_DEVICE_VERSION:
    src_ptr = device->version;
    src_size = device->version_sz;
    break;
  case CL_DEVICE_PROFILE:
    src_ptr = device->profile;
    src_size = device->profile_sz;
    break;
  case CL_DEVICE_OPENCL_C_VERSION:
    src_ptr = device->opencl_c_version;
    src_size = device->opencl_c_version_sz;
    break;
  case CL_DEVICE_SPIR_VERSIONS:
    src_ptr = device->spir_versions;
    src_size = device->spir_versions_sz;
    break;
  case CL_DEVICE_EXTENSIONS:
    src_ptr = device->extensions;
    src_size = device->extensions_sz;
    break;
  case CL_DEVICE_BUILT_IN_KERNELS:
    src_ptr = device->built_in_kernels;
    if (src_ptr)
      src_size = strlen(device->built_in_kernels) + 1;
    else
      src_size = 0;
    break;
  case CL_DEVICE_PARENT_DEVICE:
    src_ptr = &device->parent_device;
    src_size = sizeof(device->parent_device);
    break;
  case CL_DEVICE_PARTITION_MAX_SUB_DEVICES:
    src_ptr = &device->partition_max_sub_device;
    src_size = sizeof(device->partition_max_sub_device);
    break;
  case CL_DEVICE_PARTITION_PROPERTIES:
    src_ptr = &device->partition_property;
    src_size = sizeof(device->partition_property);
    break;
  case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
    src_ptr = &device->affinity_domain;
    src_size = sizeof(device->affinity_domain);
    break;
  case CL_DEVICE_PARTITION_TYPE:
    src_ptr = &device->partition_type;
    src_size = sizeof(device->partition_type);
    break;
  case CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT:
    src_ptr = &device->preferred_platform_atomic_alignment;
    src_size = sizeof(device->preferred_platform_atomic_alignment);
    break;
  case CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT:
    src_ptr = &device->preferred_global_atomic_alignment;
    src_size = sizeof(device->preferred_global_atomic_alignment);
    break;
  case CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT:
    src_ptr = &device->preferred_local_atomic_alignment;
    src_size = sizeof(device->preferred_local_atomic_alignment);
    break;
  case CL_DEVICE_IMAGE_PITCH_ALIGNMENT:
    src_ptr = &device->image_pitch_alignment;
    src_size = sizeof(device->image_pitch_alignment);
    break;
  case CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT:
    src_ptr = &device->image_base_address_alignment;
    src_size = sizeof(device->image_base_address_alignment);
    break;
  case CL_DEVICE_SVM_CAPABILITIES:
    src_ptr = &device->svm_capabilities;
    src_size = sizeof(device->svm_capabilities);
    break;
  case CL_DEVICE_REFERENCE_COUNT: {
    dev_ref = CL_OBJECT_GET_REF(device);
    src_ptr = &dev_ref;
    src_size = sizeof(cl_int);
    break;
  }
  case CL_DRIVER_VERSION:
    src_ptr = device->driver_version;
    src_size = device->driver_version_sz;
    break;

  default:
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

LOCAL cl_int
cl_devices_list_check(cl_uint num_devices, const cl_device_id *devices)
{
  cl_uint i;

  if (devices == NULL)
    return CL_INVALID_DEVICE;

  assert(num_devices > 0);
  for (i = 0; i < num_devices; i++) {
    if (!CL_OBJECT_IS_DEVICE(devices[i])) {
      return CL_INVALID_DEVICE;
    }

    if (devices[i]->available == CL_FALSE) {
      return CL_DEVICE_NOT_AVAILABLE;
    }

    // We now just support one platform.
    if (devices[i]->platform != cl_get_platform_default()) {
      return CL_INVALID_DEVICE;
    }

    // TODO: We now just support Gen Device.
    if (devices[i] != cl_device_get_id_gen(devices[i]->platform)) {
      return CL_INVALID_DEVICE;
    }
  }

  return CL_SUCCESS;
}

LOCAL cl_int
cl_devices_list_include_check(cl_uint num_devices, const cl_device_id *devices,
                              cl_uint num_to_check, const cl_device_id *devices_to_check)
{
  cl_uint i, j;

  for (i = 0; i < num_to_check; i++) {
    for (j = 0; j < num_devices; j++) {
      if (devices_to_check[i] == devices[j])
        break;
    }

    if (j == num_devices)
      return CL_INVALID_DEVICE;
  }

  return CL_SUCCESS;
}
