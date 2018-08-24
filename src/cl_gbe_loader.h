/*
 * Copyright © 2014 Intel Corporation
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

#ifndef __CL_GBE_LOADER_H__
#define __CL_GBE_LOADER_H__

#include "program.h"

#ifdef __cplusplus
extern "C" {
#endif
extern gbe_program_new_from_source_cb *compiler_program_new_from_source;
extern gbe_program_new_from_llvm_file_cb *compiler_program_new_from_llvm_file;
extern gbe_program_compile_from_source_cb *compiler_program_compile_from_source;
extern gbe_program_new_gen_program_cb *compiler_program_new_gen_program;
extern gbe_program_link_program_cb *compiler_program_link_program;
extern gbe_program_check_opt_cb *compiler_program_check_opt;
extern gbe_program_build_from_llvm_cb *compiler_program_build_from_llvm;
extern gbe_program_new_from_llvm_binary_cb *compiler_program_new_from_llvm_binary;
extern gbe_program_serialize_to_binary_cb *compiler_program_serialize_to_binary;
extern gbe_program_new_from_llvm_cb *compiler_program_new_from_llvm;
extern gbe_program_clean_llvm_resource_cb *compiler_program_clean_llvm_resource;

extern gbe_program_new_from_binary_cb *interp_program_new_from_binary;
extern gbe_program_get_global_constant_size_cb *interp_program_get_global_constant_size;
extern gbe_program_get_global_constant_data_cb *interp_program_get_global_constant_data;
extern gbe_program_get_global_reloc_count_cb *interp_program_get_global_reloc_count;
extern gbe_program_get_global_reloc_table_cb *interp_program_get_global_reloc_table;
extern gbe_program_delete_cb *interp_program_delete;
extern gbe_program_get_kernel_num_cb *interp_program_get_kernel_num;
extern gbe_program_get_kernel_by_name_cb *interp_program_get_kernel_by_name;
extern gbe_program_get_kernel_cb *interp_program_get_kernel;
extern gbe_program_get_device_enqueue_kernel_name_cb *interp_program_get_device_enqueue_kernel_name;
extern gbe_kernel_get_name_cb *interp_kernel_get_name;
extern gbe_kernel_get_attributes_cb *interp_kernel_get_attributes;
extern gbe_kernel_get_code_cb *interp_kernel_get_code;
extern gbe_kernel_get_code_size_cb *interp_kernel_get_code_size;
extern gbe_kernel_get_arg_num_cb *interp_kernel_get_arg_num;
extern gbe_kernel_get_arg_size_cb *interp_kernel_get_arg_size;
extern gbe_kernel_get_arg_bti_cb *interp_kernel_get_arg_bti;
extern gbe_kernel_get_arg_type_cb *interp_kernel_get_arg_type;
extern gbe_kernel_get_arg_align_cb *interp_kernel_get_arg_align;
extern gbe_kernel_get_simd_width_cb *interp_kernel_get_simd_width;
extern gbe_kernel_get_curbe_offset_cb *interp_kernel_get_curbe_offset;
extern gbe_kernel_get_curbe_size_cb *interp_kernel_get_curbe_size;
extern gbe_kernel_get_stack_size_cb *interp_kernel_get_stack_size;
extern gbe_kernel_get_scratch_size_cb *interp_kernel_get_scratch_size;
extern gbe_kernel_get_required_work_group_size_cb *interp_kernel_get_required_work_group_size;
extern gbe_kernel_use_slm_cb *interp_kernel_use_slm;
extern gbe_kernel_get_slm_size_cb *interp_kernel_get_slm_size;
extern gbe_kernel_get_sampler_size_cb *interp_kernel_get_sampler_size;
extern gbe_kernel_get_sampler_data_cb *interp_kernel_get_sampler_data;
extern gbe_kernel_get_compile_wg_size_cb *interp_kernel_get_compile_wg_size;
extern gbe_kernel_get_image_size_cb *interp_kernel_get_image_size;
extern gbe_kernel_get_image_data_cb *interp_kernel_get_image_data;
extern gbe_kernel_get_ocl_version_cb *interp_kernel_get_ocl_version;
extern gbe_output_profiling_cb* interp_output_profiling;
extern gbe_get_profiling_bti_cb* interp_get_profiling_bti;
extern gbe_dup_profiling_cb* interp_dup_profiling;
extern gbe_get_printf_num_cb* interp_get_printf_num;
extern gbe_get_printf_buf_bti_cb* interp_get_printf_buf_bti;
extern gbe_dup_printfset_cb* interp_dup_printfset;
extern gbe_release_printf_info_cb* interp_release_printf_info;
extern gbe_output_printf_cb* interp_output_printf;
extern gbe_kernel_get_arg_info_cb *interp_kernel_get_arg_info;
extern gbe_kernel_use_device_enqueue_cb * interp_kernel_use_device_enqueue;

int CompilerSupported();
#ifdef __cplusplus
}
#endif

#endif /* __CL_GBE_LOADER_H__ */
