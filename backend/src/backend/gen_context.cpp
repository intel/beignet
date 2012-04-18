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

  void GenContext::allocateSpecialReg(gbe_curbe_type curbe, const ir::Register &reg)
  {
    const int32_t offset = kernel->getCurbeOffset(curbe, 0);
    GBE_ASSERT(this->isScalarReg(reg) == true);
    if (offset >= 0) {
      const ir::RegisterData data = fn.getRegisterData(reg);
      const uint32_t typeSize = data.getSize();
      const uint32_t nr = (offset + GEN_REG_SIZE) / GEN_REG_SIZE;
      const uint32_t subnr = ((offset + GEN_REG_SIZE) % GEN_REG_SIZE) / typeSize;
      GBE_ASSERT(data.family == ir::FAMILY_DWORD); // XXX support the rest
      RA.insert(std::make_pair(reg, brw_vec1_reg(GEN_GENERAL_REGISTER_FILE, nr, subnr)));
    }
  }

  void GenContext::allocateRegister(void) {
    GBE_ASSERT(fn.getProfile() == ir::PROFILE_OCL);

    // Allocate the special registers (only those which are actually used)
    allocateSpecialReg(GBE_CURBE_LOCAL_ID_X, ir::ocl::lid0);
    allocateSpecialReg(GBE_CURBE_LOCAL_ID_Y, ir::ocl::lid1);
    allocateSpecialReg(GBE_CURBE_LOCAL_ID_Z, ir::ocl::lid2);
    allocateSpecialReg(GBE_CURBE_LOCAL_SIZE_X, ir::ocl::lsize0);
    allocateSpecialReg(GBE_CURBE_LOCAL_SIZE_Y, ir::ocl::lsize1);
    allocateSpecialReg(GBE_CURBE_LOCAL_SIZE_Z, ir::ocl::lsize2);
    allocateSpecialReg(GBE_CURBE_GLOBAL_SIZE_X, ir::ocl::gsize0);
    allocateSpecialReg(GBE_CURBE_GLOBAL_SIZE_Y, ir::ocl::gsize1);
    allocateSpecialReg(GBE_CURBE_GLOBAL_SIZE_Z, ir::ocl::gsize2);
    allocateSpecialReg(GBE_CURBE_GLOBAL_OFFSET_X, ir::ocl::goffset0);
    allocateSpecialReg(GBE_CURBE_GLOBAL_OFFSET_Y, ir::ocl::goffset1);
    allocateSpecialReg(GBE_CURBE_GLOBAL_OFFSET_Z, ir::ocl::goffset2);
    allocateSpecialReg(GBE_CURBE_GROUP_NUM_X, ir::ocl::numgroup0);
    allocateSpecialReg(GBE_CURBE_GROUP_NUM_Y, ir::ocl::numgroup1);
    allocateSpecialReg(GBE_CURBE_GROUP_NUM_Z, ir::ocl::numgroup2);

    // group IDs are always allocated by the hardware in r0
    RA.insert(std::make_pair(ir::ocl::groupid0, brw_vec1_reg(GEN_GENERAL_REGISTER_FILE, 0, 1)));
    RA.insert(std::make_pair(ir::ocl::groupid1, brw_vec1_reg(GEN_GENERAL_REGISTER_FILE, 0, 6)));
    RA.insert(std::make_pair(ir::ocl::groupid2, brw_vec1_reg(GEN_GENERAL_REGISTER_FILE, 0, 7)));

    // First we build the set of all used registers
    set<ir::Register> usedRegs;
    fn.foreachInstruction([&usedRegs](const ir::Instruction &insn) {
      const uint32_t srcNum = insn.getSrcNum(), dstNum = insn.getDstNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID)
        usedRegs.insert(insn.getSrc(srcID));
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID)
        usedRegs.insert(insn.getDst(dstID));
    });

    // Allocate all used registers. Just crash when we run out-of-registers
    // r0 is always taken by the HW
    uint32_t grfOffset = kernel->getCurbeSize() + GEN_REG_SIZE;
    if (simdWidth >= 16)
      grfOffset = ALIGN(grfOffset, 2 * GEN_REG_SIZE);
    GBE_ASSERT(simdWidth != 32); // a bit more complicated see later
    for (auto reg : usedRegs) {
      if (fn.isSpecialReg(reg) == true) continue; // already done
      const ir::RegisterData regData = fn.getRegisterData(reg);
      const ir::RegisterFamily family = regData.family;
      const uint32_t typeSize = regData.getSize();
      const uint32_t nr = grfOffset / GEN_REG_SIZE;
      const uint32_t subnr = (grfOffset % GEN_REG_SIZE) / typeSize;
      GBE_ASSERT(family == ir::FAMILY_DWORD); // XXX Do the rest
      GBE_ASSERT(grfOffset + simdWidth*typeSize < GEN_GRF_SIZE);
      RA.insert(std::make_pair(reg, brw_vec16_reg(GEN_GENERAL_REGISTER_FILE, nr, subnr)));
      grfOffset += simdWidth * typeSize;
    }
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

