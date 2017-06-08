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

    /*! All 0-source arithmetic instructions */
    class ALIGNED_INSTRUCTION NullaryInstruction : public NaryInstruction<0>
    {
    public:
      NullaryInstruction(Opcode opcode, Type type, Register dst) {
        this->opcode = opcode;
        this->type = type;
        this->dst[0] = dst;
      }
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

    class ALIGNED_INSTRUCTION MemInstruction :
      public BasePolicy
    {
    public:
      MemInstruction(AddressMode   _AM,
                     AddressSpace _AS,
                     bool _dwAligned,
                     Type _type,
                     Register _offset)
                   : AM(_AM),
                     AS(_AS),
                     dwAligned(_dwAligned),
                     type(_type),
                     SurfaceIndex(0),
                     offset(_offset) {
      }
      AddressMode  getAddressMode()    const { return AM; }
      AddressSpace getAddressSpace()   const { return AS; }
      /*! MemInstruction may have one possible btiReg */
      Register     getBtiReg()         const { assert(AM == AM_DynamicBti); return BtiReg; }
      unsigned     getSurfaceIndex()   const { assert(AM != AM_DynamicBti); return SurfaceIndex; }
      Register     getAddressRegister()const { return offset; }
      unsigned     getAddressIndex()   const { return 0; }
      Type         getValueType()      const { return type; }
      INLINE bool  isAligned(void)     const { return !!dwAligned; }

      void         setSurfaceIndex (unsigned id)  { SurfaceIndex = id; }
      void         setBtiReg(Register reg)        { BtiReg = reg;      }
    protected:
      /*! including address reg + optional bti reg */
      int          getBaseSrcNum()    const { return AM == AM_DynamicBti ? 2 : 1; }
      bool         hasExtraBtiReg()   const { return AM == AM_DynamicBti; }
      AddressMode       AM;
      AddressSpace      AS;
      uint8_t           dwAligned : 1;
      Type              type;
      union {
        Register        BtiReg;
        unsigned        SurfaceIndex;
      };
      Register          offset;
    };

    class ALIGNED_INSTRUCTION AtomicInstruction :
      public MemInstruction,
      public NDstPolicy<AtomicInstruction, 1>
    {
    public:
      AtomicInstruction(AtomicOps atomicOp,
                         Type type,
                         Register dst,
                         AddressSpace addrSpace,
                         Register address,
                         Tuple payload,
                         AddressMode AM)
        : MemInstruction(AM, addrSpace, true, type, address)
      {
        this->opcode = OP_ATOMIC;
        this->atomicOp = atomicOp;
        this->dst[0] = dst;
        this->payload = payload;

        int payloadNum = 1;
        if((atomicOp == ATOMIC_OP_INC) ||
          (atomicOp == ATOMIC_OP_DEC))
          payloadNum = 0;
        if(atomicOp == ATOMIC_OP_CMPXCHG)
          payloadNum = 2;

        srcNum = payloadNum + getBaseSrcNum();
      }
      INLINE Register getSrc(const Function &fn, uint32_t ID) const {
        GBE_ASSERTM((int)ID < (int)srcNum, "Out-of-bound source register for atomic");
        if (ID == 0) {
          return offset;
        } else if (hasExtraBtiReg() && (int)ID == (int)srcNum-1) {
          return getBtiReg();
        } else {
          return fn.getRegister(payload, ID - 1);
        }
      }
      INLINE void setSrc(Function &fn, uint32_t ID, Register reg) {
        GBE_ASSERTM((int)ID < (int)srcNum, "Out-of-bound source register for atomic");
        if (ID == 0) {
          offset = reg;
        } else if (hasExtraBtiReg() && (int)ID == (int)srcNum - 1) {
          setBtiReg(reg);
        } else {
          fn.setRegister(payload, ID - 1, reg);
        }
      }
      INLINE uint32_t getSrcNum(void) const { return srcNum; }

      INLINE AtomicOps getAtomicOpcode(void) const { return this->atomicOp; }
      INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Register dst[1];
      Tuple payload;
      uint8_t srcNum:3;     //!<Source Number
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
      public MemInstruction
    {
      public:
        LoadInstruction(Type type,
                        Tuple dstValues,
                        Register offset,
                        AddressSpace AS,
                        uint32_t _valueNum,
                        bool dwAligned,
                        AddressMode AM,
                        bool ifBlock = false)
                      : MemInstruction(AM, AS, dwAligned, type, offset),
                        valueNum(_valueNum),
                        values(dstValues),
                        ifBlock(ifBlock)
        {
          this->opcode = OP_LOAD;
        }

        INLINE unsigned getSrcNum() const { return getBaseSrcNum(); }
        INLINE Register getSrc(const Function &fn, unsigned id) const {
          if (id == 0) return offset;
          if (hasExtraBtiReg() && id == 1) return BtiReg;
          assert(0 && "LoadInstruction::getSrc() out-of-range");
          return ir::Register(0);
        }
        INLINE void     setSrc(Function &fn, unsigned id, Register reg) {
          assert(id < getSrcNum());
          if (id == 0) { offset = reg;   return; }
          if (id == 1) { setBtiReg(reg); return; }
        }
        INLINE unsigned getDstNum() const { return valueNum; }
        INLINE Register getDst(const Function &fn, unsigned id) const {
          assert(id < valueNum);
          return fn.getRegister(values, id);
        }
        INLINE void     setDst(Function &fn, unsigned id, Register reg) {
          assert(id < getDstNum());
          fn.setRegister(values, id, reg);
        }
        INLINE uint32_t getValueNum(void) const { return valueNum; }
        INLINE Register getValue(const Function &fn, unsigned id) const {
          assert(id < valueNum);
          return fn.getRegister(values, id);
        }
        INLINE bool wellFormed(const Function &fn, std::string &why) const;
        INLINE void out(std::ostream &out, const Function &fn) const;
        INLINE bool isBlock() const { return ifBlock; }

        uint8_t         valueNum;
        Tuple             values;
        bool             ifBlock;
    };
    class ALIGNED_INSTRUCTION StoreInstruction :
      public MemInstruction,
      public NDstPolicy<StoreInstruction, 0>
    {
      public:
        StoreInstruction(Type type,
                         Tuple values,
                         Register offset,
                         AddressSpace addrSpace,
                         uint32_t valueNum,
                         bool dwAligned,
                         AddressMode AM,
                         bool ifBlock = false)
          : MemInstruction(AM, addrSpace, dwAligned, type, offset)
        {
          this->opcode = OP_STORE;
          this->values = values;
          this->valueNum = valueNum;
          this->ifBlock = ifBlock;
        }
        INLINE unsigned getValueNum()      const { return valueNum; }
        INLINE Register getValue(const Function &fn, unsigned id) const {
          return fn.getRegister(values, id);
        }
        INLINE unsigned getSrcNum()        const { return getBaseSrcNum() + valueNum; }
        INLINE Register getSrc(const Function &fn, unsigned id) const {
          if (id == 0)  return offset;
          if (id <= valueNum) return fn.getRegister(values, id-1);
          if (hasExtraBtiReg() && (int)id == (int)valueNum+1) return getBtiReg();
          assert(0 && "StoreInstruction::getSrc() out-of-range");
          return Register(0);
        }
        INLINE void     setSrc(Function &fn, unsigned id, Register reg) {
          if (id == 0)                   { offset = reg; return; }
          if (id > 0 && id <= valueNum)  { fn.setRegister(values, id-1, reg); return; }
          if (hasExtraBtiReg() &&
              (int)id == (int)valueNum + 1)        {
            setBtiReg(reg);
            return;
          }
          assert(0 && "StoreInstruction::setSrc() index out-of-range");
        }
        INLINE bool wellFormed(const Function &fn, std::string &why) const;
        INLINE void out(std::ostream &out, const Function &fn) const;
        INLINE bool isBlock() const { return ifBlock; }

        Register      dst[0];
        uint8_t     valueNum;
        Tuple         values;
        bool         ifBlock;
    };

    class ALIGNED_INSTRUCTION SampleInstruction : // TODO
      public BasePolicy,
      public TupleSrcPolicy<SampleInstruction>,
      public TupleDstPolicy<SampleInstruction>
    {
    public:
      SampleInstruction(uint8_t imageIdx, Tuple dstTuple, Tuple srcTuple, uint8_t srcNum, bool dstIsFloat, bool srcIsFloat, uint8_t sampler, uint8_t samplerOffset) {
        this->opcode = OP_SAMPLE;
        this->dst = dstTuple;
        this->src = srcTuple;
        this->srcNum = srcNum;
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
            << " surface id " << (int)this->getImageIndex();
        out << " coord u %" << this->getSrc(fn, 0);
        if (srcNum >= 2)
          out << " coord v %" << this->getSrc(fn, 1);
        if (srcNum >= 3)
          out << " coord w %" << this->getSrc(fn, 2);
        out
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
      uint8_t srcNum;
      static const uint32_t dstNum = 4;
    };

    class ALIGNED_INSTRUCTION VmeInstruction :
      public BasePolicy,
      public TupleSrcPolicy<VmeInstruction>,
      public TupleDstPolicy<VmeInstruction>
    {
    public:
      VmeInstruction(uint8_t imageIdx, Tuple dstTuple, Tuple srcTuple,
                     uint32_t dstNum, uint32_t srcNum, int msg_type,
                     int vme_search_path_lut, int lut_sub) {
        this->opcode = OP_VME;
        this->dst = dstTuple;
        this->src = srcTuple;
        this->dstNum = dstNum;
        this->srcNum = srcNum;
        this->imageIdx = imageIdx;
        this->msg_type = msg_type;
        this->vme_search_path_lut = vme_search_path_lut;
        this->lut_sub = lut_sub;
      }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const {
        this->outOpcode(out);
        out << " src_surface id " << (int)this->getImageIndex()
            << " ref_surface id " << (int)this->getImageIndex() + 1;
        for(uint32_t i = 0; i < dstNum; i++){
          out<< " %" << this->getDst(fn, i);
        }
        for(uint32_t i = 0; i < srcNum; i++){
          out<< " %" << this->getSrc(fn, i);
        }
        out
            << " msg_type " << (int)this->getMsgType()
            << " vme_search_path_lut " << (int)this->vme_search_path_lut
            << " lut_sub " << (int)this->lut_sub;
      }
      Tuple src;
      Tuple dst;

      INLINE uint8_t getImageIndex(void) const { return this->imageIdx; }
      INLINE uint8_t getMsgType(void) const { return this->msg_type; }

      INLINE Type getSrcType(void) const { return TYPE_U32; }
      INLINE Type getDstType(void) const { return TYPE_U32; }
      uint8_t imageIdx;
      uint8_t msg_type;
      uint8_t vme_search_path_lut;
      uint8_t lut_sub;
      uint32_t srcNum;
      uint32_t dstNum;
    };


    class ALIGNED_INSTRUCTION TypedWriteInstruction : // TODO
      public BasePolicy,
      public TupleSrcPolicy<TypedWriteInstruction>,
      public NDstPolicy<TypedWriteInstruction, 0>
    {
    public:

      INLINE TypedWriteInstruction(uint8_t imageIdx, Tuple srcTuple, uint8_t srcNum, Type srcType, Type coordType) {
        this->opcode = OP_TYPED_WRITE;
        this->src = srcTuple;
        this->srcNum = srcNum;
        this->coordType = coordType;
        this->srcType = srcType;
        this->imageIdx = imageIdx;
      }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const {
        this->outOpcode(out);
        uint32_t srcID = 0;
        out << "." << this->getSrcType()
            << " surface id " << (int)this->getImageIndex()
            << " coord u %" << this->getSrc(fn, srcID++);
        if (srcNum >= 6)
          out << " coord v %" << this->getSrc(fn, srcID++);
        if (srcNum >= 7)
          out << " coord w %" << this->getSrc(fn, srcID++);
        out   << " %" << this->getSrc(fn, srcID++);
        out   << " %" << this->getSrc(fn, srcID++);
        out   << " %" << this->getSrc(fn, srcID++);
        out   << " %" << this->getSrc(fn, srcID++);
      }

      Tuple src;
      uint8_t srcType;
      uint8_t coordType;
      uint8_t imageIdx;
      // bti, u, [v], [w], 4 data elements
      uint8_t srcNum;

      INLINE uint8_t getImageIndex(void) const { return this->imageIdx; }
      INLINE Type getSrcType(void) const { return (Type)this->srcType; }
      INLINE Type getCoordType(void) const { return (Type)this->coordType; }
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

    class ALIGNED_INSTRUCTION CalcTimestampInstruction :
      public BasePolicy,
      public NSrcPolicy<CalcTimestampInstruction, 0>,
      public NDstPolicy<CalcTimestampInstruction, 0>
    {
    public:
      CalcTimestampInstruction(uint32_t pointNum, uint32_t timestampType) {
        this->opcode = OP_CALC_TIMESTAMP;
        this->timestampType = static_cast<uint8_t>(timestampType);
        this->pointNum = static_cast<uint8_t>(pointNum);
      }

      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const {
        this->outOpcode(out);
        out << "TimeStamp pointer " << static_cast<uint32_t>(pointNum)
          << " (Type " << static_cast<uint32_t>(timestampType) << ")";
      }
      uint32_t getPointNum(void) const { return this->pointNum; }
      uint32_t getTimestamptType(void) const { return this->timestampType; }
      uint8_t timestampType;       //!< Type of the time stamp, 16bits or 32bits, eg.
      uint8_t pointNum;            //!< The insert point number.
      Register dst[0], src[0];
    };

    class ALIGNED_INSTRUCTION StoreProfilingInstruction :
      public BasePolicy,
      public NSrcPolicy<StoreProfilingInstruction, 0>,
      public NDstPolicy<StoreProfilingInstruction, 0>
    {
    public:
      StoreProfilingInstruction(uint32_t bti, uint32_t profilingType) {
        this->opcode = OP_STORE_PROFILING;
        this->profilingType = static_cast<uint8_t>(profilingType);
        this->bti = static_cast<uint8_t>(bti);
      }

      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const {
        this->outOpcode(out);
        out << " BTI " << static_cast<uint32_t>(this->bti)
          << " (Type " << static_cast<uint32_t>(this->profilingType) << ")";
      }

      uint32_t getProfilingType(void) const { return this->profilingType; }
      uint32_t getBTI(void) const { return this->bti; }
      uint8_t profilingType;     //!< Type format of profiling, 16bits or 32bits, eg.
      uint8_t bti;
      Register src[0];
      Register dst[0];
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
      INLINE void setImmediateIndex(ImmediateIndex immIndex) {
        immediateIndex = immIndex;
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

    class ALIGNED_INSTRUCTION SimdShuffleInstruction : public NaryInstruction<2>
    {
    public:
      SimdShuffleInstruction(Type type,
                        Register dst,
                        Register src0,
                        Register src1) {
        this->opcode = OP_SIMD_SHUFFLE;
        this->type = type;
        this->dst[0] = dst;
        this->src[0] = src0;
        this->src[1] = src1;
      }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
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

    class ALIGNED_INSTRUCTION IndirectMovInstruction :
      public BasePolicy,
      public NSrcPolicy<IndirectMovInstruction, 2>,
      public NDstPolicy<IndirectMovInstruction, 1>
    {
    public:
      INLINE IndirectMovInstruction(Type type, Register dst, Register src0, Register src1, uint32_t offset) {
        this->type = type;
        this->offset = offset;
        this->dst[0] = dst;
        this->src[0] = src0;
        this->src[1] = src1;
        this->opcode = OP_INDIRECT_MOV;
      }
      INLINE Type getType(void) const { return this->type; }
      INLINE uint32_t getOffset(void) const { return this->offset; }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Type type;
      uint32_t offset;
      Register dst[1];
      Register src[2];
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

    /*! Wait instructions */
    class ALIGNED_INSTRUCTION WaitInstruction :
      public BasePolicy,
      public NSrcPolicy<WaitInstruction, 0>,
      public NDstPolicy<WaitInstruction, 0>
    {
    public:
      INLINE WaitInstruction() {
        this->opcode = OP_WAIT;
      }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const;
      Register dst[0], src[0];
    };

    class ALIGNED_INSTRUCTION WorkGroupInstruction :
      public BasePolicy,
      public TupleSrcPolicy<WorkGroupInstruction>,
      public NDstPolicy<WorkGroupInstruction, 1>
    {
      public:
        INLINE WorkGroupInstruction(WorkGroupOps opcode, uint32_t slmAddr, Register dst,
            Tuple srcTuple, uint8_t srcNum, Type type) {
          this->opcode = OP_WORKGROUP;
          this->workGroupOp = opcode;
          this->type = type;
          this->dst[0] = dst;
          this->src = srcTuple;
          this->srcNum = srcNum;
          this->slmAddr = slmAddr;
        }
        INLINE Type getType(void) const { return this->type; }
        INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
        INLINE void out(std::ostream &out, const Function &fn) const;
        INLINE WorkGroupOps getWorkGroupOpcode(void) const { return this->workGroupOp; }
        uint32_t getSlmAddr(void) const { return this->slmAddr; }

        WorkGroupOps workGroupOp:5;
        uint32_t srcNum:3;          //!< Source Number
        uint32_t slmAddr:24;        //!< Thread Map in SLM.
        Type type;                  //!< Type of the instruction
        Tuple src;
        Register dst[1];
    };

    class ALIGNED_INSTRUCTION SubGroupInstruction :
      public BasePolicy,
      public TupleSrcPolicy<SubGroupInstruction>,
      public NDstPolicy<SubGroupInstruction, 1>
    {
      public:
        INLINE SubGroupInstruction(WorkGroupOps opcode, Register dst,
            Tuple srcTuple, uint8_t srcNum, Type type) {
          this->opcode = OP_SUBGROUP;
          this->workGroupOp = opcode;
          this->type = type;
          this->dst[0] = dst;
          this->src = srcTuple;
          this->srcNum = srcNum;
        }
        INLINE Type getType(void) const { return this->type; }
        INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
        INLINE void out(std::ostream &out, const Function &fn) const;
        INLINE WorkGroupOps getWorkGroupOpcode(void) const { return this->workGroupOp; }

        WorkGroupOps workGroupOp:5;
        uint32_t srcNum:3;          //!< Source Number
        Type type;                  //!< Type of the instruction
        Tuple src;
        Register dst[1];
    };

    class ALIGNED_INSTRUCTION PrintfInstruction :
      public BasePolicy,
      public TupleSrcPolicy<PrintfInstruction>,
      public NDstPolicy<PrintfInstruction, 1>
    {
      public:
        INLINE PrintfInstruction(Register dst, Tuple srcTuple, Tuple typeTuple,
                                 uint8_t srcNum, uint8_t bti, uint16_t num) {
          this->opcode = OP_PRINTF;
          this->dst[0] = dst;
          this->src = srcTuple;
          this->type = typeTuple;
          this->srcNum = srcNum;
          this->bti = bti;
          this->num = num;
        }
        INLINE bool wellFormed(const Function &fn, std::string &whyNot) const;
        INLINE void out(std::ostream &out, const Function &fn) const;

        uint32_t getNum(void) const { return this->num; }
        uint32_t getBti(void) const { return this->bti; }
        Type getType(const Function& fn, uint32_t ID) const {
          GBE_ASSERTM(ID < this->srcNum, "Out-of-bound types");
          return (Type)fn.getType(type, ID);
        }

        uint32_t srcNum:8;    //!< Source Number
        uint32_t bti:8;       //!< The BTI
        uint32_t num:16;      //!< The printf statement number of one kernel.
        Tuple src;
        Tuple type;
        Register dst[1];
    };

    class ALIGNED_INSTRUCTION MediaBlockReadInstruction :
      public BasePolicy,
      public TupleSrcPolicy<MediaBlockReadInstruction>,
      public TupleDstPolicy<MediaBlockReadInstruction>
    {
    public:
      INLINE MediaBlockReadInstruction(uint8_t imageIdx, Tuple dst, uint8_t vec_size, Tuple srcTuple, uint8_t srcNum, Type type, uint8_t width, uint8_t height) {
        this->opcode = OP_MBREAD;
        this->dst = dst;
        this->dstNum = vec_size;
        this->src = srcTuple;
        this->srcNum = srcNum;
        this->imageIdx = imageIdx;
        this->type = type;
        this->width = width;
        this->height = height;
      }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const {
        this->outOpcode(out);
        out << "." << type << "."
            << (int)this->getVectorSize();
        out << " {";
        for (uint32_t i = 0; i < dstNum; ++i)
          out << "%" << this->getDst(fn, i) << (i != (dstNum-1u) ? " " : "");
        out << "}";
        out << " 2D surface id " << (int)this->getImageIndex()
            << " byte coord x %" << this->getSrc(fn, 0)
            << " row coord y %" << this->getSrc(fn, 1);
      }
      INLINE uint8_t getImageIndex(void) const { return this->imageIdx; }
      INLINE uint8_t getVectorSize(void) const { return this->dstNum; }
      INLINE Type getType(void) const { return this->type; }
      INLINE uint8_t getWidth(void) const { return this->width; }
      INLINE uint8_t getHeight(void) const { return this->height; }

      Tuple src;
      Tuple dst;
      uint8_t imageIdx;
      uint8_t srcNum;
      uint8_t dstNum;
      Type type;
      uint8_t width;
      uint8_t height;
    };

    class ALIGNED_INSTRUCTION MediaBlockWriteInstruction :
      public BasePolicy,
      public TupleSrcPolicy<MediaBlockWriteInstruction>,
      public NDstPolicy<MediaBlockWriteInstruction, 0>
    {
    public:

      INLINE MediaBlockWriteInstruction(uint8_t imageIdx, Tuple srcTuple, uint8_t srcNum, uint8_t vec_size, Type type, uint8_t width, uint8_t height) {
        this->opcode = OP_MBWRITE;
        this->src = srcTuple;
        this->srcNum = srcNum;
        this->imageIdx = imageIdx;
        this->vec_size = vec_size;
        this->type = type;
        this->width = width;
        this->height = height;
      }
      INLINE bool wellFormed(const Function &fn, std::string &why) const;
      INLINE void out(std::ostream &out, const Function &fn) const {
        this->outOpcode(out);
        out << "." << type << "."
            << (int)this->getVectorSize()
            << " 2D surface id " << (int)this->getImageIndex()
            << " byte coord x %" << this->getSrc(fn, 0)
            << " row coord y %" << this->getSrc(fn, 1);
        out << " {";
        for (uint32_t i = 0; i < vec_size; ++i)
          out << "%" << this->getSrc(fn, i + 2) << (i != (vec_size-1u) ? " " : "");
        out << "}";
      }
      INLINE uint8_t getImageIndex(void) const { return this->imageIdx; }
      INLINE uint8_t getVectorSize(void) const { return this->vec_size; }
      INLINE Type getType(void) const { return this->type; }
      INLINE uint8_t getWidth(void) const { return this->width; }
      INLINE uint8_t getHeight(void) const { return this->height; }

      Tuple src;
      Register dst[0];
      uint8_t imageIdx;
      uint8_t srcNum;
      uint8_t vec_size;
      Type type;
      uint8_t width;
      uint8_t height;
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
      if (UNLIKELY(ID.value() >= fn.regNum())) {
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
                                      TYPE_HALF, TYPE_FLOAT, TYPE_DOUBLE};
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
      if (opcode != OP_CBIT &&
          UNLIKELY(checkRegisterData(family, dst[0], fn, whyNot) == false))
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
      const RegisterFamily family = getFamily(this->type);
      if (UNLIKELY(checkRegisterData(family, dst[0], fn, whyNot) == false))
        return false;
      for (uint32_t srcID = 0; srcID < srcNum-1u; ++srcID)
        if (UNLIKELY(checkRegisterData(family, getSrc(fn, srcID+1u), fn, whyNot) == false))
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
      if (UNLIKELY(insn.getAddressRegister() >= fn.regNum())) {
        whyNot = "Out-of-bound offset register index";
        return false;
      }
      if (UNLIKELY(insn.values + insn.valueNum > fn.tupleNum())) {
        whyNot = "Out-of-bound tuple index";
        return false;
      }

      // Check all registers
      const RegisterFamily family = getFamily(insn.getValueType());
      for (uint32_t valueID = 0; valueID < insn.getValueNum(); ++valueID) {
        const Register regID = insn.getValue(fn, valueID);;
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
    INLINE bool VmeInstruction::wellFormed(const Function &fn, std::string &why) const
    { return true; }
    INLINE bool TypedWriteInstruction::wellFormed(const Function &fn, std::string &why) const
    { return true; }
    INLINE bool GetImageInfoInstruction::wellFormed(const Function &fn, std::string &why) const
    { return true; }
    INLINE bool WaitInstruction::wellFormed(const Function &fn, std::string &why) const
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
                                 SYNC_GLOBAL_WRITE_FENCE |
                                 SYNC_IMAGE_FENCE;
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

    INLINE bool SimdShuffleInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      if (UNLIKELY( this->type != TYPE_U32 && this->type != TYPE_S32 && this->type != TYPE_FLOAT &&
                    this->type != TYPE_U16 && this->type != TYPE_S16)) {
        whyNot = "Only support S16/U16/S32/U32/FLOAT type";
        return false;
      }

      if (UNLIKELY(checkRegisterData(FAMILY_DWORD, src[1], fn, whyNot) == false))
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

    INLINE bool IndirectMovInstruction::wellFormed(const Function &fn, std::string &whyNot) const
    {
      const RegisterFamily family = getFamily(this->type);
      if (UNLIKELY(checkSpecialRegForWrite(dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(family, dst[0], fn, whyNot) == false))
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

    INLINE bool CalcTimestampInstruction::wellFormed(const Function &fn, std::string &whyNot) const {
      if (UNLIKELY(this->timestampType != 1)) {
        whyNot = "Wrong time stamp type";
        return false;
      }
      if (UNLIKELY(this->pointNum >= 20 && this->pointNum != 0xff && this->pointNum != 0xfe)) {
        whyNot = "To much Insert pointer";
        return false;
      }
      return true;
    }

    INLINE bool StoreProfilingInstruction::wellFormed(const Function &fn, std::string &whyNot) const {
      if (UNLIKELY(this->profilingType != 1)) {
        whyNot = "Wrong profiling format";
        return false;
      }
      return true;
    }

    INLINE bool WorkGroupInstruction::wellFormed(const Function &fn, std::string &whyNot) const {
      const RegisterFamily family = getFamily(this->type);

      if (UNLIKELY(checkSpecialRegForWrite(dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(family, dst[0], fn, whyNot) == false))
        return false;

      switch (this->workGroupOp) {
        case WORKGROUP_OP_ANY:
        case WORKGROUP_OP_ALL:
        case WORKGROUP_OP_REDUCE_ADD:
        case WORKGROUP_OP_REDUCE_MIN:
        case WORKGROUP_OP_REDUCE_MAX:
        case WORKGROUP_OP_INCLUSIVE_ADD:
        case WORKGROUP_OP_INCLUSIVE_MIN:
        case WORKGROUP_OP_INCLUSIVE_MAX:
        case WORKGROUP_OP_EXCLUSIVE_ADD:
        case WORKGROUP_OP_EXCLUSIVE_MIN:
        case WORKGROUP_OP_EXCLUSIVE_MAX:
          if (this->srcNum != 3) {
            whyNot = "Wrong number of source.";
            return false;
          }
          break;
        case WORKGROUP_OP_BROADCAST:
          if (this->srcNum <= 1) {
            whyNot = "Wrong number of source.";
            return false;
          } else {
            const RegisterFamily fam = fn.getPointerFamily();
            for (uint32_t srcID = 1; srcID < this->srcNum; ++srcID) {
              const Register regID = fn.getRegister(src, srcID);
              if (UNLIKELY(checkRegisterData(fam, regID, fn, whyNot) == false))
                return false;
            }
          }
          break;
        default:
          whyNot = "No such work group function.";
          return false;
      }

      return true;
    }

    INLINE bool SubGroupInstruction::wellFormed(const Function &fn, std::string &whyNot) const {
      const RegisterFamily family = getFamily(this->type);

      if (UNLIKELY(checkSpecialRegForWrite(dst[0], fn, whyNot) == false))
        return false;
      if (UNLIKELY(checkRegisterData(family, dst[0], fn, whyNot) == false))
        return false;

      switch (this->workGroupOp) {
        case WORKGROUP_OP_ANY:
        case WORKGROUP_OP_ALL:
        case WORKGROUP_OP_REDUCE_ADD:
        case WORKGROUP_OP_REDUCE_MIN:
        case WORKGROUP_OP_REDUCE_MAX:
        case WORKGROUP_OP_INCLUSIVE_ADD:
        case WORKGROUP_OP_INCLUSIVE_MIN:
        case WORKGROUP_OP_INCLUSIVE_MAX:
        case WORKGROUP_OP_EXCLUSIVE_ADD:
        case WORKGROUP_OP_EXCLUSIVE_MIN:
        case WORKGROUP_OP_EXCLUSIVE_MAX:
          if (this->srcNum != 1) {
            whyNot = "Wrong number of source.";
            return false;
          }
          break;
        case WORKGROUP_OP_BROADCAST:
          if (this->srcNum != 2) {
            whyNot = "Wrong number of source.";
            return false;
          } else {
            if (UNLIKELY(checkRegisterData(FAMILY_DWORD, fn.getRegister(src, 1), fn, whyNot) == false))
              return false;
          }
          break;
        default:
          whyNot = "No such sub group function.";
          return false;
      }

      return true;
    }

    INLINE bool PrintfInstruction::wellFormed(const Function &fn, std::string &whyNot) const {
      return true;
    }

    INLINE bool MediaBlockReadInstruction::wellFormed(const Function &fn, std::string &whyNot) const {
      if (this->srcNum != 2) {
        whyNot = "Wrong number of source.";
        return false;
      }
      return true;
    }

    INLINE bool MediaBlockWriteInstruction::wellFormed(const Function &fn, std::string &whyNot) const {
      if (this->srcNum != 2 + this->vec_size) {
        whyNot = "Wrong number of source.";
        return false;
      }
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
      out << "." << AS;

#define OUT_ATOMIC_OP(TYPE)     \
      case ATOMIC_OP_##TYPE:    \
      {    out << "." << #TYPE; \
          break; \
      }
      switch(atomicOp)
      {
        OUT_ATOMIC_OP(AND)
        OUT_ATOMIC_OP(OR)
        OUT_ATOMIC_OP(XOR)
        OUT_ATOMIC_OP(XCHG)
        OUT_ATOMIC_OP(INC)
        OUT_ATOMIC_OP(DEC)
        OUT_ATOMIC_OP(ADD)
        OUT_ATOMIC_OP(SUB)
        OUT_ATOMIC_OP(IMAX)
        OUT_ATOMIC_OP(IMIN)
        OUT_ATOMIC_OP(UMAX)
        OUT_ATOMIC_OP(UMIN)
        OUT_ATOMIC_OP(CMPXCHG)
        default:
          out << "." << "INVALID";
          assert(0);
      };
      out << " %" << this->getDst(fn, 0);
      out << " {" << "%" << this->getSrc(fn, 0) << "}";
      for (uint32_t i = 1; i < srcNum; ++i)
        out << " %" << this->getSrc(fn, i);
      AddressMode am = this->getAddressMode();
      out << " bti:";
      if ( am == AM_DynamicBti) {
        out << " %" << this->getBtiReg();
      } else {
        out << this->getSurfaceIndex();
      }
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
      if(ifBlock)
        out<< "BLOCK";
      this->outOpcode(out);
      out << "." << type << "." << AS << (dwAligned ? "." : ".un") << "aligned";
      out << " {";
      for (uint32_t i = 0; i < valueNum; ++i)
        out << "%" << this->getDst(fn, i) << (i != (valueNum-1u) ? " " : "");
      out << "}";
      out << " %" << this->getSrc(fn, 0);
      AddressMode am = this->getAddressMode();
      out << " bti:";
      if ( am == AM_DynamicBti) {
        out << " %" << this->getBtiReg();
      } else {
        out << this->getSurfaceIndex();
      }
    }

    INLINE void StoreInstruction::out(std::ostream &out, const Function &fn) const {
      if(ifBlock)
        out<< "BLOCK";
      this->outOpcode(out);
      out << "." << type << "." << AS << (dwAligned ? "." : ".un") << "aligned";
      out << " %" << this->getSrc(fn, 0) << " {";
      for (uint32_t i = 0; i < valueNum; ++i)
        out << "%" << this->getSrc(fn, i+1) << (i != (valueNum-1u) ? " " : "");
      out << "}";
      AddressMode am = this->getAddressMode();
      out << " bti:";
      if ( am == AM_DynamicBti) {
        out << " %" << this->getBtiReg();
      } else {
        out << this->getSurfaceIndex();
      }
    }

    INLINE void ReadARFInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << " %" << this->getDst(fn, 0) << " arf:" << arf;
    }

    INLINE void RegionInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << " %" << this->getDst(fn, 0) << " %" << this->getSrc(fn, 0) << " offset: " << this->offset;
    }

    INLINE void IndirectMovInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      out << "." << type << " %" << this->getDst(fn, 0) << " %" << this->getSrc(fn, 0);
      out << " %" << this->getSrc(fn, 1) << " offset: " << this->offset;
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
      "workgroup", "local_read", "local_write", "global_read", "global_write", "image"
    };

    INLINE void SyncInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
      for (uint32_t field = 0; field < syncFieldNum; ++field)
        if (this->parameters & (1 << field))
          out << "." << syncStr[field];
    }

    INLINE void WaitInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
    }

    INLINE void WorkGroupInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);

      switch (this->workGroupOp) {
        case WORKGROUP_OP_ANY:
          out << "_" << "ANY";
          break;
        case WORKGROUP_OP_ALL:
          out << "_" << "ALL";
          break;
        case WORKGROUP_OP_REDUCE_ADD:
          out << "_" << "REDUCE_ADD";
          break;
        case WORKGROUP_OP_REDUCE_MIN:
          out << "_" << "REDUCE_MIN";
          break;
        case WORKGROUP_OP_REDUCE_MAX:
          out << "_" << "REDUCE_MAX";
          break;
        case WORKGROUP_OP_INCLUSIVE_ADD:
          out << "_" << "INCLUSIVE_ADD";
          break;
        case WORKGROUP_OP_INCLUSIVE_MIN:
          out << "_" << "INCLUSIVE_MIN";
          break;
        case WORKGROUP_OP_INCLUSIVE_MAX:
          out << "_" << "INCLUSIVE_MAX";
          break;
        case WORKGROUP_OP_EXCLUSIVE_ADD:
          out << "_" << "EXCLUSIVE_ADD";
          break;
        case WORKGROUP_OP_EXCLUSIVE_MIN:
          out << "_" << "EXCLUSIVE_MIN";
          break;
        case WORKGROUP_OP_EXCLUSIVE_MAX:
          out << "_" << "EXCLUSIVE_MAX";
          break;
        case WORKGROUP_OP_BROADCAST:
          out << "_" << "BROADCAST";
          break;
        default:
          GBE_ASSERT(0);
      }

      out << " %" << this->getDst(fn, 0);
      for (uint32_t i = 0; i < this->getSrcNum(); ++i)
        out << " %" << this->getSrc(fn, i);

      if (this->workGroupOp == WORKGROUP_OP_BROADCAST) {
        do {
          int localN = srcNum - 1;
          GBE_ASSERT(localN);
          out << " Local X:";
          out << " %" << this->getSrc(fn, 1);
          localN--;
          if (!localN)
            break;

          out << " Local Y:";
          out << " %" << this->getSrc(fn, 2);
          localN--;
          if (!localN)
            break;

          out << " Local Z:";
          out << " %" << this->getSrc(fn, 3);
          localN--;
          GBE_ASSERT(!localN);
        } while(0);
      }

      out << " (TheadID Map at SLM: " << this->slmAddr << ")";
    }

    INLINE void SubGroupInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);

      switch (this->workGroupOp) {
        case WORKGROUP_OP_ANY:
          out << "_" << "ANY";
          break;
        case WORKGROUP_OP_ALL:
          out << "_" << "ALL";
          break;
        case WORKGROUP_OP_REDUCE_ADD:
          out << "_" << "REDUCE_ADD";
          break;
        case WORKGROUP_OP_REDUCE_MIN:
          out << "_" << "REDUCE_MIN";
          break;
        case WORKGROUP_OP_REDUCE_MAX:
          out << "_" << "REDUCE_MAX";
          break;
        case WORKGROUP_OP_INCLUSIVE_ADD:
          out << "_" << "INCLUSIVE_ADD";
          break;
        case WORKGROUP_OP_INCLUSIVE_MIN:
          out << "_" << "INCLUSIVE_MIN";
          break;
        case WORKGROUP_OP_INCLUSIVE_MAX:
          out << "_" << "INCLUSIVE_MAX";
          break;
        case WORKGROUP_OP_EXCLUSIVE_ADD:
          out << "_" << "EXCLUSIVE_ADD";
          break;
        case WORKGROUP_OP_EXCLUSIVE_MIN:
          out << "_" << "EXCLUSIVE_MIN";
          break;
        case WORKGROUP_OP_EXCLUSIVE_MAX:
          out << "_" << "EXCLUSIVE_MAX";
          break;
        case WORKGROUP_OP_BROADCAST:
          out << "_" << "BROADCAST";
          break;
        default:
          GBE_ASSERT(0);
      }

      out << " %" << this->getDst(fn, 0);
      out << " %" << this->getSrc(fn, 0);

      if (this->workGroupOp == WORKGROUP_OP_BROADCAST) {
        do {
          int localN = srcNum - 1;
          GBE_ASSERT(localN);
          out << " Local ID:";
          out << " %" << this->getSrc(fn, 1);
          localN--;
          if (!localN)
            break;
        } while(0);
      }

    }

    INLINE void PrintfInstruction::out(std::ostream &out, const Function &fn) const {
      this->outOpcode(out);
    }

  } /* namespace internal */

  std::ostream &operator<< (std::ostream &out, AddressSpace addrSpace) {
    switch (addrSpace) {
      case MEM_GLOBAL: return out << "global";
      case MEM_LOCAL: return out << "local";
      case MEM_CONSTANT: return out << "constant";
      case MEM_PRIVATE: return out << "private";
      case MEM_MIXED: return out << "mixed";
      case MEM_GENERIC: return out << "generic";
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
  static_assert(sizeof(internal::CLASS) == (sizeof(uint64_t)*4), \
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

START_INTROSPECTION(NullaryInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(NullaryInstruction)

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

START_INTROSPECTION(CalcTimestampInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(CalcTimestampInstruction)

START_INTROSPECTION(StoreProfilingInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(StoreProfilingInstruction)

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

START_INTROSPECTION(SimdShuffleInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(SimdShuffleInstruction)

START_INTROSPECTION(IndirectMovInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(IndirectMovInstruction)

START_INTROSPECTION(LabelInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(LabelInstruction)

START_INTROSPECTION(WaitInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(WaitInstruction)

START_INTROSPECTION(VmeInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(VmeInstruction)

START_INTROSPECTION(WorkGroupInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(WorkGroupInstruction)

START_INTROSPECTION(SubGroupInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(SubGroupInstruction)

START_INTROSPECTION(PrintfInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(PrintfInstruction)

START_INTROSPECTION(MediaBlockReadInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(MediaBlockReadInstruction)

START_INTROSPECTION(MediaBlockWriteInstruction)
#include "ir/instruction.hxx"
END_INTROSPECTION(MediaBlockWriteInstruction)

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
           opcode == OP_ATOMIC ||
           opcode == OP_CALC_TIMESTAMP ||
           opcode == OP_STORE_PROFILING ||
           opcode == OP_WAIT ||
           opcode == OP_PRINTF ||
           opcode == OP_MBWRITE;
  }

#define DECL_MEM_FN(CLASS, RET, PROTOTYPE, CALL) \
  RET CLASS::PROTOTYPE const { \
    return reinterpret_cast<const internal::CLASS*>(this)->CALL; \
  }

DECL_MEM_FN(NullaryInstruction, Type, getType(void), getType())
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
DECL_MEM_FN(MemInstruction, AddressSpace, getAddressSpace(void), getAddressSpace())
DECL_MEM_FN(MemInstruction, AddressMode, getAddressMode(void), getAddressMode())
DECL_MEM_FN(MemInstruction, Register, getAddressRegister(void), getAddressRegister())
DECL_MEM_FN(MemInstruction, Register, getBtiReg(void), getBtiReg())
DECL_MEM_FN(MemInstruction, unsigned, getSurfaceIndex(void), getSurfaceIndex())
DECL_MEM_FN(MemInstruction, Type,     getValueType(void), getValueType())
DECL_MEM_FN(MemInstruction, bool,     isAligned(void), isAligned())
DECL_MEM_FN(MemInstruction, unsigned, getAddressIndex(void), getAddressIndex())
DECL_MEM_FN(AtomicInstruction, AtomicOps, getAtomicOpcode(void), getAtomicOpcode())
DECL_MEM_FN(StoreInstruction, uint32_t, getValueNum(void), getValueNum())
DECL_MEM_FN(StoreInstruction, bool, isBlock(void), isBlock())
DECL_MEM_FN(LoadInstruction, uint32_t, getValueNum(void), getValueNum())
DECL_MEM_FN(LoadInstruction, bool, isBlock(void), isBlock())
DECL_MEM_FN(LoadImmInstruction, Type, getType(void), getType())
DECL_MEM_FN(LabelInstruction, LabelIndex, getLabelIndex(void), getLabelIndex())
DECL_MEM_FN(BranchInstruction, bool, isPredicated(void), isPredicated())
DECL_MEM_FN(BranchInstruction, bool, getInversePredicated(void), getInversePredicated())
DECL_MEM_FN(BranchInstruction, LabelIndex, getLabelIndex(void), getLabelIndex())
DECL_MEM_FN(SyncInstruction, uint32_t, getParameters(void), getParameters())
DECL_MEM_FN(ReadARFInstruction, Type, getType(void), getType())
DECL_MEM_FN(ReadARFInstruction, ARFRegister, getARFRegister(void), getARFRegister())
DECL_MEM_FN(SimdShuffleInstruction, Type, getType(void), getType())
DECL_MEM_FN(RegionInstruction, uint32_t, getOffset(void), getOffset())
DECL_MEM_FN(IndirectMovInstruction, uint32_t, getOffset(void), getOffset())
DECL_MEM_FN(IndirectMovInstruction, Type, getType(void), getType())
DECL_MEM_FN(SampleInstruction, Type, getSrcType(void), getSrcType())
DECL_MEM_FN(SampleInstruction, Type, getDstType(void), getDstType())
DECL_MEM_FN(SampleInstruction, uint8_t, getSamplerIndex(void), getSamplerIndex())
DECL_MEM_FN(SampleInstruction, uint8_t, getSamplerOffset(void), getSamplerOffset())
DECL_MEM_FN(SampleInstruction, uint8_t, getImageIndex(void), getImageIndex())
DECL_MEM_FN(VmeInstruction, Type, getSrcType(void), getSrcType())
DECL_MEM_FN(VmeInstruction, Type, getDstType(void), getDstType())
DECL_MEM_FN(VmeInstruction, uint8_t, getImageIndex(void), getImageIndex())
DECL_MEM_FN(VmeInstruction, uint8_t, getMsgType(void), getMsgType())
DECL_MEM_FN(TypedWriteInstruction, Type, getSrcType(void), getSrcType())
DECL_MEM_FN(TypedWriteInstruction, Type, getCoordType(void), getCoordType())
DECL_MEM_FN(TypedWriteInstruction, uint8_t, getImageIndex(void), getImageIndex())
DECL_MEM_FN(GetImageInfoInstruction, uint32_t, getInfoType(void), getInfoType())
DECL_MEM_FN(GetImageInfoInstruction, uint8_t, getImageIndex(void), getImageIndex())
DECL_MEM_FN(CalcTimestampInstruction, uint32_t, getPointNum(void), getPointNum())
DECL_MEM_FN(CalcTimestampInstruction, uint32_t, getTimestamptType(void), getTimestamptType())
DECL_MEM_FN(StoreProfilingInstruction, uint32_t, getProfilingType(void), getProfilingType())
DECL_MEM_FN(StoreProfilingInstruction, uint32_t, getBTI(void), getBTI())
DECL_MEM_FN(WorkGroupInstruction, Type, getType(void), getType())
DECL_MEM_FN(WorkGroupInstruction, WorkGroupOps, getWorkGroupOpcode(void), getWorkGroupOpcode())
DECL_MEM_FN(WorkGroupInstruction, uint32_t, getSlmAddr(void), getSlmAddr())
DECL_MEM_FN(SubGroupInstruction, Type, getType(void), getType())
DECL_MEM_FN(SubGroupInstruction, WorkGroupOps, getWorkGroupOpcode(void), getWorkGroupOpcode())
DECL_MEM_FN(PrintfInstruction, uint32_t, getNum(void), getNum())
DECL_MEM_FN(PrintfInstruction, uint32_t, getBti(void), getBti())
DECL_MEM_FN(PrintfInstruction, Type, getType(const Function& fn, uint32_t ID), getType(fn, ID))
DECL_MEM_FN(MediaBlockReadInstruction, uint8_t, getImageIndex(void), getImageIndex())
DECL_MEM_FN(MediaBlockReadInstruction, uint8_t, getVectorSize(void), getVectorSize())
DECL_MEM_FN(MediaBlockReadInstruction, Type, getType(void), getType())
DECL_MEM_FN(MediaBlockReadInstruction, uint8_t, getWidth(void), getWidth())
DECL_MEM_FN(MediaBlockReadInstruction, uint8_t, getHeight(void), getHeight())
DECL_MEM_FN(MediaBlockWriteInstruction, uint8_t, getImageIndex(void), getImageIndex())
DECL_MEM_FN(MediaBlockWriteInstruction, uint8_t, getVectorSize(void), getVectorSize())
DECL_MEM_FN(MediaBlockWriteInstruction, Type, getType(void), getType())
DECL_MEM_FN(MediaBlockWriteInstruction, uint8_t, getWidth(void), getWidth())
DECL_MEM_FN(MediaBlockWriteInstruction, uint8_t, getHeight(void), getHeight())

#undef DECL_MEM_FN

#define DECL_MEM_FN(CLASS, RET, PROTOTYPE, CALL) \
  RET CLASS::PROTOTYPE { \
    return reinterpret_cast<internal::CLASS*>(this)->CALL; \
  }
DECL_MEM_FN(MemInstruction, void,     setSurfaceIndex(unsigned id), setSurfaceIndex(id))
DECL_MEM_FN(MemInstruction, void,     setBtiReg(Register reg), setBtiReg(reg))

#undef DECL_MEM_FN

  Immediate LoadImmInstruction::getImmediate(void) const {
    const Function &fn = this->getFunction();
    return reinterpret_cast<const internal::LoadImmInstruction*>(this)->getImmediate(fn);
  }

  void LoadImmInstruction::setImmediateIndex(ImmediateIndex immIndex) {
    reinterpret_cast<internal::LoadImmInstruction*>(this)->setImmediateIndex(immIndex);
  }

  ///////////////////////////////////////////////////////////////////////////
  // Implements the emission functions
  ///////////////////////////////////////////////////////////////////////////
  // For all nullary functions with given opcode
  Instruction ALU0(Opcode opcode, Type type, Register dst) {
    return internal::NullaryInstruction(opcode, type, dst).convert();
  }

  // All nullary functions
#define DECL_EMIT_FUNCTION(NAME) \
  Instruction NAME(Type type, Register dst) { \
    return ALU0(OP_##NAME, type, dst);\
  }

  DECL_EMIT_FUNCTION(SIMD_SIZE)
  DECL_EMIT_FUNCTION(SIMD_ID)

#undef DECL_EMIT_FUNCTION

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
  DECL_EMIT_FUNCTION(LZD)
  DECL_EMIT_FUNCTION(COS)
  DECL_EMIT_FUNCTION(SIN)
  DECL_EMIT_FUNCTION(LOG)
  DECL_EMIT_FUNCTION(SQR)
  DECL_EMIT_FUNCTION(RSQ)
  DECL_EMIT_FUNCTION(RNDD)
  DECL_EMIT_FUNCTION(RNDE)
  DECL_EMIT_FUNCTION(RNDU)
  DECL_EMIT_FUNCTION(RNDZ)
  DECL_EMIT_FUNCTION(BFREV)

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

  Instruction LRP(Type type, Register dst, Tuple src) {
    return internal::TernaryInstruction(OP_LRP, type, dst, src).convert();
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
  Instruction ATOMIC(AtomicOps atomicOp, Type type, Register dst, AddressSpace space, Register address, Tuple payload, AddressMode AM, Register bti) {
    internal::AtomicInstruction insn = internal::AtomicInstruction(atomicOp, type, dst, space, address, payload, AM);
    insn.setBtiReg(bti);
    return insn.convert();
  }

  Instruction ATOMIC(AtomicOps atomicOp, Type type, Register dst, AddressSpace space, Register address, Tuple payload, AddressMode AM, unsigned SurfaceIndex) {
    internal::AtomicInstruction insn = internal::AtomicInstruction(atomicOp, type, dst, space, address, payload, AM);
    insn.setSurfaceIndex(SurfaceIndex);
    return insn.convert();
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
                   AddressMode AM, \
                   unsigned SurfaceIndex, \
                   bool isBlock) \
  { \
    internal::CLASS insn = internal::CLASS(type,tuple,offset,space,valueNum,dwAligned,AM, isBlock); \
    insn.setSurfaceIndex(SurfaceIndex);\
    return insn.convert(); \
  } \
  Instruction NAME(Type type, \
                   Tuple tuple, \
                   Register offset, \
                   AddressSpace space, \
                   uint32_t valueNum, \
                   bool dwAligned, \
                   AddressMode AM, \
                   Register bti) \
  { \
    internal::CLASS insn = internal::CLASS(type,tuple,offset,space,valueNum,dwAligned,AM); \
    insn.setBtiReg(bti); \
    return insn.convert(); \
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
  Instruction SIMD_SHUFFLE(Type type, Register dst, Register src0, Register src1) {
    return internal::SimdShuffleInstruction(type, dst, src0, src1).convert();
  }

  Instruction INDIRECT_MOV(Type type, Register dst, Register src0, Register src1, uint32_t offset) {
    return internal::IndirectMovInstruction(type, dst, src0, src1, offset).convert();
  }

  // LABEL
  Instruction LABEL(LabelIndex labelIndex) {
    return internal::LabelInstruction(labelIndex).convert();
  }

  // SAMPLE
  Instruction SAMPLE(uint8_t imageIndex, Tuple dst, Tuple src, uint8_t srcNum, bool dstIsFloat, bool srcIsFloat, uint8_t sampler, uint8_t samplerOffset) {
    return internal::SampleInstruction(imageIndex, dst, src, srcNum, dstIsFloat, srcIsFloat, sampler, samplerOffset).convert();
  }

  Instruction VME(uint8_t imageIndex, Tuple dst, Tuple src, uint32_t dstNum, uint32_t srcNum, int msg_type, int vme_search_path_lut, int lut_sub) {
    return internal::VmeInstruction(imageIndex, dst, src, dstNum, srcNum, msg_type, vme_search_path_lut, lut_sub).convert();
  }

  Instruction TYPED_WRITE(uint8_t imageIndex, Tuple src, uint8_t srcNum, Type srcType, Type coordType) {
    return internal::TypedWriteInstruction(imageIndex, src, srcNum, srcType, coordType).convert();
  }

  Instruction GET_IMAGE_INFO(int infoType, Register dst, uint8_t imageIndex, Register infoReg) {
    return internal::GetImageInfoInstruction(infoType, dst, imageIndex, infoReg).convert();
  }

  Instruction CALC_TIMESTAMP(uint32_t pointNum, uint32_t tsType) {
    return internal::CalcTimestampInstruction(pointNum, tsType).convert();
  }

  Instruction STORE_PROFILING(uint32_t bti, uint32_t profilingType) {
    return internal::StoreProfilingInstruction(bti, profilingType).convert();
  }

  // WAIT
  Instruction WAIT(void) {
    return internal::WaitInstruction().convert();
  }

  Instruction WORKGROUP(WorkGroupOps opcode, uint32_t slmAddr, Register dst, Tuple srcTuple, uint8_t srcNum, Type type) {
    return internal::WorkGroupInstruction(opcode, slmAddr, dst, srcTuple, srcNum, type).convert();
  }

  Instruction SUBGROUP(WorkGroupOps opcode, Register dst, Tuple srcTuple, uint8_t srcNum, Type type) {
    return internal::SubGroupInstruction(opcode, dst, srcTuple, srcNum, type).convert();
  }

  Instruction PRINTF(Register dst, Tuple srcTuple, Tuple typeTuple, uint8_t srcNum, uint8_t bti, uint16_t num) {
    return internal::PrintfInstruction(dst, srcTuple, typeTuple, srcNum, bti, num).convert();
  }

  Instruction MBREAD(uint8_t imageIndex, Tuple dst, uint8_t vec_size, Tuple coord, uint8_t srcNum, Type type, uint8_t width, uint8_t height) {
    return internal::MediaBlockReadInstruction(imageIndex, dst, vec_size, coord, srcNum, type, width, height).convert();
  }

  Instruction MBWRITE(uint8_t imageIndex, Tuple srcTuple, uint8_t srcNum, uint8_t vec_size, Type type, uint8_t width, uint8_t height) {
    return internal::MediaBlockWriteInstruction(imageIndex, srcTuple, srcNum, vec_size, type, width, height).convert();
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

