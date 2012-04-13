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
 * \file register.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_REGISTER_HPP__
#define __GBE_IR_REGISTER_HPP__

#include "sys/vector.hpp"
#include "sys/platform.hpp"

namespace gbe {
namespace ir {

  /*! Basically provides the size of the register */
  enum RegisterFamily : uint8_t {
    FAMILY_BOOL  = 0,
    FAMILY_BYTE  = 1,
    FAMILY_WORD  = 2,
    FAMILY_DWORD = 3,
    FAMILY_QWORD = 4
  };

  /*! A register can be either a byte, a word, a dword or a qword. We store this
   *  value into a register data (which makes the register file) 
   */
  class RegisterData
  {
  public:
    /*! Build a register. All fields will be immutable */
    INLINE RegisterData(RegisterFamily family = FAMILY_DWORD) : family(family) {}
    /*! Copy constructor */
    INLINE RegisterData(const RegisterData &other) : family(other.family) {}
    /*! Copy operator */
    INLINE RegisterData &operator= (const RegisterData &other) {
      this->family = other.family;
      return *this;
    }
    /*! Nothing really happens here */
    INLINE ~RegisterData(void) {}
    RegisterFamily family;
    GBE_CLASS(RegisterData);
  };

  /*! Output the register file string in the given stream */
  std::ostream &operator<< (std::ostream &out, const RegisterData &regData);

  /*! Register is the position of the index of the register data in the register
   *  file. We enforce type safety with this class
   */
  TYPE_SAFE(Register, uint16_t)
  INLINE bool operator< (const Register &r0, const Register &r1) {
    return r0.value() < r1.value();
  }

  /*! Tuple is the position of the first register in the tuple vector. We
   *  enforce type safety with this class
   */
  TYPE_SAFE(Tuple, uint16_t)

  /*! A register file allocates and destroys registers. Basically, we will have
   *  one register file per function
   */
  class RegisterFile
  {
  public:
    /*! Return the index of a newly allocated register */
    INLINE Register append(RegisterFamily family) {
      GBE_ASSERTM(regNum() <= MAX_INDEX,
                  "Too many defined registers (only 65536 are supported)");
      const uint16_t index = regNum();
      const RegisterData reg(family);
      regs.push_back(reg);
      return Register(index);
    }
    /*! Make a tuple from an array of register */
    Tuple appendArrayTuple(const Register *reg, uint32_t regNum);
    /*! Make a tuple and return the index to the first element of the tuple */
    template <typename First, typename... Rest>
    INLINE Tuple appendTuple(First first, Rest... rest) {
      const Tuple index = Tuple(regTuples.size());
      GBE_ASSERTM(first < regNum(), "Out-of-bound register");
      regTuples.push_back(first);
      appendTuple(rest...);
      return index;
    }
    /*! To terminate variadic recursion */
    INLINE void appendTuple(void) {}
    /*! Return a copy of the register at index */
    INLINE RegisterData get(Register index) const {
      GBE_ASSERTM(index < regNum(), "Out-of-bound register");
      return regs[index];
    }
    /*! Get the register index from the tuple */
    INLINE Register get(Tuple index, uint32_t which) const {
      GBE_ASSERTM(uint16_t(index) + which < regTuples.size(),
                  "Out-of-bound index in the tuple file");
      return regTuples[uint16_t(index) + which];
    }
    /*! Number of registers in the register file */
    INLINE uint32_t regNum(void) const { return regs.size(); }
    /*! Number of tuples in the register file */
    INLINE uint32_t tupleNum(void) const { return regTuples.size(); }
  private:
    vector<RegisterData> regs;   //!< All the registers together
    vector<Register> regTuples;  //!< Tuples are used for many src / dst
    enum { MAX_INDEX = 0xffff }; //!< register and tuple indices are short
    GBE_CLASS(RegisterFile);
  };

  /*! Output the register file string in the given stream */
  std::ostream &operator<< (std::ostream &out, const RegisterFile &file);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_REGISTER_HPP__ */



