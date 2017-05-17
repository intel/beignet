/*
 * Copyright © 2014 Intel Corporation
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
 * This pass is to solve the __gen_ocl_sampler_need_fix() and
 * __gen_ocl_sampler_need_rounding_fix(), as for some special
 * sampler type, we need some extra work around operations to
 * make sure to get correct pixel value. But for some other
 * sampler, we don't need those work around code.
 */

#include "llvm_includes.hpp"

#include "llvm_gen_backend.hpp"
#include "ocl_common_defines.h"

using namespace llvm;

namespace gbe {

  class SamplerFix : public FunctionPass {
  public:
    SamplerFix() : FunctionPass(ID) {
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 35
      initializeDominatorTreeWrapperPassPass(*PassRegistry::getPassRegistry());
#else
      initializeDominatorTreePass(*PassRegistry::getPassRegistry());
#endif
    }

    bool visitCallInst(CallInst *I) {
      if(!I)
        return false;
      Value *Callee = I->getCalledValue();
      const std::string fnName = Callee->getName();
      bool changed = false;
      Type *boolTy = IntegerType::get(I->getContext(), 1);
      Type *i32Ty = IntegerType::get(I->getContext(), 32);

      if (fnName.compare("__gen_ocl_sampler_need_fix") == 0) {

        //  return (((sampler & __CLK_ADDRESS_MASK) == CLK_ADDRESS_CLAMP) &&
        //          ((sampler & __CLK_FILTER_MASK) == CLK_FILTER_NEAREST));
        bool needFix = true;
        Value *needFixVal;
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
        CallInst *init = dyn_cast<CallInst>(I->getOperand(0));
        if (init && init->getCalledValue()->getName().compare("__translate_sampler_initializer"))
        {
          const ConstantInt *ci = dyn_cast<ConstantInt>(init->getOperand(0));
          uint32_t samplerInt = ci->getZExtValue();
#else
        if (dyn_cast<ConstantInt>(I->getOperand(0))) {
          const ConstantInt *ci = dyn_cast<ConstantInt>(I->getOperand(0));
          uint32_t samplerInt = ci->getZExtValue();
#endif
          needFix = ((samplerInt & __CLK_ADDRESS_MASK) == CLK_ADDRESS_CLAMP &&
                     (samplerInt & __CLK_FILTER_MASK) == CLK_FILTER_NEAREST);
          needFixVal = ConstantInt::get(boolTy, needFix);
        } else {
          IRBuilder<> Builder(I->getParent());

          Builder.SetInsertPoint(I);

          Value *addressMask = ConstantInt::get(i32Ty, __CLK_ADDRESS_MASK);
          Value *clampInt =  ConstantInt::get(i32Ty, CLK_ADDRESS_CLAMP);
          Value *filterMask = ConstantInt::get(i32Ty, __CLK_FILTER_MASK);
          Value *nearestInt = ConstantInt::get(i32Ty, CLK_FILTER_NEAREST);

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
          Module *M = I->getParent()->getParent()->getParent();
          Value* samplerCvt = M->getOrInsertFunction("__gen_ocl_sampler_to_int", i32Ty, I->getOperand(0)->getType(), nullptr);
          Value *samplerVal = Builder.CreateCall(samplerCvt, {I->getOperand(0)});
#else
          Value *samplerVal = I->getOperand(0);
#endif
          Value *addressMode = Builder.CreateAnd(samplerVal, addressMask);
          Value *isClampMode = Builder.CreateICmpEQ(addressMode, clampInt);
          Value *filterMode = Builder.CreateAnd(samplerVal, filterMask);
          Value *isNearestMode = Builder.CreateICmpEQ(filterMode, nearestInt);

          needFixVal = Builder.CreateAnd(isClampMode, isNearestMode);
        }

        I->replaceAllUsesWith(needFixVal);
        changed = true;
      } else if (fnName.compare("__gen_ocl_sampler_need_rounding_fix") == 0) {

        //  return ((sampler & CLK_NORMALIZED_COORDS_TRUE) == 0);
        bool needFix = true;
        Value *needFixVal;
 #if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
        CallInst *init = dyn_cast<CallInst>(I->getOperand(0));
        if (init && init->getCalledValue()->getName().compare("__translate_sampler_initializer"))
        {
          const ConstantInt *ci = dyn_cast<ConstantInt>(init->getOperand(0));
          uint32_t samplerInt = ci->getZExtValue();
#else
        if (dyn_cast<ConstantInt>(I->getOperand(0))) {
          const ConstantInt *ci = dyn_cast<ConstantInt>(I->getOperand(0));
          uint32_t samplerInt = ci->getZExtValue();
#endif
          needFix = samplerInt & CLK_NORMALIZED_COORDS_TRUE;
          needFixVal = ConstantInt::get(boolTy, needFix);
        } else {
          IRBuilder<> Builder(I->getParent());
          Builder.SetInsertPoint(I);
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
          Module *M = I->getParent()->getParent()->getParent();
          Value* samplerCvt = M->getOrInsertFunction("__gen_ocl_sampler_to_int", i32Ty, I->getOperand(0)->getType(), nullptr);
          Value *samplerVal = Builder.CreateCall(samplerCvt, {I->getOperand(0)});
#else
          Value *samplerVal = I->getOperand(0);
#endif
          Value *normalizeMask = ConstantInt::get(i32Ty, CLK_NORMALIZED_COORDS_TRUE);
          Value *normalizeMode = Builder.CreateAnd(samplerVal, normalizeMask);
          needFixVal = Builder.CreateICmpEQ(normalizeMode, ConstantInt::get(i32Ty, 0));
        }
        I->replaceAllUsesWith(needFixVal);
        changed = true;
      }
      return changed;
    }

    bool runOnFunction(Function& F) {
      bool changed = false;
      std::set<Instruction*> deadInsnSet;
      for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I) {
        if (dyn_cast<CallInst>(&*I)) {
          if (visitCallInst(dyn_cast<CallInst>(&*I))) {
            changed = true;
            deadInsnSet.insert(&*I);
          }
        }
      }
      for (auto it: deadInsnSet)
        it->eraseFromParent();
      return changed;
    }

    static char ID;
  };

  FunctionPass* createSamplerFixPass() {
    return new SamplerFix();
  }
  char SamplerFix::ID = 0;
};
