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
  // Implements the concrete implementations of the instruction classes. We
  // cast an instruction to an internal class to run the given member function
  ///////////////////////////////////////////////////////////////////////////
  namespace internal
  {
#define ALIGNED_INSTRUCTION ALIGNED(AlignOf<Instruction>::value) 

    /*! Policy shared by all the internal instructions */
    struct BasePolicy {
      /*! Create an instruction from its internal representation */
      Instruction convert(void) const {
        return Instruction(reinterpret_cast<const char *>(&this->opcode));
      }
      /*! Output the opcode in the given stream */
      INLINE void outOpcode(std::ostream &out) const {
        switch (opcode) {
#define DECL_INSN(OPCODE, CLASS) case OP_##OPCODE: out << #OPCODE; break;
#include "instruction.hxx"
#undef DECL_INSN
          case OP_INVALID: NOT_SUPPORTED; break;
        };
      }

      /*! Instruction opcode */
      Opcode opcode;
    };

    /*! For regular n source instructions */
    template <typename T, uint32_t srcNum> 
    struct NSrcPolicy {
      INLINE uint32_t getSrcNum(void) const { return srcNum; }
      INLINE Register getSrc(const Function &fn, uint32_t ID) const {
        GBE_ASSERTM(ID < srcNum, "Out-of-bound source");
        return static_cast<const T*>(this)->src[ID];
      }
      INLINE void setSrc(Function &fn, uint32_t ID, Register reg) {
        GBE_ASSERTM(ID < srcNum, "Out-of-bound source");
        static_cast<T*>(this)->src[ID] = reg;
      }
    };

    /*! For regular n destinations instructions */
    template <typename T, uint32_t dstNum>
    struct NDstPolicy {
      INLINE uint32_t getDstNum(void) const { return dstNum; }
      INLINE Register getDst(const Function &fn, uint32_t ID) const {
        GBE_ASSERTM(ID < dstNum, "Out-of-bound destination");
        return static_cast<const T*>(this)->dst[ID];
      }
      INLINE void setDst(Function &fn, uint32_t ID, Register reg) {
        GBE_ASSERTM(ID < dstNum, "Out-of-bound destination");
        static_cast<T*>(this)->dst[ID] = reg;
      }
    };

    /*! For instructions that use a tuple for source */
    template <typename T>
    struct TupleSrcPolicy {
      INLINE uint32_t getSrcNum(void) const {
        return static_cast<const T*>(this)->srcNum;
      }
      INLINE Register getSrc(const Function &fn, uint32_t ID) const {
        GBE_ASSERTM(ID < static_cast<const T*>(this)->srcNum, "Out-of-bound source register");
        return fn.getRegister(static_cast<const T*>(this)->src, ID);
      }
      INLINE void setSrc(Function &fn, uint32_t ID, Register reg) {
        GBE_ASSERTM(ID < static_cast<const T*>(this)->srcNum, "Out-of-bound source register");
        return fn.setRegister(static_cast<T*>(this)->src, ID, reg);
      }
    };

    /*! All unary and binary arithmetic instructions */
    template <uint32_t srcNum> // 1 or 2
    class ALIGNED_INSTRUCTION NaryInstruction :
      public BasePolicy,
      public NSrcPolicy<NaryInstruction<srcNum>, srcNum>,
      public NDstPolicy<NaryInstruction<1>, 1>
    {
    public:
      INLINE Type getType(void) const { return this->type; }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Type type;            //!< Type of the instruction
      Register dst[1];      //!< Index of the register in the register file
      Register src[srcNum]; //!< Indices of the sources
    };

    /*! All 1-source arithmetic instructions */
    class ALIGNED_INSTRUCTION UnaryInstruction : public NaryInstruction<1>
    {
    public:
      UnaryInstruction(Opcode opcode, Type type, Register dst, Register src) {
        this->opcode = opcode;
        this->type = type;
        this->dst[0] = dst;
        this->src[0] = src;
      }
    };

    /*! All 2-source arithmetic instructions */
    class ALIGNED_INSTRUCTION BinaryInstruction : public NaryInstruction<2>
    {
    public:
      BinaryInstruction(Opcode opcode,
                        Type type,
                        Register dst,
                        Register src0,
                        Register src1) {
        this->opcode = opcode;
        this->type = type;
        this->dst[0] = dst;
        this->src[0] = src0;
        this->src[1] = src1;
      }
      INLINE bool commutes(void) const {
        switch (opcode) {
          case OP_ADD:
          case OP_XOR:
          case OP_OR:
          case OP_AND:
          case OP_MUL:
            return true;
          default:
            return false;
        }
      }
    };

    /*! This is for MADs mostly. Since three sources cannot be encoded in 64
     *  bytes, we use tuples of registers
     */
    class ALIGNED_INSTRUCTION TernaryInstruction :
      public BasePolicy,
      public NDstPolicy<TernaryInstruction, 1>,
      public TupleSrcPolicy<TernaryInstruction>
    {
    public:
      TernaryInstruction(Opcode opcode, Type type, Register dst, Tuple src) {
        this->opcode = opcode;
        this->type = type;
        this->dst[0] = dst;
        this->src = src;
      }
      INLINE Type getType(void) const { return this->type; }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Type type;       //!< Type of the instruction
      Register dst[1]; //!< Dst is the register index
      Tuple src;       //!< 3 sources do not fit in 8 bytes -> use a tuple
      static const uint32_t srcNum = 3;
    };

    /*! As for MADs, three sources mean we need a tuple to encode it */
    class ALIGNED_INSTRUCTION SelectInstruction :
      public BasePolicy,
      public NDstPolicy<SelectInstruction, 1>,
      public TupleSrcPolicy<SelectInstruction>
    {
    public:
      SelectInstruction(Type type, Register dst, Tuple src) {
        this->opcode = OP_SEL;
        this->type = type;
        this->dst[0] = dst;
        this->src = src;
      }
      INLINE Type getType(void) const { return this->type; }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Type type;       //!< Type of the instruction
      Register dst[1]; //!< Dst is the register index
      Tuple src;       //!< 3 sources do not fit in 8 bytes -> use a tuple
      static const uint32_t srcNum = 3;
    };

    /*! Comparison instructions take two sources of the same type and return a
     *  boolean value. Since it is pretty similar to binary instruction, we
     *  steal all the methods from it, except wellFormed (dst register is always
     *  a boolean value)
     */
    class ALIGNED_INSTRUCTION CompareInstruction :
      public NaryInstruction<2>
    {
    public:
      CompareInstruction(Opcode opcode,
                         Type type,
                         Register dst,
                         Register src0,
                         Register src1)
      {
        this->opcode = opcode;
        this->type = type;
        this->dst[0] = dst;
        this->src[0] = src0;
        this->src[1] = src1;
      }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
    };

    class ALIGNED_INSTRUCTION ConvertInstruction :
      public BasePolicy,
      public NDstPolicy<ConvertInstruction, 1>,
      public NSrcPolicy<ConvertInstruction, 1>
    {
    public:
      ConvertInstruction(Type dstType,
                         Type srcType,
                         Register dst,
                         Register src)
      {
        this->opcode = OP_CVT;
        this->dst[0] = dst;
        this->src[0] = src;
        this->dstType = dstType;
        this->srcType = srcType;
      }
      INLINE Type getSrcType(void) const { return this->srcType; }
      INLINE Type getDstType(void) const { return this->dstType; }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Register dst[1];
      Register src[1];
      Type dstType; //!< Type to convert to
      Type srcType; //!< Type to convert from
    };

    class ALIGNED_INSTRUCTION BranchInstruction :
      public BasePolicy,
      public NDstPolicy<BranchInstruction, 0>
    {
    public:
      INLINE BranchInstruction(Opcode op, LabelIndex labelIndex, Register predicate) {
        GBE_ASSERT(op == OP_BRA);
        this->opcode = op;
        this->predicate = predicate;
        this->labelIndex = labelIndex;
        this->hasPredicate = true;
        this->hasLabel = true;
      }
      INLINE BranchInstruction(Opcode op, LabelIndex labelIndex) {
        GBE_ASSERT(op == OP_BRA);
        this->opcode = OP_BRA;
        this->labelIndex = labelIndex;
        this->hasPredicate = false;
        this->hasLabel = true;
      }
      INLINE BranchInstruction(Opcode op) {
        GBE_ASSERT(op == OP_RET);
        this->opcode = OP_RET;
        this->hasPredicate = false;
        this->hasLabel = false;
      }
      INLINE LabelIndex getLabelIndex(void) const {
        GBE_ASSERTM(hasLabel, "No target label for this branch instruction");
        return labelIndex;
      }
      INLINE uint32_t getSrcNum(void) const { return hasPredicate ? 1 : 0; }
      INLINE Register getSrc(const Function &fn, uint32_t ID) const {
        GBE_ASSERTM(hasPredicate, "No source for unpredicated branches");
        GBE_ASSERTM(ID == 0, "Only one source for the branch instruction");
        return predicate;
      }
      INLINE void setSrc(Function &fn, uint32_t ID, Register reg) {
        GBE_ASSERTM(hasPredicate, "No source for unpredicated branches");
        GBE_ASSERTM(ID == 0, "Only one source for the branch instruction");
        predicate = reg;
      }
      INLINE bool isPredicated(void) const { return hasPredicate; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Register predicate;    //!< Predication means conditional branch
      LabelIndex labelIndex; //!< Index of the label the branch targets
      bool hasPredicate:1;   //!< Is it predicated?
      bool hasLabel:1;       //!< Is there any target label?
      Register dst[];        //!< No destination
    };

    class ALIGNED_INSTRUCTION LoadInstruction :
      public BasePolicy,
      public NSrcPolicy<LoadInstruction, 1>
    {
    public:
      LoadInstruction(Type type,
                      Tuple dstValues,
                      Register offset,
                      AddressSpace addrSpace,
                      uint32_t valueNum,
                      bool dwAligned)
      {
        GBE_ASSERT(valueNum < 128);
        this->opcode = OP_LOAD;
        this->type = type;
        this->offset = offset;
        this->values = dstValues;
        this->addrSpace = addrSpace;
        this->valueNum = valueNum;
        this->dwAligned = dwAligned ? 1 : 0;
      }
      INLINE Register getDst(const Function &fn, uint32_t ID) const {
        GBE_ASSERTM(ID < valueNum, "Out-of-bound source register");
        return fn.getRegister(values, ID);
      }
      INLINE void setDst(Function &fn, uint32_t ID, Register reg) {
        GBE_ASSERTM(ID < valueNum, "Out-of-bound source register");
        fn.setRegister(values, ID, reg);
      }
      INLINE uint32_t getDstNum(void) const { return valueNum; }
      INLINE Type getValueType(void) const { return type; }
      INLINE uint32_t getValueNum(void) const { return valueNum; }
      INLINE AddressSpace getAddressSpace(void) const { return addrSpace; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      INLINE bool isAligned(void) const { return !!dwAligned; }
      Type type;              //!< Type to store
      Register src[];         //!< Address where to load from
      Register offset;        //!< Alias to make it similar to store
      Tuple values;           //!< Values to load
      AddressSpace addrSpace; //!< Where to load
      uint8_t valueNum:7;     //!< Number of values to load
      uint8_t dwAligned:1;    //!< DWORD aligned is what matters with GEN
    };

    class ALIGNED_INSTRUCTION StoreInstruction :
      public BasePolicy, public NDstPolicy<StoreInstruction, 0>
    {
    public:
      StoreInstruction(Type type,
                       Tuple values,
                       Register offset,
                       AddressSpace addrSpace,
                       uint32_t valueNum,
                       bool dwAligned)
      {
        GBE_ASSERT(valueNum < 255);
        this->opcode = OP_STORE;
        this->type = type;
        this->offset = offset;
        this->values = values;
        this->addrSpace = addrSpace;
        this->valueNum = valueNum;
        this->dwAligned = dwAligned ? 1 : 0;
      }
      INLINE Register getSrc(const Function &fn, uint32_t ID) const {
        GBE_ASSERTM(ID < valueNum + 1u, "Out-of-bound source register for store");
        if (ID == 0u)
          return offset;
        else
          return fn.getRegister(values, ID - 1);
      }
      INLINE void setSrc(Function &fn, uint32_t ID, Register reg) {
        GBE_ASSERTM(ID < valueNum + 1u, "Out-of-bound source register for store");
        if (ID == 0u)
          offset = reg;
        else
          fn.setRegister(values, ID - 1, reg);
      }
      INLINE uint32_t getSrcNum(void) const { return valueNum + 1u; }
      INLINE uint32_t getValueNum(void) const { return valueNum; }
      INLINE Type getValueType(void) const { return type; }
      INLINE AddressSpace getAddressSpace(void) const { return addrSpace; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      INLINE bool isAligned(void) const { return !!dwAligned; }
      Type type;              //!< Type to store
      Register offset;        //!< First source is the offset where to store
      Tuple values;           //!< Values to store
      AddressSpace addrSpace; //!< Where to store
      uint8_t valueNum:7;     //!< Number of values to store
      uint8_t dwAligned:1;    //!< DWORD aligned is what matters with GEN
      Register dst[];         //!< No destination
    };

    class ALIGNED_INSTRUCTION SampleInstruction : // TODO
      public BasePolicy,
      public NDstPolicy<SampleInstruction, 0>,
      public NSrcPolicy<SampleInstruction, 0>
    {
    public:
      INLINE SampleInstruction(void) { this->opcode = OP_SAMPLE; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const {
        this->outOpcode(out);
        out << " ... TODO";
      }
      Register dst[], src[];
    };

    class ALIGNED_INSTRUCTION TypedWriteInstruction : // TODO
      public BasePolicy,
      public NDstPolicy<TypedWriteInstruction, 0>,
      public NSrcPolicy<TypedWriteInstruction, 0>
    {
    public:
      INLINE TypedWriteInstruction(void) { this->opcode = OP_TYPED_WRITE; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const {
        this->outOpcode(out);
        out << " ... TODO";
      }
      Register dst[], src[];
    };

    class ALIGNED_INSTRUCTION LoadImmInstruction :
      public BasePolicy,
      public NSrcPolicy<LoadImmInstruction, 0>,
      public NDstPolicy<LoadImmInstruction, 1>
    {
    public:
      INLINE LoadImmInstruction(Type type, Register dst, ImmediateIndex index)
      {
        this->dst[0] = dst;
        this->opcode = OP_LOADI;
        this->immediateIndex = index;
        this->type = type;
      }
      INLINE Immediate getImmediate(const Function &fn) const {
        return fn.getImmediate(immediateIndex);
      }
      INLINE Type getType(void) const { return this->type; }
      bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Register dst[1];               //!< RegisterData to store into
      Register src[];                //!< No source register
      ImmediateIndex immediateIndex; //!< Index in the vector of immediates
      Type type;                     //!< Type of the immediate
    };

    class ALIGNED_INSTRUCTION FenceInstruction :
      public BasePolicy,
      public NSrcPolicy<FenceInstruction, 0>,
      public NDstPolicy<FenceInstruction, 0>
    {
    public:
      INLINE FenceInstruction(AddressSpace addrSpace) {
        this->opcode = OP_FENCE;
        this->addrSpace = addrSpace;
      }
      bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const {
        this->outOpcode(out);
        out << "." << addrSpace;
      }
      AddressSpace addrSpace; //!< The loads and stores to order
      Register dst[], src[];
    };

    class ALIGNED_INSTRUCTION LabelInstruction :
      public BasePolicy,
      public NSrcPolicy<LabelInstruction, 0>,
      public NDstPolicy<LabelInstruction, 0>
    {
    public:
      INLINE LabelInstruction(LabelIndex labelIndex) {
        this->opcode = OP_LABEL;
        this->labelIndex = labelIndex;
      }
      INLINE LabelIndex getLabelIndex(void) const { return labelIndex; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      LabelIndex labelIndex;  //!< Index of the label
      Register dst[], src[];
    };

    class ALIGNED_INSTRUCTION RegionInstruction :
      public BasePolicy,
      public NDstPolicy<RegionInstruction, 1>,
      public TupleSrcPolicy<RegionInstruction>
    {
    public:
      RegionInstruction(Register dst,
                        Tuple src,
                        uint32_t srcNum,
                        uint32_t offset,
                        uint32_t vstride,
                        uint32_t width,
                        uint32_t hstride)
      {
        this->opcode = OP_REGION;
        this->dst[0] = dst;
        this->src = src;
        this->srcNum = srcNum;
        this->offset = offset;
        this->vstride = vstride;
        this->width = width;
        this->hstride = hstride;
      }
      INLINE uint32_t getOffset(void) const { return this->offset; }
      INLINE uint32_t getVStride(void) const { return this->vstride; }
      INLINE uint32_t getWidth(void) const { return this->width; }
      INLINE uint32_t getHStride(void) const { return this->hstride; }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      uint8_t srcNum:4;   //!< Number of sources in the tuple
      uint8_t width:4;    //!< width (1,2,4 or 8)
      Register dst[1];    //!< Dst is the register index
      Tuple src;          //!< Contiguous registers we gather data from
      uint16_t vstride:5; //!< vertical stride (0,1,2,4,8 or 16)
      uint16_t hstride:5; //!< horizontal stride (0,1,2,4,8 or 16)
      uint16_t offset:5;  //!< offset (0 to 7)
    };

    class ALIGNED_INSTRUCTION VoteInstruction :
      public BasePolicy,
      public NDstPolicy<VoteInstruction, 1>,
      public NSrcPolicy<VoteInstruction, 1>
    {
    public:
      VoteInstruction(Register dst, Register src, VotePredicate pred) {
        this->opcode = OP_VOTE;
        this->dst[0] = dst;
        this->src[0] = src;
        this->pred = pred;
      }
      INLINE VotePredicate getVotePredicate(void) const { return this->pred; }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Register dst[1];    //!< Destination boolean
      Register src[1];    //!< Source boolean (n lanes internally)
      VotePredicate pred; //!< Operation to apply on the lanes
    };

    class ALIGNED_INSTRUCTION RGatherInstruction :
      public BasePolicy,
      public NDstPolicy<RGatherInstruction, 1>,
      public TupleSrcPolicy<RGatherInstruction>
    {
    public:
      RGatherInstruction(Register dst, Tuple src, uint32_t srcNum) {
        this->opcode = OP_RGATHER;
        this->dst[0] = dst;
        this->src = src;
        this->srcNum = srcNum;
      }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      uint8_t srcNum:4;   //!< Number of sources in the tuple
      Register dst[1];    //!< Dst is the register index
      Tuple src;          //!< Contiguous registers we gather data from
    };

    class ALIGNED_INSTRUCTION OBReadInstruction :
      public BasePolicy,
      public NDstPolicy<OBReadInstruction, 1>,
      public NSrcPolicy<OBReadInstruction, 1>
    {
    public:
      OBReadInstruction(Register value, Register address) {
        this->opcode = OP_OBREAD;
        this->dst[0] = value;
        this->src[0] = address;
      }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Register dst[1]; //!< Value to get from memory
      Register src[1]; //!< Address to read
    };

    class ALIGNED_INSTRUCTION OBWriteInstruction :
      public BasePolicy,
      public NDstPolicy<OBWriteInstruction, 0>,
      public NSrcPolicy<OBWriteInstruction, 2>
    {
    public:
      OBWriteInstruction(Register address, Register value) {
        this->opcode = OP_OBWRITE;
        this->src[0] = address;
        this->src[1] = value;
      }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Register src[2];  // src[0] == address, src[1] == value
      Register dst[0];  //!< No destination
    };

#undef ALIGNED_INSTRUCTION

    /////////////////////////////////////////////////////////////////////////
    // Implements all the wellFormed methods
    /////////////////////////////////////////////////////////////////////////

    /*! All Nary instruction registers must be of the same family and properly
     *  defined (i.e. not out-of-bound)
     */
    static INLINE bool checkRegisterData(RegisterFamily family,
                                         const Register &ID,
                                         const Function &fn,
                                         std::string &whyNot)
    {
      if (UNLIKELY(uint16_t(ID) >= fn.regNum())) {
        whyNot = "Out-of-bound destination register index";
        return false;
      }
      const RegisterData reg = fn.getRegisterData(ID);
      if (UNLIKELY(reg.family != family)) {
        whyNot = "Destination family does not match instruction type";
        return false;
      }
      return true;
    }

    /*! Special registers are *not* writeable */
    static INLINE bool checkSpecialRegForWrite(const Register &reg,
                                               const Function &fn,
                                               std::string &whyNot)
    {
      if (fn.isSpecialReg(reg) == true && reg != ir::ocl::stackptr) {
        whyNot = "Non stack pointer special registers are not writeable";
        return false;
      }
      return true;
    }

    /*! We check that the given type belongs to the provided type family */
    static INLINE bool checkTypeFamily(const Type &type,
                                       const Type *family,
                                       uint32_t typeNum,
                                       std::string &whyNot)
    {
      uint32_t typeID = 0;
      for (; typeID < typeNum; ++typeID)
        if (family[typeID] == type)
          break;
      if (typeID == typeNum) {
        whyNot = "Type is not supported by the instruction";
        return false;
      }
      return true;
    }

#define CHECK_TYPE(TYPE, FAMILY) \
  do { \
    if (UNLIKELY(checkTypeFamily(TYPE, FAMILY, FAMILY##Num, whyNot)) == false) \
      return false; \
  } while (0)

    static const Type madType[] = {TYPE_FLOAT};
    static const uint32_t madTypeNum = ARRAY_ELEM_NUM(madType);

    // TODO add support for 64 bits values
    static const Type allButBool[] = {TYPE_S8,  TYPE_U8,
                                      TYPE_S16, TYPE_U16,
                                      TYPE_S32, TYPE_U32,
                                      TYPE_FLOAT, TYPE_DOUBLE};
    static const uint32_t allButBoolNum = ARRAY_ELEM_NUM(allButBool);

    // TODO add support for 64 bits values
    static const Type logicalType[] = {TYPE_S8,  TYPE_U8,
                                       TYPE_S16, TYPE_U16,
                                       TYPE_S32, TYPE_U32,
                                       TYPE_BOOL};
    static const uint32_t logicalTypeNum = ARRAY_ELEM_NUM(logicalType);

    // Unary and binary instructions share the same rules
    template <uint32_t srcNum>
    INLINE bool NaryInstruction<srcNum>::wellFormed(const Function &fn, std::string &whyNot) const
    {
      const RegisterFamily family = getFamily(this->type);
      if (UNLIKELY(checkSpecialRegForWrite(dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(family, dst[0], fn, whyNot) == false))
        return false;
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID)
        if (UNLIKELY(checkRegisterData(family, src[srcID], fn, whyNot) == false))
          return false;
      // We actually support logical operations on boolean values for AND, OR,
      // and XOR
      switch (this->opcode) {
        case OP_OR:
        case OP_XOR:
        case OP_AND:
          CHECK_TYPE(this->type, logicalType);
          break;
        default:
          CHECK_TYPE(this->type, allButBool);
          break;
      }
      return true;
    }

    // Idem for ternary instructions except that sources are in a tuple
    INLINE bool TernaryInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      const RegisterFamily family = getFamily(this->type);
      if (UNLIKELY(checkSpecialRegForWrite(dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(family, dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(src + 3u > fn.tupleNum())) {
        whyNot = "Out-of-bound index for ternary instruction";
        return false;
      }
      for (uint32_t srcID = 0; srcID < 3; ++srcID) {
        const Register regID = fn.getRegister(src, srcID);
        if (UNLIKELY(checkRegisterData(family, regID, fn, whyNot) == false))
          return false;
      }
      if (this->opcode == OP_MAD)
        CHECK_TYPE(this->type, madType);
      else
        NOT_IMPLEMENTED;
      return true;
    }

    // First source must a boolean. Other must match the destination type
    INLINE bool SelectInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      const RegisterFamily family = getFamily(this->type);
      if (UNLIKELY(checkSpecialRegForWrite(dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(family, dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(src + 3u > fn.tupleNum())) {
        whyNot = "Out-of-bound index for ternary instruction";
        return false;
      }
      const Register regID = fn.getRegister(src, 0);
      if (UNLIKELY(checkRegisterData(FAMILY_BOOL, regID, fn, whyNot) == false))
        return false;
      for (uint32_t srcID = 1; srcID < 3; ++srcID) {
        const Register regID = fn.getRegister(src, srcID);
        if (UNLIKELY(checkRegisterData(family, regID, fn, whyNot) == false))
          return false;
      }
      CHECK_TYPE(this->type, allButBool);
      return true;
    }

    // Pretty similar to binary instruction. Only the destination is of type
    // boolean
    INLINE bool CompareInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY(checkSpecialRegForWrite(dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(FAMILY_BOOL, dst[0], fn, whyNot) == false))
        return false;
      const RegisterFamily family = getFamily(this->type);
      for (uint32_t srcID = 0; srcID < 2; ++srcID)
        if (UNLIKELY(checkRegisterData(family, src[srcID], fn, whyNot) == false))
          return false;
      CHECK_TYPE(this->type, allButBool);
      return true;
    }

    // We can convert anything to anything, but types and families must match
    INLINE bool ConvertInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      const RegisterFamily dstFamily = getFamily(dstType);
      const RegisterFamily srcFamily = getFamily(srcType);
      if (UNLIKELY(checkSpecialRegForWrite(dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(dstFamily, dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(srcFamily, src[0], fn, whyNot) == false))
        return false;
      CHECK_TYPE(this->dstType, allButBool);
      CHECK_TYPE(this->srcType, allButBool);
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
      const RegisterFamily family = getFamily(insn.type);
      for (uint32_t valueID = 0; valueID < insn.valueNum; ++valueID) {
        const Register regID = fn.getRegister(insn.values, valueID);
        if (UNLIKELY(checkRegisterData(family, regID, fn, whyNot) == false))
          return false;
      }
      CHECK_TYPE(insn.type, allButBool);
      return true;
    }

    INLINE bool LoadInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      const uint32_t dstNum = this->getDstNum();
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
        const Register reg = this->getDst(fn, dstID);
        const bool isOK = checkSpecialRegForWrite(reg, fn, whyNot);
        if (UNLIKELY(isOK == false)) return false;
      }
      if (UNLIKELY(dstNum > Instruction::MAX_DST_NUM)) {
        whyNot = "Too many destinations for load instruction";
        return false;
      }
      return wellFormedLoadStore(*this, fn, whyNot);
    }

    INLINE bool StoreInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      const uint32_t srcNum = this->getSrcNum();
      if (UNLIKELY(srcNum > Instruction::MAX_SRC_NUM)) {
        whyNot = "Too many source for store instruction";
        return false;
      }
      return wellFormedLoadStore(*this, fn, whyNot);
    }

    // TODO
    INLINE bool SampleInstruction::wellFormed(const Function &fn, std::string &why) const
    { return true; }
    INLINE bool TypedWriteInstruction::wellFormed(const Function &fn, std::string &why) const
    { return true; }

    // Ensure that types and register family match
    INLINE bool LoadImmInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY(immediateIndex >= fn.immediateNum())) {
        whyNot = "Out-of-bound immediate value index";
        return false;
      }
      if (UNLIKELY(type != fn.getImmediate(immediateIndex).type)) {
        whyNot = "Inconsistant type for the immediate value to load";
        return false;
      }
      const RegisterFamily family = getFamily(type);
      if (UNLIKELY(checkSpecialRegForWrite(dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(family, dst[0], fn, whyNot) == false))
        return false;
      CHECK_TYPE(this->type, allButBool);
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
    INLINE bool BranchInstruction::wellFormed(const Function &fn, std::string &whyNot) const {
      if (hasLabel)
        if (UNLIKELY(labelIndex >= fn.labelNum())) {
          whyNot = "Out-of-bound label index";
          return false;
        }
      if (hasPredicate)
        if (UNLIKELY(checkRegisterData(FAMILY_BOOL, predicate, fn, whyNot) == false))
          return false;
      return true;
    }

    // Stride is 1,2,4,8 offset goes from 0 to 15 and registers must be dwords
    INLINE bool RegionInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY(vstride != 0 && vstride != 1 && vstride != 2 && vstride != 4 && vstride != 8 && vstride != 16)) {
        whyNot = "Invalid vertical stride (must be 0, 1, 2, 4 or 8)";
        return false;
      }
      if (UNLIKELY(hstride != 0 && hstride != 1 && hstride != 2 && hstride != 4 && hstride != 8 && hstride != 16)) {
        whyNot = "Invalid horizontal stride (must be 0, 1, 2, 4 or 8)";
        return false;
      }
      if (UNLIKELY(width != 0 && width != 1 && width != 2 && width != 4 && width != 8)) {
        whyNot = "Invalid width (must be 1, 2, 4 or 8)";
        return false;
      }
      if (UNLIKELY(offset > 7)) {
        whyNot = "Invalid offset (must be smaller than 8)";
        return false;
      }
      if (UNLIKELY(checkRegisterData(FAMILY_DWORD, dst[0], fn, whyNot) == false))
        return false;
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        const Register regID = fn.getRegister(src, srcID);
        if (UNLIKELY(checkRegisterData(FAMILY_DWORD, regID, fn, whyNot) == false))
          return false;
      }
      return true;
    }

    // Boolean values for both source and destination
    INLINE bool VoteInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY(checkRegisterData(FAMILY_WORD, dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(FAMILY_WORD, src[0], fn, whyNot) == false))
        return false;
      return true;
    }

    // Indices are always int16 and the rest is 32 bit integers
    INLINE bool RGatherInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY(checkRegisterData(FAMILY_DWORD, dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(FAMILY_WORD, fn.getRegister(src, 0), fn, whyNot) == false))
        return false;
      for (uint32_t srcID = 1; srcID < srcNum; ++srcID) {
        const Register regID = fn.getRegister(src, srcID);
        if (UNLIKELY(checkRegisterData(FAMILY_DWORD, regID, fn, whyNot) == false))
          return false;
      }
      return true;
    }

    // Source is an address. Destination is a 32 bit integer
    INLINE bool OBReadInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      const RegisterFamily ptrFamily = fn.getPointerFamily();
      if (UNLIKELY(checkRegisterData(ptrFamily, src[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(FAMILY_DWORD, dst[0], fn, whyNot) == false))
        return false;
      return true;
    }

    // First source is the address. Second source is the value to write
    INLINE bool OBWriteInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      const RegisterFamily ptrFamily = fn.getPointerFamily();
      if (UNLIKELY(checkRegisterData(ptrFamily, src[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(FAMILY_DWORD, src[1], fn, whyNot) == false))
        return false;
      return true;
    }

#undef CHECK_TYPE

    /////////////////////////////////////////////////////////////////////////
    // Implements all the output stream methods
    /////////////////////////////////////////////////////////////////////////
    template <uint32_t srcNum>
    INLINE void NaryInstruction<srcNum>::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << "." << this->getType()
          << " %" << this->getDst(fn, 0);
      for (uint32_t i = 0; i < srcNum; ++i)
        out << " %" << this->getSrc(fn, i);
    }

    template <typename T>
    static void ternaryOrSelectOut(const T &insn, std::ostream &out, const Function &fn) {
      insn.outOpcode(out);
      out << "." << insn.getType()
          << " %" << insn.getDst(fn, 0)
          << " %" << insn.getSrc(fn, 0)
          << " %" << insn.getSrc(fn, 1)
          << " %" << insn.getSrc(fn, 2);
    }

    INLINE void TernaryInstruction::out(std::ostream &out, const Function &fn) const {
      ternaryOrSelectOut(*this, out, fn);
    }

    INLINE void SelectInstruction::out(std::ostream &out, const Function &fn) const {
      ternaryOrSelectOut(*this, out, fn);
    }

    INLINE void ConvertInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << "." << this->getDstType()
          << "." << this->getSrcType()
          << " %" << this->getDst(fn, 0)
          << " %" << this->getSrc(fn, 0);
    }

    INLINE void LoadInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << "." << type << "." << addrSpace << (dwAligned ? "." : ".un") << "aligned";
      out << " {";
      for (uint32_t i = 0; i < valueNum; ++i)
        out << "%" << this->getDst(fn, i) << (i != (valueNum-1u) ? " " : "");
      out << "}";
      out << " %" << this->getSrc(fn, 0);
    }

    INLINE void StoreInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << "." << type << "." << addrSpace << (dwAligned ? "." : ".un") << "aligned";
      out << " %" << this->getSrc(fn, 0) << " {";
      for (uint32_t i = 0; i < valueNum; ++i)
        out << "%" << this->getSrc(fn, i+1) << (i != (valueNum-1u) ? " " : "");
      out << "}";
    }

    INLINE void LabelInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << " $" << labelIndex;
    }

    INLINE void BranchInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      if (hasPredicate)
        out << "<%" << this->getSrc(fn, 0) << ">";
      if (hasLabel) out << " -> label$" << labelIndex;
    }

    INLINE void LoadImmInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << "." << type;
      out << " %" << this->getDst(fn,0) << " ";
      fn.outImmediate(out, immediateIndex);
    }

    INLINE void RegionInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << "<" << uint32_t(vstride) << ";"
                 << uint32_t(width) << "," << uint32_t(hstride)
                 << ">." << uint32_t(offset) << " ";
      out << "%" << this->getDst(fn, 0) << " ";
      for (uint32_t i = 0; i < this->getSrcNum(); ++i)
        out << "%" << this->getSrc(fn, i) << (i != (srcNum-1u) ? " " : "");
    }

    INLINE void VoteInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << "." << (this->getVotePredicate() == VOTE_ALL ? "all" : "any")
          << " %" << this->getDst(fn, 0)
          << " %" << this->getSrc(fn, 0);
    }

    INLINE void RGatherInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << " %" << this->getDst(fn, 0);
      for (uint32_t i = 0; i < this->getSrcNum(); ++i)
        out << " %" << this->getSrc(fn, i);
    }

    INLINE void OBReadInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << " %" << this->getDst(fn, 0);
      out << " %" << this->getSrc(fn, 0);
    }

    INLINE void OBWriteInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << " %" << this->getSrc(fn, 0);
      out << " %" << this->getSrc(fn, 1);
    }

  } /* namespace internal */

  std::ostream &operator<< (std::ostream &out, AddressSpace addrSpace) {
    switch (addrSpace) {
      case MEM_GLOBAL: return out << "global";
      case MEM_LOCAL: return out << "local";
      case MEM_CONSTANT: return out << "constant";
      case MEM_PRIVATE: return out << "private";
      case MEM_INVALID: NOT_SUPPORTED; return out;
    };
    return out;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Implements the various instrospection functions
  ///////////////////////////////////////////////////////////////////////////
  template <typename T, typename U> struct HelperIntrospection {
    enum { value = 0 };
  };
  template <typename T> struct HelperIntrospection<T,T> {
    enum { value = 1 };
  };

  RegisterData Instruction::getDstData(uint32_t ID) const {
    const Function &fn = this->getFunction();
    return fn.getRegisterData(this->getDst(ID));
  }
  RegisterData Instruction::getSrcData(uint32_t ID) const {
    const Function &fn = this->getFunction();
    return fn.getRegisterData(this->getSrc(ID));
  }

#define DECL_INSN(OPCODE, CLASS) \
  case OP_##OPCODE: \
  return HelperIntrospection<CLASS, RefClass>::value == 1;

#define START_INTROSPECTION(CLASS) \
  static_assert(sizeof(internal::CLASS) == sizeof(uint64_t), \
                "Bad instruction size"); \
  static_assert(offsetof(internal::CLASS, opcode) == 0, \
                "Bad opcode offset"); \
  bool CLASS::isClassOf(const Instruction &insn) { \
    const Opcode op = insn.getOpcode(); \
    typedef CLASS RefClass; \
    switch (op) {

#define END_INTROSPECTION(CLASS) \
      default: return false; \
    }; \
  }

START_INTROSPECTION(UnaryInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(UnaryInstruction)

START_INTROSPECTION(BinaryInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(BinaryInstruction)

START_INTROSPECTION(TernaryInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(TernaryInstruction)

START_INTROSPECTION(CompareInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(CompareInstruction)

START_INTROSPECTION(ConvertInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(ConvertInstruction)

START_INTROSPECTION(SelectInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(SelectInstruction)

START_INTROSPECTION(BranchInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(BranchInstruction)

START_INTROSPECTION(SampleInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(SampleInstruction)

START_INTROSPECTION(TypedWriteInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(TypedWriteInstruction)

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

START_INTROSPECTION(RegionInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(RegionInstruction)

START_INTROSPECTION(VoteInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(VoteInstruction)

START_INTROSPECTION(RGatherInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(RGatherInstruction)

START_INTROSPECTION(OBReadInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(OBReadInstruction)

START_INTROSPECTION(OBWriteInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(OBWriteInstruction)

#undef END_INTROSPECTION
#undef START_INTROSPECTION
#undef DECL_INSN

  ///////////////////////////////////////////////////////////////////////////
  // Implements the function dispatching from public to internal with some
  // macro horrors
  ///////////////////////////////////////////////////////////////////////////

#define DECL_INSN(OPCODE, CLASS) \
  case OP_##OPCODE: return reinterpret_cast<const internal::CLASS*>(this)->CALL;

#define START_FUNCTION(CLASS, RET, PROTOTYPE) \
  RET CLASS::PROTOTYPE const { \
    const Opcode op = this->getOpcode(); \
    switch (op) {

#define END_FUNCTION(CLASS, RET) \
      case OP_INVALID: return RET(); \
    }; \
    return RET(); \
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

#undef DECL_INSN

#define DECL_INSN(OPCODE, CLASS) \
  case OP_##OPCODE: \
  { \
    const Function &fn = this->getFunction(); \
    return reinterpret_cast<const internal::CLASS*>(this)->CALL; \
  }

#define CALL wellFormed(fn, whyNot)
START_FUNCTION(Instruction, bool, wellFormed(std::string &whyNot))
#include "ir/instruction.hxx"
END_FUNCTION(Instruction, bool)
#undef CALL

#define CALL getDst(fn, ID)
START_FUNCTION(Instruction, Register, getDst(uint32_t ID))
#include "ir/instruction.hxx"
END_FUNCTION(Instruction, Register)
#undef CALL

#define CALL getSrc(fn, ID)
START_FUNCTION(Instruction, Register, getSrc(uint32_t ID))
#include "ir/instruction.hxx"
END_FUNCTION(Instruction, Register)
#undef CALL

#undef DECL_INSN
#undef END_FUNCTION
#undef START_FUNCTION

  void Instruction::setSrc(uint32_t srcID, Register reg) {
    Function &fn = this->getFunction();
#if GBE_DEBUG
    const RegisterData oldData = this->getSrcData(srcID);
    const RegisterData newData = fn.getRegisterData(reg);
    GBE_ASSERT(oldData.family == newData.family);
#endif /* GBE_DEBUG */
    const Opcode op = this->getOpcode();
    switch (op) {
#define DECL_INSN(OP, FAMILY)\
      case OP_##OP:\
        reinterpret_cast<internal::FAMILY*>(this)->setSrc(fn, srcID, reg);\
      break;
#include "instruction.hxx"
#undef DECL_INSN
      case OP_INVALID: NOT_SUPPORTED; break;
    };
  }

  void Instruction::setDst(uint32_t dstID, Register reg) {
    Function &fn = this->getFunction();
#if GBE_DEBUG
    const RegisterData oldData = this->getDstData(dstID);
    const RegisterData newData = fn.getRegisterData(reg);
    GBE_ASSERT(oldData.family == newData.family);
#endif /* GBE_DEBUG */
    const Opcode op = this->getOpcode();
    switch (op) {
#define DECL_INSN(OP, FAMILY)\
      case OP_##OP:\
        reinterpret_cast<internal::FAMILY*>(this)->setDst(fn, dstID, reg);\
      break;
#include "instruction.hxx"
#undef DECL_INSN
      case OP_INVALID: NOT_SUPPORTED; break;
    };
  }

  const Function &Instruction::getFunction(void) const {
    const BasicBlock *bb = this->getParent();
    GBE_ASSERT(bb != NULL);
    return bb->getParent();
  }
  Function &Instruction::getFunction(void) {
    BasicBlock *bb = this->getParent();
    GBE_ASSERT(bb != NULL);
    return bb->getParent();
  }

  void Instruction::replace(Instruction *other) const {
    Function &fn = other->getFunction();
    BasicBlock *bb = other->getParent();
    Instruction *insn = fn.newInstruction();
    *insn = *this;
    if (bb->getFirstInstruction() == other) bb->setFirstInstruction(insn);
    if (bb->getLastInstruction() == other) bb->setLastInstruction(insn);
    if (other->predecessor) other->predecessor->successor = insn;
    if (other->successor) other->successor->predecessor = insn;
    insn->parent = other->parent;
    insn->predecessor = other->predecessor;
    insn->successor = other->successor;
    fn.deleteInstruction(other);
  }

  void Instruction::remove(void) {
    Function &fn = this->getFunction();
    BasicBlock *bb = this->getParent();
    if (bb->getFirstInstruction() == this)
      bb->setFirstInstruction(this->successor);
    if (bb->getLastInstruction() == this)
      bb->setLastInstruction(this->predecessor);
    if (this->predecessor)
      this->predecessor->successor = this->successor;
    if (this->successor)
      this->successor->predecessor = this->predecessor;
    fn.deleteInstruction(this);
  }

  bool Instruction::hasSideEffect(void) const {
    return opcode == OP_STORE || 
           opcode == OP_OBWRITE ||
           opcode == OP_TYPED_WRITE ||
           opcode == OP_FENCE;
  }

#define DECL_MEM_FN(CLASS, RET, PROTOTYPE, CALL) \
  RET CLASS::PROTOTYPE const { \
    return reinterpret_cast<const internal::CLASS*>(this)->CALL; \
  }

DECL_MEM_FN(UnaryInstruction, Type, getType(void), getType())
DECL_MEM_FN(BinaryInstruction, Type, getType(void), getType())
DECL_MEM_FN(BinaryInstruction, bool, commutes(void), commutes())
DECL_MEM_FN(TernaryInstruction, Type, getType(void), getType())
DECL_MEM_FN(SelectInstruction, Type, getType(void), getType())
DECL_MEM_FN(CompareInstruction, Type, getType(void), getType())
DECL_MEM_FN(ConvertInstruction, Type, getSrcType(void), getSrcType())
DECL_MEM_FN(ConvertInstruction, Type, getDstType(void), getDstType())
DECL_MEM_FN(StoreInstruction, Type, getValueType(void), getValueType())
DECL_MEM_FN(StoreInstruction, uint32_t, getValueNum(void), getValueNum())
DECL_MEM_FN(StoreInstruction, AddressSpace, getAddressSpace(void), getAddressSpace())
DECL_MEM_FN(StoreInstruction, bool, isAligned(void), isAligned())
DECL_MEM_FN(LoadInstruction, Type, getValueType(void), getValueType())
DECL_MEM_FN(LoadInstruction, uint32_t, getValueNum(void), getValueNum())
DECL_MEM_FN(LoadInstruction, AddressSpace, getAddressSpace(void), getAddressSpace())
DECL_MEM_FN(LoadInstruction, bool, isAligned(void), isAligned())
DECL_MEM_FN(LoadImmInstruction, Type, getType(void), getType())
DECL_MEM_FN(LabelInstruction, LabelIndex, getLabelIndex(void), getLabelIndex())
DECL_MEM_FN(BranchInstruction, bool, isPredicated(void), isPredicated())
DECL_MEM_FN(BranchInstruction, LabelIndex, getLabelIndex(void), getLabelIndex())
DECL_MEM_FN(RegionInstruction, uint32_t, getOffset(void), getOffset())
DECL_MEM_FN(RegionInstruction, uint32_t, getVStride(void), getVStride())
DECL_MEM_FN(RegionInstruction, uint32_t, getWidth(void), getWidth())
DECL_MEM_FN(RegionInstruction, uint32_t, getHStride(void), getHStride())
DECL_MEM_FN(VoteInstruction, VotePredicate, getVotePredicate(void), getVotePredicate())

#undef DECL_MEM_FN

  Immediate LoadImmInstruction::getImmediate(void) const {
    const Function &fn = this->getFunction();
    return reinterpret_cast<const internal::LoadImmInstruction*>(this)->getImmediate(fn);
  }

  ///////////////////////////////////////////////////////////////////////////
  // Implements the emission functions
  ///////////////////////////////////////////////////////////////////////////

  // All unary functions
#define DECL_EMIT_FUNCTION(NAME) \
  Instruction NAME(Type type, Register dst, Register src) { \
    return internal::UnaryInstruction(OP_##NAME, type, dst, src).convert(); \
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
#define DECL_EMIT_FUNCTION(NAME) \
  Instruction NAME(Type type, Register dst,  Register src0, Register src1) { \
    return internal::BinaryInstruction(OP_##NAME, type, dst, src0, src1).convert(); \
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
  Instruction MAD(Type type, Register dst, Tuple src) {
    return internal::TernaryInstruction(OP_MAD, type, dst, src).convert();
  }

  // SEL
  Instruction SEL(Type type, Register dst, Tuple src) {
    return internal::SelectInstruction(type, dst, src).convert();
  }

  // All compare functions
#define DECL_EMIT_FUNCTION(NAME) \
  Instruction NAME(Type type, Register dst,  Register src0, Register src1) { \
    const internal::CompareInstruction insn(OP_##NAME, type, dst, src0, src1); \
    return insn.convert(); \
  }

  DECL_EMIT_FUNCTION(EQ)
  DECL_EMIT_FUNCTION(NE)
  DECL_EMIT_FUNCTION(LE)
  DECL_EMIT_FUNCTION(LT)
  DECL_EMIT_FUNCTION(GE)
  DECL_EMIT_FUNCTION(GT)

#undef DECL_EMIT_FUNCTION

  // CVT
  Instruction CVT(Type dstType, Type srcType, Register dst, Register src) {
    return internal::ConvertInstruction(dstType, srcType, dst, src).convert();
  }

  // BRA
  Instruction BRA(LabelIndex labelIndex) {
    return internal::BranchInstruction(OP_BRA, labelIndex).convert();
  }
  Instruction BRA(LabelIndex labelIndex, Register pred) {
    return internal::BranchInstruction(OP_BRA, labelIndex, pred).convert();
  }

  // RET
  Instruction RET(void) {
    return internal::BranchInstruction(OP_RET).convert();
  }

  // LOADI
  Instruction LOADI(Type type, Register dst, ImmediateIndex value) {
    return internal::LoadImmInstruction(type, dst, value).convert();
  }

  // LOAD and STORE
#define DECL_EMIT_FUNCTION(NAME, CLASS) \
  Instruction NAME(Type type, \
                   Tuple tuple, \
                   Register offset, \
                   AddressSpace space, \
                   uint32_t valueNum, \
                   bool dwAligned) \
  { \
    return internal::CLASS(type,tuple,offset,space,valueNum,dwAligned).convert(); \
  }

  DECL_EMIT_FUNCTION(LOAD, LoadInstruction)
  DECL_EMIT_FUNCTION(STORE, StoreInstruction)

#undef DECL_EMIT_FUNCTION

  // FENCE
  Instruction FENCE(AddressSpace space) {
    return internal::FenceInstruction(space).convert();
  }

  // LABEL
  Instruction LABEL(LabelIndex labelIndex) {
    return internal::LabelInstruction(labelIndex).convert();
  }

  // REGION
  Instruction REGION(uint32_t offset, uint32_t vstride, uint32_t width, uint32_t hstride, Register dst, Tuple src, uint32_t srcNum) {
    return internal::RegionInstruction(dst, src, srcNum, offset, vstride, width, hstride).convert();
  }

  // VOTE
  Instruction VOTE(VotePredicate pred, Register dst, Register src) {
    return internal::VoteInstruction(dst, src, pred).convert();
  }

  // RGATHER
  Instruction RGATHER(Register dst, Tuple src, uint32_t srcNum) {
    return internal::RGatherInstruction(dst, src, srcNum).convert();
  }

  // OBREAD
  Instruction OBREAD(Register dst, Register address) {
    return internal::OBReadInstruction(dst, address).convert();
  }

  // OBWRITE
  Instruction OBWRITE(Register address, Register value) {
    return internal::OBWriteInstruction(address, value).convert();
  }

  std::ostream &operator<< (std::ostream &out, const Instruction &insn) {
    const Function &fn = insn.getFunction();
    switch (insn.getOpcode()) {
#define DECL_INSN(OPCODE, CLASS) \
      case OP_##OPCODE: \
        reinterpret_cast<const internal::CLASS&>(insn).out(out, fn); \
        break;
#include "instruction.hxx"
#undef DECL_INSN
      case OP_INVALID: NOT_SUPPORTED; break;
    };
    return out;
  }

} /* namespace ir */
} /* namespace gbe */

