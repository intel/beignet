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
gbe_program_serialize_to_binary_cb *compiler_program_serialize_to_binary = NULL;
gbe_program_new_from_llvm_cb *compiler_program_new_from_llvm = NULL;
gbe_kernel_set_const_buffer_size_cb *compiler_kernel_set_const_buffer_size = NULL;
gbe_set_image_base_index_cb *compiler_set_image_base_index = NULL;

//function pointer from libgbeinterp.so
gbe_program_new_from_binary_cb *gbe_program_new_from_binary = NULL;
gbe_program_get_global_constant_size_cb *gbe_program_get_global_constant_size = NULL;
gbe_program_get_global_constant_data_cb *gbe_program_get_global_constant_data = NULL;
gbe_program_delete_cb *gbe_program_delete = NULL;
gbe_program_get_kernel_num_cb *gbe_program_get_kernel_num = NULL;
gbe_program_get_kernel_by_name_cb *gbe_program_get_kernel_by_name = NULL;
gbe_program_get_kernel_cb *gbe_program_get_kernel = NULL;
gbe_kernel_get_name_cb *gbe_kernel_get_name = NULL;
gbe_kernel_get_code_cb *gbe_kernel_get_code = NULL;
gbe_kernel_get_code_size_cb *gbe_kernel_get_code_size = NULL;
gbe_kernel_get_arg_num_cb *gbe_kernel_get_arg_num = NULL;
gbe_kernel_get_arg_size_cb *gbe_kernel_get_arg_size = NULL;
gbe_kernel_get_arg_type_cb *gbe_kernel_get_arg_type = NULL;
gbe_kernel_get_arg_align_cb *gbe_kernel_get_arg_align = NULL;
gbe_kernel_get_simd_width_cb *gbe_kernel_get_simd_width = NULL;
gbe_kernel_get_curbe_offset_cb *gbe_kernel_get_curbe_offset = NULL;
gbe_kernel_get_curbe_size_cb *gbe_kernel_get_curbe_size = NULL;
gbe_kernel_get_stack_size_cb *gbe_kernel_get_stack_size = NULL;
gbe_kernel_get_scratch_size_cb *gbe_kernel_get_scratch_size = NULL;
gbe_kernel_get_required_work_group_size_cb *gbe_kernel_get_required_work_group_size = NULL;
gbe_kernel_use_slm_cb *gbe_kernel_use_slm = NULL;
gbe_kernel_get_slm_size_cb *gbe_kernel_get_slm_size = NULL;
gbe_kernel_get_sampler_size_cb *gbe_kernel_get_sampler_size = NULL;
gbe_kernel_get_sampler_data_cb *gbe_kernel_get_sampler_data = NULL;
gbe_kernel_get_compile_wg_size_cb *gbe_kernel_get_compile_wg_size = NULL;
gbe_kernel_get_image_size_cb *gbe_kernel_get_image_size = NULL;
gbe_kernel_get_image_data_cb *gbe_kernel_get_image_data = NULL;
gbe_set_image_base_index_cb *gbe_set_image_base_index_interp = NULL;
gbe_get_image_base_index_cb *gbe_get_image_base_index = NULL;

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

    dlhInterp = dlopen(interpPath, RTLD_LAZY | RTLD_LOCAL | RTLD_DEEPBIND);
    if (dlhInterp == NULL) {
      return false;
    }

    gbe_program_new_from_binary = *(gbe_program_new_from_binary_cb**)dlsym(dlhInterp, "gbe_program_new_from_binary");
    if (gbe_program_new_from_binary == NULL)
      return false;

    gbe_program_get_global_constant_size = *(gbe_program_get_global_constant_size_cb**)dlsym(dlhInterp, "gbe_program_get_global_constant_size");
    if (gbe_program_get_global_constant_size == NULL)
      return false;

    gbe_program_get_global_constant_data = *(gbe_program_get_global_constant_data_cb**)dlsym(dlhInterp, "gbe_program_get_global_constant_data");
    if (gbe_program_get_global_constant_data == NULL)
      return false;

    gbe_program_delete = *(gbe_program_delete_cb**)dlsym(dlhInterp, "gbe_program_delete");
    if (gbe_program_delete == NULL)
      return false;

    gbe_program_get_kernel_num = *(gbe_program_get_kernel_num_cb**)dlsym(dlhInterp, "gbe_program_get_kernel_num");
    if (gbe_program_get_kernel_num == NULL)
      return false;

    gbe_program_get_kernel_by_name = *(gbe_program_get_kernel_by_name_cb**)dlsym(dlhInterp, "gbe_program_get_kernel_by_name");
    if (gbe_program_get_kernel_by_name == NULL)
      return false;

    gbe_program_get_kernel = *(gbe_program_get_kernel_cb**)dlsym(dlhInterp, "gbe_program_get_kernel");
    if (gbe_program_get_kernel == NULL)
      return false;

    gbe_kernel_get_name = *(gbe_kernel_get_name_cb**)dlsym(dlhInterp, "gbe_kernel_get_name");
    if (gbe_kernel_get_name == NULL)
      return false;

    gbe_kernel_get_code = *(gbe_kernel_get_code_cb**)dlsym(dlhInterp, "gbe_kernel_get_code");
    if (gbe_kernel_get_code == NULL)
      return false;

    gbe_kernel_get_code_size = *(gbe_kernel_get_code_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_code_size");
    if (gbe_kernel_get_code_size == NULL)
      return false;

    gbe_kernel_get_arg_num = *(gbe_kernel_get_arg_num_cb**)dlsym(dlhInterp, "gbe_kernel_get_arg_num");
    if (gbe_kernel_get_arg_num == NULL)
      return false;

    gbe_kernel_get_arg_size = *(gbe_kernel_get_arg_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_arg_size");
    if (gbe_kernel_get_arg_size == NULL)
      return false;

    gbe_kernel_get_arg_type = *(gbe_kernel_get_arg_type_cb**)dlsym(dlhInterp, "gbe_kernel_get_arg_type");
    if (gbe_kernel_get_arg_type == NULL)
      return false;

    gbe_kernel_get_arg_align = *(gbe_kernel_get_arg_align_cb**)dlsym(dlhInterp, "gbe_kernel_get_arg_align");
    if (gbe_kernel_get_arg_align == NULL)
      return false;

    gbe_kernel_get_simd_width = *(gbe_kernel_get_simd_width_cb**)dlsym(dlhInterp, "gbe_kernel_get_simd_width");
    if (gbe_kernel_get_simd_width == NULL)
      return false;

    gbe_kernel_get_curbe_offset = *(gbe_kernel_get_curbe_offset_cb**)dlsym(dlhInterp, "gbe_kernel_get_curbe_offset");
    if (gbe_kernel_get_curbe_offset == NULL)
      return false;

    gbe_kernel_get_curbe_size = *(gbe_kernel_get_curbe_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_curbe_size");
    if (gbe_kernel_get_curbe_size == NULL)
      return false;

    gbe_kernel_get_stack_size = *(gbe_kernel_get_stack_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_stack_size");
    if (gbe_kernel_get_stack_size == NULL)
      return false;

    gbe_kernel_get_scratch_size = *(gbe_kernel_get_scratch_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_scratch_size");
    if (gbe_kernel_get_scratch_size == NULL)
      return false;

    gbe_kernel_get_required_work_group_size = *(gbe_kernel_get_required_work_group_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_required_work_group_size");
    if (gbe_kernel_get_required_work_group_size == NULL)
      return false;

    gbe_kernel_use_slm = *(gbe_kernel_use_slm_cb**)dlsym(dlhInterp, "gbe_kernel_use_slm");
    if (gbe_kernel_use_slm == NULL)
      return false;

    gbe_kernel_get_slm_size = *(gbe_kernel_get_slm_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_slm_size");
    if (gbe_kernel_get_slm_size == NULL)
      return false;

    gbe_kernel_get_sampler_size = *(gbe_kernel_get_sampler_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_sampler_size");
    if (gbe_kernel_get_sampler_size == NULL)
      return false;

    gbe_kernel_get_sampler_data = *(gbe_kernel_get_sampler_data_cb**)dlsym(dlhInterp, "gbe_kernel_get_sampler_data");
    if (gbe_kernel_get_sampler_data == NULL)
      return false;

    gbe_kernel_get_compile_wg_size = *(gbe_kernel_get_compile_wg_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_compile_wg_size");
    if (gbe_kernel_get_compile_wg_size == NULL)
      return false;

    gbe_kernel_get_image_size = *(gbe_kernel_get_image_size_cb**)dlsym(dlhInterp, "gbe_kernel_get_image_size");
    if (gbe_kernel_get_image_size == NULL)
      return false;

    gbe_kernel_get_image_data = *(gbe_kernel_get_image_data_cb**)dlsym(dlhInterp, "gbe_kernel_get_image_data");
    if (gbe_kernel_get_image_data == NULL)
      return false;

    gbe_set_image_base_index_interp = *(gbe_set_image_base_index_cb**)dlsym(dlhInterp, "gbe_set_image_base_index");
    if (gbe_set_image_base_index_interp == NULL)
      return false;

    gbe_get_image_base_index = *(gbe_get_image_base_index_cb**)dlsym(dlhInterp, "gbe_get_image_base_index");
    if (gbe_get_image_base_index == NULL)
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

      compiler_program_serialize_to_binary = *(gbe_program_serialize_to_binary_cb **)dlsym(dlhCompiler, "gbe_program_serialize_to_binary");
      if (compiler_program_serialize_to_binary == NULL)
        return;

      compiler_program_new_from_llvm = *(gbe_program_new_from_llvm_cb **)dlsym(dlhCompiler, "gbe_program_new_from_llvm");
      if (compiler_program_new_from_llvm == NULL)
        return;

      //gbe_kernel_set_const_buffer_size is not used by runttime
      compiler_kernel_set_const_buffer_size = *(gbe_kernel_set_const_buffer_size_cb **)dlsym(dlhCompiler, "gbe_kernel_set_const_buffer_size");
      if (compiler_kernel_set_const_buffer_size == NULL)
        return;

      compiler_set_image_base_index = *(gbe_set_image_base_index_cb **)dlsym(dlhCompiler, "gbe_set_image_base_index");
      if (compiler_set_image_base_index == NULL)
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
