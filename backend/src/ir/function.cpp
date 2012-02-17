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
 * \file function.cpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "ir/function.hpp"

namespace gbe {
namespace ir {

  Function::Function(void) {}
  Function::~Function(void) {
    for (auto it = blocks.begin(); it != blocks.end(); ++it)
      GBE_DELETE(*it);
  }
  BasicBlock::BasicBlock(Function &fn) : fn(fn) {}
  BasicBlock::~BasicBlock(void) {
    for (auto it = instructions.begin(); it != instructions.end(); ++it)
      GBE_DELETE(*it);
  }

} /* namespace ir */
} /* namespace gbe */

