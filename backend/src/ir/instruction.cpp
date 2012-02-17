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

/**
 * \file instruction.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "ir/instruction.hpp"
#include "ir/function.hpp"

namespace gbe {
namespace ir {

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
        return RegisterIndex(0);
      }
    };

    /*! Use this when there is no destination */
    struct NoDstPolicy {
      INLINE uint32_t getDstNum(void) const { return 0; }
      INLINE RegisterIndex getDstIndex(const Function &fn, uint32_t ID) const {
        NOT_IMPLEMENTED;
        return RegisterIndex(0);
      }
    };

    /*! Policy shared by all the internal instructions */
    struct ALIGNED_INSTRUCTION BasePolicy {
      /*! Create an instruction from its internal representation */
      Instruction convert(void) const {
        return Instruction(reinterpret_cast<const char *>(&this->opcode));
      }
      /*! Instruction opcode */
      Opcode opcode;
    };

    /*! All unary and binary arithmetic instructions */
    template <uint32_t srcNum> // 1 or 2
    class NaryInstruction : public BasePolicy
    {
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
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      Type type;                //!< Type of the instruction
      RegisterIndex dst;        //!< Index of the register in the register file
      RegisterIndex src[srcNum];//!< Indices of the sources
    };

    /*! All 1-source arithmetic instructions */
    class UnaryInstruction : public NaryInstruction<1>
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
    class BinaryInstruction : public NaryInstruction<2>
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
    class TernaryInstruction : public BasePolicy
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
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      Type type;          //!< Type of the instruction
      RegisterIndex dst;  //!< Dst is the register index
      TupleIndex src;     //!< 3 sources do not fit in 8 bytes -> use a tuple
    };

    /*! Comparison instructions take two sources of the same type and return a
     *  boolean value. Since it is pretty similar to binary instruction, we
     *  steal all the methods from it, except wellFormed (dst register is always
     *  a boolean value)
     */
    class CompareInstruction : public BinaryInstruction
    {
    public:
      CompareInstruction(Type type,
                         CompareOperation operation,
                         RegisterIndex dst,
                         RegisterIndex src0,
                         RegisterIndex src1) :
        BinaryInstruction(OP_CMP, type, dst, src0, src1)
      {
        this->operation = operation;
      }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      CompareOperation operation;
    };

    class ConvertInstruction : public BasePolicy
    {
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
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      RegisterIndex dst;  //!< Converted value
      RegisterIndex src;  //!< To convert
      Type dstType;       //!< Type to convert to
      Type srcType;       //!< Type to convert from
    };

    class BranchInstruction : public BasePolicy, public NoDstPolicy
    {
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
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      RegisterIndex predicate;  //!< Predication means conditional branch
      LabelIndex labelIndex;    //!< Index of the label the branch targets
      bool hasPredicate;        //!< Is it predicated?
    };

    class LoadInstruction : public BasePolicy
    {
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
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      Type type;            //!< Type to store
      RegisterIndex offset; //!< First source is the offset where to store
      TupleIndex values;    //!< Values to load
      MemorySpace memSpace; //!< Where to store
      uint16_t valueNum;      //!< Number of values to store
    };

    class StoreInstruction : public BasePolicy, public NoDstPolicy
    {
    public:
      StoreInstruction(Type type,
                       TupleIndex values,
                       RegisterIndex offset,
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
      INLINE uint32_t getValueNum(void) const { return valueNum; }
      INLINE Type getValueType(void) const { return type; }
      INLINE MemorySpace getAddressSpace(void) const { return memSpace; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      Type type;            //!< Type to store
      RegisterIndex offset; //!< First source is the offset where to store
      TupleIndex values;    //!< Values to store
      MemorySpace memSpace; //!< Where to store
      uint16_t valueNum;    //!< Number of values to store
    };

    class TextureInstruction : public BasePolicy, public NoDstPolicy, public NoSrcPolicy // TODO REMOVE THIS
    {
    public:
      INLINE TextureInstruction(void) { this->opcode = OP_TEX; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
    };

    class LoadImmInstruction : public BasePolicy, public NoSrcPolicy
    {
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
      bool wellFormed(const Function &fn, std::string &why) const;
      RegisterIndex dst;    //!< Register to store into
      ValueIndex valueIndex;//!< Index in the vector of immediates
      Type type;            //!< Type of the immediate
    };

    class FenceInstruction : public BasePolicy, public NoSrcPolicy, public NoDstPolicy
    {
    public:
      INLINE FenceInstruction(MemorySpace memSpace) {
        this->opcode = OP_FENCE;
        this->memSpace = memSpace;
      }
      bool wellFormed(const Function &fn, std::string &why) const;
      MemorySpace memSpace; //!< The loads and stores to order
    };

    class LabelInstruction : public BasePolicy, public NoDstPolicy, public NoSrcPolicy
    {
    public:
      INLINE LabelInstruction(LabelIndex labelIndex) {
        this->opcode = OP_LABEL;
        this->labelIndex = labelIndex;
      }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      LabelIndex labelIndex;  //!< Index of the label
    };

#undef ALIGNED_INSTRUCTION

    /////////////////////////////////////////////////////////////////////////
    // Implements all the wellFormed methods
    /////////////////////////////////////////////////////////////////////////

    /*! All Nary instruction register must be of the same family and properly
     *  defined (i.e. not out-of-bound)
     */
    static INLINE bool checkRegister(Register::Family family,
                                     const RegisterIndex ID,
                                     const Function &fn,
                                     std::string &whyNot)
    {
      if (UNLIKELY(uint16_t(ID) >= fn.regNum())) {
        whyNot = "Out-of-bound destination register index";
        return false;
      }
      const Register reg = fn.getRegister(ID);
      if (UNLIKELY(reg.family != family)) {
        whyNot = "Destination family does not match instruction type";
        return false;
      }
      return true;
    }

    // Unary and binary instructions share the same rules
    template <uint32_t srcNum>
    INLINE bool NaryInstruction<srcNum>::wellFormed(const Function &fn, std::string &whyNot) const
    {
      const Register::Family family = getFamily(this->type);
      if (UNLIKELY(checkRegister(family, dst, fn, whyNot) == false))
        return false;
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID)
        if (UNLIKELY(checkRegister(family, src[srcID], fn, whyNot) == false))
          return false;
      return true;
    }

    // Idem for ternary instructions except that sources are in a tuple
    INLINE bool TernaryInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      const Register::Family family = getFamily(this->type);
      if (UNLIKELY(checkRegister(family, dst, fn, whyNot) == false))
        return false;
      if (UNLIKELY(src + 3u > fn.tupleNum())) {
        whyNot = "Out-of-bound index for ternary instruction";
        return false;
      }
      for (uint32_t srcID = 0; srcID < 3u; ++srcID) {
        const RegisterIndex regID = fn.getRegisterIndex(src, srcID);
        if (UNLIKELY(checkRegister(family, regID, fn, whyNot) == false))
          return false;
      }
      return true;
    }

    // Pretty similar to binary instruction. Only the destination is of type
    // boolean
    INLINE bool CompareInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY(checkRegister(Register::BOOL, dst, fn, whyNot) == false))
        return false;
      const Register::Family family = getFamily(this->type);
      for (uint32_t srcID = 0; srcID < 2; ++srcID)
        if (UNLIKELY(checkRegister(family, src[srcID], fn, whyNot) == false))
          return false;
      return true;
    }

    // We can convert anything to anything, but types and families must match
    INLINE bool ConvertInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      const Register::Family dstFamily = getFamily(srcType);
      const Register::Family srcFamily = getFamily(srcType);
      if (UNLIKELY(checkRegister(dstFamily, dst, fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegister(srcFamily, src, fn, whyNot) == false))
        return false;
      return true;
    }

    /*! Loads and stores follow the same restrictions */
    template <typename T>
    INLINE bool wellFormedLoadStore(const T &insn, const Function &fn, std::string &whyNot)
    {
      if (UNLIKELY(insn.offset >= fn.regNum())) {
        whyNot = "Out-of-bound offset register index";
        return false;
      }
      if (UNLIKELY(insn.values + insn.valueNum > fn.tupleNum())) {
        whyNot = "Out-of-bound tuple index";
        return false;
      }
      // Check all registers
      const Register::Family family = getFamily(insn.type);
      for (uint32_t valueID = 0; valueID < insn.valueNum; ++valueID) {
        const RegisterIndex regID = fn.getRegisterIndex(insn.values, valueID);
        if (UNLIKELY(checkRegister(family, regID, fn, whyNot) == false))
          return false;
      }
      return true;
    }

    INLINE bool LoadInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      return wellFormedLoadStore(*this, fn, whyNot);
    }

    INLINE bool StoreInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      return wellFormedLoadStore(*this, fn, whyNot);
    }

    // TODO
    INLINE bool TextureInstruction::wellFormed(const Function &fn, std::string &why) const
    {
      return true;
    }

    // Ensure that types and register family match
    INLINE bool LoadImmInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY(valueIndex >= fn.valueNum())) {
        whyNot = "Out-of-bound immediate value index";
        return false;
      }
      if (UNLIKELY(type != fn.getValue(valueIndex).type)) {
        whyNot = "Inconsistant type for the immediate value to load";
        return false;
      }
      const Register::Family family = getFamily(type);
      if (UNLIKELY(checkRegister(family, dst, fn, whyNot) == false))
        return false;
      return true;
    }

    // Nothing can go wrong here
    INLINE bool FenceInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      return true;
    }

    // Only a label index is required
    INLINE bool LabelInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY(labelIndex >= fn.labelNum())) {
        whyNot = "Out-of-bound label index";
        return false;
      }
      return true;
    }

    // The label must exist and the register must of boolean family
    INLINE bool BranchInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY(labelIndex >= fn.labelNum())) {
        whyNot = "Out-of-bound label index";
        return false;
      }
      if (hasPredicate)
        if (UNLIKELY(checkRegister(Register::BOOL, predicate, fn, whyNot) == false))
          return false;
      return true;
    }

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

