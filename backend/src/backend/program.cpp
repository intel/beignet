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

#include "program.h"
#include "program.hpp"
#include "gen_program.h"
#include "sys/platform.hpp"
#include "sys/cvar.hpp"
#include "ir/liveness.hpp"
#include "ir/value.hpp"
#include "ir/unit.hpp"
#include "llvm/llvm_to_gen.hpp"
#include "llvm/Config/config.h"
#include <cstring>
#include <algorithm>
#include <fstream>

/* Not defined for LLVM 3.0 */
#if !defined(LLVM_VERSION_MAJOR)
#define LLVM_VERSION_MAJOR 3
#endif /* !defined(LLVM_VERSION_MAJOR) */

/* Not defined for LLVM 3.0 */
#if !defined(LLVM_VERSION_MINOR)
#define LLVM_VERSION_MINOR 0
#endif /* !defined(LLVM_VERSION_MINOR) */

namespace gbe {

  Kernel::Kernel(const std::string &name) :
    name(name), args(NULL), argNum(0), curbeSize(0), stackSize(0), useSLM(false)
  {}
  Kernel::~Kernel(void) {
    GBE_SAFE_DELETE_ARRAY(args);
  }
  int32_t Kernel::getCurbeOffset(gbe_curbe_type type, uint32_t subType) const {
    const PatchInfo patch(type, subType);
    const auto it = std::lower_bound(patches.begin(), patches.end(), patch);
    if (it == patches.end()) return -1; // nothing found
    if (patch < *it) return -1; // they are not equal
    return it->offset; // we found it!
  }

  Program::Program(void) {}
  Program::~Program(void) {
    for (auto &kernel : kernels) GBE_DELETE(kernel.second);
  }

