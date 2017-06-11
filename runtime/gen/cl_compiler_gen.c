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
 * Author: He Junyan <junyan.he@intel.com>
 */

#include "cl_gen.h"
#include <dlfcn.h>

LOCAL cl_int
cl_compiler_load_gen(cl_device_id device)
{
  const char *gbePath = NULL;
  void *dlhCompiler = NULL;
  void *genBuildProgram = NULL;
  void *genLinkProgram = NULL;
  void *genCompileProgram = NULL;
  void *genCheckCompilerOption = NULL;

  if (device->compiler.available == CL_TRUE)
    return CL_SUCCESS;

  gbePath = getenv("OCL_GBE_PATH");
  if (gbePath == NULL || !strcmp(gbePath, ""))
    gbePath = COMPILER_BACKEND_OBJECT;

  dlhCompiler = dlopen(gbePath, RTLD_LAZY | RTLD_LOCAL);
  if (dlhCompiler == NULL)
    return CL_COMPILER_NOT_AVAILABLE;

  genBuildProgram = dlsym(dlhCompiler, "GenBuildProgram");
  if (genBuildProgram == NULL) {
    dlclose(dlhCompiler);
    return CL_COMPILER_NOT_AVAILABLE;
  }

  genCompileProgram = dlsym(dlhCompiler, "GenCompileProgram");
  if (genCompileProgram == NULL) {
    dlclose(dlhCompiler);
    return CL_COMPILER_NOT_AVAILABLE;
  }

  genLinkProgram = dlsym(dlhCompiler, "GenLinkProgram");
  if (genLinkProgram == NULL) {
    dlclose(dlhCompiler);
    return CL_COMPILER_NOT_AVAILABLE;
  }

  genCheckCompilerOption = dlsym(dlhCompiler, "GenCheckCompilerOption");
  if (genCheckCompilerOption == NULL) {
    dlclose(dlhCompiler);
    return CL_COMPILER_NOT_AVAILABLE;
  }

  device->compiler.opaque = dlhCompiler;
  device->compiler.available = CL_TRUE;
  device->compiler.compiler_name = "libgbe.so";
  device->compiler.check_compiler_option = genCheckCompilerOption;
  device->compiler.build_program = genBuildProgram;
  device->compiler.compile_program = genCompileProgram;
  device->compiler.link_program = genLinkProgram;
  return CL_SUCCESS;
}

LOCAL cl_int
cl_compiler_unload_gen(cl_device_id device)
{
  assert(device->compiler.available);
  assert(device->compiler.opaque);

  dlclose(device->compiler.opaque);

  device->compiler.available = CL_FALSE;
  device->compiler.opaque = NULL;
  device->compiler.compiler_name = NULL;
  device->compiler.check_compiler_option = NULL;
  device->compiler.build_program = NULL;
  device->compiler.compile_program = NULL;
  device->compiler.link_program = NULL;
  return CL_SUCCESS;
}
