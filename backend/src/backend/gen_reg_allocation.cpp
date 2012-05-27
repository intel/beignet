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
 * \file gen_reg_allocation.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "ir/profile.hpp"
#include "ir/function.hpp"
#include "backend/gen_reg_allocation.hpp"
#include "backend/program.hpp"

namespace gbe
{
  // Note that byte vector registers use two bytes per byte (and can be
  // interleaved)
  static const size_t familyVectorSize[] = {2,2,2,4,8};
  static const size_t familyScalarSize[] = {2,1,2,4,8};

  /*! IR-to-Gen type conversion */
  INLINE uint32_t getGenType(ir::Type type) {
    using namespace ir;
    switch (type) {
      case TYPE_BOOL: return GEN_TYPE_UW;
      case TYPE_S8: return GEN_TYPE_B;
      case TYPE_U8: return GEN_TYPE_UB;
      case TYPE_S16: return GEN_TYPE_W;
      case TYPE_U16: return GEN_TYPE_UW;
      case TYPE_S32: return GEN_TYPE_D;
      case TYPE_U32: return GEN_TYPE_UD;
      case TYPE_FLOAT: return GEN_TYPE_F;
      default: NOT_SUPPORTED; return GEN_TYPE_F;
    }
  }

  GenRegAllocator::GenRegAllocator(GenContext &ctx) : ctx(ctx) {}

#define INSERT_REG(SIMD16, SIMD8, SIMD1) \
  if (ctx.isScalarOrBool(reg) == true) \
    RA.insert(std::make_pair(reg, GenReg::SIMD1(nr, subnr))); \
  else if (simdWidth == 8) \
    RA.insert(std::make_pair(reg, GenReg::SIMD8(nr, subnr))); \
  else if (simdWidth == 16) \
    RA.insert(std::make_pair(reg, GenReg::SIMD16(nr, subnr)));

  void GenRegAllocator::allocatePayloadReg(gbe_curbe_type value,
                                           ir::Register reg,
                                           uint32_t subValue,
                                           uint32_t subOffset)
  {
    using namespace ir;
    const Kernel *kernel = ctx.getKernel();
    const Function &fn = ctx.getFunction();
    const uint32_t simdWidth = ctx.getSimdWidth();
    const int32_t curbeOffset = kernel->getCurbeOffset(value, subValue);
    if (curbeOffset >= 0) {
      const uint32_t offset = curbeOffset + subOffset;
      const ir::RegisterData data = fn.getRegisterData(reg);
      const ir::RegisterFamily family = data.family;
      const bool isScalar = ctx.isScalarOrBool(reg);
      const uint32_t typeSize = isScalar ? familyScalarSize[family] : familyVectorSize[family];
      const uint32_t nr = (offset + GEN_REG_SIZE) / GEN_REG_SIZE;
      const uint32_t subnr = ((offset + GEN_REG_SIZE) % GEN_REG_SIZE) / typeSize;
      switch (family) {
        case FAMILY_BOOL: INSERT_REG(uw1grf, uw1grf, uw1grf); break;
        case FAMILY_WORD: INSERT_REG(uw16grf, uw8grf, uw1grf); break;
        case FAMILY_BYTE: INSERT_REG(ub16grf, ub8grf, ub1grf); break;
        case FAMILY_DWORD: INSERT_REG(f16grf, f8grf, f1grf); break;
        default: NOT_SUPPORTED;
      }
    }
  }

#undef INSERT_REG

#define INSERT_REG(SIMD16, SIMD8, SIMD1) \
  if (ctx.isScalarOrBool(reg) == true) { \
    RA.insert(std::make_pair(reg, GenReg::SIMD1(nr, subnr))); \
    grfOffset += typeSize; \
  } else if (simdWidth == 16) { \
    RA.insert(std::make_pair(reg, GenReg::SIMD16(nr, subnr))); \
    grfOffset += simdWidth * typeSize; \
  } else if (simdWidth == 8) {\
    RA.insert(std::make_pair(reg, GenReg::SIMD8(nr, subnr))); \
    grfOffset += simdWidth * typeSize; \
  } else \
    NOT_SUPPORTED;

  uint32_t GenRegAllocator::createGenReg(ir::Register reg, uint32_t grfOffset) {
    using namespace ir;
    const Function &fn = ctx.getFunction();
    const uint32_t simdWidth = ctx.getSimdWidth();
    if (fn.isSpecialReg(reg) == true) return grfOffset; // already done
    if (fn.getArg(reg) != NULL) return grfOffset; // already done
    if (fn.getPushLocation(reg) != NULL) return grfOffset; // already done
    GBE_ASSERT(ctx.isScalarReg(reg) == false);
    const bool isScalar = ctx.isScalarOrBool(reg);
    const RegisterData regData = fn.getRegisterData(reg);
    const RegisterFamily family = regData.family;
    const uint32_t typeSize = isScalar ? familyScalarSize[family] : familyVectorSize[family];
    const uint32_t regSize = simdWidth*typeSize;
    grfOffset = ALIGN(grfOffset, regSize);
    if (grfOffset + regSize <= GEN_GRF_SIZE) {
      const uint32_t nr = grfOffset / GEN_REG_SIZE;
      const uint32_t subnr = (grfOffset % GEN_REG_SIZE) / typeSize;
      switch (family) {
        case FAMILY_BOOL: INSERT_REG(uw1grf, uw1grf, uw1grf); break;
        case FAMILY_WORD: INSERT_REG(uw16grf, uw8grf, uw1grf); break;
        case FAMILY_BYTE: INSERT_REG(ub16grf, ub8grf, ub1grf); break;
        case FAMILY_DWORD: INSERT_REG(f16grf, f8grf, f1grf); break;
        default: NOT_SUPPORTED;
      }
    } else
      NOT_SUPPORTED;
    return grfOffset;
  }

#undef INSERT_REG

