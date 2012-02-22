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
 * \file utest_llvm.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * Compile llvm kernels to gen back-end kernels
 */

#include "utest/utest.hpp"
#include <cstdlib>

// Transform some llvm code to gen code
extern "C" int llvmToGen(int argc, char **argv);

/*! Where the kernels to test are */
static std::string kernelPath;

char *copyString(const char *src) {
  const size_t len = strlen(src);
  char *dst = GBE_NEW_ARRAY(char, len + 1);
  std::memcpy(dst, src, len);
  dst[len] = 0;
  return dst;
}

static void utestLLVM2Gen(const char *kernel)
{
  const std::string path = kernelPath + kernel;
  char *toFree[] = {
    copyString(""),
    copyString("-march=gen"),
    copyString(path.c_str())
  };
  char *argv[] = {toFree[0], toFree[1], toFree[2]};
  llvmToGen(3, argv);
  GBE_DELETE_ARRAY(toFree[0]);
  GBE_DELETE_ARRAY(toFree[1]);
  GBE_DELETE_ARRAY(toFree[2]);
}

static void utestLLVM(void)
{
  // Try to find where the kernels are
  FILE *dummyKernel = NULL;
#define TRY_PATH(PATH)                                      \
  if ((dummyKernel = fopen(PATH"dummy.ll", "r")) != NULL) { \
    kernelPath = PATH;                                      \
    goto runTests;                                          \
  }
  TRY_PATH("./kernels/")
  TRY_PATH("../kernels/")
  TRY_PATH("../../kernels/")
  TRY_PATH("../../../kernels/")
#undef TRY_PATH

  std::cout << "Failed to find valid kernel path" << std::endl;
  return;

runTests:
  std::cout << "  kernel path is: \"" << kernelPath << "\"" << std::endl;
  GBE_ASSERT(dummyKernel != NULL);
  fclose(dummyKernel);

  UTEST_EXPECT_SUCCESS(utestLLVM2Gen("void.ll"));
}

UTEST_REGISTER(utestLLVM)

