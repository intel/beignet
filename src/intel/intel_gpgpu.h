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
 *         Alexei Soupikov <alexei.soupikov@intel.com>
 */

#ifndef __INTEL_GPGPU_H__
#define __INTEL_GPGPU_H__

#include "cl_utils.h"
#include "cl_driver.h"
#include "intel/intel_batchbuffer.h"
#include "intel/intel_driver.h"

#include <stdlib.h>
#include <stdint.h>


/* We can bind only a limited number of buffers */
enum { max_buf_n = 128 };

enum { max_img_n = 128};

enum {max_sampler_n = 16 };

struct intel_driver;
struct intel_batchbuffer;

/* Handle GPGPU state */
struct intel_gpgpu
{
  void* ker_opaque;
  size_t global_wk_sz[3];
  void* printf_info;
  struct intel_driver *drv;
  struct intel_batchbuffer *batch;
  cl_gpgpu_kernel *ker;
  drm_intel_bo *binded_buf[max_buf_n];  /* all buffers binded for the call */
  uint32_t target_buf_offset[max_buf_n];/* internal offset for buffers binded for the call */
  uint32_t binded_offset[max_buf_n];    /* their offsets in the curbe buffer */
  uint32_t binded_n;                    /* number of buffers binded */

  unsigned long img_bitmap;              /* image usage bitmap. */
  unsigned int img_index_base;          /* base index for image surface.*/

  unsigned long sampler_bitmap;          /* sampler usage bitmap. */

  struct { drm_intel_bo *bo; } stack_b;
  struct { drm_intel_bo *bo; } perf_b;
  struct { drm_intel_bo *bo; } scratch_b;
  struct { drm_intel_bo *bo; } constant_b;
  struct { drm_intel_bo *bo; } time_stamp_b;  /* time stamp buffer */
  struct { drm_intel_bo *bo;
           drm_intel_bo *ibo;} printf_b;      /* the printf buf and index buf*/

  struct { drm_intel_bo *bo; } aux_buf;
  struct {
    uint32_t surface_heap_offset;
    uint32_t curbe_offset;
    uint32_t idrt_offset;
    uint32_t sampler_state_offset;
    uint32_t sampler_border_color_state_offset;
  } aux_offset;

  uint32_t per_thread_scratch;
  struct {
    uint32_t num_cs_entries;
    uint32_t size_cs_entry;  /* size of one entry in 512bit elements */
  } curb;

  uint32_t max_threads;      /* max threads requested by the user */
};

struct intel_gpgpu_node {
  struct intel_gpgpu *gpgpu;
  struct intel_gpgpu_node *next;
};


/* Set the gpgpu related call backs */
extern void intel_set_gpgpu_callbacks(int device_id);

#endif /* __INTEL_GPGPU_H__ */

