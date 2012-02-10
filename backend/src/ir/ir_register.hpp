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
    /*! Register family */
    enum Family : uint8 {
      BOOL  = 0,
      BYTE  = 1,
      WORD  = 2,
      DWORD = 3,
      QWORD = 4
    };
    /*! Build a register. All fields will be immutable */
    INLINE Register(Family family = DWORD) : family(family) {}
    /*! Copy constructor */
    INLINE Register(const Register &other) : family(other.family) {}
    /*! Copy operator */
    INLINE Register &operator= (const Register &other) {
      this->family = other.family;
      return *this;
    }
    /*! Nothing really happens here */
    INLINE ~Register(void) {}
    Family family;
    GBE_CLASS(Register);
  };

  /*! Register index is the position of the register in the register file */
  typedef uint16 RegisterIndex;

  /*! Tuple index is the position of the register index in the tuple vector */
  typedef uint32 TupleIndex;

  /*! A register file allocates and destroys registers. Basically, we will have
   *  one register file per function
   */
  class RegisterFile
  {
  public:
    /*! Return the index of a newly allocated register register */
    INLINE RegisterIndex append(Register::Family family) {
      const uint32 index = regs.size();
      const Register reg(family);
      assert(index <= MAX_INDEX);
      regs.push_back(reg);
      return index;
    }
    /*! Make a tuple and return the index to the first element of the tuple */
    template <typename First, typename... Rest>
    INLINE TupleIndex appendTuple(First first, Rest... rest) {
      const TupleIndex index = regTuples.size();
      assert(first < regs.size());
      regTuples.push_back(first);
      appendTuple(rest...);
      return index;
    }
    /*! To terminate variadic recursion */
    INLINE void appendTuple(void) {}
    /*! Return a copy of the register at index */
    INLINE Register get(RegisterIndex index) const {
      assert(index < regs.size() && index <= MAX_INDEX);
      return regs[index];
    }
    /*! Get the register index from the tuple */
    INLINE RegisterIndex get(TupleIndex index, uint32 which) const {
      assert(index + which < regTuples.size());
      return regTuples[index + which];
    }
    /*! Number of registers in the register file */
    INLINE uint32 regNum(void) const { return regs.size(); }
  private:
    enum { MAX_INDEX = 0xffff };     //!< We encode indices in 2 bytes
    vector<Register> regs;           //!< All the registers together
    vector<RegisterIndex> regTuples; //!< Tuples are used for many src / dst
    GBE_CLASS(RegisterFile);
  };

} /* namespace gbe */

#endif /* __GBE_IR_REGISTER_HPP__ */

