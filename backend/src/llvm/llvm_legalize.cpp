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
 * Legalize unsupported integer data type i128/i256/...
 * right now, the implementation only consider little-endian system.
 *
 */
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"

#include "llvm/Config/llvm-config.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/IR/IRBuilder.h"
#if LLVM_VERSION_MINOR >= 5
#include "llvm/IR/CFG.h"
#else
#include "llvm/Support/CFG.h"
#endif


#include "llvm_gen_backend.hpp"

using namespace llvm;

namespace gbe {

  class Legalize : public FunctionPass {
  public:
    Legalize() : FunctionPass(ID) {
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 5
      initializeDominatorTreeWrapperPassPass(*PassRegistry::getPassRegistry());
#else
      initializeDominatorTreePass(*PassRegistry::getPassRegistry());
#endif
    }
    bool runOnFunction(Function& F) {
      if (!isKernelFunction(F)) return false;
      return legalizeFunction(F);
    }
    Value *getComponent(Value *v, uint32_t i, Type *ty);
    bool isIncomplete(Value *v);
    void legalizePHI(IRBuilder <> Builder, Instruction *p);
    void legalizeSelect(IRBuilder<> &Builder, Instruction *p);
    void legalizeICmp(IRBuilder<> &Builder, Instruction *p);
    void legalizeShl(IRBuilder<> &Builder, Instruction *p);
    void legalizeLShr(IRBuilder<> &Builder, Instruction *p);
    void legalizeAnd(IRBuilder<> &Builder, Instruction *p);
    void legalizeOr(IRBuilder<> &Builder, Instruction *p);
    void legalizeXor(IRBuilder<> &Builder, Instruction *p);
    void legalizeBitCast(IRBuilder<> &Builder, Instruction *p);
    void legalizeTrunc(IRBuilder<> &Builder, Instruction *p);
    void legalizeZExt(IRBuilder<> &Builder, Instruction *p);
    bool legalizeFunction(Function& F);
    void splitLargeInteger(APInt op, Type *splitTy, SmallVector<APInt, 16> &split);
    void splitConstantInt(ConstantInt *c, Type *splitTy, SmallVector<Value*, 16> &split);
    static char ID;
  private:
    std::set<Value *> processed;
    std::set<PHINode *> incompletePHIs;
    std::map<Value *, SmallVector<Value*, 16>> valueMap;
    typedef std::map<Value*, SmallVector<Value*, 16>>::iterator ValueMapIter;
  };

  void splitAPInt(APInt &data, SmallVectorImpl<APInt> &result, int totalBits, int subBits) {
    APInt lo = data.getLoBits(totalBits/2).trunc(totalBits/2);
    APInt hi = data.getHiBits(totalBits/2).trunc(totalBits/2);

    if (totalBits/2 <= subBits) {
      result.push_back(lo);
      result.push_back(hi);
      return;
    }
    splitAPInt(lo, result, totalBits/2, subBits);
    splitAPInt(hi, result, totalBits/2, subBits);
  }

  void Legalize::splitLargeInteger(APInt data, Type *splitTy, SmallVector<APInt, 16> &split) {
    unsigned opSz = data.getBitWidth();
    GBE_ASSERT(opSz > 7 && llvm::isPowerOf2_32(opSz));
    unsigned subSz = splitTy->getPrimitiveSizeInBits();
    splitAPInt(data, split, opSz, subSz);
  }

  void Legalize::splitConstantInt(ConstantInt *c, Type *splitTy, SmallVector<Value*, 16> &split) {
    SmallVector<APInt, 16> imm;
    splitLargeInteger(c->getValue(), splitTy, imm);
    for (unsigned i = 0; i < imm.size(); i++) {
      split.push_back(ConstantInt::get(splitTy, imm[i]));
    }
  }

  bool Legalize::isIncomplete(Value *v) {
    return valueMap.find(v) == valueMap.end() && !isa<ConstantInt>(v);
  }

