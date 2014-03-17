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
 * \file gen_insn_selection.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

/* This is the instruction selection code. First of all, this is a bunch of c++
 * crap. Sorry if this is not that readable. Anyway, the goal here is to take
 * GenIR code (i.e. the very regular, very RISC IR) and to produce GenISA with
 * virtual registers (i.e. regular GenIR registers).
 *
 * Overall idea:
 * =============
 *
 * There is a lot of papers and research about that but I tried to keep it
 * simple. No dynamic programming, nothing like this. Just a recursive maximal
 * munch.
 *
 * Basically, the code is executed per basic block from bottom to top. Patterns
 * of GenIR instructions are defined and each instruction is matched against the
 * best pattern i.e. the pattern that catches the largest number of
 * instructions. Once matched, a sequence of instructions is output.
 *
 * Each instruction the match depends on is then marked as "root" i.e. we
 * indicate that each of these instructions must be generated: we indeed need their
 * destinations for the next instructions (remember that we generate the code in
 * reverse order)
 *
 * Patterns:
 * =========
 *
 * There is a lot of patterns and I did not implement all of them obviously. I
 * just quickly gather the complete code to make pattern implementation kind of
 * easy. This is pretty verbose to add a pattern but it should be not too hard
 * to add new ones.
 *
 * To create and register patterns, I just abused C++ pre-main. A bunch of
 * patterns is then created and sorted per opcode (i.e. the opcode of the root
 * of the pattern): this creates a library of patterns that may be used in
 * run-time.
 *
 * Predication / Masking and CFG linearization
 * ===========================================
 *
 * The current version is based on an unfortunate choice. Basically, the problem
 * to solve is how to map unstructured branches (i.e. regular gotos) onto Gen.
 * Gen has a native support for structured branches (if/else/endif/while...) but
 * nothing really native for unstructured branches.
 *
 * The idea we implemented is simple. We stole one flag register (here f0.0) to
 * mask all the instructions (and only activate the proper SIMD lanes) and we
 * use the CFG linearization technique to properly handle the control flow. This
 * is not really good for one particular reason: Gen instructions must use the
 * *same* flag register for the predicates (used for masking) and the
 * conditional modifier (used as a destination for CMP). This leads to extra
 * complications with compare instructions and select instructions. Basically,
 * we need to insert extra MOVs.
 *
 * Also, there is some extra kludge to handle the predicates for JMPI.
 *
 * See TODO for a better idea for branching and masking
 *
 * TODO:
 * =====
 *
 * Sadly, I recreated here a new DAG class. This is just a bad idea since we
 * already have the DAG per basic block with the Function graph i.e. the
 * complete graph of uses and definitions. I think we should be able to save a
 * lot of code here if we can simply reuse the code from UD / DU chains.
 *
 * Finally, cross-block instruction selection is quite possible with this simple
 * approach. Basically, instructions from dominating blocks could be merged and
 * matched with other instructions in the dominated block. This leads to the
 * interesting approach which consists in traversing the dominator tree in post
 * order
 *
 * About masking and branching, a much better idea (that I found later unfortunately)
 * is to replace the use of the flag by uses of if/endif to enclose the basic
 * block. So, instead of using predication, we use auto-masking. The very cool
 * consequence is that we can reintegrate back the structured branches.
 * Basically, we will be able to identify branches that can be mapped to
 * structured branches and mix nicely unstructured branches (which will use
 * jpmi, if/endif to mask the blocks) and structured branches (which are pretty
 * fast)
 */

#include "backend/gen_insn_selection.hpp"
#include "backend/gen_context.hpp"
#include "ir/function.hpp"
#include "ir/liveness.hpp"
#include "ir/profile.hpp"
#include "sys/cvar.hpp"
#include "sys/vector.hpp"
#include <algorithm>

namespace gbe
{

  ///////////////////////////////////////////////////////////////////////////
  // Helper functions
  ///////////////////////////////////////////////////////////////////////////

  uint32_t getGenType(ir::Type type) {
    using namespace ir;
    switch (type) {
      case TYPE_BOOL: return GEN_TYPE_UW;
      case TYPE_S8: return GEN_TYPE_B;
      case TYPE_U8: return GEN_TYPE_UB;
      case TYPE_S16: return GEN_TYPE_W;
      case TYPE_U16: return GEN_TYPE_UW;
      case TYPE_S32: return GEN_TYPE_D;
      case TYPE_U32: return GEN_TYPE_UD;
      case TYPE_S64: return GEN_TYPE_L;
      case TYPE_U64: return GEN_TYPE_UL;
      case TYPE_FLOAT: return GEN_TYPE_F;
      case TYPE_DOUBLE: return GEN_TYPE_DF;
      default: NOT_SUPPORTED; return GEN_TYPE_F;
    }
  }

  uint32_t getGenCompare(ir::Opcode opcode) {
    using namespace ir;
    switch (opcode) {
      case OP_LE: return GEN_CONDITIONAL_LE;
      case OP_LT: return GEN_CONDITIONAL_L;
      case OP_GE: return GEN_CONDITIONAL_GE;
      case OP_GT: return GEN_CONDITIONAL_G;
      case OP_EQ: return GEN_CONDITIONAL_EQ;
      case OP_NE: return GEN_CONDITIONAL_NEQ;
      default: NOT_SUPPORTED; return 0u;
    };
  }

  ///////////////////////////////////////////////////////////////////////////
  // SelectionInstruction
  ///////////////////////////////////////////////////////////////////////////

  SelectionInstruction::SelectionInstruction(SelectionOpcode op, uint32_t dst, uint32_t src) :
    parent(NULL), opcode(op), dstNum(dst), srcNum(src)
  {}

  void SelectionInstruction::prepend(SelectionInstruction &other) {
    gbe::prepend(&other, this);
    other.parent = this->parent;
  }

  void SelectionInstruction::append(SelectionInstruction &other) {
    gbe::append(&other, this);
    other.parent = this->parent;
  }

  bool SelectionInstruction::isRead(void) const {
    return this->opcode == SEL_OP_UNTYPED_READ ||
           this->opcode == SEL_OP_READ64 ||
           this->opcode == SEL_OP_ATOMIC       ||
           this->opcode == SEL_OP_BYTE_GATHER;
  }

  bool SelectionInstruction::isWrite(void) const {
    return this->opcode == SEL_OP_UNTYPED_WRITE ||
           this->opcode == SEL_OP_WRITE64 ||
           this->opcode == SEL_OP_ATOMIC        ||
           this->opcode == SEL_OP_BYTE_SCATTER;
  }

  bool SelectionInstruction::isBranch(void) const {
    return this->opcode == SEL_OP_JMPI;
  }

  bool SelectionInstruction::isLabel(void) const {
    return this->opcode == SEL_OP_LABEL;
  }

  ///////////////////////////////////////////////////////////////////////////
  // SelectionVector
  ///////////////////////////////////////////////////////////////////////////

  SelectionVector::SelectionVector(void) :
    insn(NULL), reg(NULL), regNum(0), isSrc(0)
  {}

  ///////////////////////////////////////////////////////////////////////////
  // SelectionBlock
  ///////////////////////////////////////////////////////////////////////////

  SelectionBlock::SelectionBlock(const ir::BasicBlock *bb) : bb(bb) {}

  void SelectionBlock::append(ir::Register reg) { tmp.push_back(reg); }

  void SelectionBlock::append(SelectionInstruction *insn) {
    this->insnList.push_back(insn);
    insn->parent = this;
  }

  void SelectionBlock::prepend(SelectionInstruction *insn) {
    this->insnList.push_front(insn);
    insn->parent = this;
  }

  void SelectionBlock::append(SelectionVector *vec) {
    this->vectorList.push_back(vec);
  }

  ///////////////////////////////////////////////////////////////////////////
  // Maximal munch selection on DAG
  ///////////////////////////////////////////////////////////////////////////

  /*! All instructions in a block are organized into a DAG */
  class SelectionDAG
  {
  public:
    INLINE SelectionDAG(const ir::Instruction &insn) :
      insn(insn), mergeable(0), childNum(insn.getSrcNum()), isRoot(0) {
      GBE_ASSERT(insn.getSrcNum() < 127);
      for (uint32_t childID = 0; childID < childNum; ++childID)
        this->child[childID] = NULL;
    }
    /*! Mergeable are non-root instructions with valid sources */
    INLINE void setAsMergeable(uint32_t which) { mergeable|=(1<<which); }
    /*! Mergeable are non-root instructions with valid sources */
    INLINE bool isMergeable(uint32_t which) const { return mergeable&(1<<which); }
    /*! Children that need to be matched */
    SelectionDAG *child[ir::Instruction::MAX_SRC_NUM];
    /*! Instruction that needs to be matched */
    const ir::Instruction &insn;
    /*! When sources have been overwritten, a child insn cannot be merged */
    uint32_t mergeable:ir::Instruction::MAX_SRC_NUM;
    /*! Number of children we have in the pattern */
    uint32_t childNum:7;
    /*! A root must be generated, no matter what */
    uint32_t isRoot:1;
  };

  /*! A pattern is a tree to match. This is the general interface for them. For
   *  pattern to be matched, we need to match the complete tree i.e. this node
   *  and its child nodes
   */
  class SelectionPattern
  {
  public:
    SelectionPattern(uint32_t insnNum, uint32_t cost) :
      insnNum(insnNum), cost(cost) {}
    /*! This is an abstract class */
    virtual ~SelectionPattern(void) {}
    /*! Emit Gen code in the selection. Return false if no match */
    virtual bool emit(Selection::Opaque &sel, SelectionDAG &dag) const = 0;
    /*! All the possible opcodes for this pattern (for fast sort) */
    vector<ir::Opcode> opcodes;
    /*! Number of instruction generated */
    uint32_t insnNum;
    /*! Cost of the pattern */
    uint32_t cost;
  };

  /*! Store and sort all the patterns. This is our global library we use for the
   *  code selection
   */
  class SelectionLibrary
  {
  public:
    /*! Will register all the patterns */
    SelectionLibrary(void);
    /*! Release and destroy all the registered patterns */
    ~SelectionLibrary(void);
    /*! Insert the given pattern for all associated opcodes */
    template <typename PatternType> void insert(void);
    /*! One list of pattern per opcode */
    typedef vector<const SelectionPattern*> PatternList;
    /*! All lists of patterns properly sorted per opcode */
    PatternList patterns[ir::OP_INVALID];
    /*! All patterns to free */
    vector<const SelectionPattern*> toFree;
  };

  ///////////////////////////////////////////////////////////////////////////
  // Code selection internal implementation
  ///////////////////////////////////////////////////////////////////////////

  /*! Actual implementation of the instruction selection engine */
  class Selection::Opaque
  {
  public:
    /*! simdWidth is the default width for the instructions */
    Opaque(GenContext &ctx);
    /*! Release everything */
    virtual ~Opaque(void);
    /*! Implements the instruction selection itself */
    void select(void);
    /*! Start a backward generation (from the end of the block) */
    void startBackwardGeneration(void);
    /*! End backward code generation and output the code in the block */
    void endBackwardGeneration(void);
    /*! Implement public class */
    uint32_t getLargestBlockSize(void) const;
    /*! Implement public class */
    INLINE uint32_t getVectorNum(void) const { return this->vectorNum; }
    /*! Implement public class */
    INLINE ir::Register replaceSrc(SelectionInstruction *insn, uint32_t regID);
    /*! Implement public class */
    INLINE ir::Register replaceDst(SelectionInstruction *insn, uint32_t regID);
    /*! spill a register (insert spill/unspill instructions) */
    INLINE bool spillRegs(const SpilledRegs &spilledRegs, uint32_t registerPool);
    /*! indicate whether a register is a scalar/uniform register. */
    INLINE bool isScalarReg(const ir::Register &reg) const {
#if 0
      printf("reg %d ", reg.value());
      printf("uniform: %d ", getRegisterData(reg).isUniform());
      if (ctx.getFunction().getArg(reg) != NULL) { printf("true function arg\n"); return true; }
      if (ctx.getFunction().getPushLocation(reg) != NULL) { printf("true push location.\n"); return true; }
      if (reg == ir::ocl::groupid0  ||
          reg == ir::ocl::groupid1  ||
          reg == ir::ocl::groupid2  ||
          reg == ir::ocl::barrierid ||
          reg == ir::ocl::threadn   ||
          reg == ir::ocl::numgroup0 ||
          reg == ir::ocl::numgroup1 ||
          reg == ir::ocl::numgroup2 ||
          reg == ir::ocl::lsize0    ||
          reg == ir::ocl::lsize1    ||
          reg == ir::ocl::lsize2    ||
          reg == ir::ocl::gsize0    ||
          reg == ir::ocl::gsize1    ||
          reg == ir::ocl::gsize2    ||
          reg == ir::ocl::goffset0  ||
          reg == ir::ocl::goffset1  ||
          reg == ir::ocl::goffset2  ||
          reg == ir::ocl::workdim   ||
          reg == ir::ocl::emask     ||
          reg == ir::ocl::notemask  ||
          reg == ir::ocl::barriermask
        ) {
        printf("special reg.\n");
        return true;
      }
      return false;
#endif
      const ir::RegisterData &regData = getRegisterData(reg);
      return regData.isUniform();
    }
    /*! Implement public class */
    INLINE uint32_t getRegNum(void) const { return file.regNum(); }
    /*! Implements public interface */
    bool isScalarOrBool(ir::Register reg) const;
    /*! Implements public interface */
    INLINE ir::RegisterData getRegisterData(ir::Register reg) const {
      return file.get(reg);
    }
    /*! Implement public class */
    INLINE ir::RegisterFamily getRegisterFamily(ir::Register reg) const {
      return file.get(reg).family;
    }
    /*! Implement public class */
    SelectionInstruction *create(SelectionOpcode, uint32_t dstNum, uint32_t srcNum);
    /*! Return the selection register from the GenIR one */
    GenRegister selReg(ir::Register, ir::Type type = ir::TYPE_FLOAT) const;
    /*! Compute the nth register part when using SIMD8 with Qn (n in 2,3,4) */
    GenRegister selRegQn(ir::Register, uint32_t quarter, ir::Type type = ir::TYPE_FLOAT) const;
    /*! Size of the stack (should be large enough) */
    enum { MAX_STATE_NUM = 16 };
    /*! Push the current instruction state */
    INLINE void push(void) {
      assert(stateNum < MAX_STATE_NUM);
      stack[stateNum++] = curr;
    }
    /*! Pop the latest pushed state */
    INLINE void pop(void) {
      assert(stateNum > 0);
      curr = stack[--stateNum];
    }
    /*! Create a new register in the register file and append it in the
     *  temporary list of the current block
     */
    INLINE ir::Register reg(ir::RegisterFamily family, bool scalar = false) {
      GBE_ASSERT(block != NULL);
      const ir::Register reg = file.append(family, scalar);
      block->append(reg);
      return reg;
    }
    /*! Append a block at the block stream tail. It becomes the current block */
    void appendBlock(const ir::BasicBlock &bb);
    /*! Append an instruction in the current block */
    SelectionInstruction *appendInsn(SelectionOpcode, uint32_t dstNum, uint32_t srcNum);
    /*! Append a new vector of registers in the current block */
    SelectionVector *appendVector(void);
    /*! Build a DAG for the basic block (return number of instructions) */
    uint32_t buildBasicBlockDAG(const ir::BasicBlock &bb);
    /*! Perform the selection on the basic block */
    void matchBasicBlock(uint32_t insnNum);
    /*! A root instruction needs to be generated */
    bool isRoot(const ir::Instruction &insn) const;

