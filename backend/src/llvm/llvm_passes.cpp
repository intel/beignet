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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 *         Heldge RHodin <alice.rhodin@alice-dsl.net>
 */

/**
 * \file llvm_passes.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 * \author Heldge RHodin <alice.rhodin@alice-dsl.net>
 */

/* THIS CODE IS DERIVED FROM GPL LLVM PTX BACKEND. CODE IS HERE:
 * http://sourceforge.net/scm/?type=git&group_id=319085
 * Note that however, the original author, Heldge Rhodin, granted me (Benjamin
 * Segovia) the right to use another license for it (MIT here)
 */

#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Intrinsics.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/InlineAsm.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/ConstantsScanner.h"
#include "llvm/Analysis/FindUsedTypes.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/Target/Mangler.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#if !defined(LLVM_VERSION_MAJOR) || (LLVM_VERSION_MINOR == 1)
#include "llvm/Target/TargetData.h"
#endif
#include "llvm/Support/CallSite.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/GetElementPtrTypeIterator.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Config/config.h"

#include "llvm/llvm_gen_backend.hpp"
#include "ir/unit.hpp"
#include "sys/map.hpp"

using namespace llvm;

namespace gbe
{
  uint32_t getPadding(uint32_t offset, uint32_t align) {
    return (align - (offset % align)) % align; 
  }

  uint32_t getAlignmentByte(const ir::Unit &unit, Type* Ty)
  {
    const uint32_t MAX_ALIGN = 8; //maximum size is 8 for doubles

    switch (Ty->getTypeID()) {
      case Type::VoidTyID: NOT_SUPPORTED;
      case Type::VectorTyID:
      {
        const VectorType* VecTy = cast<VectorType>(Ty);
        uint32_t elemNum = VecTy->getNumElements();
        if (elemNum == 3) elemNum = 4; // OCL spec
        return elemNum * getTypeByteSize(unit, VecTy->getElementType());
      }
      case Type::PointerTyID:
      case Type::IntegerTyID:
      case Type::FloatTyID:
      case Type::DoubleTyID:
        return getTypeBitSize(unit, Ty)/8;
      case Type::ArrayTyID:
        return getAlignmentByte(unit, cast<ArrayType>(Ty)->getElementType());
      case Type::StructTyID:
      {
        const StructType* StrTy = cast<StructType>(Ty);
        uint32_t maxa = 0;
        for(uint32_t subtype = 0; subtype < StrTy->getNumElements(); subtype++)
        {
          maxa = std::max(getAlignmentByte(unit, StrTy->getElementType(subtype)), maxa);
          if(maxa==MAX_ALIGN)
            return maxa;
        }
        return maxa;
      }
      default: NOT_SUPPORTED;
    }
    return 0u;
  }

  uint32_t getTypeBitSize(const ir::Unit &unit, Type* Ty)
  {
    switch (Ty->getTypeID()) {
      case Type::VoidTyID:    NOT_SUPPORTED;
      case Type::PointerTyID: return unit.getPointerSize();
      case Type::IntegerTyID: return cast<IntegerType>(Ty)->getBitWidth();
      case Type::FloatTyID:   return 32;
      case Type::DoubleTyID:  return 64;
      case Type::VectorTyID:
      {
        const VectorType* VecTy = cast<VectorType>(Ty);
        return VecTy->getNumElements() * getTypeBitSize(unit, VecTy->getElementType());
      }
      case Type::ArrayTyID:
      {
        const ArrayType* ArrTy = cast<ArrayType>(Ty);
        Type* elementType = ArrTy->getElementType();
        uint32_t size_element = getTypeBitSize(unit, elementType);
        uint32_t size = ArrTy->getNumElements() * size_element;
        uint32_t align = 8 * getAlignmentByte(unit, elementType);
        size += (ArrTy->getNumElements()-1) * getPadding(size_element, align);
        return size;
      }
      case Type::StructTyID:
      {
        const StructType* StrTy = cast<StructType>(Ty);
        uint32_t size = 0;
        for(uint32_t subtype=0; subtype < StrTy->getNumElements(); subtype++)
        {
          Type* elementType = StrTy->getElementType(subtype);
          uint32_t align = 8 * getAlignmentByte(unit, elementType);
          size += getPadding(size, align);
          size += getTypeBitSize(unit, elementType);
        }
        return size;
      }
      default: NOT_SUPPORTED;
    }
    return 0u;
  }

  uint32_t getTypeByteSize(const ir::Unit &unit, Type* Ty)
  {
    uint32_t size_bit = getTypeBitSize(unit, Ty);
    assert((size_bit%8==0) && "no multiple of 8");
    return size_bit/8;
  }

  class GenRemoveGEPPasss : public BasicBlockPass
  {

   public:
    static char ID;
#define FORMER_VERSION 0
#if FORMER_VERSION
   GenRemoveGEPPasss(map<const Value *, const Value *>& 
                                       parentCompositePointer)
     : BasicBlockPass(ID),
     parentPointers(parentCompositePointer) {}
    map<const Value *, const Value *>& parentPointers;
#else
   GenRemoveGEPPasss(const ir::Unit &unit) :
     BasicBlockPass(ID),
     unit(unit) {}
  const ir::Unit &unit;
#endif
    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
    }

    virtual const char *getPassName() const {
      return "PTX backend: insert special ptx instructions";
    }

    bool simplifyGEPInstructions(GetElementPtrInst* GEPInst);