#define START_INTROSPECTION(CLASS)                                            \
  static_assert(sizeof(CLASS)==sizeof(Instruction), "Bad instruction size");  \
  bool CLASS::isClassOf(const Instruction &insn) {                            \
    const Opcode op = insn.getOpcode();                                       \
    typedef CLASS RefClass;                                                   \
    switch (op) {

#define END_INTROSPECTION(CLASS)                                              \
      default: return false;                                                  \
    };                                                                        \
  }                                                                           \
  static_assert(offsetof(internal::CLASS, opcode)==0, "Bad opcode offset");

START_INTROSPECTION(UnaryInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(UnaryInstruction)

START_INTROSPECTION(BinaryInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(BinaryInstruction)

START_INTROSPECTION(TernaryInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(TernaryInstruction)

START_INTROSPECTION(ConvertInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(ConvertInstruction)

START_INTROSPECTION(BranchInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(BranchInstruction)

START_INTROSPECTION(TextureInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(TextureInstruction)

START_INTROSPECTION(LoadImmInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(LoadImmInstruction)

START_INTROSPECTION(LoadInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(LoadInstruction)

START_INTROSPECTION(StoreInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(StoreInstruction)

START_INTROSPECTION(FenceInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(FenceInstruction)

START_INTROSPECTION(LabelInstruction)
#include "ir/instruction.hxx"
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
#include "ir/instruction.hxx"
END_FUNCTION(Instruction, uint32_t)
#undef CALL

#define CALL getDstNum()
START_FUNCTION(Instruction, uint32_t, getDstNum(void))
#include "ir/instruction.hxx"
END_FUNCTION(Instruction, uint32_t)
#undef CALL

#define CALL getDstIndex(fn, ID)
START_FUNCTION(Instruction, RegisterIndex, getDstIndex(const Function &fn, uint32_t ID))
#include "ir/instruction.hxx"
END_FUNCTION(Instruction, RegisterIndex)
#undef CALL

#define CALL getSrcIndex(fn, ID)
START_FUNCTION(Instruction, RegisterIndex, getSrcIndex(const Function &fn, uint32_t ID))
#include "ir/instruction.hxx"
END_FUNCTION(Instruction, RegisterIndex)
#undef CALL

#define CALL wellFormed(fn, whyNot)
START_FUNCTION(Instruction, bool, wellFormed(const Function &fn, std::string &whyNot))
#include "ir/instruction.hxx"
END_FUNCTION(Instruction, bool)
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
#define DECL_EMIT_FUNCTION(NAME)                                      \
  Instruction NAME(Type type, RegisterIndex dst, RegisterIndex src) { \
    const internal::UnaryInstruction insn(OP_##NAME, type, dst, src); \
    return insn.convert();                                            \
  }

  DECL_EMIT_FUNCTION(MOV)
  DECL_EMIT_FUNCTION(COS)
  DECL_EMIT_FUNCTION(SIN)
  DECL_EMIT_FUNCTION(TAN)
  DECL_EMIT_FUNCTION(LOG)
  DECL_EMIT_FUNCTION(SQR)
  DECL_EMIT_FUNCTION(RSQ)

#undef DECL_EMIT_FUNCTION

  // All binary functions
#define DECL_EMIT_FUNCTION(NAME)                                              \
  Instruction NAME(Type type, RegisterIndex dst,                              \
                   RegisterIndex src0,                                        \
                   RegisterIndex src1) {                                      \
    const internal::BinaryInstruction insn(OP_##NAME, type, dst, src0, src1); \
    return insn.convert();                                                    \
  }

  DECL_EMIT_FUNCTION(MUL)
  DECL_EMIT_FUNCTION(ADD)
  DECL_EMIT_FUNCTION(SUB)
  DECL_EMIT_FUNCTION(DIV)
  DECL_EMIT_FUNCTION(REM)
  DECL_EMIT_FUNCTION(SHL)
  DECL_EMIT_FUNCTION(SHR)
  DECL_EMIT_FUNCTION(ASR)
  DECL_EMIT_FUNCTION(BSF)
  DECL_EMIT_FUNCTION(BSB)
  DECL_EMIT_FUNCTION(OR)
  DECL_EMIT_FUNCTION(XOR)
  DECL_EMIT_FUNCTION(AND)

#undef DECL_EMIT_FUNCTION

  // MAD
  Instruction MAD(Type type, RegisterIndex dst, TupleIndex src) {
    internal::TernaryInstruction insn(OP_MAD, type, dst, src);
    return insn.convert();
  }

  // CVT
  Instruction CVT(Type dstType, Type srcType, RegisterIndex dst, RegisterIndex src) {
    internal::ConvertInstruction insn(dstType, srcType, dst, src);
    return insn.convert();
  }

  // BRA
  Instruction BRA(LabelIndex labelIndex) {
    internal::BranchInstruction insn(labelIndex);
    return insn.convert();
  }
  Instruction BRA(LabelIndex labelIndex, RegisterIndex pred) {
    internal::BranchInstruction insn(labelIndex, pred);
    return insn.convert();
  }

  // LOADI
  Instruction LOADI(Type type, RegisterIndex dst, ValueIndex value) {
    internal::LoadImmInstruction insn(type, dst, value);
    return insn.convert();
  }

  // LOAD and STORE
#define DECL_EMIT_FUNCTION(NAME, CLASS)                               \
  Instruction NAME(Type type,                                         \
                   TupleIndex tuple,                                  \
                   RegisterIndex offset,                              \
                   MemorySpace space,                                 \
                   uint16_t valueNum)                                 \
  {                                                                   \
    const internal::CLASS insn(type, tuple, offset, space, valueNum); \
    return insn.convert();                                            \
  }

  DECL_EMIT_FUNCTION(LOAD, LoadInstruction)
  DECL_EMIT_FUNCTION(STORE, StoreInstruction)

#undef DECL_EMIT_FUNCTION

  // FENCE
  Instruction FENCE(MemorySpace space) {
    const internal::FenceInstruction insn(space);
    return insn.convert();
  }

  // LABEL
  Instruction LABEL(LabelIndex labelIndex) {
    const internal::LabelInstruction insn(labelIndex);
    return insn.convert();
  }

} /* namespace ir */
} /* namespace gbe */

