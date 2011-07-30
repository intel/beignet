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

#include "cl_command_queue.h"
#include "cl_context.h"
#include "cl_program.h"
#include "cl_kernel.h"
#include "cl_device_id.h"
#include "cl_mem.h"
#include "cl_utils.h"
#include "cl_alloc.h"

#include "intel_bufmgr.h"
#include "intel/intel_gpgpu.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

LOCAL cl_int
cl_command_queue_ND_range_gen7(cl_command_queue queue,
                                cl_kernel ker,
                                cl_uint wk_dim,
                                const size_t *global_work_offset,
                                const size_t *global_wk_sz,
                                const size_t *local_wk_sz)
{
#if 0
  cl_context ctx = queue->ctx;
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  drm_intel_bo *private_bo = NULL, *scratch_bo = NULL;
  genx_gpgpu_kernel_t kernel;
  const size_t cst_sz = ker->patch.curbe.sz;
#endif

  return CL_SUCCESS;
}

