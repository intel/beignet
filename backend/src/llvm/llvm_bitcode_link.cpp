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

#include "sys/cvar.hpp"
#include "src/GBEConfig.h"
#include "llvm_includes.hpp"
#include "llvm/llvm_gen_backend.hpp"
#include "ir/unit.hpp"

using namespace llvm;

SVAR(OCL_BITCODE_LIB_PATH, OCL_BITCODE_BIN);
SVAR(OCL_BITCODE_LIB_20_PATH, OCL_BITCODE_BIN_20);

namespace gbe
{
  static Module* createOclBitCodeModule(LLVMContext& ctx,
                                                 bool strictMath,
                                                 uint32_t oclVersion)
  {
    std::string bitCodeFiles = oclVersion >= 200 ?
                               OCL_BITCODE_LIB_20_PATH : OCL_BITCODE_LIB_PATH;
    if(bitCodeFiles == "")
      bitCodeFiles = oclVersion >= 200 ? OCL_BITCODE_BIN_20 : OCL_BITCODE_BIN;
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
    if (!findBC) {
      printf("Fatal Error: ocl lib %s does not exist\n", bitCodeFiles.c_str());
      return NULL;
    }

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR <= 35
    oclLib = getLazyIRFileModule(FilePath, Err, ctx);
#else
    oclLib = getLazyIRFileModule(FilePath, Err, ctx).release();
#endif
    if (!oclLib) {
      printf("Fatal Error: ocl lib can not be opened\n");
      return NULL;
    }

    llvm::GlobalVariable* mathFastFlag = oclLib->getGlobalVariable("__ocl_math_fastpath_flag");
    assert(mathFastFlag);
    Type* intTy = IntegerType::get(ctx, 32);
    mathFastFlag->setInitializer(ConstantInt::get(intTy, strictMath ? 0 : 1));

    return oclLib;
  }

  static bool materializedFuncCall(Module& src, Module& lib, llvm::Function& KF,
                                   std::set<std::string>& MFS,
                                   std::vector<GlobalValue *>&Gvs) {
    bool fromSrc = false;
    for (llvm::Function::iterator B = KF.begin(), BE = KF.end(); B != BE; B++) {
      for (BasicBlock::iterator instI = B->begin(),
           instE = B->end(); instI != instE; ++instI) {
        llvm::CallInst* call = dyn_cast<llvm::CallInst>(instI);
        if (!call) {
          continue;
        }

        llvm::Function * callFunc = call->getCalledFunction();
        //if(!callFunc) {
        //  continue;
        //}

        if (callFunc && callFunc->getIntrinsicID() != 0)
          continue;

        std::string fnName = call->getCalledValue()->stripPointerCasts()->getName();

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
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
          if (llvm::Error EC = newMF->materialize()) {
            std::string Msg;
            handleAllErrors(std::move(EC), [&](ErrorInfoBase &EIB) {
              Msg = EIB.message();
            });
            printf("Can not materialize the function: %s, because %s\n", fnName.c_str(), Msg.c_str());
            return false;
          }
          Gvs.push_back((GlobalValue *)newMF);
#elif LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 36
          if (std::error_code EC = newMF->materialize()) {
            printf("Can not materialize the function: %s, because %s\n", fnName.c_str(), EC.message().c_str());
            return false;
          }
          Gvs.push_back((GlobalValue *)newMF);
#else
         if (newMF->Materialize(&ErrInfo)) {
            printf("Can not materialize the function: %s, because %s\n", fnName.c_str(), ErrInfo.c_str());
            return false;
          }

#endif
        }
        if (!materializedFuncCall(src, lib, *newMF, MFS, Gvs))
          return false;

      }
    }

