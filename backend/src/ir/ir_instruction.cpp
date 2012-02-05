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
 */

#include "ir_instruction.hpp"
#include "ir_function.hpp"

namespace gbe
{
  ///////////////////////////////////////////////////////////////////////////
  // Implements the concrete implementations of the instruction classes
  ///////////////////////////////////////////////////////////////////////////
  namespace internal
  {
#define ALIGNED_INSTRUCTION ALIGNED(AlignOf<Instruction>::value) 

    /*! All unary and binary arithmetic instructions */
    template <uint32 srcNum> // 1 or 2
    class ALIGNED_INSTRUCTION NaryInstruction
    {
    public:
      INLINE uint32 getSrcNum(void) const { return srcNum; }
      INLINE uint32 getDstNum(void) const { return 1; }
      INLINE RegisterIndex getDstIndex(const Function &fn, uint32 ID) const {
        assert(ID == 0);
        return dst;
      }
      INLINE RegisterIndex getSrcIndex(const Function &fn, uint32 ID) const {
        assert(ID <= srcNum);
        return src[ID];
      }
      INLINE Type getType(void) const { return this->type; }
      Opcode opcode;      //!< Instruction opcode
      uint8 type;         //!< Type of the instruction
      uint16 dst;         //!< Index of the register in the register file
      uint16 src[srcNum]; //!< Indices of the sources
    };

    /*! All 1-source arithmetic instructions */
    class ALIGNED_INSTRUCTION UnaryInstruction : public NaryInstruction<1>
    {
    public:
      UnaryInstruction(Opcode opcode,
                       Type type,
                       RegisterIndex dst,
                       RegisterIndex src) {
        this->opcode = opcode;
        this->type = type;
        this->dst = dst;
        this->src[0] = src;
      }
    };

    /*! All 2-source arithmetic instructions */
    class ALIGNED_INSTRUCTION BinaryInstruction : public NaryInstruction<2>
    {
    public:
      BinaryInstruction(Opcode opcode,
                        Type type,
                        RegisterIndex dst,
                        RegisterIndex src0,
                        RegisterIndex src1) {
        this->opcode = opcode;
        this->type = type;
        this->dst = dst;
        this->src[0] = src0;
        this->src[1] = src1;
      }
    };

    /*! This is for MADs mostly. Since three sources cannot be encoded in 64
     *  bytes, we use tuples of registers
     */
    class ALIGNED_INSTRUCTION TernaryInstruction
    {
    public:
      TernaryInstruction(Opcode opcode,
                         Type type,
                         RegisterIndex dst,
                         TupleIndex src)
      {
        this->opcode = opcode;
        this->type = type;
        this->dst = dst;
        this->src = src;
      }
      INLINE uint32 getSrcNum(void) const { return 3; }
      INLINE uint32 getDstNum(void) const { return 1; }
      INLINE RegisterIndex getDstIndex(const Function &fn, uint32 ID) const {
        assert(ID == 0);
        return dst;
      }
      INLINE RegisterIndex getSrcIndex(const Function &fn, uint32 ID) const {
        assert(ID <= 3);
        return fn.getRegisterIndex(src, ID);
      }
      INLINE Type getType(void) const { return this->type; }
      Opcode opcode;      //!< Opcode of the instruction
      Type type;          //!< Type of the instruction
      RegisterIndex dst;  //!< Dst is the register index
      TupleIndex src;     //!< 3 sources do not fit in 8 bytes -> use a tuple
    };

    class ALIGNED_INSTRUCTION ConvertInstruction
    {
    public:
      ConvertInstruction(RegisterIndex dst,
                         RegisterIndex src,
                         Type dstType,
                         Type srcType)
      {
        this->opcode = OP_CVT;
        this->dst = dst;
        this->src = src;
        this->dstType = dstType;
        this->srcType = srcType;
      }
      INLINE Type getSrcType(void) const { return this->srcType; }
      INLINE Type getDstType(void) const { return this->dstType; }
      INLINE RegisterIndex getDstIndex(const Function &fn, uint32 ID) const {
        assert(ID == 0);
        return dst;
      }
      INLINE RegisterIndex getSrcIndex(const Function &fn, uint32 ID) const {
        assert(ID == 0);
        return src;
      }
      Opcode opcode;      //!< Opcode of the instruction
      RegisterIndex dst;  //!< Converted value
      RegisterIndex src;  //!< To convert
      Type dstType;       //!< Type to convert to
      Type srcType;       //!< Type to convert from
    };

