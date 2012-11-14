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

static void compiler_local_memory_barrier(void)
{
  const size_t n = 1024;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_local_memory_barrier");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, 64, NULL); // 16 x int

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);

  // Check results
  uint32_t *dst = (uint32_t*)buf_data[0];
  for (uint32_t i = 0; i < n; i+=16)
  for (uint32_t j = 0; j < 16; ++j)
    OCL_ASSERT(dst[i+j] == 15-j);
}

MAKE_UTEST_FROM_FUNCTION(compiler_local_memory_barrier);

