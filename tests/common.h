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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <CL/cl.h>
#include <CL/cl_intel.h>

#ifdef __cplusplus
extern "C" {
#endif

struct args  {
  int x, y, z;
  int W;                        /* -W nnn  workgroup size */
  int d;                        /* -d [cpu|gpu]  0 or 1 */
  int v;                        /* -v verify */
  int i;                        /* -i iteration */
};

#ifdef NDEBUG
#define IF_DEBUG(EXPR)
#else
#define IF_DEBUG(EXPR) EXPR
#endif

#define CHK_ERR(x) assert(x == CL_SUCCESS)

extern void usage();
extern void parseArgs(int argcc, char **argv, struct args * pargs);
extern void clpanic(const char *msg, int rval);
extern float randf();
extern float randf2(float lo, float hi);
extern cl_device_id getDeviceID(int devtype);
extern cl_kernel getKernel(cl_device_id dev, cl_context ctx,
                           const char *filename, const char *kernelname);
extern cl_kernel getKernelFromBinary(cl_device_id dev, cl_context ctx,
                                     const char *filename,
                                     const char *kernelname);
extern void *newBuffer(int bufsiz, int etype);
int comparef(const float *refData, const float *data, int n, float eps);
int *readBmp(const char *filename, int *width, int *height);
void writeBmp(const int *data, int width, int height, const char *filename);
char* readFulsimDump(const char *name, size_t *size);
extern char *do_kiss_path(const char *file, cl_device_id device);

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_H__ */

