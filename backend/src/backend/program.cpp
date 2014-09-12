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
#include "ir/printf.hpp"
#include "llvm/llvm_to_gen.hpp"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/Threading.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/LLVMContext.h"
#include <cstring>
#include <algorithm>
#include <fstream>
#include <dlfcn.h>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <mutex>

/* Not defined for LLVM 3.0 */
#if !defined(LLVM_VERSION_MAJOR)
#define LLVM_VERSION_MAJOR 3
#endif /* !defined(LLVM_VERSION_MAJOR) */

/* Not defined for LLVM 3.0 */
#if !defined(LLVM_VERSION_MINOR)
#define LLVM_VERSION_MINOR 0
#endif /* !defined(LLVM_VERSION_MINOR) */

#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#if LLVM_VERSION_MINOR <= 1
#include <clang/Frontend/DiagnosticOptions.h>
#else
#include <clang/Basic/DiagnosticOptions.h>
#endif  /* LLVM_VERSION_MINOR <= 1 */
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#if LLVM_VERSION_MINOR <= 2
#include <llvm/Module.h>
#else
#include <llvm/IR/Module.h>
#endif  /* LLVM_VERSION_MINOR <= 2 */
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/raw_ostream.h>
#include "src/GBEConfig.h"

namespace gbe {

  Kernel::Kernel(const std::string &name) :
    name(name), args(NULL), argNum(0), curbeSize(0), stackSize(0), useSLM(false),
        slmSize(0), ctx(NULL), samplerSet(NULL), imageSet(NULL), printfSet(NULL) {}
  Kernel::~Kernel(void) {
    if(ctx) GBE_DELETE(ctx);
    if(samplerSet) GBE_DELETE(samplerSet);
    if(imageSet) GBE_DELETE(imageSet);
    if(printfSet) GBE_DELETE(printfSet);
    GBE_SAFE_DELETE_ARRAY(args);
  }
  int32_t Kernel::getCurbeOffset(gbe_curbe_type type, uint32_t subType) const {
    const PatchInfo patch(type, subType);
    const auto it = std::lower_bound(patches.begin(), patches.end(), patch);
    if (it == patches.end()) return -1; // nothing found
    if (patch < *it) return -1; // they are not equal
    return it->offset; // we found it!
  }

  Program::Program(void) : constantSet(NULL) {}
  Program::~Program(void) {
    for (auto &kernel : kernels) GBE_DELETE(kernel.second);
    if (constantSet) delete constantSet;
  }

#ifdef GBE_COMPILER_AVAILABLE
  BVAR(OCL_OUTPUT_GEN_IR, false);

  bool Program::buildFromLLVMFile(const char *fileName, const void* module, std::string &error, int optLevel) {
    ir::Unit *unit = new ir::Unit();
    llvm::Module * cloned_module = NULL;
    if(module){
      cloned_module = llvm::CloneModule((llvm::Module*)module);
    }
    if (llvmToGen(*unit, fileName, module, optLevel) == false) {
      if (fileName)
        error = std::string(fileName) + " not found";
      delete unit;
      return false;
    }
    //If unit is not valid, maybe some thing don't support by backend, introduce by some passes
    //use optLevel 0 to try again.
    if(!unit->getValid()) {
      delete unit;   //clear unit
      unit = new ir::Unit();
      if(cloned_module){
        llvmToGen(*unit, fileName, cloned_module, 0);  //suppose file exists and llvmToGen will not return false.
      }else{
        llvmToGen(*unit, fileName, module, 0);  //suppose file exists and llvmToGen will not return false.
      }
    }
    assert(unit->getValid());
    this->buildFromUnit(*unit, error);
    delete unit;
    if(cloned_module){
      delete (llvm::Module*) cloned_module;
    }
    return true;
  }

  BVAR(OCL_STRICT_CONFORMANCE, false);

