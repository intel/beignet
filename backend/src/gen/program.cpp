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
 * \file program.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "program.h"
#include "gen/program.h"
#include "gen/program.hpp"
#include "gen/program.hpp"
#include "gen/brw_eu.h"
#include "ir/liveness.hpp"
#include "ir/value.hpp"
#include "ir/unit.hpp"
#include "llvm/llvm_to_gen.hpp"
#include <cstring>

namespace gbe {
namespace gen {

  Kernel::Kernel(const std::string &name) :
    name(name), args(NULL), insns(NULL), argNum(0), insnNum(0), liveness(NULL), dag(NULL)
  {}
  Kernel::~Kernel(void) {
    GBE_SAFE_DELETE_ARRAY(insns);
    GBE_SAFE_DELETE_ARRAY(args);
    GBE_SAFE_DELETE(liveness);
    GBE_SAFE_DELETE(dag);
  }

  Program::Program(void) {}
  Program::~Program(void) {
    for (auto it = kernels.begin(); it != kernels.end(); ++it)
      GBE_DELETE(it->second);
  }

  bool Program::buildFromSource(const char *source, std::string &error) {
    NOT_IMPLEMENTED;
    return false;
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
      // const ir::Function &fn = *it->second;
      Kernel *kernel = GBE_NEW(Kernel, name);
      brw_compile *p = (brw_compile*) GBE_MALLOC(sizeof(brw_compile));
      std::memset(p, 0, sizeof(*p));
      brw_EOT(p, 127);
      kernel->insnNum = p->nr_insn;
      kernel->insns = GBE_NEW_ARRAY(brw_instruction, kernel->insnNum);
      std::memcpy(kernel->insns, p->store, kernel->insnNum * sizeof(brw_instruction));
      GBE_FREE(p);
      kernels.insert(std::make_pair(name, kernel));
    }

    return true;
  }

} /* namespace gen */
} /* namespace gbe */

/////////////////////////////////////////////////////////////////////////////
// C interface for the Gen Programs
/////////////////////////////////////////////////////////////////////////////
static
GBEProgram *GenProgramNewFromSource(const char *source) {
  NOT_IMPLEMENTED;
  return NULL;
}

static
GBEProgram *GenProgramNewFromBinary(const char *binary, size_t size) {
  NOT_IMPLEMENTED;
  return NULL;
}

static
GBEProgram *GenProgramNewFromLLVM(const char *fileName,
                                  size_t stringSize,
                                  char *err,
                                  size_t *errSize)
{
  using namespace gbe::gen;
  Program *program = GBE_NEW(Program);
  std::string error;

  // Try to compile the program
  if (program->buildFromLLVMFile(fileName, error) == false) {
    if (err != NULL && errSize != NULL && stringSize > 0u) {
      const size_t msgSize = std::min(error.size(), stringSize-1u);
      std::memcpy(err, error.c_str(), msgSize);
      *errSize = error.size();
    }
    GBE_DELETE(program);
    return NULL;
  }

  // Everything run fine
  return (GBEProgram *) program;
}

static
void GenProgramDelete(GBEProgram *genProgram) {
  gbe::gen::Program *program = (gbe::gen::Program*)(genProgram);
  GBE_SAFE_DELETE(program);
}

/////////////////////////////////////////////////////////////////////////////
// C interface for the Gen Kernels
/////////////////////////////////////////////////////////////////////////////
static uint32_t GenProgramGetKernelNum(const GBEProgram *genProgram) {
  if (genProgram == NULL) return 0;
  const gbe::gen::Program *program = (const gbe::gen::Program*) genProgram;
  return program->getKernelNum();
}

static const GBEKernel *GenProgramGetKernelByName(const GBEProgram *genProgram, const char *name) {
  if (genProgram == NULL) return NULL;
  const gbe::gen::Program *program = (const gbe::gen::Program*) genProgram;
  return (GBEKernel*) program->getKernel(std::string(name));
}

static const GBEKernel *GenProgramGetKernel(const GBEProgram *genProgram, uint32_t ID) {
  if (genProgram == NULL) return NULL;
  const gbe::gen::Program *program = (const gbe::gen::Program*) genProgram;
  return (GBEKernel*) program->getKernel(ID);
}

static const char *GenKernelGetName(const GBEKernel *genKernel) {
  if (genKernel == NULL) return NULL;
  const gbe::gen::Kernel *kernel = (const gbe::gen::Kernel*) genKernel;
  return kernel->getName();
}

static const char *GenKernelGetCode(const GBEKernel *genKernel) {
  if (genKernel == NULL) return NULL;
  const gbe::gen::Kernel *kernel = (const gbe::gen::Kernel*) genKernel;
  return kernel->getCode();
}

static const size_t GenKernelGetCodeSize(const GBEKernel *genKernel) {
  if (genKernel == NULL) return 0u;
  const gbe::gen::Kernel *kernel = (const gbe::gen::Kernel*) genKernel;
  return kernel->getCodeSize();
}

static uint32_t GenKernelGetArgNum(const GBEKernel *genKernel) {
  if (genKernel == NULL) return 0u;
  const gbe::gen::Kernel *kernel = (const gbe::gen::Kernel*) genKernel;
  return kernel->getArgNum();
}

static uint32_t GenKernelGetArgSize(const GBEKernel *genKernel, uint32_t argID) {
  if (genKernel == NULL) return 0u;
  const gbe::gen::Kernel *kernel = (const gbe::gen::Kernel*) genKernel;
  return kernel->getArgSize(argID);
}

static GBEArgType GenKernelGetArgType(const GBEKernel *genKernel, uint32_t argID) {
  if (genKernel == NULL) return GEN_ARG_INVALID;
  const gbe::gen::Kernel *kernel = (const gbe::gen::Kernel*) genKernel;
  return kernel->getArgType(argID);
}

static uint32_t GenKernelGetSIMDWidth(const GBEKernel *kernel) {
  return 16u;
}

static uint32_t GenKernelGetRequiredWorkGroupSize(const GBEKernel *kernel, uint32_t dim) {
  return 0u;
}

void GenSetupCallBacks(void)
{
  GBEProgramNewFromSource = GenProgramNewFromSource;
  GBEProgramNewFromBinary = GenProgramNewFromBinary;
  GBEProgramNewFromLLVM = GenProgramNewFromLLVM;
  GBEProgramDelete = GenProgramDelete;
  GBEProgramGetKernelNum = GenProgramGetKernelNum;
  GBEProgramGetKernelByName = GenProgramGetKernelByName;
  GBEProgramGetKernel = GenProgramGetKernel;
  GBEKernelGetName = GenKernelGetName;
  GBEKernelGetCode = GenKernelGetCode;
  GBEKernelGetCodeSize = GenKernelGetCodeSize;
  GBEKernelGetArgNum = GenKernelGetArgNum;
  GBEKernelGetArgSize = GenKernelGetArgSize;
  GBEKernelGetArgType = GenKernelGetArgType;
  GBEKernelGetSIMDWidth = GenKernelGetSIMDWidth;
  GBEKernelGetRequiredWorkGroupSize = GenKernelGetRequiredWorkGroupSize;
}

