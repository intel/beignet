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
 * \file program.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "gen/program.h"
#include "gen/program.hpp"
#include "ir/liveness.hpp"
#include "ir/value.hpp"
#include "ir/unit.hpp"
#include "llvm/llvm_to_gen.hpp"

namespace gbe {
namespace gen {

  Kernel::Kernel(void) :
    args(NULL), insns(NULL), argNum(0), insnNum(0), liveness(NULL), dag(NULL)
  {}
  Kernel::~Kernel(void) {
    GBE_SAFE_DELETE_ARRAY(insns);
    GBE_SAFE_DELETE_ARRAY(args);
    GBE_SAFE_DELETE(liveness);
    GBE_SAFE_DELETE(dag);
  }

  Program::Program(void) {}
  Program::~Program(void) {
    for (auto it = kernels.begin(); it != kernels.end(); ++it)
      GBE_DELETE(it->second);
  }

  bool Program::buildFromSource(const char *source, std::string &error) {
    NOT_IMPLEMENTED;
    return false;
  }
  bool Program::buildFromLLVMFile(const char *fileName, std::string &error) {
    ir::Unit unit;
    if (llvmToGen(unit, fileName) == false) {
      error = std::string(fileName) + " not found";
      return false;
    }
    this->buildFromUnit(unit, error);
    return true;
  }
  bool Program::buildFromUnit(const ir::Unit &unit, std::string &error) {
    return false;
  }

} /* namespace gen */
} /* namespace gbe */