  bool Program::buildFromUnit(const ir::Unit &unit, std::string &error) {
    constantSet = new ir::ConstantSet(unit.getConstantSet());
    const auto &set = unit.getFunctionSet();
    const uint32_t kernelNum = set.size();
    if (OCL_OUTPUT_GEN_IR) std::cout << unit;
    if (kernelNum == 0) return true;
    for (const auto &pair : set) {
      const std::string &name = pair.first;
      Kernel *kernel = this->compileKernel(unit, name, !OCL_STRICT_CONFORMANCE);
      kernel->setSamplerSet(pair.second->getSamplerSet());
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

  size_t Program::serializeToBin(std::ostream& outs) {
    size_t ret_size = 0;
    size_t ker_num = kernels.size();
    int has_constset = 0;

    OUT_UPDATE_SZ(magic_begin);

    if (constantSet) {
      has_constset = 1;
      OUT_UPDATE_SZ(has_constset);
      size_t sz = constantSet->serializeToBin(outs);
      if (!sz)
        return 0;

      ret_size += sz;
    } else {
      OUT_UPDATE_SZ(has_constset);
    }

    OUT_UPDATE_SZ(ker_num);
    for (auto ker : kernels) {
      size_t sz = ker.second->serializeToBin(outs);
      if (!sz)
        return 0;

      ret_size += sz;
    }

    OUT_UPDATE_SZ(magic_end);

    OUT_UPDATE_SZ(ret_size);
    return ret_size;
  }

  size_t Program::deserializeFromBin(std::istream& ins) {
    size_t total_size = 0;
    int has_constset = 0;
    size_t ker_num;
    uint32_t magic;

    IN_UPDATE_SZ(magic);
    if (magic != magic_begin)
      return 0;

    IN_UPDATE_SZ(has_constset);
    if(has_constset) {
      constantSet = new ir::ConstantSet;
      size_t sz = constantSet->deserializeFromBin(ins);

      if (sz == 0) {
        return 0;
      }

      total_size += sz;
    }

    IN_UPDATE_SZ(ker_num);

    for (size_t i = 0; i < ker_num; i++) {
      size_t ker_serial_sz;
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

    size_t total_bytes;
    IN_UPDATE_SZ(total_bytes);
    if (total_bytes + sizeof(total_size) != total_size)
      return 0;

    return total_size;
  }

  size_t Kernel::serializeToBin(std::ostream& outs) {
    unsigned int i;
    size_t ret_size = 0;
    int has_samplerset = 0;
    int has_imageset = 0;

    OUT_UPDATE_SZ(magic_begin);

    OUT_UPDATE_SZ(name.size());
    outs.write(name.c_str(), name.size());
    ret_size += sizeof(char)*name.size();

    OUT_UPDATE_SZ(argNum);
    for (i = 0; i < argNum; i++) {
      KernelArgument& arg = args[i];
      OUT_UPDATE_SZ(arg.type);
      OUT_UPDATE_SZ(arg.size);
      OUT_UPDATE_SZ(arg.align);
      OUT_UPDATE_SZ(arg.bti);
    }

    OUT_UPDATE_SZ(patches.size());
    for (auto patch : patches) {
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
      size_t sz = samplerSet->serializeToBin(outs);
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
      size_t sz = imageSet->serializeToBin(outs);
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

  size_t Kernel::deserializeFromBin(std::istream& ins) {
    size_t total_size = 0;
    int has_samplerset = 0;
    int has_imageset = 0;
    size_t code_size = 0;
    uint32_t magic = 0;
    size_t patch_num = 0;

    IN_UPDATE_SZ(magic);
    if (magic != magic_begin)
      return 0;

    size_t name_len;
    IN_UPDATE_SZ(name_len);
    char* c_name = new char[name_len+1];
    ins.read(c_name, name_len*sizeof(char));
    total_size += sizeof(char)*name_len;
    c_name[name_len] = 0;
    name = c_name;
    delete[] c_name;

    IN_UPDATE_SZ(argNum);
    args = GBE_NEW_ARRAY_NO_ARG(KernelArgument, argNum);
    for (uint32_t i = 0; i < argNum; i++) {
      KernelArgument& arg = args[i];
      IN_UPDATE_SZ(arg.type);
      IN_UPDATE_SZ(arg.size);
      IN_UPDATE_SZ(arg.align);
      IN_UPDATE_SZ(arg.bti);
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
      size_t sz = samplerSet->deserializeFromBin(ins);
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
      size_t sz = imageSet->deserializeFromBin(ins);
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

    size_t total_bytes;
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

    for (auto ker : kernels) {
      ker.second->printStatus(indent + 4, outs);
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
    for (auto patch : patches) {
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

#define REDEF_MATH_FUNC(x) "#ifdef "#x"\n#undef "#x"\n#endif\n#define "#x" __gen_ocl_internal_fastpath_"#x"\n"
  std::string ocl_mathfunc_fastpath_str =
    REDEF_MATH_FUNC(acosh)
    REDEF_MATH_FUNC(asinh)
    REDEF_MATH_FUNC(atanh)
    REDEF_MATH_FUNC(cbrt)
    REDEF_MATH_FUNC(cos)
    REDEF_MATH_FUNC(cosh)
    REDEF_MATH_FUNC(cospi)
    REDEF_MATH_FUNC(exp)
    REDEF_MATH_FUNC(exp10)
    REDEF_MATH_FUNC(expm1)
    REDEF_MATH_FUNC(fmod)
    REDEF_MATH_FUNC(hypot)
    REDEF_MATH_FUNC(ilogb)
    REDEF_MATH_FUNC(ldexp)
    REDEF_MATH_FUNC(log)
    REDEF_MATH_FUNC(log2)
    REDEF_MATH_FUNC(log10)
    REDEF_MATH_FUNC(log1p)
    REDEF_MATH_FUNC(logb)
    REDEF_MATH_FUNC(remainder)
    REDEF_MATH_FUNC(rootn)
    REDEF_MATH_FUNC(sin)
    REDEF_MATH_FUNC(sincos)
    REDEF_MATH_FUNC(sinh)
    REDEF_MATH_FUNC(sinpi)
    REDEF_MATH_FUNC(tan)
    REDEF_MATH_FUNC(tanh)
    "\n"
  ;

  static void programDelete(gbe_program gbeProgram) {
    gbe::Program *program = (gbe::Program*)(gbeProgram);
    GBE_SAFE_DELETE(program);
  }

  static void programCleanLlvmResource(gbe_program gbeProgram) {
    gbe::Program *program = (gbe::Program*)(gbeProgram);
    program->CleanLlvmResource();
  }

#ifdef GBE_COMPILER_AVAILABLE
  BVAR(OCL_OUTPUT_BUILD_LOG, false);
  SVAR(OCL_PCH_PATH, PCH_OBJECT_DIR);
  SVAR(OCL_PCM_PATH, PCM_OBJECT_DIR);

  static bool buildModuleFromSource(const char* input, llvm::Module** out_module, llvm::LLVMContext* llvm_ctx, std::string options,
                                    size_t stringSize, char *err, size_t *errSize) {
    // Arguments to pass to the clang frontend
    vector<const char *> args;
    bool bFastMath = false;

    vector<std::string> useless; //hold substrings to avoid c_str free
    size_t start = 0, end = 0;
    /* FIXME
       clang unsupport options:
       -cl-denorms-are-zero, -cl-strict-aliasing
       -cl-no-signed-zeros, -cl-fp32-correctly-rounded-divide-sqrt
       all support options, refer to clang/include/clang/Driver/Options.inc
    */
    //Handle -cl-opt-disable in llvmToGen, skip here
    const std::string unsupportedOptions("-cl-denorms-are-zero, -cl-strict-aliasing, -cl-opt-disable,"
                                         "-cl-no-signed-zeros, -cl-fp32-correctly-rounded-divide-sqrt");
    bool useDefaultCLCVersion = true;
    while (end != std::string::npos) {
      end = options.find(' ', start);
      std::string str = options.substr(start, end - start);
      start = end + 1;
      if(str.size() == 0)
        continue;
      if(str == "-cl-fast-relaxed-math") bFastMath = true;
      if(unsupportedOptions.find(str) != std::string::npos)
        continue;
      if(str.find("-cl-std=") != std::string::npos) {
        useDefaultCLCVersion = false;
        if (str == "-cl-std=CL1.1")
          args.push_back("-D__OPENCL_C_VERSION__=110");
        else if (str == "-cl-std=CL1.2")
          args.push_back("-D__OPENCL_C_VERSION__=120");
        else {
          if (err && stringSize > 0 && errSize)
            *errSize = snprintf(err, stringSize, "Invalid build option: %s\n", str.c_str());
          return false;
        }
      }
      useless.push_back(str);
      args.push_back(str.c_str());
    }
    if (useDefaultCLCVersion) {
      args.push_back("-D__OPENCL_C_VERSION__=120");
      args.push_back("-cl-std=CL1.2");
    }
    args.push_back("-mllvm");
    args.push_back("-inline-threshold=200000");
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
#if LLVM_VERSION_MINOR <= 2
    args.push_back("-triple");
    args.push_back("nvptx");
#else
    args.push_back("-x");
    args.push_back("cl");
    args.push_back("-triple");
    args.push_back("spir");
#endif /* LLVM_VERSION_MINOR <= 2 */
    args.push_back(input);

    // The compiler invocation needs a DiagnosticsEngine so it can report problems
    std::string ErrorString;
    llvm::raw_string_ostream ErrorInfo(ErrorString);
    llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions();
    DiagOpts->ShowCarets = false;
    DiagOpts->ShowPresumedLoc = true;
#if LLVM_VERSION_MINOR <= 1
    args.push_back("-triple");
    args.push_back("ptx32");

    clang::TextDiagnosticPrinter *DiagClient =
                             new clang::TextDiagnosticPrinter(ErrorInfo, *DiagOpts)
    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());
    clang::DiagnosticsEngine Diags(DiagID, DiagClient);
#else
    args.push_back("-ffp-contract=off");

    clang::TextDiagnosticPrinter *DiagClient =
                             new clang::TextDiagnosticPrinter(ErrorInfo, &*DiagOpts);
    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());
    clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);
#endif /* LLVM_VERSION_MINOR <= 1 */
    // Create the compiler invocation
    std::unique_ptr<clang::CompilerInvocation> CI(new clang::CompilerInvocation);
    clang::CompilerInvocation::CreateFromArgs(*CI,
                                              &args[0],
                                              &args[0] + args.size(),
                                              Diags);

    // Create the compiler instance
    clang::CompilerInstance Clang;
    Clang.setInvocation(CI.release());
    // Get ready to report problems
#if LLVM_VERSION_MINOR <= 2
    Clang.createDiagnostics(args.size(), &args[0]);
#else
    Clang.createDiagnostics(DiagClient, false);
#endif /* LLVM_VERSION_MINOR <= 2 */

    Clang.getDiagnosticOpts().ShowCarets = false;
    if (!Clang.hasDiagnostics())
      return false;

    // Set Language
    clang::LangOptions & lang_opts = Clang.getLangOpts();
    lang_opts.OpenCL = 1;

    clang::PreprocessorOptions& prep_opt = Clang.getPreprocessorOpts();
    prep_opt.DisablePCHValidation = 1;

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

    std::string dirs = OCL_PCM_PATH;
    std::string pcmFileName;
    std::istringstream idirs(dirs);
    bool findPcm = false;

    while (getline(idirs, pcmFileName, ':')) {
      if(access(pcmFileName.c_str(), R_OK) == 0) {
        findPcm |= true;
        break;
      }
    }

    GBE_ASSERT(findPcm && "Could not find pre compiled module library.\n");

    Clang.getCodeGenOpts().LinkBitcodeFile = pcmFileName;
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

    llvm::Module *module = Act->takeModule();

    *out_module = module;
    return true;
  }

  extern std::string ocl_stdlib_str;

  BVAR(OCL_USE_PCH, true);
  static void processSourceAndOption(const char *source,
                                     const char *options,
                                     const char *temp_header_path,
                                     std::string& clOpt,
                                     std::string& clName,
                                     int& optLevel)
  {
    char clStr[] = "/tmp/XXXXXX.cl";
    int clFd = mkstemps(clStr, 3);
    clName = std::string(clStr);

    FILE *clFile = fdopen(clFd, "w");
    FATAL_IF(clFile == NULL, "Failed to open temporary file");

    bool usePCH = OCL_USE_PCH;
    bool findPCH = false;

    /* Because our header file is so big, we want to avoid recompile the header from
       scratch. We use the PCH support of Clang to save the huge compiling time.
       We just use the most general build opt to build the PCH header file, so if
       user pass new build options here, the PCH can not pass the Clang's compitable
       validating. Clang will do three kinds of compatible check: Language Option,
       Target Option and Preprocessing Option. Other kinds of options such as the
       CodeGen options will not affect the AST result, so no need to check.

       According to OpenCL 1.1's spec, the CL build options:
       -D name=definition
       If the definition is not used in our header, it is compitable

       -cl-single-precision-constant
       -cl-denorms-are-zero
       -cl-std=
       Language options, really affect.

       -cl-opt-disable
       -cl-mad-enable
       -cl-no-signed-zeros
       -cl-unsafe-math-optimizations
       -cl-finite-math-only
       -cl-fast-relaxed-math
       CodeGen options, not affect

       -Werror
       -w
       Our header should not block the compiling because of warning.

       So we just disable the PCH validation of Clang and do the judgement by ourself. */

    /* We always add -cl-kernel-arg-info to the options. This option just generate the arg
       information for the backend, no other side effect and does not have performance issue. */
    if (!options || !strstr(const_cast<char *>(options), "-cl-kernel-arg-info"))
      clOpt += "-cl-kernel-arg-info ";

    if (options) {
      char *p;
      /* FIXME: Though we can disable the pch valid check, and load pch successfully,
         but these language opts and pre-defined macro will still generate the diag msg
         to the diag engine of the Clang and cause the Clang to report error.
         We filter them all here to avoid these. */
      const char * incompatible_opts[] = {
          "-cl-single-precision-constant",
//        "-cl-denorms-are-zero",
          "-cl-fast-relaxed-math",
          "-cl-std=CL1.1"
      };
      const char * incompatible_defs[] = {
          "GET_FLOAT_WORD",
          "__NV_CL_C_VERSION",
          "GEN7_SAMPLER_CLAMP_BORDER_WORKAROUND"
      };

      for (unsigned int i = 0; i < sizeof(incompatible_opts)/sizeof(char *); i++ ) {
        p = strstr(const_cast<char *>(options), incompatible_opts[i]);
        if (p) {
          usePCH = false;
          break;
        }
      }

      if (usePCH) {
        for (unsigned int i = 0; i < sizeof(incompatible_defs)/sizeof(char *); i++ ) {
          p = strstr(const_cast<char *>(options), incompatible_defs[i]);
          if (p) {
            usePCH = false;
            break;
          }
        }
      }

      p = strstr(const_cast<char *>(options), "-cl-opt-disable");
      if (p)
        optLevel = 0;
      // XXX enable cl_khr_fp64 may cause some potential bugs.
      // we may need to revisit here latter when we want to support fp64 completely.
      // For now, as we don't support fp64 actually, just disable it by default.
#if 0
      #define ENABLE_CL_KHR_FP64_STR "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
      if (!strstr(const_cast<char *>(options), "-cl-std=CL1.1"))
        fwrite(ENABLE_CL_KHR_FP64_STR, strlen(ENABLE_CL_KHR_FP64_STR), 1, clFile);
#endif

      clOpt += options;
    }

    std::string dirs = OCL_PCH_PATH;
    std::istringstream idirs(dirs);
    std::string pchFileName;

    while (getline(idirs, pchFileName, ':')) {
      if(access(pchFileName.c_str(), R_OK) == 0) {
        findPCH = true;
        break;
      }
    }

    if (usePCH && findPCH) {
      clOpt += " -include-pch ";
      clOpt += pchFileName;
      clOpt += " ";
    } else
      fwrite(ocl_stdlib_str.c_str(), strlen(ocl_stdlib_str.c_str()), 1, clFile);

    //for clCompilerProgram usage.
    if(temp_header_path){
      clOpt += " -I ";
      clOpt += temp_header_path;
      clOpt += " ";
    }

    if (!OCL_STRICT_CONFORMANCE) {
        fwrite(ocl_mathfunc_fastpath_str.c_str(), strlen(ocl_mathfunc_fastpath_str.c_str()), 1, clFile);
    }

    // reset the file number in case we have inserted something into the kernel
    std::string resetFileNum = "#line 1\n";
    fwrite(resetFileNum.c_str(), strlen(resetFileNum.c_str()), 1, clFile);

    // Write the source to the cl file
    fwrite(source, strlen(source), 1, clFile);
    fclose(clFile);
  }

  static gbe_program programNewFromSource(uint32_t deviceID,
                                          const char *source,
                                          size_t stringSize,
                                          const char *options,
                                          char *err,
                                          size_t *errSize)
  {
    int optLevel = 1;
    std::string clOpt;
    std::string clName;
    processSourceAndOption(source, options, NULL, clOpt, clName, optLevel);

    gbe_program p;
    // will delete the module and act in GenProgram::CleanLlvmResource().
    llvm::Module * out_module;
    llvm::LLVMContext* llvm_ctx = new llvm::LLVMContext;

    static std::mutex llvm_mutex;
    if (!llvm::llvm_is_multithreaded())
      llvm_mutex.lock();

    if (buildModuleFromSource(clName.c_str(), &out_module, llvm_ctx, clOpt.c_str(),
                              stringSize, err, errSize)) {
    // Now build the program from llvm
      size_t clangErrSize = 0;
      if (err != NULL) {
        GBE_ASSERT(errSize != NULL);
        stringSize -= *errSize;
        err += *errSize;
        clangErrSize = *errSize;
      }

      p = gbe_program_new_from_llvm(deviceID, NULL, out_module, llvm_ctx, stringSize,
                                    err, errSize, optLevel);
      if (err != NULL)
        *errSize += clangErrSize;
      if (OCL_OUTPUT_BUILD_LOG && options)
        llvm::errs() << options;
    } else
      p = NULL;

    if (!llvm::llvm_is_multithreaded())
      llvm_mutex.unlock();

    remove(clName.c_str());
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
    std::string clOpt;
    std::string clName;
    processSourceAndOption(source, options, temp_header_path, clOpt, clName, optLevel);

    gbe_program p;
    acquireLLVMContextLock();
    //FIXME: if use new allocated context to link two modules there would be context mismatch
    //for some functions, so we use global context now, need switch to new context later.
    llvm::Module * out_module;
    llvm::LLVMContext* llvm_ctx = &llvm::getGlobalContext();
    if (buildModuleFromSource(clName.c_str(), &out_module, llvm_ctx, clOpt.c_str(),
                              stringSize, err, errSize)) {
    // Now build the program from llvm
      if (err != NULL) {
        GBE_ASSERT(errSize != NULL);
        stringSize -= *errSize;
        err += *errSize;
      }

      p = gbe_program_new_gen_program(deviceID, out_module, NULL);

      if (OCL_OUTPUT_BUILD_LOG && options)
        llvm::errs() << options;
    } else
      p = NULL;
    remove(clName.c_str());
    releaseLLVMContextLock();
    return p;
  }
#endif

#ifdef GBE_COMPILER_AVAILABLE
  static void programLinkProgram(gbe_program           dst_program,
                                 gbe_program           src_program,
                                 size_t                stringSize,
                                 char *                err,
                                 size_t *              errSize)
  {
    acquireLLVMContextLock();

    gbe_program_link_from_llvm(dst_program, src_program, stringSize, err, errSize);

    releaseLLVMContextLock();

    if (OCL_OUTPUT_BUILD_LOG && err)
      llvm::errs() << err;
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
    ir::FunctionArgument::InfoFromLLVM* info = kernel->getArgInfo(argID);

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

  static uint32_t kernelGetPrintfNum(void * printf_info) {
    if (printf_info == NULL) return 0;
    const ir::PrintfSet *ps = (ir::PrintfSet *)printf_info;
    return ps->getPrintfNum();
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

  static uint8_t kernelGetPrintfIndexBufBTI(void * printf_info) {
    if (printf_info == NULL) return 0;
    const ir::PrintfSet *ps = (ir::PrintfSet *)printf_info;
    return ps->getIndexBufBTI();
  }

  static void kernelReleasePrintfSet(void * printf_info) {
    if (printf_info == NULL) return;
    ir::PrintfSet *ps = (ir::PrintfSet *)printf_info;
    delete ps;
  }

  static uint32_t kernelGetPrintfSizeOfSize(void * printf_info) {
    if (printf_info == NULL) return 0;
    const ir::PrintfSet *ps = (ir::PrintfSet *)printf_info;
    return ps->getPrintfSizeOfSize();
  }

  static void kernelOutputPrintf(void * printf_info, void* index_addr,
                                 void* buf_addr, size_t global_wk_sz0,
                                 size_t global_wk_sz1, size_t global_wk_sz2)
  {
    if (printf_info == NULL) return;
    ir::PrintfSet *ps = (ir::PrintfSet *)printf_info;
    ps->outputPrintf(index_addr, buf_addr, global_wk_sz0,
                         global_wk_sz1, global_wk_sz2);
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
GBE_EXPORT_SYMBOL gbe_program_compile_from_source_cb *gbe_program_compile_from_source = NULL;
GBE_EXPORT_SYMBOL gbe_program_link_program_cb *gbe_program_link_program = NULL;
GBE_EXPORT_SYMBOL gbe_program_new_from_binary_cb *gbe_program_new_from_binary = NULL;
GBE_EXPORT_SYMBOL gbe_program_new_from_llvm_binary_cb *gbe_program_new_from_llvm_binary = NULL;
GBE_EXPORT_SYMBOL gbe_program_serialize_to_binary_cb *gbe_program_serialize_to_binary = NULL;
GBE_EXPORT_SYMBOL gbe_program_new_from_llvm_cb *gbe_program_new_from_llvm = NULL;
GBE_EXPORT_SYMBOL gbe_program_new_gen_program_cb *gbe_program_new_gen_program = NULL;
GBE_EXPORT_SYMBOL gbe_program_link_from_llvm_cb *gbe_program_link_from_llvm = NULL;
GBE_EXPORT_SYMBOL gbe_program_build_from_llvm_cb *gbe_program_build_from_llvm = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_global_constant_size_cb *gbe_program_get_global_constant_size = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_global_constant_data_cb *gbe_program_get_global_constant_data = NULL;
GBE_EXPORT_SYMBOL gbe_program_clean_llvm_resource_cb *gbe_program_clean_llvm_resource = NULL;
GBE_EXPORT_SYMBOL gbe_program_delete_cb *gbe_program_delete = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_kernel_num_cb *gbe_program_get_kernel_num = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_kernel_by_name_cb *gbe_program_get_kernel_by_name = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_kernel_cb *gbe_program_get_kernel = NULL;
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
GBE_EXPORT_SYMBOL gbe_get_printf_num_cb *gbe_get_printf_num = NULL;
GBE_EXPORT_SYMBOL gbe_dup_printfset_cb *gbe_dup_printfset = NULL;
GBE_EXPORT_SYMBOL gbe_get_printf_buf_bti_cb *gbe_get_printf_buf_bti = NULL;
GBE_EXPORT_SYMBOL gbe_get_printf_indexbuf_bti_cb *gbe_get_printf_indexbuf_bti = NULL;
GBE_EXPORT_SYMBOL gbe_release_printf_info_cb *gbe_release_printf_info = NULL;
GBE_EXPORT_SYMBOL gbe_get_printf_sizeof_size_cb *gbe_get_printf_sizeof_size = NULL;
GBE_EXPORT_SYMBOL gbe_output_printf_cb *gbe_output_printf = NULL;

#ifdef GBE_COMPILER_AVAILABLE
namespace gbe
{
  /* Use pre-main to setup the call backs */
  struct CallBackInitializer
  {
    CallBackInitializer(void) {
      gbe_program_new_from_source = gbe::programNewFromSource;
      gbe_program_compile_from_source = gbe::programCompileFromSource;
      gbe_program_link_program = gbe::programLinkProgram;
      gbe_program_get_global_constant_size = gbe::programGetGlobalConstantSize;
      gbe_program_get_global_constant_data = gbe::programGetGlobalConstantData;
      gbe_program_clean_llvm_resource = gbe::programCleanLlvmResource;
      gbe_program_delete = gbe::programDelete;
      gbe_program_get_kernel_num = gbe::programGetKernelNum;
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
      gbe_get_printf_num = gbe::kernelGetPrintfNum;
      gbe_get_printf_buf_bti = gbe::kernelGetPrintfBufBTI;
      gbe_get_printf_indexbuf_bti = gbe::kernelGetPrintfIndexBufBTI;
      gbe_dup_printfset = gbe::kernelDupPrintfSet;
      gbe_get_printf_sizeof_size = gbe::kernelGetPrintfSizeOfSize;
      gbe_release_printf_info = gbe::kernelReleasePrintfSet;
      gbe_output_printf = gbe::kernelOutputPrintf;
      genSetupCallBacks();
    }

    ~CallBackInitializer() {
#if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR > 3)
      llvm::llvm_shutdown();
#endif
    }
  };

  static CallBackInitializer cbInitializer;
} /* namespace gbe */
#endif
