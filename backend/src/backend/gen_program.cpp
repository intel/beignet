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
#include "backend/gen/gen_mesa_disasm.h"
#include "backend/gen_reg_allocation.hpp"
#include "ir/unit.hpp"
#include "llvm/llvm_to_gen.hpp"

#include <cstring>
#include <sstream>
#include <memory>
#include <iostream>
#include <fstream>

namespace gbe {

  GenKernel::GenKernel(const std::string &name) :
    Kernel(name), insns(NULL), insnNum(0)
  {}
  GenKernel::~GenKernel(void) { GBE_SAFE_DELETE_ARRAY(insns); }
  const char *GenKernel::getCode(void) const { return (const char*) insns; }
  const void GenKernel::setCode(const char * ins, size_t size) {
    insns = (GenInstruction *)ins;
    insnNum = size / sizeof(GenInstruction);
  }
  size_t GenKernel::getCodeSize(void) const { return insnNum * sizeof(GenInstruction); }

  void GenKernel::printStatus(int indent, std::ostream& outs) {
    Kernel::printStatus(indent, outs);

    FILE *f = fopen("/dev/null", "w");
    char *buf = new char[4096];
    setbuffer(f, buf, 4096);

    for (uint32_t i = 0; i < insnNum; i++) {
      gen_disasm(f, insns+i);
      outs << buf;
      fflush(f);
      setbuffer(f, NULL, 0);
      setbuffer(f, buf, 4096);
    }

    setbuffer(f, NULL, 0);
    delete [] buf;
    fclose(f);
  }

  GenProgram::GenProgram(void) {}
  GenProgram::~GenProgram(void) {}

  /*! We must avoid spilling at all cost with Gen */
  static const struct CodeGenStrategy {
    uint32_t simdWidth;
    bool limitRegisterPressure;
  } codeGenStrategy[] = {
    {16,false},
    {16,true},
    {8,false},
    {8,true},
  };

  Kernel *GenProgram::compileKernel(const ir::Unit &unit, const std::string &name) {

    // Be careful when the simdWidth is forced by the programmer. We can see it
    // when the function already provides the simd width we need to use (i.e.
    // non zero)
    const ir::Function *fn = unit.getFunction(name);
    const uint32_t codeGenNum = fn->getSimdWidth() != 0 ? 2 : 4;
    uint32_t codeGen = fn->getSimdWidth() == 8 ? 2 : 0;
    Kernel *kernel = NULL;

    // Stop when compilation is successful
    for (; codeGen < codeGenNum; ++codeGen) {
      const uint32_t simdWidth = codeGenStrategy[codeGen].simdWidth;
      const bool limitRegisterPressure = codeGenStrategy[codeGen].limitRegisterPressure;

      // Force the SIMD width now and try to compile
      unit.getFunction(name)->setSimdWidth(simdWidth);
      Context *ctx = GBE_NEW(GenContext, unit, name, limitRegisterPressure);
      kernel = ctx->compileKernel();
      if (kernel != NULL) {
        break;
      }
      GBE_DELETE(ctx);
    }

    // XXX spill must be implemented
    GBE_ASSERTM(kernel != NULL, "Register spilling not supported yet!");
    return kernel;
  }

  static gbe_program genProgramNewFromBinary(const char *binary, size_t size) {
    using namespace gbe;
    std::string binary_content;
    binary_content.assign(binary, size);
    GenProgram *program = GBE_NEW_NO_ARG(GenProgram);
    std::istringstream ifs(binary_content, std::ostringstream::binary);

    if (!program->deserializeFromBin(ifs)) {
      delete program;
      return NULL;
    }

    //program->printStatus(0, std::cout);
    return reinterpret_cast<gbe_program>(program);
  }

  static gbe_program genProgramNewFromLLVM(const char *fileName,
                                           size_t stringSize,
                                           char *err,
                                           size_t *errSize)
  {
    using namespace gbe;
    GenProgram *program = GBE_NEW_NO_ARG(GenProgram);
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

sem_t llvm_semaphore;

void genSetupLLVMSemaphore(void)
{
  sem_init(&llvm_semaphore, 0, 1);
}
