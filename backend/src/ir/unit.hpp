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
 * \file unit.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_UNIT_HPP__
#define __GBE_IR_UNIT_HPP__

#include "ir/constant.hpp"
#include "ir/register.hpp"
#include "sys/hash_map.hpp"
#include "sys/map.hpp"

namespace gbe {
namespace ir {

  // A unit contains a set of functions
  class Function;

  /*! Complete unit of compilation. It contains a set of functions and a set of
   *  constant the functions may refer to.
   */
  class Unit : public NonCopyable
  {
  public:
    typedef hash_map<std::string, Function*> FunctionSet;
    /*! Create an empty unit */
    Unit(PointerSize pointerSize = POINTER_32_BITS);
    /*! Release everything (*including* the function pointers) */
    ~Unit(void);
    /*! Get the set of functions defined in the unit */
    const FunctionSet &getFunctionSet(void) const { return functions; }
    /*! Retrieve the function by its name */
    Function *getFunction(const std::string &name) const;
    /*! Return NULL if the function already exists */
    Function *newFunction(const std::string &name);
    /*! Create a new constant in the constant set */
    void newConstant(const char*, const std::string&, uint32_t size, uint32_t alignment);
    /*! Apply the given functor on all the functions */
    template <typename T>
    INLINE void apply(const T &functor) const {
      for (const auto &pair : functions) functor(*pair.second);
    }
    /*! Return the size of the pointers manipulated */
    INLINE PointerSize getPointerSize(void) const { return pointerSize; }
    /*! Return the family of registers that contain pointer */
    INLINE RegisterFamily getPointerFamily(void) const {
      if (this->getPointerSize() == POINTER_32_BITS)
        return FAMILY_DWORD;
      else
        return FAMILY_QWORD;
    }
    /*! Return the constant set */
    ConstantSet& getConstantSet(void) { return constantSet; }
    /*! Return the constant set */
    const ConstantSet& getConstantSet(void) const { return constantSet; }
    void setValid(bool value) { valid = value; }
    bool getValid() { return valid; }
  private:
    friend class ContextInterface; //!< Can free modify the unit
    hash_map<std::string, Function*> functions; //!< All the defined functions
    ConstantSet constantSet; //!< All the constants defined in the unit
    PointerSize pointerSize; //!< Size shared by all pointers
    GBE_CLASS(Unit);
    bool valid;
  };

  /*! Output the unit string in the given stream */
  std::ostream &operator<< (std::ostream &out, const Unit &unit);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_UNIT_HPP__ */
