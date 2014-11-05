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
 * \file unit.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "ir/unit.hpp"
#include "ir/function.hpp"

namespace gbe {
namespace ir {

  Unit::Unit(PointerSize pointerSize) : pointerSize(pointerSize), valid(true) {}
  Unit::~Unit(void) {
    for (const auto &pair : functions) GBE_DELETE(pair.second);
  }
  Function *Unit::getFunction(const std::string &name) const {
    auto it = functions.find(name);
    if (it == functions.end())
      return NULL;
    return it->second;
  }
  Function *Unit::newFunction(const std::string &name) {
    auto it = functions.find(name);
    if (it != functions.end())
      return NULL;
    Function *fn = GBE_NEW(Function, name, *this);
    functions[name] = fn;
    return fn;
  }
  void Unit::newConstant(const char *data,
                         const std::string &name,
                         uint32_t size,
                         uint32_t alignment)
  {
    constantSet.append(data, name, size, alignment);
  }

  std::ostream &operator<< (std::ostream &out, const Unit &unit) {
    unit.apply([&out] (const Function &fn) { out << fn << std::endl; });
    return out;
  }
} /* namespace ir */
} /* namespace gbe */
