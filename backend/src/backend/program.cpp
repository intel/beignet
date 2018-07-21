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
#include "ir/printf.hpp"
#include "src/cl_device_data.h"

#ifdef GBE_COMPILER_AVAILABLE
#include "llvm/llvm_to_gen.hpp"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/Threading.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"
#endif

#include <cstring>
#include <algorithm>
#include <fstream>
#include <dlfcn.h>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <mutex>

#ifdef GBE_COMPILER_AVAILABLE

#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/IR/Module.h>

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
#include <llvm/Bitcode/BitcodeWriter.h>
#include <clang/Lex/PreprocessorOptions.h>
#else
#include <llvm/Bitcode/ReaderWriter.h>
#endif
#include <llvm/Support/raw_ostream.h>
#endif

#include "src/GBEConfig.h"

namespace gbe {

  Kernel::Kernel(const std::string &name) :
    name(name), args(NULL), argNum(0), curbeSize(0), stackSize(0), useSLM(false),
        slmSize(0), ctx(NULL), samplerSet(NULL), imageSet(NULL), printfSet(NULL),
        profilingInfo(NULL), useDeviceEnqueue(false) {}

  Kernel::~Kernel(void) {
    if(ctx) GBE_DELETE(ctx);
    if(samplerSet) GBE_DELETE(samplerSet);
    if(imageSet) GBE_DELETE(imageSet);
    if(printfSet) GBE_DELETE(printfSet);
    if(profilingInfo) GBE_DELETE(profilingInfo);
    GBE_SAFE_DELETE_ARRAY(args);
  }
  int32_t Kernel::getCurbeOffset(gbe_curbe_type type, uint32_t subType) const {
    const PatchInfo patch(type, subType);
    const auto it = std::lower_bound(patches.begin(), patches.end(), patch);
    if (it == patches.end()) return -1; // nothing found
    if (patch < *it) return -1; // they are not equal
    return it->offset; // we found it!
  }

  Program::Program(uint32_t fast_relaxed_math) : fast_relaxed_math(fast_relaxed_math), 
                               constantSet(NULL),
                               relocTable(NULL) {}
  Program::~Program(void) {
    for (map<std::string, Kernel*>::iterator it = kernels.begin(); it != kernels.end(); ++it)
      GBE_DELETE(it->second);
    if (constantSet) delete constantSet;
    if (relocTable) delete relocTable;
  }

#ifdef GBE_COMPILER_AVAILABLE
  BVAR(OCL_OUTPUT_GEN_IR, false);
  BVAR(OCL_STRICT_CONFORMANCE, true);
  IVAR(OCL_PROFILING_LOG, 0, 0, 1); // Int for different profiling types.
  BVAR(OCL_OUTPUT_BUILD_LOG, false);

  bool Program::buildFromLLVMModule(const void* module,
                                              std::string &error,
                                              int optLevel) {
    ir::Unit *unit = new ir::Unit();
    bool ret = false;

    bool strictMath = true;
    if (fast_relaxed_math || !OCL_STRICT_CONFORMANCE)
      strictMath = false;

    if (llvmToGen(*unit, module, optLevel, strictMath, OCL_PROFILING_LOG, error) == false) {
      delete unit;
      return false;
    }
    //If unit is not valid, maybe some thing don't support by backend, introduce by some passes
    //use optLevel 0 to try again.
    if(!unit->getValid()) {
      delete unit;   //clear unit
      unit = new ir::Unit();
      //suppose file exists and llvmToGen will not return false.
      llvmToGen(*unit, module, 0, strictMath, OCL_PROFILING_LOG, error);
    }
    if(unit->getValid()){
      std::string error2;
      if (this->buildFromUnit(*unit, error2)){
        ret = true;
      }
      error = error + error2;
    }
    delete unit;
    return ret;
  }

  bool Program::buildFromUnit(const ir::Unit &unit, std::string &error) {
    constantSet = new ir::ConstantSet(unit.getConstantSet());
    relocTable = new ir::RelocTable(unit.getRelocTable());
    blockFuncs = unit.blockFuncs;
    const auto &set = unit.getFunctionSet();
    const uint32_t kernelNum = set.size();
    if (OCL_OUTPUT_GEN_IR) std::cout << unit;
    if (kernelNum == 0) return true;

    bool strictMath = true;
    if (fast_relaxed_math || !OCL_STRICT_CONFORMANCE)
      strictMath = false;

    for (const auto &pair : set) {
      const std::string &name = pair.first;
      Kernel *kernel = this->compileKernel(unit, name, !strictMath, OCL_PROFILING_LOG);
      if (!kernel) {
        error +=  name;
        error += ":(GBE): error: failed in Gen backend.\n";
        if (OCL_OUTPUT_BUILD_LOG)
          llvm::errs() << error;
        return false;
      }
      kernel->setSamplerSet(pair.second->getSamplerSet());
      kernel->setProfilingInfo(new ir::ProfilingInfo(*unit.getProfilingInfo()));
      kernel->setImageSet(pair.second->getImageSet());
      kernel->setPrintfSet(pair.second->getPrintfSet());
      kernel->setCompileWorkGroupSize(pair.second->getCompileWorkGroupSize());
      kernel->setFunctionAttributes(pair.second->getFunctionAttributes());
      kernels.insert(std::make_pair(name, kernel));
    }
    return true;
  }
#endif

#define OUT_UPDATE_SZ(elt) SERIALIZE_OUT(elt, outs, ret_size)
#define IN_UPDATE_SZ(elt) DESERIALIZE_IN(elt, ins, total_size)

  uint32_t Program::serializeToBin(std::ostream& outs) {
    uint32_t ret_size = 0;
    uint32_t ker_num = kernels.size();
    uint32_t has_constset = 0;
    uint32_t has_relocTable = 0;

    OUT_UPDATE_SZ(magic_begin);

    if (constantSet) {
      has_constset = 1;
      OUT_UPDATE_SZ(has_constset);
      uint32_t sz = constantSet->serializeToBin(outs);
      if (!sz)
        return 0;

      ret_size += sz;
    } else {
      OUT_UPDATE_SZ(has_constset);
    }

    if(relocTable) {
      has_relocTable = 1;
      OUT_UPDATE_SZ(has_relocTable);
      uint32_t sz = relocTable->serializeToBin(outs);
      if (!sz)
        return 0;

      ret_size += sz;
    } else {
      OUT_UPDATE_SZ(has_relocTable);
    }

    OUT_UPDATE_SZ(ker_num);
    for (map<std::string, Kernel*>::iterator it = kernels.begin(); it != kernels.end(); ++it) {
      uint32_t sz = it->second->serializeToBin(outs);
      if (!sz)
        return 0;

      ret_size += sz;
    }

    OUT_UPDATE_SZ(magic_end);

    OUT_UPDATE_SZ(ret_size);
    return ret_size;
  }

