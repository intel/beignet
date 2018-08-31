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
#undef LIBCL_VERSION_STRING
#undef LIBCL_C_VERSION_STRING
#ifdef GEN9_DEVICE
#define LIBCL_VERSION_STRING GEN9_LIBCL_VERSION_STRING
#define LIBCL_C_VERSION_STRING GEN9_LIBCL_C_VERSION_STRING
#else
#define LIBCL_VERSION_STRING NONGEN9_LIBCL_VERSION_STRING
#define LIBCL_C_VERSION_STRING NONGEN9_LIBCL_C_VERSION_STRING
#endif
/* Common fields for both all GT devices (IVB / SNB) */
.device_type = CL_DEVICE_TYPE_GPU,
.device_id=0,/* == device_id (set when requested) */
.vendor_id = INTEL_VENDOR_ID,
.max_work_item_dimensions = 3,
.max_1d_global_work_sizes = {1024 * 1024 * 256, 1, 1},
.max_2d_global_work_sizes = {8192, 8192, 1},
.max_3d_global_work_sizes = {8192, 8192, 2048},
.preferred_vector_width_char = 16,
.preferred_vector_width_short = 8,
.preferred_vector_width_int = 4,
.preferred_vector_width_long = 2,
.preferred_vector_width_float = 4,
.preferred_vector_width_double = 0,
.preferred_vector_width_half = 0,
.native_vector_width_char = 8,
.native_vector_width_short = 8,
.native_vector_width_int = 4,
.native_vector_width_long = 2,
.native_vector_width_float = 4,
.native_vector_width_double = 2,
.native_vector_width_half = 8,
.address_bits = 32,
.svm_capabilities = CL_DEVICE_SVM_COARSE_GRAIN_BUFFER,
.preferred_platform_atomic_alignment = 0,
.preferred_global_atomic_alignment = 0,
.preferred_local_atomic_alignment = 0,
.image_support = CL_TRUE,
.max_read_image_args = BTI_MAX_READ_IMAGE_ARGS,
.max_write_image_args = BTI_MAX_WRITE_IMAGE_ARGS,
.max_read_write_image_args = BTI_MAX_WRITE_IMAGE_ARGS,
.image_max_array_size = 2048,
.image2d_max_width = 8192,
.image2d_max_height = 8192,
.image3d_max_width = 8192,
.image3d_max_height = 8192,
.image3d_max_depth = 2048,
.image_mem_size = 65536,
.max_samplers = 16,
.mem_base_addr_align = sizeof(cl_long) * 16 * 8,
.min_data_type_align_size = sizeof(cl_long) * 16,
.max_pipe_args = 16,
.pipe_max_active_reservations = 1,
.pipe_max_packet_siz = 1024,
.double_fp_config = 0,
.global_mem_cache_type = CL_READ_WRITE_CACHE,
.max_constant_buffer_size = 128 * 1024 * 1024,
.max_constant_args = 8,
.max_global_variable_size = 64 * 1024,
.global_variable_preferred_total_size = 64 * 1024,
.error_correction_support = CL_FALSE,
#ifdef HAS_USERPTR
.host_unified_memory = CL_TRUE,
#else
.host_unified_memory = CL_FALSE,
#endif
.profiling_timer_resolution = 80, /* ns */
.endian_little = CL_TRUE,
.available = CL_TRUE,
.compiler_available = CL_TRUE,
.linker_available = CL_TRUE,
.execution_capabilities = CL_EXEC_KERNEL | CL_EXEC_NATIVE_KERNEL,
.queue_properties = CL_QUEUE_PROFILING_ENABLE,
.queue_on_host_properties = CL_QUEUE_PROFILING_ENABLE,
.queue_on_device_properties = CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
.queue_on_device_preferred_size = 16 * 1024,
.queue_on_device_max_size = 256 * 1024,
.max_on_device_queues = 1,
.max_on_device_events = 1024,
.platform = NULL, /* == intel_platform (set when requested) */
/* IEEE 754, XXX does IVB support CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT? */
.single_fp_config = CL_FP_INF_NAN | CL_FP_ROUND_TO_NEAREST , /* IEEE 754. */
.half_fp_config = CL_FP_INF_NAN | CL_FP_ROUND_TO_NEAREST ,
.printf_buffer_size = 1 * 1024 * 1024,
.interop_user_sync = CL_TRUE,

