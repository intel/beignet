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
 * \file function.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_FUNCTION_HPP__
#define __GBE_IR_FUNCTION_HPP__

#include "ir/immediate.hpp"
#include "ir/register.hpp"
#include "ir/instruction.hpp"
#include "ir/profile.hpp"
#include "sys/vector.hpp"
#include "sys/set.hpp"
#include "sys/alloc.hpp"

#include <ostream>

namespace gbe {
namespace ir {

  /*! Commonly used in the CFG */
  typedef set<BasicBlock*> BlockSet;

  /*! Function basic blocks really belong to a function since:
   *  1 - registers used in the basic blocks belongs to the function register
   *      file
   *  2 - branches point to basic blocks of the same function
   */
  class BasicBlock : public NonCopyable
  {
  public:
    /*! Empty basic block */
    BasicBlock(Function &fn);
    /*! Releases all the instructions */
    ~BasicBlock(void);
    /*! Append a new instruction in the stream */
    void append(Instruction &insn) {
      if (last != NULL) last->setSuccessor(&insn);
      if (first == NULL) first = &insn;
      insn.setParent(this);
      insn.setSuccessor(NULL);
      insn.setPredecessor(last);
      last = &insn;
    }
    /*! Apply the given functor on all instructions */
    template <typename T>
    INLINE void foreach(const T &functor) const {
      Instruction *curr = first;
      while (curr) {
        functor(*curr);
        curr = curr->getSuccessor();
      }
    }
    /*! Apply the given functor on all instructions (reverse order) */
    template <typename T>
    INLINE void rforeach(const T &functor) const {
      Instruction *curr = last;
      while (curr) {
        functor(*curr);
        curr = curr->getPredecessor();
      }
    }
    /*! Get the parent function */
    Function &getParent(void) { return fn; }
    const Function &getParent(void) const { return fn; }
    /*! Get the successors */
    const BlockSet &getSuccessorSet(void) const { return successors; }
    /*! Get the predecessors */
    const BlockSet &getPredecessorSet(void) const { return predecessors; }
  private:
    friend class Function; //!< Owns the basic blocks
    BlockSet predecessors; //!< Incoming blocks
    BlockSet successors;   //!< Outgoing blocks
    Instruction *first;    //!< First instruction in the block
    Instruction *last;     //!< Last instruction in the block
    Function &fn;          //!< Function the block belongs to
    GBE_CLASS(BasicBlock);
  };

  /*! In fine, function inputs (arguments) can be pushed from the constant
   *  buffer if they are structures. Other arguments can be images (textures)
   *  and will also require special treatment.
   */
  struct FunctionInput
  {
    enum Type
    {
      GLOBAL_POINTER    = 0, /* __global */
      CONSTANT_POINTER  = 1, /* __constant */
      LOCAL_POINTER     = 2, /* __local */
      VALUE             = 3, /* int, float */
      STRUCTURE         = 4  /* struct foo */
    };
    /*! Create a function input */
    INLINE FunctionInput(Type type, Register reg, uint32_t elementSize = 0u) :
      type(type), reg(reg), elementSize(elementSize) {}
    Type type;            /*! Gives the type of argument we have */
    Register reg;         /*! Holds the argument */
    uint32_t elementSize; /*! Only for structure arguments */
  };