  Value *Legalize::getComponent(Value *v, uint32_t i, Type *ty) {
    GBE_ASSERT(!isIncomplete(v));
    if (isa<ConstantInt>(v)) {
      GBE_ASSERT(ty);
      ConstantInt *CI = dyn_cast<ConstantInt>(v);
      SmallVector<APInt, 16> imm;
      splitLargeInteger(CI->getValue(), ty, imm);
      return ConstantInt::get(ty, imm[i]);
    }
    return valueMap.find(v)->second[i];
  }

  void Legalize::legalizePHI(IRBuilder <> Builder, Instruction *p) {
    PHINode *phi = dyn_cast<PHINode>(p);
    bool incomplete = false, allConst = true;
    uint32_t compNum = 0;
    Type *splitTy = NULL;
    for (unsigned int i = 0; i < phi->getNumIncomingValues(); ++i) {
      Value *val = phi->getIncomingValue(i);
      if (isIncomplete(val)) {
        incomplete = true;
        break;
      }
      if (allConst && valueMap.find(val) != valueMap.end()) {
        allConst = false;
        splitTy = valueMap.find(val)->second[0]->getType();
        compNum = valueMap.find(val)->second.size();
      }
    }

    if (incomplete) {
      // FIME, if a PHINode is totally incomplete which means
      // we don't even know the base type of this instruction.
      // Then it will be a little bit difficult to handle here.
      // Will do it in the future.
      incompletePHIs.insert(phi);
      GBE_ASSERT(0 && "unsupported PHI");
    }
    else {
      GBE_ASSERT(!allConst);
      SmallVector<Value*, 16> v;
      for (unsigned int i = 0; i < compNum; ++i) {
        PHINode* res = Builder.CreatePHI(splitTy, phi->getNumIncomingValues());

        // Loop over pairs of operands: [Value*, BasicBlock*]
        for (unsigned int j = 0; j < phi->getNumIncomingValues(); j++) {
          BasicBlock* bb = phi->getIncomingBlock(j);
          res->addIncoming(getComponent(phi->getIncomingValue(j), i, splitTy), bb);
        }
        v.push_back(res);
      }
      valueMap.insert(std::make_pair(phi, v));
    }
  }

  void Legalize::legalizeSelect(IRBuilder<> &Builder, Instruction *p) {
    SelectInst *sel = dyn_cast<SelectInst>(p);
    Value *op0 = sel->getOperand(0);
    Value *op1 = sel->getOperand(1);
    Value *op2 = sel->getOperand(2);

    ValueMapIter iter1 = valueMap.find(op1);
    ValueMapIter iter2 = valueMap.find(op2);
    SmallVector<Value*, 16> v;
    if (iter1 != valueMap.end() && iter2 != valueMap.end()) {
      SmallVectorImpl<Value*> &opVec1 = iter1->second;
      SmallVectorImpl<Value*> &opVec2 = iter2->second;

      GBE_ASSERT(opVec1.size() == opVec2.size());

      for (unsigned i = 0; i < opVec1.size(); i++) {
        Value *elemV = Builder.CreateSelect(op0, opVec1[i], opVec2[i]);
        v.push_back(elemV);
      }
    } else if (iter1 != valueMap.end()) {
      SmallVectorImpl<Value*> &opVec1 = iter1->second;
      Type *splitTy = opVec1[0]->getType();
      GBE_ASSERT(isa<ConstantInt>(op2));
      ConstantInt *CI = dyn_cast<ConstantInt>(op2);
      SmallVector<APInt, 16> imm;

      splitLargeInteger(CI->getValue(), splitTy, imm);
      for (unsigned i = 0; i < opVec1.size(); i++) {
        Value *elemV = Builder.CreateSelect(op0, opVec1[i], ConstantInt::get(splitTy, imm[i]));
        v.push_back(elemV);
      }
    } else if (iter2 != valueMap.end()) {
      SmallVectorImpl<Value*> &opVec2 = iter2->second;
      Type *splitTy = opVec2[0]->getType();
      GBE_ASSERT(isa<ConstantInt>(op1));
      ConstantInt *CI = dyn_cast<ConstantInt>(op1);
      SmallVector<APInt, 16> imm;

      splitLargeInteger(CI->getValue(), splitTy, imm);
      for (unsigned i = 0; i < opVec2.size(); i++) {
        Value *elemV = Builder.CreateSelect(op0, ConstantInt::get(splitTy, imm[i]), opVec2[i]) ;
        v.push_back(elemV);
      }
    } else {
      p->dump(); GBE_ASSERT(0 && "unsupported select.");
    }
    valueMap.insert(std::make_pair(p, v));
  }

