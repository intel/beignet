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
 * \file instruction.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_INSTRUCTION_HPP__
#define __GBE_IR_INSTRUCTION_HPP__

#include "ir/register.hpp"
#include "ir/immediate.hpp"
#include "ir/type.hpp"
#include "sys/platform.hpp"
#include "sys/intrusive_list.hpp"

#include <ostream>
#define MAX_MIXED_POINTER 4

namespace gbe {
namespace ir {
  struct BTI {
    uint8_t bti[MAX_MIXED_POINTER];
    uint8_t count;
    BTI() : count(0) {
      memset(bti, 0, MAX_MIXED_POINTER);
    }
    ~BTI() {}
  };

  /*! All opcodes */
  enum Opcode : uint8_t {
#define DECL_INSN(INSN, FAMILY) OP_##INSN,
#include "ir/instruction.hxx"
#undef DECL_INSN
    OP_INVALID
  };

  /*! Different memory spaces */
  enum AddressSpace : uint8_t {
    MEM_GLOBAL = 0, //!< Global memory (a la OCL)
    MEM_LOCAL,      //!< Local memory (thread group memory)
    MEM_CONSTANT,   //!< Immutable global memory
    MEM_PRIVATE,    //!< Per thread private memory
    IMAGE,          //!< For texture image.
    MEM_INVALID
  };

  enum AtomicOps {
    ATOMIC_OP_AND       = 1,
    ATOMIC_OP_OR        = 2,
    ATOMIC_OP_XOR       = 3,
    ATOMIC_OP_XCHG      = 4,
    ATOMIC_OP_INC       = 5,
    ATOMIC_OP_DEC       = 6,
    ATOMIC_OP_ADD       = 7,
    ATOMIC_OP_SUB       = 8,
    ATOMIC_OP_IMAX      = 10,
    ATOMIC_OP_IMIN      = 11,
    ATOMIC_OP_UMAX      = 12,
    ATOMIC_OP_UMIN      = 13,
    ATOMIC_OP_CMPXCHG   = 14,
    ATOMIC_OP_INVALID
  };

  /* Vote function per hardware thread */
  enum VotePredicate : uint8_t {
    VOTE_ALL = 0,
    VOTE_ANY
  };

  /*! Output the memory space */
  std::ostream &operator<< (std::ostream &out, AddressSpace addrSpace);

  /*! A label is identified with an unsigned short */
  TYPE_SAFE(LabelIndex, uint16_t)

  /*! Function class contains the register file and the register tuple. Any
   *  information related to the registers may therefore require a function
   */
  class Function;

  /*! Contains the stream of instructions */
  class BasicBlock;

  ///////////////////////////////////////////////////////////////////////////
  /// All public instruction classes as manipulated by all public classes
  ///////////////////////////////////////////////////////////////////////////

  /*! Stores instruction internal data and opcode */
  class ALIGNED(sizeof(uint64_t)*2) InstructionBase
  {
  public:
    /*! Initialize the instruction from a 8 bytes stream */
    INLINE InstructionBase(const char *stream) {
      opcode = Opcode(stream[0]);
      for (uint32_t byte = 0; byte < opaqueSize; ++byte)
        opaque[byte] = stream[byte+1];
    }
    /*! Uninitialized instruction */
    INLINE InstructionBase(void) {}
    /*! Get the instruction opcode */
    INLINE Opcode getOpcode(void) const { return opcode; }
  protected:
    enum { opaqueSize = sizeof(uint64_t)*2-sizeof(uint8_t) };
    Opcode opcode;               //!< Idendifies the instruction
    char opaque[opaqueSize];     //!< Remainder of it
    GBE_CLASS(InstructionBase);  //!< Use internal allocators
  };

  /*! Store the instruction description in 32 bytes */
  class Instruction : public InstructionBase, public intrusive_list_node
  {
  public:
    /*! Initialize the instruction from a 8 bytes stream */
    INLINE Instruction(const char *stream) : InstructionBase(stream) {
      parent = NULL;
    }
    /*! Copy the private fields and give it the same parent */
    INLINE Instruction(const Instruction &other) :
      InstructionBase(reinterpret_cast<const char*>(&other.opcode)) {
      parent = other.parent;
    }

