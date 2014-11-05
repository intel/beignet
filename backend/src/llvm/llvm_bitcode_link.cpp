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
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <set>
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Support/SourceMgr.h"

#include "sys/cvar.hpp"
#include "src/GBEConfig.h"
#include "llvm/llvm_gen_backend.hpp"
#if LLVM_VERSION_MINOR >= 5
#include "llvm/Linker/Linker.h"
#else
#include "llvm/Linker.h"
#endif

using namespace llvm;

SVAR(OCL_BITCODE_LIB_PATH, OCL_BITCODE_BIN);

namespace gbe
{
  static Module* createOclBitCodeModule(LLVMContext& ctx, bool strictMath)
  {
    std::string bitCodeFiles = OCL_BITCODE_LIB_PATH;
    std::istringstream bitCodeFilePath(bitCodeFiles);
    std::string FilePath;
    bool findBC = false;
    Module* oclLib = NULL;
    SMDiagnostic Err;

    while (std::getline(bitCodeFilePath, FilePath, ':')) {
      if(access(FilePath.c_str(), R_OK) == 0) {
        findBC = true;
        break;
      }
    }
    assert(findBC);

    oclLib = getLazyIRFileModule(FilePath, Err, ctx);
    if (!oclLib) {
      printf("Fatal Error: ocl lib can not be opened\n");
      return NULL;
    }

    if (strictMath) {
      llvm::GlobalVariable* mathFastFlag = oclLib->getGlobalVariable("__ocl_math_fastpath_flag");
      assert(mathFastFlag);
      Type* intTy = IntegerType::get(ctx, 32);
      mathFastFlag->setInitializer(ConstantInt::get(intTy, 0));
    }

    return oclLib;
  }

  static bool materializedFuncCall(Module& src, Module& lib, llvm::Function &KF, std::set<std::string>& MFS)
  {
    bool fromSrc = false;
    for (llvm::Function::iterator B = KF.begin(), BE = KF.end(); B != BE; B++) {
      for (BasicBlock::iterator instI = B->begin(),
           instE = B->end(); instI != instE; ++instI) {
        llvm::CallInst* call = dyn_cast<llvm::CallInst>(instI);
        if (!call) {
          continue;
        }

        if (call->getCalledFunction() &&
            call->getCalledFunction()->getIntrinsicID() != 0)
          continue;

        Value *Callee = call->getCalledValue();
        const std::string fnName = Callee->getName();

        if (!MFS.insert(fnName).second) {
          continue;
        }

        fromSrc = false;
        llvm::Function *newMF = lib.getFunction(fnName);
        if (!newMF) {
          newMF = src.getFunction(fnName);
          if (!newMF) {
	    printf("Can not find the lib: %s\n", fnName.c_str());
	    return false;
          }
	  fromSrc = true;
        }

        std::string ErrInfo;// = "Not Materializable";
        if (!fromSrc && newMF->isMaterializable()) {
          if (newMF->Materialize(&ErrInfo)) {
            printf("Can not materialize the function: %s, because %s\n", fnName.c_str(), ErrInfo.c_str());
            return false;
          }
        }

        if (!materializedFuncCall(src, lib, *newMF, MFS))
          return false;

      }
    }

    return true;
  }


  Module* runBitCodeLinker(Module *mod, bool strictMath)
  {
    LLVMContext& ctx = mod->getContext();
    std::set<std::string> materializedFuncs;
    Module* clonedLib = createOclBitCodeModule(ctx, strictMath);
    assert(clonedLib && "Can not create the beignet bitcode\n");

    std::vector<const char *> kernels;
    std::vector<const char *> builtinFuncs;
    /* Add the memset and memcpy functions here. */
    builtinFuncs.push_back("__gen_memcpy_gg");
    builtinFuncs.push_back("__gen_memcpy_gp");
    builtinFuncs.push_back("__gen_memcpy_gl");
    builtinFuncs.push_back("__gen_memcpy_pg");
    builtinFuncs.push_back("__gen_memcpy_pp");
    builtinFuncs.push_back("__gen_memcpy_pl");
    builtinFuncs.push_back("__gen_memcpy_lg");
    builtinFuncs.push_back("__gen_memcpy_lp");
    builtinFuncs.push_back("__gen_memcpy_ll");
    builtinFuncs.push_back("__gen_memset_p");
    builtinFuncs.push_back("__gen_memset_g");
    builtinFuncs.push_back("__gen_memset_l");

    builtinFuncs.push_back("__gen_memcpy_gg_align");
    builtinFuncs.push_back("__gen_memcpy_gp_align");
    builtinFuncs.push_back("__gen_memcpy_gl_align");
    builtinFuncs.push_back("__gen_memcpy_pg_align");
    builtinFuncs.push_back("__gen_memcpy_pp_align");
    builtinFuncs.push_back("__gen_memcpy_pl_align");
    builtinFuncs.push_back("__gen_memcpy_lg_align");
    builtinFuncs.push_back("__gen_memcpy_lp_align");
    builtinFuncs.push_back("__gen_memcpy_ll_align");
    builtinFuncs.push_back("__gen_memset_p_align");
    builtinFuncs.push_back("__gen_memset_g_align");
    builtinFuncs.push_back("__gen_memset_l_align");


    for (Module::iterator SF = mod->begin(), E = mod->end(); SF != E; ++SF) {
      if (SF->isDeclaration()) continue;
      if (!isKernelFunction(*SF)) continue;
      kernels.push_back(SF->getName().data());

      if (!materializedFuncCall(*mod, *clonedLib, *SF, materializedFuncs)) {
        delete clonedLib;
        return NULL;
      }
    }

    if (kernels.empty()) {
      printf("One module without kernel function!\n");
      delete clonedLib;
      return NULL;
    }

    for (auto &f : builtinFuncs) {
      const std::string fnName(f);
      if (!materializedFuncs.insert(fnName).second) {
        continue;
      }

      llvm::Function *newMF = clonedLib->getFunction(fnName);
      if (!newMF) {
        printf("Can not find the function: %s\n", fnName.c_str());
        delete clonedLib;
        return NULL;
      }
      std::string ErrInfo;// = "Not Materializable";
      if (newMF->isMaterializable()) {
        if (newMF->Materialize(&ErrInfo)) {
          printf("Can not materialize the function: %s, because %s\n", fnName.c_str(), ErrInfo.c_str());
          delete clonedLib;
          return NULL;
        }
      }

      if (!materializedFuncCall(*mod, *clonedLib, *newMF, materializedFuncs)) {
        delete clonedLib;
        return NULL;
      }

      kernels.push_back(f);
    }

    /* We use beignet's bitcode as dst because it will have a lot of
       lazy functions which will not be loaded. */
    std::string errorMsg;
    if(Linker::LinkModules(clonedLib, mod, Linker::DestroySource, &errorMsg)) {
      delete clonedLib;
      printf("Fatal Error: link the bitcode error:\n%s\n", errorMsg.c_str());
      return NULL;
    }

    llvm::PassManager passes;

    passes.add(createInternalizePass(kernels));
    passes.add(createGlobalDCEPass());

    passes.run(*clonedLib);

    return clonedLib;
  }

} // end namespace