    /*! To handle selection block allocation */
    DECL_POOL(SelectionBlock, blockPool);
    /*! To handle selection instruction allocation */
    LinearAllocator insnAllocator;
    /*! To handle selection vector allocation */
    DECL_POOL(SelectionVector, vecPool);
    /*! Per register information used with top-down block sweeping */
    vector<SelectionDAG*> regDAG;
    /*! Store one DAG per instruction */
    vector<SelectionDAG*> insnDAG;
    /*! Owns this structure */
    GenContext &ctx;
    /*! Tail of the code fragment for backward code generation */
    intrusive_list<SelectionInstruction> bwdList;
    /*! List of emitted blocks */
    intrusive_list<SelectionBlock> blockList;
    /*! Currently processed block */
    SelectionBlock *block;
    /*! Current instruction state to use */
    GenInstructionState curr;
    /*! We append new registers so we duplicate the function register file */
    ir::RegisterFile file;
    /*! State used to encode the instructions */
    GenInstructionState stack[MAX_STATE_NUM];
    /*! Maximum number of instructions in the basic blocks */
    uint32_t maxInsnNum;
    /*! Speed up instruction dag allocation */
    DECL_POOL(SelectionDAG, dagPool);
    /*! Total number of registers in the function we encode */
    uint32_t regNum;
    /*! Number of states currently pushed */
    uint32_t stateNum;
    /*! Number of vector allocated */
    uint32_t vectorNum;
    /*! If true, generate code backward */
    bool bwdCodeGeneration;
    /*! To make function prototypes more readable */
    typedef const GenRegister &Reg;

#define ALU1(OP) \
  INLINE void OP(Reg dst, Reg src) { ALU1(SEL_OP_##OP, dst, src); }
#define ALU1WithTemp(OP) \
  INLINE void OP(Reg dst, Reg src, Reg temp) { ALU1WithTemp(SEL_OP_##OP, dst, src, temp); }
#define ALU2(OP) \
  INLINE void OP(Reg dst, Reg src0, Reg src1) { ALU2(SEL_OP_##OP, dst, src0, src1); }
#define ALU2WithTemp(OP) \
  INLINE void OP(Reg dst, Reg src0, Reg src1, Reg temp) { ALU2WithTemp(SEL_OP_##OP, dst, src0, src1, temp); }
#define ALU3(OP) \
  INLINE void OP(Reg dst, Reg src0, Reg src1, Reg src2) { ALU3(SEL_OP_##OP, dst, src0, src1, src2); }
#define I64Shift(OP) \
  INLINE void OP(Reg dst, Reg src0, Reg src1, GenRegister tmp[7]) { I64Shift(SEL_OP_##OP, dst, src0, src1, tmp); }
    ALU1(MOV)
    ALU1WithTemp(MOV_DF)
    ALU1WithTemp(LOAD_DF_IMM)
    ALU1(LOAD_INT64_IMM)
    ALU1(RNDZ)
    ALU1(RNDE)
    ALU1(F16TO32)
    ALU1(F32TO16)
    ALU2(SEL)
    ALU2(SEL_INT64)
    ALU1(NOT)
    ALU2(AND)
    ALU2(OR)
    ALU2(XOR)
    ALU2(I64AND)
    ALU2(I64OR)
    ALU2(I64XOR)
    ALU2(SHR)
    ALU2(SHL)
    ALU2(RSR)
    ALU2(RSL)
    ALU2(ASR)
    ALU2(ADD)
    ALU2WithTemp(I64ADD)
    ALU2WithTemp(I64SUB)
    ALU2(MUL)
    ALU1(FRC)
    ALU1(RNDD)
    ALU1(RNDU)
    ALU2(MACH)
    ALU1(LZD)
    ALU3(MAD)
    ALU2WithTemp(MUL_HI)
    ALU1(FBH)
    ALU1(FBL)
    ALU2WithTemp(HADD)
    ALU2WithTemp(RHADD)
    ALU2(UPSAMPLE_SHORT)
    ALU2(UPSAMPLE_INT)
    ALU2(UPSAMPLE_LONG)
    ALU1WithTemp(CONVI_TO_I64)
    ALU1WithTemp(CONVF_TO_I64)
    ALU1(CONVI64_TO_I)
    I64Shift(I64SHL)
    I64Shift(I64SHR)
    I64Shift(I64ASR)
#undef ALU1
#undef ALU1WithTemp
#undef ALU2
#undef ALU2WithTemp
#undef ALU3
#undef I64Shift
    /*! Convert 64-bit integer to 32-bit float */
    void CONVI64_TO_F(Reg dst, Reg src, GenRegister tmp[7]);
    /*! Convert 64-bit integer to 32-bit float */
    void CONVF_TO_I64(Reg dst, Reg src, GenRegister tmp[3]);
    /*! Saturated 64bit x*y + z */
    void I64MADSAT(Reg dst, Reg src0, Reg src1, Reg src2, GenRegister tmp[10]);
    /*! High 64bit of x*y */
    void I64_MUL_HI(Reg dst, Reg src0, Reg src1, GenRegister tmp[10]);
    /*! (x+y)>>1 without mod. overflow */
    void I64HADD(Reg dst, Reg src0, Reg src1, GenRegister tmp[4]);
    /*! (x+y+1)>>1 without mod. overflow */
    void I64RHADD(Reg dst, Reg src0, Reg src1, GenRegister tmp[4]);
    /*! Shift a 64-bit integer */
    void I64Shift(SelectionOpcode opcode, Reg dst, Reg src0, Reg src1, GenRegister tmp[7]);
    /*! Compare 64-bit integer */
    void I64CMP(uint32_t conditional, Reg src0, Reg src1, GenRegister tmp[3], Reg dst = GenRegister::null());
    /*! Saturated addition of 64-bit integer */
    void I64SATADD(Reg dst, Reg src0, Reg src1, GenRegister tmp[6]);
    /*! Saturated subtraction of 64-bit integer */
    void I64SATSUB(Reg dst, Reg src0, Reg src1, GenRegister tmp[6]);
    /*! Encode a barrier instruction */
    void BARRIER(GenRegister src, GenRegister fence, uint32_t barrierType);
    /*! Encode a barrier instruction */
    void FENCE(GenRegister dst);
    /*! Encode a label instruction */
    void LABEL(ir::LabelIndex label);
    /*! Jump indexed instruction */
    void JMPI(Reg src, ir::LabelIndex target);
    /*! Compare instructions */
    void CMP(uint32_t conditional, Reg src0, Reg src1, Reg dst = GenRegister::null());
    /*! Select instruction with embedded comparison */
    void SEL_CMP(uint32_t conditional, Reg dst, Reg src0, Reg src1);
    /* Constant buffer move instruction */
    void INDIRECT_MOVE(Reg dst, Reg src);
    /*! EOT is used to finish GPGPU threads */
    void EOT(void);
    /*! No-op */
    void NOP(void);
    /*! Wait instruction (used for the barrier) */
    void WAIT(void);
    /*! Atomic instruction */
    void ATOMIC(Reg dst, uint32_t function, uint32_t srcNum, Reg src0, Reg src1, Reg src2, uint32_t bti);
    /*! Read 64 bits float/int array */
    void READ64(Reg addr, Reg tempAddr, const GenRegister *dst, uint32_t elemNum, uint32_t valueNum, uint32_t bti);
    /*! Write 64 bits float/int array */
    void WRITE64(Reg addr, const GenRegister *src, uint32_t srcNum, const GenRegister *dst, uint32_t dstNum, uint32_t bti);
    /*! Untyped read (up to 4 elements) */
    void UNTYPED_READ(Reg addr, const GenRegister *dst, uint32_t elemNum, uint32_t bti);
    /*! Untyped write (up to 4 elements) */
    void UNTYPED_WRITE(Reg addr, const GenRegister *src, uint32_t elemNum, uint32_t bti);
    /*! Byte gather (for unaligned bytes, shorts and ints) */
    void BYTE_GATHER(Reg dst, Reg addr, uint32_t elemSize, uint32_t bti);
    /*! Byte scatter (for unaligned bytes, shorts and ints) */
    void BYTE_SCATTER(Reg addr, Reg src, uint32_t elemSize, uint32_t bti);
    /*! DWord scatter (for constant cache read) */
    void DWORD_GATHER(Reg dst, Reg addr, uint32_t bti);
    /*! Unpack the uint to char4 */
    void UNPACK_BYTE(const GenRegister *dst, const GenRegister src, uint32_t elemNum);
    /*! pack the char4 to uint */
    void PACK_BYTE(const GenRegister dst, const GenRegister *src, uint32_t elemNum);
    /*! Extended math function (2 arguments) */
    void MATH(Reg dst, uint32_t function, Reg src0, Reg src1);
    /*! Extended math function (1 argument) */
    void MATH(Reg dst, uint32_t function, Reg src);
    /*! Encode unary instructions */
    void ALU1(SelectionOpcode opcode, Reg dst, Reg src);
    /*! Encode unary with temp reg instructions */
    void ALU1WithTemp(SelectionOpcode opcode, Reg dst, Reg src0, Reg temp);
    /*! Encode binary instructions */
    void ALU2(SelectionOpcode opcode, Reg dst, Reg src0, Reg src1);
    /*! Encode binary with temp reg instructions */
    void ALU2WithTemp(SelectionOpcode opcode, Reg dst, Reg src0, Reg src1, Reg temp);
    /*! Encode ternary instructions */
    void ALU3(SelectionOpcode opcode, Reg dst, Reg src0, Reg src1, Reg src2);
    /*! Encode sample instructions */
    void SAMPLE(GenRegister *dst, uint32_t dstNum, GenRegister *msgPayloads, uint32_t msgNum, uint32_t bti, uint32_t sampler, bool is3D);
    /*! Encode typed write instructions */
    void TYPED_WRITE(GenRegister *msgs, uint32_t msgNum, uint32_t bti, bool is3D);
    /*! Get image information */
    void GET_IMAGE_INFO(uint32_t type, GenRegister *dst, uint32_t dst_num, uint32_t bti);
    /*! Multiply 64-bit integers */
    void I64MUL(Reg dst, Reg src0, Reg src1, GenRegister tmp[6]);
    /*! 64-bit integer division */
    void I64DIV(Reg dst, Reg src0, Reg src1, GenRegister tmp[14]);
    /*! 64-bit integer remainder of division */
    void I64REM(Reg dst, Reg src0, Reg src1, GenRegister tmp[14]);
    /*! Use custom allocators */
    GBE_CLASS(Opaque);
    friend class SelectionBlock;
    friend class SelectionInstruction;
  };

  ///////////////////////////////////////////////////////////////////////////
  // Helper function
  ///////////////////////////////////////////////////////////////////////////

  /*! Directly mark all sources as root (when no match is found) */
  static void markAllChildren(SelectionDAG &dag) {
    // Do not merge anything, so all sources become roots
    for (uint32_t childID = 0; childID < dag.childNum; ++childID)
      if (dag.child[childID])
        dag.child[childID]->isRoot = 1;
  }

  /*! Helper function to figure if two sources are the same */
  static bool sourceMatch(SelectionDAG *src0DAG, uint32_t src0ID,
                          SelectionDAG *src1DAG, uint32_t src1ID)
  {
    GBE_ASSERT(src0DAG && src1DAG);
    // Ensure they are the same physical registers
    const ir::Register src0 = src0DAG->insn.getSrc(src0ID);
    const ir::Register src1 = src1DAG->insn.getSrc(src1ID);
    if (src0 != src1)
      return false;
    // Ensure they contain the same values
    return src0DAG->child[src0ID] == src1DAG->child[src1ID];
  }


  Selection::Opaque::Opaque(GenContext &ctx) :
    ctx(ctx), block(NULL),
    curr(ctx.getSimdWidth()), file(ctx.getFunction().getRegisterFile()),
    maxInsnNum(ctx.getFunction().getLargestBlockSize()), dagPool(maxInsnNum),
    stateNum(0), vectorNum(0), bwdCodeGeneration(false)
  {
    const ir::Function &fn = ctx.getFunction();
    this->regNum = fn.regNum();
    this->regDAG.resize(regNum);
    this->insnDAG.resize(maxInsnNum);
  }

  Selection::Opaque::~Opaque(void) {
    for (auto it = blockList.begin(); it != blockList.end();) {
      SelectionBlock &block = *it;
      ++it;
      this->deleteSelectionBlock(&block);
    }
  }

  SelectionInstruction*
  Selection::Opaque::create(SelectionOpcode opcode, uint32_t dstNum, uint32_t srcNum)
  {
    const size_t regSize =  (dstNum+srcNum)*sizeof(GenRegister);
    const size_t size = sizeof(SelectionInstruction) + regSize;
    void *ptr = insnAllocator.allocate(size);
    return new (ptr) SelectionInstruction(opcode, dstNum, srcNum);
  }

  void Selection::Opaque::startBackwardGeneration(void) {
    this->bwdCodeGeneration = true;
  }

  void Selection::Opaque::endBackwardGeneration(void) {
    for (auto it = bwdList.rbegin(); it != bwdList.rend();) {
      SelectionInstruction &insn = *it;
      auto toRemoveIt = it--;
      bwdList.erase(toRemoveIt);
      this->block->prepend(&insn);
    }

    this->bwdCodeGeneration = false;
  }

  uint32_t Selection::Opaque::getLargestBlockSize(void) const {
    size_t maxInsnNum = 0;
    for (const auto &bb : blockList)
      maxInsnNum = std::max(maxInsnNum, bb.insnList.size());
    return uint32_t(maxInsnNum);
  }

  void Selection::Opaque::appendBlock(const ir::BasicBlock &bb) {
    this->block = this->newSelectionBlock(&bb);
    this->blockList.push_back(this->block);
  }

  SelectionInstruction *Selection::Opaque::appendInsn(SelectionOpcode opcode,
                                                      uint32_t dstNum,
                                                      uint32_t srcNum)
  {
    GBE_ASSERT(dstNum <= SelectionInstruction::MAX_DST_NUM && srcNum <= SelectionInstruction::MAX_SRC_NUM);
    GBE_ASSERT(this->block != NULL);
    SelectionInstruction *insn = this->create(opcode, dstNum, srcNum);
    if (this->bwdCodeGeneration)
      this->bwdList.push_back(insn);
    else
      this->block->append(insn);
    insn->state = this->curr;
    return insn;
  }

  SelectionVector *Selection::Opaque::appendVector(void) {
    GBE_ASSERT(this->block != NULL);
    SelectionVector *vector = this->newSelectionVector();

    if (this->bwdCodeGeneration)
      vector->insn = this->bwdList.back();
    else
      vector->insn = this->block->insnList.back();
    this->block->append(vector);
    this->vectorNum++;
    return vector;
  }

  bool Selection::Opaque::spillRegs(const SpilledRegs &spilledRegs,
                                    uint32_t registerPool) {
    GBE_ASSERT(registerPool != 0);

    for (auto &block : blockList)
      for (auto &insn : block.insnList) {
        // spill / unspill insn should be skipped when do spilling
        if(insn.opcode == SEL_OP_SPILL_REG
           || insn.opcode == SEL_OP_UNSPILL_REG)
          continue;

        const uint32_t srcNum = insn.srcNum, dstNum = insn.dstNum;
        struct RegSlot {
          RegSlot(ir::Register _reg, uint8_t _srcID,
                   uint8_t _poolOffset, bool _isTmp, uint32_t _addr)
                 : reg(_reg), srcID(_srcID), poolOffset(_poolOffset), isTmpReg(_isTmp), addr(_addr)
          {};
          ir::Register reg;
          union {
            uint8_t srcID;
            uint8_t dstID;
          };
          uint8_t poolOffset;
          bool isTmpReg;
          int32_t addr;
        };
        uint8_t poolOffset = 1; // keep one for scratch message header
        vector <struct RegSlot> regSet;
        for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
          const GenRegister selReg = insn.src(srcID);
          const ir::Register reg = selReg.reg();
          auto it = spilledRegs.find(reg);
          if(it != spilledRegs.end()
             && selReg.file == GEN_GENERAL_REGISTER_FILE
             && selReg.physical == 0) {
            ir::RegisterFamily family = getRegisterFamily(reg);
            if(family == ir::FAMILY_QWORD && poolOffset == 1) {
              poolOffset += 1; // qword register fill could not share the scratch read message payload register
            }
            struct RegSlot regSlot(reg, srcID, poolOffset,
                                   it->second.isTmpReg,
                                   it->second.addr);
            if(family == ir::FAMILY_QWORD) {
              poolOffset += 2;
            } else {
              poolOffset += 1;
            }
            regSet.push_back(regSlot);
          }
        }

        if (poolOffset > RESERVED_REG_NUM_FOR_SPILL) {
          std::cerr << "Instruction (#" << (uint32_t)insn.opcode << ") src too large pooloffset " << (uint32_t)poolOffset << std::endl;
          return false;
        }
        while(!regSet.empty()) {
          struct RegSlot regSlot = regSet.back();
          regSet.pop_back();
          const GenRegister selReg = insn.src(regSlot.srcID);
          if (!regSlot.isTmpReg) {
          /* For temporary registers, we don't need to unspill. */
            SelectionInstruction *unspill = this->create(SEL_OP_UNSPILL_REG, 1, 0);
            unspill->state  = GenInstructionState(ctx.getSimdWidth());
            unspill->dst(0) = GenRegister(GEN_GENERAL_REGISTER_FILE,
                                          registerPool + regSlot.poolOffset, 0,
                                          selReg.type, selReg.vstride,
                                          selReg.width, selReg.hstride);
            unspill->extra.scratchOffset = regSlot.addr;
            unspill->extra.scratchMsgHeader = registerPool;
            insn.prepend(*unspill);
          }

          GenRegister src = insn.src(regSlot.srcID);
          // change nr/subnr, keep other register settings
          src.nr = registerPool + regSlot.poolOffset; src.subnr = 0; src.physical = 1;
          insn.src(regSlot.srcID) = src;
        };

        /*
          To save one register, registerPool + 1 was used by both
          the src0 as source and other operands as payload. To avoid
          side effect, we use a stack model to push all operands
          register, and spill the 0th dest at last. As all the spill
          will be append to the current instruction. Then the last spill
          instruction will be the first instruction after current
          instruction. Thus the registerPool + 1 still contain valid
          data.
         */
        for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
          const GenRegister selReg = insn.dst(dstID);
          const ir::Register reg = selReg.reg();
          auto it = spilledRegs.find(reg);
          if(it != spilledRegs.end()
             && selReg.file == GEN_GENERAL_REGISTER_FILE
             && selReg.physical == 0) {
            ir::RegisterFamily family = getRegisterFamily(reg);
            if(family == ir::FAMILY_QWORD && poolOffset == 1) {
              poolOffset += 1; // qword register spill could not share the scratch write message payload register
            }
            struct RegSlot regSlot(reg, dstID, poolOffset,
                                   it->second.isTmpReg,
                                   it->second.addr);
            if(family == ir::FAMILY_QWORD) poolOffset +=2;
            else poolOffset += 1;
            regSet.push_back(regSlot);
          }
        }

        if (poolOffset > RESERVED_REG_NUM_FOR_SPILL){
          std::cerr << "Instruction (#" << (uint32_t)insn.opcode << ") dst too large pooloffset " << (uint32_t)poolOffset << std::endl;
          return false;
        }
        while(!regSet.empty()) {
          struct RegSlot regSlot = regSet.back();
          regSet.pop_back();
          const GenRegister selReg = insn.dst(regSlot.dstID);
          if(!regSlot.isTmpReg) {
            /* For temporary registers, we don't need to unspill. */
            SelectionInstruction *spill = this->create(SEL_OP_SPILL_REG, 0, 1);
            spill->state  = GenInstructionState(ctx.getSimdWidth());
            spill->src(0) = GenRegister(GEN_GENERAL_REGISTER_FILE,
                                        registerPool + regSlot.poolOffset, 0,
                                        selReg.type, selReg.vstride,
                                        selReg.width, selReg.hstride);
            spill->extra.scratchOffset = regSlot.addr;
            spill->extra.scratchMsgHeader = registerPool;
            insn.append(*spill);
          }

          GenRegister dst = insn.dst(regSlot.dstID);
          // change nr/subnr, keep other register settings
          dst.physical =1; dst.nr = registerPool + regSlot.poolOffset; dst.subnr = 0;
          insn.dst(regSlot.dstID)= dst;
        }
      }
    return true;
  }

  ir::Register Selection::Opaque::replaceSrc(SelectionInstruction *insn, uint32_t regID) {
    SelectionBlock *block = insn->parent;
    const uint32_t simdWidth = ctx.getSimdWidth();
    ir::Register tmp;

    // This will append the temporary register in the instruction block
    this->block = block;
    tmp = this->reg(ir::FAMILY_DWORD);

    // Generate the MOV instruction and replace the register in the instruction
    SelectionInstruction *mov = this->create(SEL_OP_MOV, 1, 1);
    mov->src(0) = GenRegister::retype(insn->src(regID), GEN_TYPE_F);
    mov->state = GenInstructionState(simdWidth);
    insn->src(regID) = mov->dst(0) = GenRegister::fxgrf(simdWidth, tmp);
    insn->prepend(*mov);

    return tmp;
  }

  ir::Register Selection::Opaque::replaceDst(SelectionInstruction *insn, uint32_t regID) {
    SelectionBlock *block = insn->parent;
    uint32_t simdWidth = ctx.getSimdWidth();
    ir::Register tmp;
    ir::RegisterFamily f = file.get(insn->dst(regID).reg()).family;
    int genType = f == ir::FAMILY_QWORD ? GEN_TYPE_DF : GEN_TYPE_F;
    GenRegister gr;

    // This will append the temporary register in the instruction block
    this->block = block;
    tmp = this->reg(f);

    // Generate the MOV instruction and replace the register in the instruction
    SelectionInstruction *mov = this->create(SEL_OP_MOV, 1, 1);
    mov->dst(0) = GenRegister::retype(insn->dst(regID), genType);
    mov->state = GenInstructionState(simdWidth);
    gr = f == ir::FAMILY_QWORD ? GenRegister::dfxgrf(simdWidth, tmp) : GenRegister::fxgrf(simdWidth, tmp);
    insn->dst(regID) = mov->src(0) = gr;
    insn->append(*mov);
    return tmp;
  }

  bool Selection::Opaque::isScalarOrBool(ir::Register reg) const {
    if (isScalarReg(reg))
      return true;
    return false;
  }

#define SEL_REG(SIMD16, SIMD8, SIMD1) \
  if (ctx.sel->isScalarOrBool(reg) == true) \
    return GenRegister::retype(GenRegister::SIMD1(reg), genType); \
  else if (simdWidth == 8) \
    return GenRegister::retype(GenRegister::SIMD8(reg), genType); \
  else { \
    GBE_ASSERT (simdWidth == 16); \
    return GenRegister::retype(GenRegister::SIMD16(reg), genType); \
  }

  GenRegister Selection::Opaque::selReg(ir::Register reg, ir::Type type) const {
    using namespace ir;
    const uint32_t genType = getGenType(type);
    const uint32_t simdWidth = ctx.getSimdWidth();
    const RegisterData data = file.get(reg);
    const RegisterFamily family = data.family;
    switch (family) {
      case FAMILY_BOOL: SEL_REG(uw16grf, uw8grf, uw1grf); break;
      case FAMILY_WORD: SEL_REG(uw16grf, uw8grf, uw1grf); break;
      case FAMILY_BYTE: SEL_REG(ub16grf, ub8grf, ub1grf); break;
      case FAMILY_DWORD: SEL_REG(f16grf, f8grf, f1grf); break;
      case FAMILY_QWORD: SEL_REG(df16grf, df8grf, df1grf); break;
      default: NOT_SUPPORTED;
    }
    GBE_ASSERT(false);
    return GenRegister();
  }

#undef SEL_REG

  GenRegister Selection::Opaque::selRegQn(ir::Register reg, uint32_t q, ir::Type type) const {
    GenRegister sreg = this->selReg(reg, type);
    sreg.quarter = q;
    return sreg;
  }

  /*! Syntactic sugar for method declaration */
  typedef const GenRegister &Reg;

  void Selection::Opaque::LABEL(ir::LabelIndex index) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_LABEL, 0, 0);
    insn->index = uint16_t(index);
  }

  void Selection::Opaque::BARRIER(GenRegister src, GenRegister fence, uint32_t barrierType) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_BARRIER, 1, 1);
    insn->src(0) = src;
    insn->dst(0) = fence;
    insn->extra.barrierType = barrierType;
  }

  void Selection::Opaque::FENCE(GenRegister dst) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_FENCE, 1, 0);
    insn->dst(0) = dst;
  }

  void Selection::Opaque::JMPI(Reg src, ir::LabelIndex index) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_JMPI, 0, 1);
    insn->src(0) = src;
    insn->index = uint16_t(index);
  }

  void Selection::Opaque::CMP(uint32_t conditional, Reg src0, Reg src1, Reg dst) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_CMP, 1, 2);
    insn->src(0) = src0;
    insn->src(1) = src1;
    insn->dst(0) = dst;
    insn->extra.function = conditional;
  }

