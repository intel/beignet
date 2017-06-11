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
 */
#include "llvm/ADT/Triple.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm-c/Linker.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/CodeGen/CodeGenAction.h"

#include "src/GBEConfig.h"
#include "backend/gen_program.hpp"
#include "llvm/llvm_to_gen.hpp"
#include "sys/cvar.hpp"

#include <sstream>
#include <iostream>
#include <unistd.h>
#include <mutex>

using namespace gbe;

SVAR(OCL_PCH_PATH, OCL_PCH_OBJECT);
SVAR(OCL_PCH_20_PATH, OCL_PCH_OBJECT_20);
SVAR(OCL_HEADER_FILE_DIR, OCL_HEADER_DIR);
BVAR(OCL_OUTPUT_KERNEL_SOURCE, false);
BVAR(OCL_DEBUGINFO, false);
BVAR(OCL_OUTPUT_BUILD_LOG, false);

static llvm::Module *
loadProgramFromLLVMIRBinary(uint32_t deviceID, const char *binary, size_t size)
{
  std::string binary_content;
  //the first byte stands for binary_type.
  if (binary[0] == 'L' && binary[1] == 'I' && binary[2] == 'B' &&
      binary[3] == 'B' && binary[4] == 'C' &&
      binary[5] == (char)0xC0 && binary[6] == (char)0xDE) {
    binary_content.assign(binary + 3, size - 3);
  } else if (binary[0] == 'B' && binary[1] == 'C' &&
             binary[2] == (char)0xC0 && binary[3] == (char)0xDE) {
    binary_content.assign(binary, size);
  } else
    return NULL;

  llvm::StringRef llvm_bin_str(binary_content);
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9
  llvm::LLVMContext &c = GBEGetLLVMContext();
#else
  llvm::LLVMContext &c = llvm::getGlobalContext();
#endif
  llvm::SMDiagnostic Err;

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 6
  std::unique_ptr<llvm::MemoryBuffer> memory_buffer = llvm::MemoryBuffer::getMemBuffer(llvm_bin_str, "llvm_bin_str");
  acquireLLVMContextLock();
  llvm::Module *module = llvm::parseIR(memory_buffer->getMemBufferRef(), Err, c).release();
#else
  llvm::MemoryBuffer *memory_buffer = llvm::MemoryBuffer::getMemBuffer(llvm_bin_str, "llvm_bin_str");
  acquireLLVMContextLock();
  llvm::Module *module = llvm::ParseIR(memory_buffer, Err, c);
#endif

  if (module == NULL)
    return NULL;

  // if load 32 bit spir binary, the triple should be spir-unknown-unknown.
  llvm::Triple triple(module->getTargetTriple());
  if (triple.getArchName() == "spir" && triple.getVendorName() == "unknown" &&
      triple.getOSName() == "unknown") {
    module->setTargetTriple("spir");
  } else if (triple.getArchName() == "spir64" && triple.getVendorName() == "unknown" &&
             triple.getOSName() == "unknown") {
    module->setTargetTriple("spir64");
  }
  releaseLLVMContextLock();

  return module;
}

