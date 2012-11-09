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

#include <assert.h>
#include <stdio.h>
#include <string.h>

static INLINE size_t cl_kernel_compute_batch_sz(cl_kernel k) { return 256+32; }

/* "Varing" payload is the part of the curbe that changes accross threads in the
 *  same work group. Right now, it consists in local IDs and block IPs
 */
static cl_int
cl_set_varying_payload(const cl_kernel ker,
                       char *data,
                       const size_t *local_wk_sz,
                       size_t simd_sz,
                       size_t cst_sz,
                       size_t thread_n)
{
  uint32_t *ids[3] = {NULL,NULL,NULL};
  uint16_t *block_ips = NULL;
  size_t i, j, k, curr = 0;
  int32_t id_offset[3], ip_offset;
  cl_int err = CL_SUCCESS;

  id_offset[0] = gbe_kernel_get_curbe_offset(ker->opaque, GBE_CURBE_LOCAL_ID_X, 0);
  id_offset[1] = gbe_kernel_get_curbe_offset(ker->opaque, GBE_CURBE_LOCAL_ID_Y, 0);
  id_offset[2] = gbe_kernel_get_curbe_offset(ker->opaque, GBE_CURBE_LOCAL_ID_Z, 0);
  ip_offset = gbe_kernel_get_curbe_offset(ker->opaque, GBE_CURBE_BLOCK_IP, 0);
  assert(id_offset[0] >= 0 &&
         id_offset[1] >= 0 &&
         id_offset[2] >= 0 &&
         ip_offset >= 0);

  TRY_ALLOC(ids[0], (uint32_t*) alloca(sizeof(uint32_t)*thread_n*simd_sz));
  TRY_ALLOC(ids[1], (uint32_t*) alloca(sizeof(uint32_t)*thread_n*simd_sz));
  TRY_ALLOC(ids[2], (uint32_t*) alloca(sizeof(uint32_t)*thread_n*simd_sz));
  TRY_ALLOC(block_ips, (uint16_t*) alloca(sizeof(uint16_t)*thread_n*simd_sz));

  /* 0xffff means that the lane is inactivated */
  memset(block_ips, 0xff, sizeof(uint16_t)*thread_n*simd_sz);

  /* Compute the IDs and the block IPs */
  for (k = 0; k < local_wk_sz[2]; ++k)
  for (j = 0; j < local_wk_sz[1]; ++j)
  for (i = 0; i < local_wk_sz[0]; ++i, ++curr) {
    ids[0][curr] = i;
    ids[1][curr] = j;
    ids[2][curr] = k;
    block_ips[curr] = 0;
  }

  /* Copy them to the constant buffer */
  curr = 0;
  for (i = 0; i < thread_n; ++i, data += cst_sz) {
    uint32_t *ids0 = (uint32_t *) (data + id_offset[0]);
    uint32_t *ids1 = (uint32_t *) (data + id_offset[1]);
    uint32_t *ids2 = (uint32_t *) (data + id_offset[2]);
    uint16_t *ips  = (uint16_t *) (data + ip_offset);
    for (j = 0; j < simd_sz; ++j, ++curr) {
      ids0[j] = ids[0][curr];
      ids1[j] = ids[1][curr];
      ids2[j] = ids[2][curr];
      ips[j] = block_ips[curr];
    }
  }

error:
  return err;
}

static void
cl_curbe_fill(cl_kernel ker,
              const size_t *global_wk_off,
              const size_t *global_wk_sz,
              const size_t *local_wk_sz)
{
  int32_t offset;
#define UPLOAD(ENUM, VALUE) \
  if ((offset = gbe_kernel_get_curbe_offset(ker->opaque, ENUM, 0)) >= 0) \
    *((uint32_t *) (ker->curbe + offset)) = VALUE;
  UPLOAD(GBE_CURBE_LOCAL_SIZE_X, local_wk_sz[0]);
  UPLOAD(GBE_CURBE_LOCAL_SIZE_Y, local_wk_sz[1]);
  UPLOAD(GBE_CURBE_LOCAL_SIZE_Z, local_wk_sz[2]);
  UPLOAD(GBE_CURBE_GLOBAL_SIZE_X, global_wk_sz[0]);
  UPLOAD(GBE_CURBE_GLOBAL_SIZE_Y, global_wk_sz[1]);
  UPLOAD(GBE_CURBE_GLOBAL_SIZE_Z, global_wk_sz[2]);
  UPLOAD(GBE_CURBE_GLOBAL_OFFSET_X, global_wk_off[0]);
  UPLOAD(GBE_CURBE_GLOBAL_OFFSET_Y, global_wk_off[1]);
  UPLOAD(GBE_CURBE_GLOBAL_OFFSET_Z, global_wk_off[2]);
  UPLOAD(GBE_CURBE_GROUP_NUM_X, global_wk_sz[0]/local_wk_sz[0]);
  UPLOAD(GBE_CURBE_GROUP_NUM_Y, global_wk_sz[1]/local_wk_sz[1]);
  UPLOAD(GBE_CURBE_GROUP_NUM_Z, global_wk_sz[2]/local_wk_sz[2]);
#undef UPLOAD

  /* Write identity for the stack pointer. This is required by the stack pointer
   * computation in the kernel
   */
  if ((offset = gbe_kernel_get_curbe_offset(ker->opaque, GBE_CURBE_STACK_POINTER, 0)) >= 0) {
    const uint32_t simd_sz = gbe_kernel_get_simd_width(ker->opaque);
    uint32_t *stackptr = (uint32_t *) (ker->curbe + offset);
    int32_t i;
    for (i = 0; i < (int32_t) simd_sz; ++i) stackptr[i] = i;
  }
}

