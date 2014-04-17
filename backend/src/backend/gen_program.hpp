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

#ifndef __GBE_GEN_PROGRAM_HPP__
#define __GBE_GEN_PROGRAM_HPP__

#include "backend/program.h"
#include "backend/program.hpp"

// Gen ISA instruction
struct GenInstruction;
namespace gbe
{
  /*! Describe a compiled kernel */
  class GenKernel : public Kernel
  {
  public:
    /*! Create an empty kernel with the given name */
    GenKernel(const std::string &name);
    /*! Destroy it */
    virtual ~GenKernel(void);
    /*! Implements base class */
    virtual const char *getCode(void) const;
    /*! Set the instruction stream (to be implemented) */
    virtual const void setCode(const char *, size_t size);
    /*! Implements get the code size */
    virtual size_t getCodeSize(void) const;
    /*! Implements printStatus*/
    virtual void printStatus(int indent, std::ostream& outs);
    GenInstruction *insns; //!< Instruction stream
    uint32_t insnNum;      //!< Number of instructions
    GBE_CLASS(GenKernel);  //!< Use custom allocators
  };

  /*! Describe a compiled program */
  class GenProgram : public Program
  {
  public:
    /*! Create an empty program */
    GenProgram(uint32_t deviceID) : deviceID(deviceID) {}
    /*! Current device ID*/
    uint32_t deviceID;
    /*! Destroy the program */
    virtual ~GenProgram(void);
    /*! Implements base class */
    virtual Kernel *compileKernel(const ir::Unit &unit, const std::string &name, bool relaxMath);
    /*! Allocate an empty kernel. */
    virtual Kernel *allocateKernel(const std::string &name) {
      return GBE_NEW(GenKernel, name);
    }
    /*! Use custom allocators */
    GBE_CLASS(GenProgram);
  };

} /* namespace gbe */

#endif /* __GBE_GEN_PROGRAM_HPP__ */

