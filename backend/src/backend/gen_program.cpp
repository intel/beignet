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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

/**
 * \file program.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifdef GBE_COMPILER_AVAILABLE
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DataLayout.h"
#include "llvm-c/Linker.h"
#include "llvm-c/BitReader.h"
#include "llvm-c/BitWriter.h"
#include "llvm/Transforms/Utils/Cloning.h"
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
#include "llvm/Bitcode/BitcodeWriter.h"
#else
#include "llvm/Bitcode/ReaderWriter.h"
#endif /* LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40 */
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IRReader/IRReader.h"
#endif

#include "backend/program.h"
#include "backend/gen_program.h"
#include "backend/gen_program.hpp"
#include "backend/gen_context.hpp"
#include "backend/gen75_context.hpp"
#include "backend/gen8_context.hpp"
#include "backend/gen9_context.hpp"
#include "backend/gen_defs.hpp"
#include "backend/gen/gen_mesa_disasm.h"
#include "backend/gen_reg_allocation.hpp"
#include "ir/unit.hpp"

#ifdef GBE_COMPILER_AVAILABLE
#include "llvm/llvm_to_gen.hpp"
#include "llvm/llvm_gen_backend.hpp"
#include <clang/CodeGen/CodeGenAction.h>
#endif

#include "sys/cvar.hpp"
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
  void GenKernel::setCode(const char * ins, size_t size) {
    insns = (GenInstruction *)ins;
    insnNum = size / sizeof(GenInstruction);
  }
  uint32_t GenKernel::getCodeSize(void) const { return insnNum * sizeof(GenInstruction); }

  void GenKernel::printStatus(int indent, std::ostream& outs) {
#ifdef GBE_COMPILER_AVAILABLE
    Kernel::printStatus(indent, outs);

    FILE *f = fopen("/dev/null", "w");
    if(!f) {
      outs << "could not open /dev/null !";
      return;
    }

    char *buf = new char[4096];
    setbuffer(f, buf, 4096);
    GenCompactInstruction * pCom = NULL;
    GenInstruction insn[2];

    uint32_t insn_version = 0;
    if (IS_GEN7(deviceID) || IS_GEN75(deviceID))
      insn_version = 7;
    else if (IS_GEN8(deviceID) || IS_GEN9(deviceID))
      insn_version = 8;

    for (uint32_t i = 0; i < insnNum;) {
      pCom = (GenCompactInstruction*)(insns+i);
      if(pCom->bits1.cmpt_control == 1) {
        decompactInstruction(pCom, &insn, insn_version);
        gen_disasm(f, &insn, deviceID, 1);
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
    llvm::LLVMContext* ctx = NULL;
    if(module){
      ctx = &((llvm::Module*)module)->getContext();
      (void)ctx;
      delete (llvm::Module*)module;
      module = NULL;
    }
//llvm's version < 3.9, ctx is global ctx, can't be deleted.
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 39
    //each module's context is individual, just delete it, ignaor llvm_ctx.
    if (ctx != NULL)
      delete ctx;
#else
    if(llvm_ctx){
      delete (llvm::LLVMContext*)llvm_ctx;
      llvm_ctx = NULL;
    }
#endif
#endif
  }

  /*! We must avoid spilling at all cost with Gen */
  struct CodeGenStrategy {
    uint32_t simdWidth;
    uint32_t reservedSpillRegs;
    bool limitRegisterPressure;
  };
  static const struct CodeGenStrategy codeGenStrategyDefault[] = {
    {16, 0, false},
    {8, 0, false},
    {8, 8, false},
    {8, 16, false},
  };
  static const struct CodeGenStrategy codeGenStrategySimd16[] = {
    {16, 0, false},
    {16, 8, false},
    {16, 16, false},
  };

  IVAR(OCL_SIMD_WIDTH, 8, 15, 16);
  Kernel *GenProgram::compileKernel(const ir::Unit &unit, const std::string &name,
                                    bool relaxMath, int profiling) {
#ifdef GBE_COMPILER_AVAILABLE
    // Be careful when the simdWidth is forced by the programmer. We can see it
    // when the function already provides the simd width we need to use (i.e.
    // non zero)
    const ir::Function *fn = unit.getFunction(name);
    const struct CodeGenStrategy* codeGenStrategy = codeGenStrategyDefault;
    if(fn == NULL)
      GBE_ASSERT(0);
    uint32_t codeGenNum = sizeof(codeGenStrategyDefault) / sizeof(codeGenStrategyDefault[0]);
    uint32_t codeGen = 0;
    GenContext *ctx = NULL;
    if ( fn->getSimdWidth() != 0 && OCL_SIMD_WIDTH != 15) {
      GBE_ASSERTM(0, "unsupported SIMD width!");
    }else if (fn->getSimdWidth() == 8 || OCL_SIMD_WIDTH == 8) {
      codeGen = 1;
    } else if (fn->getSimdWidth() == 16 || OCL_SIMD_WIDTH == 16){
      codeGenStrategy = codeGenStrategySimd16;
      codeGenNum = sizeof(codeGenStrategySimd16) / sizeof(codeGenStrategySimd16[0]);
    } else if (fn->getSimdWidth() == 0 && OCL_SIMD_WIDTH == 15) {
      codeGen = 0;
    } else
      GBE_ASSERTM(0, "unsupported SIMD width!");
    Kernel *kernel = NULL;

    // Stop when compilation is successful
    if (IS_IVYBRIDGE(deviceID)) {
      ctx = GBE_NEW(GenContext, unit, name, deviceID, relaxMath);
    } else if (IS_HASWELL(deviceID)) {
      ctx = GBE_NEW(Gen75Context, unit, name, deviceID, relaxMath);
    } else if (IS_BROADWELL(deviceID)) {
      ctx = GBE_NEW(Gen8Context, unit, name, deviceID, relaxMath);
    } else if (IS_CHERRYVIEW(deviceID)) {
      ctx = GBE_NEW(ChvContext, unit, name, deviceID, relaxMath);
    } else if (IS_SKYLAKE(deviceID)) {
      ctx = GBE_NEW(Gen9Context, unit, name, deviceID, relaxMath);
    } else if (IS_BROXTON(deviceID)) {
      ctx = GBE_NEW(BxtContext, unit, name, deviceID, relaxMath);
    } else if (IS_KABYLAKE(deviceID)) {
      ctx = GBE_NEW(KblContext, unit, name, deviceID, relaxMath);
    } else if (IS_COFFEELAKE(deviceID)) {
      ctx = GBE_NEW(KblContext, unit, name, deviceID, relaxMath);
    } else if (IS_GEMINILAKE(deviceID)) {
      ctx = GBE_NEW(GlkContext, unit, name, deviceID, relaxMath);
    }
    GBE_ASSERTM(ctx != NULL, "Fail to create the gen context\n");

    if (profiling) {
      ctx->setProfilingMode(true);
      unit.getProfilingInfo()->setDeviceID(deviceID);
    }

    ctx->setASMFileName(this->asm_file_name);

    for (; codeGen < codeGenNum; ++codeGen) {
      const uint32_t simdWidth = codeGenStrategy[codeGen].simdWidth;
      const bool limitRegisterPressure = codeGenStrategy[codeGen].limitRegisterPressure;
      const uint32_t reservedSpillRegs = codeGenStrategy[codeGen].reservedSpillRegs;

      // Force the SIMD width now and try to compile
      ir::Function *simdFn = unit.getFunction(name);
      if(simdFn == NULL)
        GBE_ASSERT(0);
      simdFn->setSimdWidth(simdWidth);
      ctx->startNewCG(simdWidth, reservedSpillRegs, limitRegisterPressure);
      kernel = ctx->compileKernel();
      if (kernel != NULL) {
        GBE_ASSERT(ctx->getErrCode() == NO_ERROR);
        kernel->setOclVersion(unit.getOclVersion());
        break;
      }
      simdFn->getImageSet()->clearInfo();
      // If we get a out of range if/endif error.
      // We need to set the context to if endif fix mode and restart the previous compile.
      if ( ctx->getErrCode() == OUT_OF_RANGE_IF_ENDIF && !ctx->getIFENDIFFix() ) {
        ctx->setIFENDIFFix(true);
        codeGen--;
      } else
        GBE_ASSERT(!(ctx->getErrCode() == OUT_OF_RANGE_IF_ENDIF && ctx->getIFENDIFFix()));
    }

    //GBE_ASSERTM(kernel != NULL, "Fail to compile kernel, may need to increase reserved registers for spilling.");
    return kernel;
#else
    return NULL;
#endif
  }

#define GEN_BINARY_HEADER_LENGTH 8

  enum GEN_BINARY_HEADER_INDEX {
    GBHI_BYT = 0,
    GBHI_IVB = 1,
    GBHI_HSW = 2,
    GBHI_CHV = 3,
    GBHI_BDW = 4,
    GBHI_SKL = 5,
    GBHI_BXT = 6,
    GBHI_KBL = 7,
    GBHI_GLK = 8,
    GBHI_MAX,
  };
#define GEN_BINARY_VERSION  1
  static const unsigned char gen_binary_header[GBHI_MAX][GEN_BINARY_HEADER_LENGTH]= \
                                             {{GEN_BINARY_VERSION, 'G','E', 'N', 'C', 'B', 'Y', 'T'},
                                              {GEN_BINARY_VERSION, 'G','E', 'N', 'C', 'I', 'V', 'B'},
                                              {GEN_BINARY_VERSION, 'G','E', 'N', 'C', 'H', 'S', 'W'},
                                              {GEN_BINARY_VERSION, 'G','E', 'N', 'C', 'C', 'H', 'V'},
                                              {GEN_BINARY_VERSION, 'G','E', 'N', 'C', 'B', 'D', 'W'},
                                              {GEN_BINARY_VERSION, 'G','E', 'N', 'C', 'S', 'K', 'L'},
                                              {GEN_BINARY_VERSION, 'G','E', 'N', 'C', 'B', 'X', 'T'},
                                              {GEN_BINARY_VERSION, 'G','E', 'N', 'C', 'K', 'B', 'T'},
                                              {GEN_BINARY_VERSION, 'G','E', 'N', 'C', 'G', 'L', 'K'}
                                              };

#define FILL_GEN_HEADER(binary, index)  do {int i = 0; do {*(binary+i) = gen_binary_header[index][i]; i++; }while(i < GEN_BINARY_HEADER_LENGTH);}while(0)
#define FILL_BYT_HEADER(binary) FILL_GEN_HEADER(binary, GBHI_BYT)
#define FILL_IVB_HEADER(binary) FILL_GEN_HEADER(binary, GBHI_IVB)
#define FILL_HSW_HEADER(binary) FILL_GEN_HEADER(binary, GBHI_HSW)
#define FILL_CHV_HEADER(binary) FILL_GEN_HEADER(binary, GBHI_CHV)
#define FILL_BDW_HEADER(binary) FILL_GEN_HEADER(binary, GBHI_BDW)
#define FILL_SKL_HEADER(binary) FILL_GEN_HEADER(binary, GBHI_SKL)
#define FILL_BXT_HEADER(binary) FILL_GEN_HEADER(binary, GBHI_BXT)
#define FILL_KBL_HEADER(binary) FILL_GEN_HEADER(binary, GBHI_KBL)
#define FILL_GLK_HEADER(binary) FILL_GEN_HEADER(binary, GBHI_GLK)

  static bool genHeaderCompare(const unsigned char *BufPtr, GEN_BINARY_HEADER_INDEX index)
  {
    bool matched = true;
    for (int i = 1; i < GEN_BINARY_HEADER_LENGTH; ++i)
    {
      matched = matched && (BufPtr[i] == gen_binary_header[index][i]);
    }
    if(matched) {
      if(BufPtr[0] != gen_binary_header[index][0]) {
        std::cout << "Beignet binary format have been changed, please generate binary again.\n";
        matched = false;
      }
    }
    return matched;
  }

#define MATCH_BYT_HEADER(binary) genHeaderCompare(binary, GBHI_BYT)
#define MATCH_IVB_HEADER(binary) genHeaderCompare(binary, GBHI_IVB)
#define MATCH_HSW_HEADER(binary) genHeaderCompare(binary, GBHI_HSW)
#define MATCH_CHV_HEADER(binary) genHeaderCompare(binary, GBHI_CHV)
#define MATCH_BDW_HEADER(binary) genHeaderCompare(binary, GBHI_BDW)
#define MATCH_SKL_HEADER(binary) genHeaderCompare(binary, GBHI_SKL)
#define MATCH_BXT_HEADER(binary) genHeaderCompare(binary, GBHI_BXT)
#define MATCH_KBL_HEADER(binary) genHeaderCompare(binary, GBHI_KBL)
#define MATCH_GLK_HEADER(binary) genHeaderCompare(binary, GBHI_GLK)

#define MATCH_DEVICE(deviceID, binary) ((IS_IVYBRIDGE(deviceID) && MATCH_IVB_HEADER(binary)) ||  \
                                      (IS_IVYBRIDGE(deviceID) && MATCH_IVB_HEADER(binary)) ||  \
                                      (IS_BAYTRAIL_T(deviceID) && MATCH_BYT_HEADER(binary)) ||  \
                                      (IS_HASWELL(deviceID) && MATCH_HSW_HEADER(binary)) ||  \
                                      (IS_BROADWELL(deviceID) && MATCH_BDW_HEADER(binary)) ||  \
                                      (IS_CHERRYVIEW(deviceID) && MATCH_CHV_HEADER(binary)) ||  \
                                      (IS_SKYLAKE(deviceID) && MATCH_SKL_HEADER(binary)) || \
                                      (IS_BROXTON(deviceID) && MATCH_BXT_HEADER(binary)) || \
                                      (IS_KABYLAKE(deviceID) && MATCH_KBL_HEADER(binary)) || \
                                      (IS_COFFEELAKE(deviceID) && MATCH_KBL_HEADER(binary)) || \
                                      (IS_GEMINILAKE(deviceID) && MATCH_GLK_HEADER(binary)) \
                                      )

  static gbe_program genProgramNewFromBinary(uint32_t deviceID, const char *binary, size_t size) {
    using namespace gbe;
    std::string binary_content;

    if(size < GEN_BINARY_HEADER_LENGTH)
      return NULL;

    //the header length is 8 bytes: 1 byte is binary type, 4 bytes are bitcode header, 3  bytes are hw info.
    if(!MATCH_DEVICE(deviceID, (unsigned char*)binary)){
      return NULL;
    }

    binary_content.assign(binary+GEN_BINARY_HEADER_LENGTH, size-GEN_BINARY_HEADER_LENGTH);
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
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 39
    llvm::LLVMContext *c = new llvm::LLVMContext;
#else
    llvm::LLVMContext *c = &llvm::getGlobalContext();
#endif
    llvm::SMDiagnostic Err;
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 36
    std::unique_ptr<llvm::MemoryBuffer> memory_buffer = llvm::MemoryBuffer::getMemBuffer(llvm_bin_str, "llvm_bin_str");
    acquireLLVMContextLock();
    llvm::Module* module = llvm::parseIR(memory_buffer->getMemBufferRef(), Err, *c).release();
#else
    llvm::MemoryBuffer* memory_buffer = llvm::MemoryBuffer::getMemBuffer(llvm_bin_str, "llvm_bin_str");
    acquireLLVMContextLock();
    llvm::Module* module = llvm::ParseIR(memory_buffer, Err, *c);
#endif
    // if load 32 bit spir binary, the triple should be spir-unknown-unknown.
    llvm::Triple triple(module->getTargetTriple());
    if (triple.getArchName() == "spir" && triple.getVendorName() == "unknown" && triple.getOSName() == "unknown"){
      module->setTargetTriple("spir");
    } else if (triple.getArchName() == "spir64" && triple.getVendorName() == "unknown" && triple.getOSName() == "unknown"){
      module->setTargetTriple("spir64");
    }
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
      *binary = (char *)malloc(sizeof(char) * (sz+GEN_BINARY_HEADER_LENGTH) );
      if(*binary == NULL)
        return 0;

      memset(*binary, 0, sizeof(char) * (sz+GEN_BINARY_HEADER_LENGTH) );
      if(IS_IVYBRIDGE(prog->deviceID)){
        FILL_IVB_HEADER(*binary);
        if(IS_BAYTRAIL_T(prog->deviceID)){
        FILL_BYT_HEADER(*binary);
        }
      }else if(IS_HASWELL(prog->deviceID)){
        FILL_HSW_HEADER(*binary);
      }else if(IS_BROADWELL(prog->deviceID)){
        FILL_BDW_HEADER(*binary);
      }else if(IS_CHERRYVIEW(prog->deviceID)){
        FILL_CHV_HEADER(*binary);
      }else if(IS_SKYLAKE(prog->deviceID)){
        FILL_SKL_HEADER(*binary);
      }else if(IS_BROXTON(prog->deviceID)){
        FILL_BXT_HEADER(*binary);
      }else if(IS_KABYLAKE(prog->deviceID)){
        FILL_KBL_HEADER(*binary);
      }else if(IS_COFFEELAKE(prog->deviceID)){
        FILL_KBL_HEADER(*binary);
      }else if(IS_GEMINILAKE(prog->deviceID)){
        FILL_GLK_HEADER(*binary);
      }else {
        free(*binary);
        *binary = NULL;
        return 0;
      }
      memcpy(*binary+GEN_BINARY_HEADER_LENGTH, oss.str().c_str(), sz*sizeof(char));
      return sz+GEN_BINARY_HEADER_LENGTH;
    }else{
#ifdef GBE_COMPILER_AVAILABLE
      std::string str;
      llvm::raw_string_ostream OS(str);
#if LLVM_VERSION_MAJOR >= 7
      llvm::WriteBitcodeToFile(*((llvm::Module*)prog->module), OS);
#else
      llvm::WriteBitcodeToFile((llvm::Module*)prog->module, OS);
#endif
      std::string& bin_str = OS.str();
      int llsz = bin_str.size();
      *binary = (char *)malloc(sizeof(char) * (llsz+1) );
      if(*binary == NULL)
        return 0;

      *(*binary) = binary_type;
      memcpy(*binary+1, bin_str.c_str(), llsz);
      return llsz+1;
#else
      return 0;
#endif
    }
  }

  static gbe_program genProgramNewFromLLVM(uint32_t deviceID,
                                           const void* module,
                                           const void* llvm_ctx,
                                           const char* asm_file_name,
                                           size_t stringSize,
                                           char *err,
                                           size_t *errSize,
                                           int optLevel,
                                           const char* options)
  {
    using namespace gbe;
    uint32_t fast_relaxed_math = 0;
    if (options != NULL)
      if (strstr(options, "-cl-fast-relaxed-math") != NULL)
        fast_relaxed_math = 1;

    GenProgram *program = GBE_NEW(GenProgram, deviceID, module, llvm_ctx, asm_file_name, fast_relaxed_math);
#ifdef GBE_COMPILER_AVAILABLE
    std::string error;
    // Try to compile the program
    if (program->buildFromLLVMModule(module, error, optLevel) == false) {
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
                                             const void* llvm_ctx,const char* asm_file_name)  {
    using namespace gbe;
    GenProgram *program = GBE_NEW(GenProgram, deviceID, module, llvm_ctx, asm_file_name);
    // Everything run fine
    return (gbe_program) program;
  }

  static bool genProgramLinkFromLLVM(gbe_program           dst_program,
                                     gbe_program           src_program,
                                     size_t                stringSize,
                                     char *                err,
                                     size_t *              errSize)
  {
#ifdef GBE_COMPILER_AVAILABLE
    using namespace gbe;
    char* errMsg = NULL;
    if(((GenProgram*)dst_program)->module == NULL){
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 39
      LLVMModuleRef modRef;
      LLVMParseBitcodeInContext2(wrap(new llvm::LLVMContext()),
                                 LLVMWriteBitcodeToMemoryBuffer(wrap((llvm::Module*)((GenProgram*)src_program)->module)),
                                 &modRef);
      ((GenProgram*)dst_program)->module = llvm::unwrap(modRef);
#elif LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
      ((GenProgram*)dst_program)->module = llvm::CloneModule((llvm::Module*)((GenProgram*)src_program)->module).release();
#else
      ((GenProgram*)dst_program)->module = llvm::CloneModule((llvm::Module*)((GenProgram*)src_program)->module);
#endif
      errSize = 0;
    } else {
      llvm::Module* src = (llvm::Module*)((GenProgram*)src_program)->module;
      llvm::Module* dst = (llvm::Module*)((GenProgram*)dst_program)->module;
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 39
      if (&src->getContext() != &dst->getContext()) {
        LLVMModuleRef modRef;
        LLVMParseBitcodeInContext2(wrap(&dst->getContext()),
                                    LLVMWriteBitcodeToMemoryBuffer(wrap(src)),
                                    &modRef);
        src = llvm::unwrap(modRef);
      }
#if LLVM_VERSION_MAJOR >= 7
      llvm::Module* clone = llvm::CloneModule(*src).release();
#else
      llvm::Module* clone = llvm::CloneModule(src).release();
#endif
      if (LLVMLinkModules2(wrap(dst), wrap(clone))) {
#elif LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
      if (LLVMLinkModules(wrap(dst), wrap(src), LLVMLinkerPreserveSource_Removed, &errMsg)) {
#else
      if (LLVMLinkModules(wrap(dst), wrap(src), LLVMLinkerPreserveSource, &errMsg)) {
#endif
        if (err != NULL && errSize != NULL && stringSize > 0u && errMsg) {
          strncpy(err, errMsg, stringSize-1);
          err[stringSize-1] = '\0';
          *errSize = strlen(err);
        }
        return true;
      }
    }
#endif
    return false;
  }

  static void genProgramBuildFromLLVM(gbe_program program,
                                      size_t stringSize,
                                      char *err,
                                      size_t *errSize,
                                      const char * options)
  {
#ifdef GBE_COMPILER_AVAILABLE
    using namespace gbe;
    std::string error;
    int optLevel = 1;
    std::string dumpASMFileName;
    size_t start = 0, end = 0;
    uint32_t fast_relaxed_math = 0;

    if(options) {
      char *p;
      p = strstr(const_cast<char *>(options), "-cl-opt-disable");
      if (p)
        optLevel = 0;

      if (options != NULL)
        if (strstr(options, "-cl-fast-relaxed-math") != NULL)
          fast_relaxed_math = 1;

      char *options_str = (char *)malloc(sizeof(char) * (strlen(options) + 1));
      if (options_str == NULL)
        return;
      memcpy(options_str, options, strlen(options) + 1);
      std::string optionStr(options_str);
      while (end != std::string::npos) {
        end = optionStr.find(' ', start);
        std::string str = optionStr.substr(start, end - start);
        start = end + 1;
        if(str.size() == 0)
          continue;

        if(str.find("-dump-opt-asm=") != std::string::npos) {
          dumpASMFileName = str.substr(str.find("=") + 1);
          continue; // Don't push this str back; ignore it.
        }
      }
      free(options_str);
    }

    GenProgram* p = (GenProgram*) program;
    p->fast_relaxed_math = fast_relaxed_math;
    if (!dumpASMFileName.empty()) {
      p->asm_file_name = dumpASMFileName.c_str();
      FILE *asmDumpStream = fopen(dumpASMFileName.c_str(), "w");
      if (asmDumpStream)
        fclose(asmDumpStream);
    }
    // Try to compile the program
    acquireLLVMContextLock();
    llvm::Module* module = (llvm::Module*)p->module;

    if (p->buildFromLLVMModule(module, error, optLevel) == false) {
      if (err != NULL && errSize != NULL && stringSize > 0u) {
        const size_t msgSize = std::min(error.size(), stringSize-1u);
        std::memcpy(err, error.c_str(), msgSize);
        *errSize = error.size();
      }
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
