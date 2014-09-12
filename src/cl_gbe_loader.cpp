/*
 * Copyright © 2014 Intel Corporation
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
 */
#include <iostream>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include "cl_gbe_loader.h"
#include "backend/src/GBEConfig.h"

//function pointer from libgbe.so
gbe_program_new_from_source_cb *compiler_program_new_from_source = NULL;
gbe_program_compile_from_source_cb *compiler_program_compile_from_source = NULL;
gbe_program_new_gen_program_cb *compiler_program_new_gen_program = NULL;
gbe_program_link_program_cb *compiler_program_link_program = NULL;
gbe_program_build_from_llvm_cb *compiler_program_build_from_llvm = NULL;
gbe_program_new_from_llvm_binary_cb *compiler_program_new_from_llvm_binary = NULL;
gbe_program_serialize_to_binary_cb *compiler_program_serialize_to_binary = NULL;
gbe_program_new_from_llvm_cb *compiler_program_new_from_llvm = NULL;
gbe_program_clean_llvm_resource_cb *compiler_program_clean_llvm_resource = NULL;

//function pointer from libgbeinterp.so
gbe_program_new_from_binary_cb *interp_program_new_from_binary = NULL;
gbe_program_get_global_constant_size_cb *interp_program_get_global_constant_size = NULL;
gbe_program_get_global_constant_data_cb *interp_program_get_global_constant_data = NULL;
gbe_program_delete_cb *interp_program_delete = NULL;
gbe_program_get_kernel_num_cb *interp_program_get_kernel_num = NULL;
gbe_program_get_kernel_by_name_cb *interp_program_get_kernel_by_name = NULL;
gbe_program_get_kernel_cb *interp_program_get_kernel = NULL;
gbe_kernel_get_name_cb *interp_kernel_get_name = NULL;
gbe_kernel_get_attributes_cb *interp_kernel_get_attributes = NULL;
gbe_kernel_get_code_cb *interp_kernel_get_code = NULL;
gbe_kernel_get_code_size_cb *interp_kernel_get_code_size = NULL;
gbe_kernel_get_arg_num_cb *interp_kernel_get_arg_num = NULL;
gbe_kernel_get_arg_size_cb *interp_kernel_get_arg_size = NULL;
gbe_kernel_get_arg_bti_cb *interp_kernel_get_arg_bti = NULL;
gbe_kernel_get_arg_type_cb *interp_kernel_get_arg_type = NULL;
gbe_kernel_get_arg_align_cb *interp_kernel_get_arg_align = NULL;
gbe_kernel_get_simd_width_cb *interp_kernel_get_simd_width = NULL;
gbe_kernel_get_curbe_offset_cb *interp_kernel_get_curbe_offset = NULL;
gbe_kernel_get_curbe_size_cb *interp_kernel_get_curbe_size = NULL;
gbe_kernel_get_stack_size_cb *interp_kernel_get_stack_size = NULL;
gbe_kernel_get_scratch_size_cb *interp_kernel_get_scratch_size = NULL;
gbe_kernel_get_required_work_group_size_cb *interp_kernel_get_required_work_group_size = NULL;
gbe_kernel_use_slm_cb *interp_kernel_use_slm = NULL;
gbe_kernel_get_slm_size_cb *interp_kernel_get_slm_size = NULL;
gbe_kernel_get_sampler_size_cb *interp_kernel_get_sampler_size = NULL;
gbe_kernel_get_sampler_data_cb *interp_kernel_get_sampler_data = NULL;
gbe_kernel_get_compile_wg_size_cb *interp_kernel_get_compile_wg_size = NULL;
gbe_kernel_get_image_size_cb *interp_kernel_get_image_size = NULL;
gbe_kernel_get_image_data_cb *interp_kernel_get_image_data = NULL;
gbe_get_printf_num_cb* interp_get_printf_num = NULL;
gbe_get_printf_buf_bti_cb* interp_get_printf_buf_bti = NULL;
gbe_get_printf_indexbuf_bti_cb* interp_get_printf_indexbuf_bti = NULL;
gbe_dup_printfset_cb* interp_dup_printfset = NULL;
gbe_get_printf_sizeof_size_cb* interp_get_printf_sizeof_size = NULL;
gbe_release_printf_info_cb* interp_release_printf_info = NULL;
gbe_output_printf_cb* interp_output_printf = NULL;
gbe_kernel_get_arg_info_cb *interp_kernel_get_arg_info = NULL;

