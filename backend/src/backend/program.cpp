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
#include "llvm/llvm_to_gen.hpp"
#include "llvm/Config/config.h"
#include <cstring>
#include <algorithm>
#include <fstream>
#include <dlfcn.h>
#include <sstream>
#include <unistd.h>

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
#include <llvm/ADT/OwningPtr.h>
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
    name(name), args(NULL), argNum(0), curbeSize(0), stackSize(0), useSLM(false), ctx(NULL), samplerSet(NULL), imageSet(NULL)
  {}
  Kernel::~Kernel(void) {
    if(ctx) GBE_DELETE(ctx);
    if(samplerSet) GBE_DELETE(samplerSet);
    if(imageSet) GBE_DELETE(imageSet);
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

  BVAR(OCL_OUTPUT_GEN_IR, false);

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
    constantSet = new ir::ConstantSet(unit.getConstantSet());
    const auto &set = unit.getFunctionSet();
    const uint32_t kernelNum = set.size();
    if (OCL_OUTPUT_GEN_IR) std::cout << unit;
    if (kernelNum == 0) return true;
    for (const auto &pair : set) {
      const std::string &name = pair.first;
      Kernel *kernel = this->compileKernel(unit, name);
      kernel->setSamplerSet(pair.second->getSamplerSet());
      kernel->setImageSet(pair.second->getImageSet());
      kernels.insert(std::make_pair(name, kernel));
    }
    return true;
  }

  static void programDelete(gbe_program gbeProgram) {
    gbe::Program *program = (gbe::Program*)(gbeProgram);
    GBE_SAFE_DELETE(program);
  }

  static void buildModuleFromSource(const char* input, const char* output, std::string options) {
    // Arguments to pass to the clang frontend
    vector<const char *> args;
    bool bOpt = true;

    vector<std::string> useless; //hold substrings to avoid c_str free
    size_t start = 0, end = 0;
    /* clang unsupport options:
       -cl-denorms-are-zero, -cl-strict-aliasing
       -cl-no-signed-zeros, -cl-fp32-correctly-rounded-divide-sqrt
       all support options, refer to clang/include/clang/Driver/Options.inc
       Maybe can filter these options to avoid warning
    */
    while (end != std::string::npos) {
      end = options.find(' ', start);
      std::string str = options.substr(start, end - start);
      start = end + 1;
      if(str.size() == 0)
        continue;
      if(str == "-cl-opt-disable") bOpt = false;
      useless.push_back(str);
      args.push_back(str.c_str());
    }

    args.push_back("-emit-llvm");
    // XXX we haven't implement those builtin functions,
    // so disable it currently.
    args.push_back("-fno-builtin");
    if(bOpt)  args.push_back("-O3");
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
#if LLVM_VERSION_MINOR <= 1
    args.push_back("-triple");
    args.push_back("ptx32");

    clang::TextDiagnosticPrinter *DiagClient =
                             new clang::TextDiagnosticPrinter(llvm::errs(), clang::DiagnosticOptions());
    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());
    clang::DiagnosticsEngine Diags(DiagID, DiagClient);
#else
    args.push_back("-ffp-contract=off");

    llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions();
    clang::TextDiagnosticPrinter *DiagClient =
                             new clang::TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);
    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());
    clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);
#endif /* LLVM_VERSION_MINOR <= 1 */

    // Create the compiler invocation
    llvm::OwningPtr<clang::CompilerInvocation> CI(new clang::CompilerInvocation);
    clang::CompilerInvocation::CreateFromArgs(*CI,
                                              &args[0],
                                              &args[0] + args.size(),
                                              Diags);

    // Create the compiler instance
    clang::CompilerInstance Clang;
    Clang.setInvocation(CI.take());
    // Get ready to report problems
#if LLVM_VERSION_MINOR <= 2
    Clang.createDiagnostics(args.size(), &args[0]);
#else
    Clang.createDiagnostics();
