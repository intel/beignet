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

/* Common fields for both GT1 and GT2 devices. Fields which are not shared are
 * set in cl_device_id_object.c which basically deals with OpenCL devices
 */
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
.address_bits = 32,
.max_mem_alloc_size = 128 * 1024 * 1024,
.image_support = CL_FALSE,
.max_read_image_args = 0,
.max_write_image_args = 0,
.image2d_max_width = 0,
.image2d_max_height = 0,
.image3d_max_width = 0,
.image3d_max_height = 0,
.image3d_max_depth = 0,
.max_samplers = 0,
.max_parameter_size = 256, /* Gen6 */
.mem_base_addr_align = sizeof(cl_uint) * 8,
.min_data_type_align_size = sizeof(cl_uint),
.single_fp_config = 0, /* XXX */
.global_mem_cache_type = CL_READ_WRITE_CACHE,
.global_mem_cache_line_size = 128, /* XXX */
.global_mem_cache_size = 8 << 10, /* XXX */
.global_mem_size = 4,
.max_constant_buffer_size = 64 << 10,
.max_constant_args = 8,
.local_mem_type = CL_GLOBAL, /* Gen6 */
.local_mem_size = 16 << 10,  /* Gen6 */
.error_correction_support = CL_FALSE,
.host_unified_memory = CL_FALSE,
.profiling_timer_resolution = 80, /* ns */
.endian_little = CL_TRUE,
.available = CL_TRUE,
.compiler_available = CL_FALSE, /* XXX */
.execution_capabilities = CL_EXEC_KERNEL,
.queue_properties = CL_QUEUE_PROFILING_ENABLE,
.platform = NULL, /* == intel_platform (set when requested) */
.gfx_id = IGFX_GEN6_CORE,

#define DECL_INFO_STRING(FIELD, STRING) \
    .FIELD = STRING,                    \
    .JOIN(FIELD,_sz) = sizeof(STRING) + 1,
DECL_INFO_STRING(name, "Intel HD Graphics Family")
DECL_INFO_STRING(vendor, "Intel")
DECL_INFO_STRING(version, "OpenCL 1.10")
DECL_INFO_STRING(profile, "FULL_PROFILE")
DECL_INFO_STRING(opencl_c_version, "OpenCL 1.10")
DECL_INFO_STRING(extensions, "")
#undef DECL_INFO_STRING

