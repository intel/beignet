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
#ifndef _INTEL_BATCHBUFFER_H_
#define _INTEL_BATCHBUFFER_H_

#include "intel_defines.h"
#include "cl_utils.h"

#include <xf86drm.h>
#include <drm.h>
#include <i915_drm.h>
#include <intel_bufmgr.h>
#include <stdint.h>
#include <memory.h>
#include <assert.h>

#define BEGIN_BATCH(b, n) do {                                            \
  intel_batchbuffer_require_space(b, (n) * 4);                            \
} while (0)

#define OUT_BATCH(b, d) do {                                              \
  intel_batchbuffer_emit_dword(b, d);                                     \
} while (0)

#define OUT_RELOC(b, bo, read_domains, write_domain, delta) do {          \
  assert((delta) >= 0);                                                   \
  intel_batchbuffer_emit_reloc(b, bo, read_domains, write_domain, delta); \
} while (0)

#define ADVANCE_BATCH(b) do { } while (0)

struct intel_driver;

typedef struct intel_batchbuffer
{
  struct intel_driver *intel;
  drm_intel_bo *buffer;
  /** Last bo submitted to the hardware.  used for clFinish. */
  drm_intel_bo *last_bo;
  uint32_t size;
  uint8_t *map;
  uint8_t *ptr;
  /** HSW: can't set LRI in batch buffer, set I915_EXEC_ENABLE_SLM
   *  flag when call exec. */
  uint8_t enable_slm;
  int atomic;
} intel_batchbuffer_t;

extern intel_batchbuffer_t* intel_batchbuffer_new(struct intel_driver*);
extern void intel_batchbuffer_delete(intel_batchbuffer_t*);
extern void intel_batchbuffer_emit_reloc(intel_batchbuffer_t*,
                                         drm_intel_bo*,
                                         uint32_t read_domains,
                                         uint32_t write_domains,
                                         uint32_t delta);
extern void intel_batchbuffer_emit_mi_flush(intel_batchbuffer_t*);
extern void intel_batchbuffer_init(intel_batchbuffer_t*, struct intel_driver*);
extern void intel_batchbuffer_terminate(intel_batchbuffer_t*);
extern void intel_batchbuffer_flush(intel_batchbuffer_t*);
extern int intel_batchbuffer_reset(intel_batchbuffer_t*, size_t sz);

static INLINE uint32_t
intel_batchbuffer_space(const intel_batchbuffer_t *batch)
{
  assert(batch->ptr);
  return batch->size - (batch->ptr - batch->map);
}

static INLINE void
intel_batchbuffer_emit_dword(intel_batchbuffer_t *batch, uint32_t x)
{
  assert(intel_batchbuffer_space(batch) >= 4);
  *(uint32_t*)batch->ptr = x;
  batch->ptr += 4;
}

static INLINE void
intel_batchbuffer_require_space(intel_batchbuffer_t *batch, uint32_t size) {
  assert(size < batch->size - 8);
  if (intel_batchbuffer_space(batch) < size)
    intel_batchbuffer_space(batch);
}

static INLINE uint8_t*
intel_batchbuffer_alloc_space(intel_batchbuffer_t *batch, uint32_t size)
{
  assert(intel_batchbuffer_space(batch) >= size);
  uint8_t *space_ptr = batch->ptr;
  batch->ptr += size;
  return space_ptr;
}

static INLINE void
intel_batchbuffer_start_atomic(intel_batchbuffer_t *batch, uint32_t size)
{
  assert(!batch->atomic);
  intel_batchbuffer_require_space(batch, size);
  batch->atomic = 1;
}

static INLINE void
intel_batchbuffer_end_atomic(intel_batchbuffer_t *batch)
{
  assert(batch->atomic);
  batch->atomic = 0;
}

#endif /* _INTEL_BATCHBUFFER_H_ */

