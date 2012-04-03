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
 * \file program.h
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * C-like interface for the gen kernels and programs (either real Gen ISA or Gen
 * simulator)
 */

#ifndef __GBE_PROGRAM_H__
#define __GBE_PROGRAM_H__

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! Opaque structure that interfaces a GBE program */
typedef struct GBEProgram GBEProgram;

/*! Opaque structure that interfaces a GBE kernel (ie one OCL function) */
typedef struct GBEKernel GBEKernel;

/*! Argument type for each function call */
enum GBEArgType {
  GEN_ARG_VALUE = 0,            // int, float and so on
  GEN_ARG_GLOBAL_PTR = 1,       // __global, __constant
  GEN_ARG_STRUCTURE = 2,        // By value structure
  GEN_ARG_IMAGE = 3,            // image2d_t, image3d_t
  GEN_ARG_INVALID = 0xffffffff
};

/*! Create a new program from the given source code (zero terminated string) */
typedef GBEProgram *(GBEProgramNewFromSourceCB)(const char *source);
extern GBEProgramNewFromSourceCB *GBEProgramNewFromSource;

/*! Create a new program from the given blob */
typedef GBEProgram *(GBEProgramNewFromBinaryCB)(const char *binary, size_t size);
extern GBEProgramNewFromBinaryCB *GBEProgramNewFromBinary;

/*! Create a new program from the given LLVM file */
typedef GBEProgram *(GBEProgramNewFromLLVMCB)(const char *fileName,
                                              size_t stringSize,
                                              char *err,
                                              size_t *errSize);
extern GBEProgramNewFromLLVMCB *GBEProgramNewFromLLVM;

/*! Destroy and deallocate the given program */
typedef void (GBEProgramDeleteCB)(GBEProgram*);
extern GBEProgramDeleteCB *GBEProgramDelete;

/*! Get the number of functions in the program */
typedef uint32_t (GBEProgramGetKernelNumCB)(const GBEProgram*);
extern GBEProgramGetKernelNumCB *GBEProgramGetKernelNum;

/*! Get the kernel from its name */
typedef const GBEKernel *(GBEProgramGetKernelByNameCB)(const GBEProgram*, const char *name);
extern GBEProgramGetKernelByNameCB *GBEProgramGetKernelByName;

/*! Get the kernel from its ID */
typedef const GBEKernel *(GBEProgramGetKernelCB)(const GBEProgram*, uint32_t ID);
extern GBEProgramGetKernelCB *GBEProgramGetKernel;

/*! Get the GBE kernel name */
typedef const char *(GBEKernelGetNameCB)(const GBEKernel*);
extern GBEKernelGetNameCB *GBEKernelGetName;

/*! Get the GBE kernel source code */
typedef const char *(GBEKernelGetCodeCB)(const GBEKernel*);
extern GBEKernelGetCodeCB *GBEKernelGetCode;

/*! Get the size of the source code */
typedef const size_t (GBEKernelGetCodeSizeCB)(const GBEKernel*);
extern GBEKernelGetCodeSizeCB *GBEKernelGetCodeSize;

/*! Get the total number of arguments */
typedef uint32_t (GBEKernelGetArgNumCB)(const GBEKernel*);
extern GBEKernelGetArgNumCB *GBEKernelGetArgNum;

/*! Get the size of the given argument */
typedef uint32_t (GBEKernelGetArgSizeCB)(const GBEKernel*, uint32_t argID);
extern GBEKernelGetArgSizeCB *GBEKernelGetArgSize;

/*! Get the type of the given argument */
typedef enum GBEArgType (GBEKernelGetArgTypeCB)(const GBEKernel*, uint32_t argID);
extern GBEKernelGetArgTypeCB *GBEKernelGetArgType;

/*! Get the simd width for the kernel */
typedef uint32_t (GBEKernelGetSIMDWidthCB)(const GBEKernel*);
extern GBEKernelGetSIMDWidthCB *GBEKernelGetSIMDWidth;

/*! Indicates if a work group size is required. Return the required width or 0
 *  if none
 */
typedef uint32_t (GBEKernelGetRequiredWorkGroupSizeCB)(const GBEKernel*, uint32_t dim);
extern GBEKernelGetRequiredWorkGroupSizeCB *GBEKernelGetRequiredWorkGroupSize;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GBE_PROGRAM_H__ */

