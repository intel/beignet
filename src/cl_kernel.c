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

#include "cl_kernel.h"
#include "cl_program.h"
#include "cl_device_id.h"
#include "cl_context.h"
#include "cl_mem.h"
#include "cl_alloc.h"
#include "cl_utils.h"

#include "CL/cl.h"

#ifdef _PLASMA
#include "plasma/plasma_export.h"
#else
#include "intel_bufmgr.h"
#include "intel/intel_gpgpu.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

LOCAL void
cl_kernel_delete(cl_kernel k)
{
  if (k == NULL)
    return;

  /* We are not done with the kernel */
  if (atomic_dec(&k->ref_n) > 1) return;

  /* Release one reference on all bos we own */
  if (k->bo)       drm_intel_bo_unreference(k->bo);
  if (k->const_bo) drm_intel_bo_unreference(k->const_bo);

  /* This will be true for kernels created by clCreateKernel */
  if (k->ref_its_program) cl_program_delete(k->program);

  k->magic = CL_MAGIC_DEAD_HEADER; /* For safety */
  cl_free(k);
}

LOCAL cl_kernel
cl_kernel_new(void)
{
  cl_kernel k = NULL;
  TRY_ALLOC_NO_ERR (k, CALLOC(struct _cl_kernel));
  k->ref_n = 1;
  k->magic = CL_MAGIC_KERNEL_HEADER;

exit:
  return k;
error:
  cl_kernel_delete(k);
  k = NULL;
  goto exit;
}

LOCAL void
cl_kernel_add_ref(cl_kernel k)
{
  atomic_inc(&k->ref_n);
}

LOCAL cl_int
cl_kernel_set_arg(cl_kernel k, cl_uint index, size_t sz, const void *value)
{
  cl_int err = CL_SUCCESS;

  return err;
}