  uint32_t Program::deserializeFromBin(std::istream& ins) {
    uint32_t total_size = 0;
    int has_constset = 0;
    uint32_t ker_num;
    uint32_t magic;
    uint32_t has_relocTable = 0;

    IN_UPDATE_SZ(magic);
    if (magic != magic_begin)
      return 0;

    IN_UPDATE_SZ(has_constset);
    if(has_constset) {
      constantSet = new ir::ConstantSet;
      uint32_t sz = constantSet->deserializeFromBin(ins);

      if (sz == 0)
        return 0;

      total_size += sz;
    }

    IN_UPDATE_SZ(has_relocTable);
    if(has_relocTable) {
      relocTable = new ir::RelocTable;
      uint32_t sz = relocTable->deserializeFromBin(ins);

      if (sz == 0)
        return 0;

      total_size += sz;
    }

    IN_UPDATE_SZ(ker_num);

    for (uint32_t i = 0; i < ker_num; i++) {
      uint32_t ker_serial_sz;
      std::string ker_name; // Just a empty name here.
      Kernel* ker = allocateKernel(ker_name);

      if(!(ker_serial_sz = ker->deserializeFromBin(ins)))
        return 0;

      kernels.insert(std::make_pair(ker->getName(), ker));
      total_size += ker_serial_sz;
    }

    IN_UPDATE_SZ(magic);
    if (magic != magic_end)
      return 0;

    uint32_t total_bytes;
    IN_UPDATE_SZ(total_bytes);
    if (total_bytes + sizeof(total_size) != total_size)
      return 0;

    return total_size;
  }

  uint32_t Kernel::serializeToBin(std::ostream& outs) {
    unsigned int i;
    uint32_t ret_size = 0;
    int has_samplerset = 0;
    int has_imageset = 0;
    uint32_t sz = 0;

    OUT_UPDATE_SZ(magic_begin);

    sz = name.size();
    OUT_UPDATE_SZ(sz);
    outs.write(name.c_str(), name.size());
    ret_size += sizeof(char)*name.size();

    OUT_UPDATE_SZ(oclVersion);

    OUT_UPDATE_SZ(argNum);
    for (i = 0; i < argNum; i++) {
      KernelArgument& arg = args[i];
      OUT_UPDATE_SZ(arg.type);
      OUT_UPDATE_SZ(arg.size);
      OUT_UPDATE_SZ(arg.align);
      OUT_UPDATE_SZ(arg.bti);

      OUT_UPDATE_SZ(arg.info.addrSpace);

      sz = arg.info.typeName.size();
      OUT_UPDATE_SZ(sz);
      outs.write(arg.info.typeName.c_str(), arg.info.typeName.size());
      ret_size += sizeof(char)*arg.info.typeName.size();

      sz = arg.info.accessQual.size();
      OUT_UPDATE_SZ(sz);
      outs.write(arg.info.accessQual.c_str(), arg.info.accessQual.size());
      ret_size += sizeof(char)*arg.info.accessQual.size();

      sz = arg.info.typeQual.size();
      OUT_UPDATE_SZ(sz);
      outs.write(arg.info.typeQual.c_str(), arg.info.typeQual.size());
      ret_size += sizeof(char)*arg.info.typeQual.size();

      sz = arg.info.argName.size();
      OUT_UPDATE_SZ(sz);
      outs.write(arg.info.argName.c_str(), arg.info.argName.size());
      ret_size += sizeof(char)*arg.info.argName.size();
    }

    sz = patches.size();
    OUT_UPDATE_SZ(sz);
    for (uint32_t i = 0; i < patches.size(); ++i) {
      const PatchInfo& patch = patches[i];
      unsigned int tmp;
      tmp = patch.type;
      OUT_UPDATE_SZ(tmp);
      tmp = patch.subType;
      OUT_UPDATE_SZ(tmp);
      tmp = patch.offset;
      OUT_UPDATE_SZ(tmp);
    }

    OUT_UPDATE_SZ(curbeSize);
    OUT_UPDATE_SZ(simdWidth);
    OUT_UPDATE_SZ(stackSize);
    OUT_UPDATE_SZ(scratchSize);
    OUT_UPDATE_SZ(useSLM);
    OUT_UPDATE_SZ(slmSize);
    OUT_UPDATE_SZ(compileWgSize[0]);
    OUT_UPDATE_SZ(compileWgSize[1]);
    OUT_UPDATE_SZ(compileWgSize[2]);
    /* samplers. */
    if (!samplerSet->empty()) {   //samplerSet is always valid, allocated in Function::Function
      has_samplerset = 1;
      OUT_UPDATE_SZ(has_samplerset);
      uint32_t sz = samplerSet->serializeToBin(outs);
      if (!sz)
        return 0;

      ret_size += sz;
    } else {
      OUT_UPDATE_SZ(has_samplerset);
    }

    /* images. */
    if (!imageSet->empty()) {   //imageSet is always valid, allocated in Function::Function
      has_imageset = 1;
      OUT_UPDATE_SZ(has_imageset);
      uint32_t sz = imageSet->serializeToBin(outs);
      if (!sz)
        return 0;

      ret_size += sz;
    } else {
      OUT_UPDATE_SZ(has_imageset);
    }

    /* Code. */
    const char * code = getCode();
    OUT_UPDATE_SZ(getCodeSize());
    outs.write(code, getCodeSize()*sizeof(char));
    ret_size += getCodeSize()*sizeof(char);

    OUT_UPDATE_SZ(magic_end);

    OUT_UPDATE_SZ(ret_size);
    return ret_size;
  }

  uint32_t Kernel::deserializeFromBin(std::istream& ins) {
    uint32_t total_size = 0;
    int has_samplerset = 0;
    int has_imageset = 0;
    uint32_t code_size = 0;
    uint32_t magic = 0;
    uint32_t patch_num = 0;

    IN_UPDATE_SZ(magic);
    if (magic != magic_begin)
      return 0;

    uint32_t name_len;
    IN_UPDATE_SZ(name_len);
    char* c_name = new char[name_len+1];
    ins.read(c_name, name_len*sizeof(char));
    total_size += sizeof(char)*name_len;
    c_name[name_len] = 0;
    name = c_name;
    delete[] c_name;

    IN_UPDATE_SZ(oclVersion);

    IN_UPDATE_SZ(argNum);
    args = GBE_NEW_ARRAY_NO_ARG(KernelArgument, argNum);
    for (uint32_t i = 0; i < argNum; i++) {
      KernelArgument& arg = args[i];
      IN_UPDATE_SZ(arg.type);
      IN_UPDATE_SZ(arg.size);
      IN_UPDATE_SZ(arg.align);
      IN_UPDATE_SZ(arg.bti);

      IN_UPDATE_SZ(arg.info.addrSpace);

      uint32_t len;
      char* a_name = NULL;

      IN_UPDATE_SZ(len);
      a_name = new char[len+1];
      ins.read(a_name, len*sizeof(char));
      total_size += sizeof(char)*len;
      a_name[len] = 0;
      arg.info.typeName = a_name;
      delete[] a_name;

      IN_UPDATE_SZ(len);
      a_name = new char[len+1];
      ins.read(a_name, len*sizeof(char));
      total_size += sizeof(char)*len;
      a_name[len] = 0;
      arg.info.accessQual = a_name;
      delete[] a_name;

      IN_UPDATE_SZ(len);
      a_name = new char[len+1];
      ins.read(a_name, len*sizeof(char));
      total_size += sizeof(char)*len;
      a_name[len] = 0;
      arg.info.typeQual = a_name;
      delete[] a_name;

      IN_UPDATE_SZ(len);
      a_name = new char[len+1];
      ins.read(a_name, len*sizeof(char));
      total_size += sizeof(char)*len;
      a_name[len] = 0;
      arg.info.argName = a_name;
      delete[] a_name;
    }

    IN_UPDATE_SZ(patch_num);
    for (uint32_t i = 0; i < patch_num; i++) {
      unsigned int tmp;
      PatchInfo patch;
      IN_UPDATE_SZ(tmp);
      patch.type = tmp;
      IN_UPDATE_SZ(tmp);
      patch.subType = tmp;
      IN_UPDATE_SZ(tmp);
      patch.offset = tmp;

      patches.push_back(patch);
    }

    IN_UPDATE_SZ(curbeSize);
    IN_UPDATE_SZ(simdWidth);
    IN_UPDATE_SZ(stackSize);
    IN_UPDATE_SZ(scratchSize);
    IN_UPDATE_SZ(useSLM);
    IN_UPDATE_SZ(slmSize);
    IN_UPDATE_SZ(compileWgSize[0]);
    IN_UPDATE_SZ(compileWgSize[1]);
    IN_UPDATE_SZ(compileWgSize[2]);

    IN_UPDATE_SZ(has_samplerset);
    if (has_samplerset) {
      samplerSet = GBE_NEW(ir::SamplerSet);
      uint32_t sz = samplerSet->deserializeFromBin(ins);
      if (sz == 0) {
        return 0;
      }

      total_size += sz;
    }
    else
      samplerSet = NULL;

    IN_UPDATE_SZ(has_imageset);
    if (has_imageset) {
      imageSet = GBE_NEW(ir::ImageSet);
      uint32_t sz = imageSet->deserializeFromBin(ins);
      if (sz == 0) {
        return 0;
      }

      total_size += sz;
    }
    else
      imageSet = NULL;

    IN_UPDATE_SZ(code_size);
    if (code_size) {
      char* code = GBE_NEW_ARRAY_NO_ARG(char, code_size);
      ins.read(code, code_size*sizeof(char));
      total_size += sizeof(char)*code_size;
      setCode(code, code_size);
    }

    IN_UPDATE_SZ(magic);
    if (magic != magic_end)
      return 0;

    uint32_t total_bytes;
    IN_UPDATE_SZ(total_bytes);
    if (total_bytes + sizeof(total_size) != total_size)
      return 0;

    return total_size;
  }

#undef OUT_UPDATE_SZ
#undef IN_UPDATE_SZ