#endif /* LLVM_VERSION_MINOR <= 2 */
    if (!Clang.hasDiagnostics())
      return;

    // Set Language
    clang::LangOptions & lang_opts = Clang.getLangOpts();
    lang_opts.OpenCL = 1;

    // Create an action and make the compiler instance carry it out
    llvm::OwningPtr<clang::CodeGenAction> Act(new clang::EmitLLVMOnlyAction());
    if (!Clang.ExecuteAction(*Act))
      return;

    llvm::Module *module = Act->takeModule();

    std::string ErrorInfo;
    llvm::raw_fd_ostream OS(output, ErrorInfo,llvm::raw_fd_ostream::F_Binary);
    //still write to temp file for code simply, otherwise need add another function.
    //because gbe_program_new_from_llvm also be used by cl_program_create_from_llvm, can't be removed
    //TODO: Pass module to llvmToGen, if use module, should return Act and use OwningPtr out of this funciton
    llvm::WriteBitcodeToFile(module, OS);
    OS.close();
  }

  extern std::string ocl_stdlib_str;

  BVAR(OCL_USE_PCH, true);
  static gbe_program programNewFromSource(const char *source,
                                          size_t stringSize,
                                          const char *options,
                                          char *err,
                                          size_t *errSize)
  {
    char clStr[L_tmpnam+1], llStr[L_tmpnam+1];
    const std::string clName = std::string(tmpnam_r(clStr)) + ".cl"; /* unsafe! */
    const std::string llName = std::string(tmpnam_r(llStr)) + ".ll"; /* unsafe! */
    std::string pchHeaderName;
    std::string clOpt;

    FILE *clFile = fopen(clName.c_str(), "w");
    FATAL_IF(clFile == NULL, "Failed to open temporary file");

    bool usePCH = false;

    if(options)
      clOpt += options;

    if (options || !OCL_USE_PCH) {
      /* Some building option may cause the prebuild pch header file
         not compatible with the XXX.cl source. We need rebuild all here.*/
      usePCH = false;
    } else {
      std::string dirs = PCH_OBJECT_DIR;
      std::istringstream idirs(dirs);

      while (getline(idirs, pchHeaderName, ';')) {
        if(access(pchHeaderName.c_str(), R_OK) == 0) {
          usePCH = true;
          break;
        }
      }
    }
    if (usePCH) {
      clOpt += " -include-pch ";
      clOpt += pchHeaderName;
      clOpt += " ";
    } else
      fwrite(ocl_stdlib_str.c_str(), strlen(ocl_stdlib_str.c_str()), 1, clFile);
    // Write the source to the cl file
    fwrite(source, strlen(source), 1, clFile);
    fclose(clFile);

    buildModuleFromSource(clName.c_str(), llName.c_str(), clOpt.c_str());
    remove(clName.c_str());

    // Now build the program from llvm
    gbe_program p = gbe_program_new_from_llvm(llName.c_str(), stringSize, err, errSize);
    remove(llName.c_str());
    return p;
  }

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

  static uint32_t kernelGetArgSize(gbe_kernel genKernel, uint32_t argID) {
    if (genKernel == NULL) return 0u;
    const gbe::Kernel *kernel = (const gbe::Kernel*) genKernel;
    return kernel->getArgSize(argID);
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

  static int32_t kernelSetConstBufSize(gbe_kernel genKernel, uint32_t argID, size_t sz) {
    if (genKernel == NULL) return -1;
    gbe::Kernel *kernel = (gbe::Kernel*) genKernel;
    return kernel->setConstBufSize(argID, sz);
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

  static uint32_t gbeImageBaseIndex = 0;
  static void setImageBaseIndex(uint32_t baseIdx) {
     gbeImageBaseIndex = baseIdx;
  }

  static uint32_t getImageBaseIndex() {
    return gbeImageBaseIndex;
  }

  static uint32_t kernelGetRequiredWorkGroupSize(gbe_kernel kernel, uint32_t dim) {
    return 0u;
  }
} /* namespace gbe */

GBE_EXPORT_SYMBOL gbe_program_new_from_source_cb *gbe_program_new_from_source = NULL;
GBE_EXPORT_SYMBOL gbe_program_new_from_binary_cb *gbe_program_new_from_binary = NULL;
GBE_EXPORT_SYMBOL gbe_program_new_from_llvm_cb *gbe_program_new_from_llvm = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_global_constant_size_cb *gbe_program_get_global_constant_size = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_global_constant_data_cb *gbe_program_get_global_constant_data = NULL;
GBE_EXPORT_SYMBOL gbe_program_delete_cb *gbe_program_delete = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_kernel_num_cb *gbe_program_get_kernel_num = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_kernel_by_name_cb *gbe_program_get_kernel_by_name = NULL;
GBE_EXPORT_SYMBOL gbe_program_get_kernel_cb *gbe_program_get_kernel = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_name_cb *gbe_kernel_get_name = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_code_cb *gbe_kernel_get_code = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_code_size_cb *gbe_kernel_get_code_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_arg_num_cb *gbe_kernel_get_arg_num = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_arg_size_cb *gbe_kernel_get_arg_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_arg_type_cb *gbe_kernel_get_arg_type = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_simd_width_cb *gbe_kernel_get_simd_width = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_curbe_offset_cb *gbe_kernel_get_curbe_offset = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_curbe_size_cb *gbe_kernel_get_curbe_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_stack_size_cb *gbe_kernel_get_stack_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_scratch_size_cb *gbe_kernel_get_scratch_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_set_const_buffer_size_cb *gbe_kernel_set_const_buffer_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_required_work_group_size_cb *gbe_kernel_get_required_work_group_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_use_slm_cb *gbe_kernel_use_slm = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_sampler_size_cb *gbe_kernel_get_sampler_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_sampler_data_cb *gbe_kernel_get_sampler_data = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_image_size_cb *gbe_kernel_get_image_size = NULL;
GBE_EXPORT_SYMBOL gbe_kernel_get_image_data_cb *gbe_kernel_get_image_data = NULL;
GBE_EXPORT_SYMBOL gbe_set_image_base_index_cb *gbe_set_image_base_index = NULL;
GBE_EXPORT_SYMBOL gbe_get_image_base_index_cb *gbe_get_image_base_index = NULL;

namespace gbe
{
  /* Use pre-main to setup the call backs */
  struct CallBackInitializer
  {
    CallBackInitializer(void) {
      gbe_program_new_from_source = gbe::programNewFromSource;
      gbe_program_get_global_constant_size = gbe::programGetGlobalConstantSize;
      gbe_program_get_global_constant_data = gbe::programGetGlobalConstantData;
      gbe_program_delete = gbe::programDelete;
      gbe_program_get_kernel_num = gbe::programGetKernelNum;
      gbe_program_get_kernel_by_name = gbe::programGetKernelByName;
      gbe_program_get_kernel = gbe::programGetKernel;
      gbe_kernel_get_name = gbe::kernelGetName;
      gbe_kernel_get_code = gbe::kernelGetCode;
      gbe_kernel_get_code_size = gbe::kernelGetCodeSize;
      gbe_kernel_get_arg_num = gbe::kernelGetArgNum;
      gbe_kernel_get_arg_size = gbe::kernelGetArgSize;
      gbe_kernel_get_arg_type = gbe::kernelGetArgType;
      gbe_kernel_get_simd_width = gbe::kernelGetSIMDWidth;
      gbe_kernel_get_curbe_offset = gbe::kernelGetCurbeOffset;
      gbe_kernel_get_curbe_size = gbe::kernelGetCurbeSize;
      gbe_kernel_get_stack_size = gbe::kernelGetStackSize;
      gbe_kernel_get_scratch_size = gbe::kernelGetScratchSize;
      gbe_kernel_set_const_buffer_size = gbe::kernelSetConstBufSize;
      gbe_kernel_get_required_work_group_size = gbe::kernelGetRequiredWorkGroupSize;
      gbe_kernel_use_slm = gbe::kernelUseSLM;
      gbe_kernel_get_sampler_size = gbe::kernelGetSamplerSize;
      gbe_kernel_get_sampler_data = gbe::kernelGetSamplerData;
      gbe_kernel_get_image_size = gbe::kernelGetImageSize;
      gbe_kernel_get_image_data = gbe::kernelGetImageData;
      gbe_get_image_base_index = gbe::getImageBaseIndex;
      gbe_set_image_base_index = gbe::setImageBaseIndex;
      genSetupCallBacks();
    }
  };

  static CallBackInitializer cbInitializer;
} /* namespace gbe */

