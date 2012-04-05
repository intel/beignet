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
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "backend/context.hpp"
#include "backend/program.hpp"
#include "ir/unit.hpp"
#include "ir/function.hpp"
#include <algorithm>

namespace gbe
{
  Context::Context(const ir::Unit &unit, const std::string &name) :
    unit(unit), fn(*unit.getFunction(name)), name(name), liveness(NULL), dag(NULL)
  { GBE_ASSERT(unit.getPointerSize() == ir::POINTER_32_BITS); }
  Context::~Context(void) {}

  Kernel *Context::compileKernel(void) {
    this->kernel = this->allocateKernel();
    this->buildPatchList();
    this->emitCode();
    return this->kernel;
  }

  void Context::buildPatchList(void) {
    const uint32_t inputNum = fn.inputNum();
    uint32_t curbeSize = 0u;
    uint32_t ptrSize = unit.getPointerSize() == ir::POINTER_32_BITS ? 4u : 8u;
    for (uint32_t inputID = 0u; inputID < inputNum; ++inputID) {
      const ir::FunctionInput &input = fn.getInput(inputID);
      // This is a pointer -> 4 bytes to patch
      if (input.type == ir::FunctionInput::GLOBAL_POINTER ||
          input.type == ir::FunctionInput::CONSTANT_POINTER) {
        const PatchInfo patch(GBE_CURBE_BUFFER_ADDRESS, 0u, curbeSize);
        kernel->patches.push_back(patch);
        curbeSize += ptrSize;
      }
    }

    // After this point the vector is immutable. so, Sorting is will make
    // research faster
    std::sort(kernel->patches.begin(), kernel->patches.end());
  }

} /* namespace gbe */

