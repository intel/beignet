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
#include "ir/profiling.hpp"
#include "ir/printf.hpp"
#include "ir/reloc.hpp"
#include "sys/map.hpp"
#include <string.h>

#include "llvm/IR/Instructions.h"

namespace gbe {
namespace ir {

  // A unit contains a set of functions
  class Function;
  class ProfilingInfo;

  class Unit : public NonCopyable
  {
  public:
    typedef map<std::string, Function*> FunctionSet;
    /*! Moved from printf pass */
    map<llvm::CallInst*, PrintfSet::PrintfFmt*> printfs;
    vector<std::string> blockFuncs;
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
    void newConstant(const std::string&, uint32_t size, uint32_t alignment);
    /*! Apply the given functor on all the functions */
    template <typename T>
    INLINE void apply(const T &functor) const {
      for (FunctionSet::const_iterator it = functions.begin(); it != functions.end(); ++it)
        functor(*(it->second));
    }
    /*! Return the size of the pointers manipulated */
    INLINE PointerSize getPointerSize(void) const { return pointerSize; }
    INLINE void setPointerSize(PointerSize size) { pointerSize = size; }
    /*! Return the family of registers that contain pointer */
    INLINE RegisterFamily getPointerFamily(void) const {
      if (this->getPointerSize() == POINTER_32_BITS)
        return FAMILY_DWORD;
      else
        return FAMILY_QWORD;
    }
    /*! Return the constant set */
    ConstantSet& getConstantSet(void) { return constantSet; }
    const RelocTable& getRelocTable(void) const  { return relocTable; }
    RelocTable& getRelocTable(void)   { return relocTable; }
    /*! Return the constant set */
    const ConstantSet& getConstantSet(void) const { return constantSet; }
    /*! Get profiling info in this function */
    ProfilingInfo* getProfilingInfo(void) const { return profilingInfo; }
    /*! Set in profiling mode */
    void setInProfilingMode(bool b) { inProfilingMode = b; }
    /*! Get in profiling mode */
    bool getInProfilingMode(void) const { return inProfilingMode; }
    void setValid(bool value) { valid = value; }
    bool getValid() { return valid; }
    void setOclVersion(uint32_t version) { oclVersion = version; }
    uint32_t getOclVersion() const { return oclVersion; }
  private:
    friend class ContextInterface; //!< Can free modify the unit
    FunctionSet functions; //!< All the defined functions
    ConstantSet constantSet; //!< All the constants defined in the unit
    RelocTable relocTable;
    PointerSize pointerSize; //!< Size shared by all pointers
    ProfilingInfo *profilingInfo; //!< profilingInfo store the information for profiling.
    GBE_CLASS(Unit);
    uint32_t oclVersion;
    bool valid;
    bool inProfilingMode;
  };

  /*! Output the unit string in the given stream */
  std::ostream &operator<< (std::ostream &out, const Unit &unit);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_UNIT_HPP__ */