  private:
    /*! To be consistant with copy constructor */
    INLINE Instruction &operator= (const Instruction &other) { return *this; }
  public:
    /*! Nothing to do here */
    INLINE ~Instruction(void) {}
    /*! Uninitialized instruction */
    INLINE Instruction(void) {}
    /*! Get the number of sources for this instruction  */
    uint32_t getSrcNum(void) const;
    /*! Get the number of destination for this instruction */
    uint32_t getDstNum(void) const;
    /*! Get the register index of the given source */
    Register getSrc(uint32_t ID = 0u) const;
    /*! Get the register index of the given destination */
    Register getDst(uint32_t ID = 0u) const;
    /*! Get the register of the given source */
    RegisterData getDstData(uint32_t ID = 0u) const;
    /*! Get the register of the given destination */
    RegisterData getSrcData(uint32_t ID = 0u) const;
    /*! Set a register in src srcID */
    void setSrc(uint32_t srcID, Register reg);
    /*! Set a register in dst dstID */
    void setDst(uint32_t dstID, Register reg);
    /*! Is there any side effect in the memory sub-system? */
    bool hasSideEffect(void) const;
    /*! Get / set the parent basic block */
    BasicBlock *getParent(void) { return parent; }
    const BasicBlock *getParent(void) const { return parent; }
    void setParent(BasicBlock *block) { this->parent = block; }
    /*! Get the function from the parent basic block */
    const Function &getFunction(void) const;
    Function &getFunction(void);
    /*! Check that the instruction is well formed (type properly match,
     *  registers not of bound and so on). If not well formed, provide a reason
     *  in string why
     */
    bool wellFormed(std::string &why) const;
    /*! Replace other by this instruction */
    void replace(Instruction *other) const;
    /*! Remove the instruction from the instruction stream */
    void remove(void);
    /* Insert the instruction after the previous one. */
    void insert(Instruction *prev, Instruction ** new_ins = NULL);
    /*! Indicates if the instruction belongs to instruction type T. Typically, T
     *  can be BinaryInstruction, UnaryInstruction, LoadInstruction and so on
     */
    template <typename T> INLINE bool isMemberOf(void) const {
      return T::isClassOf(*this);
    }
    /*! max_src for store instruction (vec16 + addr) */
    static const uint32_t MAX_SRC_NUM = 32;
    static const uint32_t MAX_DST_NUM = 32;
  protected:
    BasicBlock *parent;      //!< The basic block containing the instruction
    GBE_CLASS(Instruction);  //!< Use internal allocators
  };

  /*! Output the instruction string in the given stream */
  std::ostream &operator<< (std::ostream &out, const Instruction &proxy);

