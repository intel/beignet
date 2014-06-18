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
 * Author: Ruiling, Song <ruiling.song@intel.com>
 *
 * The Idea is that: As GEN support at most 4 successive DWORD load/store,
 * then merge successive load/store that are compatible is beneficial.
 * The method of checking whether two load/store is compatible are borrowed
 * from Vectorize passes in llvm.
 */

#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"

#include "llvm/Config/llvm-config.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 2
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
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 1
#include "llvm/Support/IRBuilder.h"
#elif LLVM_VERSION_MINOR == 2
#include "llvm/IRBuilder.h"
#else
#include "llvm/IR/IRBuilder.h"
#endif /* LLVM_VERSION_MINOR <= 1 */
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

using namespace llvm;
namespace gbe {
  class GenLoadStoreOptimization : public BasicBlockPass {

  public:
    static char ID;
    ScalarEvolution *SE;
    const DataLayout *TD;
    GenLoadStoreOptimization() : BasicBlockPass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<ScalarEvolution>();
      AU.addPreserved<ScalarEvolution>();
      AU.setPreservesCFG();
    }

    virtual bool runOnBasicBlock(BasicBlock &BB) {
      SE = &getAnalysis<ScalarEvolution>();
      #if LLVM_VERSION_MINOR >= 5
        DataLayoutPass *DLP = getAnalysisIfAvailable<DataLayoutPass>();
        TD = DLP ? &DLP->getDataLayout() : nullptr;
      #else
        TD = getAnalysisIfAvailable<DataLayout>();
      #endif
      return optimizeLoadStore(BB);
    }
    Type    *getValueType(Value *insn);
    Value   *getPointerOperand(Value *I);
    unsigned getAddressSpace(Value *I);
    bool     isSimpleLoadStore(Value *I);
    bool     optimizeLoadStore(BasicBlock &BB);

    bool     isLoadStoreCompatible(Value *A, Value *B);
    void     mergeLoad(BasicBlock &BB, SmallVector<Instruction*, 4> &merged);
    void     mergeStore(BasicBlock &BB, SmallVector<Instruction*, 4> &merged);
    BasicBlock::iterator findConsecutiveAccess(BasicBlock &BB,
                                               SmallVector<Instruction*, 4> &merged,
                                               BasicBlock::iterator &start,
                                               unsigned maxLimit,
                                               bool isLoad);

