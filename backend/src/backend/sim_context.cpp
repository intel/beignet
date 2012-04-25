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
 * \file sim_context.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "backend/sim_context.hpp"
#include "backend/sim_program.hpp"
#include "ir/function.hpp"
#include "sys/cvar.hpp"
#include <cstring>
#include <cstdio>
#include <dlfcn.h>

namespace gbe
{
  SimContext::SimContext(const ir::Unit &unit, const std::string &name) :
    Context(unit, name) {}
  SimContext::~SimContext(void) {}

  Kernel *SimContext::allocateKernel(void) {
    return GBE_NEW(SimKernel, name);
  }

  extern std::string simulator_str;
  extern std::string sim_vector_str;

  void SimContext::emitRegisters(void) {
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

    // Declare register variables
    const uint32_t regNum = fn.regNum();
    bool lid0 = false, lid1 = false, lid2 = false; // for local id registers
    for (uint32_t regID = 0; regID < regNum; ++regID) {
      const ir::Register reg(regID);
      if (usedRegs.contains(reg) == false) continue;
      if (reg == ir::ocl::groupid0 ||
          reg == ir::ocl::groupid1 ||
          reg == ir::ocl::groupid2)
        continue;
      if (reg == ir::ocl::lid0) lid0 = true;
      if (reg == ir::ocl::lid1) lid1 = true;
      if (reg == ir::ocl::lid2) lid2 = true;
      const ir::RegisterData regData = fn.getRegisterData(reg);
      switch (regData.family) {
        case ir::FAMILY_BYTE:
        case ir::FAMILY_WORD:
        case ir::FAMILY_QWORD:
          NOT_IMPLEMENTED;
        break;
        case ir::FAMILY_BOOL:
          if (isScalarReg(reg) == true)
            o << "scalar_m _" << regID << ";\n";
          else
            o << "simd" << simdWidth << "m _" << regID << ";\n";
        break;
        case ir::FAMILY_DWORD:
          if (isScalarReg(reg) == true)
            o << "scalar_dw _" << regID << ";\n";
          else
            o << "simd" << simdWidth << "dw _" << regID << ";\n";
        break;
      }
    }

    // Always declare local IDs
    if (lid0 == false) o << "scalar_dw _" << uint32_t(ir::ocl::lid0) << ";\n";
    if (lid1 == false) o << "scalar_dw _" << uint32_t(ir::ocl::lid1) << ";\n";
    if (lid2 == false) o << "scalar_dw _" << uint32_t(ir::ocl::lid2) << ";\n";
  }

#define LOAD_SPECIAL_REG(CURBE, REG) do { \
    const int32_t offset = kernel->getCurbeOffset(CURBE, 0); \
    if (offset >= 0) \
      o << "LOAD(_" << uint32_t(REG) << ", curbe + " << offset << ");\n"; \
  } while (0)

  void SimContext::emitCurbeLoad(void) {
    // Right now curbe is only made of input argument stuff
    const uint32_t inputNum = fn.inputNum();
    for (uint32_t inputID = 0; inputID < inputNum; ++inputID) {
      const ir::FunctionInput &input = fn.getInput(inputID);
      const ir::Register reg = input.reg;
      const int32_t offset = kernel->getCurbeOffset(GBE_CURBE_KERNEL_ARGUMENT, inputID);
      // XXX add support for these items
      GBE_ASSERT (input.type != ir::FunctionInput::VALUE &&
                  input.type != ir::FunctionInput::STRUCTURE &&
                  input.type != ir::FunctionInput::IMAGE &&
                  input.type != ir::FunctionInput::LOCAL_POINTER);
      GBE_ASSERT(offset >= 0);
      o << "LOAD(_" << uint32_t(reg) << ", curbe + " << offset << ");\n";
    }

    // We must now load the special registers needed by the kernel
    LOAD_SPECIAL_REG(GBE_CURBE_LOCAL_ID_X, ir::ocl::lid0);
    LOAD_SPECIAL_REG(GBE_CURBE_LOCAL_ID_Y, ir::ocl::lid1);
    LOAD_SPECIAL_REG(GBE_CURBE_LOCAL_ID_Z, ir::ocl::lid2);
    LOAD_SPECIAL_REG(GBE_CURBE_LOCAL_SIZE_X, ir::ocl::lsize0);
    LOAD_SPECIAL_REG(GBE_CURBE_LOCAL_SIZE_Y, ir::ocl::lsize1);
    LOAD_SPECIAL_REG(GBE_CURBE_LOCAL_SIZE_Z, ir::ocl::lsize2);
    LOAD_SPECIAL_REG(GBE_CURBE_GLOBAL_SIZE_X, ir::ocl::gsize0);
    LOAD_SPECIAL_REG(GBE_CURBE_GLOBAL_SIZE_Y, ir::ocl::gsize1);
    LOAD_SPECIAL_REG(GBE_CURBE_GLOBAL_SIZE_Z, ir::ocl::gsize2);
    LOAD_SPECIAL_REG(GBE_CURBE_GLOBAL_OFFSET_X, ir::ocl::goffset0);
    LOAD_SPECIAL_REG(GBE_CURBE_GLOBAL_OFFSET_Y, ir::ocl::goffset1);
    LOAD_SPECIAL_REG(GBE_CURBE_GLOBAL_OFFSET_Z, ir::ocl::goffset2);
    LOAD_SPECIAL_REG(GBE_CURBE_GROUP_NUM_X, ir::ocl::numgroup0);
    LOAD_SPECIAL_REG(GBE_CURBE_GROUP_NUM_Y, ir::ocl::numgroup1);
    LOAD_SPECIAL_REG(GBE_CURBE_GROUP_NUM_Z, ir::ocl::numgroup2);
  }

  static const char *typeStr(const ir::Type &type) {
    switch (type) {
      case ir::TYPE_BOOL: return "M";
      case ir::TYPE_S8:   return "S8";
      case ir::TYPE_S16:  return "S16";
      case ir::TYPE_S32:  return "S32";
      case ir::TYPE_S64:  return "S64";
      case ir::TYPE_U8:   return "U8";
      case ir::TYPE_U16:  return "U16";
      case ir::TYPE_U32:  return "U32";
      case ir::TYPE_U64:  return "U64";
      case ir::TYPE_HALF: return "HALF";
      case ir::TYPE_FLOAT: return "F";
      case ir::TYPE_DOUBLE: return "D";
      default: NOT_IMPLEMENTED; return NULL;
    };
  }

  void SimContext::emitMaskingCode(void) {
    o << "simd" << simdWidth << "m " << "emask;\n"
      << "simd" << simdWidth << "dw " << "uip(scalar_dw(0u));\n"
      << "alltrueMask(emask);\n"
      << "uint32_t movedMask = ~0x0u;\n";
  }

  void SimContext::emitInstructionStream(void) {
    using namespace ir;
    fn.foreachInstruction([&](const Instruction &insn) {
      const char *opcodeStr = NULL;
      const Opcode opcode = insn.getOpcode();
#define DECL_INSN(OPCODE, FAMILY) \
      case OP_##OPCODE: \
      if (opcode == OP_LOAD) opcodeStr = "GATHER"; \
      else if (opcode == OP_STORE) opcodeStr = "SCATTER"; \
      else opcodeStr = #OPCODE; \
      break;
      switch (opcode) {
        #include "ir/instruction.hxx"
      default: NOT_IMPLEMENTED;
#undef DECL_INSN
      }
      if (opcode == OP_LABEL) {
        const LabelInstruction labelInsn = cast<LabelInstruction>(insn);
        const LabelIndex index = labelInsn.getLabelIndex();
        o << "\n";
        if (usedLabels.contains(index) == false)  o << "// ";
        o << "label" << index << ":\n";
        o << "SIM_JOIN(uip, emask, " << uint32_t(index) << ");\n";
        return;
      } else if (opcode == OP_BRA) {
        // Get the label of the block
        const BranchInstruction bra = cast<BranchInstruction>(insn);
        const BasicBlock *bb = insn.getParent();
        const Instruction *label = bb->getFirstInstruction();
        GBE_ASSERT(label->isMemberOf<LabelInstruction>() == true);
        const LabelIndex srcIndex = cast<LabelInstruction>(label)->getLabelIndex();
        const LabelIndex dstIndex = bra.getLabelIndex();
        const bool isPredicated = bra.isPredicated();

        if (uint32_t(dstIndex) > uint32_t(srcIndex)) { // FWD jump here
          if (isPredicated) {
            const Register pred = bra.getPredicateIndex();
            o << "SIM_FWD_BRA_C(uip, emask, " << "_" << pred
              << ", " << uint32_t(dstIndex) << ", " << uint32_t(dstIndex)
              << ");\n";
          } else {
            o << "SIM_FWD_BRA(uip, emask, "
              << uint32_t(dstIndex) << ", " << uint32_t(dstIndex)
              << ");\n";
          }
        } else { // BWD jump
          if (isPredicated) {
            const Register pred = bra.getPredicateIndex();
            o << "SIM_BWD_BRA_C(uip, _" << pred
              << ", " << uint32_t(dstIndex) << ");\n";
          } else
            o << "SIM_BWD_BRA(uip, emask, " << uint32_t(dstIndex) << ");\n";
        }
        return;
      } else if (opcode == OP_RET) {
        o << "return;\n";
        return;
      }

#if GBE_DEBUG
      // Extra checks
      if (opcode == OP_LOAD)
        GBE_ASSERT(cast<LoadInstruction>(insn).getValueNum() == 1);
      if (opcode == OP_STORE)
        GBE_ASSERT(cast<StoreInstruction>(insn).getValueNum() == 1);
#endif /* GBE_DEBUG */

      // Regular compute instruction
      const uint32_t dstNum = insn.getDstNum();
      const uint32_t srcNum = insn.getSrcNum();

      // These two needs a new instruction. Fortunately, it is just a string
      // manipulation. MASKED(OP,... just becomes MASKED_OP(...)
      if (opcode == OP_STORE || opcode == OP_LOAD)
        o << "MASKED_" << opcodeStr << "(";
      else
        o << "MASKED" << dstNum << "(" << opcodeStr;

      // Append type when needed
      if (insn.isMemberOf<UnaryInstruction>() == true)
       o << "_" << typeStr(cast<UnaryInstruction>(insn).getType());
      else if (insn.isMemberOf<BinaryInstruction>() == true)
       o << "_" << typeStr(cast<BinaryInstruction>(insn).getType());
      else if (insn.isMemberOf<TernaryInstruction>() == true)
       o << "_" << typeStr(cast<BinaryInstruction>(insn).getType());
      else if (insn.isMemberOf<CompareInstruction>() == true)
       o << "_" << typeStr(cast<CompareInstruction>(insn).getType());
      if (opcode != OP_STORE && opcode != OP_LOAD)
        o << ", ";

      // Output both destinations and sources in that order
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
        o << "_" << uint32_t(insn.getDst(dstID));
        if (dstID < dstNum-1 || srcNum > 0) o << ", ";
      }
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        o << "_" << uint32_t(insn.getSrc(srcID));
        if (srcID < srcNum-1) o << ", ";
      }

      // Append extra stuff for instructions that need it
      if (opcode == OP_LOADI) {
        Immediate imm = cast<LoadImmInstruction>(insn).getImmediate();
        GBE_ASSERT(imm.type == TYPE_S32 ||
                   imm.type == TYPE_U32 ||
                   imm.type == TYPE_FLOAT);
        o << ", " << imm.data.u32;
      } else if (opcode == OP_LOAD || opcode == OP_STORE)
        o << ", base, movedMask";
      o << ");\n";
    });
  }

