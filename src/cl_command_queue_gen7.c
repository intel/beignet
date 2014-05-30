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

  /* Copy them to the curbe buffer */
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
cl_upload_constant_buffer(cl_command_queue queue, cl_kernel ker)
{
  /* calculate constant buffer size
   * we need raw_size & aligned_size
   */
  GET_QUEUE_THREAD_GPGPU(queue);
  int32_t arg;
  size_t offset = 0;
  uint32_t raw_size = 0, aligned_size =0;
  gbe_program prog = ker->program->opaque;
  const int32_t arg_n = gbe_kernel_get_arg_num(ker->opaque);
  size_t global_const_size = gbe_program_get_global_constant_size(prog);
  aligned_size = raw_size = global_const_size;
  /* Reserve 8 bytes to get rid of 0 address */
  if(global_const_size == 0) aligned_size = 8;

  for (arg = 0; arg < arg_n; ++arg) {
    const enum gbe_arg_type type = gbe_kernel_get_arg_type(ker->opaque, arg);
    if (type == GBE_ARG_CONSTANT_PTR && ker->args[arg].mem) {
      uint32_t alignment = gbe_kernel_get_arg_align(ker->opaque, arg);
      assert(alignment != 0);
      cl_mem mem = ker->args[arg].mem;
      raw_size += mem->size;
      aligned_size = ALIGN(aligned_size, alignment);
      aligned_size += mem->size;
    }
  }
  if(raw_size == 0)
     return;

  cl_buffer bo = cl_gpgpu_alloc_constant_buffer(gpgpu, aligned_size);
  cl_buffer_map(bo, 1);
  char * cst_addr = cl_buffer_get_virtual(bo);

  /* upload the global constant data */
  if (global_const_size > 0) {
    gbe_program_get_global_constant_data(prog, (char*)(cst_addr+offset));
    offset += global_const_size;
  }

  /* reserve 8 bytes to get rid of 0 address */
  if(global_const_size == 0) {
    offset = 8;
  }

  /* upload constant buffer argument */
  int32_t curbe_offset = 0;
  for (arg = 0; arg < arg_n; ++arg) {
    const enum gbe_arg_type type = gbe_kernel_get_arg_type(ker->opaque, arg);
    if (type == GBE_ARG_CONSTANT_PTR && ker->args[arg].mem) {
      cl_mem mem = ker->args[arg].mem;
      uint32_t alignment = gbe_kernel_get_arg_align(ker->opaque, arg);
      offset = ALIGN(offset, alignment);
      curbe_offset = gbe_kernel_get_curbe_offset(ker->opaque, GBE_CURBE_KERNEL_ARGUMENT, arg);
      assert(curbe_offset >= 0);
      *(uint32_t *) (ker->curbe + curbe_offset) = offset;

      cl_buffer_map(mem->bo, 1);
      void * addr = cl_buffer_get_virtual(mem->bo);
      memcpy(cst_addr + offset, addr, mem->size);
      cl_buffer_unmap(mem->bo);
      offset += mem->size;
    }
  }
  cl_buffer_unmap(bo);
}

/* Will return the total amount of slm used */
static int32_t
cl_curbe_fill(cl_kernel ker,
              const uint32_t work_dim,
              const size_t *global_wk_off,
              const size_t *global_wk_sz,
              const size_t *local_wk_sz,
              size_t thread_n)
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
  UPLOAD(GBE_CURBE_THREAD_NUM, thread_n);
  UPLOAD(GBE_CURBE_WORK_DIM, work_dim);
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
  /* Handle the various offsets to SLM */
  const int32_t arg_n = gbe_kernel_get_arg_num(ker->opaque);
  int32_t arg, slm_offset = gbe_kernel_get_slm_size(ker->opaque);
  ker->local_mem_sz = 0;
  for (arg = 0; arg < arg_n; ++arg) {
    const enum gbe_arg_type type = gbe_kernel_get_arg_type(ker->opaque, arg);
    if (type != GBE_ARG_LOCAL_PTR)
      continue;
    uint32_t align = gbe_kernel_get_arg_align(ker->opaque, arg);
    assert(align != 0);
    slm_offset = ALIGN(slm_offset, align);
    offset = gbe_kernel_get_curbe_offset(ker->opaque, GBE_CURBE_KERNEL_ARGUMENT, arg);
    assert(offset >= 0);
    uint32_t *slmptr = (uint32_t *) (ker->curbe + offset);
    *slmptr = slm_offset;
    slm_offset += ker->args[arg].local_sz;
    ker->local_mem_sz += ker->args[arg].local_sz;
  }
  return slm_offset;
}

