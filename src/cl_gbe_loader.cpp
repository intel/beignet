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
#include <dlfcn.h>
#include <string.h>
#include "cl_gbe_loader.h"
#include "backend/src/GBEConfig.h"

struct GbeLoaderInitializer
{
  GbeLoaderInitializer() {
    inited = false;

    const char* nonCompiler = getenv("OCL_NON_COMPILER");
    if (nonCompiler != NULL) {
      if (strcmp(nonCompiler, "1") == 0)
        return;
    }

    const char* gbePath = getenv("OCL_GBE_PATH");
    if (gbePath == NULL)
      gbePath = GBE_OBJECT_DIR;

    dlh = dlopen(gbePath, RTLD_LAZY | RTLD_LOCAL | RTLD_DEEPBIND);
    if (dlh != NULL) {
      gbe_program_new_from_source = *(gbe_program_new_from_source_cb **)dlsym(dlh, "gbe_program_new_from_source");
      if (gbe_program_new_from_source == NULL)
        return;

      gbe_program_serialize_to_binary = *(gbe_program_serialize_to_binary_cb **)dlsym(dlh, "gbe_program_serialize_to_binary");
      if (gbe_program_serialize_to_binary == NULL)
        return;

      gbe_program_new_from_llvm = *(gbe_program_new_from_llvm_cb **)dlsym(dlh, "gbe_program_new_from_llvm");
      if (gbe_program_new_from_llvm == NULL)
        return;

      //gbe_kernel_set_const_buffer_size is not used by runttime
      gbe_kernel_set_const_buffer_size = *(gbe_kernel_set_const_buffer_size_cb **)dlsym(dlh, "gbe_kernel_set_const_buffer_size");
      if (gbe_kernel_set_const_buffer_size == NULL)
        return;

      gbe_set_image_base_index_compiler = *(gbe_set_image_base_index_cb **)dlsym(dlh, "gbe_set_image_base_index_compiler");
      if (gbe_set_image_base_index_compiler == NULL)
        return;

      inited = true;
    }
  }

  ~GbeLoaderInitializer() {
    if (dlh != NULL)
      dlclose(dlh);
  }

  bool inited;
  void *dlh;
};

static struct GbeLoaderInitializer gbeLoader;

int CompilerSupported()
{
  if (gbeLoader.inited)
    return 1;
  else
    return 0;
}
