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
 * \file value.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_VALUE_HPP__
#define __GBE_IR_VALUE_HPP__

#include "ir/instruction.hpp"
#include "ir/function.hpp"
#include "sys/set.hpp"
#include "sys/map.hpp"

namespace gbe {
namespace ir {

  // Make UD-Chain and DU-Chain computations faster and easier
  class Liveness;

  /*! A value definition is a destination of an instruction or a function
   *  argument. Since we support multiple destinations, we also add the
   *  destination ID.
   */
  class ValueDef
  {
  public:
    /*! Discriminates the kind of values */
    enum Type : uint32_t {
      FUNCTION_INPUT = 0,
      INSTRUCTION_DST = 1
    };
    /*! Build a value from an instruction destination */
    ValueDef(const Instruction *insn, uint32_t dstID = 0u) :
      type(INSTRUCTION_DST)
    {
      this->data.insn = insn;
      this->data.dstID = dstID;
    }
    /*! Build a value from a function argument */
    ValueDef(const FunctionInput *input) : type(FUNCTION_INPUT) {
      this->data.input = input;
    }
    /*! Get the type of the value */
    INLINE Type getType(void) const { return type; }
    /*! Get the instruction (only if this is a instruction value) */
    INLINE const Instruction *getInstruction(void) const {
      GBE_ASSERT(type == INSTRUCTION_DST);
      return data.insn;
    }
    /*! Get the destination ID (only if this is a instruction value) */
    INLINE uint32_t getDstID(void) const {
      GBE_ASSERT(type == INSTRUCTION_DST);
      return data.dstID;
    }
    /*! Get the function input (only if this is a function argument) */
    INLINE const FunctionInput *getFunctionInput(void) const {
      GBE_ASSERT(type == FUNCTION_INPUT);
      return data.input;
    }

  private:
    /*! Instruction or function argument */
    union Data {
      /*! Instruction destination or ... */
      struct {
        const Instruction *insn; //<! Instruction itself
        uint32_t dstID;          //<! Which destination we take into account
      };
      /*! ... function argument */
      const FunctionInput *input;
    } data;
    /*!< Function argument or instruction dst? */
    Type type;
  };

  /*! Compare two value definitions (used in maps) */
  INLINE bool operator< (const ValueDef &def0, const ValueDef &def1) {
    const ValueDef::Type type0 = def0.getType();
    const ValueDef::Type type1 = def1.getType();
    if (type0 != type1) return uint32_t(type0) < uint32_t(type1);
    if (type0 == ValueDef::FUNCTION_INPUT) {
      const FunctionInput *in0 = def0.getFunctionInput();
      const FunctionInput *in1 = def1.getFunctionInput();
      return uintptr_t(in0) < uintptr_t(in1);
    } else {
      const Instruction *insn0 = def0.getInstruction();
      const Instruction *insn1 = def1.getInstruction();
      if (insn0 != insn1) return uintptr_t(insn0) < uintptr_t(insn1);
      const uint32_t dst0 = def0.getDstID();
      const uint32_t dst1 = def1.getDstID();
      return dst0 < dst1;
    }
  }

  /*! A value use describes a instruction source. This is the place where a
   *  value is used
   */
  class ValueUse
  {
  public:
    /*! Build a value use */
    ValueUse(const Instruction *insn, uint32_t srcID = 0u) :
      insn(insn), srcID(srcID) {}
    /*! Get the instruction of the use */
    const Instruction *getInstruction(void) const { return insn; }
    /*! Get the source index for this use */
    uint32_t getSrcID(void) const { return srcID; }
  private:
    const Instruction *insn; //!< Instruction where the value is used
    uint32_t srcID;          //!< Index of the source in the instruction
  };

  /*! Compare two value uses (used in maps) */
  INLINE bool operator< (const ValueUse &use0, const ValueUse &use1) {
    const Instruction *insn0 = use0.getInstruction();
    const Instruction *insn1 = use1.getInstruction();
    if (insn0 != insn1) return uintptr_t(insn0) < uintptr_t(insn1);
    const uint32_t src0 = use0.getSrcID();
    const uint32_t src1 = use1.getSrcID();
    return src0 < src1;
  }

  /*! All uses of a definition */
  typedef set<ValueUse*> DUChain;
  /*! All possible definitions for a use */
  typedef set<ValueDef*> UDChain;

  /*! Get the chains (in both directions) for the complete program */
  class FunctionDAG
  {
  public:
    /*! Build the complete DU/UD graphs for the program included in liveness */
    FunctionDAG(Liveness &liveness);
    /*! Free all the resources */
    ~FunctionDAG(void);
    /*! Get the du-chain for the given instruction and destination */
    const DUChain &getUse(const Instruction *insn, uint32_t dstID) const;
    /*! Get the du-chain for the given function input */
    const DUChain &getUse(const FunctionInput *input) const;
    /*! Get the ud-chain for the instruction and source */
    const UDChain &getDef(const Instruction *insn, uint32_t srcID) const;
    /*! Get the pointer to the definition *as stored in the DAG* */
    const ValueDef *getDefAddress(const Instruction *insn, uint32_t dstID) const;
    /*! Get the pointer to the definition *as stored in the DAG* */
    const ValueDef *getDefAddress(const FunctionInput *input) const;
    /*! Get the pointer to the use *as stored in the DAG* */
    const ValueUse *getUseAddress(const Instruction *insn, uint32_t srcID) const;
    /*! The UDChain for each definition use */
    typedef map<ValueUse, UDChain*> UDGraph;
    /*! The DUChain for each definition */
    typedef map<ValueDef, DUChain*> DUGraph;
  private:
    UDGraph udGraph;                   //!< All the UD chains
    DUGraph duGraph;                   //!< All the DU chains
    UDChain *udEmpty;                  //!< Void use set
    DUChain *duEmpty;                  //!< Void def set
    map<ValueUse, ValueUse*> useName;  //!< Get the ValueUse pointer from the value
    map<ValueDef, ValueDef*> defName;  //!< Get the ValueDef pointer from the value
    DECL_POOL(ValueDef, valueDefPool); //!< Fast ValueDef allocation
    DECL_POOL(ValueUse, valueUsePool); //!< Fast ValueUse allocation
    DECL_POOL(UDChain, udChainPool);   //!< Fast UDChain allocation
    DECL_POOL(DUChain, duChainPool);   //!< Fast DUChain allocation
    GBE_CLASS(FunctionDAG);            //!< Use internal allocators
  };

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_VALUE_HPP__ */

