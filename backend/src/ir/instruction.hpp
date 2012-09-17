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
 * \file instruction.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_INSTRUCTION_HPP__
#define __GBE_IR_INSTRUCTION_HPP__

#include "ir/register.hpp"
#include "ir/immediate.hpp"
#include "ir/type.hpp"
#include "sys/platform.hpp"

#include <ostream>

namespace gbe {
namespace ir {

  /*! All opcodes */
  enum Opcode : char {
#define DECL_INSN(INSN, FAMILY) OP_##INSN,
#include "ir/instruction.hxx"
#undef DECL_INSN
  };

  /*! Different memory spaces */
  enum AddressSpace : uint8_t {
    MEM_GLOBAL = 0, //!< Global memory (a la OCL)
    MEM_LOCAL,      //!< Local memory (thread group memory)
    MEM_CONSTANT,   //!< Immutable global memory
    MEM_PRIVATE     //!< Per thread private memory
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

  /*! Store the instruction description in 8 bytes */
  class ALIGNED(sizeof(uint64_t)) Instruction
  {
  public:
    /*! Initialize the instruction from a 8 bytes stream */
    INLINE Instruction(const char *stream) {
      opcode = Opcode(stream[0]);
      for (uint32_t byte = 0; byte < opaqueSize; ++byte)
        opaque[byte] = stream[byte+1];
      predecessor = successor = NULL;
      parent = NULL;
    }
    /*! Uninitialize instruction */
    INLINE Instruction(void) {}
    /*! Get the instruction opcode */
    INLINE Opcode getOpcode(void) const { return opcode; }
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
    /*! Get / set the previous instruction in the stream */
    Instruction *getPredecessor(bool stayInBlock = true);
    const Instruction *getPredecessor(bool stayInBlock = true) const;
    void setPredecessor(Instruction *insn) { this->predecessor = insn; }
    /*! Get / set the next instruction in the stream */
    Instruction *getSuccessor(bool stayInBlock = true);
    const Instruction *getSuccessor(bool stayInBlock = true) const;
    void setSuccessor(Instruction *insn) { this->successor = insn; }
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
    /*! Indicates if the instruction belongs to instruction type T. Typically, T
     *  can be BinaryInstruction, UnaryInstruction, LoadInstruction and so on
     */
    template <typename T> INLINE bool isMemberOf(void) const {
      return T::isClassOf(*this);
    }

  protected:
    enum { opaqueSize = sizeof(uint64_t)-sizeof(uint8_t) };
    Opcode opcode;           //!< Idendifies the instruction
    char opaque[opaqueSize]; //!< Remainder of it
    Instruction *predecessor;//!< Previous instruction in the basic block
    Instruction *successor;  //!< Next instruction in the basic block
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
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Ternary instructions is mostly for MADs */
  class TernaryInstruction : public Instruction {
  public:
    /*! Get the type manipulated by the instruction */
    Type getType(void) const;
    /*! Return true if the given instruction is an instance of this class */
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
  };

  /*! Load texels from a texture */
  class SampleInstruction : public Instruction {
  public:
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

