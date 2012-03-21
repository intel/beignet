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
 * C-like interface for the gen kernels and programs
 */

#ifndef __GBE_GEN_PROGRAM_H__
#define __GBE_GEN_PROGRAM_H__

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! Opaque structure that interfaces a Gen program */
typedef struct GenProgram GenProgram;

/*! Opaque structure that interfaces a Gen kernel (ie one OCL function) */
typedef struct GenKernel GenKernel;

/*! Argument type for each function call */
enum GenArgType {
  GEN_ARG_VALUE = 0,            // int, float and so on
  GEN_ARG_GLOBAL_PTR = 1,       // __global, __constant
  GEN_ARG_STRUCTURE = 2,        // By value structure
  GEN_ARG_IMAGE = 3,            // image2d_t, image3d_t
  GEN_ARG_INVALID = 0xffffffff
};

/*! Create a new program from the given source code (zero terminated string) */
GenProgram *GenProgramNewFromSource(const char *source);

/*! Create a new program from the given blob */
GenProgram *GenProgramNewFromBinary(const char *binary, size_t size);

/*! Create a new program from the given LLVM file */
GenProgram *GenProgramNewFromLLVM(const char *fileName,
                                  size_t stringSize,
                                  char *err,
                                  size_t *errSize);

/*! Destroy and deallocate the given program */
void GenProgramDelete(GenProgram *program);

/*! Get the number of functions in the program */
uint32_t GenProgramGetKernelNum(const GenProgram *program);

/*! Get the kernel from its name */
const GenKernel *GenProgramGetKernelByName(const GenProgram *program, const char *name);

/*! Get the kernel from its ID */
const GenKernel *GenProgramGetKernel(const GenProgram *program, uint32_t ID);

/*! Get the Gen ISA source code */
const char *GenKernelGetCode(const GenKernel *kernel);

/*! Get the size of the source code */
const size_t GenKernelGetCodeSize(const GenKernel *kernel);

/*! Get the total number of arguments */
uint32_t GenKernelGetArgNum(const GenKernel *kernel);

/*! Get the size of the given argument */
uint32_t GenKernelGetArgSize(const GenKernel *kernel, uint32_t argID);

/*! Get the type of the given argument */
enum GenArgType GenKernelGetArgType(const GenKernel *kernel, uint32_t argID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GBE_GEN_PROGRAM_H__ */

