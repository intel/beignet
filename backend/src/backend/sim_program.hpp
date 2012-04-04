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
 * \file program.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GBE_SIM_PROGRAM_HPP__
#define __GBE_SIM_PROGRAM_HPP__

#include "backend/program.h"
#include "backend/program.hpp"
#include "backend/gen/brw_structs.h"

namespace gbe {

  /*! We basically create one real C function for each */
  typedef void (SimKernelCallBack)();

  /*! Describe a compiled kernel */
  struct SimKernel : public Kernel
  {
    /*! Create an empty kernel with the given name */
    SimKernel(const std::string &name);
    /*! Destroy it */
    virtual ~SimKernel(void);
    /*! Implements base class */
    virtual const char *getCode(void) const { return (const char*) &fn; }
    /*! Implements base class */
    virtual size_t getCodeSize(void) const { return sizeof(&fn); }
    SimKernelCallBack *fn; //!< Function that runs the code
    void *handle;          //!< dlopen / dlclose / dlsym handle
    GBE_STRUCT(SimKernel); //!< Use gbe allocators
  };

  /*! Describe a compiled program */
  struct SimProgram : public Program
  {
    /*! Create an empty program */
    SimProgram(void);
    /*! Destroy the program */
    virtual ~SimProgram(void);
    /*! Implements base class */
    virtual Kernel *compileKernel(const std::string &name);
    GBE_STRUCT(SimProgram); //!< Use gbe allocators
  };

} /* namespace gbe */

#endif /* __GBE_SIM_PROGRAM_HPP__ */

