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

  /*! Defines the size of the pointers. All the functions from the unit will
   *  use the same pointer size as the unit they belong to
   */
  enum PointerSize {
    POINTER_32_BITS = 32,
    POINTER_64_BITS = 64
  };

  /*! Basically provides the size of the register */
  enum RegisterFamily : uint8_t {
    FAMILY_BOOL  = 0,
    FAMILY_BYTE  = 1,
    FAMILY_WORD  = 2,
    FAMILY_DWORD = 3,
    FAMILY_QWORD = 4
  };

  INLINE char getFamilyName(RegisterFamily family) {
    static char registerFamilyName[] = {'b', 'B', 'W', 'D', 'Q'};
    return registerFamilyName[family];
  }

  INLINE uint32_t getFamilySize(RegisterFamily family) {
    switch (family) {
      case FAMILY_BYTE: return 1;
      case FAMILY_WORD: return 2;
      case FAMILY_DWORD: return 4;
      case FAMILY_QWORD: return 8;
      default: NOT_SUPPORTED;
    };
    return 0;
  }

  /*! A register can be either a byte, a word, a dword or a qword. We store this
   *  value into a register data (which makes the register file) 
   */
  class RegisterData
  {
  public:
    /*! Build a register. All fields will be immutable */
    INLINE RegisterData(RegisterFamily family,
                        bool uniform = false) : family(family), uniform(uniform) {}
    /*! Copy constructor */
    INLINE RegisterData(const RegisterData &other) : family(other.family), uniform(other.uniform) {}
    /*! Copy operator */
    INLINE RegisterData &operator= (const RegisterData &other) {
      this->family = other.family;
      this->uniform = other.uniform;
      return *this;
    }
    /*! Nothing really happens here */
    INLINE ~RegisterData(void) {}
    RegisterFamily family;            //!< Register size or if it is a flag
    INLINE const bool isUniform() const { return uniform; }
    INLINE void setUniform(bool uni) { uniform = uni; }
  private:
    bool uniform;
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
    INLINE Register append(RegisterFamily family, bool uniform = false) {
      GBE_ASSERTM(regNum() < MAX_INDEX,
                  "Too many defined registers (only 65535 are supported)");
      const uint16_t index = regNum();
      const RegisterData reg(family, uniform);
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
    INLINE RegisterData get(Register index) const { return regs[index]; }
    /*! Return true if the specified register is uniform type. */
    INLINE bool isUniform(Register index) { return regs[index].isUniform(); }
    /*! Set a register to uniform or varying data type*/
    INLINE void setUniform(Register index, bool uniform) { regs[index].setUniform(uniform); }
    /*! Get the register index from the tuple */
    INLINE Register get(Tuple index, uint32_t which) const {
      return regTuples[uint16_t(index) + which];
    }
    /*! Set the register index from the tuple */
    INLINE void set(Tuple index, uint32_t which, Register reg) {
      regTuples[uint16_t(index) + which] = reg;
    }
    /*! Number of registers in the register file */
    INLINE uint32_t regNum(void) const { return regs.size(); }
    /*! Number of tuples in the register file */
    INLINE uint32_t tupleNum(void) const { return regTuples.size(); }
    /*! register and tuple indices are short */
    enum { MAX_INDEX = 0xffff }; 
  private:
    vector<RegisterData> regs;   //!< All the registers together
    vector<Register> regTuples;  //!< Tuples are used for many src / dst
    GBE_CLASS(RegisterFile);
  };

  /*! Output the register file string in the given stream */
  std::ostream &operator<< (std::ostream &out, const RegisterFile &file);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_REGISTER_HPP__ */