  void GenRegAllocator::allocateRegister(void) {
    using namespace ir;
    const Kernel *kernel = ctx.getKernel();
    const Function &fn = ctx.getFunction();
    const uint32_t simdWidth = ctx.getSimdWidth();
    GBE_ASSERT(fn.getProfile() == PROFILE_OCL);

    // Allocate the special registers (only those which are actually used)
    allocatePayloadReg(GBE_CURBE_LOCAL_ID_X, ocl::lid0);
    allocatePayloadReg(GBE_CURBE_LOCAL_ID_Y, ocl::lid1);
    allocatePayloadReg(GBE_CURBE_LOCAL_ID_Z, ocl::lid2);
    allocatePayloadReg(GBE_CURBE_LOCAL_SIZE_X, ocl::lsize0);
    allocatePayloadReg(GBE_CURBE_LOCAL_SIZE_Y, ocl::lsize1);
    allocatePayloadReg(GBE_CURBE_LOCAL_SIZE_Z, ocl::lsize2);
    allocatePayloadReg(GBE_CURBE_GLOBAL_SIZE_X, ocl::gsize0);
    allocatePayloadReg(GBE_CURBE_GLOBAL_SIZE_Y, ocl::gsize1);
    allocatePayloadReg(GBE_CURBE_GLOBAL_SIZE_Z, ocl::gsize2);
    allocatePayloadReg(GBE_CURBE_GLOBAL_OFFSET_X, ocl::goffset0);
    allocatePayloadReg(GBE_CURBE_GLOBAL_OFFSET_Y, ocl::goffset1);
    allocatePayloadReg(GBE_CURBE_GLOBAL_OFFSET_Z, ocl::goffset2);
    allocatePayloadReg(GBE_CURBE_GROUP_NUM_X, ocl::numgroup0);
    allocatePayloadReg(GBE_CURBE_GROUP_NUM_Y, ocl::numgroup1);
    allocatePayloadReg(GBE_CURBE_GROUP_NUM_Z, ocl::numgroup2);
    allocatePayloadReg(GBE_CURBE_STACK_POINTER, ocl::stackptr);

    // Group IDs are always allocated by the hardware in r0
    RA.insert(std::make_pair(ocl::groupid0, GenReg::f1grf(0, 1)));
    RA.insert(std::make_pair(ocl::groupid1, GenReg::f1grf(0, 6)));
    RA.insert(std::make_pair(ocl::groupid2, GenReg::f1grf(0, 7)));

    // block IP used to handle the mask in SW is always allocated
    int32_t blockIPOffset = GEN_REG_SIZE + kernel->getCurbeOffset(GBE_CURBE_BLOCK_IP,0);
    GBE_ASSERT(blockIPOffset >= 0 && blockIPOffset % GEN_REG_SIZE == 0);
    blockIPOffset /= GEN_REG_SIZE;
    if (simdWidth == 8)
      RA.insert(std::make_pair(ocl::blockip, GenReg::uw8grf(blockIPOffset, 0)));
    else if (simdWidth == 16)
      RA.insert(std::make_pair(ocl::blockip, GenReg::uw16grf(blockIPOffset, 0)));
    else
      NOT_SUPPORTED;

    // Allocate all (non-structure) argument parameters
    const uint32_t argNum = fn.argNum();
    for (uint32_t argID = 0; argID < argNum; ++argID) {
      const FunctionArgument &arg = fn.getArg(argID);
      GBE_ASSERT(arg.type == FunctionArgument::GLOBAL_POINTER ||
                 arg.type == FunctionArgument::CONSTANT_POINTER ||
                 arg.type == FunctionArgument::VALUE ||
                 arg.type == FunctionArgument::STRUCTURE);
      allocatePayloadReg(GBE_CURBE_KERNEL_ARGUMENT, arg.reg, argID);
    }

    // Allocate all pushed registers (i.e. structure kernel arguments)
    const Function::PushMap &pushMap = fn.getPushMap();
    for (const auto &pushed : pushMap) {
      const uint32_t argID = pushed.second.argID;
      const uint32_t subOffset = pushed.second.offset;
      const Register reg = pushed.second.getRegister();
      allocatePayloadReg(GBE_CURBE_KERNEL_ARGUMENT, reg, argID, subOffset);
    }

    // First we build the set of all used registers
    set<Register> usedRegs;
    fn.foreachInstruction([&usedRegs](const Instruction &insn) {
      const uint32_t srcNum = insn.getSrcNum(), dstNum = insn.getDstNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID)
        usedRegs.insert(insn.getSrc(srcID));
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID)
        usedRegs.insert(insn.getDst(dstID));
    });

    // Allocate all used registers. Just crash when we run out-of-registers
    uint32_t grfOffset = kernel->getCurbeSize() + GEN_REG_SIZE;
    for (auto reg : usedRegs)
      grfOffset = this->createGenReg(reg, grfOffset);
  }

  GenReg GenRegAllocator::genReg(ir::Register reg, ir::Type type) {
    const uint32_t genType = getGenType(type);
    auto it = RA.find(reg);
    GBE_ASSERT(it != RA.end());
    return GenReg::retype(it->second, genType);
  }

  GenReg GenRegAllocator::genRegQn(ir::Register reg, uint32_t quarter, ir::Type type) {
    GBE_ASSERT(quarter == 2 || quarter == 3 || quarter == 4);
    GenReg genReg = this->genReg(reg, type);
    if (ctx.isScalarReg(reg) == true)
      return genReg;
    else
      return GenReg::Qn(genReg, quarter);
  }

} /* namespace gbe */