  void Legalize::legalizeICmp(IRBuilder<> &Builder, Instruction *p) {
    ICmpInst *IC = dyn_cast<ICmpInst>(p);
    ICmpInst::Predicate pred = IC->getPredicate();
    // I could not figure out why llvm could generate some
    // compare instruction on large integers. so here only support equality check
    GBE_ASSERT(IC->isEquality());
    Value *op0 = p->getOperand(0);
    Value *op1 = p->getOperand(1);

    if (isa<ConstantInt>(op0)) {
      op0 = p->getOperand(1);
      op1 = p->getOperand(0);
    }

    if (isa<ConstantInt>(op1)) {
      ValueMapIter iter = valueMap.find(op0);
      SmallVectorImpl<Value*> &opVec = iter->second;
      SmallVector<APInt, 16> imm;

      Value *res = NULL;
      Type *splitTy = opVec[0]->getType();
      ConstantInt *CI = dyn_cast<ConstantInt>(op1);

      splitLargeInteger(CI->getValue(), splitTy, imm);
      for (unsigned i = 0; i < opVec.size(); i++) {
        Value *tmp = Builder.CreateICmp(pred, opVec[i], ConstantInt::get(splitTy, imm[i]));
        if (res != NULL) {
          if (pred == CmpInst::ICMP_EQ)
            tmp = Builder.CreateAnd(tmp, res);
          else
            tmp = Builder.CreateOr(tmp, res);
        }
        res = tmp;
      }
      p->replaceAllUsesWith(res);
    } else {
      ValueMapIter iter0 = valueMap.find(op0);
      ValueMapIter iter1 = valueMap.find(op1);
      SmallVectorImpl<Value*> &opVec0 = iter0->second;
      SmallVectorImpl<Value*> &opVec1 = iter1->second;

      Value *res = NULL;
      for (unsigned i = 0; i < opVec0.size(); i++) {
        Value *tmp = Builder.CreateICmp(pred, opVec0[i], opVec1[i]);
        if (res != NULL) {
          if (pred == CmpInst::ICMP_EQ)
            tmp = Builder.CreateAnd(tmp, res);
          else
            tmp = Builder.CreateOr(tmp, res);
        }
        res = tmp;
      }
      p->replaceAllUsesWith(res);
    }
  }

  void Legalize::legalizeShl(IRBuilder<> &Builder, Instruction *p) {
    // only support known bits shift
    GBE_ASSERT(isa<ConstantInt>(p->getOperand(1)));

    ValueMapIter iter = valueMap.find(p->getOperand(0));
    GBE_ASSERT(iter != valueMap.end());
    SmallVectorImpl<Value*> &v0 = iter->second;

    uint64_t shiftBits = dyn_cast<ConstantInt>(p->getOperand(1))->getZExtValue();
    Type *splitTy = v0[0]->getType();

    unsigned elemNum = v0.size();
    unsigned szSplit = splitTy->getPrimitiveSizeInBits();
    unsigned shift = shiftBits / szSplit;
    unsigned unaligned = shiftBits % szSplit;

    if (unaligned == 0) {
      SmallVector<Value*, 16> v1;
      // fill lower bits with zero
      for (unsigned i = 0; i < shift; i++) {
        v1.push_back(ConstantInt::get(splitTy, 0));
      }
      // do the shift
      for (unsigned j =0; j < elemNum - shift; j++)
        v1.push_back(v0[j]);

      valueMap.insert(std::make_pair(p, v1));
    } else {
      SmallVector<Value*, 16> v1;
      // fill lower bits with zero
      for (unsigned i = 0; i < shift; i++) {
        v1.push_back(ConstantInt::get(splitTy, 0));
      }
      // first one is special, shl is enough.
      v1.push_back(Builder.CreateShl(v0[0], unaligned));

      for (unsigned i = 0; i < elemNum - shift - 1; i++) {
        Value *t0 = Builder.CreateLShr(v0[i], ConstantInt::get(v0[0]->getType(), szSplit-unaligned));
        Value *t1 = Builder.CreateShl(v0[i + 1], ConstantInt::get(v0[i + 1]->getType(), unaligned));
        Value *t2 = Builder.CreateOr(t0, t1);
        v1.push_back(t2);
      }
      valueMap.insert(std::make_pair(p, v1));
    }
  }

