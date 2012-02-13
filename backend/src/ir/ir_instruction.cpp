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
  // Implements the concrete implementations of the instruction classes. We just
  // cast an instruction to an internal class to run the given member function
  ///////////////////////////////////////////////////////////////////////////
  namespace internal
  {
#define ALIGNED_INSTRUCTION ALIGNED(AlignOf<Instruction>::value) 

    /*! Use this when there is no source */
    struct NoSrcPolicy {
      INLINE uint32_t getSrcNum(void) const { return 0; }
      INLINE RegisterIndex getSrcIndex(const Function &fn, uint32_t ID) const {
        NOT_IMPLEMENTED;
        return 0;
      }
    };

    /*! Use this when there is no destination */
    struct NoDstPolicy {
      INLINE uint32_t getDstNum(void) const { return 0; }
      INLINE RegisterIndex getDstIndex(const Function &fn, uint32_t ID) const {
        NOT_IMPLEMENTED;
        return 0;
      }
    };

    /*! All unary and binary arithmetic instructions */
    template <uint32_t srcNum> // 1 or 2
    class ALIGNED_INSTRUCTION NaryInstruction {
    public:
      INLINE uint32_t getSrcNum(void) const { return srcNum; }
      INLINE uint32_t getDstNum(void) const { return 1; }
      INLINE RegisterIndex getDstIndex(const Function &fn, uint32_t ID) const {
        assert(ID == 0);
        return dst;
      }
      INLINE RegisterIndex getSrcIndex(const Function &fn, uint32_t ID) const {
        assert(ID <= srcNum);
        return src[ID];
      }
      INLINE Type getType(void) const { return this->type; }
      Opcode opcode;            //!< Instruction opcode
      Type type;                //!< Type of the instruction
      RegisterIndex dst;        //!< Index of the register in the register file
      RegisterIndex src[srcNum];//!< Indices of the sources
    };

    /*! All 1-source arithmetic instructions */
    class ALIGNED_INSTRUCTION UnaryInstruction : public NaryInstruction<1> {
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
    class ALIGNED_INSTRUCTION BinaryInstruction : public NaryInstruction<2> {
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
    class ALIGNED_INSTRUCTION TernaryInstruction {
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
      INLINE uint32_t getSrcNum(void) const { return 3; }
      INLINE uint32_t getDstNum(void) const { return 1; }
      INLINE RegisterIndex getDstIndex(const Function &fn, uint32_t ID) const {
        assert(ID == 0);
        return dst;
      }
      INLINE RegisterIndex getSrcIndex(const Function &fn, uint32_t ID) const {
        assert(ID <= 3);
        return fn.getRegisterIndex(src, ID);
      }
      INLINE Type getType(void) const { return this->type; }
      Opcode opcode;      //!< Opcode of the instruction
      Type type;          //!< Type of the instruction
      RegisterIndex dst;  //!< Dst is the register index
      TupleIndex src;     //!< 3 sources do not fit in 8 bytes -> use a tuple
    };

    class ALIGNED_INSTRUCTION ConvertInstruction {
    public:
      ConvertInstruction(Type dstType,
                         Type srcType,
                         RegisterIndex dst,
                         RegisterIndex src)
      {
        this->opcode = OP_CVT;
        this->dst = dst;
        this->src = src;
        this->dstType = dstType;
        this->srcType = srcType;
      }
      INLINE Type getSrcType(void) const { return this->srcType; }
      INLINE Type getDstType(void) const { return this->dstType; }
      INLINE uint32_t getSrcNum(void) const { return 1; }
      INLINE uint32_t getDstNum(void) const { return 1; }
      INLINE RegisterIndex getDstIndex(const Function &fn, uint32_t ID) const {
        assert(ID == 0);
        return dst;
      }
      INLINE RegisterIndex getSrcIndex(const Function &fn, uint32_t ID) const {
        assert(ID == 0);
        return src;
      }
      Opcode opcode;      //!< Opcode of the instruction
      RegisterIndex dst;  //!< Converted value
      RegisterIndex src;  //!< To convert
      Type dstType;       //!< Type to convert to
      Type srcType;       //!< Type to convert from
    };

    class ALIGNED_INSTRUCTION BranchInstruction : public NoDstPolicy {
    public:
      INLINE BranchInstruction(LabelIndex labelIndex, RegisterIndex predicate) {
        this->opcode = OP_BRA;
        this->predicate = predicate;
        this->labelIndex = labelIndex;
        this->hasPredicate = true;
      }
      INLINE BranchInstruction(LabelIndex labelIndex) {
        this->opcode = OP_BRA;
        this->labelIndex = labelIndex;
        this->hasPredicate = false;
      }
      INLINE uint32_t getSrcNum(void) const { return hasPredicate ? 1 : 0; }
      INLINE RegisterIndex getSrcIndex(const Function &fn, uint32_t ID) const {
        assert(ID == 0 && hasPredicate);
        return predicate;
      }
      INLINE bool isPredicated(void) const { return hasPredicate; }
      Opcode opcode;            //!< Opcode of the instruction
      RegisterIndex predicate;  //!< Predication means conditional branch
      LabelIndex labelIndex;    //!< Index of the label the branch targets
      bool hasPredicate;        //!< Is it predicated?
    };

    class ALIGNED_INSTRUCTION LoadInstruction {
    public:
      LoadInstruction(Type type,
                      TupleIndex dstValues,
                      RegisterIndex offset,
                      MemorySpace memSpace,
                      uint16_t valueNum)
      {
        this->opcode = OP_STORE;
        this->type = type;
        this->offset = offset;
        this->values = dstValues;
        this->memSpace = memSpace;
        this->valueNum = valueNum;
      }
      INLINE RegisterIndex getSrcIndex(const Function &fn, uint32_t ID) const {
        assert(ID == 0u);
        return offset;
      }
      INLINE uint32_t getSrcNum(void) const { return 1; }
      INLINE RegisterIndex getDstIndex(const Function &fn, uint32_t ID) const {
        assert(ID < valueNum);
        return fn.getRegisterIndex(values, ID);
      }
      INLINE uint32_t getDstNum(void) const { return valueNum; }
      INLINE Type getValueType(void) const { return type; }
      INLINE uint32_t getValueNum(void) const { return valueNum; }
      INLINE MemorySpace getAddressSpace(void) const { return memSpace; }
      Opcode opcode;        //!< Opcode of the instruction
      Type type;            //!< Type to store
      RegisterIndex offset; //!< First source is the offset where to store
      TupleIndex values;    //!< Values to load
      MemorySpace memSpace; //!< Where to store
      uint16_t valueNum;      //!< Number of values to store
    };

    class ALIGNED_INSTRUCTION StoreInstruction : public NoDstPolicy {
    public:
      StoreInstruction(Type type,
                       RegisterIndex offset,
                       TupleIndex values,
                       MemorySpace memSpace,
                       uint16_t valueNum)
      {
        this->opcode = OP_STORE;
        this->type = type;
        this->offset = offset;
        this->values = values;
        this->memSpace = memSpace;
        this->valueNum = valueNum;
      }
      INLINE RegisterIndex getSrcIndex(const Function &fn, uint32_t ID) const {
        assert(ID < valueNum + 1u); // offset + values to store
        if (ID == 0u)
          return offset;
        else
          return fn.getRegisterIndex(values, ID - 1);
      }
      INLINE uint32_t getSrcNum(void) const { return valueNum + 1u; }
      INLINE Type getValueType(void) const { return type; }
      INLINE uint32_t getValueNum(void) const { return valueNum; }
      INLINE MemorySpace getAddressSpace(void) const { return memSpace; }
      Opcode opcode;        //!< Opcode of the instruction
      Type type;            //!< Type to store
      RegisterIndex offset; //!< First source is the offset where to store
      TupleIndex values;    //!< Values to store
      MemorySpace memSpace; //!< Where to store
      uint16_t valueNum;      //!< Number of values to store
    };

    class ALIGNED_INSTRUCTION TextureInstruction :
      public NoDstPolicy, public NoSrcPolicy // TODO REMOVE THIS
    {
    public:
      INLINE TextureInstruction(void) { this->opcode = OP_TEX; }
      Opcode opcode; //!< Opcode of the instruction
    };

    class ALIGNED_INSTRUCTION LoadImmInstruction : public NoSrcPolicy {
    public:
      INLINE LoadImmInstruction(Type type, RegisterIndex dst, ValueIndex valueIndex) {
        this->dst = dst;
        this->opcode = OP_LOADI;
        this->valueIndex = valueIndex;
        this->type = type;
      }
      INLINE Value getValue(const Function &fn) const {
        return fn.getValue(valueIndex);
      }
      INLINE uint32_t getDstNum(void) const{ return 1; }
      INLINE RegisterIndex getDstIndex(const Function &fn, uint32_t ID) const {
        assert(ID == 0);
        return dst;
      }
      INLINE Type getType(void) const { return this->type; }
      Opcode opcode;        //!< Opcode of the instruction
      RegisterIndex dst;    //!< Register to store into
      ValueIndex valueIndex;//!< Index in the vector of immediates
      Type type;            //!< Type of the immediate
    };

    class ALIGNED_INSTRUCTION FenceInstruction :
      public NoSrcPolicy, public NoDstPolicy
    {
    public:
      INLINE FenceInstruction(MemorySpace memSpace) {
        this->opcode = OP_FENCE;
        this->memSpace = memSpace;
      }
      Opcode opcode;        //!< Opcode of the instruction
      MemorySpace memSpace; //!< The loads and stores to order
    };

    class ALIGNED_INSTRUCTION LabelInstruction :
      public NoDstPolicy, public NoSrcPolicy
    {
    public:
      INLINE LabelInstruction(LabelIndex labelIndex) {
        this->opcode = OP_LABEL;
        this->labelIndex = labelIndex;
      }
      Opcode opcode;          //!< Opcode of the instruction
      LabelIndex labelIndex;  //!< Index of the label
    };

#undef ALIGNED_INSTRUCTION
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

  Register Instruction::getDst(const Function &fn, uint32_t ID) const {
    return fn.getRegister(this->getDstIndex(fn, ID));
  }
  Register Instruction::getSrc(const Function &fn, uint32_t ID) const {
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

  ///////////////////////////////////////////////////////////////////////////
  // Implements the function dispatching from public to internal with some
  // macro horrors
  ///////////////////////////////////////////////////////////////////////////

#define DECL_INSN(OPCODE, CLASS)               \
  case OP_##OPCODE: reinterpret_cast<const internal::CLASS*>(this)->CALL;

#define START_FUNCTION(CLASS, RET, PROTOTYPE)  \
  RET CLASS::PROTOTYPE const {                 \
    const Opcode op = this->getOpcode();       \
    switch (op) {

#define END_FUNCTION(CLASS, RET)              \
    };                                        \
    return RET();                             \
  }

#define CALL getSrcNum()
START_FUNCTION(Instruction, uint32_t, getSrcNum(void))
#include "ir_instruction.hxx"
END_FUNCTION(Instruction, uint32_t)
#undef CALL

#define CALL getDstNum()
START_FUNCTION(Instruction, uint32_t, getDstNum(void))
#include "ir_instruction.hxx"
END_FUNCTION(Instruction, uint32_t)
#undef CALL

#define CALL getDstIndex(fn, ID)
START_FUNCTION(Instruction, RegisterIndex, getDstIndex(const Function &fn, uint32_t ID))
#include "ir_instruction.hxx"
END_FUNCTION(Instruction, RegisterIndex)
#undef CALL

#define CALL getSrcIndex(fn, ID)
START_FUNCTION(Instruction, RegisterIndex, getSrcIndex(const Function &fn, uint32_t ID))
#include "ir_instruction.hxx"
END_FUNCTION(Instruction, RegisterIndex)
#undef CALL

#undef END_FUNCTION
#undef START_FUNCTION

#define DECL_MEM_FN(CLASS, RET, PROTOTYPE, CALL)                  \
  RET CLASS::PROTOTYPE const {                                    \
    return reinterpret_cast<const internal::CLASS*>(this)->CALL;  \
  }

DECL_MEM_FN(UnaryInstruction, Type, getType(void), getType())
DECL_MEM_FN(BinaryInstruction, Type, getType(void), getType())
DECL_MEM_FN(TernaryInstruction, Type, getType(void), getType())
DECL_MEM_FN(ConvertInstruction, Type, getSrcType(void), getSrcType())
DECL_MEM_FN(ConvertInstruction, Type, getDstType(void), getDstType())
DECL_MEM_FN(StoreInstruction, Type, getValueType(void), getValueType())
DECL_MEM_FN(StoreInstruction, uint32_t, getValueNum(void), getValueNum())
DECL_MEM_FN(StoreInstruction, MemorySpace, getAddressSpace(void), getAddressSpace())
DECL_MEM_FN(LoadInstruction, Type, getValueType(void), getValueType())
DECL_MEM_FN(LoadInstruction, uint32_t, getValueNum(void), getValueNum())
DECL_MEM_FN(LoadInstruction, MemorySpace, getAddressSpace(void), getAddressSpace())
DECL_MEM_FN(LoadImmInstruction, Value, getValue(const Function &fn), getValue(fn))
DECL_MEM_FN(LoadImmInstruction, Type, getType(void), getType())
DECL_MEM_FN(BranchInstruction, bool, isPredicated(void), isPredicated())

#undef DECL_MEM_FN

  ///////////////////////////////////////////////////////////////////////////
  // Implements the emission functions
  ///////////////////////////////////////////////////////////////////////////

  // All unary functions
#define DECL_EMIT_FUNCTION(NAME, OPCODE)\
  Instruction NAME(Type type, RegisterIndex dst, RegisterIndex src) {\
    internal::UnaryInstruction insn(OPCODE, type, dst, src);\
    return *reinterpret_cast<Instruction*>(&insn);\
  }

  DECL_EMIT_FUNCTION(mov, OP_MOV)
  DECL_EMIT_FUNCTION(cos, OP_COS)
  DECL_EMIT_FUNCTION(sin, OP_SIN)
  DECL_EMIT_FUNCTION(tan, OP_TAN)
  DECL_EMIT_FUNCTION(log, OP_LOG)
  DECL_EMIT_FUNCTION(sqr, OP_SQR)
  DECL_EMIT_FUNCTION(rsq, OP_RSQ)

#undef DECL_EMIT_FUNCTION

  // All binary functions
#define DECL_EMIT_FUNCTION(NAME, OPCODE)\
  Instruction NAME(Type type, RegisterIndex dst, RegisterIndex src0, RegisterIndex src1) {\
    internal::BinaryInstruction insn(OPCODE, type, dst, src0, src1);\
    return *reinterpret_cast<Instruction*>(&insn);\
  }

  DECL_EMIT_FUNCTION(mul, OP_MUL)
  DECL_EMIT_FUNCTION(add, OP_ADD)
  DECL_EMIT_FUNCTION(sub, OP_SUB)
  DECL_EMIT_FUNCTION(div, OP_DIV)
  DECL_EMIT_FUNCTION(rem, OP_REM)
  DECL_EMIT_FUNCTION(shl, OP_SHL)
  DECL_EMIT_FUNCTION(shr, OP_SHR)
  DECL_EMIT_FUNCTION(asr, OP_ASR)
  DECL_EMIT_FUNCTION(bsf, OP_BSF)
  DECL_EMIT_FUNCTION(bsb, OP_BSB)
  DECL_EMIT_FUNCTION(or$, OP_OR)
  DECL_EMIT_FUNCTION(xor$, OP_XOR)
  DECL_EMIT_FUNCTION(and$, OP_AND)

#undef DECL_EMIT_FUNCTION

  // MAD
  Instruction mad(Type type, RegisterIndex dst, TupleIndex src) {
    internal::TernaryInstruction insn(OP_MAD, type, dst, src);
    return *reinterpret_cast<Instruction*>(&insn);
  }

  // CVT
  Instruction cvt(Type dstType, Type srcType, RegisterIndex dst, TupleIndex src) {
    internal::ConvertInstruction insn(dstType, srcType, dst, src);
    return *reinterpret_cast<Instruction*>(&insn);
  }

  // BRA
  Instruction bra(LabelIndex labelIndex) {
    internal::BranchInstruction insn(labelIndex);
    return *reinterpret_cast<Instruction*>(&insn);
  }
  Instruction bra(LabelIndex labelIndex, RegisterIndex pred) {
    internal::BranchInstruction insn(labelIndex, pred);
    return *reinterpret_cast<Instruction*>(&insn);
  }

  // LOADI
  Instruction loadi(Type type, RegisterIndex dst, ValueIndex value) {
    internal::LoadImmInstruction insn(type, dst, value);
    return *reinterpret_cast<Instruction*>(&insn);
  }

  // LOAD and STORE
#define DECL_EMIT_FUNCTION(NAME, CLASS)   \
  Instruction NAME(Type type,             \
                   TupleIndex tuple,      \
                   RegisterIndex offset,  \
                   MemorySpace space,     \
                   uint16_t valueNum)       \
  {                                       \
    const internal::CLASS insn(type, tuple, offset, space, valueNum); \
    return *reinterpret_cast<const Instruction*>(&insn);              \
  }

  DECL_EMIT_FUNCTION(load, LoadInstruction)
  DECL_EMIT_FUNCTION(store, StoreInstruction)

#undef DECL_EMIT_FUNCTION

  // FENCE
  Instruction fence(MemorySpace space) {
    internal::FenceInstruction insn(space);
    return *reinterpret_cast<Instruction*>(&insn);
  }

  // LABEL
  Instruction label(LabelIndex labelIndex) {
    internal::LabelInstruction insn(labelIndex);
    return *reinterpret_cast<Instruction*>(&insn);
  }

} /* namespace gbe */

