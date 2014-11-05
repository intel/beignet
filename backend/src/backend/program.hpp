/*
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
#include "backend/context.hpp"
#include "ir/constant.hpp"
#include "ir/unit.hpp"
#include "ir/function.hpp"
#include "ir/printf.hpp"
#include "ir/sampler.hpp"
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
    uint32_t align;    //!< addr alignment of the argument
    uint8_t bti;      //!< binding table index for __global buffer
    ir::FunctionArgument::InfoFromLLVM info;
  };

  /*! Stores the offset where to patch where to patch */
  struct PatchInfo {
    INLINE PatchInfo(gbe_curbe_type type, uint32_t subType = 0u, uint32_t offset = 0u) :
      type(uint32_t(type)), subType(subType), offset(offset) {}
    INLINE PatchInfo(void) {}
    uint64_t type : 16;    //!< Type of the patch (see program.h for the list)
    uint64_t subType : 32; //!< Optional sub-type of the patch (see program.h)
    uint64_t offset : 16; //!< Optional offset to encode
  };

  /*! We will sort PatchInfo to make binary search */
  INLINE bool operator< (PatchInfo i0, PatchInfo i1) {
    if (i0.type != i1.type) return i0.type < i1.type;
    return i0.subType < i1.subType;
  }

  /*! Describe a compiled kernel */
  class Kernel : public NonCopyable, public Serializable
  {
  public:
    /*! Create an empty kernel with the given name */
    Kernel(const std::string &name);
    /*! Destroy it */
    virtual ~Kernel(void);
    /*! Return the instruction stream (to be implemented) */
    virtual const char *getCode(void) const = 0;
    /*! Set the instruction stream.*/
    virtual void setCode(const char *, size_t size) = 0;
    /*! Return the instruction stream size (to be implemented) */
    virtual size_t getCodeSize(void) const = 0;
    /*! Get the kernel name */
    INLINE const char *getName(void) const { return name.c_str(); }
    /*! Return the number of arguments for the kernel call */
    INLINE uint32_t getArgNum(void) const { return argNum; }
    /*! Return the size of the given argument */
    INLINE uint32_t getArgSize(uint32_t argID) const {
      return argID >= argNum ? 0u : args[argID].size;
    }
    /*! Return the bti for __global buffer */
    INLINE uint8_t getArgBTI(uint32_t argID) const {
      return argID >= argNum ? 0u : args[argID].bti;
    }
    /*! Return the alignment of buffer argument */
    INLINE uint32_t getArgAlign(uint32_t argID) const {
      return argID >= argNum ? 0u : args[argID].align;
    }
    /*! Return the type of the given argument */
    INLINE gbe_arg_type getArgType(uint32_t argID) const {
      return argID >= argNum ? GBE_ARG_INVALID : args[argID].type;
    }
    /*! Get the offset where to patch. Returns -1 if no patch needed */
    int32_t getCurbeOffset(gbe_curbe_type type, uint32_t subType) const;
    /*! Get the curbe size required by the kernel */
    INLINE uint32_t getCurbeSize(void) const { return this->curbeSize; }
    /*! Return the size of the stack (zero if none) */
    INLINE uint32_t getStackSize(void) const { return this->stackSize; }
    /*! Return the size of the scratch memory needed (zero if none) */
    INLINE uint32_t getScratchSize(void) const { return this->scratchSize; }
    /*! Get the SIMD width for the kernel */
    INLINE uint32_t getSIMDWidth(void) const { return this->simdWidth; }
    /*! Says if SLM is needed for it */
    INLINE bool getUseSLM(void) const { return this->useSLM; }
    /*! get slm size for kernel local variable */
    INLINE uint32_t getSLMSize(void) const { return this->slmSize; }
    /*! Set sampler set. */
    void setSamplerSet(ir::SamplerSet *from) {
      samplerSet = from;
    }
    /*! Get defined sampler size */
    size_t getSamplerSize(void) const { return (samplerSet == NULL ? 0 : samplerSet->getDataSize()); }
    /*! Get defined sampler value array */
    void getSamplerData(uint32_t *samplers) const { samplerSet->getData(samplers); }
    /*! Set image set. */
    void setImageSet(ir::ImageSet * from) {
      imageSet = from;
    }
    /*! Set printf set. */
    void setPrintfSet(ir::PrintfSet * from) {
      printfSet = from;
    }
    /* ! Return the offset in the sizeof(xxx). */
    uint32_t getPrintfSizeOfSize(void) const {
      return printfSet ? printfSet->getPrintfSizeOfSize() : 0;
    }
    uint32_t getPrintfNum() const {
      return printfSet ? printfSet->getPrintfNum() : 0;
    }

    void * dupPrintfSet() const {
      void* ptr = printfSet ? (void *)(new ir::PrintfSet(*printfSet)) : NULL;
      return ptr;
    }
    uint8_t getPrintfBufBTI() const {
      GBE_ASSERT(printfSet);
      return printfSet->getBufBTI();
    }

    uint8_t getPrintfIndexBufBTI() const {
      GBE_ASSERT(printfSet);
      return printfSet->getIndexBufBTI();
    }

    void outputPrintf(void* index_addr, void* buf_addr, size_t global_wk_sz0,
                      size_t global_wk_sz1, size_t global_wk_sz2) {
      if(printfSet)
        printfSet->outputPrintf(index_addr, buf_addr, global_wk_sz0,
                                global_wk_sz1, global_wk_sz2);
    }

    ir::FunctionArgument::InfoFromLLVM* getArgInfo(uint32_t id) const { return &args[id].info; }

    /*! Set compile work group size */
    void setCompileWorkGroupSize(const size_t wg_sz[3]) {
       compileWgSize[0] = wg_sz[0];
       compileWgSize[1] = wg_sz[1];
       compileWgSize[2] = wg_sz[2];
    }
    /*! Get compile work group size */
    void getCompileWorkGroupSize (size_t wg_sz[3]) const {
       wg_sz[0] = compileWgSize[0];
       wg_sz[1] = compileWgSize[1];
       wg_sz[2] = compileWgSize[2];
    }
    /*! Set function attributes string. */
    void setFunctionAttributes(const std::string& functionAttributes) {  this->functionAttributes= functionAttributes; }
    /*! Get function attributes string. */
    const char* getFunctionAttributes(void) const {return this->functionAttributes.c_str();}

    /*! Get defined image size */
    size_t getImageSize(void) const { return (imageSet == NULL ? 0 : imageSet->getDataSize()); }
    /*! Get defined image value array */
    void getImageData(ImageInfo *images) const { imageSet->getData(images); }

    static const uint32_t magic_begin = TO_MAGIC('K', 'E', 'R', 'N');
    static const uint32_t magic_end = TO_MAGIC('N', 'R', 'E', 'K');

    /* format:
       magic_begin       |
       name_size         |
       name              |
       arg_num           |
       args              |
       PatchInfo_num     |
       PatchInfo         |
       curbeSize         |
       simdWidth         |
       stackSize         |
       scratchSize       |
       useSLM            |
       slmSize           |
       samplers          |
       images            |
       code_size         |
       code              |
       magic_end
    */

    /*! Implements the serialization. */
    virtual size_t serializeToBin(std::ostream& outs);
    virtual size_t deserializeFromBin(std::istream& ins);
    virtual void printStatus(int indent, std::ostream& outs);

  protected:
    friend class Context;      //!< Owns the kernels
    friend class GenContext;
    std::string name;    //!< Kernel name
    KernelArgument *args;      //!< Each argument
    vector<PatchInfo> patches; //!< Indicates how to build the curbe
    uint32_t argNum;           //!< Number of function arguments
    uint32_t curbeSize;        //!< Size of the data to push
    uint32_t simdWidth;        //!< SIMD size for the kernel (lane number)
    uint32_t stackSize;        //!< Stack size (may be 0 if unused)
    uint32_t scratchSize;      //!< Scratch memory size (may be 0 if unused)
    bool useSLM;               //!< SLM requires a special HW config
    uint32_t slmSize;          //!< slm size for kernel variable
    Context *ctx;              //!< Save context after compiler to alloc constant buffer curbe
    ir::SamplerSet *samplerSet;//!< Copy from the corresponding function.
    ir::ImageSet *imageSet;    //!< Copy from the corresponding function.
    ir::PrintfSet *printfSet;  //!< Copy from the corresponding function.
    size_t compileWgSize[3];   //!< required work group size by kernel attribute.
    std::string functionAttributes; //!< function attribute qualifiers combined.
    GBE_CLASS(Kernel);         //!< Use custom allocators
  };

  /*! Describe a compiled program */
  class Program : public NonCopyable, public Serializable
  {
  public:
    /*! Create an empty program */
    Program(void);
    /*! Destroy the program */
    virtual ~Program(void);
    /*! Clean LLVM resource of the program */
    virtual void CleanLlvmResource() = 0;
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
    Kernel *getKernel(uint32_t ID) const {
      uint32_t currID = 0;
      Kernel *kernel = NULL;
      for (const auto &pair : kernels) {
        if (currID == ID) {
          kernel = pair.second;
          break;
        }
        currID++;
      }
      return kernel;
    }
    /*! Build a program from a ir::Unit */
    bool buildFromUnit(const ir::Unit &unit, std::string &error);
    /*! Buils a program from a LLVM source code */
    bool buildFromLLVMFile(const char *fileName, const void* module, std::string &error, int optLevel);
    /*! Buils a program from a OCL string */
    bool buildFromSource(const char *source, std::string &error);
    /*! Get size of the global constant arrays */
    size_t getGlobalConstantSize(void) const { return constantSet->getDataSize(); }
    /*! Get the content of global constant arrays */
    void getGlobalConstantData(char *mem) const { constantSet->getData(mem); }

    static const uint32_t magic_begin = TO_MAGIC('P', 'R', 'O', 'G');
    static const uint32_t magic_end = TO_MAGIC('G', 'O', 'R', 'P');

    /* format:
       magic_begin       |
       constantSet_flag  |
       constSet_data     |
       kernel_num        |
       kernel_1          |
       ........          |
       kernel_n          |
       magic_end         |
       total_size
    */

    /*! Implements the serialization. */
    virtual size_t serializeToBin(std::ostream& outs);
    virtual size_t deserializeFromBin(std::istream& ins);
    virtual void printStatus(int indent, std::ostream& outs);

  protected:
    /*! Compile a kernel */
    virtual Kernel *compileKernel(const ir::Unit &unit, const std::string &name, bool relaxMath) = 0;
    /*! Allocate an empty kernel. */
    virtual Kernel *allocateKernel(const std::string &name) = 0;
    /*! Kernels sorted by their name */
    hash_map<std::string, Kernel*> kernels;
    /*! Global (constants) outside any kernel */
    ir::ConstantSet *constantSet;
    /*! Use custom allocators */
    GBE_CLASS(Program);
  };

} /* namespace gbe */

#endif /* __GBE_PROGRAM_HPP__ */

