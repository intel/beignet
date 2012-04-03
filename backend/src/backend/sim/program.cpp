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

namespace gbe {
namespace sim {


} /* namespace sim */
} /* namespace gen */

  void simSetupCallBacks(void)
  {
#if 0
    GBEProgramNewFromSource = SimProgramNewFromSource;
    GBEProgramNewFromBinary = SimProgramNewFromBinary;
    GBEProgramNewFromLLVM = SimProgramNewFromLLVM;
    GBEProgramDelete = SimProgramDelete;
    GBEProgramGetKernelNum = SimProgramGetKernelNum;
    GBEProgramGetKernelByName = SimProgramGetKernelByName;
    GBEProgramGetKernel = SimProgramGetKernel;
    GBEKernelGetName = SimKernelGetName;
    GBEKernelGetCode = SimKernelGetCode;
    GBEKernelGetCodeSize = SimKernelGetCodeSize;
    GBEKernelGetArgNum = SimKernelGetArgNum;
    GBEKernelGetArgSize = SimKernelGetArgSize;
    GBEKernelGetArgType = SimKernelGetArgType;
    GBEKernelGetSIMDWidth = SimKernelGetSIMDWidth;
    GBEKernelGetRequiredWorkGroupSize = SimKernelGetRequiredWorkGroupSize;
#endif
  }

