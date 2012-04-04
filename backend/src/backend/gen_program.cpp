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
#include "backend/gen_program.hpp"
#include "backend/gen/brw_eu.h"
#include "ir/liveness.hpp"
#include "ir/value.hpp"
#include "ir/unit.hpp"
#include "llvm/llvm_to_gen.hpp"
#include <cstring>

namespace gbe {

  GenKernel::GenKernel(const std::string &name) :
    Kernel(name), insns(NULL), insnNum(0)
  {}
  GenKernel::~GenKernel(void) { GBE_SAFE_DELETE_ARRAY(insns); }

  GenProgram::GenProgram(void) {}
  GenProgram::~GenProgram(void) {}

  Kernel *GenProgram::compileKernel(const std::string &name) {
    GenKernel *kernel = GBE_NEW(GenKernel, name);
    brw_compile *p = (brw_compile*) GBE_MALLOC(sizeof(brw_compile));
    std::memset(p, 0, sizeof(*p));
    brw_EOT(p, 127);
    kernel->insnNum = p->nr_insn;
    kernel->insns = GBE_NEW_ARRAY(brw_instruction, kernel->insnNum);
    std::memcpy(kernel->insns, p->store, kernel->insnNum * sizeof(brw_instruction));
    GBE_FREE(p);
    return kernel;
  }

  static gbe_program genProgramNewFromSource(const char *source) {
    NOT_IMPLEMENTED;
    return NULL;
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
  gbe_program_new_from_source = gbe::genProgramNewFromSource;
  gbe_program_new_from_binary = gbe::genProgramNewFromBinary;
  gbe_program_new_from_llvm = gbe::genProgramNewFromLLVM;
}