  void Legalize::legalizeLShr(IRBuilder<> &Builder, Instruction *p) {
    Value *op0 = p->getOperand(0);
    Value *op1 = p->getOperand(1);
    SmallVector<Value*, 16> result;

    GBE_ASSERT(isa<ConstantInt>(p->getOperand(1)));

    ValueMapIter iter = valueMap.find(op0);
    GBE_ASSERT(iter != valueMap.end());
    SmallVectorImpl<Value*> &opVec = iter->second;

    unsigned szTotal = op1->getType()->getPrimitiveSizeInBits();
    unsigned elemNum = opVec.size();
    unsigned szSplit = szTotal / elemNum;
    int64_t shift = dyn_cast<ConstantInt>(op1)->getSExtValue();
    GBE_ASSERT(shift > 0);
    unsigned elemShift = shift / szSplit;
    unsigned unalign = shift % szSplit;

    if (unalign == 0) {
      // the shift bits is aligned with the split size
      Constant *zero = ConstantInt::getSigned(opVec[0]->getType(), 0);
      for (unsigned s = 0; s < elemNum - elemShift; s++)
        result.push_back(opVec[s + elemShift]);

      for (unsigned s = 0; s < elemShift; s++)
        result.push_back(zero);

      valueMap.insert(std::make_pair(p, result));
    } else {
      // not aligned case
      for (unsigned s = elemShift; s < elemNum-1; s++) {
        Value *t0 = Builder.CreateLShr(opVec[s], ConstantInt::get(opVec[s]->getType(), unalign));
        Value *t1 = Builder.CreateShl(opVec[s + 1], ConstantInt::get(opVec[s + 1]->getType(), szSplit - unalign));
        Value *t2 = Builder.CreateOr(t0, t1);
        result.push_back(t2);
      }
      // last element only need lshr
      result.push_back(Builder.CreateLShr(opVec[elemNum-1], ConstantInt::get(opVec[elemNum - 1]->getType(), unalign)));

      for (unsigned s = 0; s < elemShift; s++) {
        result.push_back(ConstantInt::getSigned(opVec[0]->getType(), 0));
      }
      valueMap.insert(std::make_pair(p, result));
    }
  }

