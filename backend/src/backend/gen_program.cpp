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

#include "llvm/Config/llvm-config.h"
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 2
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/DataLayout.h"
#else
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DataLayout.h"
#endif  /* LLVM_VERSION_MINOR <= 2 */

#if LLVM_VERSION_MINOR >= 5
#include "llvm/Linker/Linker.h"
#else
#include "llvm/Linker.h"
#endif
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IRReader/IRReader.h"

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
#include "llvm/llvm_gen_backend.hpp"

#include <clang/CodeGen/CodeGenAction.h>

#include <cstring>
#include <sstream>
#include <memory>
#include <iostream>
#include <fstream>
#include <mutex>
#include <unistd.h>

namespace gbe {

  GenKernel::GenKernel(const std::string &name, uint32_t deviceID) :
    Kernel(name), deviceID(deviceID), insns(NULL), insnNum(0)
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
    GenCompactInstruction * pCom = NULL;
    GenNativeInstruction nativeInsn;

    for (uint32_t i = 0; i < insnNum;) {
      pCom = (GenCompactInstruction*)(insns+i);
      if(pCom->bits1.cmpt_control == 1) {
        decompactInstruction(pCom, &nativeInsn);
        gen_disasm(f, &nativeInsn, deviceID, 1);
        i++;
      } else {
        gen_disasm(f, insns+i, deviceID, 0);
        i = i + 2;
      }
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

  void GenProgram::CleanLlvmResource(void){
#ifdef GBE_COMPILER_AVAILABLE
    if(module){
      delete (llvm::Module*)module;
      module = NULL;
    }

    if(llvm_ctx){
      delete (llvm::LLVMContext*)llvm_ctx;
      llvm_ctx = NULL;
    }
#endif
  }

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

#define BINARY_HEADER_LENGTH 8
#define IS_GEN_BINARY(binary) (*binary == '\0' && *(binary+1) == 'G'&& *(binary+2) == 'E' &&*(binary+3) == 'N' &&*(binary+4) == 'C')
#define FILL_GEN_BINARY(binary) do{*binary = '\0'; *(binary+1) = 'G'; *(binary+2) = 'E'; *(binary+3) = 'N'; *(binary+4) = 'C';}while(0)
#define FILL_DEVICE_ID(binary, src_hw_info) do {*(binary+5) = src_hw_info[0]; *(binary+6) = src_hw_info[1]; *(binary+7) = src_hw_info[2];}while(0)
#define DEVICE_MATCH(typeA, src_hw_info) ((IS_IVYBRIDGE(typeA) && !strcmp(src_hw_info, "IVB")) ||  \
                                      (IS_IVYBRIDGE(typeA) && !strcmp(src_hw_info, "BYT")) ||  \
                                      (IS_BAYTRAIL_T(typeA) && !strcmp(src_hw_info, "BYT")) ||  \
                                      (IS_HASWELL(typeA) && !strcmp(src_hw_info, "HSW")) )

  static gbe_program genProgramNewFromBinary(uint32_t deviceID, const char *binary, size_t size) {
    using namespace gbe;
    std::string binary_content;
    //the header length is 8 bytes: 1 byte is binary type, 4 bytes are bitcode header, 3  bytes are hw info.
    char src_hw_info[4]="";
    src_hw_info[0] = *(binary+5);
    src_hw_info[1] = *(binary+6);
    src_hw_info[2] = *(binary+7);

    // check whether is gen binary ('/0GENC')
    if(!IS_GEN_BINARY(binary)){
        return NULL;
    }
    // check the whether the current device ID match the binary file's.
    if(!DEVICE_MATCH(deviceID, src_hw_info)){
      return NULL;
    }

    binary_content.assign(binary+BINARY_HEADER_LENGTH, size-BINARY_HEADER_LENGTH);
    GenProgram *program = GBE_NEW(GenProgram, deviceID);
    std::istringstream ifs(binary_content, std::ostringstream::binary);

    if (!program->deserializeFromBin(ifs)) {
      delete program;
      return NULL;
    }

    //program->printStatus(0, std::cout);
    return reinterpret_cast<gbe_program>(program);
  }

