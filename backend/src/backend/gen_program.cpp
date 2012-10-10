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

#include "backend/program.h"
#include "backend/gen_program.h"
#include "backend/gen_program.hpp"
#include "backend/gen_context.hpp"
#include "backend/gen_defs.hpp"
#include "backend/gen_reg_allocation.hpp"
#include "ir/unit.hpp"
#include "llvm/llvm_to_gen.hpp"

#include <cstring>
#include <memory>

namespace gbe {

  GenKernel::GenKernel(const std::string &name) :
    Kernel(name), insns(NULL), insnNum(0)
  {}
  GenKernel::~GenKernel(void) { GBE_SAFE_DELETE_ARRAY(insns); }
  const char *GenKernel::getCode(void) const { return (const char*) insns; }
  size_t GenKernel::getCodeSize(void) const { return insnNum * sizeof(GenInstruction); }

  GenProgram::GenProgram(void) {}
  GenProgram::~GenProgram(void) {}

  Kernel *GenProgram::compileKernel(const ir::Unit &unit, const std::string &name) {
    Context *ctx = GBE_NEW(GenContext, unit, name);
    Kernel *kernel = NULL;

    // register allocation may fail. We may need to recompile in that case
    try {
      kernel = ctx->compileKernel();
    } catch (NotEnoughRegisterException e) {
      GBE_SAFE_DELETE(ctx->getKernel());
      GBE_SAFE_DELETE(ctx);
      unit.getFunction(name)->setSimdWidth(8);
      ctx = GBE_NEW(GenContext, unit, name);
      kernel = ctx->compileKernel();
    }
    GBE_DELETE(ctx);
    return kernel;
  }

  static gbe_program genProgramNewFromBinary(const char *binary, size_t size) {
    NOT_IMPLEMENTED;
    return NULL;
  }

  static gbe_program genProgramNewFromLLVM(const char *fileName,
                                           size_t stringSize,
                                           char *err,
                                           size_t *errSize)
  {
    using namespace gbe;
    GenProgram *program = GBE_NEW(GenProgram);
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
    return (gbe_program) program;
  }
} /* namespace gbe */

void genSetupCallBacks(void)
{
  gbe_program_new_from_binary = gbe::genProgramNewFromBinary;
  gbe_program_new_from_llvm = gbe::genProgramNewFromLLVM;
}

