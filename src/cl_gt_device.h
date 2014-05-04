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
.preferred_vector_width_char = 16,
.preferred_vector_width_short = 16,
.preferred_vector_width_int = 16,
.preferred_vector_width_long = 16,
.preferred_vector_width_float = 16,
.preferred_vector_width_double = 0,
.preferred_vector_width_half = 0,
.native_vector_width_char = 16,
.native_vector_width_short = 16,
.native_vector_width_int = 16,
.native_vector_width_long = 16,
.native_vector_width_float = 16,
.native_vector_width_double = 16,
.native_vector_width_half = 16,
.preferred_wg_sz_mul = 16,
.address_bits = 32,
.max_mem_alloc_size = 256 * 1024 * 1024,
.image_support = CL_TRUE,
.max_read_image_args = 128,
.max_write_image_args = 8,
.image2d_max_width = 8192,
.image2d_max_height = 8192,
.image3d_max_width = 8192,
.image3d_max_height = 8192,
.image3d_max_depth = 2048,
.max_samplers = 16,
.mem_base_addr_align = sizeof(cl_long) * 16 * 8,
.min_data_type_align_size = sizeof(cl_long) * 16,
.single_fp_config = 0, /* XXX */
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
.execution_capabilities = CL_EXEC_KERNEL | CL_EXEC_NATIVE_KERNEL,
.queue_properties = CL_QUEUE_PROFILING_ENABLE,
.platform = NULL, /* == intel_platform (set when requested) */
/* IEEE 754, XXX does IVB support CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT? */
.single_fp_config = CL_FP_INF_NAN | CL_FP_ROUND_TO_NEAREST , /* IEEE 754. */

#define DECL_INFO_STRING(FIELD, STRING) \
    .FIELD = STRING,                    \
    .JOIN(FIELD,_sz) = sizeof(STRING),
DECL_INFO_STRING(name, "Intel HD Graphics Family")
DECL_INFO_STRING(vendor, "Intel")
DECL_INFO_STRING(version, LIBCL_VERSION_STRING)
DECL_INFO_STRING(profile, "FULL_PROFILE")
DECL_INFO_STRING(opencl_c_version, LIBCL_C_VERSION_STRING)
DECL_INFO_STRING(extensions, "")
DECL_INFO_STRING(built_in_kernels, "")
DECL_INFO_STRING(driver_version, LIBCL_DRIVER_VERSION_STRING)
#undef DECL_INFO_STRING


