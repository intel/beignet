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

#ifndef __GBE_IR_FUNCTION_HPP__
#define __GBE_IR_FUNCTION_HPP__

#include "ir_value.hpp"
#include "ir_register.hpp"
#include "ir_instruction.hpp"
#include "sys/vector.hpp"

namespace gbe
{
  /*! A function is no more that a set of declared registers and a set of
   *  basic blocks
   */
  class Function
  {
  public:
    /*! Create an empty function */
    Function(void);

    /*! Function basic blocks really belong to a function since:
     * 1 - registers used in the basic blocks belongs to the function register
     * file
     * 2 - branches point to basic blocks of the same function
     */
    class BasicBlock
    {
    public:
      /*! Empty basic block */
      BasicBlock(void);
      /*! Return the number of instruction in the block */
      INLINE uint32 getInsnNum(void) { return insn.size(); }
    private:
      vector<Instruction> insn; //!< Sequence of instructions in the block
    };

    /*! Extract the register from the register file */
    INLINE Register getRegister(uint32 ID) const { return file.get(ID); }
    /*! Get the register index from the tuple vector */
    INLINE RegisterIndex getRegisterIndex(TupleIndex ID, uint32 which) const {
      return file.get(ID, which);
    }
    /*! Get the given value ie immediate from the function */
    INLINE Value getValue(uint32 ID) const {
      assert(ID < value.size());
      return value[ID];
    }

  private:
    vector<uint16> input;    //!< Input registers of the function
    vector<uint16> output;   //!< Output registers of the function
    vector<Value> value;     //!< All immediate values stored in the function
    vector<BasicBlock> insn; //!< All the basic blocks one after the others
    RegisterFile file;       //!< All the registers used in the instructions
  };

} /* namespace gbe */

#endif /* __GBE_IR_FUNCTION_HPP__ */