    virtual const char *getPassName() const {
      return "Merge compatible Load/stores for Gen";
    }
  };

  char GenLoadStoreOptimization::ID = 0;

  Value *GenLoadStoreOptimization::getPointerOperand(Value *I) {
    if (LoadInst *LI = dyn_cast<LoadInst>(I)) return LI->getPointerOperand();
    if (StoreInst *SI = dyn_cast<StoreInst>(I)) return SI->getPointerOperand();
    return NULL;
  }
  unsigned GenLoadStoreOptimization::getAddressSpace(Value *I) {
    if (LoadInst *L=dyn_cast<LoadInst>(I)) return L->getPointerAddressSpace();
    if (StoreInst *S=dyn_cast<StoreInst>(I)) return S->getPointerAddressSpace();
    return -1;
  }
  bool GenLoadStoreOptimization::isSimpleLoadStore(Value *I) {
    if (LoadInst *L=dyn_cast<LoadInst>(I)) return L->isSimple();
    if (StoreInst *S=dyn_cast<StoreInst>(I)) return S->isSimple();
    return false;
  }
  Type *GenLoadStoreOptimization::getValueType(Value *insn) {
    if(LoadInst *ld = dyn_cast<LoadInst>(insn)) return ld->getType();
    if(StoreInst *st = dyn_cast<StoreInst>(insn)) return st->getValueOperand()->getType();

    return NULL;
  }

  bool GenLoadStoreOptimization::isLoadStoreCompatible(Value *A, Value *B) {
    Value *ptrA = getPointerOperand(A);
    Value *ptrB = getPointerOperand(B);
    unsigned ASA = getAddressSpace(A);
    unsigned ASB = getAddressSpace(B);

    // Check that the address spaces match and that the pointers are valid.
    if (!ptrA || !ptrB || (ASA != ASB)) return false;

    if(!isSimpleLoadStore(A) || !isSimpleLoadStore(B)) return false;
    // Check that A and B are of the same type.
    if (ptrA->getType() != ptrB->getType()) return false;

    // Calculate the distance.
    const SCEV *ptrSCEVA = SE->getSCEV(ptrA);
    const SCEV *ptrSCEVB = SE->getSCEV(ptrB);
    const SCEV *offsetSCEV = SE->getMinusSCEV(ptrSCEVA, ptrSCEVB);
    const SCEVConstant *constOffSCEV = dyn_cast<SCEVConstant>(offsetSCEV);

    // Non constant distance.
    if (!constOffSCEV) return false;

    int64_t offset = constOffSCEV->getValue()->getSExtValue();
    Type *Ty = cast<PointerType>(ptrA->getType())->getElementType();
    // The Instructions are connsecutive if the size of the first load/store is
    // the same as the offset.
    int64_t sz = TD->getTypeStoreSize(Ty);
    return ((-offset) == sz);
  }

  void GenLoadStoreOptimization::mergeLoad(BasicBlock &BB, SmallVector<Instruction*, 4> &merged) {
    IRBuilder<> Builder(&BB);

    unsigned size = merged.size();
    SmallVector<Value *, 4> values;
    for(unsigned i = 0; i < size; i++) {
      values.push_back(merged[i]);
    }
    LoadInst *ld = cast<LoadInst>(merged[0]);
    unsigned align = ld->getAlignment();
    unsigned addrSpace = ld->getPointerAddressSpace();
    // insert before first load
    Builder.SetInsertPoint(ld);
    VectorType *vecTy = VectorType::get(ld->getType(), size);
    Value *vecPtr = Builder.CreateBitCast(ld->getPointerOperand(),
                                          PointerType::get(vecTy, addrSpace));
    LoadInst *vecValue = Builder.CreateLoad(vecPtr);
    vecValue->setAlignment(align);

    for (unsigned i = 0; i < size; ++i) {
      Value *S = Builder.CreateExtractElement(vecValue, Builder.getInt32(i));
      values[i]->replaceAllUsesWith(S);
    }
  }

  BasicBlock::iterator
  GenLoadStoreOptimization::findConsecutiveAccess(BasicBlock &BB,
                            SmallVector<Instruction*, 4> &merged,
                            BasicBlock::iterator &start,
                            unsigned maxLimit,
                            bool isLoad) {

    BasicBlock::iterator stepForward = start;
    if(!isSimpleLoadStore(start)) return stepForward;

    merged.push_back(start);

    BasicBlock::iterator E = BB.end();
    BasicBlock::iterator J = ++start;

    for(unsigned ss = 0; J != E && ss <= maxLimit; ++ss, ++J) {
      if((isLoad && isa<LoadInst>(*J)) || (!isLoad && isa<StoreInst>(*J))) {
        if(isLoadStoreCompatible(merged[merged.size()-1], J)) {
          merged.push_back(J);
          stepForward = ++J;
        }
      } else if((isLoad && isa<StoreInst>(*J)) || (!isLoad && isa<LoadInst>(*J))) {
        // simple stop to keep read/write order
        break;
      }

      if(merged.size() >= 4) break;
    }
    return stepForward;
  }

  void GenLoadStoreOptimization::mergeStore(BasicBlock &BB, SmallVector<Instruction*, 4> &merged) {
    IRBuilder<> Builder(&BB);

    unsigned size = merged.size();
    SmallVector<Value *, 4> values;
    for(unsigned i = 0; i < size; i++) {
      values.push_back(cast<StoreInst>(merged[i])->getValueOperand());
    }
    StoreInst *st = cast<StoreInst>(merged[0]);
    unsigned addrSpace = st->getPointerAddressSpace();

    unsigned align = st->getAlignment();
    // insert before the last store
    Builder.SetInsertPoint(merged[size-1]);

    Type *dataTy = st->getValueOperand()->getType();
    VectorType *vecTy = VectorType::get(dataTy, size);
    Value * parent = UndefValue::get(vecTy);
    for(unsigned i = 0; i < size; i++) {
      parent = Builder.CreateInsertElement(parent, values[i], ConstantInt::get(IntegerType::get(st->getContext(), 32), i));
    }

    Value *newPtr = Builder.CreateBitCast(st->getPointerOperand(), PointerType::get(vecTy, addrSpace));
    StoreInst *newST = Builder.CreateStore(parent, newPtr);
    newST->setAlignment(align);
  }

  bool GenLoadStoreOptimization::optimizeLoadStore(BasicBlock &BB) {
    bool changed = false;
    SmallVector<Instruction*, 4> merged;
    for (BasicBlock::iterator BBI = BB.begin(), E = BB.end(); BBI != E;++BBI) {
      if(isa<LoadInst>(*BBI) || isa<StoreInst>(*BBI)) {
        bool isLoad = isa<LoadInst>(*BBI) ? true: false;
        Type *ty = getValueType(BBI);
        if(ty->isVectorTy()) continue;
        // we only support DWORD data type merge
        if(!ty->isFloatTy() && !ty->isIntegerTy(32)) continue;
        BBI = findConsecutiveAccess(BB, merged, BBI, 10, isLoad);
        if(merged.size() > 1) {
          if(isLoad)
            mergeLoad(BB, merged);
          else
            mergeStore(BB, merged);
          // remove merged insn
          int size = merged.size();
          for(int i = 0; i < size; i++)
            merged[i]->eraseFromParent();
          changed = true;
        }
        merged.clear();
      }
    }
    return changed;
  }

  BasicBlockPass *createLoadStoreOptimizationPass() {
    return new GenLoadStoreOptimization();
  }
};

