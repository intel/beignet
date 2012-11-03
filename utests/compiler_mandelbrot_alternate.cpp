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
static const size_t w = 256;
static const size_t h = 256;
static const float criterium = 4.f;

static void compiler_mandelbrot_alternate(void)
{
  const size_t global[2] = {w, h};
  const size_t local[2] = {16, 1};
  const size_t sz = w * h * sizeof(char[4]);
  const float rcpWidth = 1.f / float(w);
  const float rcpHeight = 1.f / float(h);

  OCL_CREATE_KERNEL("compiler_mandelbrot_alternate");

  OCL_CREATE_BUFFER(buf[0], 0, sz, NULL);
  OCL_CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &buf[0]);
  OCL_CALL (clSetKernelArg, kernel, 1, sizeof(float), &rcpWidth);
  OCL_CALL (clSetKernelArg, kernel, 2, sizeof(float), &rcpHeight);
  OCL_CALL (clSetKernelArg, kernel, 3, sizeof(float), &criterium);
  OCL_CALL (clEnqueueNDRangeKernel, queue, kernel, 2, NULL, global, local, 0, NULL, NULL);
  OCL_MAP_BUFFER(0);
  dst = (int *) buf_data[0];

  /* Save the image (for debug purpose) */
  cl_write_bmp(dst, w, h, "compiler_mandelbrot_alternate.bmp");

  /* Compare with the golden image */
  OCL_CHECK_IMAGE(dst, w, h, "compiler_mandelbrot_alternate_ref.bmp");
}

MAKE_UTEST_FROM_FUNCTION(compiler_mandelbrot_alternate);

