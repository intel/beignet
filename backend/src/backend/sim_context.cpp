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
#include <cstring>
#include <cstdio>
#include <fstream>
#include <dlfcn.h>

namespace gbe
{
  SimContext::SimContext(const ir::Unit &unit, const std::string &name) :
    Context(unit, name) {}
  SimContext::~SimContext(void) {}

  Kernel *SimContext::allocateKernel(void) {
    return GBE_NEW(SimKernel, name);
  }

  void SimContext::emitCode(void) {
    SimKernel *simKernel = static_cast<SimKernel*>(this->kernel);
    char srcStr[L_tmpnam+1], libStr[L_tmpnam+1];
    const std::string srcName = std::string(tmpnam_r(srcStr)) + ".cpp"; /* unsafe! */
    const std::string libName = std::string(tmpnam_r(libStr)) + ".so";  /* unsafe! */

    /* Output the code first */
    std::ofstream ostream;
    ostream.open(srcName);
    ostream << "#include <stdint.h>\n";
    ostream << "typedef struct _gen_simulator *gen_simulator;\n";
    ostream << "extern \"C\" void " << name
            << "(gen_simulator sim, "
            << "uint32_t thread, uint32_t group_x, uint32_t group_y, uint32_t group_z) {}"
            << std::endl;
    ostream.close();

    /* Compile the function */
    std::cout << srcName << " " << libName;
    std::string compileCmd = "g++ -shared -O3 -o ";
    compileCmd += libName;
    compileCmd += " ";
    compileCmd += srcName;
    printf(compileCmd.c_str());
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


