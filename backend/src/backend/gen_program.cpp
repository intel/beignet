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
#include "backend/gen75_context.hpp"
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
#ifdef GBE_COMPILER_AVAILABLE
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
#endif
  }

  GenProgram::~GenProgram(void) {}

  /*! We must avoid spilling at all cost with Gen */
  static const struct CodeGenStrategy {
    uint32_t simdWidth;
    uint32_t reservedSpillRegs;
    bool limitRegisterPressure;
  } codeGenStrategy[] = {
    {16, 0, false},
    {16, 10, false},
    {8, 0, false},
    {8, 8, false},
    {8, 16, false},
  };

  Kernel *GenProgram::compileKernel(const ir::Unit &unit, const std::string &name, bool relaxMath) {
#ifdef GBE_COMPILER_AVAILABLE
    // Be careful when the simdWidth is forced by the programmer. We can see it
    // when the function already provides the simd width we need to use (i.e.
    // non zero)
    const ir::Function *fn = unit.getFunction(name);
    uint32_t codeGenNum = sizeof(codeGenStrategy) / sizeof(codeGenStrategy[0]);
    uint32_t codeGen = 0;
    GenContext *ctx = NULL;
    if (fn->getSimdWidth() == 8) {
      codeGen = 2;
    } else if (fn->getSimdWidth() == 16) {
      codeGenNum = 2;
    } else if (fn->getSimdWidth() == 0) {
      codeGen = 0;
    } else
      GBE_ASSERT(0);
    Kernel *kernel = NULL;

    // Stop when compilation is successful
    if (IS_IVYBRIDGE(deviceID)) {
      ctx = GBE_NEW(GenContext, unit, name, deviceID, relaxMath);
    } else if (IS_HASWELL(deviceID)) {
      ctx = GBE_NEW(Gen75Context, unit, name, deviceID, relaxMath);
    }
    GBE_ASSERTM(ctx != NULL, "Fail to create the gen context\n");

    for (; codeGen < codeGenNum; ++codeGen) {
      const uint32_t simdWidth = codeGenStrategy[codeGen].simdWidth;
      const bool limitRegisterPressure = codeGenStrategy[codeGen].limitRegisterPressure;
      const uint32_t reservedSpillRegs = codeGenStrategy[codeGen].reservedSpillRegs;

      // Force the SIMD width now and try to compile
      unit.getFunction(name)->setSimdWidth(simdWidth);
      ctx->startNewCG(simdWidth, reservedSpillRegs, limitRegisterPressure);
      kernel = ctx->compileKernel();
      if (kernel != NULL) {
        GBE_ASSERT(ctx->getErrCode() == NO_ERROR);
        break;
      }
      fn->getImageSet()->clearInfo();
      // If we get a out of range if/endif error.
      // We need to set the context to if endif fix mode and restart the previous compile.
      if ( ctx->getErrCode() == OUT_OF_RANGE_IF_ENDIF && !ctx->getIFENDIFFix() ) {
        ctx->setIFENDIFFix(true);
        codeGen--;
      } else
        GBE_ASSERT(!(ctx->getErrCode() == OUT_OF_RANGE_IF_ENDIF && ctx->getIFENDIFFix()));
    }

    GBE_ASSERTM(kernel != NULL, "Fail to compile kernel, may need to increase reserved registers for spilling.");
    return kernel;
#else
    return NULL;
#endif
  }

  static gbe_program genProgramNewFromBinary(uint32_t deviceID, const char *binary, size_t size) {
    using namespace gbe;
    std::string binary_content;
    binary_content.assign(binary, size);
    GenProgram *program = GBE_NEW(GenProgram, deviceID);
    std::istringstream ifs(binary_content, std::ostringstream::binary);
    // FIXME we need to check the whether the current device ID match the binary file's.
    deviceID = deviceID;

    if (!program->deserializeFromBin(ifs)) {
      delete program;
      return NULL;
    }

    //program->printStatus(0, std::cout);
    return reinterpret_cast<gbe_program>(program);
  }

  static size_t genProgramSerializeToBinary(gbe_program program, char **binary) {
    using namespace gbe;
    size_t sz;
    std::ostringstream oss;
    GenProgram *prog = (GenProgram*)program;

    if ((sz = prog->serializeToBin(oss)) == 0) {
      *binary = 0;
      return 0;
    }

    *binary = (char *)malloc(sizeof(char) * sz);
    memcpy(*binary, oss.str().c_str(), sz*sizeof(char));
    return sz;
  }

  static gbe_program genProgramNewFromLLVM(uint32_t deviceID,
                                           const char *fileName,
                                           size_t stringSize,
                                           char *err,
                                           size_t *errSize,
                                           int optLevel)
  {
    using namespace gbe;
    GenProgram *program = GBE_NEW(GenProgram, deviceID);
#ifdef GBE_COMPILER_AVAILABLE
    std::string error;
    // Try to compile the program
    if (program->buildFromLLVMFile(fileName, error, optLevel) == false) {
      if (err != NULL && errSize != NULL && stringSize > 0u) {
        const size_t msgSize = std::min(error.size(), stringSize-1u);
        std::memcpy(err, error.c_str(), msgSize);
        *errSize = error.size();
      }
      GBE_DELETE(program);
      return NULL;
    }
#endif
    // Everything run fine
    return (gbe_program) program;
  }
} /* namespace gbe */

void genSetupCallBacks(void)
{
  gbe_program_new_from_binary = gbe::genProgramNewFromBinary;
  gbe_program_serialize_to_binary = gbe::genProgramSerializeToBinary;
  gbe_program_new_from_llvm = gbe::genProgramNewFromLLVM;
}