  static gbe_program genProgramNewFromLLVMBinary(uint32_t deviceID, const char *binary, size_t size) {
#ifdef GBE_COMPILER_AVAILABLE
    using namespace gbe;
    std::string binary_content;
    //the first byte stands for binary_type.
    binary_content.assign(binary+1, size-1);
    llvm::StringRef llvm_bin_str(binary_content);
    llvm::LLVMContext& c = llvm::getGlobalContext();
    llvm::SMDiagnostic Err;
    llvm::MemoryBuffer* memory_buffer = llvm::MemoryBuffer::getMemBuffer(llvm_bin_str, "llvm_bin_str");
    acquireLLVMContextLock();
    llvm::Module* module = llvm::ParseIR(memory_buffer, Err, c);
    releaseLLVMContextLock();
    if(module == NULL){
      GBE_ASSERT(0);
    }

    GenProgram *program = GBE_NEW(GenProgram, deviceID, module);

    //program->printStatus(0, std::cout);
    return reinterpret_cast<gbe_program>(program);
#else
      return NULL;
#endif
  }

  static size_t genProgramSerializeToBinary(gbe_program program, char **binary, int binary_type) {
    using namespace gbe;
    size_t sz;
    std::ostringstream oss;
    GenProgram *prog = (GenProgram*)program;

    //0 means GEN binary, 1 means LLVM bitcode compiled object, 2 means LLVM bitcode library
    if(binary_type == 0){
      if ((sz = prog->serializeToBin(oss)) == 0) {
        *binary = NULL;
        return 0;
      }

      //add header to differetiate from llvm bitcode binary.
      //the header length is 8 bytes: 1 byte is binary type, 4 bytes are bitcode header, 3  bytes are hw info.
      *binary = (char *)malloc(sizeof(char) * (sz+BINARY_HEADER_LENGTH) );
      memset(*binary, 0, sizeof(char) * (sz+BINARY_HEADER_LENGTH) );
      FILL_GEN_BINARY(*binary);
      char src_hw_info[4]="";
      if(IS_IVYBRIDGE(prog->deviceID)){
        src_hw_info[0]='I';
        src_hw_info[1]='V';
        src_hw_info[2]='B';
        if(IS_BAYTRAIL_T(prog->deviceID)){
          src_hw_info[0]='B';
          src_hw_info[1]='Y';
          src_hw_info[2]='T';
        }
      }else if(IS_HASWELL(prog->deviceID)){
        src_hw_info[0]='H';
        src_hw_info[1]='S';
        src_hw_info[2]='W';
      }
      FILL_DEVICE_ID(*binary, src_hw_info);
      memcpy(*binary+BINARY_HEADER_LENGTH, oss.str().c_str(), sz*sizeof(char));
      return sz+BINARY_HEADER_LENGTH;
    }else{
#ifdef GBE_COMPILER_AVAILABLE
      std::string str;
      llvm::raw_string_ostream OS(str);
      llvm::WriteBitcodeToFile((llvm::Module*)prog->module, OS);
      std::string& bin_str = OS.str();
      int llsz = bin_str.size();
      *binary = (char *)malloc(sizeof(char) * (llsz+1) );
      *(*binary) = binary_type;
      memcpy(*binary+1, bin_str.c_str(), llsz);
      return llsz+1;
#else
      return 0;
#endif
    }
  }

