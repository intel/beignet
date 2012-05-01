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

#ifndef __CL_TEST_H__
#define __CL_TEST_H__

#include "CL/cl.h"
#include "common.h"

#define TEST_SIMD8 0
#define CALL(FN, ...)                             \
  do {                                            \
    status = FN(__VA_ARGS__);                     \
    if (status != CL_SUCCESS) {                   \
      fprintf(stderr, "error calling %s\n", #FN); \
      goto error;                                 \
    }                                             \
  } while (0)

#ifdef __cplusplus
extern "C" {
#endif

extern cl_platform_id platform;
extern cl_device_id device;
extern cl_context ctx;
extern cl_program program;
extern cl_kernel kernel;
extern cl_command_queue queue;

enum  {
  SOURCE = 0,
  LLVM = 1,
  BIN = 2
};

/* Init the bunch of global varaibles here */
extern int cl_test_init(const char *file_name, const char *kernel_name, int format);

/* Release everything allocated in cl_test_init */
extern void cl_test_destroy(void);

/* Properly report the error in stderr */
extern void cl_report_error(cl_int err);

/* Nicely output the performance counters */
extern void cl_report_perf_counters(cl_mem perf);

#ifdef __cplusplus
}
#endif

#endif /* __CL_TEST_H__ */