  void Program::printStatus(int indent, std::ostream& outs) {
    using namespace std;
    string spaces = indent_to_str(indent);

    outs << spaces << "=============== Begin Program ===============" << "\n";

    if (constantSet) {
      constantSet->printStatus(indent + 4, outs);
    }

    for (map<std::string, Kernel*>::iterator it = kernels.begin(); it != kernels.end(); ++it) {
      it->second->printStatus(indent + 4, outs);
    }

    outs << spaces << "================ End Program ================" << "\n";
  }

  void Kernel::printStatus(int indent, std::ostream& outs) {
    using namespace std;
    string spaces = indent_to_str(indent);
    string spaces_nl = indent_to_str(indent + 4);
    int num;

    outs << spaces << "+++++++++++ Begin Kernel +++++++++++" << "\n";
    outs << spaces_nl << "Kernel Name: " << name << "\n";
    outs << spaces_nl << "  curbeSize: " << curbeSize << "\n";
    outs << spaces_nl << "  simdWidth: " << simdWidth << "\n";
    outs << spaces_nl << "  stackSize: " << stackSize << "\n";
    outs << spaces_nl << "  scratchSize: " << scratchSize << "\n";
    outs << spaces_nl << "  useSLM: " << useSLM << "\n";
    outs << spaces_nl << "  slmSize: " << slmSize << "\n";
    outs << spaces_nl << "  compileWgSize: " << compileWgSize[0] << compileWgSize[1] << compileWgSize[2] << "\n";

    outs << spaces_nl << "  Argument Number is " << argNum << "\n";
    for (uint32_t i = 0; i < argNum; i++) {
      KernelArgument& arg = args[i];
      outs << spaces_nl << "  Arg " << i << ":\n";
      outs << spaces_nl << "      type value: "<< arg.type << "\n";
      outs << spaces_nl << "      size: "<< arg.size << "\n";
      outs << spaces_nl << "      align: "<< arg.align << "\n";
      outs << spaces_nl << "      bti: "<< arg.bti << "\n";
    }

    outs << spaces_nl << "  Patches Number is " << patches.size() << "\n";
    num = 0;
    for (size_t i = 0; i < patches.size(); ++i) {
      PatchInfo& patch = patches[i];
      num++;
      outs << spaces_nl << "  patch " << num << ":\n";
      outs << spaces_nl << "      type value: "<< patch.type << "\n";
      outs << spaces_nl << "      subtype value: "<< patch.subType << "\n";
      outs << spaces_nl << "      offset: "<< patch.offset << "\n";
    }

    if (samplerSet) {
      samplerSet->printStatus(indent + 4, outs);
    }

    if (imageSet) {
      imageSet->printStatus(indent + 4, outs);
    }

    outs << spaces << "++++++++++++ End Kernel ++++++++++++" << "\n";
  }

  /*********************** End of Program class member function *************************/

  static void programDelete(gbe_program gbeProgram) {
    gbe::Program *program = (gbe::Program*)(gbeProgram);
    GBE_SAFE_DELETE(program);
  }

  static void programCleanLlvmResource(gbe_program gbeProgram) {
    gbe::Program *program = (gbe::Program*)(gbeProgram);
    program->CleanLlvmResource();
  }

  BVAR(OCL_DEBUGINFO, false);
#ifdef GBE_COMPILER_AVAILABLE
  static bool buildModuleFromSource(const char *source, llvm::Module** out_module, llvm::LLVMContext* llvm_ctx,
                                    std::string dumpLLVMFileName, std::string dumpSPIRBinaryName, std::vector<std::string>& options, size_t stringSize, char *err,
                                    size_t *errSize, uint32_t oclVersion) {
    // Arguments to pass to the clang frontend
    vector<const char *> args;
    bool bFastMath = false;

    for (auto &s : options) {
      args.push_back(s.c_str());
    }

    args.push_back("-cl-kernel-arg-info");
    // The ParseCommandLineOptions used for mllvm args can not be used with multithread
    // and GVN now have a 100 inst limit on block scan. Now only pass a bigger limit
    // for each context only once, this can also fix multithread bug.
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
    static bool ifsetllvm = false;
    if(!ifsetllvm) {
      args.push_back("-mllvm");
      args.push_back("-memdep-block-scan-limit=200");
      ifsetllvm = true;
    }
#endif
#ifdef GEN7_SAMPLER_CLAMP_BORDER_WORKAROUND
    args.push_back("-DGEN7_SAMPLER_CLAMP_BORDER_WORKAROUND");
#endif
    args.push_back("-emit-llvm");
    // FIXME we haven't implement those builtin functions,
    // so disable it currently.
    args.push_back("-fno-builtin");
    args.push_back("-disable-llvm-optzns");
    if(bFastMath)
      args.push_back("-D __FAST_RELAXED_MATH__=1");
    args.push_back("-x");
    args.push_back("cl");
    args.push_back("-triple");
    if (oclVersion >= 200) {
      args.push_back("spir64");
      args.push_back("-fblocks");
    } else
      args.push_back("spir");
    args.push_back("stringInput.cl");
    args.push_back("-ffp-contract=on");
    if(OCL_DEBUGINFO) args.push_back("-g");

    // The compiler invocation needs a DiagnosticsEngine so it can report problems
    std::string ErrorString;
    llvm::raw_string_ostream ErrorInfo(ErrorString);
    llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions();
    DiagOpts->ShowCarets = false;
    DiagOpts->ShowPresumedLoc = true;

  
    clang::TextDiagnosticPrinter *DiagClient =
                             new clang::TextDiagnosticPrinter(ErrorInfo, &*DiagOpts);
    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());
    clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

