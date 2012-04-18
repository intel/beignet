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
#include "backend/gen_defs.hpp"
#include "backend/gen_eu.hpp"
#include "ir/function.hpp"
#include <cstring>

namespace gbe
{
  GenContext::GenContext(const ir::Unit &unit, const std::string &name) :
    Context(unit, name) {}
  GenContext::~GenContext(void) {}

  void GenContext::allocateRegister(void) {
    GBE_ASSERT(fn.getProfile() == ir::PROFILE_OCL);

    // First we build the set of all used registers
    set<ir::Register> usedRegs;
    fn.foreachInstruction([&usedRegs](const ir::Instruction &insn) {
      const uint32_t srcNum = insn.getSrcNum(), dstNum = insn.getDstNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID)
        usedRegs.insert(insn.getSrc(srcID));
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID)
        usedRegs.insert(insn.getDst(dstID));
    });


  }

  void GenContext::emitCode(void) {
    GenKernel *genKernel = static_cast<GenKernel*>(this->kernel);
    GenEmitter *p = (GenEmitter*) GBE_MALLOC(sizeof(GenEmitter));
    std::memset(p, 0, sizeof(*p));
    p->EOT(127);
    genKernel->insnNum = p->nr_insn;
    genKernel->insns = GBE_NEW_ARRAY(GenInstruction, genKernel->insnNum);
    std::memcpy(genKernel->insns, p->store, genKernel->insnNum * sizeof(GenInstruction));
    GBE_FREE(p);
  }

  Kernel *GenContext::allocateKernel(void) {
    return GBE_NEW(GenKernel, name);
  }

} /* namespace gbe */


