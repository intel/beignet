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
 * \file constant.cpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_CONSTANT_HPP__
#define __GBE_IR_CONSTANT_HPP__

#include "sys/vector.hpp"

namespace gbe {
namespace ir {

  /*! Describe one constant (may be a scalar or an array) */
  class Constant
  {
  public:
    /*! Build a constant description */
    INLINE Constant(const std::string &name, uint32_t size, uint32_t alignment, uint32_t offset) :
      name(name), size(size), alignment(alignment), offset(offset) {}
    /*! Copy constructor */
    INLINE Constant(const Constant &other) :
      name(other.name), size(other.size), alignment(other.alignment), offset(other.offset) {}
    /*! Copy operator */
    INLINE Constant& operator= (const Constant &other) {
      this->name = other.name;
      this->size = other.size;
      this->alignment = other.alignment;
      this->offset = other.offset;
      return *this;
    }
    /*! Nothing happens here */
    INLINE ~Constant(void) {}
  private:
    std::string name; //!< Optional name of the constant
    uint32_t size;      //!< Size of the constant
    uint32_t alignment; //!< Alignment required for each constant
    uint32_t offset;    //!< Offset of the constant in the data segment
  };

  /*! A constant set is a set of immutable data associated to a compilation
   *  unit
   */
  class ConstantSet
  {
  public:
    /*! Append a new constant in the constant set */
    void append(const char*, const std::string&, uint32_t size, uint32_t alignment);
  private:
    vector<char> data;         //!< The constant data serialized in one array
    vector<Constant> constants;//!< Each constant description
  };

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_CONSTANT_HPP__ */