static void
cl_bind_stack(cl_gpgpu gpgpu, cl_kernel ker)
{
  cl_context ctx = ker->program->ctx;
  cl_device_id device = ctx->device;
  const int32_t per_lane_stack_sz = ker->stack_size;
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
  cl_gpgpu_set_stack(gpgpu, offset, stack_sz, cc_llc_l3);
}

LOCAL cl_int
cl_command_queue_ND_range_gen7(cl_command_queue queue,
                               cl_kernel ker,
                               const uint32_t work_dim,
                               const size_t *global_wk_off,
                               const size_t *global_wk_sz,
                               const size_t *local_wk_sz)
{
  GET_QUEUE_THREAD_GPGPU(queue);
  cl_context ctx = queue->ctx;
  char *final_curbe = NULL;  /* Includes them and one sub-buffer per group */
  cl_gpgpu_kernel kernel;
  const uint32_t simd_sz = cl_kernel_get_simd_width(ker);
  size_t i, batch_sz = 0u, local_sz = 0u;
  size_t cst_sz = ker->curbe_sz= gbe_kernel_get_curbe_size(ker->opaque);
  int32_t scratch_sz = gbe_kernel_get_scratch_size(ker->opaque);
  size_t thread_n = 0u;
  cl_int err = CL_SUCCESS;

  /* Setup kernel */
  kernel.name = "KERNEL";
  kernel.grf_blocks = 128;
  kernel.bo = ker->bo;
  kernel.barrierID = 0;
  kernel.slm_sz = 0;
  kernel.use_slm = gbe_kernel_use_slm(ker->opaque);

  /* Compute the number of HW threads we need */
  TRY (cl_kernel_work_group_sz, ker, local_wk_sz, 3, &local_sz);
  kernel.thread_n = thread_n = (local_sz + simd_sz - 1) / simd_sz;
  kernel.curbe_sz = cst_sz;

  if (scratch_sz > ker->program->ctx->device->scratch_mem_size) {
    fprintf(stderr, "Beignet: Out of scratch memory %d.\n", scratch_sz);
    return CL_OUT_OF_RESOURCES;
  }
  /* Curbe step 1: fill the constant urb buffer data shared by all threads */
  if (ker->curbe) {
    kernel.slm_sz = cl_curbe_fill(ker, work_dim, global_wk_off, global_wk_sz, local_wk_sz, thread_n);
    if (kernel.slm_sz > ker->program->ctx->device->local_mem_size) {
      fprintf(stderr, "Beignet: Out of shared local memory %d.\n", kernel.slm_sz);
      return CL_OUT_OF_RESOURCES;
    }
  }

  /* Setup the kernel */
  if (queue->props & CL_QUEUE_PROFILING_ENABLE)
    cl_gpgpu_state_init(gpgpu, ctx->device->max_compute_unit, cst_sz / 32, 1);
  else
    cl_gpgpu_state_init(gpgpu, ctx->device->max_compute_unit, cst_sz / 32, 0);

  /* Bind user buffers */
  cl_command_queue_bind_surface(queue, ker);
  /* Bind user images */
  cl_command_queue_bind_image(queue, ker);
  /* Bind all samplers */
  cl_gpgpu_bind_sampler(gpgpu, ker->samplers, ker->sampler_sz);

  cl_gpgpu_set_scratch(gpgpu, scratch_sz);

  /* Bind a stack if needed */
  cl_bind_stack(gpgpu, ker);

  cl_upload_constant_buffer(queue, ker);

  cl_gpgpu_states_setup(gpgpu, &kernel);

  /* Curbe step 2. Give the localID and upload it to video memory */
  if (ker->curbe) {
    assert(cst_sz > 0);
    TRY_ALLOC (final_curbe, (char*) alloca(thread_n * cst_sz));
    for (i = 0; i < thread_n; ++i) {
        memcpy(final_curbe + cst_sz * i, ker->curbe, cst_sz);
    }
    TRY (cl_set_varying_payload, ker, final_curbe, local_wk_sz, simd_sz, cst_sz, thread_n);
    cl_gpgpu_upload_curbes(gpgpu, final_curbe, thread_n*cst_sz);
  }

  /* Start a new batch buffer */
  batch_sz = cl_kernel_compute_batch_sz(ker);
  cl_gpgpu_batch_reset(gpgpu, batch_sz);
  cl_set_thread_batch_buf(queue, cl_gpgpu_ref_batch_buf(gpgpu));
  cl_gpgpu_batch_start(gpgpu);

  /* Issue the GPGPU_WALKER command */
  cl_gpgpu_walker(gpgpu, simd_sz, thread_n, global_wk_off, global_wk_sz, local_wk_sz);

  /* Close the batch buffer and submit it */
  cl_gpgpu_batch_end(gpgpu, 0);
error:
  return err;
}