  /*! A function is no more that a set of declared registers and a set of
   *  basic blocks
   */
  class Function : public NonCopyable
  {
  public:
    /*! Create an empty function */
    Function(const std::string &name, Profile profile = PROFILE_OCL);
    /*! Release everything *including* the basic block pointers */
    ~Function(void);
    /*! Says if this is the top basic block (entry point) */
    INLINE bool isEntryBlock(const BasicBlock &bb) const {
      if (this->blockNum() == 0)
        return false;
      else
        return &bb == this->blocks[0];
    }
    /*! Get the function profile */
    INLINE Profile getProfile(void) const { return profile; }
    /*! Get a new valid register */
    INLINE Register newRegister(RegisterFamily family) {
      return this->file.append(family);
    }
    /*! Get the function name */
    const std::string &getName(void) const { return name; }
    /*! Extract the register from the register file */
    INLINE RegisterData getRegisterData(Register ID) const { return file.get(ID); }
    /*! Get the register family from the register itself */
    INLINE RegisterFamily getRegisterFamiy(Register ID) const {
      return this->getRegisterData(ID).family;
    }
    /*! Get the register index from the tuple vector */
    INLINE Register getRegister(Tuple ID, uint32_t which) const {
      return file.get(ID, which);
    }
    /*! Get the register file */
    INLINE const RegisterFile &getRegisterFile(void) const { return file; }
    /*! Get the given value ie immediate from the function */
    INLINE Immediate getImmediate(ImmediateIndex ID) const {
      GBE_ASSERT(ID < immediateNum());
      return immediates[ID];
    }
    /*! Create a new immediate and returns its index */
    INLINE ImmediateIndex newImmediate(const Immediate &imm) {
      const ImmediateIndex index(this->immediateNum());
      this->immediates.push_back(imm);
      return index;
    }
    /*! Fast allocation / deallocation of instructions */
    DECL_POOL(Instruction, insnPool);
    /*! Get input argument */
    INLINE const FunctionInput &getInput(uint32_t ID) const {
      GBE_ASSERT(ID < inputNum() && inputs[ID] != NULL);
      return *inputs[ID];
    }
    /*! Get input argument from the register (linear research). Return NULL if
     *  this is not an input argument
     */
    INLINE const FunctionInput *getInput(const Register &reg) const {
      for (auto it = inputs.begin(); it != inputs.end(); ++it)
        if ((*it)->reg == reg) return *it;
      return NULL;
    }
    /*! Get output register */
    INLINE Register getOutput(uint32_t ID) const {
      GBE_ASSERT(ID < outputNum());
      return outputs[ID];
    }
    /*! Get block ID */
    INLINE const BasicBlock &getBlock(uint32_t ID) const {
      GBE_ASSERT(ID < blockNum());
      GBE_ASSERT(blocks[ID] != NULL);
      return *blocks[ID];
    }
    /*! Get the first index of the special registers and number of them */
    uint32_t getFirstSpecialReg(void) const;
    uint32_t getSpecialRegNum(void) const;
    /*! Indicate if the given register is a special one */
    INLINE bool isSpecialReg(const Register &reg) const {
      const uint32_t ID = uint32_t(reg);
      const uint32_t firstID = this->getFirstSpecialReg();
      const uint32_t specialNum = this->getSpecialRegNum();
      return ID >= firstID && ID < firstID + specialNum;
    }
    /*! Create a new label (still not bound to a basic block) */
    LabelIndex newLabel(void);
    /*! Create the control flow graph */
    void computeCFG(void);
    /*! Number of registers in the register file */
    INLINE uint32_t regNum(void) const { return file.regNum(); }
    /*! Number of register tuples in the register file */
    INLINE uint32_t tupleNum(void) const { return file.tupleNum(); }
    /*! Number of labels in the function */
    INLINE uint32_t labelNum(void) const { return labels.size(); }
    /*! Number of immediate values in the function */
    INLINE uint32_t immediateNum(void) const { return immediates.size(); }
    /*! Get the number of input register */
    INLINE uint32_t inputNum(void) const { return inputs.size(); }
    /*! Get the number of output register */
    INLINE uint32_t outputNum(void) const { return outputs.size(); }
    /*! Number of blocks in the function */
    INLINE uint32_t blockNum(void) const { return blocks.size(); }
    /*! Output an immediate value in a stream */
    void outImmediate(std::ostream &out, ImmediateIndex index) const;
    /*! Apply the given functor on all basic blocks */
    template <typename T>
    INLINE void foreachBlock(const T &functor) const {
      for (auto it = blocks.begin(); it != blocks.end(); ++it)
        functor(**it);
    }
    /*! Apply the given functor on all instructions */
    template <typename T>
    INLINE void foreachInstruction(const T &functor) const {
      for (auto it = blocks.begin(); it != blocks.end(); ++it)
        (*it)->foreach(functor);
    }

  private:
    friend class Context;         //!< Can freely modify a function
    std::string name;             //!< Function name
    vector<FunctionInput*> inputs;//!< Input registers of the function
    vector<Register> outputs;     //!< Output registers of the function
    vector<BasicBlock*> labels;   //!< Each label points to a basic block
    vector<Immediate> immediates; //!< All immediate values in the function
    vector<BasicBlock*> blocks;   //!< All chained basic blocks
    RegisterFile file;            //!< RegisterDatas used by the instructions
    Profile profile;              //!< Current function profile
    GBE_CLASS(Function);          //!< Use gbe allocators
  };

  /*! Output the function string in the given stream */
  std::ostream &operator<< (std::ostream &out, const Function &fn);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_FUNCTION_HPP__ */