  void Selection::Opaque::SEL_CMP(uint32_t conditional, Reg dst, Reg src0, Reg src1) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_SEL_CMP, 1, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    insn->extra.function = conditional;
  }
  void Selection::Opaque::INDIRECT_MOVE(Reg dst, Reg src) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_INDIRECT_MOVE, 1, 1);
    insn->dst(0) = dst;
    insn->src(0) = src;
  }

  void Selection::Opaque::ATOMIC(Reg dst, uint32_t function,
                                     uint32_t srcNum, Reg src0,
                                     Reg src1, Reg src2, uint32_t bti) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_ATOMIC, 1, srcNum);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    if(srcNum > 1) insn->src(1) = src1;
    if(srcNum > 2) insn->src(2) = src2;
    insn->extra.function = function;
    insn->extra.elem     = bti;
    SelectionVector *vector = this->appendVector();

    vector->regNum = srcNum;
    vector->reg = &insn->src(0);
    vector->isSrc = 1;
  }

  void Selection::Opaque::EOT(void) { this->appendInsn(SEL_OP_EOT, 0, 0); }
  void Selection::Opaque::NOP(void) { this->appendInsn(SEL_OP_NOP, 0, 0); }
  void Selection::Opaque::WAIT(void) { this->appendInsn(SEL_OP_WAIT, 0, 0); }

  /* elemNum contains all the temporary register and the
     real destination registers.*/
  void Selection::Opaque::READ64(Reg addr,
                                 Reg tempAddr,
                                 const GenRegister *dst,
                                 uint32_t elemNum,
                                 uint32_t valueNum,
                                 uint32_t bti)
  {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_READ64, elemNum + 1, 1);
    SelectionVector *srcVector = this->appendVector();
    SelectionVector *dstVector = this->appendVector();

    // Regular instruction to encode
    for (uint32_t elemID = 0; elemID < elemNum; ++elemID)
      insn->dst(elemID) = dst[elemID];
    /* temporary addr register is to be modified, set it to dst registers.*/
    insn->dst(elemNum) = tempAddr;
    insn->src(0) = addr;
    insn->extra.function = bti;
    insn->extra.elem = valueNum;

    // Only the temporary registers need contiguous allocation
    dstVector->regNum = elemNum - valueNum;
    dstVector->isSrc = 0;
    dstVector->reg = &insn->dst(0);

    // Source cannot be scalar (yet)
    srcVector->regNum = 1;
    srcVector->isSrc = 1;
    srcVector->reg = &insn->src(0);
  }

  void Selection::Opaque::UNTYPED_READ(Reg addr,
                                       const GenRegister *dst,
                                       uint32_t elemNum,
                                       uint32_t bti)
  {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_UNTYPED_READ, elemNum, 1);
    SelectionVector *srcVector = this->appendVector();
    SelectionVector *dstVector = this->appendVector();

    // Regular instruction to encode
    for (uint32_t elemID = 0; elemID < elemNum; ++elemID)
      insn->dst(elemID) = dst[elemID];
    insn->src(0) = addr;
    insn->extra.function = bti;
    insn->extra.elem = elemNum;

    // Sends require contiguous allocation
    dstVector->regNum = elemNum;
    dstVector->isSrc = 0;
    dstVector->reg = &insn->dst(0);
    // Source cannot be scalar (yet)
    srcVector->regNum = 1;
    srcVector->isSrc = 1;
    srcVector->reg = &insn->src(0);
  }

  /* elemNum contains all the temporary register and the
     real data registers.*/
  void Selection::Opaque::WRITE64(Reg addr,
                                  const GenRegister *src,
                                  uint32_t srcNum,
                                  const GenRegister *dst,
                                  uint32_t dstNum,
                                  uint32_t bti)
  {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_WRITE64, dstNum, srcNum + 1);
    SelectionVector *vector = this->appendVector();

    // Regular instruction to encode
    insn->src(0) = addr;
    for (uint32_t elemID = 0; elemID < srcNum; ++elemID)
      insn->src(elemID + 1) = src[elemID];
    for (uint32_t elemID = 0; elemID < dstNum; ++elemID)
      insn->dst(elemID) = dst[elemID];
    insn->extra.function = bti;
    insn->extra.elem = srcNum;

    // Only the addr + temporary registers need to be contiguous.
    vector->regNum = dstNum;
    vector->reg = &insn->dst(0);
    vector->isSrc = 1;
  }

  void Selection::Opaque::UNTYPED_WRITE(Reg addr,
                                        const GenRegister *src,
                                        uint32_t elemNum,
                                        uint32_t bti)
  {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_UNTYPED_WRITE, 0, elemNum+1);
    SelectionVector *vector = this->appendVector();

    // Regular instruction to encode
    insn->src(0) = addr;
    for (uint32_t elemID = 0; elemID < elemNum; ++elemID)
      insn->src(elemID+1) = src[elemID];
    insn->extra.function = bti;
    insn->extra.elem = elemNum;

    // Sends require contiguous allocation for the sources
    vector->regNum = elemNum+1;
    vector->reg = &insn->src(0);
    vector->isSrc = 1;
  }

  void Selection::Opaque::BYTE_GATHER(Reg dst, Reg addr, uint32_t elemSize, uint32_t bti) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_BYTE_GATHER, 1, 1);
    SelectionVector *srcVector = this->appendVector();
    SelectionVector *dstVector = this->appendVector();

    // Instruction to encode
    insn->src(0) = addr;
    insn->dst(0) = dst;
    insn->extra.function = bti;
    insn->extra.elem = elemSize;

    // byte gather requires vector in the sense that scalar are not allowed
    // (yet)
    dstVector->regNum = 1;
    dstVector->isSrc = 0;
    dstVector->reg = &insn->dst(0);
    srcVector->regNum = 1;
    srcVector->isSrc = 1;
    srcVector->reg = &insn->src(0);
  }

  void Selection::Opaque::BYTE_SCATTER(Reg addr, Reg src, uint32_t elemSize, uint32_t bti) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_BYTE_SCATTER, 0, 2);
    SelectionVector *vector = this->appendVector();

    // Instruction to encode
    insn->src(0) = addr;
    insn->src(1) = src;
    insn->extra.function = bti;
    insn->extra.elem = elemSize;

    // value and address are contiguous in the send
    vector->regNum = 2;
    vector->isSrc = 1;
    vector->reg = &insn->src(0);
  }

  void Selection::Opaque::DWORD_GATHER(Reg dst, Reg addr, uint32_t bti) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_DWORD_GATHER, 1, 1);

    insn->src(0) = addr;
    insn->dst(0) = dst;
    insn->extra.function = bti;
  }
  void Selection::Opaque::UNPACK_BYTE(const GenRegister *dst, const GenRegister src, uint32_t elemNum) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_UNPACK_BYTE, elemNum, 1);
    insn->src(0) = src;
    for(uint32_t i = 0; i < elemNum; i++)
      insn->dst(i) = dst[i];
  }
  void Selection::Opaque::PACK_BYTE(const GenRegister dst, const GenRegister *src, uint32_t elemNum) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_PACK_BYTE, 1, elemNum);
    for(uint32_t i = 0; i < elemNum; i++)
      insn->src(i) = src[i];
    insn->dst(0) = dst;
  }

  void Selection::Opaque::MATH(Reg dst, uint32_t function, Reg src0, Reg src1) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_MATH, 1, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    insn->extra.function = function;
  }

  void Selection::Opaque::MATH(Reg dst, uint32_t function, Reg src) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_MATH, 1, 1);
    insn->dst(0) = dst;
    insn->src(0) = src;
    insn->extra.function = function;
  }

  void Selection::Opaque::I64MUL(Reg dst, Reg src0, Reg src1, GenRegister tmp[6]) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_I64MUL, 7, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    for(int i = 0; i < 6; i++)
      insn->dst(i + 1) = tmp[i];
  }

  void Selection::Opaque::I64DIV(Reg dst, Reg src0, Reg src1, GenRegister tmp[14]) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_I64DIV, 15, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    for(int i = 0; i < 14; i++)
      insn->dst(i + 1) = tmp[i];
  }

  void Selection::Opaque::I64REM(Reg dst, Reg src0, Reg src1, GenRegister tmp[14]) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_I64REM, 15, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    for(int i = 0; i < 14; i++)
      insn->dst(i + 1) = tmp[i];
  }

  void Selection::Opaque::ALU1(SelectionOpcode opcode, Reg dst, Reg src) {
    SelectionInstruction *insn = this->appendInsn(opcode, 1, 1);
    insn->dst(0) = dst;
    insn->src(0) = src;
  }

  void Selection::Opaque::ALU1WithTemp(SelectionOpcode opcode, Reg dst, Reg src, Reg temp) {
    SelectionInstruction *insn = this->appendInsn(opcode, 2, 1);
    insn->dst(0) = dst;
    insn->src(0) = src;
    insn->dst(1) = temp;
  }

  void Selection::Opaque::ALU2(SelectionOpcode opcode, Reg dst, Reg src0, Reg src1) {
    SelectionInstruction *insn = this->appendInsn(opcode, 1, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
  }

  void Selection::Opaque::ALU2WithTemp(SelectionOpcode opcode, Reg dst, Reg src0, Reg src1, Reg temp) {
    SelectionInstruction *insn = this->appendInsn(opcode, 2, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    insn->dst(1) = temp;
  }

  void Selection::Opaque::ALU3(SelectionOpcode opcode, Reg dst, Reg src0, Reg src1, Reg src2) {
    SelectionInstruction *insn = this->appendInsn(opcode, 1, 3);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    insn->src(2) = src2;
  }

  void Selection::Opaque::I64CMP(uint32_t conditional, Reg src0, Reg src1, GenRegister tmp[3], Reg dst) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_I64CMP, 4, 2);
    insn->src(0) = src0;
    insn->src(1) = src1;
    for(int i=0; i<3; i++)
      insn->dst(i) = tmp[i];
    insn->dst(3) = dst;
    insn->extra.function = conditional;
  }

  void Selection::Opaque::I64SATADD(Reg dst, Reg src0, Reg src1, GenRegister tmp[6]) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_I64SATADD, 7, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    for(int i=0; i<6; i++)
      insn->dst(i + 1) = tmp[i];
  }

  void Selection::Opaque::I64SATSUB(Reg dst, Reg src0, Reg src1, GenRegister tmp[6]) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_I64SATSUB, 7, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    for(int i=0; i<6; i++)
      insn->dst(i + 1) = tmp[i];
  }

  void Selection::Opaque::CONVI64_TO_F(Reg dst, Reg src, GenRegister tmp[7]) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_CONVI64_TO_F, 8, 1);
    insn->dst(0) = dst;
    insn->src(0) = src;
    for(int i = 0; i < 7; i ++)
      insn->dst(i + 1) = tmp[i];
  }

  void Selection::Opaque::CONVF_TO_I64(Reg dst, Reg src, GenRegister tmp[3]) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_CONVF_TO_I64, 4, 1);
    insn->dst(0) = dst;
    insn->src(0) = src;
    for(int i = 0; i < 3; i ++)
      insn->dst(i + 1) = tmp[i];
  }

  void Selection::Opaque::I64MADSAT(Reg dst, Reg src0, Reg src1, Reg src2, GenRegister tmp[10]) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_I64MADSAT, 11, 3);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    insn->src(2) = src2;
    for(int i = 0; i < 10; i ++)
      insn->dst(i + 1) = tmp[i];
  }

  void Selection::Opaque::I64_MUL_HI(Reg dst, Reg src0, Reg src1, GenRegister tmp[10]) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_I64_MUL_HI, 11, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    for(int i = 0; i < 10; i ++)
      insn->dst(i + 1) = tmp[i];
  }

  void Selection::Opaque::I64HADD(Reg dst, Reg src0, Reg src1, GenRegister tmp[4]) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_I64HADD, 5, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    for(int i = 0; i < 4; i ++)
      insn->dst(i + 1) = tmp[i];
  }

  void Selection::Opaque::I64RHADD(Reg dst, Reg src0, Reg src1, GenRegister tmp[4]) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_I64RHADD, 5, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    for(int i = 0; i < 4; i ++)
      insn->dst(i + 1) = tmp[i];
  }

  void Selection::Opaque::I64Shift(SelectionOpcode opcode, Reg dst, Reg src0, Reg src1, GenRegister tmp[7]) {
    SelectionInstruction *insn = this->appendInsn(opcode, 8, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    for(int i = 0; i < 7; i ++)
      insn->dst(i + 1) = tmp[i];
  }

  // Boiler plate to initialize the selection library at c++ pre-main
  static SelectionLibrary *selLib = NULL;
  static void destroySelectionLibrary(void) { GBE_DELETE(selLib); }
  static struct SelectionLibraryInitializer {
    SelectionLibraryInitializer(void) {
      selLib = GBE_NEW_NO_ARG(SelectionLibrary);
      atexit(destroySelectionLibrary);
    }
  } selectionLibraryInitializer;

  bool Selection::Opaque::isRoot(const ir::Instruction &insn) const {
    if (insn.getDstNum() > 1 ||
        insn.hasSideEffect() ||
        insn.isMemberOf<ir::BranchInstruction>() ||
        insn.isMemberOf<ir::LabelInstruction>())
    return true;

    // No side effect, not a branch and no destination? Impossible
    GBE_ASSERT(insn.getDstNum() == 1);

    // Root if alive outside the block.
    // XXX we should use Value and not registers in liveness info
    const ir::BasicBlock *insnBlock = insn.getParent();
    const ir::Liveness &liveness = this->ctx.getLiveness();
    const ir::Liveness::LiveOut &liveOut = liveness.getLiveOut(insnBlock);
    const ir::Register reg = insn.getDst(0);
    if (liveOut.contains(reg))
      return true;

    // The instruction is only used in the current basic block
    return false;
  }

  uint32_t Selection::Opaque::buildBasicBlockDAG(const ir::BasicBlock &bb)
  {
    using namespace ir;

    // Clear all registers
    for (uint32_t regID = 0; regID < this->regNum; ++regID)
      this->regDAG[regID] = NULL;

    // Build the DAG on the fly
    uint32_t insnNum = 0;
    const_cast<BasicBlock&>(bb).foreach([&](const Instruction &insn) {

      // Build a selectionDAG node for instruction
      SelectionDAG *dag = this->newSelectionDAG(insn);

      // Point to non-root children
      const uint32_t srcNum = insn.getSrcNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        const ir::Register reg = insn.getSrc(srcID);
        SelectionDAG *child = this->regDAG[reg];
        if (child) {
          const ir::Instruction &childInsn = child->insn;
          const uint32_t childSrcNum = childInsn.getSrcNum();

          // We can merge a child only if its sources are still valid
          bool mergeable = true;
          for (uint32_t otherID = 0; otherID < childSrcNum; ++otherID) {
            const SelectionDAG *srcDAG = child->child[otherID];
            const ir::Register srcReg = childInsn.getSrc(otherID);
            SelectionDAG *currDAG = this->regDAG[srcReg];
            if (srcDAG != currDAG) {
              mergeable = false;
              break;
            }
          }
          if (mergeable) dag->setAsMergeable(srcID);
          dag->child[srcID] = child;
        } else
          dag->child[srcID] = NULL;
      }

      // Make it a root if we must
      if (this->isRoot(insn)) dag->isRoot = 1;

      // Save the DAG <-> instruction mapping
      this->insnDAG[insnNum++] = dag;

      // Associate all output registers to this instruction
      const uint32_t dstNum = insn.getDstNum();
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
        const ir::Register reg = insn.getDst(dstID);
        this->regDAG[reg] = dag;
      }
    });

    return insnNum;
  }

  void Selection::Opaque::matchBasicBlock(uint32_t insnNum)
  {
    // Bottom up code generation
    for (int32_t insnID = insnNum-1; insnID >= 0; --insnID) {
      // Process all possible patterns for this instruction
      SelectionDAG &dag = *insnDAG[insnID];
      if (dag.isRoot) {
        const ir::Instruction &insn = dag.insn;
        const ir::Opcode opcode = insn.getOpcode();
        auto it = selLib->patterns[opcode].begin();
        const auto end = selLib->patterns[opcode].end();

        // Start a new code fragment
        this->startBackwardGeneration();

        // Try all the patterns from best to worst
        do {
          if ((*it)->emit(*this, dag))
            break;
          ++it;
        } while (it != end);
        GBE_ASSERT(it != end);

        // Output the code in the current basic block
        this->endBackwardGeneration();
      }
    }
  }

  void Selection::Opaque::select(void)
  {
    using namespace ir;
    const Function &fn = ctx.getFunction();

    // Perform the selection per basic block
    fn.foreachBlock([&](const BasicBlock &bb) {
      this->dagPool.rewind();
      this->appendBlock(bb);
      const uint32_t insnNum = this->buildBasicBlockDAG(bb);
      this->matchBasicBlock(insnNum);
    });
   }

  void Selection::Opaque::SAMPLE(GenRegister *dst, uint32_t dstNum,
                                 GenRegister *msgPayloads, uint32_t msgNum,
                                 uint32_t bti, uint32_t sampler, bool is3D) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_SAMPLE, dstNum, msgNum);
    SelectionVector *dstVector = this->appendVector();
    SelectionVector *msgVector = this->appendVector();

    // Regular instruction to encode
    for (uint32_t elemID = 0; elemID < dstNum; ++elemID)
      insn->dst(elemID) = dst[elemID];
    for (uint32_t elemID = 0; elemID < msgNum; ++elemID)
      insn->src(elemID) = msgPayloads[elemID];

    // Sends require contiguous allocation
    dstVector->regNum = dstNum;
    dstVector->isSrc = 0;
    dstVector->reg = &insn->dst(0);

    // Only the messages require contiguous registers.
    msgVector->regNum = msgNum;
    msgVector->isSrc = 1;
    msgVector->reg = &insn->src(0);

    insn->extra.rdbti = bti;
    insn->extra.sampler = sampler;
    insn->extra.rdmsglen = msgNum;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Code selection public implementation
  ///////////////////////////////////////////////////////////////////////////

  Selection::Selection(GenContext &ctx) {
    this->blockList = NULL;
    this->opaque = GBE_NEW(Selection::Opaque, ctx);
  }

  void Selection::Opaque::TYPED_WRITE(GenRegister *msgs, uint32_t msgNum,
                                      uint32_t bti, bool is3D) {
    uint32_t elemID = 0;
    uint32_t i;
    SelectionInstruction *insn = this->appendInsn(SEL_OP_TYPED_WRITE, 0, msgNum);
    SelectionVector *msgVector = this->appendVector();;

    for( i = 0; i < msgNum; ++i, ++elemID)
      insn->src(elemID) = msgs[i];

    insn->extra.bti = bti;
    insn->extra.msglen = msgNum;
    insn->extra.is3DWrite = is3D;
    // Sends require contiguous allocation
    msgVector->regNum = msgNum;
    msgVector->isSrc = 1;
    msgVector->reg = &insn->src(0);
  }

  Selection::~Selection(void) { GBE_DELETE(this->opaque); }

  void Selection::select(void) {
    this->opaque->select();
    this->blockList = &this->opaque->blockList;
  }

  bool Selection::isScalarOrBool(ir::Register reg) const {
    return this->opaque->isScalarOrBool(reg);
  }

  uint32_t Selection::getLargestBlockSize(void) const {
    return this->opaque->getLargestBlockSize();
  }

  uint32_t Selection::getVectorNum(void) const {
    return this->opaque->getVectorNum();
  }

  uint32_t Selection::getRegNum(void) const {
    return this->opaque->getRegNum();
  }

  ir::RegisterFamily Selection::getRegisterFamily(ir::Register reg) const {
    return this->opaque->getRegisterFamily(reg);
  }

  ir::RegisterData Selection::getRegisterData(ir::Register reg) const {
    return this->opaque->getRegisterData(reg);
  }

  ir::Register Selection::replaceSrc(SelectionInstruction *insn, uint32_t regID) {
    return this->opaque->replaceSrc(insn, regID);
  }

  ir::Register Selection::replaceDst(SelectionInstruction *insn, uint32_t regID) {
    return this->opaque->replaceDst(insn, regID);
  }
  bool Selection::spillRegs(const SpilledRegs &spilledRegs, uint32_t registerPool) {
    return this->opaque->spillRegs(spilledRegs, registerPool);
  }

  bool Selection::isScalarReg(const ir::Register &reg) const {
    return this->opaque->isScalarReg(reg);
  }

  SelectionInstruction *Selection::create(SelectionOpcode opcode, uint32_t dstNum, uint32_t srcNum) {
    return this->opaque->create(opcode, dstNum, srcNum);
  }

  ///////////////////////////////////////////////////////////////////////////
  // Implementation of all patterns
  ///////////////////////////////////////////////////////////////////////////

  bool canGetRegisterFromImmediate(const ir::Instruction &insn) {
    using namespace ir;
    const auto &childInsn = cast<LoadImmInstruction>(insn);
    const auto &imm = childInsn.getImmediate();
    if(imm.type != TYPE_DOUBLE && imm.type != TYPE_S64 && imm.type != TYPE_U64)
      return true;
    return false;
  }

  GenRegister getRegisterFromImmediate(ir::Immediate imm)
  {
    using namespace ir;
    switch (imm.type) {
      case TYPE_U32:   return GenRegister::immud(imm.data.u32);
      case TYPE_S32:   return GenRegister::immd(imm.data.s32);
      case TYPE_FLOAT: return GenRegister::immf(imm.data.f32);
      case TYPE_U16: return GenRegister::immuw(imm.data.u16);
      case TYPE_S16: return  GenRegister::immw(imm.data.s16);
      case TYPE_U8:  return GenRegister::immuw(imm.data.u8);
      case TYPE_S8:  return GenRegister::immw(imm.data.s8);
      case TYPE_DOUBLE: return GenRegister::immdf(imm.data.f64);
      case TYPE_BOOL: return GenRegister::immuw(-imm.data.b);  //return 0xffff when true
      default: NOT_SUPPORTED; return GenRegister::immuw(0);
    }
  }

  /*! Template for the one-to-many instruction patterns */
  template <typename T, typename U>
  class OneToManyPattern : public SelectionPattern
  {
  public:
    /*! Register the pattern for all opcodes of the family */
    OneToManyPattern(uint32_t insnNum, uint32_t cost) :
      SelectionPattern(insnNum, cost)
    {
      for (uint32_t op = 0; op < ir::OP_INVALID; ++op)
        if (ir::isOpcodeFrom<U>(ir::Opcode(op)) == true)
          this->opcodes.push_back(ir::Opcode(op));
    }
    /*! Call the child method with the proper prototype */
    virtual bool emit(Selection::Opaque &sel, SelectionDAG &dag) const {
      if (static_cast<const T*>(this)->emitOne(sel, ir::cast<U>(dag.insn))) {
        markAllChildren(dag);
        return true;
      }
      return false;
    }
  };

