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

#ifndef __GBE_CONTEXT_HPP__
#define __GBE_CONTEXT_HPP__

#include "sys/platform.hpp"
#include <string>

namespace gbe {
namespace ir {

  class Unit;        // Contains the complete program
  class Function;    // We compile a function into a kernel
  class Liveness;    // Describes liveness of each ir function register
  class FunctionDAG; // Describes the instruction dependencies

} /* namespace ir */
} /* namespace gbe */

namespace gbe
{
  struct Kernel; // we build this structure

  /*! Context is the helper structure to build the Gen ISA or simulation code
   *  from GenIR
   */
  class Context : public NonCopyable
  {
  public:
    /*! Create a new context. name is the name of the function we want to
     *  compile
     */
    Context(const ir::Unit &unit, const std::string &name);
    /*! Release everything needed */
    ~Context(void);
    /*! Compile the code */
    Kernel *compileKernel(void);
  protected:
    /*! Build the curbe patch list for the given kernel */
    void buildPatchList(void);
    /*! Build the instruction stream */
    virtual void emitCode(void) = 0;
    /*! Allocate a new empty kernel */
    virtual Kernel *allocateKernel(void) = 0;
    const ir::Unit &unit;    //!< Unit that contains the kernel
    const ir::Function &fn;  //!< Function to compile
    std::string name;        //!< Name of the kernel to compile
    Kernel *kernel;          //!< Kernel we are building
    ir::Liveness *liveness;  //!< Liveness info for the variables
    ir::FunctionDAG *dag;    //!< Complete DAG of values on the function
  };

} /* namespace gbe */

#endif /* __GBE_CONTEXT_HPP__ */

