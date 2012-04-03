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

/**
 * \file callback interface for the compiler
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "gbe_program.h"
#include "gbe_program.hpp"
#include "gen/program.h"
#include "sim/program.h"
#include "sys/platform.hpp"
#include "ir/liveness.hpp"
#include "ir/value.hpp"
#include "ir/unit.hpp"
#include "llvm/llvm_to_gen.hpp"
#include <cstring>

namespace gbe {

  Kernel::Kernel(const std::string &name) :
    name(name), args(NULL), argNum(0), liveness(NULL), dag(NULL)
  {}
  Kernel::~Kernel(void) {
    GBE_SAFE_DELETE_ARRAY(args);
    GBE_SAFE_DELETE(liveness);
    GBE_SAFE_DELETE(dag);
  }

  Program::Program(void) {}
  Program::~Program(void) {
    for (auto it = kernels.begin(); it != kernels.end(); ++it)
      GBE_DELETE(it->second);
  }

  bool Program::buildFromLLVMFile(const char *fileName, std::string &error) {
    ir::Unit unit;
    if (llvmToGen(unit, fileName) == false) {
      error = std::string(fileName) + " not found";
      return false;
    }
    this->buildFromUnit(unit, error);
    return true;
  }

  bool Program::buildFromUnit(const ir::Unit &unit, std::string &error) {
    const auto &set = unit.getFunctionSet();
    const uint32_t kernelNum = set.size();
    if (kernelNum == 0) return true;

    // Dummy functions now
    for (auto it = set.begin(); it != set.end(); ++it) {
      const std::string &name = it->first;
      Kernel *kernel = this->compileKernel(name);
      kernels.insert(std::make_pair(name, kernel));
    }

    return true;
  }

} /* namespace gbe */

/////////////////////////////////////////////////////////////////////////////
// Common C interface for both Gen and simulator
/////////////////////////////////////////////////////////////////////////////
static void GBEProgramDelete(gbe_program gbeProgram) {
  gbe::Program *program = (gbe::Program*)(gbeProgram);
  GBE_SAFE_DELETE(program);
}

static uint32_t GBEProgramGetKernelNum(gbe_program gbeProgram) {
  if (gbeProgram == NULL) return 0;
  const gbe::Program *program = (const gbe::Program*) gbeProgram;
  return program->getKernelNum();
}

static gbe_kernel GBEProgramGetKernelByName(gbe_program gbeProgram, const char *name) {
  if (gbeProgram == NULL) return NULL;
  const gbe::Program *program = (gbe::Program*) gbeProgram;
  return (gbe_kernel) program->getKernel(std::string(name));
}

static gbe_kernel GBEProgramGetKernel(const gbe_program gbeProgram, uint32_t ID) {
  if (gbeProgram == NULL) return NULL;
  const gbe::Program *program = (gbe::Program*) gbeProgram;
  return (gbe_kernel) program->getKernel(ID);
}

static const char *GBEKernelGetName(gbe_kernel genKernel) {
  if (genKernel == NULL) return NULL;
  const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
  return kernel->getName();
}

static const char *GBEKernelGetCode(gbe_kernel genKernel) {
  if (genKernel == NULL) return NULL;
  const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
  return kernel->getCode();
}

static size_t GBEKernelGetCodeSize(gbe_kernel genKernel) {
  if (genKernel == NULL) return 0u;
  const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
  return kernel->getCodeSize();
}

static uint32_t GBEKernelGetArgNum(gbe_kernel genKernel) {
  if (genKernel == NULL) return 0u;
  const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
  return kernel->getArgNum();
}

static uint32_t GBEKernelGetArgSize(gbe_kernel genKernel, uint32_t argID) {
  if (genKernel == NULL) return 0u;
  const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
  return kernel->getArgSize(argID);
}

static gbe_arg_type GBEKernelGetArgType(gbe_kernel genKernel, uint32_t argID) {
  if (genKernel == NULL) return GBE_ARG_INVALID;
  const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
  return kernel->getArgType(argID);
}

static uint32_t GBEKernelGetSIMDWidth(gbe_kernel kernel) {
  return 16u;
}

static uint32_t GBEKernelGetRequiredWorkGroupSize(gbe_kernel kernel, uint32_t dim) {
  return 0u;
}

GBE_EXPORT_SYMBOL gbe_program_new_from_source_cb *gbe_program_new_from_source = NULL;
GBE_EXPORT_SYMBOL gbe_program_new_from_binary_cb *gbe_program_new_from_binary = NULL;
GBE_EXPORT_SYMBOL gbe_program_new_from_llvm_cb *gbe_program_new_from_llvm = NULL;
GBE_EXPORT_SYMBOL gbe_program_delete_cb *gbe_program_delete = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_kernel_num_cb *gbe_program_get_kernel_num = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_kernel_by_name_cb *gbe_program_get_kernel_by_name = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_kernel_cb *gbe_program_get_kernel = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_name_cb *gbe_kernel_get_name = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_code_cb *gbe_kernel_get_code = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_code_size_cb *gbe_kernel_get_code_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_arg_num_cb *gbe_kernel_get_arg_num = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_arg_size_cb *gbe_kernel_get_arg_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_arg_type_cb *gbe_kernel_get_arg_type = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_simd_width_cb *gbe_kernel_get_simd_width = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_required_work_group_size_cb *gbe_kernel_get_required_work_group_size = NULL;

/* Use pre-main to setup the call backs */
struct CallBackInitializer
{
  CallBackInitializer(void) {
    gbe_program_delete = GBEProgramDelete;
    gbe_program_get_kernel_num = GBEProgramGetKernelNum;
    gbe_program_get_kernel_by_name = GBEProgramGetKernelByName;
    gbe_program_get_kernel = GBEProgramGetKernel;
    gbe_kernel_get_name = GBEKernelGetName;
    gbe_kernel_get_code = GBEKernelGetCode;
    gbe_kernel_get_code_size = GBEKernelGetCodeSize;
    gbe_kernel_get_arg_num = GBEKernelGetArgNum;
    gbe_kernel_get_arg_size = GBEKernelGetArgSize;
    gbe_kernel_get_arg_type = GBEKernelGetArgType;
    gbe_kernel_get_simd_width = GBEKernelGetSIMDWidth;
    gbe_kernel_get_required_work_group_size = GBEKernelGetRequiredWorkGroupSize;
    const char *run_it = getenv("OCL_SIMULATOR");
    if (run_it != NULL && !strcmp(run_it, "2"))
      simSetupCallBacks();
    else
      genSetupCallBacks();
  }
};

static CallBackInitializer cbInitializer;

