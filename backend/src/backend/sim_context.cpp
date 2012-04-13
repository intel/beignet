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
    const uint32_t regNum = fn.regNum();
    for (uint32_t regID = 0; regID < regNum; ++regID) {
      const ir::Register reg(regID);
      if (reg == ir::ocl::groupid0 ||
          reg == ir::ocl::groupid1 ||
          reg == ir::ocl::groupid2)
        continue;
      const ir::RegisterData regData = fn.getRegisterData(reg);
      switch (regData.family) {
        case ir::FAMILY_BOOL:
        case ir::FAMILY_BYTE:
        case ir::FAMILY_WORD:
        case ir::FAMILY_QWORD:
          NOT_IMPLEMENTED;
        break;
        case ir::FAMILY_DWORD:
          if (isScalarReg(reg) == true)
            o << "scalar_dw _" << regID << ";\n";
          else
            o << "simd" << simdWidth << "dw _" << regID << ";\n";
        break;
      }
    }
  }

  void SimContext::loadCurbe(void) {
    // Right now curbe is only made of input argument stuff
    const uint32_t inputNum = fn.inputNum();
    for (uint32_t inputID = 0; inputID < inputNum; ++inputID) {

    }
  }

  void SimContext::emitCode(void) {
    SimKernel *simKernel = static_cast<SimKernel*>(this->kernel);
    char srcStr[L_tmpnam+1], libStr[L_tmpnam+1];
    const std::string srcName = std::string(tmpnam_r(srcStr)) + ".cpp"; /* unsafe! */
    const std::string libName = std::string(tmpnam_r(libStr)) + ".so";  /* unsafe! */
    std::cout << fn;
    /* Output the code first */
    o.open(srcName);
    o << simulator_str << std::endl;
    o << sim_vector_str << std::endl;
    o << "#include <stdint.h>\n";
    o << "extern \"C\" void " << name
      << "(gbe_simulator sim, uint32_t tid, scalar_dw _3, scalar_dw _4, scalar_dw _5)\n"
      << "{\n"
      << "const size_t curbe_sz = sim->get_curbe_size(sim);\n"
      << "const char *curbe = (const char*) sim->get_curbe_address(sim) + curbe_sz * tid;\n";
    this->emitRegisters();
    o << "}\n";
    o << std::endl;
    o.close();

    /* Compile the function */
    std::cout << "# source: " << srcName << " library: " << libName << std::endl;
    std::string compileCmd = "g++ -funroll-loops -shared -msse -msse2 -msse3 -mssse3 -msse4.1 -g -O3 -o ";
    compileCmd += libName;
    compileCmd += " ";
    compileCmd += srcName;
    std::cout << "# compilation command: " << compileCmd << std::endl;
    if (UNLIKELY(system(compileCmd.c_str()) != 0))
      FATAL("Simulation program compilation failed");

    /* Load it and get the function pointer */
    simKernel->handle = dlopen(libName.c_str(), RTLD_NOW);
    if (UNLIKELY(simKernel->handle == NULL))
      FATAL("Failed to open the compiled shared object");
    simKernel->fn = (SimKernelCallBack*) dlsym(simKernel->handle, name.c_str());
    if (UNLIKELY(simKernel->fn == NULL))
      FATAL("Failed to get the symbol from the compiled shared object");
  }
} /* namespace gbe */

