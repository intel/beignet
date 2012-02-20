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
  LabelIndex Function::newLabel(void) {
    GBE_ASSERTM(labels.size() < 0xffff,
                "Too many labels are defined (65536 only are supported)");
    const LabelIndex index(labels.size());
    labels.push_back(NULL);
    return index;
  }
  BasicBlock::BasicBlock(Function &fn) : fn(fn) {}
  BasicBlock::~BasicBlock(void) {
    for (auto it = instructions.begin(); it != instructions.end(); ++it)
      fn.deleteInstruction(*it);
  }

} /* namespace ir */
} /* namespace gbe */