/*! Declare a naive one-to-many pattern */
#define DECL_PATTERN(FAMILY) \
  struct FAMILY##Pattern : public OneToManyPattern<FAMILY##Pattern, ir::FAMILY>

#define DECL_CTOR(FAMILY, INSN_NUM, COST) \
  FAMILY##Pattern(void) : OneToManyPattern<FAMILY##Pattern, ir::FAMILY>(INSN_NUM, COST) {}

  /*! Unary instruction patterns */
  DECL_PATTERN(UnaryInstruction)
  {
    static ir::Type getType(const ir::Opcode opcode, const ir::Type insnType) {
      if (insnType == ir::TYPE_S64 || insnType == ir::TYPE_U64 || insnType == ir::TYPE_S8 || insnType == ir::TYPE_U8)
        return insnType;
      if (opcode == ir::OP_FBH || opcode == ir::OP_FBL)
        return ir::TYPE_U32;
      if (insnType == ir::TYPE_S16 || insnType == ir::TYPE_U16)
        return insnType;
      if (insnType == ir::TYPE_BOOL)
        return ir::TYPE_U16;
      return ir::TYPE_FLOAT;
    }

    INLINE bool emitOne(Selection::Opaque &sel, const ir::UnaryInstruction &insn) const {
      const ir::Opcode opcode = insn.getOpcode();
      const ir::Type insnType = insn.getType();
      const GenRegister dst = sel.selReg(insn.getDst(0), getType(opcode, insnType));
      const GenRegister src = sel.selReg(insn.getSrc(0), getType(opcode, insnType));
      switch (opcode) {
        case ir::OP_ABS:
          if (insn.getType() == ir::TYPE_S32) {
            const GenRegister src_ = GenRegister::retype(src, GEN_TYPE_D);
            const GenRegister dst_ = GenRegister::retype(dst, GEN_TYPE_D);
            sel.MOV(dst_, GenRegister::abs(src_));
          } else {
            GBE_ASSERT(insn.getType() == ir::TYPE_FLOAT);
            sel.MOV(dst, GenRegister::abs(src));
          }
          break;
        case ir::OP_MOV:
          if (dst.isdf()) {
            ir::Register r = sel.reg(ir::RegisterFamily::FAMILY_QWORD);
            sel.MOV_DF(dst, src, sel.selReg(r));
          } else
            sel.MOV(dst, src);
          break;
        case ir::OP_RNDD: sel.RNDD(dst, src); break;
        case ir::OP_RNDE: sel.RNDE(dst, src); break;
        case ir::OP_RNDU: sel.RNDU(dst, src); break;
        case ir::OP_RNDZ: sel.RNDZ(dst, src); break;
        case ir::OP_FBH: sel.FBH(dst, src); break;
        case ir::OP_FBL: sel.FBL(dst, src); break;
        case ir::OP_COS: sel.MATH(dst, GEN_MATH_FUNCTION_COS, src); break;
        case ir::OP_SIN: sel.MATH(dst, GEN_MATH_FUNCTION_SIN, src); break;
        case ir::OP_LOG: sel.MATH(dst, GEN_MATH_FUNCTION_LOG, src); break;
        case ir::OP_EXP: sel.MATH(dst, GEN_MATH_FUNCTION_EXP, src); break;
        case ir::OP_SQR: sel.MATH(dst, GEN_MATH_FUNCTION_SQRT, src); break;
        case ir::OP_RSQ: sel.MATH(dst, GEN_MATH_FUNCTION_RSQ, src); break;
        case ir::OP_RCP: sel.MATH(dst, GEN_MATH_FUNCTION_INV, src); break;
        default: NOT_SUPPORTED;
      }
      return true;
    }
    DECL_CTOR(UnaryInstruction, 1, 1)
  };

  BVAR(OCL_OPTIMIZE_IMMEDIATE, true);

  /*! Binary regular instruction pattern */
  class BinaryInstructionPattern : public SelectionPattern
  {
  public:
    BinaryInstructionPattern(void) : SelectionPattern(1,1) {
      for (uint32_t op = 0; op < ir::OP_INVALID; ++op)
        if (ir::isOpcodeFrom<ir::BinaryInstruction>(ir::Opcode(op)) == true)
          this->opcodes.push_back(ir::Opcode(op));
    }

    bool emitDivRemInst(Selection::Opaque &sel, SelectionDAG &dag, ir::Opcode op) const
    {
      using namespace ir;
      const ir::BinaryInstruction &insn = cast<BinaryInstruction>(dag.insn);
      const Type type = insn.getType();
      GenRegister dst  = sel.selReg(insn.getDst(0), type);
      GenRegister src0 = sel.selReg(insn.getSrc(0), type);
      GenRegister src1 = sel.selReg(insn.getSrc(1), type);
      const uint32_t simdWidth = sel.curr.execWidth;
      const RegisterFamily family = getFamily(type);
      uint32_t function = (op == OP_DIV)?
                          GEN_MATH_FUNCTION_INT_DIV_QUOTIENT :
                          GEN_MATH_FUNCTION_INT_DIV_REMAINDER;

      //bytes and shorts must be converted to int for DIV and REM per GEN restriction
      if((family == FAMILY_WORD || family == FAMILY_BYTE)) {
        GenRegister tmp0, tmp1;
        ir::Register reg = sel.reg(FAMILY_DWORD);

        tmp0 = GenRegister::udxgrf(simdWidth, reg);
        tmp0 = GenRegister::retype(tmp0, GEN_TYPE_D);
        sel.MOV(tmp0, src0);

        tmp1 = GenRegister::udxgrf(simdWidth, sel.reg(FAMILY_DWORD));
        tmp1 = GenRegister::retype(tmp1, GEN_TYPE_D);
        sel.MOV(tmp1, src1);

        sel.MATH(tmp0, function, tmp0, tmp1);
        GenRegister unpacked;
        if(family == FAMILY_WORD) {
          unpacked = GenRegister::unpacked_uw(reg);
        } else {
          unpacked = GenRegister::unpacked_ub(reg);
        }
        unpacked = GenRegister::retype(unpacked, getGenType(type));
        sel.MOV(dst, unpacked);
      } else if (type == TYPE_S32 || type == TYPE_U32 ) {
        sel.MATH(dst, function, src0, src1);
      } else if(type == TYPE_FLOAT) {
        GBE_ASSERT(op != OP_REM);
        sel.MATH(dst, GEN_MATH_FUNCTION_FDIV, src0, src1);
      } else if (type == TYPE_S64 || type == TYPE_U64) {
        GenRegister tmp[14];
        for(int i=0; i<13; i++) {
          tmp[i] = sel.selReg(sel.reg(FAMILY_DWORD));
          tmp[i].type = GEN_TYPE_UD;
        }
        tmp[13] = sel.selReg(sel.reg(FAMILY_BOOL, true));
        if(op == OP_DIV)
          sel.I64DIV(dst, src0, src1, tmp);
        else
          sel.I64REM(dst, src0, src1, tmp);
      }
      markAllChildren(dag);
      return true;
    }

    INLINE bool emit(Selection::Opaque &sel, SelectionDAG &dag) const
    {
      using namespace ir;
      const ir::BinaryInstruction &insn = cast<BinaryInstruction>(dag.insn);
      const Opcode opcode = insn.getOpcode();
      const Type type = insn.getType();
      GenRegister dst  = sel.selReg(insn.getDst(0), type);

      if(opcode == OP_DIV || opcode == OP_REM) {
        return this->emitDivRemInst(sel, dag, opcode);
      }
      // Immediates not supported
      if (opcode == OP_POW) {
        GenRegister src0 = sel.selReg(insn.getSrc(0), type);
        GenRegister src1 = sel.selReg(insn.getSrc(1), type);

        if(type == TYPE_FLOAT) {
          sel.MATH(dst, GEN_MATH_FUNCTION_POW, src0, src1);
        } else {
          NOT_IMPLEMENTED;
        }
        markAllChildren(dag);
        return true;
      }

      sel.push();

      // Boolean values use scalars
      if (sel.isScalarOrBool(insn.getDst(0)) == true) {
        sel.curr.execWidth = 1;
        sel.curr.predicate = GEN_PREDICATE_NONE;
        sel.curr.noMask = 1;
      }

      // Look for immediate values
      GenRegister src0, src1;
      SelectionDAG *dag0 = dag.child[0];
      SelectionDAG *dag1 = dag.child[1];

      // Right source can always be an immediate
      //logica ops of bool shouldn't use 0xffff, may use flag reg, so can't optimize
      if (OCL_OPTIMIZE_IMMEDIATE && dag1 != NULL && dag1->insn.getOpcode() == OP_LOADI &&
          canGetRegisterFromImmediate(dag1->insn) && type != TYPE_BOOL) {
        const auto &childInsn = cast<LoadImmInstruction>(dag1->insn);
        src0 = sel.selReg(insn.getSrc(0), type);
        src1 = getRegisterFromImmediate(childInsn.getImmediate());
        if (dag0) dag0->isRoot = 1;
      }
      // Left source cannot be immediate but it is OK if we can commute
      else if (OCL_OPTIMIZE_IMMEDIATE && dag0 != NULL && insn.commutes() && dag0->insn.getOpcode() == OP_LOADI &&
               canGetRegisterFromImmediate(dag0->insn) && type != TYPE_BOOL) {
        const auto &childInsn = cast<LoadImmInstruction>(dag0->insn);
        src0 = sel.selReg(insn.getSrc(1), type);
        src1 = getRegisterFromImmediate(childInsn.getImmediate());
        if (dag1) dag1->isRoot = 1;
      }
      // Just grab the two sources
      else {
        src0 = sel.selReg(insn.getSrc(0), type);
        src1 = sel.selReg(insn.getSrc(1), type);
        markAllChildren(dag);
      }

      // Output the binary instruction
      switch (opcode) {
        case OP_ADD:
          if (type == Type::TYPE_U64 || type == Type::TYPE_S64) {
            GenRegister t = sel.selReg(sel.reg(RegisterFamily::FAMILY_QWORD), Type::TYPE_S64);
            sel.I64ADD(dst, src0, src1, t);
          } else
            sel.ADD(dst, src0, src1);
          break;
        case OP_ADDSAT:
          if (type == Type::TYPE_U64 || type == Type::TYPE_S64) {
            GenRegister tmp[6];
            for(int i=0; i<5; i++) {
              tmp[i] = sel.selReg(sel.reg(FAMILY_DWORD));
              tmp[i].type = GEN_TYPE_UD;
            }
            tmp[5] = sel.selReg(sel.reg(FAMILY_BOOL, true));
            sel.I64SATADD(dst, src0, src1, tmp);
            break;
          }
          sel.push();
            sel.curr.saturate = GEN_MATH_SATURATE_SATURATE;
            sel.ADD(dst, src0, src1);
          sel.pop();
          break;
        case OP_XOR:
          if (type == Type::TYPE_U64 || type == Type::TYPE_S64)
            sel.I64XOR(dst, src0, src1);
          else
            sel.XOR(dst, src0, src1);
          break;
        case OP_OR:
          if (type == Type::TYPE_U64 || type == Type::TYPE_S64)
            sel.I64OR(dst, src0, src1);
          else
            sel.OR(dst, src0, src1);
          break;
        case OP_AND:
          if (type == Type::TYPE_U64 || type == Type::TYPE_S64)
            sel.I64AND(dst, src0, src1);
          else
            sel.AND(dst, src0, src1);
          break;
        case OP_SUB:
          if (type == Type::TYPE_U64 || type == Type::TYPE_S64) {
            GenRegister t = sel.selReg(sel.reg(RegisterFamily::FAMILY_QWORD), Type::TYPE_S64);
            sel.I64SUB(dst, src0, src1, t);
          } else
            sel.ADD(dst, src0, GenRegister::negate(src1));
          break;
        case OP_SUBSAT:
          if (type == Type::TYPE_U64 || type == Type::TYPE_S64) {
            GenRegister tmp[6];
            for(int i=0; i<5; i++) {
              tmp[i] = sel.selReg(sel.reg(FAMILY_DWORD));
              tmp[i].type = GEN_TYPE_UD;
            }
            tmp[5] = sel.selReg(sel.reg(FAMILY_BOOL, true));
            sel.I64SATSUB(dst, src0, src1, tmp);
            break;
          }
          sel.push();
            sel.curr.saturate = GEN_MATH_SATURATE_SATURATE;
            sel.ADD(dst, src0, GenRegister::negate(src1));
          sel.pop();
          break;
        case OP_SHL:
          if (type == TYPE_S64 || type == TYPE_U64) {
            GenRegister tmp[7];
            for(int i = 0; i < 6; i ++)
              tmp[i] = sel.selReg(sel.reg(FAMILY_DWORD));
            tmp[6] = sel.selReg(sel.reg(FAMILY_BOOL, true));
            sel.I64SHL(dst, src0, src1, tmp);
          } else
            sel.SHL(dst, src0, src1);
          break;
        case OP_SHR:
          if (type == TYPE_S64 || type == TYPE_U64) {
            GenRegister tmp[7];
            for(int i = 0; i < 6; i ++)
              tmp[i] = sel.selReg(sel.reg(FAMILY_DWORD));
            tmp[6] = sel.selReg(sel.reg(FAMILY_BOOL, true));
            sel.I64SHR(dst, src0, src1, tmp);
          } else
            sel.SHR(dst, src0, src1);
          break;
        case OP_ASR:
          if (type == TYPE_S64 || type == TYPE_U64) {
            GenRegister tmp[7];
            for(int i = 0; i < 6; i ++)
              tmp[i] = sel.selReg(sel.reg(FAMILY_DWORD));
            tmp[6] = sel.selReg(sel.reg(FAMILY_BOOL, true));
            sel.I64ASR(dst, src0, src1, tmp);
          } else
            sel.ASR(dst, src0, src1);
          break;
        case OP_MUL_HI: {
            GenRegister temp = GenRegister::retype(sel.selReg(sel.reg(FAMILY_DWORD)), GEN_TYPE_UD);
            sel.MUL_HI(dst, src0, src1, temp);
            break;
          }
        case OP_I64_MUL_HI:
         {
          GenRegister temp[10];
          for(int i=0; i<9; i++) {
            temp[i] = sel.selReg(sel.reg(FAMILY_DWORD));
            temp[i].type = GEN_TYPE_UD;
          }
          temp[9] = sel.selReg(sel.reg(FAMILY_BOOL, true));
          sel.I64_MUL_HI(dst, src0, src1, temp);
          break;
         }
        case OP_MUL:
          if (type == TYPE_U32 || type == TYPE_S32) {
            sel.pop();
            return false;
          } else if (type == TYPE_S64 || type == TYPE_U64) {
            GenRegister tmp[6];
            for(int i = 0; i < 6; i++)
              tmp[i] = sel.selReg(sel.reg(FAMILY_DWORD));
            sel.I64MUL(dst, src0, src1, tmp);
          } else
            sel.MUL(dst, src0, src1);
          break;
        case OP_HADD: {
            GenRegister temp = GenRegister::retype(sel.selReg(sel.reg(FAMILY_DWORD)), GEN_TYPE_D);
            sel.HADD(dst, src0, src1, temp);
            break;
          }
        case OP_RHADD: {
            GenRegister temp = GenRegister::retype(sel.selReg(sel.reg(FAMILY_DWORD)), GEN_TYPE_D);
            sel.RHADD(dst, src0, src1, temp);
            break;
          }
        case OP_I64HADD:
         {
          GenRegister tmp[4];
          for(int i=0; i<4; i++)
            tmp[i] = sel.selReg(sel.reg(FAMILY_DWORD));
          sel.I64HADD(dst, src0, src1, tmp);
          break;
         }
        case OP_I64RHADD:
         {
          GenRegister tmp[4];
          for(int i=0; i<4; i++)
            tmp[i] = sel.selReg(sel.reg(FAMILY_DWORD));
          sel.I64RHADD(dst, src0, src1, tmp);
          break;
         }
        case OP_UPSAMPLE_SHORT:
          sel.UPSAMPLE_SHORT(dst, src0, src1);
          break;
        case OP_UPSAMPLE_INT:
          sel.UPSAMPLE_INT(dst, src0, src1);
          break;
        case OP_UPSAMPLE_LONG:
          sel.UPSAMPLE_LONG(dst, src0, src1);
          break;
        default: NOT_IMPLEMENTED;
      }
      sel.pop();
      return true;
    }
  };

  /*! MAD pattern */
  class MulAddInstructionPattern : public SelectionPattern
  {
  public:
    /*! Register the pattern for all opcodes of the family */
    MulAddInstructionPattern(void) : SelectionPattern(2, 1) {
       this->opcodes.push_back(ir::OP_ADD);
    }

    /*! Implements base class */
    virtual bool emit(Selection::Opaque  &sel, SelectionDAG &dag) const
    {
      using namespace ir;

      // XXX TODO: we need a clean support of FP_CONTRACT to remove below line 'return false'
      // if 'pragma FP_CONTRACT OFF' is used in cl kernel, we should not do mad optimization.
      return false;
      // MAD tend to increase liveness of the sources (since there are three of
      // them). TODO refine this strategy. Well, we should be able at least to
      // evaluate per basic block register pressure and selectively enable
      // disable MADs
       if (sel.ctx.limitRegisterPressure)
        return false;

      // We are good to try. We need a MUL for one of the two sources
      const ir::BinaryInstruction &insn = cast<ir::BinaryInstruction>(dag.insn);
      if (insn.getType() != TYPE_FLOAT)
        return false;
      SelectionDAG *child0 = dag.child[0];
      SelectionDAG *child1 = dag.child[1];
      const GenRegister dst = sel.selReg(insn.getDst(0), TYPE_FLOAT);
      if (child0 && child0->insn.getOpcode() == OP_MUL) {
        GBE_ASSERT(cast<ir::BinaryInstruction>(child0->insn).getType() == TYPE_FLOAT);
        const GenRegister src0 = sel.selReg(child0->insn.getSrc(0), TYPE_FLOAT);
        const GenRegister src1 = sel.selReg(child0->insn.getSrc(1), TYPE_FLOAT);
        const GenRegister src2 = sel.selReg(insn.getSrc(1), TYPE_FLOAT);
        sel.MAD(dst, src2, src0, src1); // order different on HW!
        if (child0->child[0]) child0->child[0]->isRoot = 1;
        if (child0->child[1]) child0->child[1]->isRoot = 1;
        if (child1) child1->isRoot = 1;
        return true;
      }
      if (child1 && child1->insn.getOpcode() == OP_MUL) {
        GBE_ASSERT(cast<ir::BinaryInstruction>(child1->insn).getType() == TYPE_FLOAT);
        const GenRegister src0 = sel.selReg(child1->insn.getSrc(0), TYPE_FLOAT);
        const GenRegister src1 = sel.selReg(child1->insn.getSrc(1), TYPE_FLOAT);
        const GenRegister src2 = sel.selReg(insn.getSrc(0), TYPE_FLOAT);
        sel.MAD(dst, src2, src0, src1); // order different on HW!
        if (child1->child[0]) child1->child[0]->isRoot = 1;
        if (child1->child[1]) child1->child[1]->isRoot = 1;
        if (child0) child0->isRoot = 1;
        return true;
      }
      return false;
    }
  };

  /*! sel.{le,l,ge...} like patterns */
  class SelectModifierInstructionPattern : public SelectionPattern
  {
  public:
    /*! Register the pattern for all opcodes of the family */
    SelectModifierInstructionPattern(void) : SelectionPattern(2, 1) {
      this->opcodes.push_back(ir::OP_SEL);
    }

    /*! Implements base class */
    virtual bool emit(Selection::Opaque &sel, SelectionDAG &dag) const
    {
      using namespace ir;
      SelectionDAG *cmp = dag.child[0];
      const SelectInstruction &insn = cast<SelectInstruction>(dag.insn);

      if (insn.getType() == TYPE_S64 || insn.getType() == TYPE_U64) // not support
        return false;

      // Not in this block
      if (cmp == NULL) return false;

      // We need to match a compare
      if (cmp->insn.isMemberOf<CompareInstruction>() == false) return false;

      // We look for something like that:
      // cmp.{le,ge...} flag src0 src1
      // sel dst flag src0 src1
      // So both sources must match
      if (sourceMatch(cmp, 0, &dag, 1) == false) return false;
      if (sourceMatch(cmp, 1, &dag, 2) == false) return false;

      // OK, we merge the instructions
      const ir::CompareInstruction &cmpInsn = cast<CompareInstruction>(cmp->insn);
      const ir::Opcode opcode = cmpInsn.getOpcode();
      if(opcode == OP_ORD) return false;
      const uint32_t genCmp = getGenCompare(opcode);

      // Like for regular selects, we need a temporary since we cannot predicate
      // properly
      const ir::Type type = cmpInsn.getType();
      const RegisterFamily family = getFamily(type);
      const GenRegister tmp = sel.selReg(sel.reg(family), type);
      const uint32_t simdWidth = sel.curr.execWidth;
      const GenRegister dst  = sel.selReg(insn.getDst(0), type);
      const GenRegister src0 = sel.selReg(cmpInsn.getSrc(0), type);
      const GenRegister src1 = sel.selReg(cmpInsn.getSrc(1), type);

      sel.push();
        sel.curr.predicate = GEN_PREDICATE_NONE;
        sel.curr.execWidth = simdWidth;
        sel.SEL_CMP(genCmp, tmp, src0, src1);
      sel.pop();

      // Update the destination register properly now
      sel.MOV(dst, tmp);

      // We need the sources of the compare instruction
      markAllChildren(*cmp);

      return true;
    }
  };

  /*! 32 bits integer multiply needs more instructions */
  class Int32x32MulInstructionPattern : public SelectionPattern
  {
  public:
    /*! Register the pattern for all opcodes of the family */
    Int32x32MulInstructionPattern(void) : SelectionPattern(1, 4) {
       this->opcodes.push_back(ir::OP_MUL);
    }

    /*! Implements base class */
    virtual bool emit(Selection::Opaque &sel, SelectionDAG &dag) const
    {
      using namespace ir;
      const ir::BinaryInstruction &insn = cast<ir::BinaryInstruction>(dag.insn);
      const uint32_t simdWidth = sel.curr.execWidth;
      const Type type = insn.getType();
      if (type == TYPE_U32 || type == TYPE_S32) {
        GenRegister dst  = sel.selReg(insn.getDst(0), type);
        GenRegister src0 = sel.selReg(insn.getSrc(0), type);
        GenRegister src1 = sel.selReg(insn.getSrc(1), type);

        sel.push();

        // Either left part of the 16-wide register or just a simd 8 register
        dst  = GenRegister::retype(dst,  GEN_TYPE_D);
        src0 = GenRegister::retype(src0, GEN_TYPE_D);
        src1 = GenRegister::retype(src1, GEN_TYPE_D);
        sel.curr.execWidth = 8;
        sel.curr.quarterControl = GEN_COMPRESSION_Q1;
        sel.MUL(GenRegister::retype(GenRegister::acc(), GEN_TYPE_D), src0, src1);
        sel.curr.accWrEnable = 1;
        sel.MACH(GenRegister::retype(GenRegister::null(), GEN_TYPE_D), src0, src1);
        sel.curr.accWrEnable = 0;
        sel.MOV(GenRegister::retype(dst, GEN_TYPE_F), GenRegister::acc());

        // Right part of the 16-wide register now
        if (simdWidth == 16) {
          int predicate = sel.curr.predicate;
          int noMask = sel.curr.noMask;
          sel.curr.noMask = 1;
          sel.curr.predicate = GEN_PREDICATE_NONE;
          const GenRegister nextSrc0 = sel.selRegQn(insn.getSrc(0), 1, TYPE_S32);
          const GenRegister nextSrc1 = sel.selRegQn(insn.getSrc(1), 1, TYPE_S32);
          sel.MUL(GenRegister::retype(GenRegister::acc(), GEN_TYPE_D), nextSrc0, nextSrc1);
          sel.curr.accWrEnable = 1;
          sel.MACH(GenRegister::retype(GenRegister::null(), GEN_TYPE_D), nextSrc0, nextSrc1);
          sel.curr.accWrEnable = 0;
          sel.curr.quarterControl = GEN_COMPRESSION_Q2;
          if (predicate != GEN_PREDICATE_NONE || noMask != 1) {
            const ir::Register reg = sel.reg(FAMILY_DWORD);
            sel.MOV(GenRegister::f8grf(reg), GenRegister::acc());
            sel.curr.noMask = noMask;;
            sel.curr.predicate = predicate;
            sel.MOV(GenRegister::retype(GenRegister::next(dst), GEN_TYPE_F),
                    GenRegister::f8grf(reg));
          } else
            sel.MOV(GenRegister::retype(GenRegister::next(dst), GEN_TYPE_F), GenRegister::acc());
        }

        sel.pop();

        // All children are marked as root
        markAllChildren(dag);
        return true;
      } else
        return false;
    }
  };

  /*! 32x16 bits integer can be done in one instruction */
  class Int32x16MulInstructionPattern : public SelectionPattern
  {
  public:
    /*! Register the pattern for all opcodes of the family */
    Int32x16MulInstructionPattern(void) : SelectionPattern(1, 1) {
       this->opcodes.push_back(ir::OP_MUL);
    }

    bool is16BitSpecialReg(ir::Register reg) const {
      if (reg == ir::ocl::lid0 ||
          reg == ir::ocl::lid1 ||
          reg == ir::ocl::lid2 ||
          reg == ir::ocl::lsize0 ||
          reg == ir::ocl::lsize1||
          reg == ir::ocl::lsize2)
        return true;
      else
        return false;
    }

    /*! Try to emit a multiply where child childID is a 16 immediate */
    bool emitMulImmediate(Selection::Opaque  &sel, SelectionDAG &dag, uint32_t childID) const {
      using namespace ir;
      const ir::BinaryInstruction &insn = cast<ir::BinaryInstruction>(dag.insn);
      const Register dst  = insn.getDst(0);
      const Register src1 = insn.getSrc(childID ^ 1);
      const SelectionDAG *src0DAG = dag.child[childID];
      if (src0DAG != NULL) {
        if (src0DAG->insn.getOpcode() == OP_LOADI) {
          const auto &loadimm = cast<LoadImmInstruction>(src0DAG->insn);
          const Immediate imm = loadimm.getImmediate();
          const Type type = imm.type;
          GBE_ASSERT(type == TYPE_U32 || type == TYPE_S32);
          if (type == TYPE_U32 && imm.data.u32 <= 0xffff) {
            sel.MUL(sel.selReg(dst, type),
                    sel.selReg(src1, type),
                    GenRegister::immuw(imm.data.u32));
            if (dag.child[childID ^ 1] != NULL)
              dag.child[childID ^ 1]->isRoot = 1;
            return true;
          }
          if (type == TYPE_S32 && (imm.data.s32 >= -32768 && imm.data.s32 <= 32767)) {
            sel.MUL(sel.selReg(dst, type),
                    sel.selReg(src1, type),
                    GenRegister::immw(imm.data.s32));
            if (dag.child[childID ^ 1] != NULL)
              dag.child[childID ^ 1]->isRoot = 1;
            return true;
          }
        }
      }
      return false;
    }

    /*! Try to emit a multiply with a 16 bit special register */
    bool emitMulSpecialReg(Selection::Opaque &sel, SelectionDAG &dag, uint32_t childID) const {
      using namespace ir;
      const BinaryInstruction &insn = cast<ir::BinaryInstruction>(dag.insn);
      const Type type = insn.getType();
      const Register dst  = insn.getDst(0);
      const Register src0 = insn.getSrc(childID);
      const Register src1 = insn.getSrc(childID ^ 1);
      if (is16BitSpecialReg(src0)) {
        sel.MUL(sel.selReg(dst, type),
                sel.selReg(src1, type),
                sel.selReg(src0, TYPE_U32));
        markAllChildren(dag);
        return true;
      }
      return false;
    }

    virtual bool emit(Selection::Opaque &sel, SelectionDAG &dag) const
    {
      using namespace ir;
      const BinaryInstruction &insn = cast<ir::BinaryInstruction>(dag.insn);
      const Type type = insn.getType();
      if (type == TYPE_U32 || type == TYPE_S32) {
        if (this->emitMulSpecialReg(sel, dag, 0))
          return true;
        if (this->emitMulSpecialReg(sel, dag, 1))
          return true;
        if (this->emitMulImmediate(sel, dag, 0))
          return true;
        if (this->emitMulImmediate(sel, dag, 1))
          return true;
      }
      return false;
    }
  };

