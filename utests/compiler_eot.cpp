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

#include "cl_test.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  const size_t n = 1024;
  const size_t global_work_size = n;
  const size_t local_work_size = 16;
  int status = 0;

  if ((status = cl_test_init("dummy.ll", "hop", LLVM)) != 0) goto error;

  /* Run the kernel */
  CALL (clEnqueueNDRangeKernel, queue,
                                kernel,
                                1,
                                NULL,
                                &global_work_size,
                                &local_work_size,
                                0,
                                NULL,
                                NULL);

  cl_test_destroy();
  printf("%i memory leaks\n", clIntelReportUnfreed());
  assert(clIntelReportUnfreed() == 0);

error:
  cl_report_error(status);
  return status;
}

