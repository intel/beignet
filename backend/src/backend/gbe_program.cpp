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
 * \file callback interface for the compiler
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "gbe_program.h"
#include "gen/program.h"
#include "sim/program.h"
#include "sys/platform.hpp"
#include <cstring>

GBE_EXPORT_SYMBOL GBEProgramNewFromSourceCB *GBEProgramNewFromSource = NULL;
GBE_EXPORT_SYMBOL GBEProgramNewFromBinaryCB *GBEProgramNewFromBinary = NULL;
GBE_EXPORT_SYMBOL GBEProgramNewFromLLVMCB *GBEProgramNewFromLLVM = NULL;
GBE_EXPORT_SYMBOL GBEProgramDeleteCB *GBEProgramDelete = NULL;
GBE_EXPORT_SYMBOL GBEProgramGetKernelNumCB *GBEProgramGetKernelNum = NULL;
GBE_EXPORT_SYMBOL GBEProgramGetKernelByNameCB *GBEProgramGetKernelByName = NULL;
GBE_EXPORT_SYMBOL GBEProgramGetKernelCB *GBEProgramGetKernel = NULL;
GBE_EXPORT_SYMBOL GBEKernelGetNameCB *GBEKernelGetName = NULL;
GBE_EXPORT_SYMBOL GBEKernelGetCodeCB *GBEKernelGetCode = NULL;
GBE_EXPORT_SYMBOL GBEKernelGetCodeSizeCB *GBEKernelGetCodeSize = NULL;
GBE_EXPORT_SYMBOL GBEKernelGetArgNumCB *GBEKernelGetArgNum = NULL;
GBE_EXPORT_SYMBOL GBEKernelGetArgSizeCB *GBEKernelGetArgSize = NULL;
GBE_EXPORT_SYMBOL GBEKernelGetArgTypeCB *GBEKernelGetArgType = NULL;
GBE_EXPORT_SYMBOL GBEKernelGetSIMDWidthCB *GBEKernelGetSIMDWidth = NULL;
GBE_EXPORT_SYMBOL GBEKernelGetRequiredWorkGroupSizeCB *GBEKernelGetRequiredWorkGroupSize = NULL;

/* Use pre-main to setup the call backs */
struct CallBackInitializer
{
  CallBackInitializer(void) {
    const char *run_it = getenv("OCL_SIMULATOR");
    if (run_it != NULL && !strcmp(run_it, "2"))
      SimSetupCallBacks();
    else
      GenSetupCallBacks();
  }
};

static CallBackInitializer cbInitializer;