    virtual bool runOnBasicBlock(BasicBlock &BB)
    {
      bool changedBlock = false;
      iplist<Instruction>::iterator I = BB.getInstList().begin();
      for (auto nextI = I, E = --BB.getInstList().end(); I != E; I = nextI) {
        iplist<Instruction>::iterator I = nextI++;
        if(GetElementPtrInst* gep = dyn_cast<GetElementPtrInst>(&*I))
          changedBlock = (simplifyGEPInstructions(gep) || changedBlock);
      }
      return changedBlock;
    }
  };

  char GenRemoveGEPPasss::ID = 0;

  bool GenRemoveGEPPasss::simplifyGEPInstructions(GetElementPtrInst* GEPInst)
  {
    const uint32_t ptrSize = unit.getPointerSize();
    Value* parentPointer = GEPInst->getOperand(0);
#if FORMER_VERSION
    Value* topParent = parentPointer;
#endif
    CompositeType* CompTy = cast<CompositeType>(parentPointer->getType());

    if(isa<GlobalVariable>(parentPointer)) //HACK: !!!!
    {
#if 1//FORMER_VERSION
      Function *constWrapper = 
        Function::Create(FunctionType::get(parentPointer->getType(),true),
            GlobalValue::ExternalLinkage,
            Twine("__gen_ocl_const_wrapper"));

      llvm::ArrayRef<Value*> params(parentPointer);
      // params.push_back(parentPointer);

      //create and insert wrapper call
      CallInst * wrapperCall = 
        CallInst::Create(constWrapper,params,"",GEPInst);
      parentPointer = wrapperCall;
#else
      // NOT_IMPLEMENTED;
#endif
    }

    Value* currentAddrInst = 
      new PtrToIntInst(parentPointer, IntegerType::get(GEPInst->getContext(), ptrSize), "", GEPInst);

    uint32_t constantOffset = 0;

    for(uint32_t op=1; op<GEPInst->getNumOperands(); ++op)
    {
      uint32_t TypeIndex;
      //we have a constant struct/array acces
      if(ConstantInt* ConstOP = dyn_cast<ConstantInt>(GEPInst->getOperand(op)))
      {
        uint32_t offset = 0;
        TypeIndex = ConstOP->getZExtValue();
        for(uint32_t ty_i=0; ty_i<TypeIndex; ty_i++)
        {
          Type* elementType = CompTy->getTypeAtIndex(ty_i);
          uint32_t align = getAlignmentByte(unit, elementType);
          offset += getPadding(offset, align);
          offset += getTypeByteSize(unit, elementType);
        }

        //add getPaddingding for accessed type
        const uint32_t align = getAlignmentByte(unit, CompTy->getTypeAtIndex(TypeIndex));
        offset += getPadding(offset, align);

        constantOffset += offset;
      }
      // none constant index (=> only array/verctor allowed)
      else
      {
        // we only have array/vectors here, 
        // therefore all elements have the same size
        TypeIndex = 0;

        Type* elementType = CompTy->getTypeAtIndex(TypeIndex);
        uint32_t size = getTypeByteSize(unit, elementType);

        //add padding
        uint32_t align = getAlignmentByte(unit, elementType);
        size += getPadding(size, align);

        Constant* newConstSize = 
          ConstantInt::get(IntegerType::get(GEPInst->getContext(), ptrSize), size);

        Value *operand = GEPInst->getOperand(op); 

        //HACK TODO: Inserted by type replacement.. this code could break something????
        if(getTypeByteSize(unit, operand->getType())>4)
        {
          GBE_ASSERTM(false, "CHECK IT");
          operand->dump();

          //previous instruction is sext or zext instr. ignore it
          CastInst *cast = dyn_cast<CastInst>(operand);
          if(cast && (isa<ZExtInst>(operand) || isa<SExtInst>(operand)))
          {
            //hope that CastInst is a s/zext
            operand = cast->getOperand(0);
          }
          else
          {
            //trunctate
            operand = 
              new TruncInst(operand, 
                  IntegerType::get(GEPInst->getContext(), 
                    ptrSize), 
                  "", GEPInst);
          }
        }

        BinaryOperator* tmpMul = 
          BinaryOperator::Create(Instruction::Mul, newConstSize, operand,
              "", GEPInst);
        currentAddrInst = 
          BinaryOperator::Create(Instruction::Add, currentAddrInst, tmpMul,
              "", GEPInst);
      }

      //step down in type hirachy
      CompTy = dyn_cast<CompositeType>(CompTy->getTypeAtIndex(TypeIndex));
    }

    //insert addition of new offset before GEPInst
    Constant* newConstOffset = 
      ConstantInt::get(IntegerType::get(GEPInst->getContext(), 
            ptrSize),
          constantOffset);
    currentAddrInst = 
      BinaryOperator::Create(Instruction::Add, currentAddrInst, 
          newConstOffset, "", GEPInst);

    //convert offset to ptr type (nop)
    IntToPtrInst* intToPtrInst = 
      new IntToPtrInst(currentAddrInst,GEPInst->getType(),"", GEPInst);

    //replace uses of the GEP instruction with the newly calculated pointer
    GEPInst->replaceAllUsesWith(intToPtrInst);
    GEPInst->dropAllReferences();
    GEPInst->removeFromParent();

#if FORMER_VERSION
    //insert new pointer into parent list
    while(parentPointers.find(topParent)!=parentPointers.end())
      topParent = parentPointers.find(topParent)->second;
    parentPointers[intToPtrInst] = topParent;
#endif

    return true;
  }

  BasicBlockPass *createRemoveGEPPass(const ir::Unit &unit) {
    return new GenRemoveGEPPasss(unit);
  }
} /* namespace gbe */