#define DECL_NOT_IMPLEMENTED_ONE_TO_MANY(FAMILY) \
  struct FAMILY##Pattern : public OneToManyPattern<FAMILY##Pattern, ir::FAMILY>\
  {\
    INLINE bool emitOne(Selection::Opaque &sel, const ir::FAMILY &insn) const {\
      NOT_IMPLEMENTED;\
      return false;\
    }\
    DECL_CTOR(FAMILY, 1, 1); \
  }
#undef DECL_NOT_IMPLEMENTED_ONE_TO_MANY

  /*! Load immediate pattern */
  DECL_PATTERN(LoadImmInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::LoadImmInstruction &insn) const
    {
      using namespace ir;
      const Type type = insn.getType();
      const Immediate imm = insn.getImmediate();
      const GenRegister dst = sel.selReg(insn.getDst(0), type);
      GenRegister flagReg;

      sel.push();
      if (sel.isScalarOrBool(insn.getDst(0)) == true) {
        sel.curr.execWidth = 1;
        sel.curr.predicate = GEN_PREDICATE_NONE;
        sel.curr.noMask = 1;
      }

      switch (type) {
        case TYPE_BOOL:
          sel.MOV(dst, imm.data.b ? GenRegister::immuw(0xffff) : GenRegister::immuw(0));
        break;
        case TYPE_U32:
        case TYPE_S32:
        case TYPE_FLOAT:
          sel.MOV(GenRegister::retype(dst, GEN_TYPE_F),
                  GenRegister::immf(imm.data.f32));
        break;
        case TYPE_U16: sel.MOV(dst, GenRegister::immuw(imm.data.u16)); break;
        case TYPE_S16: sel.MOV(dst, GenRegister::immw(imm.data.s16)); break;
        case TYPE_U8:  sel.MOV(dst, GenRegister::immuw(imm.data.u8)); break;
        case TYPE_S8:  sel.MOV(dst, GenRegister::immw(imm.data.s8)); break;
        case TYPE_DOUBLE: sel.LOAD_DF_IMM(dst, GenRegister::immdf(imm.data.f64), sel.selReg(sel.reg(FAMILY_QWORD))); break;
        case TYPE_S64: sel.LOAD_INT64_IMM(dst, GenRegister::immint64(imm.data.s64)); break;
        case TYPE_U64: sel.LOAD_INT64_IMM(dst, GenRegister::immint64(imm.data.u64)); break;
        default: NOT_SUPPORTED;
      }
      sel.pop();
      return true;
    }

    DECL_CTOR(LoadImmInstruction, 1,1);
  };

  /*! Sync instruction */
  DECL_PATTERN(SyncInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::SyncInstruction &insn) const
    {
      using namespace ir;
      const ir::Register reg = sel.reg(FAMILY_DWORD);
      const GenRegister barrierMask = sel.selReg(ocl::barriermask, TYPE_BOOL);
      const uint32_t params = insn.getParameters();

      sel.push();
        sel.curr.predicate = GEN_PREDICATE_NONE;
        sel.curr.noMask = 1;
        sel.curr.execWidth = 1;
        sel.OR(barrierMask, GenRegister::flag(0, 0), barrierMask);
        sel.MOV(GenRegister::flag(1, 1), barrierMask);
      sel.pop();

      // A barrier is OK to start the thread synchronization *and* SLM fence
      sel.push();
        //sel.curr.predicate = GEN_PREDICATE_NONE;
        sel.curr.flag = 1;
        sel.curr.subFlag = 1;
        sel.BARRIER(GenRegister::ud8grf(reg), sel.selReg(sel.reg(FAMILY_DWORD)), params);
      sel.pop();
      return true;
    }

    DECL_CTOR(SyncInstruction, 1,1);
  };

  INLINE uint32_t getByteScatterGatherSize(ir::Type type) {
    using namespace ir;
    switch (type) {
      case TYPE_DOUBLE:
      case TYPE_S64:
      case TYPE_U64:
        return GEN_BYTE_SCATTER_QWORD;
      case TYPE_FLOAT:
      case TYPE_U32:
      case TYPE_S32:
        return GEN_BYTE_SCATTER_DWORD;
      case TYPE_U16:
      case TYPE_S16:
        return GEN_BYTE_SCATTER_WORD;
      case TYPE_U8:
      case TYPE_S8:
        return GEN_BYTE_SCATTER_BYTE;
      default: NOT_SUPPORTED;
        return GEN_BYTE_SCATTER_BYTE;
    }
  }

  /*! Load instruction pattern */
  DECL_PATTERN(LoadInstruction)
  {
    void emitUntypedRead(Selection::Opaque &sel,
                         const ir::LoadInstruction &insn,
                         GenRegister addr,
                         uint32_t bti) const
    {
      using namespace ir;
      const uint32_t valueNum = insn.getValueNum();
      vector<GenRegister> dst(valueNum);
      for (uint32_t dstID = 0; dstID < valueNum; ++dstID)
        dst[dstID] = GenRegister::retype(sel.selReg(insn.getValue(dstID)), GEN_TYPE_F);
      sel.UNTYPED_READ(addr, dst.data(), valueNum, bti);
    }

    void emitDWordGather(Selection::Opaque &sel,
                         const ir::LoadInstruction &insn,
                         GenRegister addr,
                         uint32_t bti) const
    {
      using namespace ir;
      const uint32_t valueNum = insn.getValueNum();
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      GBE_ASSERT(valueNum == 1);
      GenRegister dst = GenRegister::retype(sel.selReg(insn.getValue(0)), GEN_TYPE_F);
      // get dword based address
      GenRegister addrDW = GenRegister::udxgrf(simdWidth, sel.reg(FAMILY_DWORD));
      sel.SHR(addrDW, GenRegister::retype(addr, GEN_TYPE_UD), GenRegister::immud(2));

      sel.DWORD_GATHER(dst, addrDW, bti);
    }

    void emitRead64(Selection::Opaque &sel,
                         const ir::LoadInstruction &insn,
                         GenRegister addr,
                         uint32_t bti) const
    {
      using namespace ir;
      const uint32_t valueNum = insn.getValueNum();
      uint32_t dstID;
      /* XXX support scalar only right now. */
      GBE_ASSERT(valueNum == 1);

      // The first 16 DWORD register space is for temporary usage at encode stage.
      uint32_t tmpRegNum = (sel.ctx.getSimdWidth() == 8) ? valueNum * 2 : valueNum;
      GenRegister dst[valueNum + tmpRegNum];
      for (dstID = 0; dstID < tmpRegNum ; ++dstID)
        dst[dstID] = sel.selReg(sel.reg(FAMILY_DWORD));
      for ( uint32_t valueID = 0; valueID < valueNum; ++dstID, ++valueID)
        dst[dstID] = sel.selReg(insn.getValue(valueID), ir::TYPE_U64);
      sel.READ64(addr, sel.selReg(sel.reg(FAMILY_QWORD), ir::TYPE_U64), dst, valueNum + tmpRegNum, valueNum, bti);
    }

    void emitByteGather(Selection::Opaque &sel,
                        const ir::LoadInstruction &insn,
                        const uint32_t elemSize,
                        GenRegister address,
                        uint32_t bti) const
    {
      using namespace ir;
      const uint32_t valueNum = insn.getValueNum();
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      if(valueNum > 1) {
        vector<GenRegister> dst(valueNum);
        const uint32_t typeSize = getFamilySize(getFamily(insn.getValueType()));

        if(elemSize == GEN_BYTE_SCATTER_WORD) {
          for(uint32_t i = 0; i < valueNum; i++)
            dst[i] = sel.selReg(insn.getValue(i), ir::TYPE_U16);
        } else if(elemSize == GEN_BYTE_SCATTER_BYTE) {
          for(uint32_t i = 0; i < valueNum; i++)
            dst[i] = sel.selReg(insn.getValue(i), ir::TYPE_U8);
        }

        uint32_t tmpRegNum = typeSize*valueNum / 4;
        vector<GenRegister> tmp(tmpRegNum);
        for(uint32_t i = 0; i < tmpRegNum; i++) {
          tmp[i] = GenRegister::udxgrf(simdWidth, sel.reg(FAMILY_DWORD));
        }

        sel.UNTYPED_READ(address, tmp.data(), tmpRegNum, bti);
        for(uint32_t i = 0; i < tmpRegNum; i++) {
          sel.UNPACK_BYTE(dst.data() + i * 4/typeSize, tmp[i], 4/typeSize);
        }
      } else {
        GBE_ASSERT(insn.getValueNum() == 1);
        const GenRegister value = sel.selReg(insn.getValue(0));
        // We need a temporary register if we read bytes or words
        Register dst = Register(value.value.reg);
        if (elemSize == GEN_BYTE_SCATTER_WORD ||
            elemSize == GEN_BYTE_SCATTER_BYTE) {
          dst = sel.reg(FAMILY_DWORD);
          sel.BYTE_GATHER(GenRegister::fxgrf(simdWidth, dst), address, elemSize, bti);
        }

        // Repack bytes or words using a converting mov instruction
        if (elemSize == GEN_BYTE_SCATTER_WORD)
          sel.MOV(GenRegister::retype(value, GEN_TYPE_UW), GenRegister::unpacked_uw(dst));
        else if (elemSize == GEN_BYTE_SCATTER_BYTE)
          sel.MOV(GenRegister::retype(value, GEN_TYPE_UB), GenRegister::unpacked_ub(dst));
      }
    }

    void emitIndirectMove(Selection::Opaque &sel,
                         const ir::LoadInstruction &insn,
                         GenRegister address) const
    {
      using namespace ir;
      GBE_ASSERT(insn.getValueNum() == 1);   //todo: handle vec later

      const GenRegister dst = sel.selReg(insn.getValue(0), insn.getValueType());
      const GenRegister src = address;
      sel.INDIRECT_MOVE(dst, src);
    }

    INLINE bool emitOne(Selection::Opaque &sel, const ir::LoadInstruction &insn) const {
      using namespace ir;
      const GenRegister address = sel.selReg(insn.getAddress());
      const AddressSpace space = insn.getAddressSpace();
      GBE_ASSERT(insn.getAddressSpace() == MEM_GLOBAL ||
                 insn.getAddressSpace() == MEM_CONSTANT ||
                 insn.getAddressSpace() == MEM_PRIVATE ||
                 insn.getAddressSpace() == MEM_LOCAL);
      GBE_ASSERT(sel.isScalarReg(insn.getValue(0)) == false);
      const Type type = insn.getValueType();
      const uint32_t elemSize = getByteScatterGatherSize(type);
      if (insn.getAddressSpace() == MEM_CONSTANT) {
        // XXX TODO read 64bit constant through constant cache
        // Per HW Spec, constant cache messages can read at least DWORD data.
        // So, byte/short data type, we have to read through data cache.
        if(insn.isAligned() == true && elemSize == GEN_BYTE_SCATTER_QWORD)
          this->emitRead64(sel, insn, address, 0x2);
        else if(insn.isAligned() == true && elemSize == GEN_BYTE_SCATTER_DWORD)
          this->emitDWordGather(sel, insn, address, 0x2);
        else {
          this->emitByteGather(sel, insn, elemSize, address, 0x2);
        }
      }
      else if (insn.isAligned() == true && elemSize == GEN_BYTE_SCATTER_QWORD)
        this->emitRead64(sel, insn, address, space == MEM_LOCAL ? 0xfe : 0x00);
      else if (insn.isAligned() == true && elemSize == GEN_BYTE_SCATTER_DWORD)
        this->emitUntypedRead(sel, insn, address, space == MEM_LOCAL ? 0xfe : 0x00);
      else {
        this->emitByteGather(sel, insn, elemSize, address, space == MEM_LOCAL ? 0xfe : 0x01);
      }
      return true;
    }
    DECL_CTOR(LoadInstruction, 1, 1);
  };

  /*! Store instruction pattern */
  DECL_PATTERN(StoreInstruction)
  {
    void emitUntypedWrite(Selection::Opaque &sel,
                          const ir::StoreInstruction &insn,
                          uint32_t bti) const
    {
      using namespace ir;
      const uint32_t valueNum = insn.getValueNum();
      const uint32_t addrID = ir::StoreInstruction::addressIndex;
      GenRegister addr;
      vector<GenRegister> value(valueNum);

      addr = GenRegister::retype(sel.selReg(insn.getSrc(addrID)), GEN_TYPE_F);;
      for (uint32_t valueID = 0; valueID < valueNum; ++valueID)
        value[valueID] = GenRegister::retype(sel.selReg(insn.getValue(valueID)), GEN_TYPE_F);
      sel.UNTYPED_WRITE(addr, value.data(), valueNum, bti);
    }

    void emitWrite64(Selection::Opaque &sel,
                          const ir::StoreInstruction &insn,
                          uint32_t bti) const
    {
      using namespace ir;
      const uint32_t valueNum = insn.getValueNum();
      const uint32_t addrID = ir::StoreInstruction::addressIndex;
      GenRegister addr;
      uint32_t srcID;
      /* XXX support scalar only right now. */
      GBE_ASSERT(valueNum == 1);
      addr = GenRegister::retype(sel.selReg(insn.getSrc(addrID)), GEN_TYPE_F);
      // The first 16 DWORD register space is for temporary usage at encode stage.
      uint32_t tmpRegNum = (sel.ctx.getSimdWidth() == 8) ? valueNum * 2 : valueNum;
      GenRegister src[valueNum];
      GenRegister dst[tmpRegNum + 1];
      /* dst 0 is for the temporary address register. */
      dst[0] = sel.selReg(sel.reg(FAMILY_DWORD));
      for (srcID = 0; srcID < tmpRegNum; ++srcID)
        dst[srcID + 1] = sel.selReg(sel.reg(FAMILY_DWORD));

      for (uint32_t valueID = 0; valueID < valueNum; ++valueID)
        src[valueID] = sel.selReg(insn.getValue(valueID), ir::TYPE_U64);
      sel.WRITE64(addr, src, valueNum, dst, tmpRegNum + 1, bti);
    }

    void emitByteScatter(Selection::Opaque &sel,
                         const ir::StoreInstruction &insn,
                         const uint32_t elemSize,
                         GenRegister addr,
                         uint32_t bti) const
    {
      using namespace ir;
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      uint32_t valueNum = insn.getValueNum();

      if(valueNum > 1) {
        const uint32_t typeSize = getFamilySize(getFamily(insn.getValueType()));
        vector<GenRegister> value(valueNum);

        if(elemSize == GEN_BYTE_SCATTER_WORD) {
          for(uint32_t i = 0; i < valueNum; i++)
            value[i] = sel.selReg(insn.getValue(i), ir::TYPE_U16);
        } else if(elemSize == GEN_BYTE_SCATTER_BYTE) {
          for(uint32_t i = 0; i < valueNum; i++)
            value[i] = sel.selReg(insn.getValue(i), ir::TYPE_U8);
        }

        uint32_t tmpRegNum = typeSize*valueNum / 4;
        vector<GenRegister> tmp(tmpRegNum);
        for(uint32_t i = 0; i < tmpRegNum; i++) {
          tmp[i] = GenRegister::udxgrf(simdWidth, sel.reg(FAMILY_DWORD));
          sel.PACK_BYTE(tmp[i], value.data() + i * 4/typeSize, 4/typeSize);
        }

        sel.UNTYPED_WRITE(addr, tmp.data(), tmpRegNum, bti);
      } else {
        const GenRegister value = sel.selReg(insn.getValue(0));
        GBE_ASSERT(insn.getValueNum() == 1);
        const GenRegister tmp = GenRegister::udxgrf(simdWidth, sel.reg(FAMILY_DWORD));
        if (elemSize == GEN_BYTE_SCATTER_WORD) {
          sel.MOV(tmp, GenRegister::retype(value, GEN_TYPE_UW));
        } else if (elemSize == GEN_BYTE_SCATTER_BYTE) {
          sel.MOV(tmp, GenRegister::retype(value, GEN_TYPE_UB));
        }
        sel.BYTE_SCATTER(addr, tmp, elemSize, bti);
      }
    }

    INLINE bool emitOne(Selection::Opaque &sel, const ir::StoreInstruction &insn) const
    {
      using namespace ir;
      const AddressSpace space = insn.getAddressSpace();
      const uint32_t bti = space == MEM_LOCAL ? 0xfe : 0x01;
      const Type type = insn.getValueType();
      const uint32_t elemSize = getByteScatterGatherSize(type);
      if (insn.isAligned() == true && elemSize == GEN_BYTE_SCATTER_QWORD)
        this->emitWrite64(sel, insn, bti);
      else if (insn.isAligned() == true && elemSize == GEN_BYTE_SCATTER_DWORD)
        this->emitUntypedWrite(sel, insn, bti);
      else {
        const GenRegister address = sel.selReg(insn.getAddress());
        this->emitByteScatter(sel, insn, elemSize, address, bti);
      }
      return true;
    }
    DECL_CTOR(StoreInstruction, 1, 1);
  };

  /*! Compare instruction pattern */
  class CompareInstructionPattern : public SelectionPattern
  {
  public:
    CompareInstructionPattern(void) : SelectionPattern(1,1) {
      for (uint32_t op = 0; op < ir::OP_INVALID; ++op)
        if (ir::isOpcodeFrom<ir::CompareInstruction>(ir::Opcode(op)) == true)
          this->opcodes.push_back(ir::Opcode(op));
    }

    INLINE bool emit(Selection::Opaque &sel, SelectionDAG &dag) const
    {
      using namespace ir;
      const ir::CompareInstruction &insn = cast<CompareInstruction>(dag.insn);
      const Opcode opcode = insn.getOpcode();
      const Type type = insn.getType();
      const Register dst = insn.getDst(0);
      GenRegister tmpDst;

      if (type == TYPE_BOOL || type == TYPE_U16 || type == TYPE_S16)
        tmpDst = sel.selReg(sel.reg(FAMILY_WORD), TYPE_BOOL);
      else
        tmpDst = sel.selReg(sel.reg(FAMILY_DWORD), TYPE_S32);

      // Look for immediate values for the right source
      GenRegister src0, src1;
      SelectionDAG *dag0 = dag.child[0];
      SelectionDAG *dag1 = dag.child[1];

      // Right source can always be an immediate
      if (OCL_OPTIMIZE_IMMEDIATE && dag1 != NULL && dag1->insn.getOpcode() == OP_LOADI &&
          canGetRegisterFromImmediate(dag1->insn) && opcode != OP_ORD) {
        const auto &childInsn = cast<LoadImmInstruction>(dag1->insn);
        src0 = sel.selReg(insn.getSrc(0), type);
        Immediate imm = childInsn.getImmediate();
        if(imm.type != type)
          imm.type = type;
        src1 = getRegisterFromImmediate(imm);
        if (dag0) dag0->isRoot = 1;
      } else {
        src0 = sel.selReg(insn.getSrc(0), type);
        src1 = sel.selReg(insn.getSrc(1), type);
        markAllChildren(dag);
      }

      sel.push();
        sel.curr.flag = 1;
        sel.curr.subFlag = 1;
        sel.curr.predicate  = GEN_PREDICATE_NONE;
        if (type == TYPE_S64 || type == TYPE_U64) {
          GenRegister tmp[3];
          for(int i=0; i<3; i++)
            tmp[i] = sel.selReg(sel.reg(FAMILY_DWORD));
          sel.push();
            sel.curr.execWidth = 1;
            sel.curr.noMask = 1;
            sel.MOV(GenRegister::flag(1, 1), GenRegister::flag(0, 0));
          sel.pop();
          sel.curr.predicate = GEN_PREDICATE_NORMAL;
          sel.I64CMP(getGenCompare(opcode), src0, src1, tmp, tmpDst);
        } else if(opcode == OP_ORD) {
          sel.push();
            sel.curr.execWidth = 1;
            sel.curr.noMask = 1;
            sel.MOV(GenRegister::flag(1, 1), GenRegister::flag(0, 0));
          sel.pop();
          sel.curr.predicate = GEN_PREDICATE_NORMAL;

          sel.CMP(GEN_CONDITIONAL_EQ, src0, src0, tmpDst);
          sel.CMP(GEN_CONDITIONAL_EQ, src1, src1, tmpDst);
        } else
          sel.CMP(getGenCompare(opcode), src0, src1, tmpDst);
      sel.pop();

      if (!(type == TYPE_BOOL || type == TYPE_U16 || type == TYPE_S16))
        sel.MOV(sel.selReg(dst, TYPE_U16), GenRegister::unpacked_uw((ir::Register)tmpDst.value.reg));
      else
        sel.MOV(sel.selReg(dst, TYPE_U16), tmpDst);
      return true;
    }
  };

  /*! Bit cast instruction pattern */
  DECL_PATTERN(BitCastInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::BitCastInstruction &insn) const
    {
      using namespace ir;
      const Type dstType = insn.getDstType();
      const Type srcType = insn.getSrcType();
      const uint32_t dstNum = insn.getDstNum();
      const uint32_t srcNum = insn.getSrcNum();
      int index = 0, multiple, narrowNum;
      bool narrowDst;
      Type narrowType;

      if(dstNum > srcNum) {
        multiple = dstNum / srcNum;
        narrowType = dstType;
        narrowNum = dstNum;
        narrowDst = 1;
      } else {
        multiple = srcNum / dstNum;
        narrowType = srcType;
        narrowNum = srcNum;
        narrowDst = 0;
      }

      for(int i = 0; i < narrowNum; i++, index++) {
        GenRegister narrowReg, wideReg;
        if(narrowDst) {
          narrowReg = sel.selReg(insn.getDst(i), narrowType);
          wideReg = sel.selReg(insn.getSrc(index/multiple), narrowType);  //retype to narrow type
        } else {
          wideReg = sel.selReg(insn.getDst(index/multiple), narrowType);
          narrowReg = sel.selReg(insn.getSrc(i), narrowType);  //retype to narrow type
        }
        if(wideReg.hstride != GEN_VERTICAL_STRIDE_0) {
          if(multiple == 2) {
            wideReg = GenRegister::unpacked_uw(wideReg.reg());
            wideReg = GenRegister::retype(wideReg, getGenType(narrowType));
          } else if(multiple == 4) {
            wideReg = GenRegister::unpacked_ub(wideReg.reg());
            wideReg = GenRegister::retype(wideReg, getGenType(narrowType));
          } else if(multiple == 8) {  //need to specail handle long to char
            GBE_ASSERT(multiple == 8);
          }
        }
        if(index % multiple) {
          wideReg = GenRegister::offset(wideReg, 0, (index % multiple) * typeSize(wideReg.type));
          wideReg.subphysical = 1;
        }
        GenRegister xdst = narrowDst ? narrowReg : wideReg;
        GenRegister xsrc = narrowDst ? wideReg : narrowReg;

        if((srcType == TYPE_S64 || srcType == TYPE_U64 || srcType == TYPE_DOUBLE) ||
           (dstType == TYPE_S64 || dstType == TYPE_U64 || dstType == TYPE_DOUBLE)) {
          const int simdWidth = sel.curr.execWidth;
          sel.push();
            sel.curr.execWidth = 8;
            xdst.subphysical = 1;
            xsrc.subphysical = 1;
            for(int i = 0; i < simdWidth/4; i ++) {
              sel.curr.chooseNib(i);
              sel.MOV(xdst, xsrc);
              xdst = GenRegister::offset(xdst, 0, 4 * typeSize(getGenType(dstType)));
              xsrc = GenRegister::offset(xsrc, 0, 4 * typeSize(getGenType(srcType)));
            }
          sel.pop();
        } else
          sel.MOV(xdst, xsrc);
      }

      return true;
    }
    DECL_CTOR(BitCastInstruction, 1, 1);
  };

  /*! Convert instruction pattern */
  DECL_PATTERN(ConvertInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::ConvertInstruction &insn) const
    {
      using namespace ir;
      const Type dstType = insn.getDstType();
      const Type srcType = insn.getSrcType();
      const RegisterFamily dstFamily = getFamily(dstType);
      const RegisterFamily srcFamily = getFamily(srcType);
      const GenRegister dst = sel.selReg(insn.getDst(0), dstType);
      const GenRegister src = sel.selReg(insn.getSrc(0), srcType);
      const Opcode opcode = insn.getOpcode();

      if(opcode == ir::OP_SAT_CVT) {
        sel.push();
        sel.curr.saturate = 1;
      }

      // We need two instructions to make the conversion
      if (opcode == OP_F16TO32) {
        sel.F16TO32(dst, src);
      } else if (opcode == OP_F32TO16) {
        GenRegister unpacked;
        unpacked = GenRegister::unpacked_uw(sel.reg(FAMILY_DWORD));
        sel.F32TO16(unpacked, src);
        sel.MOV(dst, unpacked);
      } else if (dstFamily != FAMILY_DWORD && dstFamily != FAMILY_QWORD && (srcFamily == FAMILY_DWORD || srcFamily == FAMILY_QWORD)) {
        GenRegister unpacked;
        if (dstFamily == FAMILY_WORD) {
          const uint32_t type = dstType == TYPE_U16 ? GEN_TYPE_UW : GEN_TYPE_W;
          unpacked = GenRegister::unpacked_uw(sel.reg(FAMILY_DWORD));
          unpacked = GenRegister::retype(unpacked, type);
        } else {
          const uint32_t type = dstType == TYPE_U8 ? GEN_TYPE_UB : GEN_TYPE_B;
          unpacked = GenRegister::unpacked_ub(sel.reg(FAMILY_DWORD));
          unpacked = GenRegister::retype(unpacked, type);
        }
        if(srcFamily == FAMILY_QWORD) {
          GenRegister tmp = sel.selReg(sel.reg(FAMILY_DWORD));
          tmp.type = GEN_TYPE_D;
          sel.CONVI64_TO_I(tmp, src);
          sel.MOV(unpacked, tmp);
        } else
          sel.MOV(unpacked, src);
        sel.MOV(dst, unpacked);
      } else if ((dstType == ir::TYPE_S32 || dstType == ir::TYPE_U32) && srcFamily == FAMILY_QWORD) {
        sel.CONVI64_TO_I(dst, src);
      } else if (dstType == ir::TYPE_FLOAT && srcFamily == FAMILY_QWORD) {
        GenRegister tmp[7];
        for(int i=0; i<6; i++) {
          tmp[i] = sel.selReg(sel.reg(FAMILY_DWORD), TYPE_U32);
        }
        tmp[6] = sel.selReg(sel.reg(FAMILY_BOOL, true), TYPE_BOOL);
        sel.CONVI64_TO_F(dst, src, tmp);
      } else if (dst.isdf()) {
        ir::Register r = sel.reg(ir::RegisterFamily::FAMILY_QWORD);
        sel.MOV_DF(dst, src, sel.selReg(r));
      } else if (dst.isint64()) {
        switch(src.type) {
          case GEN_TYPE_F:
          {
            GenRegister tmp[3];
            tmp[0] = sel.selReg(sel.reg(FAMILY_DWORD), TYPE_U32);
            tmp[1] = sel.selReg(sel.reg(FAMILY_DWORD), TYPE_FLOAT);
            tmp[2] = sel.selReg(sel.reg(FAMILY_BOOL, true), TYPE_BOOL);
            sel.CONVF_TO_I64(dst, src, tmp);
            break;
          }
          case GEN_TYPE_DF:
            NOT_IMPLEMENTED;
          default:
            sel.CONVI_TO_I64(dst, src, sel.selReg(sel.reg(FAMILY_DWORD)));
        }
      } else
        sel.MOV(dst, src);

      if(opcode == ir::OP_SAT_CVT)
        sel.pop();

      return true;
    }
    DECL_CTOR(ConvertInstruction, 1, 1);
  };

  /*! Convert instruction pattern */
  DECL_PATTERN(AtomicInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::AtomicInstruction &insn) const
    {
      using namespace ir;
      const AtomicOps atomicOp = insn.getAtomicOpcode();
      const AddressSpace space = insn.getAddressSpace();
      const uint32_t bti = space == MEM_LOCAL ? 0xfe : 0x01;
      const uint32_t srcNum = insn.getSrcNum();
      const GenRegister src0 = sel.selReg(insn.getSrc(0), TYPE_U32);   //address
      GenRegister src1 = src0, src2 = src0;
      if(srcNum > 1) src1 = sel.selReg(insn.getSrc(1), TYPE_U32);
      if(srcNum > 2) src2 = sel.selReg(insn.getSrc(2), TYPE_U32);
      GenRegister dst  = sel.selReg(insn.getDst(0), TYPE_U32);
      GenAtomicOpCode genAtomicOp = (GenAtomicOpCode)atomicOp;
      sel.ATOMIC(dst, genAtomicOp, srcNum, src0, src1, src2, bti);
      return true;
    }
    DECL_CTOR(AtomicInstruction, 1, 1);
  };

  /*! Select instruction pattern */
  class SelectInstructionPattern : public SelectionPattern
  {
  public:
    SelectInstructionPattern(void) : SelectionPattern(1,1) {
      for (uint32_t op = 0; op < ir::OP_INVALID; ++op)
        if (ir::isOpcodeFrom<ir::SelectInstruction>(ir::Opcode(op)) == true)
          this->opcodes.push_back(ir::Opcode(op));
    }

    INLINE bool emit(Selection::Opaque &sel, SelectionDAG &dag) const
    {
      using namespace ir;
      const ir::SelectInstruction &insn = cast<SelectInstruction>(dag.insn);

      // Get all registers for the instruction
      const Type type = insn.getType();
      const GenRegister dst  = sel.selReg(insn.getDst(0), type);

      // Look for immediate values for the right source
      GenRegister src0, src1;
      SelectionDAG *dag0 = dag.child[0]; // source 0 is the predicate!
      SelectionDAG *dag1 = dag.child[1];
      SelectionDAG *dag2 = dag.child[2];

      // Right source can always be an immediate
      if (OCL_OPTIMIZE_IMMEDIATE && dag2 != NULL && dag2->insn.getOpcode() == OP_LOADI && canGetRegisterFromImmediate(dag2->insn)) {
        const auto &childInsn = cast<LoadImmInstruction>(dag2->insn);
        src0 = sel.selReg(insn.getSrc(SelectInstruction::src0Index), type);
        src1 = getRegisterFromImmediate(childInsn.getImmediate());
        if (dag0) dag0->isRoot = 1;
        if (dag1) dag1->isRoot = 1;
      } else {
        src0 = sel.selReg(insn.getSrc(SelectInstruction::src0Index), type);
        src1 = sel.selReg(insn.getSrc(SelectInstruction::src1Index), type);
        markAllChildren(dag);
      }

      // Since we cannot predicate the select instruction with our current mask,
      // we need to perform the selection in two steps (one to select, one to
      // update the destination register)
      const RegisterFamily family = getFamily(type);
      const GenRegister tmp = sel.selReg(sel.reg(family), type);
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      const Register pred = insn.getPredicate();
      sel.push();
        sel.curr.predicate = GEN_PREDICATE_NONE;
        sel.curr.execWidth = simdWidth;
        sel.curr.flag = 1;
        sel.curr.subFlag = 1;
        sel.CMP(GEN_CONDITIONAL_NEQ, sel.selReg(pred, TYPE_U16), GenRegister::immuw(0));
        sel.curr.noMask = 0;
        sel.curr.predicate = GEN_PREDICATE_NORMAL;
        if(type == ir::TYPE_S64 || type == ir::TYPE_U64)
          sel.SEL_INT64(tmp, src0, src1);
        else
          sel.SEL(tmp, src0, src1);
      sel.pop();

      // Update the destination register properly now
      sel.MOV(dst, tmp);
      return true;
    }
  };

  DECL_PATTERN(TernaryInstruction)
   {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::TernaryInstruction &insn) const {
      using namespace ir;
      const Type type = insn.getType();
      const GenRegister dst = sel.selReg(insn.getDst(0), type),
                        src0 = sel.selReg(insn.getSrc(0), type),
                        src1 = sel.selReg(insn.getSrc(1), type),
                        src2 = sel.selReg(insn.getSrc(2), type);
      switch(insn.getOpcode()) {
        case OP_I64MADSAT:
         {
          GenRegister tmp[10];
          for(int i=0; i<9; i++) {
            tmp[i] = sel.selReg(sel.reg(FAMILY_DWORD));
            tmp[i].type = GEN_TYPE_UD;
          }
          tmp[9] = sel.selReg(sel.reg(FAMILY_BOOL, true));
          sel.I64MADSAT(dst, src0, src1, src2, tmp);
          break;
         }
        case OP_MAD:
         {
          sel.MAD(dst, src2, src0, src1);
          break;
         }
        default:
          NOT_IMPLEMENTED;
      }
      return true;
    }

    DECL_CTOR(TernaryInstruction, 1, 1);
   };

  /*! Label instruction pattern */
  DECL_PATTERN(LabelInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::LabelInstruction &insn) const
    {
      using namespace ir;
      const LabelIndex label = insn.getLabelIndex();
      const GenRegister src0 = sel.selReg(ocl::blockip);
      const GenRegister src1 = GenRegister::immuw(label);
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      sel.LABEL(label);

     // Do not emit any code for the "returning" block. There is no need for it
     if (insn.getParent() == &sel.ctx.getFunction().getBottomBlock())
        return true;

      // Emit the mask computation at the head of each basic block
      sel.push();
        sel.curr.predicate = GEN_PREDICATE_NONE;
        sel.curr.flag = 0;
        sel.curr.subFlag = 0;
        sel.CMP(GEN_CONDITIONAL_LE, GenRegister::retype(src0, GEN_TYPE_UW), src1);
      sel.pop();

      // If it is required, insert a JUMP to bypass the block
      if (sel.ctx.hasJIP(&insn)) {
        const LabelIndex jip = sel.ctx.getLabelIndex(&insn);
        sel.push();

          sel.curr.noMask = 1;
          sel.curr.execWidth = 1;
          sel.curr.predicate = GEN_PREDICATE_NONE;
          GenRegister emaskReg = GenRegister::uw1grf(ocl::emask);
          GenRegister flagReg = GenRegister::flag(0, 0);
          sel.AND(flagReg, flagReg, emaskReg);

          if (simdWidth == 8)
            sel.curr.predicate = GEN_PREDICATE_ALIGN1_ANY8H;
          else if (simdWidth == 16)
            sel.curr.predicate = GEN_PREDICATE_ALIGN1_ANY16H;
          else
            NOT_IMPLEMENTED;
          sel.curr.inversePredicate = 1;
          sel.curr.flag = 0;
          sel.curr.subFlag = 0;
          sel.JMPI(GenRegister::immd(0), jip);
        sel.pop();
      }
      return true;
    }
    DECL_CTOR(LabelInstruction, 1, 1);
  };

  DECL_PATTERN(SampleInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::SampleInstruction &insn) const
    {
      using namespace ir;
      GenRegister msgPayloads[4];
      GenRegister dst[insn.getDstNum()];
      uint32_t srcNum = insn.getSrcNum();
      uint32_t valueID = 0;

      for (valueID = 0; valueID < insn.getDstNum(); ++valueID)
        dst[valueID] = sel.selReg(insn.getDst(valueID), insn.getDstType());

      if (!insn.is3D())
        srcNum--;
      /* U, V, [W] */
      for (valueID = 0; valueID < srcNum; ++valueID)
        msgPayloads[valueID] = sel.selReg(insn.getSrc(valueID), insn.getSrcType());

      uint32_t bti = insn.getImageIndex();
      /* We have the clamp border workaround. */
      uint32_t sampler = insn.getSamplerIndex() + insn.getSamplerOffset() * 8;

      sel.SAMPLE(dst, insn.getDstNum(), msgPayloads, srcNum, bti, sampler, insn.is3D());
      return true;
    }
    DECL_CTOR(SampleInstruction, 1, 1);
  };

  /*! Typed write instruction pattern. */
  DECL_PATTERN(TypedWriteInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::TypedWriteInstruction &insn) const
    {
      using namespace ir;
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      GenRegister msgs[9]; // (header + U + V + R + LOD + 4)
      const uint32_t msgNum = (8 / (simdWidth / 8)) + 1;
      const uint32_t coordNum = 3;

      if (simdWidth == 16) {
        for(uint32_t i = 0; i < msgNum; i++)
          msgs[i] = sel.selReg(sel.reg(FAMILY_DWORD), TYPE_U32);
      } else {
        uint32_t valueID = 0;
        msgs[0] = sel.selReg(sel.reg(FAMILY_DWORD), TYPE_U32);
        for(uint32_t msgID = 1; msgID < 1 + coordNum; msgID++, valueID++)
          msgs[msgID] = sel.selReg(insn.getSrc(msgID - 1), insn.getCoordType());
        // fake w.
        if (!insn.is3D())
          msgs[3] = sel.selReg(sel.reg(FAMILY_DWORD), TYPE_U32);
        // LOD.
        msgs[4] = sel.selReg(sel.reg(FAMILY_DWORD), TYPE_U32);
        for(uint32_t msgID = 5; valueID < insn.getSrcNum(); msgID++, valueID++)
          msgs[msgID] = sel.selReg(insn.getSrc(valueID), insn.getSrcType());
      }

      sel.push();
      sel.curr.predicate = GEN_PREDICATE_NONE;
      sel.curr.noMask = 1;
      sel.MOV(msgs[0], GenRegister::immud(0));
      sel.curr.execWidth = 1;

      GenRegister channelEn = GenRegister::offset(msgs[0], 0, 7*4);
      channelEn.subphysical = 1;
      // Enable all channels.
      sel.MOV(channelEn, GenRegister::immud(0xffff));
      sel.curr.execWidth = 8;
      // Set zero LOD.
      if (simdWidth == 8)
        sel.MOV(msgs[4], GenRegister::immud(0));
      else
        sel.MOV(GenRegister::Qn(msgs[2], 0), GenRegister::immud(0));
      sel.pop();

      uint32_t bti = insn.getImageIndex();
      if (simdWidth == 8)
        sel.TYPED_WRITE(msgs, msgNum, bti, insn.is3D());
      else {
        sel.push();
        sel.curr.execWidth = 8;
        for( uint32_t quarter = 0; quarter < 2; quarter++)
        {
          #define QUARTER_MOV0(msgs, msgid, src) \
                    sel.MOV(GenRegister::Qn(GenRegister::retype(msgs[msgid/2], GEN_TYPE_UD), msgid % 2), \
                            GenRegister::Qn(src, quarter))

          #define QUARTER_MOV1(msgs, msgid, src) \
                  sel.MOV(GenRegister::Qn(GenRegister::retype(msgs[msgid/2], src.type), msgid % 2), \
                          GenRegister::Qn(src, quarter))
          sel.curr.quarterControl = (quarter == 0) ? GEN_COMPRESSION_Q1 : GEN_COMPRESSION_Q2;
          // Set U,V,W
          QUARTER_MOV0(msgs, 1, sel.selReg(insn.getSrc(0), insn.getCoordType()));
          QUARTER_MOV0(msgs, 2, sel.selReg(insn.getSrc(1), insn.getCoordType()));
          if (insn.is3D())
            QUARTER_MOV0(msgs, 3, sel.selReg(insn.getSrc(2), insn.getCoordType()));
          // Set R, G, B, A
          QUARTER_MOV1(msgs, 5, sel.selReg(insn.getSrc(3), insn.getSrcType()));
          QUARTER_MOV1(msgs, 6, sel.selReg(insn.getSrc(4), insn.getSrcType()));
          QUARTER_MOV1(msgs, 7, sel.selReg(insn.getSrc(5), insn.getSrcType()));
          QUARTER_MOV1(msgs, 8, sel.selReg(insn.getSrc(6), insn.getSrcType()));
          sel.TYPED_WRITE(msgs, msgNum, bti, insn.is3D());
          #undef QUARTER_MOV0
          #undef QUARTER_MOV1
        }
        sel.pop();
      }
      return true;
    }
    DECL_CTOR(TypedWriteInstruction, 1, 1);
  };

  /*! get image info instruction pattern. */
  DECL_PATTERN(GetImageInfoInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::GetImageInfoInstruction &insn) const
    {
      using namespace ir;
      GenRegister dst;
      dst = sel.selReg(insn.getDst(0), TYPE_U32);
      GenRegister imageInfoReg = GenRegister::ud1grf(insn.getSrc(0));
      sel.MOV(dst, imageInfoReg);

      return true;
    }
    DECL_CTOR(GetImageInfoInstruction, 1, 1);
  };

  /*! Branch instruction pattern */
  DECL_PATTERN(BranchInstruction)
  {

    void emitForwardBranch(Selection::Opaque &sel,
                           const ir::BranchInstruction &insn,
                           ir::LabelIndex dst,
                           ir::LabelIndex src) const
    {
      using namespace ir;
      const GenRegister ip = sel.selReg(ocl::blockip, TYPE_U16);
      const LabelIndex jip = sel.ctx.getLabelIndex(&insn);
      const uint32_t simdWidth = sel.ctx.getSimdWidth();

      // We will not emit any jump if we must go the next block anyway
      const BasicBlock *curr = insn.getParent();
      const BasicBlock *next = curr->getNextBlock();
      const LabelIndex nextLabel = next->getLabelIndex();

      if (insn.isPredicated() == true) {
        const Register pred = insn.getPredicateIndex();

        sel.push();
          // we don't need to set next label to the pcip
          // as if there is no backward jump latter, then obviously everything will work fine.
          // If there is backward jump latter, then all the pcip will be updated correctly there.
          sel.curr.flag = 0;
          sel.curr.subFlag = 0;
          sel.CMP(GEN_CONDITIONAL_NEQ, sel.selReg(pred, TYPE_U16), GenRegister::immuw(0));
          sel.MOV(ip, GenRegister::immuw(uint16_t(dst)));
        sel.pop();

        if (nextLabel == jip) return;
      } else {
        // Update the PcIPs
        sel.MOV(ip, GenRegister::immuw(uint16_t(dst)));

        // Do not emit branch when we go to the next block anyway
        if (nextLabel == jip) return;
        sel.push();
          sel.curr.execWidth = 1;
          sel.curr.noMask = 1;
          sel.curr.predicate = GEN_PREDICATE_NONE;
          sel.JMPI(GenRegister::immd(0), jip);
        sel.pop();
      }
    }

    void emitBackwardBranch(Selection::Opaque &sel,
                            const ir::BranchInstruction &insn,
                            ir::LabelIndex dst,
                            ir::LabelIndex src) const
    {
      using namespace ir;
      const GenRegister ip = sel.selReg(ocl::blockip, TYPE_U16);
      const Function &fn = sel.ctx.getFunction();
      const BasicBlock &bb = fn.getBlock(src);
      const LabelIndex jip = sel.ctx.getLabelIndex(&insn);
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      GBE_ASSERT(bb.getNextBlock() != NULL);

      if (insn.isPredicated() == true) {
        const Register pred = insn.getPredicateIndex();

        // Update the PcIPs for all the branches. Just put the IPs of the next
        // block. Next instruction will properly update the IPs of the lanes
        // that actually take the branch
        const LabelIndex next = bb.getNextBlock()->getLabelIndex();
        sel.MOV(ip, GenRegister::immuw(uint16_t(next)));

        sel.push();
          sel.curr.flag = 0;
          sel.curr.subFlag = 0;
          sel.CMP(GEN_CONDITIONAL_NEQ, sel.selReg(pred, TYPE_U16), GenRegister::immuw(0));
          // Re-update the PcIPs for the branches that takes the backward jump
          sel.MOV(ip, GenRegister::immuw(uint16_t(dst)));

          // We clear all the inactive channel to 0 as the GEN_PREDICATE_ALIGN1_ANY8/16
          // will check those bits as well.
          sel.curr.predicate = GEN_PREDICATE_NONE;
          sel.curr.execWidth = 1;
          sel.curr.noMask = 1;
          GenRegister emaskReg = GenRegister::uw1grf(ocl::emask);
          sel.AND(GenRegister::flag(0, 1), GenRegister::flag(0, 1), emaskReg);

          // Branch to the jump target
          if (simdWidth == 8)
            sel.curr.predicate = GEN_PREDICATE_ALIGN1_ANY8H;
          else if (simdWidth == 16)
            sel.curr.predicate = GEN_PREDICATE_ALIGN1_ANY16H;
          else
            NOT_SUPPORTED;
          sel.JMPI(GenRegister::immd(0), jip);
        sel.pop();

      } else {

        // Update the PcIPs
        sel.MOV(ip, GenRegister::immuw(uint16_t(dst)));

        // Branch to the jump target
        sel.push();
          sel.curr.execWidth = 1;
          sel.curr.noMask = 1;
          sel.curr.predicate = GEN_PREDICATE_NONE;
          sel.JMPI(GenRegister::immd(0), jip);
        sel.pop();
      }
    }

    INLINE bool emitOne(Selection::Opaque &sel, const ir::BranchInstruction &insn) const {
      using namespace ir;
      const Opcode opcode = insn.getOpcode();
      if (opcode == OP_RET)
        sel.EOT();
      else if (opcode == OP_BRA) {
        const LabelIndex dst = insn.getLabelIndex();
        const LabelIndex src = insn.getParent()->getLabelIndex();

        // We handle foward and backward branches differently
        if (uint32_t(dst) <= uint32_t(src))
          this->emitBackwardBranch(sel, insn, dst, src);
        else
          this->emitForwardBranch(sel, insn, dst, src);
      } else
        NOT_IMPLEMENTED;
      return true;
    }

    DECL_CTOR(BranchInstruction, 1, 1);
  };

  /*! Sort patterns */
  INLINE bool cmp(const SelectionPattern *p0, const SelectionPattern *p1) {
    if (p0->insnNum != p1->insnNum)
      return p0->insnNum > p1->insnNum;
    return p0->cost < p1->cost;
  }

  SelectionLibrary::SelectionLibrary(void) {
    this->insert<UnaryInstructionPattern>();
    this->insert<BinaryInstructionPattern>();
    this->insert<TypedWriteInstructionPattern>();
    this->insert<SyncInstructionPattern>();
    this->insert<LoadImmInstructionPattern>();
    this->insert<LoadInstructionPattern>();
    this->insert<StoreInstructionPattern>();
    this->insert<SelectInstructionPattern>();
    this->insert<CompareInstructionPattern>();
    this->insert<BitCastInstructionPattern>();
    this->insert<ConvertInstructionPattern>();
    this->insert<AtomicInstructionPattern>();
    this->insert<TernaryInstructionPattern>();
    this->insert<LabelInstructionPattern>();
    this->insert<BranchInstructionPattern>();
    this->insert<Int32x32MulInstructionPattern>();
    this->insert<Int32x16MulInstructionPattern>();
    this->insert<MulAddInstructionPattern>();
    this->insert<SelectModifierInstructionPattern>();
    this->insert<SampleInstructionPattern>();
    this->insert<GetImageInfoInstructionPattern>();

    // Sort all the patterns with the number of instructions they output
    for (uint32_t op = 0; op < ir::OP_INVALID; ++op)
      std::sort(this->patterns[op].begin(), this->patterns[op].end(), cmp);
  }

  SelectionLibrary::~SelectionLibrary(void) {
    for (auto pattern : this->toFree)
      GBE_DELETE(const_cast<SelectionPattern*>(pattern));
  }

  template <typename PatternType>
  void SelectionLibrary::insert(void) {
    const SelectionPattern *pattern = GBE_NEW_NO_ARG(PatternType);
    this->toFree.push_back(pattern);
    for (auto opcode : pattern->opcodes)
      this->patterns[opcode].push_back(pattern);
  }

} /* namespace gbe */