  static gbe_program genProgramNewFromLLVM(uint32_t deviceID,
                                           const char *fileName,
                                           const void* module,
                                           const void* llvm_ctx,
                                           size_t stringSize,
                                           char *err,
                                           size_t *errSize,
                                           int optLevel)
  {
    using namespace gbe;
    GenProgram *program = GBE_NEW(GenProgram, deviceID, module, llvm_ctx);
#ifdef GBE_COMPILER_AVAILABLE
    std::string error;
    // Try to compile the program
    if (program->buildFromLLVMFile(fileName, module, error, optLevel) == false) {
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

  static gbe_program genProgramNewGenProgram(uint32_t deviceID, const void* module,
                                             const void* llvm_ctx)  {
    using namespace gbe;
    GenProgram *program = GBE_NEW(GenProgram, deviceID, module, llvm_ctx);
    // Everything run fine
    return (gbe_program) program;
  }

  static void genProgramLinkFromLLVM(gbe_program           dst_program,
                                     gbe_program           src_program,
                                     size_t                stringSize,
                                     char *                err,
                                     size_t *              errSize)
  {
#ifdef GBE_COMPILER_AVAILABLE
    using namespace gbe;
    std::string errMsg;
    if(((GenProgram*)dst_program)->module == NULL){
      ((GenProgram*)dst_program)->module = llvm::CloneModule((llvm::Module*)((GenProgram*)src_program)->module);
      errSize = 0;
    }else{
      //set the global variables and functions to link once to fix redefine.
      llvm::Module* src = (llvm::Module*)((GenProgram*)src_program)->module;
      for (llvm::Module::global_iterator I = src->global_begin(), E = src->global_end(); I != E; ++I) {
        I->setLinkage(llvm::GlobalValue::LinkOnceAnyLinkage);
      }

      for (llvm::Module::iterator I = src->begin(), E = src->end(); I != E; ++I) {
        llvm::Function *F = llvm::dyn_cast<llvm::Function>(I);
        if (F && isKernelFunction(*F)) continue;
        I->setLinkage(llvm::GlobalValue::LinkOnceAnyLinkage);
      }
      llvm::Module* dst = (llvm::Module*)((GenProgram*)dst_program)->module;
      llvm::Linker::LinkModules( dst,
                                 src,
                                 llvm::Linker::PreserveSource,
                                 &errMsg);
      if (errMsg.c_str() != NULL) {
        if (err != NULL && errSize != NULL && stringSize > 0u) {
          if(errMsg.length() < stringSize )
            stringSize = errMsg.length();
          strcpy(err, errMsg.c_str());
          err[stringSize+1] = '\0';
        }
      }
    }
    // Everything run fine
#endif
  }

  static void genProgramBuildFromLLVM(gbe_program program,
                                      size_t stringSize,
                                      char *err,
                                      size_t *errSize,
                                      const char *          options)
  {
#ifdef GBE_COMPILER_AVAILABLE
    using namespace gbe;
    std::string error;

    int optLevel = 1;

    if(options) {
      char *p;
      p = strstr(const_cast<char *>(options), "-cl-opt-disable");
      if (p)
        optLevel = 0;
    }

    GenProgram* p = (GenProgram*) program;
    // Try to compile the program
    acquireLLVMContextLock();
    llvm::Module* module = (llvm::Module*)p->module;

    if (p->buildFromLLVMFile(NULL, module, error, optLevel) == false) {
      if (err != NULL && errSize != NULL && stringSize > 0u) {
        const size_t msgSize = std::min(error.size(), stringSize-1u);
        std::memcpy(err, error.c_str(), msgSize);
        *errSize = error.size();
      }
      GBE_DELETE(p);
    }
    releaseLLVMContextLock();
#endif
  }

} /* namespace gbe */

void genSetupCallBacks(void)
{
  gbe_program_new_from_binary = gbe::genProgramNewFromBinary;
  gbe_program_new_from_llvm_binary = gbe::genProgramNewFromLLVMBinary;
  gbe_program_serialize_to_binary = gbe::genProgramSerializeToBinary;
  gbe_program_new_from_llvm = gbe::genProgramNewFromLLVM;
  gbe_program_new_gen_program = gbe::genProgramNewGenProgram;
  gbe_program_link_from_llvm = gbe::genProgramLinkFromLLVM;
  gbe_program_build_from_llvm = gbe::genProgramBuildFromLLVM;
}
