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

/**
 * \file llvm_profiling.cpp
 * This file will insert some instructions for each profiling point.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "llvm/Config/llvm-config.h"
#if LLVM_VERSION_MINOR <= 2
#include "llvm/Function.h"
#include "llvm/InstrTypes.h"
#include "llvm/Instructions.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Module.h"
#else
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#endif  /* LLVM_VERSION_MINOR <= 2 */
#include "llvm/Pass.h"
#if LLVM_VERSION_MINOR <= 1
#include "llvm/Support/IRBuilder.h"
#elif LLVM_VERSION_MINOR == 2
#include "llvm/IRBuilder.h"
#else
#include "llvm/IR/IRBuilder.h"
#endif /* LLVM_VERSION_MINOR <= 1 */

#if LLVM_VERSION_MINOR >= 5
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CFG.h"
#else
#include "llvm/Support/CallSite.h"
#include "llvm/Support/CFG.h"
#endif

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Attributes.h"

#include "llvm/llvm_gen_backend.hpp"
#include "sys/map.hpp"
#include "ir/unit.hpp"

#include <iostream>
#include <vector>


using namespace llvm;
using std::vector;


namespace gbe
{
  using namespace ir;

  class ProfilingInserter : public FunctionPass
  {
  public:
    static char ID;
    Module* module;
    IRBuilder<>* builder;
    Type* intTy;
    Type *ptrTy;
    int profilingType;

    ProfilingInserter(int profiling) : FunctionPass(ID), profilingType(profiling)
    {
      module = NULL;
      builder = NULL;
      intTy = NULL;
      ptrTy = NULL;
    }

    ~ProfilingInserter(void)
    {
    }

    virtual const char *getPassName() const
    {
      return "Timestamp Parser";
    }

    virtual bool runOnFunction(llvm::Function &F);
  };

  bool ProfilingInserter::runOnFunction(llvm::Function &F)
  {
    bool changed = false;
    int pointNum = 0;

    switch (F.getCallingConv()) {
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 2
      case CallingConv::PTX_Device:
        return false;
      case CallingConv::PTX_Kernel:
#else
      case CallingConv::C:
      case CallingConv::Fast:
      case CallingConv::SPIR_KERNEL:
#endif
        break;
      default:
        GBE_ASSERTM(false, "Unsupported calling convention");
    }

    // As we inline all function calls, so skip non-kernel functions
    bool bKernel = isKernelFunction(F);
    if (!bKernel) return changed;

    module = F.getParent();
    intTy = IntegerType::get(module->getContext(), 32);
    ptrTy = Type::getInt32PtrTy(module->getContext(), 1);
    builder = new IRBuilder<>(module->getContext());

    /* alloc a new buffer ptr to collect the timestamps. */
    builder->SetInsertPoint(&*F.begin()->begin());
    llvm::Constant *profilingBuf = module->getGlobalVariable("__gen_ocl_profiling_buf");
    if (!profilingBuf) {
      profilingBuf = new GlobalVariable(*module, intTy, false,
          GlobalVariable::ExternalLinkage, nullptr, StringRef("__gen_ocl_profiling_buf"),
          nullptr, GlobalVariable::NotThreadLocal, 1);
    }

    changed = true;

    for (llvm::Function::iterator B = F.begin(), BE = F.end(); B != BE; B++) {
      /* Skip the empty blocks. */
      if (B->empty())
        continue;

      BasicBlock::iterator instI = B->begin();
      for ( ; instI != B->end(); instI++) {
        if (dyn_cast<llvm::PHINode>(instI))
          continue;
        if (dyn_cast<llvm::ReturnInst>(instI)) {
          instI++;
          GBE_ASSERT(instI == B->end());
          break;
        }
        if (dyn_cast<llvm::BranchInst>(instI)) {
          instI++;
          GBE_ASSERT(instI == B->end());
          break;
        }
        break;
      }

      if (instI == B->end())
        continue;

      if (pointNum >= 20) // To many timestamp.
        continue;

      // Insert the first one at beginning of not PHI.
      builder->SetInsertPoint(&*instI);
      /* Add the timestamp store function call. */
      // __gen_ocl_store_timestamp(int nth, int type);
      Value *Args[2] = {ConstantInt::get(intTy, pointNum++), ConstantInt::get(intTy, profilingType)};
      builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
              "__gen_ocl_calc_timestamp", Type::getVoidTy(module->getContext()),
              IntegerType::getInt32Ty(module->getContext()),
              IntegerType::getInt32Ty(module->getContext()),
              NULL)),
              ArrayRef<Value*>(Args));
    }
    /* We insert one store_profiling at the end of the last block to hold the place. */
    llvm::Function::iterator BE = F.end();
    BE--;
    BasicBlock::iterator retInst = BE->end();
    retInst--;
    builder->SetInsertPoint(&*retInst);
    Value *Args2[2] = {profilingBuf, ConstantInt::get(intTy, profilingType)};

    builder->CreateCall(cast<llvm::Function>(module->getOrInsertFunction(
            "__gen_ocl_store_profiling", Type::getVoidTy(module->getContext()),
            ptrTy,
            IntegerType::getInt32Ty(module->getContext()),
            NULL)),
            ArrayRef<Value*>(Args2));

    delete builder;
    return changed;
  }

  FunctionPass* createProfilingInserterPass(int profilingType, ir::Unit &unit)
  {
    unit.setInProfilingMode(true);
    return new ProfilingInserter(profilingType);
  }
  char ProfilingInserter::ID = 0;

} // end namespace
