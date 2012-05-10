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

static void compiler_short_scatter(void)
{
  const size_t n = 128;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_short_scatter");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int16_t), NULL);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Check result
  OCL_MAP_BUFFER(0);
  for (int32_t i = 0; i < (int32_t) n; ++i)
    OCL_ASSERT(((int16_t*)buf_data[0])[i] == (int16_t) i);
}

MAKE_UTEST_FROM_FUNCTION(compiler_short_scatter);


