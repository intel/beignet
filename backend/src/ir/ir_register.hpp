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

#include "sys/ref.hpp"
#include <new>

namespace gbe
{
  /*! Describe a register in GBE register file. Each register can be either a
   *  boolean value or a {8|16|32|64} bit word. Also, each register contains some
   *  number of lanes that describes its SIMD width
   */
  class Register : public RefCount, public NonCopyable
  {
  public:
    /*! Family basically gives the size of each register lane */
    enum Family
    {
      BOOL  = 0,
      BYTE  = 1,
      WORD  = 2,
      DWORD = 3,
      QWORD = 4
    };
    friend class RegisterFile; // Allocates and destroys registers
    /*! Get the register identifier */
    INLINE uint32 id(void)     const { return this->_id; }
    /*! Get the register width (ie number of lanes) */
    INLINE uint16 width(void)  const { return this->_width; }
    /*! Get the register family */
    INLINE Family family(void) const { return this->_family; }
    /*! Get the register *individual* lane size */
    INLINE uint8  size(void)   const { return this->_size; }
  private:
    /*! */
    INLINE Register(uint32 id, Family family, uint8 width) :
      _id(id), _width(width), _family(family), _size(familySize[family]) {}
    INLINE ~Register(void) {}
    const uint32 _id;     //!< Uniquely identifies the register
    const uint16 _width;  //!< Number of lane in the vector
    const Family _family; //!< From Family enum
    const uint8 _size;    //!< Directly related to its type
    /*! Gives the size for each family */
    static const uint8 familySize[];
    GBE_CLASS(Register);
  };

  /*! Registers are equal if they have same width, ID and family */
  INLINE bool operator== (const Register &a, const Register &b) {
    return a.id() == b.id() &&
           a.width() == b.width() &&
           a.family() == b.family();
  }

  /*! != is just "not equal" */
  INLINE bool operator!= (const Register &a, const Register &b) {
    return !(a == b);
  }

  /*! To sort registers */
  INLINE bool operator< (const Register &a, const Register &b) {
    if (a.family() != b.family()) return a.family() < b.family();
    if (a.width() != b.width()) return a.width() < b.width();
    if (a.id() != b.id()) return a.id() < b.id();
    return false;
  }

  /*! A register file allocates and destroys registers. Basically, we will have
   *  one register file per function
   */
  class RegisterFile : public RefCount
  {
  public:
    /*! Destroy the register file */
    virtual ~RegisterFile(void) = 0;
    /*! Allocate and return a valid register */
    virtual Register *create(Register::Family family, uint8 width) = 0;
    /*! Get the register with index id */
    virtual Register *get(uint32 id) = 0;
    /*! Deallocate the register */
    virtual void destroy(Register *reg) = 0;
  };

  /*! Allocate a new register file */
  RegisterFile *RegisterFileNew(void);

} /* namespace gbe */

#endif /* __GBE_IR_REGISTER_HPP__ */