  void Legalize::legalizeAnd(IRBuilder<> &Builder, Instruction *p) {
    Value *op0 = p->getOperand(0);
    Value *op1 = p->getOperand(1);

    if ((isa<UndefValue>(op0) || isa<UndefValue>(op1))) {
      // I meet some special case as below:
      //   %82 = zext i32 %81 to i512
      //   %mask148 = and i512 undef, -4294967296
      //   %ins149 = or i512 %mask148, %82
      // I don't know how to split this kind of i512 instruction in a good way,
      // to simplify the situation, I directly optimize it to zero.
      // And in later instructions like and/or/shr... that operates on
      // the value can be optimized.
      p->replaceAllUsesWith(ConstantInt::get(p->getType(), 0));
      return;
    }

    if ((isa<ConstantInt>(op0) && dyn_cast<ConstantInt>(op0)->isZero())
       || (isa<ConstantInt>(op1) && dyn_cast<ConstantInt>(op1)->isZero())) {
      // zero & anyValue  ==> zero
      p->replaceAllUsesWith(ConstantInt::get(p->getType(), 0));
      return;
    }

    if (isa<ConstantInt>(op0)) {
      op0 = p->getOperand(1);
      op1 = p->getOperand(0);
    }

    ValueMapIter iter = valueMap.find(op0);
    SmallVector<Value*, 16> v0 = iter->second;
    SmallVector<Value*, 16> v1;
    SmallVector<Value*, 16> v2;

    if (isa<ConstantInt>(op1)) {
      splitConstantInt(dyn_cast<ConstantInt>(op1), v0[0]->getType(), v1);
    } else {
      v1 = valueMap.find(op1)->second;
    }

    for (unsigned i = 0; i < v0.size(); i++) {
      ConstantInt *c0 = NULL, *c1 = NULL;
      if (isa<ConstantInt>(v0[i])) c0 = dyn_cast<ConstantInt>(v0[i]);
      if (isa<ConstantInt>(v1[i])) c1 = dyn_cast<ConstantInt>(v1[i]);

      if ((c0 &&c0->isZero()) || (c1 && c1->isZero())) {
        // zero & anyvalue ==> zero
        v2.push_back(ConstantInt::get(v0[i]->getType(), 0));
      } else if (c0 && c0->isMinusOne()) {
        // 1111s & anyvalue ==> anyvalue
        v2.push_back(v1[i]);
      } else if (c1 && c1->isMinusOne()) {
        // 1111s & anyvalue ==> anyvalue
        v2.push_back(v0[i]);
      } else {
        v2.push_back(Builder.CreateAnd(v0[i], v1[i]));
      }
    }
    valueMap.insert(std::make_pair(p, v2));
  }

  void Legalize::legalizeOr(IRBuilder<> &Builder, Instruction *p) {
    Value *op0 = p->getOperand(0);
    Value *op1 = p->getOperand(1);

    if (isa<ConstantInt>(op0)) {
      op0 = p->getOperand(1);
      op1 = p->getOperand(0);
    }

    if (isa<ConstantInt>(op1) && dyn_cast<ConstantInt>(op1)->isZero()) {
      ValueMapIter iter = valueMap.find(op0);
      valueMap.insert(std::make_pair(p, iter->second));
      return;
    }

    ValueMapIter iter = valueMap.find(op0);
    SmallVector<Value*, 16> v0 = iter->second;
    SmallVector<Value*, 16> v1;
    SmallVector<Value*, 16> v2;

    if (isa<ConstantInt>(op1)) {
      splitConstantInt(dyn_cast<ConstantInt>(op1), v0[0]->getType(), v1);
    } else {
      v1 = valueMap.find(op1)->second;
    }

    for (unsigned i = 0; i < v0.size(); i++) {
      ConstantInt *c0 = NULL, *c1 = NULL;
      if (isa<ConstantInt>(v0[i])) c0 = dyn_cast<ConstantInt>(v0[i]);
      if (isa<ConstantInt>(v1[i])) c1 = dyn_cast<ConstantInt>(v1[i]);

      if ((c0 &&c0->isZero())) {
        // zero | anyvalue ==> anyvalue
        v2.push_back(v1[i]);
      } else if (c1 && c1->isZero()) {
        // zero | anyvalue ==> anyvalue
        v2.push_back(v0[i]);
      } else if (c0 && c0->isMinusOne()) {
        // 1111 | anyvalue ==> 1111
        v2.push_back(c0);
      } else if (c1 && c1->isMinusOne()) {
        // 1111 | anyvalue ==> 1111
        v2.push_back(c1);
      } else {
        v2.push_back(Builder.CreateOr(v0[i], v1[i]));
      }
    }
    valueMap.insert(std::make_pair(p, v2));
  }

  void Legalize::legalizeXor(IRBuilder<> &Builder, Instruction *p) {
    Value *op0 = p->getOperand(0);
    Value *op1 = p->getOperand(1);

    if (isa<ConstantInt>(op0)) {
      op0 = p->getOperand(1);
      op1 = p->getOperand(0);
    }

    ValueMapIter iter = valueMap.find(op0);
    SmallVector<Value*, 16> v0 = iter->second;
    SmallVector<Value*, 16> v1;
    SmallVector<Value*, 16> v2;

    if (isa<ConstantInt>(op1)) {
      splitConstantInt(dyn_cast<ConstantInt>(op1), v0[0]->getType(), v1);
    } else {
      v1 = valueMap.find(op1)->second;
    }

    for (unsigned i = 0; i < v0.size(); i++) {
      v2.push_back(Builder.CreateXor(v0[i], v1[i]));
    }
    valueMap.insert(std::make_pair(p, v2));
  }

