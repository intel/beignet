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
#include "sys/set.hpp"
#include "sys/map.hpp"
#include "ir/instruction.hpp"
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
    virtual ~Context(void);
    /*! Compile the code */
    Kernel *compileKernel(void);
    /*! Tells if the labels is used */
    INLINE bool isLabelUsed(ir::LabelIndex index) const {
      return usedLabels.contains(index);
    }
    /*! Tells if the register is used */
    bool isRegUsed(const ir::Register &reg) const;
    /*! Indicate if a register is scalar or not */
    bool isScalarReg(const ir::Register &reg) const;
    /*! Get the kernel we are currently compiling */
    INLINE Kernel *getKernel(void) const { return this->kernel; }
    /*! Get the function we are currently compiling */
    INLINE const ir::Function &getFunction(void) const { return this->fn; }
  protected:
    /*! Look if a stack is needed and allocate it */
    void buildStack(void);
    /*! Build the curbe patch list for the given kernel */
    void buildPatchList(void);
    /*! Build the list of arguments to set to launch the kernel */
    void buildArgList(void);
    /*! Build the sets of used labels */
    void buildUsedLabels(void);
    /*! Build JIPs for each branch and possibly labels. Can be different from
     *  the branch target due to unstructured branches
     */
    void buildJIPs(void);
    /*! Build the instruction stream */
    virtual void emitCode(void) = 0;
    /*! Allocate a new empty kernel */
    virtual Kernel *allocateKernel(void) = 0;
    /*! Provide for each branch and label the label index target */
    typedef map<const ir::Instruction*, ir::LabelIndex> JIPMap;
    const ir::Unit &unit;           //!< Unit that contains the kernel
    const ir::Function &fn;         //!< Function to compile
    std::string name;               //!< Name of the kernel to compile
    Kernel *kernel;                 //!< Kernel we are building
    ir::Liveness *liveness;         //!< Liveness info for the variables
    ir::FunctionDAG *dag;           //!< Graph of values on the function
    set<ir::LabelIndex> usedLabels; //!< Set of all used labels
    JIPMap JIPs;                    //!< Where to jump all labels / branches
    uint32_t simdWidth;             //!< Number of lanes per HW threads
  };

} /* namespace gbe */

#endif /* __GBE_CONTEXT_HPP__ */

