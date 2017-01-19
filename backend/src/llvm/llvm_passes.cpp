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

#include "llvm_includes.hpp"

#include "llvm/llvm_gen_backend.hpp"
#include "ir/unit.hpp"
#include "sys/map.hpp"

using namespace llvm;

namespace gbe
{
  bool isKernelFunction(const llvm::Function &F) {
    bool bKernel = false;
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9
    bKernel = F.getMetadata("kernel_arg_name") != NULL;
#else
    const Module *module = F.getParent();
    const Module::NamedMDListType& globalMD = module->getNamedMDList();
    for(auto i = globalMD.begin(); i != globalMD.end(); i++) {
      const NamedMDNode &md = *i;
      if(strcmp(md.getName().data(), "opencl.kernels") != 0) continue;
      uint32_t ops = md.getNumOperands();
      for(uint32_t x = 0; x < ops; x++) {
        MDNode* node = md.getOperand(x);
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 5
        Value * op = node->getOperand(0);
#else
        Value * op = cast<ValueAsMetadata>(node->getOperand(0))->getValue();
#endif
        if(op == &F) bKernel = true;
      }
    }
#endif
    return bKernel;
  }

  uint32_t getModuleOclVersion(const llvm::Module *M) {
    uint32_t oclVersion = 120;
    NamedMDNode *version = M->getNamedMetadata("opencl.ocl.version");
    if (version == NULL)
      return oclVersion;
    uint32_t ops = version->getNumOperands();
    if(ops > 0) {
      uint32_t major = 0, minor = 0;
      MDNode* node = version->getOperand(0);
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 6
      major = mdconst::extract<ConstantInt>(node->getOperand(0))->getZExtValue();
      minor = mdconst::extract<ConstantInt>(node->getOperand(1))->getZExtValue();
#else
      major = cast<ConstantInt>(node->getOperand(0))->getZExtValue();
      minor = cast<ConstantInt>(node->getOperand(1))->getZExtValue();
#endif
      oclVersion = major * 100 + minor * 10;
    }
    return oclVersion;
  }

  int32_t getPadding(int32_t offset, int32_t align) {
    return (align - (offset % align)) % align; 
  }

  uint32_t getAlignmentByte(const ir::Unit &unit, Type* Ty)
  {
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
      case Type::HalfTyID:
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
      case Type::IntegerTyID:
      {
        // use S16 to represent SLM bool variables.
        int bitWidth = cast<IntegerType>(Ty)->getBitWidth();
        return (bitWidth == 1) ? 16 : bitWidth;
      }
      case Type::HalfTyID:    return 16;
      case Type::FloatTyID:   return 32;
      case Type::DoubleTyID:  return 64;
      case Type::VectorTyID:
      {
        const VectorType* VecTy = cast<VectorType>(Ty);
        uint32_t numElem = VecTy->getNumElements();
        if(numElem == 3) numElem = 4; // OCL spec
        return numElem * getTypeBitSize(unit, VecTy->getElementType());
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

  int32_t getGEPConstOffset(const ir::Unit &unit, CompositeType *CompTy, int32_t TypeIndex) {
    int32_t offset = 0;
    SequentialType * seqType = dyn_cast<SequentialType>(CompTy);
    if (seqType != NULL) {
      if (TypeIndex != 0) {
        Type *elementType = seqType->getElementType();
        uint32_t elementSize = getTypeByteSize(unit, elementType);
        uint32_t align = getAlignmentByte(unit, elementType);
        elementSize += getPadding(elementSize, align);
        offset = elementSize * TypeIndex;
      }
    } else {
      int32_t step = TypeIndex > 0 ? 1 : -1;
      GBE_ASSERT(CompTy->isStructTy());
      for(int32_t ty_i=0; ty_i != TypeIndex; ty_i += step)
      {
        Type* elementType = CompTy->getTypeAtIndex(ty_i);
        uint32_t align = getAlignmentByte(unit, elementType);
        offset += getPadding(offset, align * step);
        offset += getTypeByteSize(unit, elementType) * step;
      }

      //add getPaddingding for accessed type
      const uint32_t align = getAlignmentByte(unit, CompTy->getTypeAtIndex(TypeIndex));
      offset += getPadding(offset, align * step);
    }
    return offset;
  }

  class GenRemoveGEPPasss : public BasicBlockPass
  {

   public:
    static char ID;
    GenRemoveGEPPasss(const ir::Unit &unit) :
      BasicBlockPass(ID),
      unit(unit) {}
    const ir::Unit &unit;
    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
    }

    virtual const char *getPassName() const {
      return "SPIR backend: insert special spir instructions";
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
    CompositeType* CompTy = parentPointer ? cast<CompositeType>(parentPointer->getType()) : NULL;
    if(!CompTy)
      return false;

    Value* currentAddrInst = 
      new PtrToIntInst(parentPointer, IntegerType::get(GEPInst->getContext(), ptrSize), "", GEPInst);

    int32_t constantOffset = 0;

    for(uint32_t op=1; op<GEPInst->getNumOperands(); ++op)
    {
      int32_t TypeIndex;
      ConstantInt* ConstOP = dyn_cast<ConstantInt>(GEPInst->getOperand(op));
      if (ConstOP != NULL) {
        TypeIndex = ConstOP->getZExtValue();
        constantOffset += getGEPConstOffset(unit, CompTy, TypeIndex);
      }
      else {
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

        if(!operand)
          continue;
#if 0
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
#endif
        Value* tmpMul = operand;
        if (size != 1) {
          tmpMul = BinaryOperator::Create(Instruction::Mul, newConstSize, operand,
                                         "", GEPInst);
        }
        currentAddrInst = 
          BinaryOperator::Create(Instruction::Add, currentAddrInst, tmpMul,
              "", GEPInst);
      }

      //step down in type hirachy
      CompTy = dyn_cast<CompositeType>(CompTy->getTypeAtIndex(TypeIndex));
    }

    //insert addition of new offset before GEPInst when it is not zero
    if (constantOffset != 0) {
      Constant* newConstOffset =
        ConstantInt::get(IntegerType::get(GEPInst->getContext(),
              ptrSize),
            constantOffset);
      currentAddrInst =
        BinaryOperator::Create(Instruction::Add, currentAddrInst,
            newConstOffset, "", GEPInst);
    }

    //convert offset to ptr type (nop)
    IntToPtrInst* intToPtrInst = 
      new IntToPtrInst(currentAddrInst,GEPInst->getType(),"", GEPInst);

    //replace uses of the GEP instruction with the newly calculated pointer
    GEPInst->replaceAllUsesWith(intToPtrInst);
    GEPInst->dropAllReferences();
    GEPInst->eraseFromParent();

    return true;
  }

  BasicBlockPass *createRemoveGEPPass(const ir::Unit &unit) {
    return new GenRemoveGEPPasss(unit);
  }
} /* namespace gbe */

