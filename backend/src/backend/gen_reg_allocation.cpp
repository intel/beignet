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
#include "backend/gen_insn_selection.hpp"
#include "backend/gen_reg_allocation.hpp"
#include "backend/program.hpp"
#include <algorithm>

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
  if (ctx.sel->isScalarOrBool(reg) == true) { \
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
    if (RA.contains(reg) == true) return grfOffset; // already allocated
    if (fn.isSpecialReg(reg) == true) return grfOffset; // already done
    if (fn.getArg(reg) != NULL) return grfOffset; // already done
    if (fn.getPushLocation(reg) != NULL) return grfOffset; // already done
    GBE_ASSERT(ctx.isScalarReg(reg) == false);
    const bool isScalar = ctx.sel->isScalarOrBool(reg);
    const RegisterData regData = ctx.sel->getRegisterData(reg);
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

  bool GenRegAllocator::isAllocated(const SelectionVector *vector) const {
    const ir::Register first = vector->reg[0].reg;
    const auto it = vectorMap.find(first);

    // If the first register is not allocated we are done
    if (it == vectorMap.end())
      return false;

    // If there are more left registers than in the found vector, there are
    // still registers to allocate
    const SelectionVector *other = it->second.first;
    const uint32_t otherFirst = it->second.second;
    const uint32_t leftNum = other->regNum - otherFirst;
    if (leftNum < vector->regNum)
      return false;

    // Now check that all the registers in the already allocated vector match
    // the current vector
    for (uint32_t regID = 1; regID < vector->regNum; ++regID) {
       const ir::Register from = vector->reg[regID].reg;
       const ir::Register to = other->reg[regID + otherFirst].reg;
       if (from != to)
         return false;
    }
    return true;
  }

  void GenRegAllocator::coalesce(Selection &selection, SelectionVector *vector) {
    for (uint32_t regID = 0; regID < vector->regNum; ++regID) {
      const ir::Register reg = vector->reg[regID].reg;
      const auto it = this->vectorMap.find(reg);
      // case 1: the register is not already in a vector, so it can stay in this
      // vector. Note that local IDs are *non-scalar* special registers but will
      // require a MOV anyway since pre-allocated in the CURBE
      if (it == vectorMap.end() &&
          ctx.sel->isScalarOrBool(reg) == false &&
          ctx.isSpecialReg(reg) == false)
      {
        const VectorLocation location = std::make_pair(vector, regID);
        this->vectorMap.insert(std::make_pair(reg, location));
      }
      // case 2: the register is already in another vector, so we need to move
      // it to a temporary register
      else {
#if 1
        ir::Register tmp;
        if (vector->isSrc)
          tmp = selection.replaceSrc(vector->insn, regID);
        else
          tmp = selection.replaceDst(vector->insn, regID);
        const VectorLocation location = std::make_pair(vector, regID);
        this->vectorMap.insert(std::make_pair(tmp, location));
#endif
      }
    }
  }

  /*! Will sort vector in decreasing order */
  INLINE bool cmp(const SelectionVector *v0, const SelectionVector *v1) {
    return v0->regNum > v1->regNum;
  }

  uint32_t GenRegAllocator::allocateVector(Selection &selection) {
    const uint32_t vectorNum = selection.getVectorNum();
    SelectionVector *vectors[vectorNum];

    // First we find and store all vectors
    uint32_t vectorID = 0;
    selection.foreach([&](SelectionTile &tile) {
      SelectionVector *v = tile.vector;
      while (v) {
        GBE_ASSERT(vectorID < vectorNum);
        vectors[vectorID++] = v;
        v = v->next;
      }
    });
    GBE_ASSERT(vectorID == vectorNum);

    // Heuristic (really simple...): sort them by the number of registers they
    // contain
    std::sort(vectors, vectors + vectorNum, cmp);

    // Insert MOVs when this is required
    for (vectorID = 0; vectorID < vectorNum; ++vectorID) {
      SelectionVector *vector = vectors[vectorID];
      if (this->isAllocated(vector))
        continue;
      this->coalesce(selection, vector);
    }

    // Allocate all the vector registers
    uint32_t grfOffset = ctx.getKernel()->getCurbeSize() + GEN_REG_SIZE;
    for (vectorID = 0; vectorID < vectorNum; ++vectorID) {
      const SelectionVector *vector = vectors[vectorID];
      const ir::Register first = vector->reg[0].reg;

      // Since there is no interference, if the first register is allocated,
      // this means that this vector is a sub-vector of a previous one, and
      // therefore all the registers are allocated
      if (RA.contains(first) == true) {
#if GBE_DEBUG
        for (uint32_t regID = 1; regID < vector->regNum; ++regID)
          GBE_ASSERT(RA.contains(vector->reg[regID].reg));
#endif /* GBE_DEBUG */
        continue;
      }

      // Allocate all the registers consecutively
      for (uint32_t regID = 0; regID < vector->regNum; ++regID) {
        const ir::Register reg = vector->reg[regID].reg;
        GBE_ASSERT(RA.contains(reg) == false);
        grfOffset = this->createGenReg(reg, grfOffset);
      }
    }
    return grfOffset;
  }

  void GenRegAllocator::allocate(Selection &selection) {
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
#if 0
    // First we build the set of all used registers
    set<Register> usedRegs;
    fn.foreachInstruction([&usedRegs](const Instruction &insn) {
      const uint32_t srcNum = insn.getSrcNum(), dstNum = insn.getDstNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID)
        usedRegs.insert(insn.getSrc(srcID));
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID)
        usedRegs.insert(insn.getDst(dstID));
    });

    this->allocateVector(selection);
#else
    // First we build the set of all used registers
    set<Register> usedRegs;
    selection.foreachInstruction([&](const SelectionInstruction &insn) {
      const uint32_t srcNum = insn.srcNum, dstNum = insn.dstNum;
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        const SelectionReg reg = insn.src[srcID];
        if (reg.file == GEN_GENERAL_REGISTER_FILE)
          usedRegs.insert(reg.reg);
      }
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
        const SelectionReg reg = insn.dst[dstID];
        if (reg.file == GEN_GENERAL_REGISTER_FILE)
          usedRegs.insert(reg.reg);
      }
    });

#endif

    // Allocate all the vectors first since they need to be contiguous
    uint32_t grfOffset = this->allocateVector(selection);

    // Allocate all used registers. Just crash when we run out-of-registers
    for (auto reg : usedRegs)
      grfOffset = this->createGenReg(reg, grfOffset);
  }

  INLINE void setGenReg(GenReg &dst, const SelectionReg &src) {
    dst.type = src.type;
    dst.file = src.file;
    dst.negation = src.negation;
    dst.absolute = src.absolute;
    dst.vstride = src.vstride;
    dst.width = src.width;
    dst.hstride = src.hstride;
    dst.address_mode = GEN_ADDRESS_DIRECT;
    dst.dw1.ud = src.immediate.ud;
  }

  GenReg GenRegAllocator::genReg(const SelectionReg &reg) {
    // Right now, only GRF are allocated ...
    if (reg.file == GEN_GENERAL_REGISTER_FILE) {
      GBE_ASSERT(RA.contains(reg.reg) != false);
      GenReg dst = RA.find(reg.reg)->second;
      setGenReg(dst, reg);
      if (reg.quarter != 0)
        dst = GenReg::Qn(dst, reg.quarter+1);
      return dst;
    }
    // Other registers are already physical registers
    else {
      GenReg dst;
      setGenReg(dst, reg);
      dst.nr = reg.nr;
      dst.subnr = reg.subnr;
      return dst;
    }
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

