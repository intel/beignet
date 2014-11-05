/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

/**************************************************************************
 * 
 * Copyright 2006 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

#include "intel/intel_batchbuffer.h"
#include "intel/intel_driver.h"
#include "cl_alloc.h"
#include "cl_utils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

LOCAL int
intel_batchbuffer_reset(intel_batchbuffer_t *batch, size_t sz)
{
  if (batch->buffer != NULL) {
    dri_bo_unreference(batch->buffer);
    batch->buffer = NULL;
    batch->last_bo = NULL;
  }

  batch->buffer = dri_bo_alloc(batch->intel->bufmgr,
                               "batch buffer",
                               sz,
                               64);
  if (!batch->buffer || (dri_bo_map(batch->buffer, 1) != 0)) {
    if (batch->buffer)
      dri_bo_unreference(batch->buffer);
    batch->buffer = NULL;
    return -1;
  }
  batch->map = (uint8_t*) batch->buffer->virtual;
  batch->size = sz;
  batch->ptr = batch->map;
  batch->atomic = 0;
  batch->last_bo = batch->buffer;
  batch->enable_slm = 0;
  return 0;
}

LOCAL void
intel_batchbuffer_init(intel_batchbuffer_t *batch, intel_driver_t *intel)
{
  assert(intel);
  batch->intel = intel;
}

LOCAL void
intel_batchbuffer_terminate(intel_batchbuffer_t *batch)
{
  assert(batch->buffer);

  if (batch->map) {
    dri_bo_unmap(batch->buffer);
    batch->map = NULL;
  }

  dri_bo_unreference(batch->buffer);
  batch->buffer = NULL;
}

LOCAL void
intel_batchbuffer_flush(intel_batchbuffer_t *batch)
{
  uint32_t used = batch->ptr - batch->map;
  int is_locked = batch->intel->locked;

  if (used == 0)
    return;

  if ((used & 4) == 0) {
    *(uint32_t*) batch->ptr = 0;
    batch->ptr += 4;
  }

  *(uint32_t*)batch->ptr = MI_BATCH_BUFFER_END;
  batch->ptr += 4;
  used = batch->ptr - batch->map;
  dri_bo_unmap(batch->buffer);
  batch->ptr = batch->map = NULL;

  if (!is_locked)
    intel_driver_lock_hardware(batch->intel);

  int flag = I915_EXEC_RENDER;
  if(batch->enable_slm) {
    /* use the hard code here temp, must change to
     * I915_EXEC_ENABLE_SLM when it drm accept the patch */
    flag |= (1<<13);
  }
  drm_intel_gem_bo_context_exec(batch->buffer, batch->intel->ctx, used, flag);

  if (!is_locked)
    intel_driver_unlock_hardware(batch->intel);

  // Can't release buffer here. gpgpu only can be delete only when the batch buffer is complete.
  // Remain the buffer for gpgpu delete check.
  //intel_batchbuffer_terminate(batch);
}

LOCAL void 
intel_batchbuffer_emit_reloc(intel_batchbuffer_t *batch,
                             dri_bo *bo, 
                             uint32_t read_domains,
                             uint32_t write_domains, 
                             uint32_t delta)
{
  assert(batch->ptr - batch->map < batch->size);
  dri_bo_emit_reloc(batch->buffer,
                    read_domains,
                    write_domains,
                    delta,
                    batch->ptr - batch->map,
                    bo);
  intel_batchbuffer_emit_dword(batch, bo->offset + delta);
}

LOCAL void
intel_batchbuffer_emit_mi_flush(intel_batchbuffer_t *batch)
{
  intel_batchbuffer_require_space(batch, 4);
  intel_batchbuffer_emit_dword(batch, MI_FLUSH | STATE_INSTRUCTION_CACHE_INVALIDATE);
}

LOCAL intel_batchbuffer_t*
intel_batchbuffer_new(intel_driver_t *intel)
{
  intel_batchbuffer_t *batch = NULL;
  assert(intel);
  TRY_ALLOC_NO_ERR (batch, CALLOC(intel_batchbuffer_t));
  intel_batchbuffer_init(batch, intel);

exit:
  return batch;
error:
  intel_batchbuffer_delete(batch);
  batch = NULL;
  goto exit;
}

LOCAL void
intel_batchbuffer_delete(intel_batchbuffer_t *batch)
{
  if (batch == NULL)
    return;
  if(batch->buffer)
    intel_batchbuffer_terminate(batch);

  cl_free(batch);
}
