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

#ifndef __GBE_IR_REGISTER_HPP__
#define __GBE_IR_REGISTER_HPP__

#include "sys/vector.hpp"

namespace gbe
{
  /*! A register can be either a byte, a word, a dword or a qword. It can be
   *  either scalar or a vector. Everything is packed in 32 bits
   */
  class Register
  {
  public:
    /*! Build a register. All fields will be immutable */
    INLINE Register(uint8 type) : type(type) {}
    /*! Copy constructor */
    INLINE Register(const Register &other) : type(other.type) {}
    /*! Copy operator */
    Register &operator= (const Register &other) {
      *this = Register(other.type);
      return *this;
    }
    /*! Nothing really happens here */
    INLINE ~Register(void) {}
    /*! Register type */
    enum {
      BOOL  = 0,
      BYTE  = 1,
      WORD  = 2,
      DWORD = 3,
      QWORD = 4
    };
    const uint8 type;
    GBE_CLASS(Register);
  };

  /*! A register file allocates and destroys registers. Basically, we will have
   *  one register file per function
   */
  class RegisterFile
  {
  public:
    /*! Return the index of a newly allocated register register */
    INLINE uint32 append(uint32 type) {
      const uint32 index = regs.size();
      const Register reg(type);
      assert(index <= MAX_INDEX);
      regs.push_back(reg);
      return index;
    }
    /*! Return a copy of the register at index */
    INLINE Register get(uint32 index) const {
      assert(index < regs.size() && index <= MAX_INDEX);
      return regs[index];
    }
    /*! Number of registers in the register file */
    INLINE uint32 regNum(void) const { return regs.size(); }
  private:
    enum { MAX_INDEX = 0xffff }; //!< We encode indices in 2 bytes
    vector<Register> regs;       //!< All the registers together
    GBE_CLASS(RegisterFile);
  };

} /* namespace gbe */

#endif /* __GBE_IR_REGISTER_HPP__ */