    llvm::StringRef srcString(source);
    // Create the compiler invocation
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
    auto CI = std::make_shared<clang::CompilerInvocation>();
    CI->getPreprocessorOpts().addRemappedFile("stringInput.cl",
#else
    std::unique_ptr<clang::CompilerInvocation> CI(new clang::CompilerInvocation);
    (*CI).getPreprocessorOpts().addRemappedFile("stringInput.cl",
#endif
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR <= 35
                llvm::MemoryBuffer::getMemBuffer(srcString)
#else
                llvm::MemoryBuffer::getMemBuffer(srcString).release()
#endif
                );

    clang::CompilerInvocation::CreateFromArgs(*CI,
                                              &args[0],
                                              &args[0] + args.size(),
                                              Diags);
    // Create the compiler instance
    clang::CompilerInstance Clang;
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
    Clang.setInvocation(std::move(CI));
#else
    Clang.setInvocation(CI.release());
#endif
    // Get ready to report problems
    Clang.createDiagnostics(DiagClient, false);

    Clang.getDiagnosticOpts().ShowCarets = false;
    if (!Clang.hasDiagnostics())
      return false;

    // Set Language
    clang::LangOptions & lang_opts = Clang.getLangOpts();
    lang_opts.OpenCL = 1;
    
    //llvm flags need command line parsing to take effect
    if (!Clang.getFrontendOpts().LLVMArgs.empty()) {
      unsigned NumArgs = Clang.getFrontendOpts().LLVMArgs.size();
      const char **Args = new const char*[NumArgs + 2];
      Args[0] = "clang (LLVM option parsing)";
      for (unsigned i = 0; i != NumArgs; ++i){
        Args[i + 1] = Clang.getFrontendOpts().LLVMArgs[i].c_str();
      }
      Args[NumArgs + 1] = 0;
      llvm::cl::ParseCommandLineOptions(NumArgs + 1, Args);
      delete [] Args;
    }
  
    // Create an action and make the compiler instance carry it out
    std::unique_ptr<clang::CodeGenAction> Act(new clang::EmitLLVMOnlyAction(llvm_ctx));
    
    auto retVal = Clang.ExecuteAction(*Act);

    if (err != NULL) {
      GBE_ASSERT(errSize != NULL);
      *errSize = ErrorString.copy(err, stringSize - 1, 0);
    }
  
    if (err == NULL || OCL_OUTPUT_BUILD_LOG) {
      // flush the error messages to the errs() if there is no
      // error string buffer.
      llvm::errs() << ErrorString;
    }
    ErrorString.clear();
    if (!retVal)
      return false;

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR <= 35
    llvm::Module *module = Act->takeModule();
#else
    llvm::Module *module = Act->takeModule().release();
#endif

    *out_module = module;

// Dump the LLVM if requested.
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR < 36
    if (!dumpLLVMFileName.empty()) {
      std::string err;
      llvm::raw_fd_ostream ostream (dumpLLVMFileName.c_str(),
                                    err,
                                    llvm::sys::fs::F_None
                                    );

      if (err.empty()) {
        (*out_module)->print(ostream, 0);
      } //Otherwise, you'll have to make do without the dump.
    }

    if (!dumpSPIRBinaryName.empty()) {
      std::string err;
      llvm::raw_fd_ostream ostream (dumpSPIRBinaryName.c_str(),
                                    err,
                                    llvm::sys::fs::F_None
                                    );
      if (err.empty())
        llvm::WriteBitcodeToFile(*out_module, ostream);
    }
#else
    if (!dumpLLVMFileName.empty()) {
      std::error_code err;
      llvm::raw_fd_ostream ostream (dumpLLVMFileName.c_str(),
                                    err, llvm::sys::fs::F_None);
      if (!err) {
        (*out_module)->print(ostream, 0);
      } //Otherwise, you'll have to make do without the dump.
    }

    if (!dumpSPIRBinaryName.empty()) {
      std::error_code err;
      llvm::raw_fd_ostream ostream (dumpSPIRBinaryName.c_str(),
                                    err, llvm::sys::fs::F_None);
      if (!err)
#if LLVM_VERSION_MAJOR<7
        llvm::WriteBitcodeToFile(*out_module, ostream);
#else
        llvm::WriteBitcodeToFile(**out_module, ostream);
#endif
    }
#endif
    return true;
  }


  SVAR(OCL_PCH_PATH, OCL_PCH_OBJECT);
  SVAR(OCL_PCH_20_PATH, OCL_PCH_OBJECT_20);
  SVAR(OCL_HEADER_FILE_DIR, OCL_HEADER_DIR);
  BVAR(OCL_OUTPUT_KERNEL_SOURCE, false);

