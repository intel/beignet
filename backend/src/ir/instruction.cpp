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
#define ALIGNED_INSTRUCTION ALIGNED(ALIGNOF(Instruction))

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
        GBE_ASSERTM((int) ID < (int) srcNum, "Out-of-bound source");
        return static_cast<const T*>(this)->src[ID];
      }
      INLINE void setSrc(Function &fn, uint32_t ID, Register reg) {
        GBE_ASSERTM((int) ID < (int) srcNum, "Out-of-bound source");
        static_cast<T*>(this)->src[ID] = reg;
      }
    };

    /*! For regular n destinations instructions */
    template <typename T, uint32_t dstNum>
    struct NDstPolicy {
      INLINE uint32_t getDstNum(void) const { return dstNum; }
      INLINE Register getDst(const Function &fn, uint32_t ID) const {
        GBE_ASSERTM((int) ID < (int) dstNum, "Out-of-bound destination");
        return static_cast<const T*>(this)->dst[ID];
      }
      INLINE void setDst(Function &fn, uint32_t ID, Register reg) {
        GBE_ASSERTM((int) ID < (int) dstNum, "Out-of-bound destination");
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

    /*! For instructions that use a tuple for destination */
    template <typename T>
    struct TupleDstPolicy {
      INLINE uint32_t getDstNum(void) const {
        return static_cast<const T*>(this)->dstNum;
      }
      INLINE Register getDst(const Function &fn, uint32_t ID) const {
        GBE_ASSERTM(ID < static_cast<const T*>(this)->dstNum, "Out-of-bound source register");
        return fn.getRegister(static_cast<const T*>(this)->dst, ID);
      }
      INLINE void setDst(Function &fn, uint32_t ID, Register reg) {
        GBE_ASSERTM(ID < static_cast<const T*>(this)->dstNum, "Out-of-bound source register");
        return fn.setRegister(static_cast<T*>(this)->dst, ID, reg);
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
          case OP_ADDSAT:
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

    class ALIGNED_INSTRUCTION TernaryInstruction :
      public BasePolicy,
      public NDstPolicy<TernaryInstruction, 1>,
      public TupleSrcPolicy<TernaryInstruction>
    {
     public:
      TernaryInstruction(Opcode opcode,
                         Type type,
                         Register dst,
                         Tuple src) {
        this->opcode = opcode;
        this->type = type;
        this->dst[0] = dst;
        this->src = src;
      }
      Type getType(void) const { return type; }
      bool wellFormed(const Function &fn, std::string &whyNot) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Type type;
      Register dst[1];
      Tuple src;
      static const uint32_t srcNum = 3;
    };

    /*! Three sources mean we need a tuple to encode it */
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

    class ALIGNED_INSTRUCTION BitCastInstruction :
      public BasePolicy,
      public TupleSrcPolicy<BitCastInstruction>,
      public TupleDstPolicy<BitCastInstruction>
    {
    public:
      BitCastInstruction(Type dstType,
                         Type srcType,
                         Tuple dst,
                         Tuple src,
                         uint8_t dstNum,
                         uint8_t srcNum)
      {
        this->opcode = OP_BITCAST;
        this->dst = dst;
        this->src = src;
        this->dstFamily = getFamily(dstType);
        this->srcFamily = getFamily(srcType);
        GBE_ASSERT(srcNum <= Instruction::MAX_SRC_NUM && dstNum <= Instruction::MAX_DST_NUM);
        this->dstNum = dstNum;
        this->srcNum = srcNum;
      }
      INLINE Type getSrcType(void) const { return getType((RegisterFamily)srcFamily); }
      INLINE Type getDstType(void) const { return getType((RegisterFamily)dstFamily); }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      uint8_t dstFamily:4; //!< family to cast to
      uint8_t srcFamily:4; //!< family to cast from
      Tuple dst;
      Tuple src;
      uint8_t dstNum;     //!<Dst Number
      uint8_t srcNum;     //!<Src Number
    };

    class ALIGNED_INSTRUCTION ConvertInstruction :
      public BasePolicy,
      public NDstPolicy<ConvertInstruction, 1>,
      public NSrcPolicy<ConvertInstruction, 1>
    {
    public:
      ConvertInstruction(Opcode opcode,
                         Type dstType,
                         Type srcType,
                         Register dst,
                         Register src)
      {
        this->opcode = opcode;
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

    class ALIGNED_INSTRUCTION AtomicInstruction :
      public BasePolicy,
      public TupleSrcPolicy<AtomicInstruction>,
      public NDstPolicy<AtomicInstruction, 1>
    {
    public:
      AtomicInstruction(AtomicOps atomicOp,
                         Register dst,
                         AddressSpace addrSpace,
                         BTI bti,
                         Tuple src)
      {
        this->opcode = OP_ATOMIC;
        this->atomicOp = atomicOp;
        this->dst[0] = dst;
        this->src = src;
        this->addrSpace = addrSpace;
        this->bti = bti;
        srcNum = 2;
        if((atomicOp == ATOMIC_OP_INC) ||
          (atomicOp == ATOMIC_OP_DEC))
          srcNum = 1;
        if(atomicOp == ATOMIC_OP_CMPXCHG)
          srcNum = 3;
      }
      INLINE AddressSpace getAddressSpace(void) const { return this->addrSpace; }
      INLINE BTI getBTI(void) const { return bti; }
      INLINE AtomicOps getAtomicOpcode(void) const { return this->atomicOp; }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Register dst[1];
      Tuple src;
      AddressSpace addrSpace; //!< Address space
      BTI bti;               //!< bti
      uint8_t srcNum:2;     //!<Source Number
      AtomicOps atomicOp:6;     //!<Source Number
    };

    class ALIGNED_INSTRUCTION BranchInstruction :
      public BasePolicy,
      public NDstPolicy<BranchInstruction, 0>
    {
    public:
      INLINE BranchInstruction(Opcode op, LabelIndex labelIndex, Register predicate, bool inv_pred=false) {
        GBE_ASSERT(op == OP_BRA || op == OP_IF || op == OP_WHILE);
        this->opcode = op;
        this->predicate = predicate;
        this->labelIndex = labelIndex;
        this->hasPredicate = true;
        this->hasLabel = true;
        this->inversePredicate = inv_pred;
      }
      INLINE BranchInstruction(Opcode op, LabelIndex labelIndex) {
        GBE_ASSERT(op == OP_BRA || op == OP_ELSE || op == OP_ENDIF);
        this->opcode = op;
        this->labelIndex = labelIndex;
        this->hasPredicate = false;
        this->hasLabel = true;
      }
      INLINE BranchInstruction(Opcode op) {
        GBE_ASSERT(op == OP_RET);
        this->opcode = op;
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
      INLINE bool getInversePredicated(void) const { return inversePredicate; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Register predicate;    //!< Predication means conditional branch
      LabelIndex labelIndex; //!< Index of the label the branch targets
      bool hasPredicate:1;   //!< Is it predicated?
      bool inversePredicate:1;   //!< Is it inverse predicated?
      bool hasLabel:1;       //!< Is there any target label?
      Register dst[0];       //!< No destination
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
                      bool dwAligned,
                      BTI bti)
      {
        GBE_ASSERT(valueNum < 128);
        this->opcode = OP_LOAD;
        this->type = type;
        this->offset = offset;
        this->values = dstValues;
        this->addrSpace = addrSpace;
        this->valueNum = valueNum;
        this->dwAligned = dwAligned ? 1 : 0;
        this->bti = bti;
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
      INLINE BTI getBTI(void) const { return bti; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      INLINE bool isAligned(void) const { return !!dwAligned; }
      Type type;              //!< Type to store
      Register src[0];        //!< Address where to load from
      Register offset;        //!< Alias to make it similar to store
      Tuple values;           //!< Values to load
      AddressSpace addrSpace; //!< Where to load
      BTI bti;
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
                       bool dwAligned,
                       BTI bti)
      {
        GBE_ASSERT(valueNum < 255);
        this->opcode = OP_STORE;
        this->type = type;
        this->offset = offset;
        this->values = values;
        this->addrSpace = addrSpace;
        this->valueNum = valueNum;
        this->dwAligned = dwAligned ? 1 : 0;
        this->bti = bti;
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
      INLINE BTI getBTI(void) const { return bti; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      INLINE bool isAligned(void) const { return !!dwAligned; }
      Type type;              //!< Type to store
      Register offset;        //!< First source is the offset where to store
      Tuple values;           //!< Values to store
      AddressSpace addrSpace; //!< Where to store
      BTI bti;                //!< Which btis need access
      uint8_t valueNum:7;     //!< Number of values to store
      uint8_t dwAligned:1;    //!< DWORD aligned is what matters with GEN
      Register dst[0];        //!< No destination
    };

    class ALIGNED_INSTRUCTION SampleInstruction : // TODO
      public BasePolicy,
      public TupleSrcPolicy<SampleInstruction>,
      public TupleDstPolicy<SampleInstruction>
    {
    public:
      SampleInstruction(uint8_t imageIdx, Tuple dstTuple, Tuple srcTuple, bool dstIsFloat, bool srcIsFloat, uint8_t sampler, uint8_t samplerOffset) {
        this->opcode = OP_SAMPLE;
        this->dst = dstTuple;
        this->src = srcTuple;
        this->dstIsFloat = dstIsFloat;
        this->srcIsFloat = srcIsFloat;
        this->samplerIdx = sampler;
        this->imageIdx = imageIdx;
        this->samplerOffset = samplerOffset;
      }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const {
        this->outOpcode(out);
        out << "." << this->getDstType()
            << "." << this->getSrcType()
            << " surface id " << (int)this->getImageIndex()
            << " coord u %" << this->getSrc(fn, 0)
            << " coord v %" << this->getSrc(fn, 1)
            << " coord w %" << this->getSrc(fn, 2)
            << " %" << this->getDst(fn, 0)
            << " %" << this->getDst(fn, 1)
            << " %" << this->getDst(fn, 2)
            << " %" << this->getDst(fn, 3)
            << " sampler idx " << (int)this->getSamplerIndex();
      }
      Tuple src;
      Tuple dst;

      INLINE uint8_t getImageIndex(void) const { return this->imageIdx; }
      INLINE Type getSrcType(void) const { return this->srcIsFloat ? TYPE_FLOAT : TYPE_S32; }
      INLINE Type getDstType(void) const { return this->dstIsFloat ? TYPE_FLOAT : TYPE_U32; }
      INLINE uint8_t getSamplerIndex(void) const { return this->samplerIdx; }
      INLINE uint8_t getSamplerOffset(void) const { return this->samplerOffset; }
      uint8_t srcIsFloat:1;
      uint8_t dstIsFloat:1;
      uint8_t samplerIdx:4;
      uint8_t samplerOffset:2;
      uint8_t imageIdx;
      static const uint32_t srcNum = 3;
      static const uint32_t dstNum = 4;
    };

    class ALIGNED_INSTRUCTION TypedWriteInstruction : // TODO
      public BasePolicy,
      public TupleSrcPolicy<TypedWriteInstruction>,
      public NDstPolicy<TypedWriteInstruction, 0>
    {
    public:

      INLINE TypedWriteInstruction(uint8_t imageIdx, Tuple srcTuple, Type srcType, Type coordType) {
        this->opcode = OP_TYPED_WRITE;
        this->src = srcTuple;
        this->coordType = coordType;
        this->srcType = srcType;
        this->imageIdx = imageIdx;
      }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const {
        this->outOpcode(out);
        out << "." << this->getSrcType()
            << " surface id " << (int)this->getImageIndex()
            << " coord u %" << this->getSrc(fn, 0)
            << " coord v %" << this->getSrc(fn, 1)
            << " coord w %" << this->getSrc(fn, 2)
            << " %" << this->getSrc(fn, 3)
            << " %" << this->getSrc(fn, 4)
            << " %" << this->getSrc(fn, 5)
            << " %" << this->getSrc(fn, 6);
      }

      Tuple src;
      uint8_t srcType;
      uint8_t coordType;
      uint8_t imageIdx;

      INLINE uint8_t getImageIndex(void) const { return this->imageIdx; }
      INLINE Type getSrcType(void) const { return (Type)this->srcType; }
      INLINE Type getCoordType(void) const { return (Type)this->coordType; }
      // bti, u, v, w, 4 data elements
      static const uint32_t srcNum = 7;
      Register dst[0];               //!< No dest register
    };

    class ALIGNED_INSTRUCTION GetImageInfoInstruction :
      public BasePolicy,
      public NSrcPolicy<GetImageInfoInstruction, 1>,
      public NDstPolicy<GetImageInfoInstruction, 1>
    {
    public:
      GetImageInfoInstruction( int type,
                               Register dst,
                               uint8_t imageIdx,
                               Register infoReg)
      {
        this->opcode = OP_GET_IMAGE_INFO;
        this->infoType = type;
        this->dst[0] = dst;
        this->src[0] = infoReg;
        this->imageIdx = imageIdx;
      }

      INLINE uint32_t getInfoType(void) const { return infoType; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const {
        this->outOpcode(out);
        out << "." << this->getInfoType()
            << " %" << this->getDst(fn, 0)
            << " surface id " << (int)this->getImageIndex()
            << " info reg %" << this->getSrc(fn, 0);
      }

      INLINE uint8_t getImageIndex(void) const { return imageIdx; }

      uint8_t infoType;                 //!< Type of the requested information.
      uint8_t imageIdx;                //!< surface index.
      Register src[1];                  //!< surface info register.
      Register dst[1];                  //!< dest register to put the information.
      static const uint32_t dstNum = 1;
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
      Register src[0];               //!< No source register
      ImmediateIndex immediateIndex; //!< Index in the vector of immediates
      Type type;                     //!< Type of the immediate
    };

    class ALIGNED_INSTRUCTION SyncInstruction :
      public BasePolicy,
      public NSrcPolicy<SyncInstruction, 0>,
      public NDstPolicy<SyncInstruction, 0>
    {
    public:
      INLINE SyncInstruction(uint32_t parameters) {
        this->opcode = OP_SYNC;
        this->parameters = parameters;
      }
      INLINE uint32_t getParameters(void) const { return this->parameters; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      uint32_t parameters;
      Register dst[0], src[0];
    };

    class ALIGNED_INSTRUCTION ReadARFInstruction :
      public BasePolicy,
      public NSrcPolicy<ReadARFInstruction, 0>,
      public NDstPolicy<ReadARFInstruction, 1>
    {
    public:
      INLINE ReadARFInstruction(Type type, Register dst, ARFRegister arf) {
        this->type = type;
        this->dst[0] = dst;
        this->opcode = OP_READ_ARF;
        this->arf = arf;
      }
      INLINE ir::ARFRegister getARFRegister(void) const { return this->arf; }
      INLINE Type getType(void) const { return this->type; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Type type;
      ARFRegister arf;
      Register dst[1];
      Register src[0];
    };

    class ALIGNED_INSTRUCTION RegionInstruction :
      public BasePolicy,
      public NSrcPolicy<RegionInstruction, 1>,
      public NDstPolicy<RegionInstruction, 1>
    {
    public:
      INLINE RegionInstruction(Register dst, Register src, uint32_t offset) {
        this->offset = offset;
        this->dst[0] = dst;
        this->src[0] = src;
        this->opcode = OP_REGION;
      }
      INLINE uint32_t getOffset(void) const { return this->offset; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      uint32_t offset;
      Register dst[1];
      Register src[1];
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
      Register dst[0], src[0];
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
                                      TYPE_S64, TYPE_U64,
                                      TYPE_FLOAT, TYPE_DOUBLE};
    static const uint32_t allButBoolNum = ARRAY_ELEM_NUM(allButBool);

    // TODO add support for 64 bits values
    static const Type logicalType[] = {TYPE_S8,  TYPE_U8,
                                       TYPE_S16, TYPE_U16,
                                       TYPE_S32, TYPE_U32,
                                       TYPE_S64, TYPE_U64,
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
        case OP_MOV:
          break;
        case OP_POW:
        case OP_COS:
        case OP_SIN:
        case OP_RCP:
        case OP_ABS:
        case OP_RSQ:
        case OP_SQR:
        case OP_RNDD:
        case OP_RNDE:
        case OP_RNDU:
        case OP_RNDZ:
          const Type fp = TYPE_FLOAT;
          if (UNLIKELY(checkTypeFamily(TYPE_FLOAT, &fp, 1, whyNot)) == false)
            return false;
          break;
      }
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

    // The bit sizes of src and the dst must be identical, and don't support bool now, bool need double check.
    INLINE bool BitCastInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
        if (UNLIKELY(checkSpecialRegForWrite(getDst(fn, dstID), fn, whyNot) == false))
          return false;
        if (UNLIKELY(checkRegisterData((RegisterFamily)dstFamily, getDst(fn, dstID), fn, whyNot) == false))
          return false;
      }
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        if (UNLIKELY(checkRegisterData((RegisterFamily)srcFamily, getSrc(fn, srcID), fn, whyNot) == false))
          return false;
      }

      CHECK_TYPE(getType((RegisterFamily)dstFamily), allButBool);
      CHECK_TYPE(getType((RegisterFamily)srcFamily), allButBool);

      uint32_t dstBytes = 0, srcBtyes = 0;
      dstBytes = dstNum * getFamilySize((RegisterFamily)dstFamily);
      srcBtyes = srcNum * getFamilySize((RegisterFamily)srcFamily);

      if(dstBytes != srcBtyes){
        whyNot = " The bit sizes of src and the dst is not identical.";
        return false;
      }

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

    // We can convert anything to anything, but types and families must match
    INLINE bool AtomicInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY(checkSpecialRegForWrite(dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(FAMILY_DWORD, dst[0], fn, whyNot) == false))
        return false;
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID)
        if (UNLIKELY(checkRegisterData(FAMILY_DWORD, getSrc(fn, srcID), fn, whyNot) == false))
          return false;

      return true;
    }

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
    INLINE bool GetImageInfoInstruction::wellFormed(const Function &fn, std::string &why) const
    { return true; }


    // Ensure that types and register family match
    INLINE bool LoadImmInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY(immediateIndex >= fn.immediateNum())) {
        whyNot = "Out-of-bound immediate value index";
        return false;
      }
      const ir::Type immType = fn.getImmediate(immediateIndex).getType();
      if (UNLIKELY(type != immType)) {
        whyNot = "Inconsistent type for the immediate value to load";
        return false;
      }
      const RegisterFamily family = getFamily(type);
      if (UNLIKELY(checkSpecialRegForWrite(dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(family, dst[0], fn, whyNot) == false))
        return false;
      //Support all type IMM, disable check
      //CHECK_TYPE(this->type, allButBool);
      return true;
    }

    INLINE bool SyncInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      const uint32_t maxParams = SYNC_WORKGROUP_EXEC |
                                 SYNC_LOCAL_READ_FENCE |
                                 SYNC_LOCAL_WRITE_FENCE |
                                 SYNC_GLOBAL_READ_FENCE |
                                 SYNC_GLOBAL_WRITE_FENCE;
      if (UNLIKELY(this->parameters > maxParams)) {
        whyNot = "Invalid parameters for sync instruction";
        return false;
      } else if (UNLIKELY(this->parameters == 0)) {
        whyNot = "Missing parameters for sync instruction";
        return false;
      }
      return true;
    }

    INLINE bool ReadARFInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY( this->type != TYPE_U32 && this->type != TYPE_S32)) {
        whyNot = "Only support S32/U32 type";
        return false;
      }

      const RegisterFamily family = getFamily(this->type);
      if (UNLIKELY(checkRegisterData(family, dst[0], fn, whyNot) == false))
        return false;

      return true;
    }

    INLINE bool RegionInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY(checkRegisterData(FAMILY_DWORD, src[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(FAMILY_DWORD, dst[0], fn, whyNot) == false))
        return false;

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

    INLINE void SelectInstruction::out(std::ostream &out, const Function &fn) const {
      ternaryOrSelectOut(*this, out, fn);
    }

    INLINE void TernaryInstruction::out(std::ostream &out, const Function &fn) const {
      ternaryOrSelectOut(*this, out, fn);
    }

    INLINE void AtomicInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << "." << addrSpace;
      out << " %" << this->getDst(fn, 0);
      out << " {" << "%" << this->getSrc(fn, 0) << "}";
      for (uint32_t i = 1; i < srcNum; ++i)
        out << " %" << this->getSrc(fn, i);
      out << " bti";
      for (uint32_t i = 0; i < bti.count; ++i)
        out << ": " << (int)bti.bti[i];
    }


    INLINE void BitCastInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << "." << this->getDstType()
          << "." << this->getSrcType();
      out << " {";
      for (uint32_t i = 0; i < dstNum; ++i)
        out << "%" << this->getDst(fn, i) << (i != (dstNum-1u) ? " " : "");
      out << "}";
      out << " {";
      for (uint32_t i = 0; i < srcNum; ++i)
        out << "%" << this->getSrc(fn, i) << (i != (srcNum-1u) ? " " : "");
      out << "}";
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
      out << " bti";
      for (uint32_t i = 0; i < bti.count; ++i)
        out << ": " << (int)bti.bti[i];
    }

    INLINE void StoreInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << "." << type << "." << addrSpace << (dwAligned ? "." : ".un") << "aligned";
      out << " %" << this->getSrc(fn, 0) << " {";
      for (uint32_t i = 0; i < valueNum; ++i)
        out << "%" << this->getSrc(fn, i+1) << (i != (valueNum-1u) ? " " : "");
      out << "}";
      out << " bti";
      for (uint32_t i = 0; i < bti.count; ++i)
        out << ": " << (int)bti.bti[i];
    }

    INLINE void ReadARFInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << " %" << this->getDst(fn, 0) << " arf:" << arf;
    }

    INLINE void RegionInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << " %" << this->getDst(fn, 0) << " %" << this->getSrc(fn, 0) << " offset: " << this->offset;
    }

    INLINE void LabelInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << " $" << labelIndex;
    }

    INLINE void BranchInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      if(opcode == OP_IF && inversePredicate)
        out << " !";
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

    static const char *syncStr[syncFieldNum] = {
      "workgroup", "local_read", "local_write", "global_read", "global_write"
    };

    INLINE void SyncInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      for (uint32_t field = 0; field < syncFieldNum; ++field)
        if (this->parameters & (1 << field))
          out << "." << syncStr[field];
    }


  } /* namespace internal */

  std::ostream &operator<< (std::ostream &out, AddressSpace addrSpace) {
    switch (addrSpace) {
      case MEM_GLOBAL: return out << "global";
      case MEM_LOCAL: return out << "local";
      case MEM_CONSTANT: return out << "constant";
      case MEM_PRIVATE: return out << "private";
      case IMAGE: return out << "image";
      case MEM_INVALID: return out << "invalid";
    };
    return out;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Implements the various introspection functions
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
  static_assert(sizeof(internal::CLASS) == (sizeof(uint64_t)*2), \
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

START_INTROSPECTION(CompareInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(CompareInstruction)

START_INTROSPECTION(BitCastInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(BitCastInstruction)

START_INTROSPECTION(ConvertInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(ConvertInstruction)

START_INTROSPECTION(AtomicInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(AtomicInstruction)

START_INTROSPECTION(SelectInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(SelectInstruction)

START_INTROSPECTION(TernaryInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(TernaryInstruction)

START_INTROSPECTION(BranchInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(BranchInstruction)

START_INTROSPECTION(SampleInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(SampleInstruction)

START_INTROSPECTION(TypedWriteInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(TypedWriteInstruction)

START_INTROSPECTION(GetImageInfoInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(GetImageInfoInstruction)

START_INTROSPECTION(LoadImmInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(LoadImmInstruction)

START_INTROSPECTION(LoadInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(LoadInstruction)

START_INTROSPECTION(StoreInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(StoreInstruction)

START_INTROSPECTION(SyncInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(SyncInstruction)

START_INTROSPECTION(ReadARFInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(ReadARFInstruction)

START_INTROSPECTION(RegionInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(RegionInstruction)

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
    Instruction *insn = fn.newInstruction(*this);
    intrusive_list_node *prev = other->prev;
    insn->parent = other->parent;
    other->remove();
    append(insn, prev);
  }

  void Instruction::remove(void) {
    Function &fn = this->getFunction();
    unlink(this);
    fn.deleteInstruction(this);
  }

  void Instruction::insert(Instruction *prev, Instruction ** new_ins) {
    Function &fn = prev->getFunction();
    Instruction *insn = fn.newInstruction(*this);
    insn->parent = prev->parent;
    append(insn, prev);
    if (new_ins)
      *new_ins = insn;
  }

  bool Instruction::hasSideEffect(void) const {
    return opcode == OP_STORE ||
           opcode == OP_TYPED_WRITE ||
           opcode == OP_SYNC ||
           opcode == OP_ATOMIC;
  }

#define DECL_MEM_FN(CLASS, RET, PROTOTYPE, CALL) \
  RET CLASS::PROTOTYPE const { \
    return reinterpret_cast<const internal::CLASS*>(this)->CALL; \
  }

DECL_MEM_FN(UnaryInstruction, Type, getType(void), getType())
DECL_MEM_FN(BinaryInstruction, Type, getType(void), getType())
DECL_MEM_FN(BinaryInstruction, bool, commutes(void), commutes())
DECL_MEM_FN(SelectInstruction, Type, getType(void), getType())
DECL_MEM_FN(TernaryInstruction, Type, getType(void), getType())
DECL_MEM_FN(CompareInstruction, Type, getType(void), getType())
DECL_MEM_FN(BitCastInstruction, Type, getSrcType(void), getSrcType())
DECL_MEM_FN(BitCastInstruction, Type, getDstType(void), getDstType())
DECL_MEM_FN(ConvertInstruction, Type, getSrcType(void), getSrcType())
DECL_MEM_FN(ConvertInstruction, Type, getDstType(void), getDstType())
DECL_MEM_FN(AtomicInstruction, AddressSpace, getAddressSpace(void), getAddressSpace())
DECL_MEM_FN(AtomicInstruction, BTI, getBTI(void), getBTI())
DECL_MEM_FN(AtomicInstruction, AtomicOps, getAtomicOpcode(void), getAtomicOpcode())
DECL_MEM_FN(StoreInstruction, Type, getValueType(void), getValueType())
DECL_MEM_FN(StoreInstruction, uint32_t, getValueNum(void), getValueNum())
DECL_MEM_FN(StoreInstruction, AddressSpace, getAddressSpace(void), getAddressSpace())
DECL_MEM_FN(StoreInstruction, BTI, getBTI(void), getBTI())
DECL_MEM_FN(StoreInstruction, bool, isAligned(void), isAligned())
DECL_MEM_FN(LoadInstruction, Type, getValueType(void), getValueType())
DECL_MEM_FN(LoadInstruction, uint32_t, getValueNum(void), getValueNum())
DECL_MEM_FN(LoadInstruction, AddressSpace, getAddressSpace(void), getAddressSpace())
DECL_MEM_FN(LoadInstruction, BTI, getBTI(void), getBTI())
DECL_MEM_FN(LoadInstruction, bool, isAligned(void), isAligned())
DECL_MEM_FN(LoadImmInstruction, Type, getType(void), getType())
DECL_MEM_FN(LabelInstruction, LabelIndex, getLabelIndex(void), getLabelIndex())
DECL_MEM_FN(BranchInstruction, bool, isPredicated(void), isPredicated())
DECL_MEM_FN(BranchInstruction, bool, getInversePredicated(void), getInversePredicated())
DECL_MEM_FN(BranchInstruction, LabelIndex, getLabelIndex(void), getLabelIndex())
DECL_MEM_FN(SyncInstruction, uint32_t, getParameters(void), getParameters())
DECL_MEM_FN(ReadARFInstruction, Type, getType(void), getType())
DECL_MEM_FN(ReadARFInstruction, ARFRegister, getARFRegister(void), getARFRegister())
DECL_MEM_FN(RegionInstruction, uint32_t, getOffset(void), getOffset())
DECL_MEM_FN(SampleInstruction, Type, getSrcType(void), getSrcType())
DECL_MEM_FN(SampleInstruction, Type, getDstType(void), getDstType())
DECL_MEM_FN(SampleInstruction, uint8_t, getSamplerIndex(void), getSamplerIndex())
DECL_MEM_FN(SampleInstruction, uint8_t, getSamplerOffset(void), getSamplerOffset())
DECL_MEM_FN(SampleInstruction, uint8_t, getImageIndex(void), getImageIndex())
DECL_MEM_FN(TypedWriteInstruction, Type, getSrcType(void), getSrcType())
DECL_MEM_FN(TypedWriteInstruction, Type, getCoordType(void), getCoordType())
DECL_MEM_FN(TypedWriteInstruction, uint8_t, getImageIndex(void), getImageIndex())
DECL_MEM_FN(GetImageInfoInstruction, uint32_t, getInfoType(void), getInfoType())
DECL_MEM_FN(GetImageInfoInstruction, uint8_t, getImageIndex(void), getImageIndex())

#undef DECL_MEM_FN

  Immediate LoadImmInstruction::getImmediate(void) const {
    const Function &fn = this->getFunction();
    return reinterpret_cast<const internal::LoadImmInstruction*>(this)->getImmediate(fn);
  }

  ///////////////////////////////////////////////////////////////////////////
  // Implements the emission functions
  ///////////////////////////////////////////////////////////////////////////

  // For all unary functions with given opcode
  Instruction ALU1(Opcode opcode, Type type, Register dst, Register src) {
    return internal::UnaryInstruction(opcode, type, dst, src).convert();
  }

  // All unary functions
#define DECL_EMIT_FUNCTION(NAME) \
  Instruction NAME(Type type, Register dst, Register src) { \
    return ALU1(OP_##NAME, type, dst, src);\
  }

  DECL_EMIT_FUNCTION(MOV)
  DECL_EMIT_FUNCTION(FBH)
  DECL_EMIT_FUNCTION(FBL)
  DECL_EMIT_FUNCTION(CBIT)
  DECL_EMIT_FUNCTION(COS)
  DECL_EMIT_FUNCTION(SIN)
  DECL_EMIT_FUNCTION(LOG)
  DECL_EMIT_FUNCTION(SQR)
  DECL_EMIT_FUNCTION(RSQ)
  DECL_EMIT_FUNCTION(RNDD)
  DECL_EMIT_FUNCTION(RNDE)
  DECL_EMIT_FUNCTION(RNDU)
  DECL_EMIT_FUNCTION(RNDZ)

#undef DECL_EMIT_FUNCTION

  // All binary functions
#define DECL_EMIT_FUNCTION(NAME) \
  Instruction NAME(Type type, Register dst,  Register src0, Register src1) { \
    return internal::BinaryInstruction(OP_##NAME, type, dst, src0, src1).convert(); \
  }

  DECL_EMIT_FUNCTION(POW)
  DECL_EMIT_FUNCTION(MUL)
  DECL_EMIT_FUNCTION(ADD)
  DECL_EMIT_FUNCTION(ADDSAT)
  DECL_EMIT_FUNCTION(SUB)
  DECL_EMIT_FUNCTION(SUBSAT)
  DECL_EMIT_FUNCTION(MUL_HI)
  DECL_EMIT_FUNCTION(I64_MUL_HI)
  DECL_EMIT_FUNCTION(UPSAMPLE_SHORT)
  DECL_EMIT_FUNCTION(UPSAMPLE_INT)
  DECL_EMIT_FUNCTION(UPSAMPLE_LONG)
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
  DECL_EMIT_FUNCTION(HADD)
  DECL_EMIT_FUNCTION(RHADD)
  DECL_EMIT_FUNCTION(I64HADD)
  DECL_EMIT_FUNCTION(I64RHADD)

#undef DECL_EMIT_FUNCTION

  // SEL
  Instruction SEL(Type type, Register dst, Tuple src) {
    return internal::SelectInstruction(type, dst, src).convert();
  }

  Instruction I64MADSAT(Type type, Register dst, Tuple src) {
    return internal::TernaryInstruction(OP_I64MADSAT, type, dst, src).convert();
  }

  Instruction MAD(Type type, Register dst, Tuple src) {
    return internal::TernaryInstruction(OP_MAD, type, dst, src).convert();
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
  DECL_EMIT_FUNCTION(ORD)

#undef DECL_EMIT_FUNCTION

  // BITCAST
  Instruction BITCAST(Type dstType, Type srcType, Tuple dst, Tuple src, uint8_t dstNum, uint8_t srcNum) {
    return internal::BitCastInstruction(dstType, srcType, dst, src, dstNum, srcNum).convert();
  }

  // CVT
  Instruction CVT(Type dstType, Type srcType, Register dst, Register src) {
    return internal::ConvertInstruction(OP_CVT, dstType, srcType, dst, src).convert();
  }

  // saturated convert
  Instruction SAT_CVT(Type dstType, Type srcType, Register dst, Register src) {
    return internal::ConvertInstruction(OP_SAT_CVT, dstType, srcType, dst, src).convert();
  }

  // CVT
  Instruction F16TO32(Type dstType, Type srcType, Register dst, Register src) {
    return internal::ConvertInstruction(OP_F16TO32, dstType, srcType, dst, src).convert();
  }

  // saturated convert
  Instruction F32TO16(Type dstType, Type srcType, Register dst, Register src) {
    return internal::ConvertInstruction(OP_F32TO16, dstType, srcType, dst, src).convert();
  }

  // For all unary functions with given opcode
  Instruction ATOMIC(AtomicOps atomicOp, Register dst, AddressSpace space, BTI bti, Tuple src) {
    return internal::AtomicInstruction(atomicOp, dst, space, bti, src).convert();
  }

  // BRA
  Instruction BRA(LabelIndex labelIndex) {
    return internal::BranchInstruction(OP_BRA, labelIndex).convert();
  }
  Instruction BRA(LabelIndex labelIndex, Register pred) {
    return internal::BranchInstruction(OP_BRA, labelIndex, pred).convert();
  }

  // IF
  Instruction IF(LabelIndex labelIndex, Register pred, bool inv_pred) {
    return internal::BranchInstruction(OP_IF, labelIndex, pred, inv_pred).convert();
  }

  // ELSE
  Instruction ELSE(LabelIndex labelIndex) {
    return internal::BranchInstruction(OP_ELSE, labelIndex).convert();
  }
  // ENDIF
  Instruction ENDIF(LabelIndex labelIndex) {
    return internal::BranchInstruction(OP_ENDIF, labelIndex).convert();
  }

  // WHILE
  Instruction WHILE(LabelIndex labelIndex, Register pred) {
    return internal::BranchInstruction(OP_WHILE, labelIndex, pred).convert();
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
                   bool dwAligned, \
                   BTI bti) \
  { \
    return internal::CLASS(type,tuple,offset,space,valueNum,dwAligned,bti).convert(); \
  }

  DECL_EMIT_FUNCTION(LOAD, LoadInstruction)
  DECL_EMIT_FUNCTION(STORE, StoreInstruction)

#undef DECL_EMIT_FUNCTION

  // FENCE
  Instruction SYNC(uint32_t parameters) {
    return internal::SyncInstruction(parameters).convert();
  }

  Instruction READ_ARF(Type type, Register dst, ARFRegister arf) {
    return internal::ReadARFInstruction(type, dst, arf).convert();
  }
  Instruction REGION(Register dst, Register src, uint32_t offset) {
    return internal::RegionInstruction(dst, src, offset).convert();
  }

  // LABEL
  Instruction LABEL(LabelIndex labelIndex) {
    return internal::LabelInstruction(labelIndex).convert();
  }

  // SAMPLE
  Instruction SAMPLE(uint8_t imageIndex, Tuple dst, Tuple src, bool dstIsFloat, bool srcIsFloat, uint8_t sampler, uint8_t samplerOffset) {
    return internal::SampleInstruction(imageIndex, dst, src, dstIsFloat, srcIsFloat, sampler, samplerOffset).convert();
  }

  Instruction TYPED_WRITE(uint8_t imageIndex, Tuple src, Type srcType, Type coordType) {
    return internal::TypedWriteInstruction(imageIndex, src, srcType, coordType).convert();
  }

  Instruction GET_IMAGE_INFO(int infoType, Register dst, uint8_t imageIndex, Register infoReg) {
    return internal::GetImageInfoInstruction(infoType, dst, imageIndex, infoReg).convert();
  }

  std::ostream &operator<< (std::ostream &out, const Instruction &insn) {
    const Function &fn = insn.getFunction();
    const BasicBlock *bb = insn.getParent();
    switch (insn.getOpcode()) {
#define DECL_INSN(OPCODE, CLASS) \
      case OP_##OPCODE: \
          if(OP_##OPCODE == OP_ELSE) \
          { \
            reinterpret_cast<const internal::CLASS&>(insn).out(out, fn); \
            out << "  <**>label: " << bb->thisElseLabel; \
            break; \
          } \
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

