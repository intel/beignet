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
 */

/**
 * \file llvm_intrinisc_lowering.cpp
 * \author Yang Rong <rong.r.yang@intel.com>
 */

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
#include "llvm/Support/raw_ostream.h"

#include "llvm/llvm_gen_backend.hpp"
#include "sys/map.hpp"


using namespace llvm;

namespace gbe {
    class InstrinsicLowering : public BasicBlockPass
    {
    public:
      static char ID;
      InstrinsicLowering() :
        BasicBlockPass(ID) {}

      void getAnalysisUsage(AnalysisUsage &AU) const {

      }

      virtual const char *getPassName() const {
        return "SPIR backend: lowering instrinsics";
      }
      static char convertSpaceToName(Value *val) {
        const uint32_t space = val->getType()->getPointerAddressSpace();
        switch(space) {
          case 0:
            return 'p';
          case 1:
            return 'g';
          case 3:
            return 'l';
          default:
            assert("Non support address space");
            return '\0';
        }
      }
      static CallInst *replaceCallWith(const char *NewFn, CallInst *CI,
                                     Value **ArgBegin, Value **ArgEnd,
                                     Type *RetTy)
      {
        // If we haven't already looked up this function, check to see if the
        // program already contains a function with this name.
        Module *M = CI->getParent()->getParent()->getParent();
        // Get or insert the definition now.
        std::vector<Type *> ParamTys;
        for (Value** I = ArgBegin; I != ArgEnd; ++I)
          ParamTys.push_back((*I)->getType());
        Constant* FCache = M->getOrInsertFunction(NewFn,
                                        FunctionType::get(RetTy, ParamTys, false));

        IRBuilder<> Builder(CI->getParent(), CI);
        SmallVector<Value *, 8> Args(ArgBegin, ArgEnd);
        CallInst *NewCI = Builder.CreateCall(FCache, Args);
        NewCI->setName(CI->getName());
        if (!CI->use_empty())
          CI->replaceAllUsesWith(NewCI);
        CI->eraseFromParent();
        return NewCI;
      }
      virtual bool runOnBasicBlock(BasicBlock &BB)
      {
        bool changedBlock = false;
        Module *M = BB.getParent()->getParent();

        DataLayout TD(M);
        LLVMContext &Context = BB.getContext();
        for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
          Instruction *Inst = DI++;
          CallInst* CI = dyn_cast<CallInst>(Inst);
          if(CI == NULL)
            continue;

          IRBuilder<> Builder(&BB, CI);
          // only support memcpy and memset
          if (Function *F = CI->getCalledFunction()) {
            const Intrinsic::ID intrinsicID = (Intrinsic::ID) F->getIntrinsicID();
            if (intrinsicID == 0)
              continue;
            switch (intrinsicID) {
              case Intrinsic::memcpy: {
                Type *IntPtr = TD.getIntPtrType(Context);
                Value *Size = Builder.CreateIntCast(CI->getArgOperand(2), IntPtr,
                                                    /* isSigned */ false);
                Value *Ops[3];
                Ops[0] = CI->getArgOperand(0);
                Ops[1] = CI->getArgOperand(1);
                Ops[2] = Size;
                char name[16] = "__gen_memcpy_xx";
                name[13] = convertSpaceToName(Ops[0]);
                name[14] = convertSpaceToName(Ops[1]);
                replaceCallWith(name, CI, Ops, Ops+3, Type::getVoidTy(Context));
                break;
              }
              case Intrinsic::memset: {
                Value *Op0 = CI->getArgOperand(0);
                Value *val = Builder.CreateIntCast(CI->getArgOperand(1), IntegerType::getInt8Ty(Context),
                                                    /* isSigned */ false);
                Type *IntPtr = TD.getIntPtrType(Op0->getType());
                Value *Size = Builder.CreateIntCast(CI->getArgOperand(2), IntPtr,
                                                    /* isSigned */ false);
                Value *Ops[3];
                Ops[0] = Op0;
                // Extend the amount to i32.
                Ops[1] = val;
                Ops[2] = Size;
                char name[16] = "__gen_memset_x";
                name[13] = convertSpaceToName(Ops[0]);
                replaceCallWith(name, CI, Ops, Ops+3, Type::getVoidTy(Context));
                break;
              }
              default:
                continue;
            }
          }
        }
        return changedBlock;
      }
    };

    char InstrinsicLowering::ID = 0;

    BasicBlockPass *createIntrinsicLoweringPass() {
      return new InstrinsicLowering();
    }
} // end namespace
