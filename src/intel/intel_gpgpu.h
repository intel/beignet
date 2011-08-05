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

#ifndef __GENX_GPGPU_H__
#define __GENX_GPGPU_H__

#include "cl_utils.h"

#include <stdlib.h>
#include <stdint.h>

enum gen6_cache_control {
  cc_gtt        = 0x0,    /* don't use L3, use GTT for LLC */
  cc_mlc        = 0x1,    /* IVB: use L3, use GTT for LLC; SNB: UC */
  cc_llc        = 0x2,
  cc_llc_mlc    = 0x3,
};

#define MAX_SURFACES   255
#define MAX_SAMPLERS   16

/* Use this structure to bind kernels in the gpgpu state */
typedef struct genx_gpgpu_kernel {
  const char *name;        /* kernel name and bo name */
  uint32_t grf_blocks;     /* register blocks kernel wants (in 8 reg blocks) */
  uint32_t cst_sz;         /* total size of all constants */
  const uint32_t *bin;     /* binary code of the kernel */
  int32_t size;            /* kernel code size */
  struct _drm_intel_bo *bo;/* kernel code in the proper addr space */
  int32_t barrierID;       /* barrierID for _this_ kernel */
  uint32_t use_barrier:1;  /* For gen7 (automatic barrier management) */
  uint32_t thread_n:15;    /* For gen7 (automatic barrier management) */
  uint32_t slm_sz:16;      /* For gen7 (automatic SLM allocation) */
} genx_gpgpu_kernel_t;

/* Convenient abstraction of the device */
struct intel_driver;

/* Covenient way to talk to the device */
typedef struct intel_gpgpu intel_gpgpu_t;

/* Buffer object as exposed by drm_intel */
struct _drm_intel_bo;

/* Allocate and initialize a GPGPU state */
extern intel_gpgpu_t* intel_gpgpu_new(struct intel_driver*);

/* Destroy and deallocate a GPGPU state */
extern void intel_gpgpu_delete(intel_gpgpu_t*);

/* Get the device generation */
extern int32_t intel_gpgpu_version(intel_gpgpu_t*);

/* Set surface descriptor in the current binding table */
extern void gpgpu_bind_surf_2d(intel_gpgpu_t*,
                               int32_t index,
                               struct _drm_intel_bo* obj_bo,
                               uint32_t offset,
                               uint32_t format,
                               int32_t w,
                               int32_t h,
                               uint32_t is_dst,
                               int32_t vert_line_stride,
                               int32_t vert_line_stride_offset,
                               uint32_t cchint);

/* Set shared surface descriptor in the current binding table
 * Automatically determines and sets tiling mode which is transparently
 * supported by media block read/write
 */
extern void gpgpu_bind_shared_surf_2d(intel_gpgpu_t*,
                                      int32_t index,
                                      struct _drm_intel_bo* obj_bo,
                                      uint32_t offset,
                                      uint32_t format,
                                      int32_t w,
                                      int32_t h,
                                      uint32_t is_dst,
                                      int32_t vert_line_stride,
                                      int32_t vert_line_stride_offset,
                                      uint32_t cchint);

/* Set typeless buffer descriptor in the current binding table */
extern void gpgpu_bind_buf(intel_gpgpu_t*,
                           int32_t index,
                           struct _drm_intel_bo* obj_bo,
                           uint32_t offset,
                           uint32_t size,
                           uint32_t cchint);

/* Configure state, size in 512-bit units */
extern void gpgpu_state_init(intel_gpgpu_t*, uint32_t max_threads, uint32_t size_cs_entry);

/* Set the buffer object where to report performance counters */
extern void gpgpu_set_perf_counters(intel_gpgpu_t*, struct _drm_intel_bo *perf);

/* Fills current constant buffer with data */
extern void gpgpu_upload_constants(intel_gpgpu_t*, void* data, uint32_t size);

/* Setup all indirect states */
extern void gpgpu_states_setup(intel_gpgpu_t*, genx_gpgpu_kernel_t* kernel, uint32_t ker_n);

/* Make HW threads use barrierID */
extern void gpgpu_update_barrier(intel_gpgpu_t*, uint32_t barrierID, uint32_t thread_n);

/* Set a sampler (TODO: add other sampler fields) */
extern void gpgpu_set_sampler(intel_gpgpu_t*, uint32_t index, uint32_t non_normalized);

/* Allocate the batch buffer and return the BO used for the batch buffer */
extern void gpgpu_batch_reset(intel_gpgpu_t*, size_t sz);

/* Atomic begin, pipeline select, urb, pipeline state and constant buffer */
extern void gpgpu_batch_start(intel_gpgpu_t*);

/* atomic end with possibly inserted flush */
extern void gpgpu_batch_end(intel_gpgpu_t*, int32_t flush_mode);

/* Emit MI_FLUSH */
extern void gpgpu_flush(intel_gpgpu_t*);

/* Enqueue a MEDIA object with no inline data */
extern void gpgpu_run(intel_gpgpu_t*, int32_t ki);

/* Enqueue a MEDIA object with inline data to push afterward. Returns the
 * pointer where to push. sz is the size of the data we are going to pass
 */
extern char* gpgpu_run_with_inline(intel_gpgpu_t*, int32_t ki, size_t sz);

/* Will spawn all threads */
extern void
gpgpu_walker(intel_gpgpu_t *state,
             uint32_t thread_n,
             const size_t global_wk_off[3],
             const size_t global_wk_sz[3],
             const size_t local_wk_sz[3]);

#endif /* __GENX_GPGPU_H__ */

