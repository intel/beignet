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

#include "gen/brw_structs.h"
#include "sys/hash_map.hpp"
#include <string>

namespace gbe {
namespace ir {

  class Unit;        // Compilation unit. Contains the program to compile
  class Liveness;    // Describes liveness of each ir function register
  class FunctionDAG; // Describes the instruction dependencies

} /* namespace ir */
} /* namespace gbe */

namespace gbe {
namespace gen {

  struct KernelArgument
  {
    GenArgType type; //!< Pointer, structure, regular value?
    size_t size;     //!< Size of each argument
  };

  /*! Describe a compiled kernel */
  struct Kernel : public NonCopyable
  {
    /*! Create an empty kernel with the given name */
    Kernel(void);
    /*! Destroy it */
    ~Kernel(void);

    std::string name;        //!< Kernel name
    KernelArgument *args;    //!< Each argument
    brw_instruction *insns;  //!< Instruction stream
    uint32_t argNum;         //!< Number of function arguments
    uint32_t insnNum;        //!< Number of instructions
    ir::Liveness *liveness;  //!< Used only for the build
    ir::FunctionDAG *dag;    //!< Used only for the build
    GBE_STRUCT(Kernel);      //!< Use gbe allocators
  };

  /*! Describe a compiled program */
  struct Program : public NonCopyable
  {
    /*! Create an empty program */
    Program(void);
    /*! Destroy the program */
    ~Program(void);
    /*! Build a program from a ir::Unit */
    bool buildFromUnit(const ir::Unit &unit, std::string &error);
    /*! Buils a program from a LLVM source code */
    bool buildFromLLVMFile(const char *fileName, std::string &error);
    /*! Buils a program from a OCL string */
    bool buildFromSource(const char *source, std::string &error);
    /*! Kernels sorted by their name */
    hash_map<std::string, Kernel*> kernels;
  };

} /* namespace gen */
} /* namespace gbe */

#endif /* __GBE_GEN_PROGRAM_HPP__ */