#define DECL_INFO_STRING(FIELD, STRING) \
    .FIELD = STRING,                    \
    .JOIN(FIELD,_sz) = sizeof(STRING),
DECL_INFO_STRING(name, "Intel HD Graphics Family")
DECL_INFO_STRING(vendor, "Intel")
DECL_INFO_STRING(version, LIBCL_VERSION_STRING)
DECL_INFO_STRING(profile, "FULL_PROFILE")
DECL_INFO_STRING(opencl_c_version, LIBCL_C_VERSION_STRING)
DECL_INFO_STRING(extensions, "")
DECL_INFO_STRING(built_in_kernels, "__cl_copy_region_align4;"
                                   "__cl_copy_region_align16;"
                                   "__cl_copy_region_unalign_same_offset;"
                                   "__cl_copy_region_unalign_dst_offset;"
                                   "__cl_copy_region_unalign_src_offset;"
                                   "__cl_copy_buffer_rect;"
                                   "__cl_copy_buffer_rect_align4;"
                                   "__cl_copy_image_1d_to_1d;"
                                   "__cl_copy_image_2d_to_2d;"
                                   "__cl_copy_image_3d_to_2d;"
                                   "__cl_copy_image_2d_to_3d;"
                                   "__cl_copy_image_3d_to_3d;"
                                   "__cl_copy_image_2d_to_buffer;"
                                   "__cl_copy_image_2d_to_buffer_align4;"
                                   "__cl_copy_image_2d_to_buffer_align16;"
                                   "__cl_copy_image_3d_to_buffer;"
                                   "__cl_copy_image_3d_to_buffer_align4;"
                                   "__cl_copy_image_3d_to_buffer_align16;"
                                   "__cl_copy_buffer_to_image_2d;"
                                   "__cl_copy_buffer_to_image_2d_align4;"
                                   "__cl_copy_buffer_to_image_2d_align16;"
                                   "__cl_copy_buffer_to_image_3d;"
                                   "__cl_copy_buffer_to_image_3d_align4;"
                                   "__cl_copy_buffer_to_image_3d_align16;"
                                   "__cl_copy_image_1d_array_to_1d_array;"
                                   "__cl_copy_image_2d_array_to_2d_array;"
                                   "__cl_copy_image_2d_array_to_2d;"
                                   "__cl_copy_image_2d_array_to_3d;"
                                   "__cl_copy_image_2d_to_2d_array;"
                                   "__cl_copy_image_3d_to_2d_array;"
                                   "__cl_fill_region_unalign;"
                                   "__cl_fill_region_align2;"
                                   "__cl_fill_region_align4;"
                                   "__cl_fill_region_align8_2;"
                                   "__cl_fill_region_align8_4;"
                                   "__cl_fill_region_align8_8;"
                                   "__cl_fill_region_align8_16;"
                                   "__cl_fill_region_align128;"
                                   "__cl_fill_image_1d;"
                                   "__cl_fill_image_1d_array;"
                                   "__cl_fill_image_2d;"
                                   "__cl_fill_image_2d_array;"
                                   "__cl_fill_image_3d;"
#ifdef GEN7_DEVICE
                                   "block_motion_estimate_intel;"
#endif
                                   )

DECL_INFO_STRING(driver_version, LIBCL_DRIVER_VERSION_STRING)
DECL_INFO_STRING(spir_versions, "1.2")
#undef DECL_INFO_STRING
.parent_device = NULL,
.partition_max_sub_device = 1,
.partition_property = {0},
.affinity_domain = 0,
.partition_type = {0},
.image_pitch_alignment = 1,
.image_base_address_alignment = 4096,
.sub_group_sizes = {8, 16},
.sub_group_sizes_sz = sizeof(size_t) * 2,
.cmrt_device = NULL