  void Legalize::legalizeBitCast(IRBuilder<> &Builder, Instruction *p) {
    SmallVector<Value*, 16> split;
    Type *dstTy = p->getType();
    Type *srcTy = dyn_cast<CastInst>(p)->getSrcTy();

    if(srcTy->isVectorTy()) {
      VectorType *vecTy = dyn_cast<VectorType>(srcTy);
      Type *splitTy = vecTy->getElementType();
      unsigned elements = srcTy->getPrimitiveSizeInBits()/splitTy->getPrimitiveSizeInBits();
      // bitcast large integer from vector, so we do extractElement to get split integer
      unsigned splitSz = splitTy->getPrimitiveSizeInBits();
      Value *src = p->getOperand(0);
      // if it is cast from <4 x float> to i128
      // we cast <4 x float> to <4 x i32> first
      if (!splitTy->isIntegerTy())
        src = Builder.CreateBitCast(src, VectorType::get(IntegerType::get(p->getContext(), splitSz), elements));

      for (unsigned i = 0; i < elements; i++) {
        Value *NV = Builder.CreateExtractElement(src,
                      ConstantInt::get(IntegerType::get(p->getContext(), 32), i));
        split.push_back(NV);
      }
      valueMap.insert(std::make_pair(p, split));
    } else if (dstTy->isVectorTy()) {
      //bitcast from large integer to vector, so we do insertElement to build the vector
      ValueMapIter iter = valueMap.find(p->getOperand(0));
      SmallVectorImpl<Value*> &opVec = iter->second;
      Type *splitTy = opVec[0]->getType();
      GBE_ASSERT(dstTy->getPrimitiveSizeInBits() % splitTy->getPrimitiveSizeInBits() == 0);
      GBE_ASSERT(dstTy->getPrimitiveSizeInBits() / splitTy->getPrimitiveSizeInBits() == opVec.size());
      Value *vec = NULL;
      Type *idxTy = IntegerType::get(p->getContext(), 32);
      for (unsigned i = 0; i < opVec.size(); ++i) {
        Value *tmp = vec ? vec : UndefValue::get(VectorType::get(splitTy, opVec.size()));
        Value *idx = ConstantInt::get(idxTy, i);
        vec = Builder.CreateInsertElement(tmp, opVec[i], idx);
      }
      Type *elemTy = cast<VectorType>(dstTy)->getElementType();
      if (elemTy == opVec[0]->getType())
        p->replaceAllUsesWith(vec);
      else {
        Value *newVec = Builder.CreateBitCast(vec, dstTy);
        p->replaceAllUsesWith(newVec);
      }
    } else {
      p->dump(); GBE_ASSERT(0 && "Unsupported bitcast");
    }
  }

  void Legalize::legalizeTrunc(IRBuilder<> &Builder, Instruction *p) {
    Type *dstTy = p->getType();

    ValueMapIter iter = valueMap.find(p->getOperand(0));
    SmallVector<Value*, 16> &opVec = iter->second;
    unsigned szSplit = opVec[0]->getType()->getPrimitiveSizeInBits();
    unsigned szResult = dstTy->getPrimitiveSizeInBits();

    if(szResult > szSplit) {
      // the needed bits is larger than what is already split,
      // we have to merge the split Value, use Shl/Or to do it.
      int endIdx = (szResult + szSplit-1 )/szSplit;
      Value * prev = ConstantInt::get(dstTy, 0);
      for (int i = endIdx - 1; i >=0; i--) {
        Value * res = Builder.CreateZExt(opVec[i], dstTy);
        if (i > 0)
          res = Builder.CreateShl(res, i*szSplit);
        prev = Builder.CreateOr(res, prev);
      }
      Value *newValue = Builder.CreateTrunc(prev, dstTy);
      p->replaceAllUsesWith(newValue);
    } else if (szResult == szSplit) {
      // same bit width, should use bitcast instead of trunc.
      Value *newValue = Builder.CreateBitCast(opVec[0], dstTy);
      p->replaceAllUsesWith(newValue);
    } else {
      // normal case, trunc to a shorter bit width
      Value *newValue = Builder.CreateTrunc(opVec[0], dstTy);
      p->replaceAllUsesWith(newValue);
    }
  }