    class ALIGNED_INSTRUCTION BranchInstruction
    {
    public:
      INLINE BranchInstruction(LabelIndex labelIndex, RegisterIndex predicate)
      {
        this->opcode = OP_BRA;
        this->predicate = predicate;
        this->labelIndex = labelIndex;
        this->hasPredicate = true;
      }
      INLINE BranchInstruction(LabelIndex labelIndex)
      {
        this->opcode = OP_BRA;
        this->labelIndex = labelIndex;
        this->hasPredicate = false;
      }
      INLINE bool isPredicated(void) const { return hasPredicate; }
      Opcode opcode;            //!< Opcode of the instruction
      RegisterIndex predicate;  //!< Predication means conditional branch
      LabelIndex labelIndex;    //!< Index of the label the branch targets
      bool hasPredicate;        //!< Is it predicated?
    };

    class ALIGNED_INSTRUCTION LoadInstruction
    {
    public:
      LoadInstruction(Type type,
                      RegisterIndex offset,
                      TupleIndex values,
                      MemorySpace memSpace,
                      uint16 valueNum)
      {
        this->opcode = OP_STORE;
        this->type = type;
        this->offset = offset;
        this->values = values;
        this->memSpace = memSpace;
        this->valueNum = valueNum;
      }
      INLINE RegisterIndex getSrcIndex(const Function &fn, uint32 ID) const {
        assert(ID == 0u);
        return offset;
      }
      INLINE RegisterIndex getDstIndex(const Function &fn, uint32 ID) const {
        assert(ID < valueNum);
        return fn.getRegisterIndex(values, ID);
      }
      INLINE uint32 getValueNum(void) const { return valueNum; }
      INLINE MemorySpace getAddressSpace(void) const { return memSpace; }
      Opcode opcode;        //!< Opcode of the instruction
      Type type;            //!< Type to store
      RegisterIndex offset; //!< First source is the offset where to store
      TupleIndex values;    //!< Values to load
      MemorySpace memSpace; //!< Where to store
      uint16 valueNum;      //!< Number of values to store
    };

    class ALIGNED_INSTRUCTION StoreInstruction
    {
    public:
      StoreInstruction(Type type,
                       RegisterIndex offset,
                       TupleIndex values,
                       MemorySpace memSpace,
                       uint16 valueNum)
      {
        this->opcode = OP_STORE;
        this->type = type;
        this->offset = offset;
        this->values = values;
        this->memSpace = memSpace;
        this->valueNum = valueNum;
      }
      INLINE RegisterIndex getSrcIndex(const Function &fn, uint32 ID) {
        assert(ID < valueNum + 1u); // offset + values to store
        if (ID == 0u)
          return offset;
        else
          return fn.getRegisterIndex(values, ID - 1);
      }
      INLINE RegisterIndex getDstIndex(const Function &fn, uint32 ID) {
        NOT_IMPLEMENTED;
        return 0u;
      }
      INLINE uint32 getValueNum(void) const { return valueNum; }
      INLINE MemorySpace getAddressSpace(void) const { return memSpace; }
      Opcode opcode;        //!< Opcode of the instruction
      Type type;            //!< Type to store
      RegisterIndex offset; //!< First source is the offset where to store
      TupleIndex values;    //!< Values to store
      MemorySpace memSpace; //!< Where to store
      uint16 valueNum;      //!< Number of values to store
    };

    class ALIGNED_INSTRUCTION TextureInstruction
    {
    public:
      INLINE TextureInstruction(void) { this->opcode = OP_TEX; }
      Opcode opcode; //!< Opcode of the instruction
    };

    class ALIGNED_INSTRUCTION LoadImmInstruction
    {
    public:
      INLINE LoadImmInstruction(ValueIndex valueIndex, Type type) {
        this->opcode = OP_LOADI;
        this->valueIndex = valueIndex;
        this->type = type;
      }
      INLINE Type getType(void) const { return this->type; }
      Opcode opcode;        //!< Opcode of the instruction
      ValueIndex valueIndex;//!< Index in the vector of immediates
      Type type;            //!< Type of the immediate
    };

