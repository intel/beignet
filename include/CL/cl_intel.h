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

#ifndef __OPENCL_CL_INTEL_H
#define __OPENCL_CL_INTEL_H

#include "CL/cl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CL_MEM_PINNABLE (1 << 10)

/* Track allocations and report current number of unfreed allocations */
extern CL_API_ENTRY cl_int CL_API_CALL
clIntelReportUnfreed(void);

/* 1 to 1 mapping of drm_intel_bo_map */
extern CL_API_ENTRY void* CL_API_CALL
clIntelMapBuffer(cl_mem, cl_int*);

/* 1 to 1 mapping of drm_intel_bo_unmap */
extern CL_API_ENTRY cl_int CL_API_CALL
clIntelUnmapBuffer(cl_mem);

/* Pin /Unpin the buffer in GPU memory (must be root) */
extern CL_API_ENTRY cl_int CL_API_CALL
clIntelPinBuffer(cl_mem);
extern CL_API_ENTRY cl_int CL_API_CALL
clIntelUnpinBuffer(cl_mem);

/* Set the buffer where to report the performance counters. If NULL, nothing
 * will be report
 */
extern CL_API_ENTRY cl_int CL_API_CALL
clIntelSetReportBuffer(cl_command_queue, cl_mem);

#ifdef __cplusplus
}
#endif

#endif /* __OPENCL_CL_INTEL_H */

