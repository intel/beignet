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

/* Common fields for both all GT devices (IVB / SNB) */
.device_type = CL_DEVICE_TYPE_GPU,
.vendor_id = 0, /* == device_id (set when requested) */
.max_work_item_dimensions = 3,
.max_1d_global_work_sizes = {1024 * 1024 * 256, 1, 1},
.max_2d_global_work_sizes = {8192, 8192, 1},
.max_3d_global_work_sizes = {8192, 8192, 2048},
.preferred_vector_width_char = 8,
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
.preferred_wg_sz_mul = 16,
.address_bits = 32,
.max_mem_alloc_size = 256 * 1024 * 1024,
.image_support = CL_TRUE,
.max_read_image_args = 128,
.max_write_image_args = 8,
.image_max_array_size = 2048,
.image2d_max_width = 8192,
.image2d_max_height = 8192,
.image3d_max_width = 8192,
.image3d_max_height = 8192,
.image3d_max_depth = 2048,
.image_mem_size = 8192,
.max_samplers = 16,
.mem_base_addr_align = sizeof(cl_long) * 16 * 8,
.min_data_type_align_size = sizeof(cl_long) * 16,
.single_fp_config = 0, /* XXX */
.double_fp_config = 0,
.global_mem_cache_type = CL_READ_WRITE_CACHE,
.global_mem_size = 1024 * 1024 * 1024,
.max_constant_buffer_size = 512 << 10,
.max_constant_args = 8,
.error_correction_support = CL_FALSE,
.host_unified_memory = CL_FALSE,
.profiling_timer_resolution = 80, /* ns */
.endian_little = CL_TRUE,
.available = CL_TRUE,
.compiler_available = CL_TRUE,
.linker_available = CL_TRUE,
.execution_capabilities = CL_EXEC_KERNEL | CL_EXEC_NATIVE_KERNEL,
.queue_properties = CL_QUEUE_PROFILING_ENABLE,
.platform = NULL, /* == intel_platform (set when requested) */
/* IEEE 754, XXX does IVB support CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT? */
.single_fp_config = CL_FP_INF_NAN | CL_FP_ROUND_TO_NEAREST , /* IEEE 754. */
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
                                   "__cl_cpy_region_unalign_same_offset;"
                                   "__cl_copy_region_unalign_dst_offset;"
                                   "__cl_copy_region_unalign_src_offset;"
                                   "__cl_copy_buffer_rect;"
                                   "__cl_copy_image_1d_to_1d;"
                                   "__cl_copy_image_2d_to_2d;"
                                   "__cl_copy_image_3d_to_2d;"
                                   "__cl_copy_image_2d_to_3d;"
                                   "__cl_copy_image_3d_to_3d;"
                                   "__cl_copy_image_2d_to_buffer;"
                                   "__cl_copy_image_3d_to_buffer;"
                                   "__cl_copy_buffer_to_image_2d;"
                                   "__cl_copy_buffer_to_image_3d;"
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
                                   "__cl_fill_image_3d;")

DECL_INFO_STRING(driver_version, LIBCL_DRIVER_VERSION_STRING)
#undef DECL_INFO_STRING
.parent_device = NULL,
.partition_max_sub_device = 1,
.partition_property = {0},
.affinity_domain = 0,
.partition_type = {0},
.device_reference_count = 1,

