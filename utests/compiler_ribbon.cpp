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

#include "utest_helper.hpp"

static int *dst = NULL;
static const int w = 256;
static const int h = 256;

static void compiler_ribbon(void)
{
  const size_t global[2] = {size_t(w), size_t(h)};
  const size_t local[2] = {16, 1};
  const size_t sz = w * h * sizeof(char[4]);
  const float fx = float(w);
  const float fy = float(h);
  OCL_CREATE_KERNEL("compiler_ribbon");

  OCL_CREATE_BUFFER(buf[0], 0, sz, NULL);
  OCL_CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &buf[0]);
  OCL_CALL (clSetKernelArg, kernel, 1, sizeof(float), &fx);
  OCL_CALL (clSetKernelArg, kernel, 2, sizeof(float), &fy);
  OCL_CALL (clSetKernelArg, kernel, 3, sizeof(int), &w);
  OCL_CALL (clEnqueueNDRangeKernel, queue, kernel, 2, NULL, global, local, 0, NULL, NULL);
  OCL_MAP_BUFFER(0);
  dst = (int*) buf_data[0];

  /* Save the image (for debug purpose) */
  cl_write_bmp(dst, w, h, "compiler_ribbon.bmp");

  /* Compare with the golden image */
  OCL_CHECK_IMAGE(dst, w, h, "compiler_ribbon_ref.bmp");
}

MAKE_UTEST_FROM_FUNCTION(compiler_ribbon);

