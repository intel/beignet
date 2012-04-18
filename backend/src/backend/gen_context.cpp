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
 * \file gen_context.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "backend/gen_context.hpp"
#include "backend/gen_program.hpp"
#include "backend/gen/brw_defines.h"
#include "backend/gen/brw_eu.h"
#include <cstring>

namespace gbe
{
  GenContext::GenContext(const ir::Unit &unit, const std::string &name) :
    Context(unit, name) {}
  GenContext::~GenContext(void) {}

  void GenContext::emitCode(void) {
    GenKernel *genKernel = static_cast<GenKernel*>(this->kernel);
    brw_compile *p = (brw_compile*) GBE_MALLOC(sizeof(brw_compile));
    std::memset(p, 0, sizeof(*p));
    p->brw_EOT(127);
    genKernel->insnNum = p->nr_insn;
    genKernel->insns = GBE_NEW_ARRAY(brw_instruction, genKernel->insnNum);
    std::memcpy(genKernel->insns, p->store, genKernel->insnNum * sizeof(brw_instruction));
    GBE_FREE(p);
  }
  Kernel *GenContext::allocateKernel(void) {
    return GBE_NEW(GenKernel, name);
  }

} /* namespace gbe */