    return true;
  }


  Module* runBitCodeLinker(Module *mod, bool strictMath, ir::Unit &unit)
  {
    LLVMContext& ctx = mod->getContext();
    std::set<std::string> materializedFuncs;
    std::vector<GlobalValue *> Gvs;

    uint32_t oclVersion = getModuleOclVersion(mod);
    ir::PointerSize size = oclVersion >= 200 ? ir::POINTER_64_BITS : ir::POINTER_32_BITS;
    unit.setPointerSize(size);
    Module* clonedLib = createOclBitCodeModule(ctx, strictMath, oclVersion);
    if (clonedLib == NULL)
      return NULL;

    std::vector<const char *> kernels;
    std::vector<const char *> kerneltmp;
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

    builtinFuncs.push_back("__gen_memcpy_pc");
    builtinFuncs.push_back("__gen_memcpy_gc");
    builtinFuncs.push_back("__gen_memcpy_lc");

    builtinFuncs.push_back("__gen_memcpy_pc_align");
    builtinFuncs.push_back("__gen_memcpy_gc_align");
    builtinFuncs.push_back("__gen_memcpy_lc_align");

    if (oclVersion >= 200) {
      builtinFuncs.push_back("__gen_memcpy_gn");
      builtinFuncs.push_back("__gen_memcpy_pn");
      builtinFuncs.push_back("__gen_memcpy_ln");
      builtinFuncs.push_back("__gen_memcpy_ng");
      builtinFuncs.push_back("__gen_memcpy_np");
      builtinFuncs.push_back("__gen_memcpy_nl");
      builtinFuncs.push_back("__gen_memcpy_nc");
      builtinFuncs.push_back("__gen_memcpy_nn");
      builtinFuncs.push_back("__gen_memset_n");

      builtinFuncs.push_back("__gen_memcpy_gn_align");
      builtinFuncs.push_back("__gen_memcpy_pn_align");
      builtinFuncs.push_back("__gen_memcpy_ln_align");
      builtinFuncs.push_back("__gen_memcpy_ng_align");
      builtinFuncs.push_back("__gen_memcpy_np_align");
      builtinFuncs.push_back("__gen_memcpy_nl_align");
      builtinFuncs.push_back("__gen_memcpy_nc_align");
      builtinFuncs.push_back("__gen_memcpy_nn_align");
      builtinFuncs.push_back("__gen_memset_n_align");
    }

    for (Module::iterator SF = mod->begin(), E = mod->end(); SF != E; ++SF) {
      if (SF->isDeclaration()) continue;
      if (!isKernelFunction(*SF)) continue;
      // mod will be deleted after link, copy the names.
      const char *funcName = SF->getName().data();
      char * tmp = new char[strlen(funcName)+1];
      strcpy(tmp,funcName);
      kernels.push_back(tmp);
      kerneltmp.push_back(tmp);

      if (!materializedFuncCall(*mod, *clonedLib, *SF, materializedFuncs, Gvs)) {
        delete clonedLib;
        return NULL;
      }
      Gvs.push_back((GlobalValue *)&*SF);
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
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
        if (llvm::Error EC = newMF->materialize()) {
          std::string Msg;
          handleAllErrors(std::move(EC), [&](ErrorInfoBase &EIB) {
            Msg = EIB.message();
          });
          printf("Can not materialize the function: %s, because %s\n", fnName.c_str(), Msg.c_str());
          delete clonedLib;
          return NULL;
        }
#elif LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 36
        if (std::error_code EC = newMF->materialize()) {
          printf("Can not materialize the function: %s, because %s\n", fnName.c_str(), EC.message().c_str());
          delete clonedLib;
          return NULL;
        }
#else
        if (newMF->Materialize(&ErrInfo)) {
          printf("Can not materialize the function: %s, because %s\n", fnName.c_str(), ErrInfo.c_str());
          delete clonedLib;
          return NULL;
        }
#endif
      }

      if (!materializedFuncCall(*mod, *clonedLib, *newMF, materializedFuncs, Gvs)) {
        delete clonedLib;
        return NULL;
      }

      Gvs.push_back((GlobalValue *)newMF);
      kernels.push_back(f);
    }

  /* The llvm 3.8 now has a strict materialized check for all value by checking
   * module is materialized. If we want to use library as old style that just
   * materialize what we need, we need to remove what we did not need before
   * materialize all of the module. To do this, we need all of the builtin
   * funcitons and what are needed from the kernel functions, these functions
   * are materalized and are recorded in Gvs, the GlobalValue like PI are also
   * needed and are added. Now we could not use use_empty to check if the GVs
   * are needed before the module is marked as all materialized, so we just
   * materialize all of them as there are only 7 GVs. Then we use GVExtraction
   * pass to extract the functions and values in Gvs from the library module.
   * After extract what we need and remove what we do not need, we use 
   * materializeAll to mark the module as materialized. */
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
    /* Get all GlobalValue from module. */
    Module::GlobalListType &GVlist = clonedLib->getGlobalList();
    for(Module::global_iterator GVitr = GVlist.begin();GVitr != GVlist.end();++GVitr) {
      GlobalValue * GV = &*GVitr;
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
      ExitOnError ExitOnErr("Can not materialize the clonedLib: ");
      ExitOnErr(clonedLib->materialize(GV));
#else
      clonedLib->materialize(GV);
#endif
      Gvs.push_back(GV);
    }
    llvm::legacy::PassManager Extract;
    /* Extract all values we need using GVExtractionPass. */
    Extract.add(createGVExtractionPass(Gvs, false));
    Extract.run(*clonedLib);
    /* Mark the library module as materialized for later use. */
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
    ExitOnError ExitOnErr("Can not materialize the clonedLib: ");
    ExitOnErr(clonedLib->materializeAll());
#else
    clonedLib->materializeAll();
#endif
#endif

    /* the SPIR binary datalayout maybe different with beignet's bitcode */
    if(clonedLib->getDataLayout() != mod->getDataLayout())
      mod->setDataLayout(clonedLib->getDataLayout());

    /* We use beignet's bitcode as dst because it will have a lot of
       lazy functions which will not be loaded. */
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 39
#if LLVM_VERSION_MAJOR >= 7
    llvm::Module * linked_module = llvm::CloneModule(*(llvm::Module*)mod).release();
#else
    llvm::Module * linked_module = llvm::CloneModule((llvm::Module*)mod).release();
#endif
    if(LLVMLinkModules2(wrap(clonedLib), wrap(linked_module))) {
#else
    char* errorMsg;
    if(LLVMLinkModules(wrap(clonedLib), wrap(mod), LLVMLinkerDestroySource, &errorMsg)) {
      printf("Fatal Error: link the bitcode error:\n%s\n", errorMsg);
#endif
      delete clonedLib;
      return NULL;
    }
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
    llvm::legacy::PassManager passes;
#else
    llvm::PassManager passes;
#endif

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 39
    auto PreserveKernel = [=](const GlobalValue &GV) {
      for(size_t i = 0;i < kernels.size(); ++i)
        if(strcmp(GV.getName().data(), kernels[i]))
          return true;
      return false;
    };

    passes.add(createInternalizePass(PreserveKernel));
#else
    passes.add(createInternalizePass(kernels));
#endif
    passes.add(createGlobalDCEPass());

    passes.run(*clonedLib);

    for(size_t i = 0;i < kerneltmp.size(); i++)
      delete[] kerneltmp[i];

    return clonedLib;
  }

} // end namespace