    class ALIGNED_INSTRUCTION FenceInstruction
    {
    public:
      INLINE FenceInstruction(MemorySpace memSpace) {
        this->opcode = OP_FENCE;
        this->memSpace = memSpace;
      }
      Opcode opcode;        //!< Opcode of the instruction
      MemorySpace memSpace; //!< The loads and stores to order
    };

    class ALIGNED_INSTRUCTION LabelInstruction
    {
    public:
      INLINE LabelInstruction(LabelIndex labelIndex) {
        this->opcode = OP_LABEL;
        this->labelIndex = labelIndex;
      }
      Opcode opcode;          //!< Opcode of the instruction
      LabelIndex labelIndex;  //!< Index of the label
    };

  } /* namespace internal */

  ///////////////////////////////////////////////////////////////////////////
  // Implements the various instrospection functions
  ///////////////////////////////////////////////////////////////////////////
  template <typename T, typename U> struct HelperIntrospection {
    enum { value = 0 };
  };
  template <typename T> struct HelperIntrospection<T,T> {
    enum { value = 1 };
  };

  Register Instruction::getDst(const Function &fn, uint32 ID) const {
    return fn.getRegister(this->getDstIndex(fn, ID));
  }
  Register Instruction::getSrc(const Function &fn, uint32 ID) const {
    return fn.getRegister(this->getSrcIndex(fn, ID));
  }

#define DECL_INSN(OPCODE, CLASS)                           \
  case OP_##OPCODE:                                        \
  return HelperIntrospection<CLASS, RefClass>::value == 1;

#define START_INTROSPECTION(CLASS)                    \
  STATIC_ASSERT(sizeof(CLASS) == sizeof(Instruction));\
  bool CLASS::isClassOf(const Instruction &insn) {    \
    const Opcode op = insn.getOpcode();               \
    typedef CLASS RefClass;                           \
    switch (op) {

#define END_INTROSPECTION(CLASS)  \
      default: return false;      \
    };                            \
  }                               \
  STATIC_ASSERT(offsetof(internal::CLASS, opcode) == 0);

START_INTROSPECTION(UnaryInstruction)
#include "ir_instruction.hxx"
END_INTROSPECTION(UnaryInstruction)

START_INTROSPECTION(BinaryInstruction)
#include "ir_instruction.hxx"
END_INTROSPECTION(BinaryInstruction)

START_INTROSPECTION(TernaryInstruction)
#include "ir_instruction.hxx"
END_INTROSPECTION(TernaryInstruction)

START_INTROSPECTION(ConvertInstruction)
#include "ir_instruction.hxx"
END_INTROSPECTION(ConvertInstruction)

START_INTROSPECTION(BranchInstruction)
#include "ir_instruction.hxx"
END_INTROSPECTION(BranchInstruction)

START_INTROSPECTION(TextureInstruction)
#include "ir_instruction.hxx"
END_INTROSPECTION(TextureInstruction)

START_INTROSPECTION(LoadImmInstruction)
#include "ir_instruction.hxx"
END_INTROSPECTION(LoadImmInstruction)

START_INTROSPECTION(LoadInstruction)
#include "ir_instruction.hxx"
END_INTROSPECTION(LoadInstruction)

START_INTROSPECTION(StoreInstruction)
#include "ir_instruction.hxx"
END_INTROSPECTION(StoreInstruction)

START_INTROSPECTION(FenceInstruction)
#include "ir_instruction.hxx"
END_INTROSPECTION(FenceInstruction)

START_INTROSPECTION(LabelInstruction)
#include "ir_instruction.hxx"
END_INTROSPECTION(LabelInstruction)

#undef END_INTROSPECTION
#undef START_INTROSPECTION
#undef DECL_INSN
#undef ALIGNED_INSTRUCTION

#if 0
  ///////////////////////////////////////////////////////////////////////////
  // Implements the function dispatching from public to internal
  ///////////////////////////////////////////////////////////////////////////
#define DECL_INSN(OPCODE, CLASS)                           \
  case OP_##OPCODE: return cast<CLASS>(this)->getSrcNum();

  uint32 Instruction::getSrcNum(void) const {
    const Opcode op = this->getOpcode();
    switch (op) {
      #include "ir_instruction.hxx"
    };
    return 0;
  }
#endif
} /* namespace gbe */