static bool
processSourceAndOption(const char *source, const char *options, const char *temp_header_path,
                       std::vector<std::string> &clOpt, std::string &dumpLLVMFileName,
                       std::string &dumpASMFileName, std::string &dumpSPIRBinaryName,
                       int &optLevel, size_t stringSize, char *err, size_t *errSize,
                       uint32_t &oclVersion)
{
  std::string pchFileName;
  bool findPCH = false;
#if defined(__ANDROID__)
  bool invalidPCH = true;
#else
  bool invalidPCH = false;
#endif
  size_t start = 0, end = 0;

  std::string hdirs = OCL_HEADER_FILE_DIR;
  if (hdirs == "")
    hdirs = OCL_HEADER_DIR;
  std::istringstream hidirs(hdirs);
  std::string headerFilePath;
  bool findOcl = false;

  while (getline(hidirs, headerFilePath, ':')) {
    std::string oclDotHName = headerFilePath + "/ocl.h";
    if (access(oclDotHName.c_str(), R_OK) == 0) {
      findOcl = true;
      break;
    }
  }
  (void)findOcl;
  assert(findOcl);
  if (OCL_OUTPUT_KERNEL_SOURCE) {
    if (options) {
      std::cout << "Build options:" << std::endl;
      std::cout << options << std::endl;
    }
    std::cout << "CL kernel source:" << std::endl;
    std::cout << source << std::endl;
  }
  std::string includePath = "-I" + headerFilePath;
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

    const std::string uncompatiblePCHOptions = ("-cl-single-precision-constant, -cl-fast-relaxed-math,"
                                                " -cl-std=CL1.1, -cl-finite-math-only,"
                                                " -cl-unsafe-math-optimizations");
    const std::string fastMathOption = ("-cl-fast-relaxed-math");
    while (end != std::string::npos) {
      end = optionStr.find(' ', start);
      std::string str = optionStr.substr(start, end - start);

      if (str.size() == 0) {
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

      if (unsupportedOptions.find(str) != std::string::npos) {
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

      if (str.find("-cl-std=") != std::string::npos) {
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

      if (str.find("-dump-opt-llvm=") != std::string::npos) {
        dumpLLVMFileName = str.substr(str.find("=") + 1);
        continue; // Don't push this str back; ignore it.
      }

      if (str.find("-dump-opt-asm=") != std::string::npos) {
        dumpASMFileName = str.substr(str.find("=") + 1);
        continue; // Don't push this str back; ignore it.
      }

      if (str.find("-dump-spir-binary=") != std::string::npos) {
        dumpSPIRBinaryName = str.substr(str.find("=") + 1);
        continue; // Don't push this str back; ignore it.
      }

      clOpt.push_back(str);
    }
    free(c_str);
  }

  if (useDefaultCLCVersion) {
#ifdef ENABLE_OPENCL_20
    clOpt.push_back("-D__OPENCL_C_VERSION__=200");
    clOpt.push_back("-cl-std=CL2.0");
    oclVersion = 200;
#else
    clOpt.push_back("-D__OPENCL_C_VERSION__=120");
    clOpt.push_back("-cl-std=CL1.2");
    oclVersion = 120;
#endif
  }
  //for clCompilerProgram usage.
  if (temp_header_path) {
    clOpt.push_back("-I");
    clOpt.push_back(temp_header_path);
  }

  std::string dirs = OCL_PCH_PATH;
  if (oclVersion >= 200)
    dirs = OCL_PCH_20_PATH;
  if (dirs == "") {
    dirs = oclVersion >= 200 ? OCL_PCH_OBJECT_20 : OCL_PCH_OBJECT;
  }
  std::istringstream idirs(dirs);

  while (getline(idirs, pchFileName, ':')) {
    if (access(pchFileName.c_str(), R_OK) == 0) {
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

  return true;
}

static bool
buildLLVMModuleFromSource(const char *source, size_t src_length, const char **headers,
                          size_t *header_length, const char **header_names, int headerNum,
                          llvm::Module **out_module, llvm::LLVMContext *llvm_ctx,
                          std::string dumpLLVMFileName, std::string dumpSPIRBinaryName,
                          std::vector<std::string> &options, size_t stringSize, char *err,
                          size_t *errSize, uint32_t oclVersion)
{
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
#if LLVM_VERSION_MINOR >= 9
  static bool ifsetllvm = false;
  if (!ifsetllvm) {
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
  if (bFastMath)
    args.push_back("-D __FAST_RELAXED_MATH__=1");
  args.push_back("-x");
  args.push_back("cl");
  args.push_back("-triple");

  if (oclVersion >= 200)
    args.push_back("spir64");
  else
    args.push_back("spir");

  args.push_back("stringInput.cl");
  args.push_back("-ffp-contract=on");
  if (OCL_DEBUGINFO)
    args.push_back("-g");

  if (headers) {
    GBE_ASSERT(header_names != NULL);
    GBE_ASSERT(headerNum != 0);
    args.push_back("-I/cl/include/path/"); //addRemappedFile must find a abs file name
  }

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
  clang::CompilerInvocation::CreateFromArgs(*CI, &args[0], &args[0] + args.size(), Diags);
  llvm::StringRef srcString(source, src_length - 1);
  (*CI).getPreprocessorOpts().addRemappedFile("stringInput.cl",
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 5
                                              llvm::MemoryBuffer::getMemBuffer(srcString)
#else
                                              llvm::MemoryBuffer::getMemBuffer(srcString).release()
#endif
                                                );

  if (headers) {
    for (int n = 0; n < headerNum; n++) {
      GBE_ASSERT(headers[n] != NULL);
      GBE_ASSERT(header_length[n] != 0);
      llvm::StringRef headerString(headers[n], header_length[n] - 1);
      std::string hdPath("/cl/include/path/");
      hdPath += header_names[n];
      (*CI).getPreprocessorOpts().addRemappedFile(hdPath,
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 5
                                                  llvm::MemoryBuffer::getMemBuffer(headerString)
#else
                                                  llvm::MemoryBuffer::getMemBuffer(headerString).release()
#endif
                                                    );
    }
  }

  // Create the compiler instance
  clang::CompilerInstance Clang;
  Clang.setInvocation(CI.release());
  // Get ready to report problems
  Clang.createDiagnostics(DiagClient, false);

  Clang.getDiagnosticOpts().ShowCarets = false;
  if (!Clang.hasDiagnostics())
    return false;

  // Set Language
  clang::LangOptions &lang_opts = Clang.getLangOpts();
  lang_opts.OpenCL = 1;

  //llvm flags need command line parsing to take effect
  if (!Clang.getFrontendOpts().LLVMArgs.empty()) {
    unsigned NumArgs = Clang.getFrontendOpts().LLVMArgs.size();
    const char **Args = new const char *[NumArgs + 2];
    Args[0] = "clang (LLVM option parsing)";
    for (unsigned i = 0; i != NumArgs; ++i) {
      Args[i + 1] = Clang.getFrontendOpts().LLVMArgs[i].c_str();
    }
    Args[NumArgs + 1] = 0;
    llvm::cl::ParseCommandLineOptions(NumArgs + 1, Args);
    delete[] Args;
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

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 5
  llvm::Module *module = Act->takeModule();
#else
  llvm::Module *module = Act->takeModule().release();
#endif
  *out_module = module;

// Dump the LLVM if requested.
#if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR < 6)
  if (!dumpLLVMFileName.empty()) {
    std::string err;
    llvm::raw_fd_ostream ostream(dumpLLVMFileName.c_str(),
                                 err,
                                 llvm::sys::fs::F_None);

    if (err.empty()) {
      (*out_module)->print(ostream, 0);
    } //Otherwise, you'll have to make do without the dump.
  }

  if (!dumpSPIRBinaryName.empty()) {
    std::string err;
    llvm::raw_fd_ostream ostream(dumpSPIRBinaryName.c_str(),
                                 err,
                                 llvm::sys::fs::F_None);
    if (err.empty())
      llvm::WriteBitcodeToFile(*out_module, ostream);
  }
#else
  if (!dumpLLVMFileName.empty()) {
    std::error_code err;
    llvm::raw_fd_ostream ostream(dumpLLVMFileName.c_str(),
                                 err, llvm::sys::fs::F_None);
    if (!err) {
      (*out_module)->print(ostream, 0);
    } //Otherwise, you'll have to make do without the dump.
  }

  if (!dumpSPIRBinaryName.empty()) {
    std::error_code err;
    llvm::raw_fd_ostream ostream(dumpSPIRBinaryName.c_str(),
                                 err, llvm::sys::fs::F_None);
    if (!err)
      llvm::WriteBitcodeToFile(*out_module, ostream);
  }
#endif

  return true;
}

static GenProgram *
createProgramFromLLVMModule(uint32_t deviceID, const void *module,
                            const void *llvm_ctx, const char *asm_file_name, size_t stringSize,
                            char *err, size_t *errSize, int optLevel, const char *options)
{
  using namespace gbe;
  uint32_t fast_relaxed_math = 0;
  if (options != NULL)
    if (strstr(options, "-cl-fast-relaxed-math") != NULL)
      fast_relaxed_math = 1;

  GenProgram *program = GBE_NEW(GenProgram, deviceID, module, llvm_ctx, asm_file_name, fast_relaxed_math);

  std::string error;
  // Try to compile the program
  if (program->buildFromLLVMModule(module, error, optLevel) == false) {
    if (err != NULL && errSize != NULL && stringSize > 0u) {
      const size_t msgSize = std::min(error.size(), stringSize - 1u);
      std::memcpy(err, error.c_str(), msgSize);
      *errSize = error.size();
    }
    GBE_DELETE(program);
    return NULL;
  }

  // Everything run fine
  return program;
}

extern "C" GBE_EXPORT_SYMBOL bool
GenBuildProgram(uint32_t deviceID, const char *source, size_t src_length, const char *options,
                size_t errBufSize, char *err, size_t *errRetSize, char **binary, size_t *binarySize)
{
  int optLevel = 1;
  std::vector<std::string> clOpt;
  std::string dumpLLVMFileName, dumpASMFileName;
  std::string dumpSPIRBinaryName;
  uint32_t oclVersion = 0;
  bool ret = false;

  if (source == NULL || src_length == 0)
    return false;
  if (errBufSize == 0 || err == NULL || errRetSize == NULL)
    return false;
  if (binary == NULL || binarySize == NULL)
    return false;

  if (!processSourceAndOption(source, options, NULL, clOpt,
                              dumpLLVMFileName, dumpASMFileName, dumpSPIRBinaryName,
                              optLevel, errBufSize, err, errRetSize, oclVersion))
    return false;

  GenProgram *p;
  // will delete the module and act in GenProgram::CleanLlvmResource().
  llvm::Module *out_module;
  llvm::LLVMContext *llvm_ctx = new llvm::LLVMContext;

  static std::mutex llvm_mutex;
  if (!llvm::llvm_is_multithreaded())
    llvm_mutex.lock();

  if (buildLLVMModuleFromSource(source, src_length, NULL, NULL, NULL, 0, &out_module, llvm_ctx,
                                dumpLLVMFileName, dumpSPIRBinaryName, clOpt, errBufSize, err,
                                errRetSize, oclVersion)) {
    // Now build the program from llvm
    size_t clangErrSize = 0;
    errBufSize = errBufSize - *errRetSize;
    err = err + *errRetSize;
    clangErrSize = *errRetSize;

    if (!dumpASMFileName.empty()) {
      FILE *asmDumpStream = fopen(dumpASMFileName.c_str(), "w");
      if (asmDumpStream)
        fclose(asmDumpStream);
    }

    p = createProgramFromLLVMModule(deviceID, out_module, llvm_ctx,
                                    dumpASMFileName.empty() ? NULL : dumpASMFileName.c_str(),
                                    errBufSize, err, errRetSize, optLevel, options);
    *errRetSize += clangErrSize; // errRetSize has been reseted

    if (OCL_OUTPUT_BUILD_LOG && options)
      llvm::errs() << options;
  } else
    p = NULL;

  if (!llvm::llvm_is_multithreaded())
    llvm_mutex.unlock();

  if (p) {
    *binary = static_cast<char *>(p->toBinaryFormat(*binarySize));
    ret = true;
  }

  /* Release all the resource */
  if (p) {
    p->CleanLlvmResource();
    GBE_DELETE(p);
  }

  return ret;
}

extern "C" GBE_EXPORT_SYMBOL bool
GenCompileProgram(uint32_t deviceID, const char *source, size_t src_length, const char **headers,
                  size_t *header_length, const char **header_names, int headerNum, const char *options,
                  size_t errBufSize, char *err, size_t *errRetSize, char **binary, size_t *binarySize)
{

  int optLevel = 1;
  std::vector<std::string> clOpt;
  std::string dumpLLVMFileName, dumpASMFileName;
  std::string dumpSPIRBinaryName;
  uint32_t oclVersion = 0;
  bool ret = false;

  if (source == NULL || src_length == 0)
    return false;
  if (errBufSize == 0 || err == NULL || errRetSize == NULL)
    return false;
  if (binary == NULL || binarySize == NULL)
    return false;

  if (headers) {
    if (header_length == NULL || header_names == NULL || headerNum == 0)
      return false;

    for (int i = 0; i < headerNum; i++) {
      if (header_length[i] == 0 || header_names[i] == NULL || header_names[i][0] == 0)
        return false;
    }
  }

  if (!processSourceAndOption(source, options, NULL, clOpt, dumpLLVMFileName, dumpASMFileName,
                              dumpSPIRBinaryName, optLevel, errBufSize, err, errRetSize, oclVersion))
    return NULL;

  acquireLLVMContextLock();
  //FIXME: if use new allocated context to link two modules there would be context mismatch
  //for some functions, so we use global context now, need switch to new context later.
  llvm::Module *out_module = NULL;
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9
  llvm::LLVMContext *llvm_ctx = &GBEGetLLVMContext();
#else
  llvm::LLVMContext *llvm_ctx = &llvm::getGlobalContext();
#endif

  if (buildLLVMModuleFromSource(source, src_length, headers, header_length, header_names, headerNum,
                                &out_module, llvm_ctx, dumpLLVMFileName, dumpSPIRBinaryName,
                                clOpt, errBufSize, err, errRetSize, oclVersion)) {
    // Now build the program from llvm
    GBE_ASSERT(errRetSize != NULL);
    errBufSize -= *errRetSize;
    err += *errRetSize;

    if (OCL_OUTPUT_BUILD_LOG && options)
      llvm::errs() << options;
  }

  releaseLLVMContextLock();

  if (out_module) {
    ret = true;
    std::string irBuf;
    llvm::raw_string_ostream ostream(irBuf);
    llvm::WriteBitcodeToFile(out_module, ostream);
    ostream.flush();
    *binarySize = irBuf.capacity();
    *binary = static_cast<char *>(::malloc(*binarySize));
    ::memcpy(*binary, irBuf.c_str(), *binarySize);
  }

  /* llvm context is a global one, no need to delete */
  if (out_module) {
    delete out_module;
  }

  return ret;
}

extern "C" GBE_EXPORT_SYMBOL bool
GenLinkProgram(uint32_t deviceID, int binary_num, const char **binaries, size_t *binSizes, const char *options,
               size_t errBufSize, char *err, size_t *errRetSize,
               char **retBinary, size_t *retBinarySize)
{
  bool ret = true;
  char *errMsg = NULL;
  bool createLibrary = false;
  std::string str;
  std::string dumpASMFileName;

  if (binaries == NULL || binSizes == 0)
    return false;
  if (errBufSize == 0 || err == NULL || errRetSize == NULL)
    return false;
  if (retBinary == NULL || retBinarySize == NULL)
    return false;
  if (binary_num < 1)
    return false;

  if (options)
    str = options;
  if (str.find("-dump-opt-asm=") != std::string::npos) {
    dumpASMFileName = str.substr(str.find("=") + 1);
  }
  if (str.find("-create-library") != std::string::npos) {
    createLibrary = true;
  }

  llvm::Module *target_module = loadProgramFromLLVMIRBinary(deviceID, binaries[0], binSizes[0]);
  if (target_module == NULL)
    return false;

  for (int i = 1; i < binary_num; i++) {
    llvm::Module *mod = loadProgramFromLLVMIRBinary(deviceID, binaries[i], binSizes[i]);
    bool link_ret =
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9
      LLVMLinkModules2(wrap(target_module), wrap(mod));
#elif LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 7
      LLVMLinkModules(wrap(target_module), wrap(mod), LLVMLinkerPreserveSource_Removed, &errMsg);
#else
      LLVMLinkModules(wrap(target_module), wrap(mod), LLVMLinkerPreserveSource, &errMsg);
#endif

    if (errMsg) {
      strncpy(err, errMsg, errBufSize - 1);
      *errRetSize += strlen(errMsg);
      errBufSize -= strlen(errMsg);
      err += strlen(errMsg);
    }

    if (link_ret == true) { //error happened
      ret = false;
      break;
    }

    assert(mod != NULL);
  }

  if (ret == true) {
    if (createLibrary) {
      std::string irBuf;
      llvm::raw_string_ostream ostream(irBuf);
      llvm::WriteBitcodeToFile(target_module, ostream);
      ostream.flush();
      *retBinarySize = irBuf.capacity() + 3; // For add 'L' 'I' 'B'
      *retBinary = static_cast<char *>(::malloc(*retBinarySize));
      (*retBinary)[0] = 'L';
      (*retBinary)[1] = 'I';
      (*retBinary)[2] = 'B';
      ::memcpy(*retBinary + 3, irBuf.c_str(), *retBinarySize - 3);
    } else {
      size_t clangErrSize = *errRetSize;

      if (!dumpASMFileName.empty()) {
        FILE *asmDumpStream = fopen(dumpASMFileName.c_str(), "w");
        if (asmDumpStream)
          fclose(asmDumpStream);
      }

      GenProgram *p = createProgramFromLLVMModule(deviceID, target_module, NULL,
                                                  dumpASMFileName.empty() ? NULL : dumpASMFileName.c_str(),
                                                  errBufSize, err, errRetSize, 0, options);
      *errRetSize += clangErrSize; //errRetSize may be reseted

      if (OCL_OUTPUT_BUILD_LOG && options)
        llvm::errs() << options;

      if (p) {
        *retBinary = static_cast<char *>(p->toBinaryFormat(*retBinarySize));
        GBE_DELETE(p);
      } else {
        ret = false;
      }
    }
  }

  delete target_module;
  return ret;
}

extern "C" GBE_EXPORT_SYMBOL bool
GenCheckCompilerOption(const char *option)
{
  vector<const char *> args;
  if (option == NULL)
    return 1; //if NULL, return ok

  std::string s(option);
  size_t pos = s.find("-create-library");
  //clang don't accept -create-library and -enable-link-options, erase them
  if (pos != std::string::npos) {
    s.erase(pos, strlen("-create-library"));
  }
  pos = s.find("-enable-link-options");
  if (pos != std::string::npos) {
    s.erase(pos, strlen("-enable-link-options"));
  }
  pos = s.find("-dump-opt-asm");
  if (pos != std::string::npos) {
    size_t pos2 = s.find(" ", pos);
    if (pos2 == std::string::npos)
      s.erase(pos);
    else
      s.erase(pos, pos2 - pos);
  }
  pos = s.find("-dump-opt-llvm");
  if (pos != std::string::npos) {
    size_t pos2 = s.find(" ", pos);
    if (pos2 == std::string::npos)
      s.erase(pos);
    else
      s.erase(pos, pos2 - pos);
  }
  pos = s.find("-dump-spir-binary");
  if (pos != std::string::npos) {
    size_t pos2 = s.find(" ", pos);
    if (pos2 == std::string::npos)
      s.erase(pos);
    else
      s.erase(pos, pos2 - pos);
  }

  // -cl-no-signed-zeros is not supported, and some verion can not recognize it
  pos = s.find("-cl-no-signed-zeros");
  if (pos != std::string::npos) {
    s.erase(pos, strlen("-cl-no-signed-zeros"));
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
  return clang::CompilerInvocation::CreateFromArgs(*CI, &args[0], &args[0] + args.size(), Diags);
}