  /*! Fence instructions are used to order loads and stores for a given memory
   *  space
   */
  class FenceInstruction : public Instruction {
  public:
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Register region instructions are specific to OpenCL Gen and allow to
   *  manipulate the register file and to do cross lane shuffles (Gen extension)
   */
  class RegionInstruction : public Instruction {
  public:
    /*! Return the offset index (0..7) for the strided load*/
    uint32_t getOffset(void) const;
    /*! Return the vertical stride (0,1,2,4,8) */
    uint32_t getVStride(void) const;
    /*! Return the width (0,1,2,4,8) */
    uint32_t getWidth(void) const;
    /*! Return the horizontal stride (0,1,2,4,8) */
    uint32_t getHStride(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Vote instruction that operates accross lanes from the same hardware
   *  thread (Gen extension)
   */
  class VoteInstruction : public Instruction {
  public:
    /*! Return the vote predicate */
    VotePredicate getVotePredicate(void) const;
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! Gather from register file instruction. Similar to register region but with
   *  indirect addressing (Gen extension)
   */
  class RGatherInstruction : public Instruction {
  public:
    /*!< Source ID for the indices */
    static const uint32_t indexID = 0;
    /*! Get the indices for the gather */
    INLINE Register getIndices(void) const { return this->getSrc(indexID); }
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! OBlock read. Only the first lane is considered for the address
   *  (Gen extension)
   */
  class OBReadInstruction : public Instruction {
  public:
    /*! Get the address register */
    INLINE Register getAddress(void) const { return this->getSrc(0); }
    /*! Get the value (i.e. destination here) */
    INLINE Register getValue(void) const { return this->getDst(0); }
    /*! Return true if the given instruction is an instance of this class */
    static bool isClassOf(const Instruction &insn);
  };

  /*! OBlock write. Only the first lane is considered for the address
   *  (Gen extension)
   */
  class OBWriteInstruction : public Instruction {
  public:
    /*! Get the address register */
    INLINE Register getAddress(void) const { return this->getSrc(0); }
    /*! Get the value to write */
    INLINE Register getValue(void) const { return this->getSrc(1); }
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

  ///////////////////////////////////////////////////////////////////////////
  /// All emission functions
  ///////////////////////////////////////////////////////////////////////////

  /*! mov.type dst src */
  Instruction MOV(Type type, Register dst, Register src);
  /*! cos.type dst src */
  Instruction COS(Type type, Register dst, Register src);
  /*! sin.type dst src */
  Instruction SIN(Type type, Register dst, Register src);
  /*! tan.type dst src */
  Instruction TAN(Type type, Register dst, Register src);
  /*! log.type dst src */
  Instruction LOG(Type type, Register dst, Register src);
  /*! sqr.type dst src */
  Instruction SQR(Type type, Register dst, Register src);
  /*! rsq.type dst src */
  Instruction RSQ(Type type, Register dst, Register src);
  /*! pow.type dst src0 src1 */
  Instruction POW(Type type, Register dst, Register src0, Register src1);
  /*! mul.type dst src0 src1 */
  Instruction MUL(Type type, Register dst, Register src0, Register src1);
  /*! add.type dst src0 src1 */
  Instruction ADD(Type type, Register dst, Register src0, Register src1);
  /*! sub.type dst src0 src1 */
  Instruction SUB(Type type, Register dst, Register src0, Register src1);
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
  /*! mad.type dst {src0, src1, src2} (== src) */
  Instruction MAD(Type type, Register dst, Tuple src);
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
  /*! cvt.{dstType <- srcType} dst src */
  Instruction CVT(Type dstType, Type srcType, Register dst, Register src);
  /*! bra labelIndex */
  Instruction BRA(LabelIndex labelIndex);
  /*! (pred) bra labelIndex */
  Instruction BRA(LabelIndex labelIndex, Register pred);
  /*! ret */
  Instruction RET(void);
  /*! load.type.space {dst1,...,dst_valueNum} offset value */
  Instruction LOAD(Type type, Tuple dst, Register offset, AddressSpace space, uint32_t valueNum, bool dwAligned);
  /*! store.type.space offset {src1,...,src_valueNum} value */
  Instruction STORE(Type type, Tuple src, Register offset, AddressSpace space, uint32_t valueNum, bool dwAligned);
  /*! loadi.type dst value */
  Instruction LOADI(Type type, Register dst, ImmediateIndex value);
  /*! typed write TODO */
  Instruction TYPED_WRITE(void);
  /*! sample TODO */
  Instruction SAMPLE(void);
  /*! fence.space */
  Instruction FENCE(AddressSpace space);
  /*! label labelIndex */
  Instruction LABEL(LabelIndex labelIndex);
  /*! region.offset.stride dst {src1,...,src_srcNum} */
  Instruction REGION(uint32_t offset, uint32_t vstride, uint32_t width, uint32_t hstride, Register dst, Tuple src, uint32_t srcNum);
  /*! vote.predcate dst src */
  Instruction VOTE(VotePredicate predicate, Register dst, Register src);
  /*! rgather dst index {src...} (tuple contains index and sources) */
  Instruction RGATHER(Register dst, Tuple tuple, uint32_t srcNum);
  /*! obread dst address */
  Instruction OBREAD(Register dst, Register address);
  /*! obwrite address data */
  Instruction OBWRITE(Register address, Register value);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_INSTRUCTION_HPP__ */