  /*! Unary instructions are typed. dst and sources share the same type */
  class UnaryInstruction : public Instruction {
  public:
    /*! Get the type manipulated by the instruction */
    Type getType(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Binary instructions are typed. dst and sources share the same type */
  class BinaryInstruction : public Instruction {
  public:
    /*! Get the type manipulated by the instruction */
    Type getType(void) const;
    /*! Commutative instructions can allow better optimizations */
    bool commutes(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Ternary instructions are typed. dst and sources share the same type */
  class TernaryInstruction : public Instruction {
   public:
    Type getType(void) const;
    static bool isClassOf(const Instruction &insn);
  };

  /*! Select instructions writes src0 to dst if cond is true. Otherwise, it
   *  writes src1
   */
  class SelectInstruction : public Instruction {
  public:
    /*! Predicate is in slot 0. So first source to selec is in slot 1 */
    static const uint32_t src0Index = 1;
    /*! Second source to select is in slot 2 */
    static const uint32_t src1Index = 2;
    /*! Get the predicate of the selection instruction */
    INLINE Register getPredicate(void) const { return this->getSrc(0); }
    /*! Get the type of both sources */
    Type getType(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Compare instructions compare anything from the same type and return a
   *  boolean value
   */
  class CompareInstruction : public Instruction {
  public:
    /*! Get the type of the source registers */
    Type getType(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! BitCast instruction converts from one type to another */
  class BitCastInstruction : public Instruction {
  public:
    /*! Get the type of the source */
    Type getSrcType(void) const;
    /*! Get the type of the destination */
    Type getDstType(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Conversion instruction converts from one type to another */
  class ConvertInstruction : public Instruction {
  public:
    /*! Get the type of the source */
    Type getSrcType(void) const;
    /*! Get the type of the destination */
    Type getDstType(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Atomic instruction */
  class AtomicInstruction : public Instruction {
  public:
    /*! Where the address register goes */
    static const uint32_t addressIndex = 0;
    /*! Address space that is manipulated here */
    AddressSpace getAddressSpace(void) const;
    BTI getBTI(void) const;
    /*! Return the atomic function code */
    AtomicOps getAtomicOpcode(void) const;
    /*! Return the register that contains the addresses */
    INLINE Register getAddress(void) const { return this->getSrc(addressIndex); }
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Store instruction. First source is the address. Next sources are the
   *  values to store contiguously at the given address
   */
  class StoreInstruction : public Instruction {
  public:
    /*! Where the address register goes */
    static const uint32_t addressIndex = 0;
    /*! Return the types of the values to store */
    Type getValueType(void) const;
    /*! Give the number of values the instruction is storing (srcNum-1) */
    uint32_t getValueNum(void) const;
    BTI getBTI(void) const;
    /*! Address space that is manipulated here */
    AddressSpace getAddressSpace(void) const;
    /*! DWORD aligned means untyped read for Gen. That is what matters */
    bool isAligned(void) const;
    /*! Return the register that contains the addresses */
    INLINE Register getAddress(void) const { return this->getSrc(addressIndex); }
    /*! Return the register that contain value valueID */
    INLINE Register getValue(uint32_t valueID) const {
      GBE_ASSERT(valueID < this->getValueNum());
      return this->getSrc(valueID + 1u);
    }
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Load instruction. The source is simply the address where to get the data.
   *  The multiple destinations are the contiguous values loaded at the given
   *  address
   */
  class LoadInstruction : public Instruction {
  public:
    /*! Type of the loaded values (ie type of all the destinations) */
    Type getValueType(void) const;
    /*! Number of values loaded (ie number of destinations) */
    uint32_t getValueNum(void) const;
    /*! Address space that is manipulated here */
    AddressSpace getAddressSpace(void) const;
    /*! DWORD aligned means untyped read for Gen. That is what matters */
    bool isAligned(void) const;
    /*! Return the register that contains the addresses */
    INLINE Register getAddress(void) const { return this->getSrc(0u); }
    BTI getBTI(void) const;
    /*! Return the register that contain value valueID */
    INLINE Register getValue(uint32_t valueID) const {
      return this->getDst(valueID);
    }
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Load immediate instruction loads an typed immediate value into the given
   *  register. Since double and uint64_t values will not fit into an
   *  instruction, the immediate themselves are stored in the function core.
   *  Contrary to regular load instructions, there is only one destination
   *  possible
   */
  class LoadImmInstruction : public Instruction {
  public:
    /*! Return the value stored in the instruction */
    Immediate getImmediate(void) const;
    /*! Return the type of the stored value */
    Type getType(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Store data in an texture */
  class TypedWriteInstruction : public Instruction {
  public:
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
    uint8_t getImageIndex() const;
    Type getSrcType(void) const;
    Type getCoordType(void) const;
  };

  /*! Load texels from a texture */
  class SampleInstruction : public Instruction {
  public:
    uint8_t getImageIndex() const;
    uint8_t getSamplerIndex(void) const;
    uint8_t getSamplerOffset(void) const;
    Type getSrcType(void) const;
    Type getDstType(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  typedef union _ImageInfoKey{
    _ImageInfoKey(uint8_t i, uint8_t t) : index(i), type(t) {};
    struct {
     uint8_t index; /*! the allocated image index */
     uint8_t  type;  /*! the information type */
    };
    uint16_t data;
  } ImageInfoKey;

  /*! Get image information */
  class GetImageInfoInstruction : public Instruction {
  public:
    enum {
     WIDTH = 0,
     HEIGHT = 1,
     DEPTH = 2,
     CHANNEL_DATA_TYPE = 3,
     CHANNEL_ORDER = 4,
    };

    static INLINE uint32_t getDstNum4Type(int infoType) {
      switch (infoType) {
        case WIDTH:
        case HEIGHT:
        case DEPTH:
        case CHANNEL_DATA_TYPE:
        case CHANNEL_ORDER:
          return 1;
        break;
        default:
          GBE_ASSERT(0);
     }
     return 0;
   }

    uint8_t getImageIndex() const;
    uint32_t getInfoType() const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Branch instruction is the unified way to branch (with or without
   *  predicate)
   */
  class BranchInstruction : public Instruction {
  public:
    /*! Indicate if the branch is predicated */
    bool isPredicated(void) const;
    /*! Indicate if the branch is inverse predicated */
    bool getInversePredicated(void) const;
    /*! Return the predicate register (if predicated) */
    RegisterData getPredicate(void) const {
      GBE_ASSERTM(this->isPredicated() == true, "Branch is not predicated");
      return this->getSrcData(0);
    }
    /*! Return the predicate register index (if predicated) */
    Register getPredicateIndex(void) const {
      GBE_ASSERTM(this->isPredicated() == true, "Branch is not predicated");
      return this->getSrc(0);
    }
    /*! Return the label index pointed by the branch */
    LabelIndex getLabelIndex(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Label instruction are actual no-op but are referenced by branches as their
   *  targets
   */
  class LabelInstruction : public Instruction {
  public:
    /*! Return the label index of the instruction */
    LabelIndex getLabelIndex(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Texture instruction are used for any texture mapping requests */
  class TextureInstruction : public Instruction {
  public:
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Mapped to OpenCL (mem_fence, read_mem_fence, write_mem_fence, barrier) */
  enum {
    SYNC_WORKGROUP_EXEC     = 1<<0,
    SYNC_LOCAL_READ_FENCE   = 1<<1,
    SYNC_LOCAL_WRITE_FENCE  = 1<<2,
    SYNC_GLOBAL_READ_FENCE  = 1<<3,
    SYNC_GLOBAL_WRITE_FENCE = 1<<4,
    SYNC_INVALID            = 1<<5
  };

  /*! 5 bits to encode all possible synchronization capablities */
  static const uint32_t syncFieldNum = 5u;

  /*! When barrier(CLK_LOCAL_MEM_FENCE) is issued */
  static const uint32_t syncLocalBarrier = SYNC_WORKGROUP_EXEC |SYNC_LOCAL_WRITE_FENCE | SYNC_LOCAL_READ_FENCE;

  /*! When barrier(CLK_GLOBAL_MEM_FENCE) is issued */
  static const uint32_t syncGlobalBarrier = SYNC_WORKGROUP_EXEC | SYNC_GLOBAL_WRITE_FENCE | SYNC_GLOBAL_READ_FENCE;

  /*! Sync instructions are used to order loads and stores for a given memory
   *  space and/or to serialize threads at a given point in the program
   */
  class SyncInstruction : public Instruction {
  public:
    /*! Get the parameters (bitfields) of the sync instructions (see above) */
    uint32_t getParameters(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Read one register (8 DWORD) in arf */
  class ReadARFInstruction : public Instruction {
  public:
    Type getType() const;
    ir::ARFRegister getARFRegister() const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! return a region of a register, make sure the offset does not exceed the register size */
  class RegionInstruction : public Instruction {
  public:
    uint32_t getOffset(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Specialize the instruction. Also performs typechecking first based on the
   *  opcode. Crashes if it fails
   */
  template <typename T>
  INLINE T *cast(Instruction *insn) {
    if(insn->isMemberOf<T>())
      return reinterpret_cast<T*>(insn);
    else
      return NULL;
  }
  template <typename T>
  INLINE const T *cast(const Instruction *insn) {
    if(insn->isMemberOf<T>())
      return reinterpret_cast<const T*>(insn);
    else
      return NULL;
  }
  template <typename T>
  INLINE T &cast(Instruction &insn) {
    GBE_ASSERTM(insn.isMemberOf<T>() == true, "Invalid instruction type");
    return reinterpret_cast<T&>(insn);
  }
  template <typename T>
  INLINE const T &cast(const Instruction &insn) {
    GBE_ASSERTM(insn.isMemberOf<T>() == true, "Invalid instruction type");
    return reinterpret_cast<const T&>(insn);
  }

  /*! Indicates if the given opcode belongs the given instruction family */
  template <typename T, typename U> struct EqualType {enum {value = false};};
  template <typename T> struct EqualType<T,T> { enum {value = true};};
  template <typename T>
  INLINE bool isOpcodeFrom(Opcode op) {
    switch (op) {
#define DECL_INSN(OPCODE, FAMILY) \
      case OP_##OPCODE: return EqualType<T, FAMILY>::value;
#include "instruction.hxx"
#undef DECL_INSN
      default: NOT_SUPPORTED; return false;
    }
  }

  ///////////////////////////////////////////////////////////////////////////
  /// All emission functions
  ///////////////////////////////////////////////////////////////////////////

  /*! alu1.type dst src */
  Instruction ALU1(Opcode opcode, Type type, Register dst, Register src);
  /*! mov.type dst src */
  Instruction MOV(Type type, Register dst, Register src);
  /*! cos.type dst src */
  Instruction COS(Type type, Register dst, Register src);
  /*! sin.type dst src */
  Instruction SIN(Type type, Register dst, Register src);
  /*! mul_hi.type dst src */
  Instruction MUL_HI(Type type, Register dst, Register src0, Register src1);
  /*! i64_mul_hi.type dst src */
  Instruction I64_MUL_HI(Type type, Register dst, Register src0, Register src1);
  /*! i64madsat.type dst src */
  Instruction I64MADSAT(Type type, Register dst, Tuple src);
  /*! mad.type dst src */
  Instruction MAD(Type type, Register dst, Tuple src);
  /*! upsample_short.type dst src */
  Instruction UPSAMPLE_SHORT(Type type, Register dst, Register src0, Register src1);
  /*! upsample_int.type dst src */
  Instruction UPSAMPLE_INT(Type type, Register dst, Register src0, Register src1);
  /*! upsample_long.type dst src */
  Instruction UPSAMPLE_LONG(Type type, Register dst, Register src0, Register src1);
  /*! fbh.type dst src */
  Instruction FBH(Type type, Register dst, Register src);
  /*! fbl.type dst src */
  Instruction FBL(Type type, Register dst, Register src);
  /*! cbit.type dst src */
  Instruction CBIT(Type type, Register dst, Register src);
  /*! hadd.type dst src */
  Instruction HADD(Type type, Register dst, Register src0, Register src1);
  /*! rhadd.type dst src */
  Instruction RHADD(Type type, Register dst, Register src0, Register src1);
  /*! i64hadd.type dst src */
  Instruction I64HADD(Type type, Register dst, Register src0, Register src1);
  /*! i64rhadd.type dst src */
  Instruction I64RHADD(Type type, Register dst, Register src0, Register src1);
  /*! tan.type dst src */
  Instruction RCP(Type type, Register dst, Register src);
  /*! abs.type dst src */
  Instruction ABS(Type type, Register dst, Register src);
  /*! simd_all.type dst src */
  Instruction SIMD_ALL(Type type, Register dst, Register src);
  /*! simd_any.type dst src */
  Instruction SIMD_ANY(Type type, Register dst, Register src);
  /*! log.type dst src */
  Instruction LOG(Type type, Register dst, Register src);
  /*! exp.type dst src */
  Instruction EXP(Type type, Register dst, Register src);
  /*! sqr.type dst src */
  Instruction SQR(Type type, Register dst, Register src);
  /*! rsq.type dst src */
  Instruction RSQ(Type type, Register dst, Register src);
  /*! rndd.type dst src */
  Instruction RNDD(Type type, Register dst, Register src);
  /*! rnde.type dst src */
  Instruction RNDE(Type type, Register dst, Register src);
  /*! rndu.type dst src */
  Instruction RNDU(Type type, Register dst, Register src);
  /*! rndz.type dst src */
  Instruction RNDZ(Type type, Register dst, Register src);
  /*! pow.type dst src0 src1 */
  Instruction POW(Type type, Register dst, Register src0, Register src1);
  /*! mul.type dst src0 src1 */
  Instruction MUL(Type type, Register dst, Register src0, Register src1);
  /*! add.type dst src0 src1 */
  Instruction ADD(Type type, Register dst, Register src0, Register src1);
  /*! addsat.type dst src0 src1 */
  Instruction ADDSAT(Type type, Register dst, Register src0, Register src1);
  /*! sub.type dst src0 src1 */
  Instruction SUB(Type type, Register dst, Register src0, Register src1);
  /*! subsat.type dst src0 src1 */
  Instruction SUBSAT(Type type, Register dst, Register src0, Register src1);
  /*! div.type dst src0 src1 */
  Instruction DIV(Type type, Register dst, Register src0, Register src1);
  /*! rem.type dst src0 src1 */
  Instruction REM(Type type, Register dst, Register src0, Register src1);
  /*! shl.type dst src0 src1 */
  Instruction SHL(Type type, Register dst, Register src0, Register src1);
  /*! shr.type dst src0 src1 */
  Instruction SHR(Type type, Register dst, Register src0, Register src1);
  /*! asr.type dst src0 src1 */
  Instruction ASR(Type type, Register dst, Register src0, Register src1);
  /*! bsf.type dst src0 src1 */
  Instruction BSF(Type type, Register dst, Register src0, Register src1);
  /*! bsb.type dst src0 src1 */
  Instruction BSB(Type type, Register dst, Register src0, Register src1);
  /*! or.type dst src0 src1 */
  Instruction OR(Type type, Register dst, Register src0, Register src1);
  /*! xor.type dst src0 src1 */
  Instruction XOR(Type type, Register dst, Register src0, Register src1);
  /*! and.type dst src0 src1 */
  Instruction AND(Type type, Register dst, Register src0, Register src1);
  /*! sel.type dst {cond, src0, src1} (== src) */
  Instruction SEL(Type type, Register dst, Tuple src);
  /*! eq.type dst src0 src1 */
  Instruction EQ(Type type, Register dst, Register src0, Register src1);
  /*! ne.type dst src0 src1 */
  Instruction NE(Type type, Register dst, Register src0, Register src1);
  /*! lt.type dst src0 src1 */
  Instruction LE(Type type, Register dst, Register src0, Register src1);
  /*! le.type dst src0 src1 */
  Instruction LT(Type type, Register dst, Register src0, Register src1);
  /*! gt.type dst src0 src1 */
  Instruction GE(Type type, Register dst, Register src0, Register src1);
  /*! ge.type dst src0 src1 */
  Instruction GT(Type type, Register dst, Register src0, Register src1);
  /*! ord.type dst src0 src1 */
  Instruction ORD(Type type, Register dst, Register src0, Register src1);
  /*! BITCAST.{dstType <- srcType} dst src */
  Instruction BITCAST(Type dstType, Type srcType, Tuple dst, Tuple src, uint8_t dstNum, uint8_t srcNum);
  /*! cvt.{dstType <- srcType} dst src */
  Instruction CVT(Type dstType, Type srcType, Register dst, Register src);
  /*! sat_cvt.{dstType <- srcType} dst src */
  Instruction SAT_CVT(Type dstType, Type srcType, Register dst, Register src);
  /*! F16TO32.{dstType <- srcType} dst src */
  Instruction F16TO32(Type dstType, Type srcType, Register dst, Register src);
  /*! F32TO16.{dstType <- srcType} dst src */
  Instruction F32TO16(Type dstType, Type srcType, Register dst, Register src);
  /*! atomic dst addr.space {src1 {src2}} */
  Instruction ATOMIC(AtomicOps opcode, Register dst, AddressSpace space, BTI bti, Tuple src);
  /*! bra labelIndex */
  Instruction BRA(LabelIndex labelIndex);
  /*! (pred) bra labelIndex */
  Instruction BRA(LabelIndex labelIndex, Register pred);
  /*! (pred) if labelIndex */
  Instruction IF(LabelIndex labelIndex, Register pred, bool inv_pred=true);
  /*! else labelIndex */
  Instruction ELSE(LabelIndex labelIndex);
  /*! endif */
  Instruction ENDIF(LabelIndex labelIndex);
  /*! (pred) while labelIndex */
  Instruction WHILE(LabelIndex labelIndex, Register pred);
  /*! ret */
  Instruction RET(void);
  /*! load.type.space {dst1,...,dst_valueNum} offset value */
  Instruction LOAD(Type type, Tuple dst, Register offset, AddressSpace space, uint32_t valueNum, bool dwAligned, BTI bti);
  /*! store.type.space offset {src1,...,src_valueNum} value */
  Instruction STORE(Type type, Tuple src, Register offset, AddressSpace space, uint32_t valueNum, bool dwAligned, BTI bti);
  /*! loadi.type dst value */
  Instruction LOADI(Type type, Register dst, ImmediateIndex value);
  /*! sync.params... (see Sync instruction) */
  Instruction SYNC(uint32_t parameters);

  Instruction READ_ARF(Type type, Register dst, ARFRegister arf);
  Instruction REGION(Register dst, Register src, uint32_t offset);
  /*! typed write */
  Instruction TYPED_WRITE(uint8_t imageIndex, Tuple src, Type srcType, Type coordType);
  /*! sample textures */
  Instruction SAMPLE(uint8_t imageIndex, Tuple dst, Tuple src, bool dstIsFloat, bool srcIsFloat, uint8_t sampler, uint8_t samplerOffset);
  /*! get image information , such as width/height/depth/... */
  Instruction GET_IMAGE_INFO(int infoType, Register dst, uint8_t imageIndex, Register infoReg);
  /*! label labelIndex */
  Instruction LABEL(LabelIndex labelIndex);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_INSTRUCTION_HPP__ */

