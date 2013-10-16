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

/* This is a super simple wrapper for the OpenCL kernels I ported from GLSL code
 * taken in Inigo's web site:
 * http://www.iquilezles.org/apps/shadertoy/index.html
 *
 * They are pretty cool and rather complex kernels. Just the right thing to have
 * something a bit more complicated and interesting than unit tests.
 *
 * The code here is just to wrap the common code used by all the kernels (to run
 * the code and assert its correctness)
 */
#include "utest_helper.hpp"

static const int dim = 256;

// tricky here 'name' stands for Kernel and Reference
// 'file' stands for .cl file name and dst image name
static void run_kernel(int w, int h, const char *file, const char *name)
{
  const size_t global[2] = {size_t(w), size_t(h)};
  const size_t local[2] = {16, 1};
  const size_t sz = w * h * sizeof(char[4]);
  const float fx = float(w);
  const float fy = float(h);
  char kernel_file[256];
  char dst_img[256];
  char ref_img[256];

  snprintf(kernel_file, sizeof(kernel_file), "%s.cl", file);
  snprintf(dst_img, sizeof(dst_img), "%s.bmp", file);
  snprintf(ref_img, sizeof(ref_img), "%s_ref.bmp", name);
  OCL_CALL (cl_kernel_init, kernel_file, name, SOURCE, NULL);

  OCL_CREATE_BUFFER(buf[0], 0, sz, NULL);
  OCL_CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &buf[0]);
  OCL_CALL (clSetKernelArg, kernel, 1, sizeof(float), &fx);
  OCL_CALL (clSetKernelArg, kernel, 2, sizeof(float), &fy);
  OCL_CALL (clSetKernelArg, kernel, 3, sizeof(int), &w);
  OCL_CALL (clEnqueueNDRangeKernel, queue, kernel, 2, NULL, global, local, 0, NULL, NULL);
  OCL_MAP_BUFFER(0);
  int *dst = (int*) buf_data[0];

  /* Save the image (for debug purpose) */
  cl_write_bmp(dst, w, h, dst_img);

  /* Compare with the golden image */
  OCL_CHECK_IMAGE(dst, w, h, ref_img);
}

#define DECL_SHADER_TOY_TEST(W,H,FILE_NAME, KERNEL_NAME) \
  static void FILE_NAME(void) { run_kernel(W,H,#FILE_NAME, #KERNEL_NAME); } \
  MAKE_UTEST_FROM_FUNCTION(FILE_NAME);

DECL_SHADER_TOY_TEST(dim,dim,compiler_clod,compiler_clod);
DECL_SHADER_TOY_TEST(dim,dim,compiler_ribbon,compiler_ribbon);
DECL_SHADER_TOY_TEST(dim,dim,compiler_nautilus,compiler_nautilus);
DECL_SHADER_TOY_TEST(dim,dim,compiler_menger_sponge_no_shadow,compiler_menger_sponge_no_shadow);
DECL_SHADER_TOY_TEST(dim,dim,compiler_julia,compiler_julia);
DECL_SHADER_TOY_TEST(dim,dim,compiler_julia_no_break,compiler_julia_no_break);
// test for function calls
DECL_SHADER_TOY_TEST(dim,dim,compiler_clod_function_call,compiler_clod);
DECL_SHADER_TOY_TEST(dim,dim,compiler_julia_function_call,compiler_julia);

// Still issues here for LLVM 3.2
// DECL_SHADER_TOY_TEST(dim,dim,compiler_chocolux,compiler_chocolux);
// DECL_SHADER_TOY_TEST(dim,dim,compiler_menger_sponge,compiler_menger_sponge);

#undef DECL_SHADER_TOY_TEST

