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
 * Author: Ruiling, Song <ruiling.song@intel.com>
 *
 * The Idea is that: As GEN support at most 4 successive DWORD load/store,
 * then merge successive load/store that are compatible is beneficial.
 * The method of checking whether two load/store is compatible are borrowed
 * from Vectorize passes in llvm.
 */

#include "llvm_includes.hpp"

using namespace llvm;
namespace gbe {
  class GenLoadStoreOptimization : public BasicBlockPass {

  public:
    static char ID;
    ScalarEvolution *SE;
    const DataLayout *TD;
    GenLoadStoreOptimization() : BasicBlockPass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const {
#if LLVM_VERSION_MAJOR == 3 &&  LLVM_VERSION_MINOR >= 8
      AU.addRequired<ScalarEvolutionWrapperPass>();
      AU.addPreserved<ScalarEvolutionWrapperPass>();
#else
      AU.addRequired<ScalarEvolution>();
      AU.addPreserved<ScalarEvolution>();
#endif
      AU.setPreservesCFG();
    }

    virtual bool runOnBasicBlock(BasicBlock &BB) {
#if LLVM_VERSION_MAJOR == 3 &&  LLVM_VERSION_MINOR >= 8
      SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
#else
      SE = &getAnalysis<ScalarEvolution>();
#endif
      #if LLVM_VERSION_MINOR >= 7
        TD = &BB.getModule()->getDataLayout();
      #elif LLVM_VERSION_MINOR >= 5
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
    void     mergeLoad(BasicBlock &BB, SmallVector<Instruction*, 16> &merged);
    void     mergeStore(BasicBlock &BB, SmallVector<Instruction*, 16> &merged);
    bool     findConsecutiveAccess(BasicBlock &BB,
                                  SmallVector<Instruction*, 16> &merged,
                                  const BasicBlock::iterator &start,
                                  unsigned maxVecSize,
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

  void GenLoadStoreOptimization::mergeLoad(BasicBlock &BB, SmallVector<Instruction*, 16> &merged) {
    IRBuilder<> Builder(&BB);

    unsigned size = merged.size();
    SmallVector<Value *, 16> values;
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
  // When searching for consecutive memory access, we do it in a small window,
  // if the window is too large, it would take up too much compiling time.
  // An Important rule we have followed is don't try to change load/store order.
  // But an exeption is 'load& store that are from different address spaces. The
  // return value will indicate wheter such kind of reorder happens.
  bool
  GenLoadStoreOptimization::findConsecutiveAccess(BasicBlock &BB,
                            SmallVector<Instruction*, 16> &merged,
                            const BasicBlock::iterator &start,
                            unsigned maxVecSize,
                            bool isLoad) {

    if(!isSimpleLoadStore(&*start)) return false;

    merged.push_back(&*start);
    unsigned targetAddrSpace = getAddressSpace(&*start);

    BasicBlock::iterator E = BB.end();
    BasicBlock::iterator J = start;
    ++J;

    unsigned maxLimit = maxVecSize * 8;
    bool reordered = false;

    for(unsigned ss = 0; J != E && ss <= maxLimit; ++ss, ++J) {
      if((isLoad && isa<LoadInst>(*J)) || (!isLoad && isa<StoreInst>(*J))) {
        if(isLoadStoreCompatible(merged[merged.size()-1], &*J)) {
          merged.push_back(&*J);
        }
      } else if((isLoad && isa<StoreInst>(*J))) {
        // simple stop to keep read/write order
        StoreInst *st = cast<StoreInst>(&*J);
        unsigned addrSpace = st->getPointerAddressSpace();
        if (addrSpace != targetAddrSpace) {
          reordered = true;
        } else {
          break;
        }
      } else if ((!isLoad && isa<LoadInst>(*J))) {
        LoadInst *ld = cast<LoadInst>(&*J);
        unsigned addrSpace = ld->getPointerAddressSpace();
        if (addrSpace != targetAddrSpace) {
          reordered = true;
        } else {
          break;
        }
      }

      if(merged.size() >= maxVecSize) break;
    }

    return reordered;
  }

  void GenLoadStoreOptimization::mergeStore(BasicBlock &BB, SmallVector<Instruction*, 16> &merged) {
    IRBuilder<> Builder(&BB);

    unsigned size = merged.size();
    SmallVector<Value *, 4> values;
    for(unsigned i = 0; i < size; i++) {
      values.push_back(cast<StoreInst>(merged[i])->getValueOperand());
    }
    StoreInst *st = cast<StoreInst>(merged[0]);
    if(!st)
      return;

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

    Value * stPointer = st->getPointerOperand();
    if(!stPointer)
      return;
    Value *newPtr = Builder.CreateBitCast(stPointer, PointerType::get(vecTy, addrSpace));
    StoreInst *newST = Builder.CreateStore(parent, newPtr);
    newST->setAlignment(align);
  }

  // Find the safe iterator we can point to. If reorder happens, we need to
  // point to the instruction after the first of toBeDeleted. If no reorder,
  // we are safe to point to the instruction after the last of toBeDeleted
  static BasicBlock::iterator
  findSafeInstruction(SmallVector<Instruction*, 16> &toBeDeleted,
                           const BasicBlock::iterator &current,
                           bool reorder) {
    BasicBlock::iterator safe = current;
    unsigned size = toBeDeleted.size();
    if (reorder) {
      unsigned i = 0;
      while (i < size && toBeDeleted[i] == &*safe) {
        ++i;
        ++safe;
      }
    } else {
      safe = BasicBlock::iterator(toBeDeleted[size - 1]);
      ++safe;
    }
    return safe;
  }

  bool GenLoadStoreOptimization::optimizeLoadStore(BasicBlock &BB) {
    bool changed = false;
    SmallVector<Instruction*, 16> merged;
    for (BasicBlock::iterator BBI = BB.begin(), E = BB.end(); BBI != E;++BBI) {
      if(isa<LoadInst>(*BBI) || isa<StoreInst>(*BBI)) {
        bool isLoad = isa<LoadInst>(*BBI) ? true: false;
        Type *ty = getValueType(&*BBI);
        if(!ty) continue;
        if(ty->isVectorTy()) continue;
        // TODO Support DWORD/WORD/BYTE LOAD for store support DWORD only now.
        if (!(ty->isFloatTy() || ty->isIntegerTy(32) ||
             ((ty->isIntegerTy(8) || ty->isIntegerTy(16)) && isLoad)))
          continue;

        unsigned maxVecSize = (ty->isFloatTy() || ty->isIntegerTy(32)) ? 4 :
                              (ty->isIntegerTy(16) ? 8 : 16);
        bool reorder = findConsecutiveAccess(BB, merged, BBI, maxVecSize, isLoad);
        uint32_t size = merged.size();
        uint32_t pos = 0;
        bool doDeleting = size > 1;
        if (doDeleting) {
          // choose next undeleted instruction
          BBI = findSafeInstruction(merged, BBI, reorder);
        }

        while(size > 1) {
          unsigned vecSize = (size >= 16) ? 16 :
                             (size >= 8 ? 8 :
                             (size >= 4 ? 4 : size));
          SmallVector<Instruction*, 16> mergedVec(merged.begin() + pos, merged.begin() + pos + vecSize);
          if(isLoad)
            mergeLoad(BB, mergedVec);
          else
            mergeStore(BB, mergedVec);
          // remove merged insn
          for(uint32_t i = 0; i < mergedVec.size(); i++)
            mergedVec[i]->eraseFromParent();
          changed = true;
          pos += vecSize;
          size -= vecSize;
        }
        if (doDeleting) {
          //adjust the BBI back by one, as we would increase it in for loop
          //don't do this if BBI points to the very first instruction.
          if (BBI != BB.begin())
            --BBI;
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

