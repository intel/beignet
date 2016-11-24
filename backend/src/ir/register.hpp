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
 * \file register.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_REGISTER_HPP__
#define __GBE_IR_REGISTER_HPP__

#include "sys/vector.hpp"
#include "sys/platform.hpp"
#include "../backend/program.h"

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
    FAMILY_QWORD = 4,
    FAMILY_OWORD = 5,
    FAMILY_HWORD = 6,
    FAMILY_REG   = 7
  };

  INLINE char getFamilyName(RegisterFamily family) {
    static char registerFamilyName[] = {'b', 'B', 'W', 'D', 'Q', 'O', 'H', 'R'};
    return registerFamilyName[family];
  }

  INLINE uint32_t getFamilySize(RegisterFamily family) {
    switch (family) {
      case FAMILY_BYTE: return 1;
      case FAMILY_WORD: return 2;
      case FAMILY_DWORD: return 4;
      case FAMILY_QWORD: return 8;
      case FAMILY_REG: return 32;
      default: NOT_SUPPORTED;
    };
    return 0;
  }

  enum ARFRegister {
    ARF_NULL = 0,
    ARF_ADDRESS,
    ARF_ACCUMULATOR,
    ARF_FLAG,
    ARF_MASK,
    ARF_MASK_STACK,
    ARF_MASK_STACK_DEPTH,
    ARF_STATE,
    ARF_CONTROL,
    ARF_NOTIFICATION_COUNT,
    ARF_IP,
    ARF_TM
  };

  /*! Register is the position of the index of the register data in the register
   *  file. We enforce type safety with this class
   */
  TYPE_SAFE(Register, uint32_t)

  /*! A register can be either a byte, a word, a dword or a qword. We store this
   *  value into a register data (which makes the register file) 
   */
  class RegisterData
  {
  public:
    struct PayloadRegisterData {
      gbe_curbe_type  curbeType;
      int subType;
    };

    /*! Build a register. All fields will be immutable */
    INLINE RegisterData(RegisterFamily family,
                        bool uniform,
                        gbe_curbe_type curbeType,
                        int subType) : family(family), uniform(uniform) {
      payloadData.curbeType = curbeType;
      payloadData.subType = subType;
    }

    /*! Copy constructor */
    INLINE RegisterData(const RegisterData &other) : family(other.family), uniform(other.uniform), payloadData(other.payloadData) {}
    /*! Copy operator */
    INLINE RegisterData &operator= (const RegisterData &other) {
      this->family = other.family;
      this->uniform = other.uniform;
      this->payloadData = other.payloadData;
      return *this;
    }
    /*! Nothing really happens here */
    INLINE ~RegisterData(void) {}
    RegisterFamily family;            //!< Register size or if it is a flag
    INLINE bool isUniform() const { return uniform; }
    INLINE void setUniform(bool uni) { uniform = uni; }
    INLINE void setPayloadType(gbe_curbe_type curbeType, int subType) {
      payloadData.curbeType = curbeType;
      payloadData.subType = subType;
    }
    INLINE void getPayloadType(gbe_curbe_type &curbeType, int &subType) const {
      curbeType = payloadData.curbeType;
      subType = payloadData.subType;
    }
    INLINE bool isPayloadType(void) const {
      return payloadData.curbeType != GBE_GEN_REG;
    }
  private:
    bool uniform;
    PayloadRegisterData payloadData;
    GBE_CLASS(RegisterData);
  };

  /*! Output the register file string in the given stream */
  std::ostream &operator<< (std::ostream &out, const RegisterData &regData);

  INLINE bool operator< (const Register &r0, const Register &r1) {
    return r0.value() < r1.value();
  }

  /*! Tuple is the position of the first register in the tuple vector. We
   *  enforce type safety with this class
   */
  TYPE_SAFE(Tuple, uint32_t)

  /*! A register file allocates and destroys registers. Basically, we will have
   *  one register file per function
   */
  class RegisterFile
  {
  public:
    /*! Return the index of a newly allocated register */
    INLINE Register append(RegisterFamily family,
                           bool uniform = false,
                           gbe_curbe_type curbeType = GBE_GEN_REG,
                           int subType = 0) {
      GBE_ASSERTM((uint64_t)regNum() < MAX_INDEX,
                  "Too many defined registers (only 4G are supported)");
      const uint32_t index = regNum();
      const RegisterData reg(family, uniform, curbeType, subType);
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
    /*! Make a tuple from an array of Type */
    Tuple appendArrayTypeTuple(const uint8_t *types, uint32_t num);
    /*! Make a tuple and return the index to the first element of the tuple */
    template <typename First, typename... Rest>
    INLINE Tuple appendTypeTuple(First first, Rest... rest) {
      const Tuple index = Tuple(typeTuples.size());
      typeTuples.push_back(first);
      appendTuple(rest...);
      return index;
    }
    /*! To terminate variadic recursion */
    INLINE void appendTypeTuple(void) {}
    /*! Return a copy of the register at index */
    INLINE RegisterData get(Register index) const { return regs[index]; }
    /*! Return true if the specified register is uniform type. */
    INLINE bool isUniform(Register index) { return regs[index].isUniform(); }
    /*! Set a register to uniform or varying data type*/
    INLINE void setUniform(Register index, bool uniform) { regs[index].setUniform(uniform); }
    /*! Set payload type of a register */
    INLINE void setPayloadType(Register index, gbe_curbe_type curbeType, int subType) {
      regs[index].setPayloadType(curbeType, subType);
    }
    /*! Get payload type of a register */
    INLINE void getPayloadType(Register index, gbe_curbe_type &curbeType, int &subType) const {
      regs[index].getPayloadType(curbeType, subType);
    }
    /*! Check whether the register is a payload register */
    INLINE bool isPayloadReg(Register index) const {
      return regs[index].isPayloadType();
    }
    /*! Get the register index from the tuple */
    INLINE Register get(Tuple index, uint32_t which) const {
      return regTuples[index.value() + which];
    }
    /*! Set the register index from the tuple */
    INLINE void set(Tuple index, uint32_t which, Register reg) {
      regTuples[index.value() + which] = reg;
    }
    /*! Get the type from the tuple */
    INLINE uint8_t getType(Tuple index, uint32_t which) const {
      return typeTuples[index.value() + which];
    }
    /*! Set the type to the tuple */
    INLINE void setType(Tuple index, uint32_t which, uint8_t type) {
      typeTuples[index.value() + which] = type;
    }
    /*! Number of registers in the register file */
    INLINE uint32_t regNum(void) const { return regs.size(); }
    /*! Number of tuples in the register file */
    INLINE uint32_t tupleNum(void) const { return regTuples.size(); }
    /*! register and tuple indices are short */
    enum { MAX_INDEX = 0xffffffff };
  private:
    vector<RegisterData> regs;   //!< All the registers together
    vector<Register> regTuples;  //!< Tuples are used for many src / dst
    vector<uint8_t> typeTuples;  //!< Tuples are used for one instruction has multi src/dst types.
    GBE_CLASS(RegisterFile);
  };

  /*! Output the register file string in the given stream */
  std::ostream &operator<< (std::ostream &out, const RegisterFile &file);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_REGISTER_HPP__ */

