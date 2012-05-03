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

static void compiler_copy_buffer(void)
{
  const size_t n = 8192 * 4;
  int status = 0;

  CALL (cl_test_init, "test_copy_buffer.cl", "test_copy_buffer", SOURCE);
  buf_data[0] = (uint32_t*) malloc(sizeof(uint32_t) * n);
  for (uint32_t i = 0; i < n; ++i) ((uint32_t*)buf_data[0])[i] = i;
  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, n * sizeof(uint32_t), buf_data[0]);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  free(buf_data[0]);
  buf_data[0] = NULL;

  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < n; ++i)
    assert(((uint32_t*)buf_data[0])[i] == ((uint32_t*)buf_data[1])[i]);
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

error:
  cl_release_buffers();
  cl_report_error(status);
  cl_test_destroy();
}

UTEST_REGISTER(compiler_copy_buffer)

