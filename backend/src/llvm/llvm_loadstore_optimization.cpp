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
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
      AU.addRequired<ScalarEvolutionWrapperPass>();
      AU.addPreserved<ScalarEvolutionWrapperPass>();
#else
      AU.addRequired<ScalarEvolution>();
      AU.addPreserved<ScalarEvolution>();
#endif
      AU.setPreservesCFG();
    }

    virtual bool runOnBasicBlock(BasicBlock &BB) {
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
      SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
#else
      SE = &getAnalysis<ScalarEvolution>();
#endif
      #if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
        TD = &BB.getModule()->getDataLayout();
      #elif LLVM_VERSION_MINOR >= 5
        DataLayoutPass *DLP = getAnalysisIfAvailable<DataLayoutPass>();
        TD = DLP ? &DLP->getDataLayout() : nullptr;
      #else
        TD = getAnalysisIfAvailable<DataLayout>();
      #endif
      return optimizeLoadStore(BB);
    }
    Type *getValueType(Value *insn);
    Value *getPointerOperand(Value *I);
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

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
    virtual StringRef getPassName() const
#else
    virtual const char *getPassName() const
#endif
    {
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

  bool GenLoadStoreOptimization::isLoadStoreCompatible(Value *A, Value *B, int *dist, int* elementSize, int maxVecSize) {
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
    *dist = -offset;
    *elementSize = sz;

    //a insn with small distance from the search load/store is a candidate one
    return (abs(-offset) < sz*maxVecSize);
  }

  void GenLoadStoreOptimization::mergeLoad(BasicBlock &BB,
                                            SmallVector<Instruction*, 16> &merged,
                                            Instruction *first,
                                            int offset) {
    IRBuilder<> Builder(&BB);

    unsigned size = merged.size();
    SmallVector<Value *, 16> values;
    for(unsigned i = 0; i < size; i++) {
      values.push_back(merged[i]);
    }
    LoadInst *ld = cast<LoadInst>(first);
    unsigned align = ld->getAlignment();
    unsigned addrSpace = ld->getPointerAddressSpace();
    // insert before first load
    Builder.SetInsertPoint(ld);

    //modify offset
    Value *newPtr = ld->getPointerOperand();
    if(offset != 0)
    {
      Type *ptype = ld->getPointerOperand()->getType();
      unsigned typeSize = TD->getPointerTypeSize(ptype);
      ptype = (typeSize == 4) ? Builder.getInt32Ty():Builder.getInt64Ty();
      Value *StartAddr = Builder.CreatePtrToInt(ld->getPointerOperand(), ptype);
      Value *offsetVal = ConstantInt::get(ptype, offset);
      Value *newAddr = Builder.CreateAdd(StartAddr, offsetVal);
      newPtr = Builder.CreateIntToPtr(newAddr, ld->getPointerOperand()->getType());
    }

    VectorType *vecTy = VectorType::get(ld->getType(), size);
    Value *vecPtr = Builder.CreateBitCast(newPtr, PointerType::get(vecTy, addrSpace));
    LoadInst *vecValue = Builder.CreateLoad(vecPtr);
    vecValue->setAlignment(align);

    for (unsigned i = 0; i < size; ++i) {
      Value *S = Builder.CreateExtractElement(vecValue, Builder.getInt32(i));
      values[i]->replaceAllUsesWith(S);
    }
  }

  class mergedInfo{
    public:
    Instruction* mInsn;
    int mOffset;

    void init(Instruction* insn, int offset)
    {
      mInsn = insn;
      mOffset = offset;
    }
  };

  struct offsetSorter {
    bool operator()(mergedInfo* m0, mergedInfo* m1) const {
    return m0->mOffset < m1->mOffset;
    }
  };

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
                            bool isLoad,
                            int *addrOffset,
                            Instruction *&first,
                            Instruction *&last) {
    if(!isSimpleLoadStore(&*start)) return false;

    unsigned targetAddrSpace = getAddressSpace(&*start);

    BasicBlock::iterator E = BB.end();
    BasicBlock::iterator J = start;
    ++J;

    unsigned maxLimit = maxVecSize * 8;
    bool crossAddressSpace = false;
    // When we are merging loads and there are some other AddressSpace stores
    // lies among them, we are saying that loadStoreReorder happens. The same
    // for merging stores and there are some other AddressSpace load among them.
    bool loadStoreReorder = false;
    bool ready = false;
    int elementSize;

    SmallVector<mergedInfo *, 32> searchInsnArray;
    SmallVector<mergedInfo *, 32> orderedInstrs;
    mergedInfo meInfoArray[32];
    int indx = 0;
    meInfoArray[indx++].init(&*start, 0);
    searchInsnArray.push_back(&meInfoArray[0]);

    for(unsigned ss = 0; J!= E && ss <= maxLimit; ++ss, ++J) {
      if((isLoad && isa<LoadInst>(*J)) || (!isLoad && isa<StoreInst>(*J))) {
          int distance;
          if(isLoadStoreCompatible(searchInsnArray[0]->mInsn, &*J, &distance, &elementSize, maxVecSize))
          {
            meInfoArray[indx].init(&*J, distance);
            searchInsnArray.push_back(&meInfoArray[indx]);
            indx++;
            if (crossAddressSpace)
              loadStoreReorder = true;

            if(indx >= 32)
              break;
          }
      } else if((isLoad && isa<StoreInst>(*J))) {
        // simple stop to keep read/write order
        StoreInst *st = cast<StoreInst>(&*J);
        unsigned addrSpace = st->getPointerAddressSpace();
        if (addrSpace != targetAddrSpace) {
          crossAddressSpace = true;
        } else {
          break;
        }
      } else if ((!isLoad && isa<LoadInst>(*J))) {
        LoadInst *ld = cast<LoadInst>(&*J);
        unsigned addrSpace = ld->getPointerAddressSpace();
        if (addrSpace != targetAddrSpace) {
          crossAddressSpace = true;
        } else {
          break;
        }
      }
    }


    if(indx > 1)
    {
      first = (*searchInsnArray.begin())->mInsn;
      //try to sort the load/store by the offset from the start
      //the farthest is at the beginning. this is easy to find the
      //continuous load/store
      orderedInstrs = searchInsnArray;
      std::sort(searchInsnArray.begin(), searchInsnArray.end(), offsetSorter());

      // try to find continuous loadstore insn in the candidate array
      for (unsigned i = 0; i < searchInsnArray.size(); i++)
      {
        unsigned j;
        for(j = 0; j < maxVecSize-1 && (j+i+1) < searchInsnArray.size(); j++)
        {
          if(searchInsnArray[i+j+1]->mOffset - searchInsnArray[i+j]->mOffset != elementSize)
            break;

          //this means the search load/store which offset is 0, is in the sequence
          if(searchInsnArray[i+j]->mOffset == 0 || searchInsnArray[i+j+1]->mOffset == 0)
            ready = true;
        }

        if(j > 0 && ready)
        {
          unsigned endIndx = j + 1;
          *addrOffset = searchInsnArray[i]->mOffset;
          endIndx = (endIndx >= 16) ? 16 : (endIndx >= 8 ? 8 : (endIndx >= 4 ? 4 : endIndx));

          for(unsigned k = 0; k < endIndx; k++)
          {
            merged.push_back(searchInsnArray[i+k]->mInsn);
            if (k >= maxVecSize)
              break;
          }
          // find the last instruction if we are trying to merge STOREs.
          // we will later use it as the insertion point.
          if (!isLoad)
            for (auto insn = orderedInstrs.rbegin();
                 insn != orderedInstrs.rend(); ++insn) {
              if (std::find(merged.begin(), merged.end(), (*insn)->mInsn) !=
                  merged.end()) {
                last = (*insn)->mInsn;
                break;
              }
            }

          break;
        }
      }
    }

    return loadStoreReorder;
  }

  void GenLoadStoreOptimization::mergeStore(BasicBlock &BB,
                                            SmallVector<Instruction*, 16> &merged,
                                            Instruction *first,
                                            Instruction *last,
                                            int offset) {
    IRBuilder<> Builder(&BB);

    unsigned size = merged.size();
    SmallVector<Value *, 4> values;
    for(unsigned i = 0; i < size; i++) {
      values.push_back(cast<StoreInst>(merged[i])->getValueOperand());
    }
    StoreInst *st = cast<StoreInst>(first);
    if(!st)
      return;

    unsigned addrSpace = st->getPointerAddressSpace();

    unsigned align = st->getAlignment();
    // insert before the last store
    Builder.SetInsertPoint(last);

    Type *dataTy = st->getValueOperand()->getType();
    VectorType *vecTy = VectorType::get(dataTy, size);
    Value * parent = UndefValue::get(vecTy);
    for(unsigned i = 0; i < size; i++) {
      parent = Builder.CreateInsertElement(parent, values[i], ConstantInt::get(IntegerType::get(st->getContext(), 32), i));
    }

    Value * stPointer = st->getPointerOperand();
    if(!stPointer)
      return;

    //modify offset
    Value *newSPtr = stPointer;
    if(offset != 0)
    {
      unsigned typeSize = TD->getPointerTypeSize(stPointer->getType());
      Type *ptype = (typeSize == 4) ? Builder.getInt32Ty() : Builder.getInt64Ty();
      Value *StartAddr = Builder.CreatePtrToInt(stPointer, ptype);
      Value *offsetVal = ConstantInt::get(ptype, offset);
      Value *newAddr = Builder.CreateAdd(StartAddr, offsetVal);
      newSPtr = Builder.CreateIntToPtr(newAddr, stPointer->getType());
    }

    Value *newPtr = Builder.CreateBitCast(newSPtr, PointerType::get(vecTy, addrSpace));
    StoreInst *newST = Builder.CreateStore(parent, newPtr);
    newST->setAlignment(align);
  }

  // Find the safe iterator (will not be deleted after the merge) we can
  // point to. If reorder happens, we need to
  // point to the instruction after the first of toBeDeleted. If no reorder,
  // we are safe to point to the instruction after the last of toBeDeleted
  static BasicBlock::iterator
  findSafeInstruction(SmallVector<Instruction*, 16> &toBeDeleted,
                           const BasicBlock::iterator &current,
                           bool reorder) {
    BasicBlock::iterator safe = current;
    unsigned size = toBeDeleted.size();
    if (reorder) {
      BasicBlock *BB = &*current->getParent();
      for (; safe != BB->end(); ++safe) {
        if (std::find(toBeDeleted.begin(), toBeDeleted.end(), &*safe) ==
            toBeDeleted.end())
          break;
      }
    } else {
      // TODO we should use the furthest instruction, so that the outer loop
      // ends quicker.
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

        int addrOffset = 0;
        Instruction *first = nullptr, *last = nullptr;
        unsigned maxVecSize = (ty->isFloatTy() || ty->isIntegerTy(32)) ? 4 :
                              (ty->isIntegerTy(16) ? 8 : 16);
        bool reorder = findConsecutiveAccess(BB, merged, BBI, maxVecSize,
                                             isLoad, &addrOffset, first, last);
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
            mergeLoad(BB, mergedVec, first, addrOffset);
          else
            mergeStore(BB, mergedVec, first, last, addrOffset);
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

