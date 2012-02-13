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
 * \file context.cpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "ir/context.hpp"
#include "ir/unit.hpp"

namespace gbe {
namespace ir {

  Context::Context(Unit &unit) : unit(unit), fn(NULL), bb(NULL) {}

  void Context::startFunction(const std::string &name) {
    Function *fn = unit.newFunction(name);
    GBE_ASSERT(fn != NULL);
    fnStack.push_back(fn);
  }

} /* namespace ir */
} /* namespace gbe */

