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
      DEF_FN_ARG = 0,
      DEF_FN_PUSHED = 1,
      DEF_INSN_DST = 2,
      DEF_SPECIAL_REG = 3
    };
    /*! Build a value from an instruction destination */
    explicit ValueDef(const Instruction *insn, uint32_t dstID = 0u) :
      type(DEF_INSN_DST)
    {
      this->data.insn = insn;
      this->data.dstID = dstID;
    }
    /*! Build a value from a function argument */
    explicit ValueDef(const FunctionArgument *arg) : type(DEF_FN_ARG) {
      this->data.arg = arg;
    }
    /*! Build a value from a pushed register */
    explicit ValueDef(const PushLocation *pushed) : type(DEF_FN_PUSHED) {
      this->data.pushed = pushed;
    }
    /*! Build a value from a special register */
    explicit ValueDef(const Register &reg) : type(DEF_SPECIAL_REG) {
      this->data.regID = uint32_t(reg);
    }
    /*! Get the type of the value */
    INLINE Type getType(void) const { return type; }
    /*! Get the instruction (only if this is a instruction value) */
    INLINE const Instruction *getInstruction(void) const {
      GBE_ASSERT(type == DEF_INSN_DST);
      return data.insn;
    }
    /*! Get the destination ID (only if this is a instruction value) */
    INLINE uint32_t getDstID(void) const {
      GBE_ASSERT(type == DEF_INSN_DST);
      return data.dstID;
    }
    /*! Get the function input (only if this is a function argument) */
    INLINE const FunctionArgument *getFunctionArgument(void) const {
      GBE_ASSERT(type == DEF_FN_ARG);
      return data.arg;
    }
    /*! Get the pushed location */
    INLINE const PushLocation *getPushLocation(void) const {
      GBE_ASSERT(type == DEF_FN_PUSHED);
      return data.pushed;
    }
    /*! Get the special register */
    INLINE Register getSpecialReg(void) const {
      GBE_ASSERT(type == DEF_SPECIAL_REG);
      return Register(data.regID);
    }
    /*! Retrieve the register associated to the definition */
    INLINE Register getRegister(void) const {
      if (type == DEF_SPECIAL_REG)
        return Register(data.regID);
      else if (type == DEF_FN_ARG)
        return data.arg->reg;
      else if (type == DEF_FN_PUSHED)
        return data.pushed->getRegister();
      else
        return data.insn->getDst(data.dstID);
    }

  private:
    /*! Instruction or function argument */
    union Data {
      /*! Instruction destination or ... */
      struct {
        const Instruction *insn; //<! Instruction itself
        uint32_t dstID;          //<! Which destination we take into account
      };
      /*! Pushed value */
      const PushLocation *pushed;
      /*! ... function argument or ... */
      const FunctionArgument *arg;
      /*! ... special register */
      uint32_t regID;
    } data;
    /*!< Function argument or instruction dst? */
    Type type;
    GBE_CLASS(ValueDef); // Use gbe allocators
  };

  /*! Compare two value definitions (used in maps) */
  INLINE bool operator< (const ValueDef &def0, const ValueDef &def1) {
    const ValueDef::Type type0 = def0.getType();
    const ValueDef::Type type1 = def1.getType();
    if (type0 != type1) return uint32_t(type0) < uint32_t(type1);
    if (type0 == ValueDef::DEF_FN_ARG) {
      const FunctionArgument *in0 = def0.getFunctionArgument();
      const FunctionArgument *in1 = def1.getFunctionArgument();
      return uintptr_t(in0) < uintptr_t(in1);
    } else if (type0 == ValueDef::DEF_FN_PUSHED) {
      const PushLocation *pushed0 = def0.getPushLocation();
      const PushLocation *pushed1 = def1.getPushLocation();
      return uintptr_t(pushed0) < uintptr_t(pushed1);
    } else if (type0 == ValueDef::DEF_SPECIAL_REG) {
      const Register reg0 = def0.getSpecialReg();
      const Register reg1 = def1.getSpecialReg();
      return uint32_t(reg0) < uint32_t(reg1);
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
    explicit ValueUse(const Instruction *insn, uint32_t srcID = 0u) :
      insn(insn), srcID(srcID) {}
    /*! Get the instruction of the use */
    const Instruction *getInstruction(void) const { return insn; }
    /*! Get the source index for this use */
    uint32_t getSrcID(void) const { return srcID; }
    /*! Get the register for this use */
    Register getRegister(void) const { return insn->getSrc(srcID); }
  private:
    const Instruction *insn; //!< Instruction where the value is used
    uint32_t srcID;          //!< Index of the source in the instruction
    GBE_CLASS(ValueUse);     // Use gbe allocators
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
  typedef set<ValueUse*> UseSet;
  /*! All possible definitions for a use */
  typedef set<ValueDef*> DefSet;

  /*! Get the chains (in both directions) for the complete program. This data
   *  structure is unfortunately way too brutal. Using std::sets all over the
   *  place just burns a huge amount of memory. There is work to do to decrease
   *  the memory footprint
   */
  class FunctionDAG : public NonCopyable
  {
  public:
    /*! Build the complete DU/UD graphs for the program included in liveness */
    FunctionDAG(Liveness &liveness);
    /*! Free all the resources */
    ~FunctionDAG(void);
    /*! Get the du-chain for the definition */
    const UseSet &getUse(const ValueDef &def) const;
    /*! Get the du-chain for the given instruction and destination */
    const UseSet &getUse(const Instruction *insn, uint32_t dstID) const;
    /*! Get the du-chain for the given function input */
    const UseSet &getUse(const FunctionArgument *arg) const;
    /*! Get the du-chain for the given pushed location */
    const UseSet &getUse(const PushLocation *pushed) const;
    /*! Get the du-chain for the given special register */
    const UseSet &getUse(const Register &reg) const;
    /*! Get the ud-chain for the given use */
    const DefSet &getDef(const ValueUse &use) const;
    /*! Get the ud-chain for the instruction and source */
    const DefSet &getDef(const Instruction *insn, uint32_t srcID) const;
    /*! Get the pointer to the definition *as stored in the DAG* */
    const ValueDef *getDefAddress(const ValueDef &def) const;
    /*! Get the pointer to the definition *as stored in the DAG* */
    const ValueDef *getDefAddress(const PushLocation *pushed) const;
    /*! Get the pointer to the definition *as stored in the DAG* */
    const ValueDef *getDefAddress(const Instruction *insn, uint32_t dstID) const;
    /*! Get the pointer to the definition *as stored in the DAG* */
    const ValueDef *getDefAddress(const FunctionArgument *input) const;
    /*! Get the pointer to the definition *as stored in the DAG* */
    const ValueDef *getDefAddress(const Register &reg) const;
    /*! Get the pointer to the use *as stored in the DAG* */
    const ValueUse *getUseAddress(const Instruction *insn, uint32_t srcID) const;
    /*! Get the set of all uses for the register */
    const UseSet *getRegUse(const Register &reg) const;
    /*! Get the set of all definitions for the register */
    const DefSet *getRegDef(const Register &reg) const;
    /*! Get the function we have the graph for */
    INLINE const Function &getFunction(void) const { return fn; }
    /*! The DefSet for each definition use */
    typedef map<ValueUse, DefSet*> UDGraph;
    /*! The UseSet for each definition */
    typedef map<ValueDef, UseSet*> DUGraph;
  private:
    UDGraph udGraph;                   //!< All the UD chains
    DUGraph duGraph;                   //!< All the DU chains
    DefSet *udEmpty;                   //!< Void use set
    UseSet *duEmpty;                   //!< Void def set
    ValueDef *undefined;               //!< Undefined value
    map<ValueUse, ValueUse*> useName;  //!< Get the ValueUse pointer from the value
    map<ValueDef, ValueDef*> defName;  //!< Get the ValueDef pointer from the value
    map<Register, UseSet*> regUse;     //!< All uses of registers
    map<Register, DefSet*> regDef;     //!< All defs of registers
    DECL_POOL(ValueDef, valueDefPool); //!< Fast ValueDef allocation
    DECL_POOL(ValueUse, valueUsePool); //!< Fast ValueUse allocation
    DECL_POOL(DefSet, udChainPool);    //!< Fast DefSet allocation
    DECL_POOL(UseSet, duChainPool);    //!< Fast UseSet allocation
    const Function &fn;                //!< Function we are referring to
    GBE_CLASS(FunctionDAG);            //   Use internal allocators
  };

  /*! Pretty print of the function DAG */
  std::ostream &operator<< (std::ostream &out, const FunctionDAG &dag);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_VALUE_HPP__ */