  BVAR(OCL_OUTPUT_GEN_IR, false);

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
    if (OCL_OUTPUT_GEN_IR) std::cout << unit;
    if (kernelNum == 0) return true;
    for (const auto &pair : set) {
      const std::string &name = pair.first;
      Kernel *kernel = this->compileKernel(unit, name);
      kernels.insert(std::make_pair(name, kernel));
    }
    return true;
  }

  static void programDelete(gbe_program gbeProgram) {
    gbe::Program *program = (gbe::Program*)(gbeProgram);
    GBE_SAFE_DELETE(program);
  }

  extern std::string ocl_stdlib_str;
  extern std::string ocl_common_defines_str;
  static gbe_program programNewFromSource(const char *source,
                                          size_t stringSize,
                                          const char *options,
                                          char *err,
                                          size_t *errSize)
  {
    char clStr[L_tmpnam+1], llStr[L_tmpnam+1];
    const std::string clName = std::string(tmpnam_r(clStr)) + ".cl"; /* unsafe! */
    const std::string llName = std::string(tmpnam_r(llStr)) + ".ll"; /* unsafe! */

    // Write the source to the cl file
    FILE *clFile = fopen(clName.c_str(), "w");
    FATAL_IF(clFile == NULL, "Failed to open temporary file");
    fwrite(ocl_common_defines_str.c_str(), strlen(ocl_common_defines_str.c_str()), 1, clFile);
    fwrite(ocl_stdlib_str.c_str(), strlen(ocl_stdlib_str.c_str()), 1, clFile);
    fwrite(source, strlen(source), 1, clFile);
    fclose(clFile);

    // Now compile the code to llvm using clang
#if LLVM_VERSION_MINOR <= 1
    std::string compileCmd = "clang -x cl -fno-color-diagnostics -emit-llvm -O3 -ccc-host-triple ptx32 -c ";
#else
    std::string compileCmd = "clang -ffp-contract=off -emit-llvm -O3 -target nvptx -x cl -c ";
#endif /* LLVM_VERSION_MINOR <= 1 */
    compileCmd += clName;
    compileCmd += " ";
    if(options)
      compileCmd += options;
    compileCmd += " -o ";
    compileCmd += llName;

    // Open a pipe and compile from here. Using Clang API instead is better
    FILE *pipe = popen(compileCmd.c_str(), "r");
    FATAL_IF (pipe == NULL, "Unable to run extern compilation command");
    char msg[256];
    while (fgets(msg, sizeof(msg), pipe))
      std::cout << msg;
    pclose(pipe);
    remove(clName.c_str());

    // Now build the program from llvm
    gbe_program p = gbe_program_new_from_llvm(llName.c_str(), stringSize, err, errSize);
    remove(llName.c_str());
    return p;
  }

  static uint32_t programGetKernelNum(gbe_program gbeProgram) {
    if (gbeProgram == NULL) return 0;
    const gbe::Program *program = (const gbe::Program*) gbeProgram;
    return program->getKernelNum();
  }

  static gbe_kernel programGetKernelByName(gbe_program gbeProgram, const char *name) {
    if (gbeProgram == NULL) return NULL;
    const gbe::Program *program = (gbe::Program*) gbeProgram;
    return (gbe_kernel) program->getKernel(std::string(name));
  }

  static gbe_kernel programGetKernel(const gbe_program gbeProgram, uint32_t ID) {
    if (gbeProgram == NULL) return NULL;
    const gbe::Program *program = (gbe::Program*) gbeProgram;
    return (gbe_kernel) program->getKernel(ID);
  }

  static const char *kernelGetName(gbe_kernel genKernel) {
    if (genKernel == NULL) return NULL;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getName();
  }

  static const char *kernelGetCode(gbe_kernel genKernel) {
    if (genKernel == NULL) return NULL;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getCode();
  }

  static size_t kernelGetCodeSize(gbe_kernel genKernel) {
    if (genKernel == NULL) return 0u;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getCodeSize();
  }

  static uint32_t kernelGetArgNum(gbe_kernel genKernel) {
    if (genKernel == NULL) return 0u;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getArgNum();
  }

  static uint32_t kernelGetArgSize(gbe_kernel genKernel, uint32_t argID) {
    if (genKernel == NULL) return 0u;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getArgSize(argID);
  }

  static gbe_arg_type kernelGetArgType(gbe_kernel genKernel, uint32_t argID) {
    if (genKernel == NULL) return GBE_ARG_INVALID;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getArgType(argID);
  }

  static uint32_t kernelGetSIMDWidth(gbe_kernel genKernel) {
    if (genKernel == NULL) return GBE_ARG_INVALID;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getSIMDWidth();
  }

  static int32_t kernelGetCurbeOffset(gbe_kernel genKernel, gbe_curbe_type type, uint32_t subType) {
    if (genKernel == NULL) return 0;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getCurbeOffset(type, subType);
  }

  static int32_t kernelGetCurbeSize(gbe_kernel genKernel) {
    if (genKernel == NULL) return 0;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getCurbeSize();
  }

  static int32_t kernelGetStackSize(gbe_kernel genKernel) {
    if (genKernel == NULL) return 0;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getStackSize();
  }

  static int32_t kernelUseSLM(gbe_kernel genKernel) {
    if (genKernel == NULL) return 0;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getUseSLM() ? 1 : 0;
  }

  static uint32_t kernelGetRequiredWorkGroupSize(gbe_kernel kernel, uint32_t dim) {
    return 0u;
  }
} /* namespace gbe */

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
GBE_EXPORT_SYMBOL gbe_kernel_get_curbe_offset_cb *gbe_kernel_get_curbe_offset = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_curbe_size_cb *gbe_kernel_get_curbe_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_stack_size_cb *gbe_kernel_get_stack_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_required_work_group_size_cb *gbe_kernel_get_required_work_group_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_use_slm_cb *gbe_kernel_use_slm = NULL;

namespace gbe
{
  /* Use pre-main to setup the call backs */
  struct CallBackInitializer
  {
    CallBackInitializer(void) {
      gbe_program_new_from_source = gbe::programNewFromSource;
      gbe_program_delete = gbe::programDelete;
      gbe_program_get_kernel_num = gbe::programGetKernelNum;
      gbe_program_get_kernel_by_name = gbe::programGetKernelByName;
      gbe_program_get_kernel = gbe::programGetKernel;
      gbe_kernel_get_name = gbe::kernelGetName;
      gbe_kernel_get_code = gbe::kernelGetCode;
      gbe_kernel_get_code_size = gbe::kernelGetCodeSize;
      gbe_kernel_get_arg_num = gbe::kernelGetArgNum;
      gbe_kernel_get_arg_size = gbe::kernelGetArgSize;
      gbe_kernel_get_arg_type = gbe::kernelGetArgType;
      gbe_kernel_get_simd_width = gbe::kernelGetSIMDWidth;
      gbe_kernel_get_curbe_offset = gbe::kernelGetCurbeOffset;
      gbe_kernel_get_curbe_size = gbe::kernelGetCurbeSize;
      gbe_kernel_get_stack_size = gbe::kernelGetStackSize;
      gbe_kernel_get_required_work_group_size = gbe::kernelGetRequiredWorkGroupSize;
      gbe_kernel_use_slm = gbe::kernelUseSLM;
      genSetupCallBacks();
    }
  };

  static CallBackInitializer cbInitializer;
} /* namespace gbe */