struct GbeLoaderInitializer
{
  GbeLoaderInitializer()
  {
    LoadCompiler();

    const char* path;
    if (!LoadInterp(path))
      std::cerr << "unable to load " << path << " which is part of the driver, please check!" << std::endl;
  }

  bool LoadInterp(const char*& path)
  {
    const char* interpPath = getenv("OCL_INTERP_PATH");
    if (interpPath == NULL)
      interpPath = INTERP_OBJECT_DIR;

    path = interpPath;

    dlhInterp = dlopen(interpPath, RTLD_LAZY | RTLD_LOCAL);
    if (dlhInterp == NULL) {
      return false;
    }

    interp_program_new_from_binary = *(gbe_program_new_from_binary_cb**)dlsym(dlhInterp, "gbe_program_new_from_binary");
    if (interp_program_new_from_binary == NULL)
      return false;

    interp_program_get_global_constant_size = *(gbe_program_get_global_constant_size_cb**)dlsym(dlhInterp, "gbe_program_get_global_constant_size");
    if (interp_program_get_global_constant_size == NULL)
      return false;

    interp_program_get_global_constant_data = *(gbe_program_get_global_constant_data_cb**)dlsym(dlhInterp, "gbe_program_get_global_constant_data");
    if (interp_program_get_global_constant_data == NULL)
      return false;

    interp_program_delete = *(gbe_program_delete_cb**)dlsym(dlhInterp, "gbe_program_delete");
    if (interp_program_delete == NULL)
      return false;

    interp_program_get_kernel_num = *(gbe_program_get_kernel_num_cb**)dlsym(dlhInterp, "gbe_program_get_kernel_num");
    if (interp_program_get_kernel_num == NULL)
      return false;

    interp_program_get_kernel_by_name = *(gbe_program_get_kernel_by_name_cb**)dlsym(dlhInterp, "gbe_program_get_kernel_by_name");
    if (interp_program_get_kernel_by_name == NULL)
      return false;

    interp_program_get_kernel = *(gbe_program_get_kernel_cb**)dlsym(dlhInterp, "gbe_program_get_kernel");
    if (interp_program_get_kernel == NULL)
      return false;

    interp_kernel_get_name = *(gbe_kernel_get_name_cb**)dlsym(dlhInterp, "gbe_kernel_get_name");
    if (interp_kernel_get_name == NULL)
      return false;

    interp_kernel_get_attributes = *(gbe_kernel_get_attributes_cb**)dlsym(dlhInterp, "gbe_kernel_get_attributes");
    if (interp_kernel_get_attributes == NULL)
      return false;

    interp_kernel_get_code = *(gbe_kernel_get_code_cb**)dlsym(dlhInterp, "gbe_kernel_get_code");
    if (interp_kernel_get_code == NULL)
      return false;

    interp_kernel_get_code_size = *(gbe_kernel_get_code_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_code_size");
    if (interp_kernel_get_code_size == NULL)
      return false;

    interp_kernel_get_arg_num = *(gbe_kernel_get_arg_num_cb**)dlsym(dlhInterp, "gbe_kernel_get_arg_num");
    if (interp_kernel_get_arg_num == NULL)
      return false;

    interp_kernel_get_arg_size = *(gbe_kernel_get_arg_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_arg_size");
    if (interp_kernel_get_arg_size == NULL)
      return false;

    interp_kernel_get_arg_bti = *(gbe_kernel_get_arg_bti_cb**)dlsym(dlhInterp, "gbe_kernel_get_arg_bti");
    if (interp_kernel_get_arg_bti == NULL)
      return false;

    interp_kernel_get_arg_type = *(gbe_kernel_get_arg_type_cb**)dlsym(dlhInterp, "gbe_kernel_get_arg_type");
    if (interp_kernel_get_arg_type == NULL)
      return false;

    interp_kernel_get_arg_align = *(gbe_kernel_get_arg_align_cb**)dlsym(dlhInterp, "gbe_kernel_get_arg_align");
    if (interp_kernel_get_arg_align == NULL)
      return false;

    interp_kernel_get_simd_width = *(gbe_kernel_get_simd_width_cb**)dlsym(dlhInterp, "gbe_kernel_get_simd_width");
    if (interp_kernel_get_simd_width == NULL)
      return false;

    interp_kernel_get_curbe_offset = *(gbe_kernel_get_curbe_offset_cb**)dlsym(dlhInterp, "gbe_kernel_get_curbe_offset");
    if (interp_kernel_get_curbe_offset == NULL)
      return false;

    interp_kernel_get_curbe_size = *(gbe_kernel_get_curbe_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_curbe_size");
    if (interp_kernel_get_curbe_size == NULL)
      return false;

    interp_kernel_get_stack_size = *(gbe_kernel_get_stack_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_stack_size");
    if (interp_kernel_get_stack_size == NULL)
      return false;

    interp_kernel_get_scratch_size = *(gbe_kernel_get_scratch_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_scratch_size");
    if (interp_kernel_get_scratch_size == NULL)
      return false;

    interp_kernel_get_required_work_group_size = *(gbe_kernel_get_required_work_group_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_required_work_group_size");
    if (interp_kernel_get_required_work_group_size == NULL)
      return false;

    interp_kernel_use_slm = *(gbe_kernel_use_slm_cb**)dlsym(dlhInterp, "gbe_kernel_use_slm");
    if (interp_kernel_use_slm == NULL)
      return false;

    interp_kernel_get_slm_size = *(gbe_kernel_get_slm_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_slm_size");
    if (interp_kernel_get_slm_size == NULL)
      return false;

    interp_kernel_get_sampler_size = *(gbe_kernel_get_sampler_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_sampler_size");
    if (interp_kernel_get_sampler_size == NULL)
      return false;

    interp_kernel_get_sampler_data = *(gbe_kernel_get_sampler_data_cb**)dlsym(dlhInterp, "gbe_kernel_get_sampler_data");
    if (interp_kernel_get_sampler_data == NULL)
      return false;

    interp_kernel_get_compile_wg_size = *(gbe_kernel_get_compile_wg_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_compile_wg_size");
    if (interp_kernel_get_compile_wg_size == NULL)
      return false;

    interp_kernel_get_image_size = *(gbe_kernel_get_image_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_image_size");
    if (interp_kernel_get_image_size == NULL)
      return false;

    interp_kernel_get_image_data = *(gbe_kernel_get_image_data_cb**)dlsym(dlhInterp, "gbe_kernel_get_image_data");
    if (interp_kernel_get_image_data == NULL)
      return false;

    interp_get_printf_num = *(gbe_get_printf_num_cb**)dlsym(dlhInterp, "gbe_get_printf_num");
    if (interp_get_printf_num == NULL)
      return false;

    interp_get_printf_buf_bti = *(gbe_get_printf_buf_bti_cb**)dlsym(dlhInterp, "gbe_get_printf_buf_bti");
    if (interp_get_printf_buf_bti == NULL)
      return false;

    interp_get_printf_indexbuf_bti = *(gbe_get_printf_indexbuf_bti_cb**)dlsym(dlhInterp, "gbe_get_printf_indexbuf_bti");
    if (interp_get_printf_indexbuf_bti == NULL)
      return false;

    interp_dup_printfset = *(gbe_dup_printfset_cb**)dlsym(dlhInterp, "gbe_dup_printfset");
    if (interp_dup_printfset == NULL)
      return false;

    interp_get_printf_sizeof_size = *(gbe_get_printf_sizeof_size_cb**)dlsym(dlhInterp, "gbe_get_printf_sizeof_size");
    if (interp_get_printf_sizeof_size == NULL)
      return false;

    interp_release_printf_info = *(gbe_release_printf_info_cb**)dlsym(dlhInterp, "gbe_release_printf_info");
    if (interp_release_printf_info == NULL)
      return false;

    interp_output_printf = *(gbe_output_printf_cb**)dlsym(dlhInterp, "gbe_output_printf");
    if (interp_output_printf == NULL)
      return false;

    interp_kernel_get_arg_info = *(gbe_kernel_get_arg_info_cb**)dlsym(dlhInterp, "gbe_kernel_get_arg_info");
    if (interp_kernel_get_arg_info == NULL)
      return false;

    return true;
  }

  void LoadCompiler()
  {
    compilerLoaded = false;

    const char* nonCompiler = getenv("OCL_NON_COMPILER");
    if (nonCompiler != NULL) {
      if (strcmp(nonCompiler, "1") == 0)
        return;
    }

    const char* gbePath = getenv("OCL_GBE_PATH");
    if (gbePath == NULL)
      gbePath = GBE_OBJECT_DIR;

    dlhCompiler = dlopen(gbePath, RTLD_LAZY | RTLD_LOCAL);
    if (dlhCompiler != NULL) {
      compiler_program_new_from_source = *(gbe_program_new_from_source_cb **)dlsym(dlhCompiler, "gbe_program_new_from_source");
      if (compiler_program_new_from_source == NULL)
        return;

      compiler_program_compile_from_source = *(gbe_program_compile_from_source_cb **)dlsym(dlhCompiler, "gbe_program_compile_from_source");
      if (compiler_program_compile_from_source == NULL)
        return;

      compiler_program_new_gen_program = *(gbe_program_new_gen_program_cb **)dlsym(dlhCompiler, "gbe_program_new_gen_program");
      if (compiler_program_new_gen_program == NULL)
        return;

      compiler_program_link_program = *(gbe_program_link_program_cb **)dlsym(dlhCompiler, "gbe_program_link_program");
      if (compiler_program_link_program == NULL)
        return;

      compiler_program_build_from_llvm = *(gbe_program_build_from_llvm_cb **)dlsym(dlhCompiler, "gbe_program_build_from_llvm");
      if (compiler_program_build_from_llvm == NULL)
        return;

      compiler_program_new_from_llvm_binary = *(gbe_program_new_from_llvm_binary_cb **)dlsym(dlhCompiler, "gbe_program_new_from_llvm_binary");
      if (compiler_program_new_from_llvm_binary == NULL)
        return;

      compiler_program_serialize_to_binary = *(gbe_program_serialize_to_binary_cb **)dlsym(dlhCompiler, "gbe_program_serialize_to_binary");
      if (compiler_program_serialize_to_binary == NULL)
        return;

      compiler_program_new_from_llvm = *(gbe_program_new_from_llvm_cb **)dlsym(dlhCompiler, "gbe_program_new_from_llvm");
      if (compiler_program_new_from_llvm == NULL)
        return;

      compiler_program_clean_llvm_resource = *(gbe_program_clean_llvm_resource_cb **)dlsym(dlhCompiler, "gbe_program_clean_llvm_resource");
      if (compiler_program_clean_llvm_resource == NULL)
        return;

      compilerLoaded = true;
    }
  }

  ~GbeLoaderInitializer()
  {
    if (dlhCompiler != NULL)
      dlclose(dlhCompiler);

    if (dlhInterp != NULL)
      dlclose(dlhInterp);
  }

  bool compilerLoaded;
  void *dlhCompiler;
  void *dlhInterp;
};

static struct GbeLoaderInitializer gbeLoader;

int CompilerSupported()
{
  if (gbeLoader.compilerLoaded)
    return 1;
  else
    return 0;
}
