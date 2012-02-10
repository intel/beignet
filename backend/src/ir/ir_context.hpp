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

#ifndef __GBE_IR_CONTEXT_HPP__
#define __GBE_IR_CONTEXT_HPP__

#include "ir_instruction.hpp"
#include "ir_function.hpp"
#include "ir_register.hpp"
#include "sys/vector.hpp"

namespace gbe
{
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
    /*! Create a new register for the given type */
    RegisterIndex reg(Register::Family type);
    /*! Append a new input register for the function */
    void input(RegisterIndex reg);
    /*! Append a new output register for the function */
    void output(RegisterIndex reg);
    /*! Append a new tuple */
    template <typename... Args>
    INLINE TupleIndex tuple(Args...args) {
      if (UNLIKELY(fn == NULL))
        throw std::exception("Tuple not defined in a function");
      fn->file.append(args...);
    }
  private:
    Unit &unit;               //!< A unit is associated to a contect
    Function *fn;             //!< Current function we are processing
    Function::BasicBlock *bb; //!< Current basic block we are filling
    vector<Function*> fnStack;//!< Stack of functions still to finish
  };

} /* namespace gbe */

#endif /* __GBE_IR_CONTEXT_HPP__ */

