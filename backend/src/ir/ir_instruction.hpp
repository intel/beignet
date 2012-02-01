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

#ifndef __GBE_IR_INSTRUCTION_HPP__
#define __GBE_IR_INSTRUCTION_HPP__

#include "sys/platform.hpp"
#include "ir_register.hpp"

namespace gbe
{
  /*! All opcodes */
  enum {
#define DECL_INSN(INSN, FAMILY) OP_##INSN,
#include "ir_instruction.hxx"
#undef DECL_INSN
    OP_INVALID
  };

  /*! Different memory spaces */
  enum {
    MEM_GLOBAL = 0, /*! Global memory (a la OCL) */
    MEM_LOCAL,      /*! Local memory (thread group memory) */
    MEM_PRIVATE     /*! Per thread private memory */
  };

  /*! All types possibly supported by the instruction */
  enum {
    TYPE_S8 = 0,  /*! signed 8 bits integer */
    TYPE_U8,      /*! unsigned 8 bits integer */
    TYPE_S16,     /*! signed 16 bits integer */
    TYPE_U16,     /*! unsigned 16 bits integer */
    TYPE_S32,     /*! signed 32 bits integer */
    TYPE_U32,     /*! unsigned 32 bits integer */
    TYPE_S64,     /*! signed 64 bits integer */
    TYPE_U64,     /*! unsigned 64 bits integer */
    TYPE_FLOAT,   /*! 32 bits floating point value */
    TYPE_DOUBLE   /*! 64 bits floating point value */
  };

  /*! Function class contains the register file and the register tuple. Any
   *  information related to the registers may therefore require a function
   */
  class Function;

  /*! Store the instruction description in 8 bytes */
  class Instruction
  {
  public:
    /*! Get the instruction opcode */
    uint32 getOpcode(void) const;
    /*! Get the number of sources for this instruction  */
    uint32 getSrcNum(void) const;
    /*! Get the number of destination for this instruction */
    uint32 getDstNum(void) const;
    /*! Get the register index of the given source */
    uint32 getSrcIndex(const Function &fn, uint32 ID) const;
    /*! Get the register index of the given destination */
    uint32 getDstIndex(const Function &fn, uint32 ID) const;
    /*! Get the register of the given source */
    Register getSrc(const Function &fn, uint32 ID) const;
    /*! Get the register of the given destination */
    Register getDst(const Function &fn, uint32 ID) const;
    /*! Check that the instruction is well formed. Return true if well formed.
     *  Otherwise, fill the string with a help message
     */
    bool check(void) const;
  protected:
    uint64 opaque; //!< Data depends on the instruction itself
  };

  /*! Binary instructions are typed. dst and sources share the same type */
  class BinaryInstruction : public Instruction
  {
  public:
    /*! Get the type manipulated by the instruction */
    uint32 getType(void) const;
  };

  /*! Ternary instructions is mostly for MADs */
  class TernaryInstruction : public Instruction
  {
  public:
    /*! Get the type manipulated by the instruction */
    uint32 getType(void) const;
  };

  /*! Conversion instruction converts from one type to another */
  class ConvertInstruction : public Instruction
  {
  public:
    /*! Get the type of the source */
    uint32 getSrcType(void) const;
    /*! Get the type of the destination */
    uint32 getDstType(void) const;
  };

  /*! Store instruction */
  class StoreInstruction : public Instruction
  {
  public:

  };

} /* namespace gbe */

#endif /* __GBE_IR_INSTRUCTION_HPP__ */