#undef LOAD_SPECIAL_REG

  SVAR(OCL_GCC_SIM_COMPILER, "gcc");
  SVAR(OCL_ICC_SIM_COMPILER, "icc");
  SVAR(OCL_GCC_SIM_COMPILER_OPTIONS, "-Wall -fPIC -shared -msse -msse2 -msse3 -mssse3 -msse4.1 -g -O3");
  SVAR(OCL_ICC_SIM_COMPILER_OPTIONS, "-Wall -ldl -fabi-version=2 -fPIC -shared -O3 -g");
  BVAR(OCL_USE_ICC, false);

  void SimContext::emitCode(void) {
    SimKernel *simKernel = static_cast<SimKernel*>(this->kernel);
    char srcStr[L_tmpnam+1], libStr[L_tmpnam+1];
    const std::string srcName = std::string(tmpnam_r(srcStr)) + ".cpp"; /* unsafe! */
    const std::string libName = std::string(tmpnam_r(libStr)) + ".so";  /* unsafe! */

    // Output the code first
    o.open(srcName);
    o << simulator_str << std::endl;
    o << sim_vector_str << std::endl;
    o << "#include <stdint.h>\n";
    o << "extern \"C\" void " << name
      << "(gbe_simulator sim, uint32_t tid, scalar_dw _3, scalar_dw _4, scalar_dw _5)\n"
      << "{\n"
      << "const size_t curbe_sz = sim->get_curbe_size(sim);\n"
      << "const char *curbe = (const char*) sim->get_curbe_address(sim) + curbe_sz * tid;\n"
      << "char *base = (char*) sim->get_base_address(sim);\n";

    this->emitRegisters();
    this->emitMaskingCode();
    this->emitCurbeLoad();
    this->emitInstructionStream();
    o << "}\n";
    o << std::endl;
    o.close();

    // Compile the function
    std::cout << "# source: " << srcName << " library: " << libName << std::endl;
    std::string compileCmd;
    if (OCL_USE_ICC)
      compileCmd = OCL_ICC_SIM_COMPILER + " " + OCL_ICC_SIM_COMPILER_OPTIONS + " -o ";
    else
      compileCmd = OCL_GCC_SIM_COMPILER + " " + OCL_GCC_SIM_COMPILER_OPTIONS + " -o ";
    compileCmd += libName;
    compileCmd += " ";
    compileCmd += srcName;
    std::cout << "# compilation command: " << compileCmd << std::endl;
    if (UNLIKELY(system(compileCmd.c_str()) != 0))
      FATAL("Simulation program compilation failed");

    // Load it and get the function pointer
    simKernel->handle = dlopen(libName.c_str(), RTLD_NOW);
    if (UNLIKELY(simKernel->handle == NULL)) {
      std::cerr << "errno[" << errno << "], errmsg[" << dlerror() << "]\n";
      FATAL("Failed to open the compiled shared object");
    }
    simKernel->fn = (SimKernelCallBack*) dlsym(simKernel->handle, name.c_str());
    if (UNLIKELY(simKernel->fn == NULL))
      FATAL("Failed to get the symbol from the compiled shared object");
  }
} /* namespace gbe */

