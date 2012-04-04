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

#include "backend/gbe_program.h"
#include "backend/sim/program.h"
#include "backend/sim/program.hpp"
#include <cstring>
#include <cstdio>
#include <fstream>
#include "dlfcn.h"

namespace gbe {

  SimKernel::SimKernel(const std::string &name) :
    Kernel(name), fn(NULL), handle(NULL) {}
  SimKernel::~SimKernel(void) { if (this->handle) dlclose(this->handle); }

  SimProgram::SimProgram(void) {}
  SimProgram::~SimProgram(void) {}

  Kernel *SimProgram::compileKernel(const std::string &name) {
    SimKernel *kernel = GBE_NEW(SimKernel, name);
    char srcStr[L_tmpnam+1], libStr[L_tmpnam+1];
    const std::string srcName = std::string(tmpnam_r(srcStr)) + ".cpp"; /* unsecure but we don't care */
    const std::string libName = std::string(tmpnam_r(libStr)) + ".so";  /* unsecure but we don't care */

    /* Output the code first */
    std::ofstream ostream;
    ostream.open(srcName);
    ostream << "extern \"C\" void " << name << "() {}" << std::endl;
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
    kernel->handle = dlopen(libName.c_str(), RTLD_NOW);
    if (UNLIKELY(kernel->handle == NULL))
      FATAL("Failed to open the compiled shared object");
    kernel->fn = (SimKernelCallBack*) dlsym(kernel->handle, name.c_str());
    if (UNLIKELY(kernel->fn == NULL))
      FATAL("Failed to get the symbol from the compiled shared object");
    return kernel;
  }

  static gbe_program simProgramNewFromSource(const char *source) {
    NOT_IMPLEMENTED;
    return NULL;
  }

  static gbe_program simProgramNewFromBinary(const char *binary, size_t size) {
    NOT_IMPLEMENTED;
    return NULL;
  }

  static gbe_program simProgramNewFromLLVM(const char *fileName,
      size_t stringSize,
      char *err,
      size_t *errSize)
  {
    using namespace gbe;
    SimProgram *program = GBE_NEW(SimProgram);
    std::string error;
    /* Try to compile the program */
    if (program->buildFromLLVMFile(fileName, error) == false) {
      if (err != NULL && errSize != NULL && stringSize > 0u) {
        const size_t msgSize = std::min(error.size(), stringSize-1u);
        std::memcpy(err, error.c_str(), msgSize);
        *errSize = error.size();
      }
      GBE_DELETE(program);
      return NULL;
    }
    /* Everything run fine */
    return (gbe_program) program;
  }

} /* namespace gen */

void simSetupCallBacks(void)
{
  gbe_program_new_from_source = gbe::simProgramNewFromSource;
  gbe_program_new_from_binary = gbe::simProgramNewFromBinary;
  gbe_program_new_from_llvm = gbe::simProgramNewFromLLVM;
}