static void
cl_bind_stack(cl_gpgpu gpgpu, cl_kernel ker)
{
  cl_context ctx = ker->program->ctx;
  cl_device_id device = ctx->device;
  const int32_t per_lane_stack_sz = gbe_kernel_get_stack_size(ker->opaque);
  const int32_t value = GBE_CURBE_EXTRA_ARGUMENT;
  const int32_t sub_value = GBE_STACK_BUFFER;
  const int32_t offset = gbe_kernel_get_curbe_offset(ker->opaque, value, sub_value);
  int32_t stack_sz = per_lane_stack_sz;

  /* No stack required for this kernel */
  if (per_lane_stack_sz == 0)
    return;

  /* The stack size is given for *each* SIMD lane. So, we accordingly compute
   * the size we need for the complete machine
   */
  assert(offset >= 0);
  stack_sz *= gbe_kernel_get_simd_width(ker->opaque);
  stack_sz *= device->max_compute_unit;
  stack_sz *= device->max_thread_per_unit;
  cl_gpgpu_set_stack(gpgpu, offset, stack_sz, cc_llc_l3);
}

LOCAL cl_int
cl_command_queue_ND_range_gen7(cl_command_queue queue,
                               cl_kernel ker,
                               const size_t *global_wk_off,
                               const size_t *global_wk_sz,
                               const size_t *local_wk_sz)
{
  cl_context ctx = queue->ctx;
  cl_gpgpu gpgpu = queue->gpgpu;
  char *final_curbe = NULL;  /* Includes them and one sub-buffer per group */
  cl_gpgpu_kernel kernel;
  const uint32_t simd_sz = cl_kernel_get_simd_width(ker);
  size_t i, batch_sz = 0u, local_sz = 0u, cst_sz = ker->curbe_sz;
  size_t thread_n = 0u;
  cl_int err = CL_SUCCESS;

  /* Setup kernel */
  kernel.name = "KERNEL";
  kernel.grf_blocks = 128;
  kernel.bo = ker->bo;
  kernel.barrierID = 0;
  kernel.use_barrier = 0;
  kernel.slm_sz = 0;

  /* Curbe step 1: fill the constant buffer data shared by all threads */
  if (ker->curbe)
    cl_curbe_fill(ker, global_wk_off, global_wk_sz, local_wk_sz);

  /* Compute the number of HW threads we need */
  TRY (cl_kernel_work_group_sz, ker, local_wk_sz, 3, &local_sz);
  kernel.thread_n = thread_n = local_sz / simd_sz;
  kernel.cst_sz = cst_sz;

  /* Setup the kernel */
  cl_gpgpu_state_init(gpgpu, ctx->device->max_compute_unit, cst_sz / 32);
  if (queue->last_batch != NULL)
    cl_buffer_unreference(queue->last_batch);
  queue->last_batch = NULL;

  /* Bind user buffers */
  cl_command_queue_bind_surface(queue, ker);

  /* Bind a stack if needed */
  cl_bind_stack(gpgpu, ker);
  cl_gpgpu_states_setup(gpgpu, &kernel);

  /* Curbe step 2. Give the localID and upload it to video memory */
  if (ker->curbe) {
    assert(cst_sz > 0);
    TRY_ALLOC (final_curbe, (char*) alloca(thread_n * cst_sz));
      for (i = 0; i < thread_n; ++i)
        memcpy(final_curbe + cst_sz * i, ker->curbe, cst_sz);
    TRY (cl_set_varying_payload, ker, final_curbe, local_wk_sz, simd_sz, cst_sz, thread_n);
    cl_gpgpu_upload_constants(gpgpu, final_curbe, thread_n*cst_sz);
  }

  /* Start a new batch buffer */
  batch_sz = cl_kernel_compute_batch_sz(ker);
  cl_gpgpu_batch_reset(gpgpu, batch_sz);
  cl_gpgpu_batch_start(gpgpu);

  /* Issue the GPGPU_WALKER command */
  cl_gpgpu_walker(gpgpu, simd_sz, thread_n, global_wk_off, global_wk_sz, local_wk_sz);

  /* Close the batch buffer and submit it */
  cl_gpgpu_batch_end(gpgpu, 0);
  cl_gpgpu_flush(gpgpu);

error:
  return err;
}

