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
 * \file context.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GBE_SIM_CONTEXT_HPP__
#define __GBE_SIM_CONTEXT_HPP__

#include <string>
#include "backend/context.hpp"

namespace gbe
{
  struct Kernel; // we build this structure

  /*! Context is the helper structure to build the Gen ISA or simulation code
   *  from GenIR
   */
  class SimContext : public Context
  {
  public:
    /*! Create a new context. name is the name of the function we want to
     *  compile
     */
    SimContext(const ir::Unit &unit, const std::string &name);
    /*! Release everything needed */
    ~SimContext(void);
    /*! Implements base class */
    virtual void emitCode(void);
    /*! Implements base class */
    virtual Kernel *allocateKernel(void);
  };

} /* namespace gbe */

#endif /* __GBE_SIM_CONTEXT_HPP__ */

