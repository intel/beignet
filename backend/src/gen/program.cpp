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

#include "gen/program.h"
#include "gen/program.hpp"
#include "ir/liveness.hpp"
#include "ir/value.hpp"
#include "ir/unit.hpp"
#include "llvm/llvm_to_gen.hpp"
#include "gen/brw_eu.h"
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
    }

    return true;
  }

} /* namespace gen */
} /* namespace gbe */

/////////////////////////////////////////////////////////////////////////////
// C interface for the Gen Programs
/////////////////////////////////////////////////////////////////////////////
GBE_EXPORT_SYMBOL
GenProgram *GenProgramNewFromSource(const char *source) {
  NOT_IMPLEMENTED;
  return NULL;
}

GBE_EXPORT_SYMBOL
GenProgram *GenProgramNewFromBinary(const char *binary, size_t size) {
  NOT_IMPLEMENTED;
  return NULL;
}

GBE_EXPORT_SYMBOL
GenProgram *GenProgramNewFromLLVM(const char *fileName,
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
  return (GenProgram *) program;
}

GBE_EXPORT_SYMBOL
void GenProgramDelete(GenProgram *genProgram) {
  gbe::gen::Program *program = (gbe::gen::Program*)(genProgram);
  GBE_SAFE_DELETE(program);
}

/////////////////////////////////////////////////////////////////////////////
// C interface for the Gen Kernels
/////////////////////////////////////////////////////////////////////////////
GBE_EXPORT_SYMBOL
uint32_t GenProgramGetKernelNum(const GenProgram *genProgram) {
  if (genProgram == NULL) return 0;
  const gbe::gen::Program *program = (const gbe::gen::Program*) genProgram;
  return program->getKernelNum();
}

GBE_EXPORT_SYMBOL
const GenKernel *GenProgramGetKernelByName(const GenProgram *genProgram, const char *name) {
  if (genProgram == NULL) return NULL;
  const gbe::gen::Program *program = (const gbe::gen::Program*) genProgram;
  return (GenKernel*) program->getKernel(std::string(name));
}

GBE_EXPORT_SYMBOL
const GenKernel *GenProgramGetKernel(const GenProgram *genProgram, uint32_t ID) {
  if (genProgram == NULL) return NULL;
  const gbe::gen::Program *program = (const gbe::gen::Program*) genProgram;
  return (GenKernel*) program->getKernel(ID);
}

GBE_EXPORT_SYMBOL
const char *GenKernelGetCode(const GenKernel *genKernel) {
  if (genKernel == NULL) return NULL;
  const gbe::gen::Kernel *kernel = (const gbe::gen::Kernel*) genKernel;
  return kernel->getCode();
}

GBE_EXPORT_SYMBOL
const size_t GenKernelGetCodeSize(const GenKernel *genKernel) {
  if (genKernel == NULL) return 0u;
  const gbe::gen::Kernel *kernel = (const gbe::gen::Kernel*) genKernel;
  return kernel->getCodeSize();
}

GBE_EXPORT_SYMBOL
uint32_t GenKernelGetArgNum(const GenKernel *genKernel) {
  if (genKernel == NULL) return 0u;
  const gbe::gen::Kernel *kernel = (const gbe::gen::Kernel*) genKernel;
  return kernel->getArgNum();
}

GBE_EXPORT_SYMBOL
uint32_t GenKernelGetArgSize(const GenKernel *genKernel, uint32_t argID) {
  if (genKernel == NULL) return 0u;
  const gbe::gen::Kernel *kernel = (const gbe::gen::Kernel*) genKernel;
  return kernel->getArgSize(argID);
}

GBE_EXPORT_SYMBOL
GenArgType GenKernelGetArgType(const GenKernel *genKernel, uint32_t argID) {
  if (genKernel == NULL) return GEN_ARG_INVALID;
  const gbe::gen::Kernel *kernel = (const gbe::gen::Kernel*) genKernel;
  return kernel->getArgType(argID);
}