  void Legalize::legalizeZExt(IRBuilder<> &Builder, Instruction *p) {
    SmallVector<Value*, 16> split;
    Type *dstTy = dyn_cast<CastInst>(p)->getDestTy();
    Type *srcTy = p->getOperand(0)->getType();
    int elements = dstTy->getPrimitiveSizeInBits() / srcTy->getPrimitiveSizeInBits();

    split.push_back(p->getOperand(0));
    for (int i = 0; i < elements - 1; i++)
      split.push_back(ConstantInt::getSigned(srcTy, 0));

    valueMap.insert(std::make_pair(p, split));
  }

  bool Legalize::legalizeFunction(Function &F) {
    bool changed = false;

    typedef ReversePostOrderTraversal<Function*> RPOTType;
    RPOTType rpot(&F);

    for (RPOTType::rpo_iterator bb = rpot.begin(), bbE = rpot.end(); bb != bbE; ++bb) {
      IRBuilder<> Builder(*bb);
      for (BasicBlock::iterator it = (*bb)->begin(), itE = (*bb)->end(); it != itE; ++it) {
        Instruction *insn = it;
        Type *ty = insn->getType();
        if(ty->isIntegerTy() && ty->getIntegerBitWidth() > 64) {
          // result is large integer, push back itself and its users
          changed = true;

          processed.insert(insn);

          for(Value::use_iterator iter = insn->use_begin(); iter != insn->use_end(); ++iter) {
            // After LLVM 3.5, use_iterator points to 'Use' instead of 'User', which is more straightforward.
          #if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR < 5)
            User *theUser = *iter;
          #else
            User *theUser = iter->getUser();
          #endif
            processed.insert(theUser);
          }
        }

        if(processed.empty() || processed.find(insn) == processed.end())
          continue;

        Builder.SetInsertPoint(insn);
        switch(insn->getOpcode()) {
          default: { insn->dump(); GBE_ASSERT(false && "Illegal instruction\n"); break;}
          case Instruction::PHI:
            legalizePHI(Builder, insn);
            break;
          case Instruction::Select:
            legalizeSelect(Builder, insn);
            break;
          case Instruction::ICmp:
            legalizeICmp(Builder, insn);
            break;

          case Instruction::Shl:
            legalizeShl(Builder, insn);
            break;

          case Instruction::LShr:
            legalizeLShr(Builder, insn);
            break;

          case Instruction::And:
            legalizeAnd(Builder, insn);
            break;

          case Instruction::Or:
            legalizeOr(Builder, insn);
            break;

          case Instruction::Xor:
            legalizeXor(Builder, insn);
            break;

          case Instruction::BitCast:
            legalizeBitCast(Builder, insn);
            break;

          case Instruction::Trunc:
            legalizeTrunc(Builder, insn);
            break;

          case Instruction::ZExt:
            legalizeZExt(Builder, insn);
            break;
        }
      }
    }

    for (Value *v : processed) {
      if (isa<Instruction>(v)) {
        dyn_cast<Instruction>(v)->dropAllReferences();
      }
    }

    for (Value *v : processed) {
      if (isa<Instruction>(v)) {
        dyn_cast<Instruction>(v)->eraseFromParent();
      }
    }

    processed.clear();
    valueMap.clear();
    incompletePHIs.clear();
    return changed;
  }

  FunctionPass* createLegalizePass() {
    return new Legalize();
  }
  char Legalize::ID = 0;
};
