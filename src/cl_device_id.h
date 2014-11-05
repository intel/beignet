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

#ifndef __CL_DEVICE_ID_H__
#define __CL_DEVICE_ID_H__

/* Store complete information about the device */
struct _cl_device_id {
  DEFINE_ICD(dispatch)
  cl_device_type device_type;
  cl_uint  vendor_id;
  cl_uint  max_compute_unit;               // maximum EU number
  cl_uint  max_thread_per_unit;            // maximum EU threads per EU.
  cl_uint  sub_slice_count;                // Device's sub slice count
  cl_uint  max_work_item_dimensions;       // should be 3.
  size_t   max_work_item_sizes[3];         // equal to maximum work group size.
  size_t   max_work_group_size;            // maximum work group size under simd16 mode.
  size_t   max_1d_global_work_sizes[3];       // maximum 1d global work size for builtin kernels.
  size_t   max_2d_global_work_sizes[3];       // maximum 2d global work size for builtin kernels.
  size_t   max_3d_global_work_sizes[3];       // maximum 3d global work size for builtin kernels.
  cl_uint  preferred_vector_width_char;
  cl_uint  preferred_vector_width_short;
  cl_uint  preferred_vector_width_int;
  cl_uint  preferred_vector_width_long;
  cl_uint  preferred_vector_width_float;
  cl_uint  preferred_vector_width_double;
  cl_uint  preferred_vector_width_half;
  cl_uint  native_vector_width_char;
  cl_uint  native_vector_width_short;
  cl_uint  native_vector_width_int;
  cl_uint  native_vector_width_long;
  cl_uint  native_vector_width_float;
  cl_uint  native_vector_width_double;
  cl_uint  native_vector_width_half;
  cl_uint  max_clock_frequency;
  cl_uint  address_bits;
  cl_ulong max_mem_alloc_size;
  cl_bool  image_support;
  cl_uint  max_read_image_args;
  cl_uint  max_write_image_args;
  size_t   image2d_max_width;
  size_t   image_max_array_size;
  size_t   image2d_max_height;
  size_t   image3d_max_width;
  size_t   image3d_max_height;
  size_t   image3d_max_depth;
  size_t   image_mem_size;
  cl_uint  max_samplers;
  size_t   max_parameter_size;
  cl_uint  mem_base_addr_align;
  cl_uint  min_data_type_align_size;
  cl_device_fp_config single_fp_config;
  cl_device_fp_config double_fp_config;
  cl_device_mem_cache_type global_mem_cache_type;
  cl_uint  global_mem_cache_line_size;
  cl_ulong global_mem_cache_size;
  cl_ulong global_mem_size;
  cl_ulong max_constant_buffer_size;
  cl_uint  max_constant_args;
  cl_device_local_mem_type local_mem_type;
  cl_ulong local_mem_size;
  cl_ulong scratch_mem_size;
  cl_bool  error_correction_support;
  cl_bool  host_unified_memory;
  size_t   profiling_timer_resolution;
  cl_bool  endian_little;
  cl_bool  available;
  cl_bool  compiler_available;
  cl_bool  linker_available;
  cl_device_exec_capabilities execution_capabilities;
  cl_command_queue_properties queue_properties;
  cl_platform_id platform;
  size_t printf_buffer_size;
  cl_bool interop_user_sync;
  const char *name;
  const char *vendor;
  const char *version;
  const char *profile;
  const char *opencl_c_version;
  const char *extensions;
  const char *driver_version;
  const char *built_in_kernels;
  size_t name_sz;
  size_t vendor_sz;
  size_t version_sz;
  size_t profile_sz;
  size_t opencl_c_version_sz;
  size_t extensions_sz;
  size_t driver_version_sz;
  size_t built_in_kernels_sz;
  /* Kernel specific info that we're assigning statically */
  size_t preferred_wg_sz_mul;
  /* SubDevice specific info */
  cl_device_id parent_device;
  cl_uint      partition_max_sub_device;
  cl_device_partition_property partition_property[3];
  cl_device_affinity_domain    affinity_domain;
  cl_device_partition_property partition_type[3];
  cl_uint      device_reference_count;
};

/* Get a device from the given platform */
extern cl_int cl_get_device_ids(cl_platform_id    platform,
                                cl_device_type    device_type,
                                cl_uint           num_entries,
                                cl_device_id *    devices,
                                cl_uint *         num_devices);

/* Get the intel GPU device we currently have in this machine (if any) */
extern cl_device_id cl_get_gt_device(void);

/* Provide info about the device */
extern cl_int cl_get_device_info(cl_device_id     device,
                                 cl_device_info   param_name,
                                 size_t           param_value_size,
                                 void *           param_value,
                                 size_t *         param_value_size_ret);

extern cl_int cl_get_kernel_workgroup_info(cl_kernel kernel,
                                           cl_device_id     device,
                                           cl_kernel_work_group_info   param_name,
                                           size_t           param_value_size,
                                           void *           param_value,
                                           size_t *         param_value_size_ret);
/* Returns the Gen device ID */
extern cl_int cl_device_get_version(cl_device_id device, cl_int *ver);
extern size_t cl_get_kernel_max_wg_sz(cl_kernel);

#endif /* __CL_DEVICE_ID_H__ */