  static bool processSourceAndOption(const char *source,
                                     const char *options,
                                     const char *temp_header_path,
                                     std::vector<std::string>& clOpt,
                                     std::string& dumpLLVMFileName,
                                     std::string& dumpASMFileName,
                                     std::string& dumpSPIRBinaryName,
                                     int& optLevel,
                                     size_t stringSize,
                                     char *err,
                                     size_t *errSize,
                                     uint32_t &oclVersion)
  {
    uint32_t maxoclVersion = oclVersion;
    std::string pchFileName;
    bool findPCH = false;
#if defined(__ANDROID__)
    bool invalidPCH = true;
#else
    bool invalidPCH = false;
#endif
    size_t start = 0, end = 0;

    std::string hdirs = OCL_HEADER_FILE_DIR;
    if(hdirs == "")
      hdirs = OCL_HEADER_DIR;
    std::istringstream hidirs(hdirs);
    std::string headerFilePath;
    bool findOcl = false;

    while (getline(hidirs, headerFilePath, ':')) {
      std::string oclDotHName = headerFilePath + "/ocl.h";
      if(access(oclDotHName.c_str(), R_OK) == 0) {
        findOcl = true;
        break;
      }
    }
    (void) findOcl;
    assert(findOcl);
    if (OCL_OUTPUT_KERNEL_SOURCE) {
      if(options) {
        std::cout << "Build options:" << std::endl;
        std::cout << options << std::endl;
      }
      std::cout << "CL kernel source:" << std::endl;
      std::cout << source << std::endl;
    }
    std::string includePath  = "-I" + headerFilePath;
    clOpt.push_back(includePath);
    bool useDefaultCLCVersion = true;

    if (options) {
      char *c_str = (char *)malloc(sizeof(char) * (strlen(options) + 1));
      if (c_str == NULL)
        return false;
      memcpy(c_str, options, strlen(options) + 1);
      std::string optionStr(c_str);
      const std::string unsupportedOptions("-cl-denorms-are-zero, -cl-strict-aliasing, -cl-opt-disable,"
                       "-cl-no-signed-zeros, -cl-fp32-correctly-rounded-divide-sqrt");

      const std::string uncompatiblePCHOptions = ("-cl-single-precision-constant, -cl-fast-relaxed-math, -cl-std=CL1.1, -cl-finite-math-only, -cl-unsafe-math-optimizations");
      const std::string fastMathOption = ("-cl-fast-relaxed-math");
      while (end != std::string::npos) {
        end = optionStr.find(' ', start);
        std::string str = optionStr.substr(start, end - start);

        if(str.size() == 0) {
          start = end + 1;
          continue;
        }

EXTEND_QUOTE:
        /* We need to find the ", if the there are odd number of " within this string,
           we need to extend the string to the matched " of the last one. */
        int quoteNum = 0;
        for (size_t i = 0; i < str.size(); i++) {
          if (str[i] == '"') {
            quoteNum++;
          }
        }

        if (quoteNum % 2) { // Odd number of ", need to extend the string.
          /* find the second " */
          while (end < optionStr.size() && optionStr[end] != '"')
            end++;

          if (end == optionStr.size()) {
            printf("Warning: Unmatched \" number in build option\n");
            free(c_str);
            return false;
          }

          GBE_ASSERT(optionStr[end] == '"');
          end++;

          if (end < optionStr.size() && optionStr[end] != ' ') {
            // "CC AAA"BBDDDD case, need to further extend.
            end = optionStr.find(' ', end);
            str = optionStr.substr(start, end - start);
            goto EXTEND_QUOTE;
          } else {
            str = optionStr.substr(start, end - start);
          }
        }
        start = end + 1;

        if(unsupportedOptions.find(str) != std::string::npos) {
          continue;
        }

        /* if -I, we need to extract "path" to path, no " */
        if (clOpt.back() == "-I") {
          if (str[0] == '"') {
            GBE_ASSERT(str[str.size() - 1] == '"');
            if (str.size() > 2) {
              clOpt.push_back(str.substr(1, str.size() - 2));
            } else {
              clOpt.push_back("");
            }
            continue;
          }
        }
        // The -I"YYYY" like case.
        if (str.size() > 4 && str[0] == '-' && str[1] == 'I' && str[2] == '"') {
          GBE_ASSERT(str[str.size() - 1] == '"');
          clOpt.push_back("-I");
          if (str.size() > 4) {
            clOpt.push_back(str.substr(3, str.size() - 4));
          } else {
            clOpt.push_back("");
          }
          continue;
        }

        if(str.find("-cl-std=") != std::string::npos) {
          useDefaultCLCVersion = false;
          if (str == "-cl-std=CL1.1") {
            clOpt.push_back("-D__OPENCL_C_VERSION__=110");
            oclVersion = 110;
          } else if (str == "-cl-std=CL1.2") {
            clOpt.push_back("-D__OPENCL_C_VERSION__=120");
            oclVersion = 120;
          } else if (str == "-cl-std=CL2.0") {
            clOpt.push_back("-D__OPENCL_C_VERSION__=200");
            oclVersion = 200;
          } else {
            if (err && stringSize > 0 && errSize)
              *errSize = snprintf(err, stringSize, "Invalid build option: %s\n", str.c_str());
            return false;
          }
        }

        if (uncompatiblePCHOptions.find(str) != std::string::npos)
          invalidPCH = true;

        if (fastMathOption.find(str) != std::string::npos) {
          clOpt.push_back("-D");
          clOpt.push_back("__FAST_RELAXED_MATH__=1");
        }

        if(str.find("-dump-opt-llvm=") != std::string::npos) {
          dumpLLVMFileName = str.substr(str.find("=") + 1);
          continue; // Don't push this str back; ignore it.
        }

        if(str.find("-dump-opt-asm=") != std::string::npos) {
          dumpASMFileName = str.substr(str.find("=") + 1);
          continue; // Don't push this str back; ignore it.
        }

        if(str.find("-dump-spir-binary=") != std::string::npos) {
          dumpSPIRBinaryName = str.substr(str.find("=") + 1);
          continue; // Don't push this str back; ignore it.
        }

        clOpt.push_back(str);
      }
      free(c_str);
    }

    if (useDefaultCLCVersion) {
      clOpt.push_back("-D__OPENCL_C_VERSION__=120");
      clOpt.push_back("-cl-std=CL1.2");
      oclVersion = 120;
    }
    //for clCompilerProgram usage.
    if(temp_header_path){
      clOpt.push_back("-I");
      clOpt.push_back(temp_header_path);
    }

    std::string dirs = OCL_PCH_PATH;
    if(oclVersion >= 200)
      dirs = OCL_PCH_20_PATH;
    if(dirs == "") {
      dirs = oclVersion >= 200 ? OCL_PCH_OBJECT_20 : OCL_PCH_OBJECT;
    }
    std::istringstream idirs(dirs);

    while (getline(idirs, pchFileName, ':')) {
      if(access(pchFileName.c_str(), R_OK) == 0) {
        findPCH = true;
        break;
      }
    }

    if (!findPCH || invalidPCH) {
      clOpt.push_back("-include");
      clOpt.push_back("ocl.h");
    } else {
      clOpt.push_back("-fno-validate-pch");
      clOpt.push_back("-include-pch");
      clOpt.push_back(pchFileName);
    }
    if (oclVersion > maxoclVersion){
      if (err && stringSize > 0 && errSize) {
         *errSize = snprintf(err, stringSize, "Requested OpenCL version %lf is higher than maximum supported version %lf\n", (float)oclVersion/100.0,(float)maxoclVersion/100.0);
      }
      return false;
    }
    return true;
  }

  static gbe_program programNewFromSource(uint32_t deviceID,
                                          const char *source,
                                          size_t stringSize,
                                          const char *options,
                                          char *err,
                                          size_t *errSize)
  {
    int optLevel = 1;
    std::vector<std::string> clOpt;
    std::string dumpLLVMFileName, dumpASMFileName;
    std::string dumpSPIRBinaryName;
    uint32_t oclVersion = MAX_OCLVERSION(deviceID);
    if (!processSourceAndOption(source, options, NULL, clOpt,
                                dumpLLVMFileName, dumpASMFileName, dumpSPIRBinaryName,
                                optLevel,
                                stringSize, err, errSize, oclVersion))
      return NULL;

    gbe_program p;
    // will delete the module and act in GenProgram::CleanLlvmResource().
    llvm::Module * out_module;
    llvm::LLVMContext* llvm_ctx = new llvm::LLVMContext;
    static std::mutex llvm_mutex;
    if (!llvm::llvm_is_multithreaded())
      llvm_mutex.lock();

    if (buildModuleFromSource(source, &out_module, llvm_ctx, dumpLLVMFileName, dumpSPIRBinaryName, clOpt,
                              stringSize, err, errSize, oclVersion)) {
    // Now build the program from llvm
      size_t clangErrSize = 0;
      if (err != NULL && *errSize != 0) {
        GBE_ASSERT(errSize != NULL);
        stringSize = stringSize - *errSize;
        err = err + *errSize;
        clangErrSize = *errSize;
      }

      if (!dumpASMFileName.empty()) {
        FILE *asmDumpStream = fopen(dumpASMFileName.c_str(), "w");
        if (asmDumpStream)
          fclose(asmDumpStream);
      }

      p = gbe_program_new_from_llvm(deviceID, out_module, llvm_ctx,
                                    dumpASMFileName.empty() ? NULL : dumpASMFileName.c_str(),
                                    stringSize, err, errSize, optLevel, options);
      if (err != NULL)
        *errSize += clangErrSize;
      if (OCL_OUTPUT_BUILD_LOG && options)
        llvm::errs() << "options:" << options << "\n";
      if (OCL_OUTPUT_BUILD_LOG && err && *errSize)
        llvm::errs() << err << "\n";
    } else
      p = NULL;

    if (!llvm::llvm_is_multithreaded())
      llvm_mutex.unlock();

    return p;
  }
#endif

#ifdef GBE_COMPILER_AVAILABLE

