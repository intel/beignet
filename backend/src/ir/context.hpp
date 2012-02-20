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
 * \file context.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_CONTEXT_HPP__
#define __GBE_IR_CONTEXT_HPP__

#include "ir/instruction.hpp"
#include "ir/function.hpp"
#include "ir/register.hpp"
#include "sys/vector.hpp"
#include <tuple>

namespace gbe {
namespace ir {

  // We compile a unit
  class Unit;

  /*! A context allows an easy creation of the functions (instruction stream and
   *  the set of immediates and registers needed for it) and constant arrays
   */
  class Context
  {
  public:
    /*! Create a new context for this unit */
    Context(Unit &unit);
    /*! Create a new function "name" */
    void startFunction(const std::string &name);
    /*! Close the function */
    void endFunction(void);
    /*! Create a new register for the given family */
    Register reg(RegisterData::Family family);
    /*! Append a new input register for the function */
    void input(Register reg);
    /*! Append a new output register for the function */
    void output(Register reg);
    /*! Append a new tuple */
    template <typename... Args> INLINE Tuple tuple(Args...args);
    /*! We just use variadic templates to forward instruction functions */
#define DECL_INSN(NAME, FAMILY)                                       \
    template <typename... Args> INLINE void NAME(Args...args);
#include "ir/instruction.hxx"
#undef DECL_INSN

    /*! MAD with sources directly specified */
    INLINE void MAD(Type type,
                    Register dst,
                    Register src0,
                    Register src1,
                    Register src2)
    {
      const Tuple index = this->tuple(src0, src1, src2);
      return this->MAD(type, dst, index);
    }

    /*! LOAD with the destinations directly specified */
    template <typename... Args>
    void LOAD(Type type, Register offset, MemorySpace space, Args...values)
    {
      const Tuple index = this->tuple(values...);
      const uint16_t valueNum = std::tuple_size<std::tuple<Args...>>::value;
      GBE_ASSERT(valueNum > 0);
      this->LOAD(type, index, offset, space, valueNum);
    }

    /*! STORE with the sources directly specified */
    template <typename... Args>
    void STORE(Type type, Register offset, MemorySpace space, Args...values)
    {
      const Tuple index = this->tuple(values...);
      const uint16_t valueNum = std::tuple_size<std::tuple<Args...>>::value;
      GBE_ASSERT(valueNum > 0);
      this->STORE(type, index, offset, space, valueNum);
    }

#define DECL_CMP(NAME)                              \
    void NAME(Type type,                            \
              Register dst,                         \
              Register src0,                        \
              Register src1)                        \
    {                                               \
      this->CMP(type, CMP_##NAME, dst, src0, src1); \
    }
DECL_CMP(EQ)
DECL_CMP(NE)
DECL_CMP(LT)
DECL_CMP(LE)
DECL_CMP(GT)
DECL_CMP(GE)
#undef DECL_CMP

  private:
    /*! A block must be started with a label */
    void startBlock(void);
    /*! A block must be ended with a branch */
    void endBlock(void);
    /*! Append the instruction in the current basic block */
    void append(const Instruction &insn);
    Unit &unit;               //!< A unit is associated to a contect
    Function *fn;             //!< Current function we are processing
    BasicBlock *bb;           //!< Current basic block we are filling
    vector<Function*> fnStack;//!< Stack of functions still to finish
    GBE_CLASS(Context);
  };

  template <typename... Args>
  INLINE Tuple Context::tuple(Args...args) {
    GBE_ASSERTM(fn != NULL, "No function currently defined");
    return fn->file.appendTuple(args...);
  }

  // Use argument checker to assert argument value correctness
#define DECL_INSN(NAME, FAMILY)                                   \
  template <typename... Args>                                     \
  INLINE void Context::NAME(Args...args) {                        \
    GBE_ASSERTM(fn != NULL, "No function currently defined");     \
    const Instruction insn = gbe::ir::NAME(args...);              \
    this->append(insn);                                           \
  }
#include "ir/instruction.hxx"
#undef DECL_INSN

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_CONTEXT_HPP__ */

