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

#ifndef __GBE_PROGRAM_HPP__
#define __GBE_PROGRAM_HPP__

#include "backend/program.h"
#include "sys/hash_map.hpp"
#include "sys/vector.hpp"
#include <string>

namespace gbe {
namespace ir {
  class Unit; // Compilation unit. Contains the program to compile
} /* namespace ir */
} /* namespace gbe */

namespace gbe {

  /*! Info for the kernel argument */
  struct KernelArgument {
    gbe_arg_type type; //!< Pointer, structure, image, regular value?
    uint32_t size;     //!< Size of the argument
  };

  /*! Stores the offset where to patch where to patch */
  struct PatchInfo {
    INLINE PatchInfo(gbe_curbe_type type, uint32_t subType = 0u, uint32_t offset = 0u) :
      type(uint32_t(type)), subType(subType), offset(offset) {}
    INLINE PatchInfo(void) {}
    uint32_t type : 8;
    uint32_t subType : 8;
    uint32_t offset : 16;
  };

  /*! We will sort PatchInfo to make binary search */
  INLINE bool operator< (PatchInfo i0, PatchInfo i1) {
    if (i0.type != i1.type) return i0.type < i1.type;
    return i0.subType < i1.subType;
  }

  /*! Describe a compiled kernel */
  struct Kernel : public NonCopyable
  {
    /*! Create an empty kernel with the given name */
    Kernel(const std::string &name);
    /*! Destroy it */
    virtual ~Kernel(void);
    /*! Return the instruction stream */
    virtual const char *getCode(void) const = 0;
    /*! Return the instruction stream size */
    virtual size_t getCodeSize(void) const = 0;
    /*! Get the kernel name */
    INLINE const char *getName(void) const { return name.c_str(); }
    /*! Return the number of arguments for the kernel call */
    INLINE uint32_t getArgNum(void) const { return argNum; }
    /*! Return the size of the given argument */
    INLINE uint32_t getArgSize(uint32_t argID) const {
      return argID >= argNum ? 0u : args[argID].size;
    }
    /*! Return the type of the given argument */
    INLINE gbe_arg_type getArgType(uint32_t argID) const {
      return argID >= argNum ? GBE_ARG_INVALID : args[argID].type;
    }
    /*! Get the offset where to patch. Returns -1 if no patch needed */
    int32_t getCurbeOffset(gbe_curbe_type type, uint32_t subType) const;
    /*! Get the curbe size required by the kernel */
    uint32_t getCurbeSize(void) const { return this->curbeSize; }
  protected:
    friend class Context;       //!< Owns the kernels
    const std::string name;     //!< Kernel name
    KernelArgument *args;       //!< Each argument
    uint32_t argNum;            //!< Number of function arguments
    vector<PatchInfo> patches;  //!< Indicates how to build the curbe
    uint32_t curbeSize;         //!< Size of the data to push
  };

  /*! Describe a compiled program */
  struct Program : public NonCopyable
  {
    /*! Create an empty program */
    Program(void);
    /*! Destroy the program */
    virtual ~Program(void);
    /*! Get the number of kernels in the program */
    uint32_t getKernelNum(void) const { return kernels.size(); }
    /*! Get the kernel from its name */
    Kernel *getKernel(const std::string &name) const {
      auto it = kernels.find(name);
      if (it == kernels.end())
        return NULL;
      else
        return it->second;
    }
    /*! Get the kernel from its ID */
    Kernel *getKernel(const uint32_t ID) const {
      uint32_t currID = 0;
      Kernel *kernel = NULL;
      for (auto it = kernels.begin(); it != kernels.end(); ++it) {
        if (currID == ID) {
          kernel = it->second;
          break;
        }
      }
      return kernel;
    }
    /*! Build a program from a ir::Unit */
    bool buildFromUnit(const ir::Unit &unit, std::string &error);
    /*! Buils a program from a LLVM source code */
    bool buildFromLLVMFile(const char *fileName, std::string &error);
    /*! Buils a program from a OCL string */
    bool buildFromSource(const char *source, std::string &error);
  protected:
    /*! Compile a kernel */
    virtual Kernel *compileKernel(const ir::Unit &unit, const std::string &name) = 0;
    /*! Kernels sorted by their name */
    hash_map<std::string, Kernel*> kernels;
  };

} /* namespace gbe */

#endif /* __GBE_PROGRAM_HPP__ */