  static gbe_program programNewFromLLVMFile(uint32_t deviceID,
                                            const char *fileName,
                                            size_t string_size,
                                            char *err,
                                            size_t *err_size)
  {
    gbe_program p = NULL;
    if (fileName == NULL)
      return NULL;

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 39
    llvm::LLVMContext *c = new llvm::LLVMContext;
#else
    llvm::LLVMContext *c = &llvm::getGlobalContext();
#endif
    // Get the module from its file
    llvm::SMDiagnostic errDiag;
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 36
    llvm::Module *module = parseIRFile(fileName, errDiag, *c).release();
#else
    llvm::Module *module = ParseIRFile(fileName, errDiag, *c);
#endif

    int optLevel = 1;

    //module will be delete in programCleanLlvmResource
    p = gbe_program_new_from_llvm(deviceID, module, c, NULL,
                                  string_size, err, err_size, optLevel, NULL);
    if (OCL_OUTPUT_BUILD_LOG && err && *err_size)
      llvm::errs() << err << "\n";

    return p;
  }
#endif



#ifdef GBE_COMPILER_AVAILABLE

  static gbe_program programCompileFromSource(uint32_t deviceID,
                                          const char *source,
                                          const char *temp_header_path,
                                          size_t stringSize,
                                          const char *options,
                                          char *err,
                                          size_t *errSize)
  {
    int optLevel = 1;
    std::vector<std::string> clOpt;
    std::string dumpLLVMFileName, dumpASMFileName;
    std::string dumpSPIRBinaryName;
    uint32_t oclVersion = MAX_OCLVERSION(deviceID);
    if (!processSourceAndOption(source, options, temp_header_path, clOpt,
                                dumpLLVMFileName, dumpASMFileName, dumpSPIRBinaryName,
                                optLevel, stringSize, err, errSize, oclVersion))
      return NULL;

    gbe_program p;
    acquireLLVMContextLock();

    llvm::Module * out_module;
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 39
    llvm::LLVMContext* llvm_ctx = new llvm::LLVMContext;
#else
    llvm::LLVMContext* llvm_ctx = &llvm::getGlobalContext();
#endif

    if (buildModuleFromSource(source, &out_module, llvm_ctx, dumpLLVMFileName, dumpSPIRBinaryName, clOpt,
                              stringSize, err, errSize, oclVersion)) {
    // Now build the program from llvm
      if (err != NULL) {
        GBE_ASSERT(errSize != NULL);
        stringSize -= *errSize;
        err += *errSize;
      }

      p = gbe_program_new_gen_program(deviceID, out_module, NULL, NULL);

      if (OCL_OUTPUT_BUILD_LOG && options)
        llvm::errs() << "options:" << options << "\n";
      if (OCL_OUTPUT_BUILD_LOG && err && *errSize)
        llvm::errs() << err << "\n";
    } else
      p = NULL;
    releaseLLVMContextLock();
    return p;
  }
#endif

#ifdef GBE_COMPILER_AVAILABLE
  static bool programLinkProgram(gbe_program           dst_program,
                                 gbe_program           src_program,
                                 size_t                stringSize,
                                 char *                err,
                                 size_t *              errSize)
  {
    bool ret = 0;
    acquireLLVMContextLock();

    ret = gbe_program_link_from_llvm(dst_program, src_program, stringSize, err, errSize);

    releaseLLVMContextLock();

    if (OCL_OUTPUT_BUILD_LOG && err)
      llvm::errs() << err;
    return ret;
  }
#endif

#ifdef GBE_COMPILER_AVAILABLE
    static bool programCheckOption(const char * option)
    {
      vector<const char *> args;
      if (option == NULL) return 1;   //if NULL, return ok
      std::string s(option);
      size_t pos = s.find("-create-library");
      //clang don't accept -create-library and -enable-link-options, erase them
      if(pos != std::string::npos) {
        s.erase(pos, strlen("-create-library"));
      }
      pos = s.find("-enable-link-options");
      if(pos != std::string::npos) {
        s.erase(pos, strlen("-enable-link-options"));
      }
      pos = s.find("-dump-opt-asm");
      if(pos != std::string::npos) {
        s.erase(pos, strlen("-dump-opt-asm"));
      }
      args.push_back(s.c_str());

      // The compiler invocation needs a DiagnosticsEngine so it can report problems
      std::string ErrorString;
      llvm::raw_string_ostream ErrorInfo(ErrorString);
      llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions();
      DiagOpts->ShowCarets = false;
      DiagOpts->ShowPresumedLoc = true;

      clang::TextDiagnosticPrinter *DiagClient =
                               new clang::TextDiagnosticPrinter(ErrorInfo, &*DiagOpts);
      llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());
      clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

      // Create the compiler invocation
      std::unique_ptr<clang::CompilerInvocation> CI(new clang::CompilerInvocation);
      return clang::CompilerInvocation::CreateFromArgs(*CI,
                                                       &args[0],
                                                       &args[0] + args.size(),
                                                       Diags);
    }
#endif


  static size_t programGetGlobalConstantSize(gbe_program gbeProgram) {
    if (gbeProgram == NULL) return 0;
    const gbe::Program *program = (const gbe::Program*) gbeProgram;
    return program->getGlobalConstantSize();
  }

  static void programGetGlobalConstantData(gbe_program gbeProgram, char *mem) {
    if (gbeProgram == NULL) return;
    const gbe::Program *program = (const gbe::Program*) gbeProgram;
    program->getGlobalConstantData(mem);
  }

  static size_t programGetGlobalRelocCount(gbe_program gbeProgram) {
    if (gbeProgram == NULL) return 0;
    const gbe::Program *program = (const gbe::Program*) gbeProgram;
    return program->getGlobalRelocCount();
  }

  static void programGetGlobalRelocTable(gbe_program gbeProgram, char *mem) {
    if (gbeProgram == NULL) return;
    const gbe::Program *program = (const gbe::Program*) gbeProgram;
    program->getGlobalRelocTable(mem);
  }

  static uint32_t programGetKernelNum(gbe_program gbeProgram) {
    if (gbeProgram == NULL) return 0;
    const gbe::Program *program = (const gbe::Program*) gbeProgram;
    return program->getKernelNum();
  }

  const static char* programGetDeviceEnqueueKernelName(gbe_program gbeProgram, uint32_t index) {
    if (gbeProgram == NULL) return 0;
    const gbe::Program *program = (const gbe::Program*) gbeProgram;
    return program->getDeviceEnqueueKernelName(index);
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

  static const char *kernelGetAttributes(gbe_kernel genKernel) {
    if (genKernel == NULL) return NULL;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getFunctionAttributes();
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

  static void *kernelGetArgInfo(gbe_kernel genKernel, uint32_t argID, uint32_t value) {
    if (genKernel == NULL) return NULL;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    KernelArgument::ArgInfo* info = kernel->getArgInfo(argID);

    switch (value) {
      case GBE_GET_ARG_INFO_ADDRSPACE:
        return (void*)((unsigned long)info->addrSpace);
      case GBE_GET_ARG_INFO_TYPE:
        return (void *)(info->typeName.c_str());
      case GBE_GET_ARG_INFO_ACCESS:
        return (void *)(info->accessQual.c_str());
      case GBE_GET_ARG_INFO_TYPEQUAL:
        return (void *)(info->typeQual.c_str());
      case GBE_GET_ARG_INFO_NAME:
        return (void *)(info->argName.c_str());
      case GBE_GET_ARG_INFO_TYPESIZE:
        return (void *)((size_t)info->typeSize);
      default:
        assert(0);
    }

    return NULL;
  }

  static uint32_t kernelGetArgSize(gbe_kernel genKernel, uint32_t argID) {
    if (genKernel == NULL) return 0u;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getArgSize(argID);
  }

  static uint8_t kernelGetArgBTI(gbe_kernel genKernel, uint32_t argID) {
    if (genKernel == NULL) return 0u;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getArgBTI(argID);
  }

  static uint32_t kernelGetArgAlign(gbe_kernel genKernel, uint32_t argID) {
    if (genKernel == NULL) return 0u;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getArgAlign(argID);
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

  static int32_t kernelGetScratchSize(gbe_kernel genKernel) {
    if (genKernel == NULL) return 0;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getScratchSize();
  }

  static int32_t kernelUseSLM(gbe_kernel genKernel) {
    if (genKernel == NULL) return 0;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getUseSLM() ? 1 : 0;
  }

  static int32_t kernelGetSLMSize(gbe_kernel genKernel) {
    if (genKernel == NULL) return 0;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getSLMSize();
  }

  static size_t kernelGetSamplerSize(gbe_kernel gbeKernel) {
    if (gbeKernel == NULL) return 0;
    const gbe::Kernel *kernel = (const gbe::Kernel*) gbeKernel;
    return kernel->getSamplerSize();
  }

  static void kernelGetSamplerData(gbe_kernel gbeKernel, uint32_t *samplers) {
    if (gbeKernel == NULL) return;
    const gbe::Kernel *kernel = (const gbe::Kernel*) gbeKernel;
    kernel->getSamplerData(samplers);
  }

  static void* kernelDupProfiling(gbe_kernel gbeKernel) {
    if (gbeKernel == NULL) return NULL;
    const gbe::Kernel *kernel = (const gbe::Kernel*) gbeKernel;
    return kernel->dupProfilingInfo();
  }
  static uint32_t kernelGetProfilingBTI(gbe_kernel gbeKernel) {
    if (gbeKernel == NULL) return 0;
    const gbe::Kernel *kernel = (const gbe::Kernel*) gbeKernel;
    return kernel->getProfilingBTI();
  }
  static void kernelOutputProfiling(void *profiling_info, void* buf) {
    if (profiling_info == NULL) return;
    ir::ProfilingInfo *pi = (ir::ProfilingInfo *)profiling_info;
    return pi->outputProfilingInfo(buf);
  }
  static uint32_t kernelGetPrintfNum(void * printf_info) {
    if (printf_info == NULL) return 0;
    const ir::PrintfSet *ps = (ir::PrintfSet *)printf_info;
    return ps->getPrintfNum();
  }

  static uint32_t kernelUseDeviceEnqueue(gbe_kernel gbeKernel) {
    if (gbeKernel == NULL) return 0;
    const gbe::Kernel *kernel = (const gbe::Kernel*) gbeKernel;
    return kernel->getUseDeviceEnqueue();
  }

  static void* kernelDupPrintfSet(gbe_kernel gbeKernel) {
    if (gbeKernel == NULL) return NULL;
    const gbe::Kernel *kernel = (const gbe::Kernel*) gbeKernel;
    return kernel->dupPrintfSet();
  }

  static uint8_t kernelGetPrintfBufBTI(void * printf_info) {
    if (printf_info == NULL) return 0;
    const ir::PrintfSet *ps = (ir::PrintfSet *)printf_info;
    return ps->getBufBTI();
  }

  static void kernelReleasePrintfSet(void * printf_info) {
    if (printf_info == NULL) return;
    ir::PrintfSet *ps = (ir::PrintfSet *)printf_info;
    delete ps;
  }

  static void kernelOutputPrintf(void * printf_info, void* buf_addr)
  {
    if (printf_info == NULL) return;
    ir::PrintfSet *ps = (ir::PrintfSet *)printf_info;
    ps->outputPrintf(buf_addr);
  }

  static void kernelGetCompileWorkGroupSize(gbe_kernel gbeKernel, size_t wg_size[3]) {
    if (gbeKernel == NULL) return;
    const gbe::Kernel *kernel = (const gbe::Kernel*) gbeKernel;
    kernel->getCompileWorkGroupSize(wg_size);
  }

  static size_t kernelGetImageSize(gbe_kernel gbeKernel) {
    if (gbeKernel == NULL) return 0;
    const gbe::Kernel *kernel = (const gbe::Kernel*) gbeKernel;
    return kernel->getImageSize();
  }

  static void kernelGetImageData(gbe_kernel gbeKernel, ImageInfo *images) {
    if (gbeKernel == NULL) return;
    const gbe::Kernel *kernel = (const gbe::Kernel*) gbeKernel;
    kernel->getImageData(images);
  }

  static uint32_t kernelGetOclVersion(gbe_kernel gbeKernel) {
    if (gbeKernel == NULL) return 0;
    const gbe::Kernel *kernel = (const gbe::Kernel*) gbeKernel;
    return kernel->getOclVersion();
  }

  static uint32_t kernelGetRequiredWorkGroupSize(gbe_kernel kernel, uint32_t dim) {
    return 0u;
  }
} /* namespace gbe */

std::mutex llvm_ctx_mutex;
void acquireLLVMContextLock()
{
  llvm_ctx_mutex.lock();
}

void releaseLLVMContextLock()
{
  llvm_ctx_mutex.unlock();
}

GBE_EXPORT_SYMBOL gbe_program_new_from_source_cb *gbe_program_new_from_source = NULL;
GBE_EXPORT_SYMBOL gbe_program_new_from_llvm_file_cb *gbe_program_new_from_llvm_file = NULL;
GBE_EXPORT_SYMBOL gbe_program_compile_from_source_cb *gbe_program_compile_from_source = NULL;
GBE_EXPORT_SYMBOL gbe_program_link_program_cb *gbe_program_link_program = NULL;
GBE_EXPORT_SYMBOL gbe_program_check_opt_cb *gbe_program_check_opt = NULL;
GBE_EXPORT_SYMBOL gbe_program_new_from_binary_cb *gbe_program_new_from_binary = NULL;
GBE_EXPORT_SYMBOL gbe_program_new_from_llvm_binary_cb *gbe_program_new_from_llvm_binary = NULL;
GBE_EXPORT_SYMBOL gbe_program_serialize_to_binary_cb *gbe_program_serialize_to_binary = NULL;
GBE_EXPORT_SYMBOL gbe_program_new_from_llvm_cb *gbe_program_new_from_llvm = NULL;
GBE_EXPORT_SYMBOL gbe_program_new_gen_program_cb *gbe_program_new_gen_program = NULL;
GBE_EXPORT_SYMBOL gbe_program_link_from_llvm_cb *gbe_program_link_from_llvm = NULL;
GBE_EXPORT_SYMBOL gbe_program_build_from_llvm_cb *gbe_program_build_from_llvm = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_global_constant_size_cb *gbe_program_get_global_constant_size = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_global_constant_data_cb *gbe_program_get_global_constant_data = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_global_reloc_count_cb *gbe_program_get_global_reloc_count = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_global_reloc_table_cb *gbe_program_get_global_reloc_table = NULL;
GBE_EXPORT_SYMBOL gbe_program_clean_llvm_resource_cb *gbe_program_clean_llvm_resource = NULL;
GBE_EXPORT_SYMBOL gbe_program_delete_cb *gbe_program_delete = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_kernel_num_cb *gbe_program_get_kernel_num = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_kernel_by_name_cb *gbe_program_get_kernel_by_name = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_kernel_cb *gbe_program_get_kernel = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_device_enqueue_kernel_name_cb *gbe_program_get_device_enqueue_kernel_name = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_name_cb *gbe_kernel_get_name = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_attributes_cb *gbe_kernel_get_attributes = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_code_cb *gbe_kernel_get_code = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_code_size_cb *gbe_kernel_get_code_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_arg_num_cb *gbe_kernel_get_arg_num = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_arg_info_cb *gbe_kernel_get_arg_info = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_arg_size_cb *gbe_kernel_get_arg_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_arg_bti_cb *gbe_kernel_get_arg_bti = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_arg_type_cb *gbe_kernel_get_arg_type = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_arg_align_cb *gbe_kernel_get_arg_align = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_simd_width_cb *gbe_kernel_get_simd_width = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_curbe_offset_cb *gbe_kernel_get_curbe_offset = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_curbe_size_cb *gbe_kernel_get_curbe_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_stack_size_cb *gbe_kernel_get_stack_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_scratch_size_cb *gbe_kernel_get_scratch_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_required_work_group_size_cb *gbe_kernel_get_required_work_group_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_use_slm_cb *gbe_kernel_use_slm = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_slm_size_cb *gbe_kernel_get_slm_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_sampler_size_cb *gbe_kernel_get_sampler_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_sampler_data_cb *gbe_kernel_get_sampler_data = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_compile_wg_size_cb *gbe_kernel_get_compile_wg_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_image_size_cb *gbe_kernel_get_image_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_image_data_cb *gbe_kernel_get_image_data = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_ocl_version_cb *gbe_kernel_get_ocl_version = NULL;
GBE_EXPORT_SYMBOL gbe_output_profiling_cb *gbe_output_profiling = NULL;
GBE_EXPORT_SYMBOL gbe_dup_profiling_cb *gbe_dup_profiling = NULL;
GBE_EXPORT_SYMBOL gbe_get_profiling_bti_cb *gbe_get_profiling_bti = NULL;
GBE_EXPORT_SYMBOL gbe_get_printf_num_cb *gbe_get_printf_num = NULL;
GBE_EXPORT_SYMBOL gbe_dup_printfset_cb *gbe_dup_printfset = NULL;
GBE_EXPORT_SYMBOL gbe_get_printf_buf_bti_cb *gbe_get_printf_buf_bti = NULL;
GBE_EXPORT_SYMBOL gbe_release_printf_info_cb *gbe_release_printf_info = NULL;
GBE_EXPORT_SYMBOL gbe_output_printf_cb *gbe_output_printf = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_use_device_enqueue_cb *gbe_kernel_use_device_enqueue = NULL;

#ifdef GBE_COMPILER_AVAILABLE
namespace gbe
{
  /* Use pre-main to setup the call backs */
  struct CallBackInitializer
  {
    CallBackInitializer(void) {
      gbe_program_new_from_source = gbe::programNewFromSource;
      gbe_program_new_from_llvm_file = gbe::programNewFromLLVMFile;
      gbe_program_compile_from_source = gbe::programCompileFromSource;
      gbe_program_link_program = gbe::programLinkProgram;
      gbe_program_check_opt = gbe::programCheckOption;
      gbe_program_get_global_constant_size = gbe::programGetGlobalConstantSize;
      gbe_program_get_global_constant_data = gbe::programGetGlobalConstantData;
      gbe_program_get_global_reloc_count = gbe::programGetGlobalRelocCount;
      gbe_program_get_global_reloc_table = gbe::programGetGlobalRelocTable;
      gbe_program_clean_llvm_resource = gbe::programCleanLlvmResource;
      gbe_program_delete = gbe::programDelete;
      gbe_program_get_kernel_num = gbe::programGetKernelNum;
      gbe_program_get_device_enqueue_kernel_name = gbe::programGetDeviceEnqueueKernelName;
      gbe_program_get_kernel_by_name = gbe::programGetKernelByName;
      gbe_program_get_kernel = gbe::programGetKernel;
      gbe_kernel_get_name = gbe::kernelGetName;
      gbe_kernel_get_attributes = gbe::kernelGetAttributes;
      gbe_kernel_get_code = gbe::kernelGetCode;
      gbe_kernel_get_code_size = gbe::kernelGetCodeSize;
      gbe_kernel_get_arg_num = gbe::kernelGetArgNum;
      gbe_kernel_get_arg_info = gbe::kernelGetArgInfo;
      gbe_kernel_get_arg_size = gbe::kernelGetArgSize;
      gbe_kernel_get_arg_bti = gbe::kernelGetArgBTI;
      gbe_kernel_get_arg_type = gbe::kernelGetArgType;
      gbe_kernel_get_arg_align = gbe::kernelGetArgAlign;
      gbe_kernel_get_simd_width = gbe::kernelGetSIMDWidth;
      gbe_kernel_get_curbe_offset = gbe::kernelGetCurbeOffset;
      gbe_kernel_get_curbe_size = gbe::kernelGetCurbeSize;
      gbe_kernel_get_stack_size = gbe::kernelGetStackSize;
      gbe_kernel_get_scratch_size = gbe::kernelGetScratchSize;
      gbe_kernel_get_required_work_group_size = gbe::kernelGetRequiredWorkGroupSize;
      gbe_kernel_use_slm = gbe::kernelUseSLM;
      gbe_kernel_get_slm_size = gbe::kernelGetSLMSize;
      gbe_kernel_get_sampler_size = gbe::kernelGetSamplerSize;
      gbe_kernel_get_sampler_data = gbe::kernelGetSamplerData;
      gbe_kernel_get_compile_wg_size = gbe::kernelGetCompileWorkGroupSize;
      gbe_kernel_get_image_size = gbe::kernelGetImageSize;
      gbe_kernel_get_image_data = gbe::kernelGetImageData;
      gbe_kernel_get_ocl_version = gbe::kernelGetOclVersion;
      gbe_get_profiling_bti = gbe::kernelGetProfilingBTI;
      gbe_get_printf_num = gbe::kernelGetPrintfNum;
      gbe_dup_profiling = gbe::kernelDupProfiling;
      gbe_output_profiling = gbe::kernelOutputProfiling;
      gbe_get_printf_buf_bti = gbe::kernelGetPrintfBufBTI;
      gbe_dup_printfset = gbe::kernelDupPrintfSet;
      gbe_release_printf_info = gbe::kernelReleasePrintfSet;
      gbe_output_printf = gbe::kernelOutputPrintf;
      gbe_kernel_use_device_enqueue = gbe::kernelUseDeviceEnqueue;
      genSetupCallBacks();
    }

    ~CallBackInitializer() {
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 34
      llvm::llvm_shutdown();
#endif
    }
  };

  static CallBackInitializer cbInitializer;
} /* namespace gbe */
#endif
