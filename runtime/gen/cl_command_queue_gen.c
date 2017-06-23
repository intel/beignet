/*
 * Copyright Â© 2012 Intel Corporation
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
 */

#include "cl_gen.h"
#include "gen_device_pci_id.h"

#include "intel_defines.h"
#include "intel_structs.h"
#include "intel_batchbuffer.h"

#include <i915_drm.h>
#include <drm.h>
#include <intel_bufmgr.h>
#include <assert.h>
#include <string.h>

/* We can bind only a limited number of buffers */
enum { max_buf_n = 128 };
enum { max_img_n = 128 };
enum { max_sampler_n = 16 };

// BTI magic number
#define BTI_CONSTANT 0
#define BTI_PRIVATE 1
#define BTI_RESERVED_NUM 2
#define BTI_MAX_READ_IMAGE_ARGS 128
#define BTI_MAX_WRITE_IMAGE_ARGS 8
#define BTI_WORKAROUND_IMAGE_OFFSET 128
#define BTI_MAX_ID 253
#define BTI_LOCAL 0xfe

typedef struct gen_gpgpu {
  drm_intel_bufmgr *bufmgr; // The drm buffer mgr
  cl_device_id device;      // The device of this gpu
  drm_intel_bo *kernel_bo;  // The buffer object holding kernel bitcode
  uint32_t simd_size;       // The simd size we are executing.
  uint32_t atomic_test_result;

  struct intel_batchbuffer *batch; // The batch buffer holding GPU command

  struct {
    drm_intel_bo *aux_bo; // Aux buffer needed by GPU command
    uint32_t surface_heap_offset;
    uint32_t curbe_offset;
    uint32_t idrt_offset;
    uint32_t sampler_state_offset;
    uint32_t sampler_border_color_state_offset;
  } aux; // All aux setting info

  struct {
    uint32_t local_mem_size; // The total local memory size

    uint32_t max_bti;                      /* Max bti number */
    uint32_t binded_n;                     /* Number of buffers binded */
    drm_intel_bo *binded_buf[max_buf_n];   /* All buffers binded for the kernel, e.g. kernel's arg */
    uint32_t binded_offset[max_buf_n];     /* The offset in the curbe buffer */
    uint32_t target_buf_offset[max_buf_n]; /* The offset within the buffers to be binded */

    uint32_t per_thread_scratch_size;
    uint32_t total_scratch_size;
    drm_intel_bo *scratch_bo; /* Scratch buffer */

    drm_intel_bo *const_bo; /* Constant buffer */
    drm_intel_bo *stack_bo; /* stack buffer */

    drm_intel_bo *time_stamp_bo; /* The buffer to record exec timestamps */
  } mem;

  struct {
    uint64_t sampler_bitmap; /* sampler usage bitmap. */
  } sampler;

  struct {
    uint32_t barrier_slm_used;   /* Use barrier or slm */
    uint32_t thread_num;         // Total thread number we need for this kernel
    uint32_t max_thread_num;     // Max thread number we can run at same time
    uint32_t per_thread_scratch; // Scratch buffer size for each thread
    uint32_t num_cs_entries;     /* Curbe entry number */
    uint32_t size_cs_entry;      /* size of one entry in 512bit elements */
    char *curbe;                 /* Curbe content */
    uint32_t curbe_size;         /* Curbe size */
  } thread;

} gen_gpgpu;

typedef struct gen_gpgpu_exec_ctx {
  void *device_enqueue_helper_ptr;
  drm_intel_bo *device_enqueue_helper_bo;
  size_t helper_bo_size;
  cl_int gpu_num;
  gen_gpgpu *all_gpu[8];
} gen_gpgpu_exec_ctx;

#define MAX_IF_DESC 32

typedef struct surface_heap {
  uint32_t binding_table[256];
  char surface[256 * sizeof(gen_surface_state_t)];
} surface_heap_t;

#include "gen_gpgpu_func.c"

static cl_int
check_work_group_capability(cl_command_queue queue, cl_kernel kernel,
                            const size_t *local_wk_sz, uint32_t wk_dim)
{
  size_t sz = 0;
  int i;

  sz = local_wk_sz[0];
  for (i = 1; i < wk_dim; ++i)
    sz *= local_wk_sz[i];

  if (sz > cl_kernel_get_max_workgroup_size_gen(kernel, queue->device))
    return CL_INVALID_WORK_ITEM_SIZE;

  return CL_SUCCESS;
}

static cl_int
gen_gpgpu_setup_curbe(cl_kernel kernel, cl_kernel_gen kernel_gen, gen_gpgpu *gpu,
                      const uint32_t work_dim, const size_t *global_wk_off,
                      const size_t *global_wk_sz, const size_t *local_wk_sz,
                      const size_t *enqueued_local_wk_sz, uint64_t device_enqueue_helper)
{
  int curbe_size = 0;
  char *curbe = NULL;
  int i;
  int sz = 0;
  uint32_t slm_offset;

  /* Calculate the total size needed */
  for (i = 0; i < kernel->arg_n; i++) {
    if (kernel->args[i].arg_size + kernel_gen->arg_extra_info[i].arg_offset > curbe_size)
      curbe_size = kernel->args[i].arg_size + kernel_gen->arg_extra_info[i].arg_offset;
  }
  for (i = 0; i < kernel_gen->virt_reg_phy_offset_num; i++) {
    sz = kernel_gen->virt_reg_phy_offset[i].phy_offset +
         kernel_gen->virt_reg_phy_offset[i].size;
    if (sz > curbe_size)
      curbe_size = sz;
  }
  for (i = 0; i < kernel_gen->image_info_num; i++) {
    if (kernel_gen->image_info[i].width > curbe_size)
      curbe_size = sz;
    if (kernel_gen->image_info[i].height > curbe_size)
      curbe_size = sz;
    if (kernel_gen->image_info[i].depth > curbe_size)
      curbe_size = sz;
    if (kernel_gen->image_info[i].data_type > curbe_size)
      curbe_size = sz;
    if (kernel_gen->image_info[i].channel_order > curbe_size)
      curbe_size = sz;
  }

  curbe_size = ALIGN(curbe_size, 32);

  gpu->thread.curbe_size = curbe_size;

  if (curbe_size == 0) {
    assert(kernel->arg_n == 0);
    return CL_SUCCESS;
  }

  curbe = CL_MALLOC(curbe_size);
  if (curbe == NULL) {
    return CL_OUT_OF_HOST_MEMORY;
  }
  gpu->thread.curbe = curbe;
  memset(curbe, 0, curbe_size);

  slm_offset = kernel_gen->local_mem_size;
  for (i = 0; i < kernel->arg_n; i++) {
    if (kernel_gen->arg_extra_info[i].arg_offset < 0) // no usage argument
      continue;

    if (kernel->args[i].arg_type == ArgTypePointer &&
        kernel->args[i].arg_addrspace == AddressSpaceLocal) { // SLM setting
      assert(kernel->args[i].val_size > 0);
      assert(kernel->args[i].arg_size == sizeof(uint32_t) || kernel->args[i].arg_size == sizeof(uint64_t));
      assert(kernel_gen->arg_extra_info[i].arg_align > 0);
      // Need to be aligned address
      slm_offset = ALIGN(slm_offset, kernel_gen->arg_extra_info[i].arg_align);
      if (kernel->args[i].arg_size == sizeof(uint32_t)) {
        *((uint32_t *)(curbe + kernel_gen->arg_extra_info[i].arg_offset)) = slm_offset;
      } else {
        *((uint64_t *)(curbe + kernel_gen->arg_extra_info[i].arg_offset)) = slm_offset;
      }
      slm_offset += kernel->args[i].val_size;
      continue;
    }

    if (kernel->args[i].arg_type == ArgTypePointer) {
      assert(kernel->args[i].arg_addrspace == AddressSpaceConstant ||
             kernel->args[i].arg_addrspace == AddressSpaceGlobal);
      /* For other buffer, we will set this value in surface binding */
      continue;
    }

    if (kernel->args[i].arg_type == ArgTypeSampler) {
      continue;
    }

    if (kernel->args[i].arg_type == ArgTypeImage) {
      continue;
    }

    /* Common value or struct data, just copy the content */
    assert(kernel->args[i].val_size == kernel->args[i].arg_size);
    if (kernel->args[i].arg_type == ArgTypeValue && kernel->args[i].arg_size <= sizeof(cl_double))
      memcpy(curbe + kernel_gen->arg_extra_info[i].arg_offset, &kernel->args[i].val, kernel->args[i].arg_size);
    else
      memcpy(curbe + kernel_gen->arg_extra_info[i].arg_offset, kernel->args[i].val.val_ptr, kernel->args[i].arg_size);
  }

#define UPLOAD(ENUM, VALUE, SIZE)                                               \
  if (kernel_gen->virt_reg_phy_offset[i].virt_reg == ENUM) {                    \
    assert(kernel_gen->virt_reg_phy_offset[i].size == sizeof(SIZE));            \
    *((SIZE *)(curbe + kernel_gen->virt_reg_phy_offset[i].phy_offset)) = VALUE; \
    continue;                                                                   \
  }

  for (i = 0; i < kernel_gen->virt_reg_phy_offset_num; i++) {
    UPLOAD(CL_GEN_VIRT_REG_ENQUEUE_BUF_POINTER, device_enqueue_helper, uint64_t);
    UPLOAD(CL_GEN_VIRT_REG_LOCAL_SIZE_X, local_wk_sz[0], uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_LOCAL_SIZE_Y, local_wk_sz[1], uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_LOCAL_SIZE_Z, local_wk_sz[2], uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_ENQUEUED_LOCAL_SIZE_X, enqueued_local_wk_sz[0], uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_ENQUEUED_LOCAL_SIZE_Y, enqueued_local_wk_sz[1], uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_ENQUEUED_LOCAL_SIZE_Z, enqueued_local_wk_sz[2], uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_GLOBAL_SIZE_X, global_wk_sz[0], uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_GLOBAL_SIZE_Y, global_wk_sz[1], uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_GLOBAL_SIZE_Z, global_wk_sz[2], uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_GLOBAL_OFFSET_X, global_wk_off[0], uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_GLOBAL_OFFSET_Y, global_wk_off[1], uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_GLOBAL_OFFSET_Z, global_wk_off[2], uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_GROUP_NUM_X,
           global_wk_sz[0] / enqueued_local_wk_sz[0] + (global_wk_sz[0] % enqueued_local_wk_sz[0] ? 1 : 0),
           uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_GROUP_NUM_Y,
           global_wk_sz[1] / enqueued_local_wk_sz[1] + (global_wk_sz[1] % enqueued_local_wk_sz[1] ? 1 : 0),
           uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_GROUP_NUM_Z,
           global_wk_sz[2] / enqueued_local_wk_sz[2] + (global_wk_sz[2] % enqueued_local_wk_sz[2] ? 1 : 0),
           uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_THREAD_NUM, gpu->thread.thread_num,
           uint32_t);
    UPLOAD(CL_GEN_VIRT_REG_WORK_DIM, work_dim, uint32_t);
  }
#undef UPLOAD

  return CL_SUCCESS;
}

static void
gen_gpgpu_bind_one_bo(gen_gpgpu *gpu, drm_intel_bo *buf, uint32_t offset,
                      uint32_t internal_offset, size_t size, uint8_t bti)
{
  if (buf == NULL)
    return;

  assert(gpu->mem.binded_n < max_buf_n);
  if (offset != -1) {
    gpu->mem.binded_buf[gpu->mem.binded_n] = buf;
    gpu->mem.target_buf_offset[gpu->mem.binded_n] = internal_offset;
    gpu->mem.binded_offset[gpu->mem.binded_n] = offset;
    gpu->mem.binded_n++;
  }
  gen_gpgpu_setup_bti(gpu, buf, internal_offset, size, bti, I965_SURFACEFORMAT_RAW);
}

static void
gen_gpgpu_setup_global_mem(cl_kernel kernel, cl_kernel_gen kernel_gen, gen_gpgpu *gpu)
{
  int i;
  int32_t offset = 0;
  cl_mem mem;
  uint32_t bti;
  cl_program_gen prog_gen;
  cl_mem_gen mem_gen;

  DEV_PRIVATE_DATA(kernel->program, gpu->device, prog_gen);

  for (i = 0; i < kernel->arg_n; i++) {
    if (kernel->args[i].arg_type != ArgTypePointer && kernel->args[i].arg_type != ArgTypePipe)
      continue;

    if (kernel->args[i].arg_addrspace != AddressSpaceGlobal &&
        kernel->args[i].arg_addrspace != AddressSpaceConstant)
      continue;

    if (prog_gen->cl_version < 200 && kernel->args[i].arg_addrspace == AddressSpaceConstant)
      continue;

    mem = NULL;
    mem_gen = NULL;
    offset = 0;
    bti = kernel_gen->arg_extra_info[i].arg_misc;

    if (kernel->args[i].use_svm) {
      assert(CL_OBJECT_IS_SVM(kernel->args[i].val.val_svm.svm));
      mem = kernel->args[i].val.val_svm.svm;
      DEV_PRIVATE_DATA(mem, gpu->device, mem_gen);
      assert(mem_gen->drm_bo);
      assert(mem_gen->mem_base.device == gpu->device);
      assert(mem->host_ptr);
      gen_gpgpu_bind_one_bo(gpu, mem_gen->drm_bo->bo, kernel_gen->arg_extra_info[i].arg_offset,
                            kernel->args[i].val.val_svm.ptr - mem->host_ptr,
                            mem_gen->drm_bo->gpu_size, bti);
    } else {
      if (kernel->args[i].val.val_mem != NULL) {
        mem = (cl_mem)kernel->args[i].val.val_mem;
        mem_gen = (cl_mem_gen)mem->each_device[0];
        assert(mem_gen);
        assert(mem_gen->drm_bo);
        assert(mem_gen->mem_base.device == gpu->device);
        offset = mem_gen->drm_bo->in_page_offset;
      }

      if (CL_OBJECT_IS_BUFFER(mem) && cl_mem_to_buffer(mem)->svm_buf) {
        offset += cl_mem_to_buffer(mem)->svm_offset;
      } else if (CL_OBJECT_IS_SUB_BUFFER(mem)) {
        offset += cl_mem_to_buffer(mem)->sub_offset;
      }

      gen_gpgpu_bind_one_bo(gpu, mem ? mem_gen->drm_bo->bo : NULL,
                            kernel_gen->arg_extra_info[i].arg_offset, offset,
                            mem ? mem_gen->drm_bo->gpu_size : 0, bti);
    }

    if (gpu->mem.max_bti < bti)
      gpu->mem.max_bti = bti;
  }
}

static cl_int
gen_gpgpu_setup_kernel_exec_svm_mem(cl_kernel kernel, cl_kernel_gen kernel_gen, gen_gpgpu *gpu)
{
  int i;
  int32_t offset = 0;
  cl_mem mem;
  uint32_t bti;
  cl_mem_gen mem_gen;

  if (kernel->exec_info == NULL)
    return CL_SUCCESS;

  assert(kernel->exec_info_n > 0);
  for (i = 0; i < kernel->exec_info_n; i++) {
    offset = kernel->exec_info[i].offset;
    mem = kernel->exec_info[i].svm;
    DEV_PRIVATE_DATA(mem, gpu->device, mem_gen);

    if (gpu->mem.max_bti == BTI_MAX_ID)
      return CL_OUT_OF_RESOURCES;

    bti = gpu->mem.max_bti;
    gpu->mem.max_bti++;

    /* No need to setup the offset in curbe, just setup bti */
    gen_gpgpu_setup_bti(gpu, mem_gen->drm_bo->bo, offset, mem->size, bti, I965_SURFACEFORMAT_RAW);
  }
  return CL_SUCCESS;
}

static cl_int
gen_gpgpu_setup_image(cl_kernel kernel, cl_kernel_gen kernel_gen, gen_gpgpu *gpu)
{
  int i;
  cl_mem mem;
  cl_mem_image image;
  cl_mem_gen image_gen;
  cl_gen_image_info_offset info;

  for (i = 0; i < kernel->arg_n; i++) {
    if (kernel->args[i].arg_type != ArgTypeImage)
      continue;

    assert(kernel->args[i].val.val_ptr != NULL);
    mem = (cl_mem)kernel->args[i].val.val_ptr;
    image = cl_mem_to_image(mem);
    image_gen = (cl_mem_gen)mem->each_device[0];
    assert(image_gen);
    assert(image_gen->drm_bo);

    info = &kernel_gen->image_info[kernel_gen->arg_extra_info[i].arg_misc];

    /* Set the image info to the curbe */
    if (info->width >= 0)
      *(uint32_t *)(gpu->thread.curbe + info->width) = image->w;
    if (info->height >= 0)
      *(uint32_t *)(gpu->thread.curbe + info->height) = image->h;
    if (info->depth >= 0)
      *(uint32_t *)(gpu->thread.curbe + info->depth) = image->depth;
    if (info->channel_order >= 0)
      *(uint32_t *)(gpu->thread.curbe + info->channel_order) =
        image->fmt.image_channel_order;
    if (info->data_type >= 0)
      *(uint32_t *)(gpu->thread.curbe + info->data_type) =
        image->fmt.image_channel_data_type;

    if (gpu->mem.max_bti < info->bti)
      gpu->mem.max_bti = info->bti;

    gen_gpgpu_bind_image(gpu, info->bti, image_gen->drm_bo->bo,
                         image_gen->image.sub_offset + image_gen->drm_bo->in_page_offset,
                         image_gen->image.intel_fmt, image->image_type, image->bpp, image->w,
                         image->h, image->depth, image_gen->image.gpu_row_pitch,
                         image_gen->image.gpu_slice_pitch, image_gen->drm_bo->tiling);

    // TODO, this workaround is for GEN7/GEN75 only, we may need to do it in the driver layer
    // on demand.
    if (image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
      gen_gpgpu_bind_image(gpu, info->bti + BTI_WORKAROUND_IMAGE_OFFSET,
                           image_gen->drm_bo->bo,
                           image_gen->image.sub_offset + image_gen->drm_bo->in_page_offset,
                           image_gen->image.intel_fmt, image->image_type, image->bpp, image->w,
                           image->h, image->depth, image_gen->image.gpu_row_pitch,
                           image_gen->image.gpu_slice_pitch, image_gen->drm_bo->tiling);
  }

  return CL_SUCCESS;
}

static cl_int
gen_gpgpu_setup_scratch(gen_gpgpu *gpu)
{
  drm_intel_bufmgr *bufmgr = gpu->bufmgr;
  cl_uint device_id = gpu->device->device_id;

  gpu->mem.total_scratch_size = gpu->mem.per_thread_scratch_size * gpu->thread.max_thread_num;
  /* Per Bspec, scratch should 2X the desired size when EU index is not continuous */
  if (IS_HASWELL(device_id) || IS_CHERRYVIEW(device_id) ||
      device_id == PCI_CHIP_BROXTON_1 || device_id == PCI_CHIP_BROXTON_3) {
    gpu->mem.total_scratch_size = gpu->mem.total_scratch_size * 2;
  }

  if (gpu->mem.total_scratch_size) {
    gpu->mem.scratch_bo = drm_intel_bo_alloc(bufmgr, "SCRATCH_BO",
                                             gpu->mem.total_scratch_size, 4096);
    if (gpu->mem.scratch_bo == NULL)
      return CL_OUT_OF_RESOURCES;
  }
  return CL_SUCCESS;
}

static cl_int
gen_setup_constant_buffer_for_20(cl_kernel kernel, cl_kernel_gen kernel_gen,
                                 cl_program_gen prog_gen, gen_gpgpu *gpu)
{
#ifndef HAS_BO_SET_SOFTPIN
  return CL_OUT_OF_RESOURCES;
#else
  int i;
  cl_bool need_const_buf = CL_FALSE;
  cl_int const_addr_curbe_offset = -1;
  cl_gen_virt_phy_offset map = kernel_gen->virt_reg_phy_offset;

  for (i = 0; i < kernel_gen->virt_reg_phy_offset_num; i++) {
    if (map[i].virt_reg == CL_GEN_VIRT_REG_CONSTANT_ADDRSPACE) {
      need_const_buf = CL_TRUE;
      const_addr_curbe_offset = map[i].phy_offset;
      assert(map[i].size == 8);
      break;
    }
  }

  if (need_const_buf == CL_FALSE)
    return CL_SUCCESS;

  assert(prog_gen->global_mem_data); // Should always have something
  assert(const_addr_curbe_offset >= 0);

  gpu->mem.const_bo = intel_buffer_alloc_userptr(gpu->bufmgr, "program global data",
                                                 prog_gen->global_mem_data, prog_gen->global_mem_data_size, 0);
  drm_intel_bo_set_softpin_offset(gpu->mem.const_bo, (size_t)prog_gen->global_mem_data);
  drm_intel_bo_use_48b_address_range(gpu->mem.const_bo, 1);
  *(char **)(gpu->thread.curbe + const_addr_curbe_offset) = prog_gen->global_mem_data;
  gen_gpgpu_bind_one_bo(gpu, gpu->mem.const_bo, const_addr_curbe_offset, 0,
                        prog_gen->global_mem_data_size, BTI_CONSTANT);
  return CL_SUCCESS;
#endif
}

static cl_int
gen_setup_constant_buffer(cl_kernel kernel, cl_kernel_gen kernel_gen, gen_gpgpu *gpu)
{
  cl_program_gen prog_gen;
  cl_uint const_buf_size = 0;
  cl_uint aligned_const_buf_size = 0;
  cl_mem mem;
  cl_uint addr_offset;
  char *const_buf_addr = NULL;
  int i;
  DEV_PRIVATE_DATA(kernel->program, gpu->device, prog_gen);

  /* 2.0 is different from before */
  if (prog_gen->cl_version >= 200) {
    return gen_setup_constant_buffer_for_20(kernel, kernel_gen, prog_gen, gpu);
  }

  if (prog_gen->rodata) {
    const_buf_size = prog_gen->rodata_data->d_size;
    aligned_const_buf_size = ALIGN(const_buf_size, 8);
  } else {
    /* Reserve 8 bytes to get rid of 0 address */
    aligned_const_buf_size = 8;
  }

  /* Calculate all the constant mem size */
  for (i = 0; i < kernel->arg_n; i++) {
    if (kernel->args[i].arg_type != ArgTypePointer)
      continue;
    if (kernel->args[i].arg_addrspace != AddressSpaceConstant)
      continue;

    if (kernel->args[i].val.val_ptr == NULL)
      continue;

    assert(kernel_gen->arg_extra_info[i].arg_align != 0);
    mem = (cl_mem)kernel->args[i].val.val_ptr;
    const_buf_size += mem->size;
    aligned_const_buf_size = ALIGN(aligned_const_buf_size, kernel_gen->arg_extra_info[i].arg_align);
    aligned_const_buf_size += mem->size;
  }

  if (const_buf_size == 0) // No need for constant buffer.
    return CL_SUCCESS;

  gen_gpgpu_alloc_constant_buffer(gpu, aligned_const_buf_size, BTI_CONSTANT);
  if (gpu->mem.const_bo == NULL)
    return CL_OUT_OF_RESOURCES;

  drm_intel_bo_map(gpu->mem.const_bo, 1);

  const_buf_addr = gpu->mem.const_bo->virtual;
  if (const_buf_addr == NULL)
    return CL_OUT_OF_RESOURCES;

  addr_offset = 0;
  /* upload the global constant data, in rodata */
  if (prog_gen->rodata && prog_gen->rodata_data->d_size > 0) {
    memcpy(const_buf_addr, prog_gen->rodata_data->d_buf, prog_gen->rodata_data->d_size);
    addr_offset = prog_gen->rodata_data->d_size;
    addr_offset = ALIGN(addr_offset, 8);
  } else {
    addr_offset = 8;
  }

  /* Upload constant ptr content */
  for (i = 0; i < kernel->arg_n; i++) {
    cl_uint ptr_val = 0;

    if (kernel->args[i].arg_type != ArgTypePointer)
      continue;
    if (kernel->args[i].arg_addrspace != AddressSpaceConstant)
      continue;

    assert(kernel_gen->arg_extra_info[i].arg_align > 0);
    addr_offset = ALIGN(addr_offset, kernel_gen->arg_extra_info[i].arg_align);
    assert(kernel->args[i].arg_size == sizeof(uint32_t) || kernel->args[i].arg_size == sizeof(uint64_t));

    mem = (cl_mem)kernel->args[i].val.val_ptr;
    if (mem) {
      cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];
      void *cst_ptr = NULL;
      assert(mem_gen);
      assert(mem_gen->drm_bo);
      cst_ptr = cl_mem_gen_drm_bo_map(mem_gen->drm_bo, CL_FALSE);
      memcpy(const_buf_addr + addr_offset, cst_ptr, mem->size);
      cl_mem_gen_drm_bo_unmap(mem_gen->drm_bo);
      ptr_val = addr_offset;
      addr_offset += mem->size;
      addr_offset = ALIGN(addr_offset, kernel_gen->arg_extra_info[i].arg_align);
    }

    /* Set curbe */
    if (kernel_gen->arg_extra_info[i].arg_offset >= 0) {
      if (kernel->args[i].arg_size == sizeof(uint32_t)) {
        *(uint32_t *)(gpu->thread.curbe + kernel_gen->arg_extra_info[i].arg_offset) = ptr_val;
      } else {
        *(uint64_t *)(gpu->thread.curbe + kernel_gen->arg_extra_info[i].arg_offset) = ptr_val;
      }
    }
  }

  drm_intel_bo_unmap(gpu->mem.const_bo);
  return CL_SUCCESS;
}

static cl_int
gen_gpgpu_upload_final_curbe(cl_kernel kernel, cl_kernel_gen kernel_gen,
                             gen_gpgpu *gpu, const size_t *local_wk_sz)
{
  char *final_curbe = NULL;
  char *final_curbe_ptr = NULL;
  cl_gen_virt_phy_offset map = kernel_gen->virt_reg_phy_offset;
  int i, j, k, curr = 0;
  uint32_t *ids[3] = {NULL, NULL, NULL};
  int32_t id_offset[3], ip_offset, tid_offset;
  uint16_t *block_ips = NULL;
  uint32_t *thread_ids = NULL;
  int32_t dw_ip_offset = -1;

  if (gpu->thread.curbe_size == 0) {
    assert(gpu->thread.curbe == NULL);
    return CL_SUCCESS;
  }

  assert(gpu->thread.thread_num > 0);
  final_curbe = CL_MALLOC(gpu->thread.thread_num * gpu->thread.curbe_size);
  if (final_curbe == NULL)
    return CL_OUT_OF_HOST_MEMORY;

  for (i = 0; i < gpu->thread.thread_num; ++i) {
    memcpy(final_curbe + gpu->thread.curbe_size * i,
           gpu->thread.curbe, gpu->thread.curbe_size);
  }

  id_offset[0] = id_offset[1] = id_offset[2] = -1;
  ip_offset = -1;
  tid_offset = -1;
  if (map) {
    for (i = 0; i < kernel_gen->virt_reg_phy_offset_num; i++) {
      if (map[i].virt_reg == CL_GEN_VIRT_REG_LOCAL_ID_X ||
          map[i].virt_reg == CL_GEN_VIRT_REG_LOCAL_ID_Y ||
          map[i].virt_reg == CL_GEN_VIRT_REG_LOCAL_ID_Z) {
        id_offset[map[i].virt_reg - CL_GEN_VIRT_REG_LOCAL_ID_X] = map[i].phy_offset;
        assert(map[i].phy_offset >= 0);
        assert(map[i].size / gpu->simd_size == sizeof(uint32_t));
        continue;
      }
      if (map[i].virt_reg == CL_GEN_VIRT_REG_BLOCK_IP) {
        ip_offset = map[i].phy_offset;
        assert(map[i].phy_offset >= 0);
        assert(map[i].size / gpu->simd_size == sizeof(uint16_t));
        continue;
      }
      if (map[i].virt_reg == CL_GEN_VIRT_REG_THREAD_ID) {
        tid_offset = map[i].phy_offset;
        assert(map[i].phy_offset >= 0);
        assert(map[i].size == sizeof(uint32_t));
        continue;
      }
      if (map[i].virt_reg == CL_GEN_VIRT_REG_DW_BLOCK_IP) {
        dw_ip_offset = map[i].phy_offset;
        assert(map[i].phy_offset >= 0);
        assert(map[i].size / gpu->simd_size == sizeof(uint32_t));
        continue;
      }
    }

    assert(ip_offset < 0 || dw_ip_offset < 0);

    if (id_offset[0] >= 0) {
      ids[0] = (uint32_t *)alloca(sizeof(uint32_t) * gpu->thread.thread_num * gpu->simd_size);
      assert(id_offset[0] >= 0);
    }
    if (id_offset[1] >= 0) {
      ids[1] = (uint32_t *)alloca(sizeof(uint32_t) * gpu->thread.thread_num * gpu->simd_size);
      assert(id_offset[1] >= 0);
    }
    if (id_offset[2] >= 0) {
      ids[2] = (uint32_t *)alloca(sizeof(uint32_t) * gpu->thread.thread_num * gpu->simd_size);
      assert(id_offset[2] >= 0);
    }

    block_ips = (uint16_t *)alloca(sizeof(uint16_t) * gpu->thread.thread_num * gpu->simd_size);
    assert(block_ips >= 0);
    memset(block_ips, 0xff, sizeof(int16_t) * gpu->thread.thread_num * gpu->simd_size);

    if (tid_offset >= 0) {
      thread_ids = (uint32_t *)alloca(sizeof(uint32_t) * gpu->thread.thread_num);
      assert(thread_ids >= 0);
      memset(thread_ids, 0, sizeof(uint32_t) * gpu->thread.thread_num);
    }
    /* Compute the IDs and the block IPs */
    for (k = 0; k < local_wk_sz[2]; ++k) {
      for (j = 0; j < local_wk_sz[1]; ++j) {
        for (i = 0; i < local_wk_sz[0]; ++i, ++curr) {
          if (id_offset[0] >= 0)
            ids[0][curr] = i;
          if (id_offset[1] >= 0)
            ids[1][curr] = j;
          if (id_offset[2] >= 0)
            ids[2][curr] = k;
          block_ips[curr] = 0;
          if (thread_ids)
            thread_ids[curr / gpu->simd_size] = curr / gpu->simd_size;
        }
      }
    }

    /* Set the vary part of curbe */
    curr = 0;
    final_curbe_ptr = final_curbe;
    for (i = 0; i < gpu->thread.thread_num; ++i, final_curbe_ptr += gpu->thread.curbe_size) {
      uint32_t *ids0 = (uint32_t *)(final_curbe_ptr + id_offset[0]);
      uint32_t *ids1 = (uint32_t *)(final_curbe_ptr + id_offset[1]);
      uint32_t *ids2 = (uint32_t *)(final_curbe_ptr + id_offset[2]);
      uint16_t *ips = (uint16_t *)(final_curbe_ptr + ip_offset);
      uint32_t *dw_ips = (uint32_t *)(final_curbe_ptr + dw_ip_offset);

      if (thread_ids)
        *(uint32_t *)(final_curbe_ptr + tid_offset) = thread_ids[i];

      for (j = 0; j < gpu->simd_size; ++j, ++curr) {
        if (id_offset[0] >= 0)
          ids0[j] = ids[0][curr];
        if (id_offset[1] >= 0)
          ids1[j] = ids[1][curr];
        if (id_offset[2] >= 0)
          ids2[j] = ids[2][curr];
        if (ip_offset >= 0)
          ips[j] = block_ips[curr];
        if (dw_ip_offset >= 0)
          dw_ips[j] = block_ips[curr];
      }
    }
  }

  /* All settings are OK, upload it to GPU */
  gen_gpgpu_upload_curbes(gpu, final_curbe, gpu->thread.thread_num * gpu->thread.curbe_size);
  CL_FREE(final_curbe);
  return CL_SUCCESS;
}

static cl_int
gen_gpgu_bind_stack(gen_gpgpu *gpu, cl_kernel kernel, cl_kernel_gen kernel_gen)
{
  int32_t stack_sz = kernel_gen->stack_size;
  int32_t stack_offset = -1;
  int32_t stack_size_offset = -1;
  int i;

  if (stack_sz == 0)
    return CL_SUCCESS;

  stack_sz *= kernel_gen->simd_width;
  stack_sz *= gpu->thread.max_thread_num;

  if (IS_GEN75(gpu->device->device_id))
    stack_sz = stack_sz * 4;
  else if (gpu->device->device_id == PCI_CHIP_BROXTON_1 || gpu->device->device_id == PCI_CHIP_BROXTON_3 ||
           IS_CHERRYVIEW(gpu->device->device_id))
    stack_sz = stack_sz * 2;

  for (i = 0; i < kernel_gen->virt_reg_phy_offset_num; i++) {
    if (kernel_gen->virt_reg_phy_offset[i].virt_reg == CL_GEN_VIRT_REG_STACK_SIZE) {
      assert(kernel_gen->virt_reg_phy_offset[i].size == sizeof(uint32_t));
      stack_size_offset = kernel_gen->virt_reg_phy_offset[i].phy_offset;
      continue;
    }
    if (kernel_gen->virt_reg_phy_offset[i].virt_reg == CL_GEN_VIRT_REG_EXTRA_ARGUMENT) {
      assert(kernel_gen->virt_reg_phy_offset[i].size == sizeof(uint64_t));
      stack_offset = kernel_gen->virt_reg_phy_offset[i].phy_offset;
      continue;
    }
  }
  assert(stack_offset >= 0);

  if (stack_size_offset >= 0)
    *((uint32_t *)(gpu->thread.curbe + stack_offset)) = stack_sz;

  gpu->mem.stack_bo = drm_intel_bo_alloc(gpu->bufmgr, "STACK", stack_sz, 64);
  if (gpu->mem.stack_bo == NULL)
    return CL_OUT_OF_RESOURCES;

  gen_gpgpu_bind_one_bo(gpu, gpu->mem.stack_bo, stack_offset, 0, stack_sz, BTI_PRIVATE);
  return CL_SUCCESS;
}

#define MAX_GROUP_SIZE_IN_HALFSLICE 512
static size_t
gen_gpu_compute_batch_sz(cl_kernel k)
{
  return 256 + 256;
}

static void
cl_command_queue_delete_gpgpu(void *gpgpu)
{
  gen_gpgpu *gpu = gpgpu;

  if (gpgpu == NULL)
    return;

  if (gpu->kernel_bo) {
    drm_intel_bo_unreference(gpu->kernel_bo);
    gpu->kernel_bo = NULL;
  }

  if (gpu->thread.curbe) {
    CL_FREE(gpu->thread.curbe);
    gpu->thread.curbe = NULL;
  }

  if (gpu->aux.aux_bo != NULL) {
    assert(gpu->aux.aux_bo->virtual == NULL);
    drm_intel_bo_unreference(gpu->aux.aux_bo);
    gpu->aux.aux_bo = NULL;
  }

  if (gpu->mem.scratch_bo) {
    drm_intel_bo_unreference(gpu->mem.scratch_bo);
    gpu->mem.scratch_bo = NULL;
  }

  if (gpu->mem.stack_bo) {
    drm_intel_bo_unreference(gpu->mem.stack_bo);
    gpu->mem.stack_bo = NULL;
  }

  if (gpu->mem.const_bo) {
    drm_intel_bo_unreference(gpu->mem.const_bo);
    gpu->mem.const_bo = NULL;
  }

  if (gpu->mem.time_stamp_bo) {
    drm_intel_bo_unreference(gpu->mem.time_stamp_bo);
    gpu->mem.time_stamp_bo = NULL;
  }

  if (gpu->batch) {
    intel_batchbuffer_delete(gpu->batch);
    gpu->batch = NULL;
  }

  CL_FREE(gpu);
  return;
}

static void
gen_gpgpu_setup_sampler(cl_kernel kernel, cl_kernel_gen kernel_gen, gen_gpgpu *gpu)
{
  cl_uint i;
  clk_sampler_type *spt;
  cl_uint *samper_info = NULL;

  if (kernel_gen->samper_info == NULL) {
    assert(kernel_gen->samper_info_num == 0);
    return;
  }

  samper_info = CL_MALLOC(sizeof(cl_uint) * kernel_gen->samper_info_num);
  assert(samper_info);
  memcpy(samper_info, kernel_gen->samper_info, sizeof(cl_uint) * kernel_gen->samper_info_num);

  for (i = 0; i < kernel->arg_n; i++) {
    if (kernel->args[i].arg_type != ArgTypeSampler)
      continue;

    assert(kernel_gen->arg_extra_info != NULL);
    assert(kernel_gen->samper_info_num > kernel_gen->arg_extra_info[i].arg_misc);
    spt = &(samper_info[kernel_gen->arg_extra_info[i].arg_misc]);
    assert(GEN_IS_SAMPLER_ARG(*spt));
    assert(GEN_SAMPLER_ARG_ID(*spt) == i);
    *spt = kernel->args[i].val.val_sampler->clkSamplerValue;

    /* Set its value in curbe */
    if (kernel_gen->arg_extra_info[i].arg_offset >= 0)
      *(uint32_t *)(gpu->thread.curbe + kernel_gen->arg_extra_info[i].arg_offset) = *spt;
  }

  gen_gpgpu_bind_sampler(gpu, samper_info, kernel_gen->samper_info_num);
  CL_FREE(samper_info);
}

/* This is a very important function. It is responsible for loading and setting GPU
   execution context based on the cl_kernel and kernel's arguments. */
static gen_gpgpu *
cl_command_queue_ND_range_gen_once(cl_command_queue queue, cl_kernel kernel, cl_int *err,
                                   const uint32_t work_dim, const size_t *global_wk_off,
                                   const size_t *global_dim_off, const size_t *global_wk_sz,
                                   const size_t *global_wk_sz_use, const size_t *local_wk_sz,
                                   const size_t *local_wk_sz_use, gen_gpgpu_exec_ctx *gpu_exec_ctx)
{
  cl_int ret = CL_SUCCESS;
  gen_gpgpu *gpu = NULL;
  size_t local_size = local_wk_sz_use[0] * local_wk_sz_use[1] * local_wk_sz_use[2];
  cl_kernel_gen kernel_gen;
  cl_context_gen ctx_gen;
  int i;
  drm_intel_bufmgr *bufmgr = NULL;

  DEV_PRIVATE_DATA(kernel, queue->device, kernel_gen);
  DEV_PRIVATE_DATA(queue->ctx, queue->device, ctx_gen);
  bufmgr = ctx_gen->drv->bufmgr;
  assert(bufmgr);

  ret = check_work_group_capability(queue, kernel, local_wk_sz_use, 3);
  if (ret != CL_SUCCESS) {
    *err = ret;
    return NULL;
  }

  if (kernel_gen->scratch_size > queue->device->scratch_mem_size) {
    *err = CL_OUT_OF_RESOURCES;
    return NULL;
  }

  gpu = CL_CALLOC(1, sizeof(gen_gpgpu));
  if (gpu == NULL) {
    *err = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }

  do {
    /* Init the gpu parameters */
    gpu->bufmgr = bufmgr;
    gpu->simd_size = kernel_gen->simd_width;
    gpu->device = queue->device;
    gpu->thread.max_thread_num = queue->device->max_compute_unit * queue->device->max_thread_per_unit;
    gpu->thread.thread_num = (local_size + gpu->simd_size - 1) / gpu->simd_size;
    gpu->sampler.sampler_bitmap = ~((1 << max_sampler_n) - 1);
    gpu->mem.max_bti = 0;
    gpu->mem.per_thread_scratch_size = kernel_gen->scratch_size;
    gpu->mem.total_scratch_size = 0;
    gpu->atomic_test_result = gpu->device->atomic_test_result;
    gpu->thread.barrier_slm_used = kernel_gen->barrier_slm_used;

    gpu->mem.local_mem_size = kernel_gen->local_mem_size;
    for (i = 0; i < kernel->arg_n; i++) {
      if (kernel->args[i].arg_type == ArgTypePointer &&
          kernel->args[i].arg_addrspace == AddressSpaceLocal) {
        assert(kernel->args[i].is_set);
        assert(kernel_gen->arg_extra_info[i].arg_align > 0);
        gpu->mem.local_mem_size = ALIGN(gpu->mem.local_mem_size, kernel_gen->arg_extra_info[i].arg_align);
        gpu->mem.local_mem_size += kernel->args[i].val_size;
      }
    }

    if (gpu->mem.local_mem_size > queue->device->local_mem_size) {
      ret = CL_OUT_OF_HOST_MEMORY;
      break;
    }

    // Setup the kernel bitcode and upload it to GPU side
    gpu->kernel_bo = drm_intel_bo_alloc(bufmgr, "CL kernel", kernel_gen->kern_base.exec_code_sz, 64u);
    if (gpu->kernel_bo == NULL) {
      ret = CL_OUT_OF_RESOURCES;
      break;
    }
    /* Upload the bitcode */
    drm_intel_bo_subdata(gpu->kernel_bo, 0, kernel_gen->kern_base.exec_code_sz,
                         kernel_gen->kern_base.exec_code);

    ret = gen_gpgpu_setup_curbe(kernel, kernel_gen, gpu, work_dim, global_wk_off, global_wk_sz,
                                local_wk_sz_use, local_wk_sz, (uint64_t)gpu_exec_ctx->device_enqueue_helper_ptr);
    if (ret != CL_SUCCESS)
      break;

    gpu->thread.num_cs_entries = 64;
    gpu->thread.size_cs_entry = gpu->thread.curbe_size / 32;

    ret = gen_gpgpu_setup_aux(gpu);
    if (ret != CL_SUCCESS)
      break;

    if (queue->props & CL_QUEUE_PROFILING_ENABLE) { // Need to alloc profiling buffer
      gpu->mem.time_stamp_bo = dri_bo_alloc(bufmgr, "timestamp query", 4096, 4096);

      if (gpu->mem.time_stamp_bo == NULL) {
        ret = CL_OUT_OF_RESOURCES;
        break;
      }
    }

    /* Bind user buffers */
    gen_gpgpu_setup_global_mem(kernel, kernel_gen, gpu);

    ret = gen_gpgpu_setup_image(kernel, kernel_gen, gpu);
    if (ret != CL_SUCCESS)
      break;

    gen_gpgpu_setup_kernel_exec_svm_mem(kernel, kernel_gen, gpu);

    /* also setup the device enqueue helper bo if exist */
    if (gpu_exec_ctx->device_enqueue_helper_bo) {
      gen_gpgpu_setup_bti(gpu, gpu_exec_ctx->device_enqueue_helper_bo, 0,
                          gpu_exec_ctx->helper_bo_size, gpu->mem.max_bti, I965_SURFACEFORMAT_RAW);
      gpu->mem.max_bti++;
    }

    gen_gpgpu_setup_sampler(kernel, kernel_gen, gpu);

    ret = gen_gpgpu_setup_scratch(gpu);
    if (ret != CL_SUCCESS)
      break;

    /* Bind a stack if needed */
    ret = gen_gpgu_bind_stack(gpu, kernel, kernel_gen);
    if (ret != CL_SUCCESS)
      break;

    ret = gen_setup_constant_buffer(kernel, kernel_gen, gpu);
    if (ret != CL_SUCCESS)
      break;

    gen_gpgpu_build_idrt(gpu);
    gen_gpgpu_upload_final_curbe(kernel, kernel_gen, gpu, local_wk_sz_use);
    gen_gpgpu_finish_aux(gpu);

    /* Start a new batch buffer */
    gpu->batch = intel_batchbuffer_create(ctx_gen->drv, gen_gpu_compute_batch_sz(kernel));
    if (gpu->batch == NULL) {
      ret = CL_OUT_OF_RESOURCES;
      break;
    }

    gen_gpgpu_batch_start(gpu);
    gen_gpgpu_walker(gpu, gpu->simd_size, gpu->thread.thread_num,
                     global_wk_off, global_dim_off, global_wk_sz_use, local_wk_sz_use);
    gen_gpgpu_batch_end(gpu, 0);
  } while (0);

  if (ret != CL_SUCCESS) {
    gen_gpgpu_finish_aux(gpu);
    cl_command_queue_delete_gpgpu(gpu);
    *err = ret;
    return NULL;
  }

  *err = CL_SUCCESS;
  return gpu;
}

LOCAL cl_int
cl_command_queue_ND_range(cl_command_queue queue, cl_kernel ker, void *exec_ctx, cl_uint work_dim,
                          size_t *global_wk_off, size_t *global_wk_sz, size_t *local_wk_sz)
{
  /* Used for non uniform work group size */
  cl_int err = CL_SUCCESS;
  gen_gpgpu *gpu = NULL;
  gen_gpgpu_exec_ctx *gpu_exec_ctx = exec_ctx;
  cl_int n;
  int i, j, k;
  const size_t global_wk_sz_div[3] = {
    global_wk_sz[0] / local_wk_sz[0] * local_wk_sz[0],
    global_wk_sz[1] / local_wk_sz[1] * local_wk_sz[1],
    global_wk_sz[2] / local_wk_sz[2] * local_wk_sz[2]};

  const size_t global_wk_sz_rem[3] = {
    global_wk_sz[0] % local_wk_sz[0],
    global_wk_sz[1] % local_wk_sz[1],
    global_wk_sz[2] % local_wk_sz[2]};

  const size_t *global_wk_all[2] = {global_wk_sz_div, global_wk_sz_rem};

  /* Go through the at most 8 cases and euque if there is work items left */
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 2; j++) {
      for (k = 0; k < 2; k++) {
        size_t global_wk_sz_use[3] = {global_wk_all[k][0], global_wk_all[j][1], global_wk_all[i][2]};
        size_t global_dim_off[3] = {
          k * global_wk_sz_div[0] / local_wk_sz[0],
          j * global_wk_sz_div[1] / local_wk_sz[1],
          i * global_wk_sz_div[2] / local_wk_sz[2]};
        size_t local_wk_sz_use[3] = {
          k ? global_wk_sz_rem[0] : local_wk_sz[0],
          j ? global_wk_sz_rem[1] : local_wk_sz[1],
          i ? global_wk_sz_rem[2] : local_wk_sz[2]};
        if (local_wk_sz_use[0] == 0 || local_wk_sz_use[1] == 0 || local_wk_sz_use[2] == 0)
          continue;

        gpu = cl_command_queue_ND_range_gen_once(queue, ker, &err, work_dim, global_wk_off, global_dim_off,
                                                 global_wk_sz, global_wk_sz_use, local_wk_sz, local_wk_sz_use,
                                                 exec_ctx);
        if (err != CL_SUCCESS) {
          assert(gpu == NULL);
          for (n = 0; n < gpu_exec_ctx->gpu_num; n++) {
            assert(gpu_exec_ctx->all_gpu[n]);
            cl_command_queue_delete_gpgpu(gpu_exec_ctx->all_gpu[n]);
          }

          return err;
        }

        gpu_exec_ctx->all_gpu[gpu_exec_ctx->gpu_num] = gpu;
        gpu_exec_ctx->gpu_num++;
      }
      if (work_dim < 2)
        break;
    }
    if (work_dim < 3)
      break;
  }

  assert(err == CL_SUCCESS);
  return err;
}

LOCAL cl_int
cl_command_queue_ND_range_wrap(cl_command_queue queue, cl_kernel ker, cl_event e, cl_uint work_dim,
                               size_t *global_wk_off, size_t *global_wk_sz, size_t *local_wk_sz)
{
  cl_int err = CL_SUCCESS;
  cl_kernel_gen kernel_gen;
  cl_uint i;
  cl_bool use_device_enqueue = CL_FALSE;

  gen_gpgpu_exec_ctx *exec_ctx = CL_CALLOC(1, sizeof(gen_gpgpu_exec_ctx));
  if (exec_ctx == NULL)
    return CL_OUT_OF_HOST_MEMORY;

  DEV_PRIVATE_DATA(ker, queue->device, kernel_gen);

  for (i = 0; i < kernel_gen->virt_reg_phy_offset_num; i++) {
    if (kernel_gen->virt_reg_phy_offset[i].virt_reg == CL_GEN_VIRT_REG_ENQUEUE_BUF_POINTER) {
      use_device_enqueue = CL_TRUE;
      break;
    }
  }
  /* We need to make all kernel entry mem uniform address, later device enqueue can use them */
  if (use_device_enqueue) {
    cl_mem mem;
    cl_mem_gen mem_gen;
    size_t buf_size = 32 * 1024 * 1024; //fix 32M
    cl_context_gen ctx_gen;
    DEV_PRIVATE_DATA(queue->ctx, queue->device, ctx_gen);
    void *tmp_ptr;

    exec_ctx->device_enqueue_helper_ptr = CL_MEMALIGN(4096, buf_size);
    if (exec_ctx->device_enqueue_helper_ptr == NULL) {
      CL_FREE(exec_ctx);
      return CL_OUT_OF_RESOURCES;
    }
    memset(exec_ctx->device_enqueue_helper_ptr, 0, buf_size);
    exec_ctx->helper_bo_size = buf_size;

    exec_ctx->device_enqueue_helper_bo =
      intel_buffer_alloc_userptr(ctx_gen->drv->bufmgr, "CL device enqueue helper object",
                                 exec_ctx->device_enqueue_helper_ptr, buf_size, 0);
    assert(exec_ctx->device_enqueue_helper_bo);

    drm_intel_bo_set_softpin_offset(exec_ctx->device_enqueue_helper_bo,
                                    (size_t)exec_ctx->device_enqueue_helper_ptr);
    drm_intel_bo_use_48b_address_range(exec_ctx->device_enqueue_helper_bo, 1);
    drm_intel_bo_disable_reuse(exec_ctx->device_enqueue_helper_bo);

    for (i = 0; i < ker->arg_n; i++) {
      if (ker->args[i].arg_type != ArgTypePointer &&
          ker->args[i].arg_type != ArgTypePipe && ker->args[i].arg_type != ArgTypeImage)
        continue;

      if (ker->args[i].arg_type == ArgTypePointer && ker->args[i].arg_addrspace == AddressSpaceLocal)
        continue;

      mem = ker->args[i].val.val_mem;
      if (mem == NULL)
        continue;

      if (ker->args[i].use_svm) // Already SVM
        continue;

      mem_gen = (cl_mem_gen)mem->each_device[0];
      assert(mem_gen);
      assert(mem_gen->drm_bo);
      assert(mem_gen->mem_base.device == queue->device);

      /* Just find a unused virtual address for binding, make the BO always use same address in GTT */
      drm_intel_bo_map(mem_gen->drm_bo->bo, 1);
      tmp_ptr = mem_gen->drm_bo->bo->virtual;
      drm_intel_bo_set_softpin_offset(mem_gen->drm_bo->bo, (size_t)tmp_ptr);
      drm_intel_bo_use_48b_address_range(mem_gen->drm_bo->bo, 1);
      drm_intel_bo_disable_reuse(mem_gen->drm_bo->bo);
      drm_intel_bo_unmap(mem_gen->drm_bo->bo);
    }
  }

  err = cl_command_queue_ND_range(queue, ker, exec_ctx, work_dim, global_wk_off, global_wk_sz, local_wk_sz);
  if (err != CL_SUCCESS) {
    if (exec_ctx->device_enqueue_helper_bo)
      drm_intel_bo_unreference(exec_ctx->device_enqueue_helper_bo);
    if (exec_ctx->device_enqueue_helper_ptr)
      CL_FREE(exec_ctx->device_enqueue_helper_ptr);
    CL_FREE(exec_ctx);
    return err;
  }

  e->exec_data.exec_ctx = exec_ctx;
  return err;
}

LOCAL int
cl_command_queue_flush_gpgpu(void *gpgpu)
{
  gen_gpgpu_exec_ctx *gpu_exec_ctx = gpgpu;
  gen_gpgpu *gpu;
  cl_int i;

  for (i = 0; i < gpu_exec_ctx->gpu_num; i++) {
    gpu = gpu_exec_ctx->all_gpu[i];
    assert(gpu);

    if (!gpu->batch || !gpu->batch->buffer)
      return CL_INVALID_VALUE;

    if (intel_batchbuffer_flush(gpu->batch) < 0)
      return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;

  /* FIXME:
     Remove old assert here for binded buffer offset 0 which
     tried to guard possible NULL buffer pointer check in kernel, as
     in case like "runtime_null_kernel_arg", but that's wrong to just
     take buffer offset 0 as NULL, and cause failure for normal
     kernels which has no such NULL ptr check but with buffer offset 0
     (which is possible now and will be normal if full PPGTT is on).

     Need to fix NULL ptr check otherwise.
  */
}

typedef struct ndrange_info_t {
  int type;
  int global_work_size[3];
  int local_work_size[3];
  int global_work_offset[3];
} ndrange_info_t;

typedef struct Block_literal {
  void *isa; // initialized to &_NSConcreteStackBlock or &_NSConcreteGlobalBlock
  int flags;
  int reserved;
  size_t index;
  struct Block_descriptor_1 {
    unsigned long int slm_size; // NULL
    unsigned long int size;     // sizeof(struct Block_literal_1)
    // optional helper functions
    void *copy_helper;    // IFF (1<<25)
    void *dispose_helper; // IFF (1<<25)
    // required ABI.2010.3.16
    const char *signature; // IFF (1<<30)
  } * descriptor;
  // imported variables
} Block_literal;

static cl_int
cl_command_queue_gen_device_enqueue_once(cl_command_queue queue, cl_kernel kernel, drm_intel_bufmgr *bufmgr,
                                         const uint32_t work_dim, const size_t *global_wk_off,
                                         const size_t *global_dim_off, const size_t *global_wk_sz,
                                         const size_t *global_wk_sz_use, const size_t *local_wk_sz,
                                         const size_t *local_wk_sz_use, gen_gpgpu_exec_ctx *gpu_ctx)
{
  cl_int ret = CL_SUCCESS;
  gen_gpgpu *gpu = NULL;
  size_t local_size = local_wk_sz_use[0] * local_wk_sz_use[1] * local_wk_sz_use[2];
  cl_kernel_gen kernel_gen;
  cl_program_gen prog_gen;
  cl_context_gen ctx_gen;
  gen_gpgpu *parent_gpu = gpu_ctx->all_gpu[0];
  assert(parent_gpu);
  cl_uint i;

  DEV_PRIVATE_DATA(kernel, queue->device, kernel_gen);
  DEV_PRIVATE_DATA(kernel->program, queue->device, prog_gen);
  DEV_PRIVATE_DATA(queue->ctx, queue->device, ctx_gen);

  ret = check_work_group_capability(queue, kernel, local_wk_sz_use, 3);
  if (ret != CL_SUCCESS) {
    return ret;
  }

  if (kernel_gen->scratch_size > queue->device->scratch_mem_size) {
    return CL_OUT_OF_RESOURCES;
  }

  gpu = CL_CALLOC(1, sizeof(gen_gpgpu));
  if (gpu == NULL) {
    return CL_OUT_OF_HOST_MEMORY;
  }

  gpu->bufmgr = bufmgr;
  gpu->simd_size = kernel_gen->simd_width;
  gpu->device = queue->device;
  gpu->thread.max_thread_num = queue->device->max_compute_unit * queue->device->max_thread_per_unit;
  gpu->thread.thread_num = (local_size + gpu->simd_size - 1) / gpu->simd_size;
  gpu->sampler.sampler_bitmap = ~((1 << max_sampler_n) - 1);
  gpu->mem.max_bti = 0;
  gpu->mem.per_thread_scratch_size = kernel_gen->scratch_size;
  gpu->mem.total_scratch_size = 0;
  gpu->atomic_test_result = gpu->device->atomic_test_result;
  gpu->thread.barrier_slm_used = kernel_gen->barrier_slm_used;

  // TODO: Need to handle SLM here
  gpu->mem.local_mem_size = 0;

  // Setup the kernel bitcode and upload it to GPU side
  gpu->kernel_bo = drm_intel_bo_alloc(bufmgr, "CL kernel", kernel_gen->kern_base.exec_code_sz, 64u);
  if (gpu->kernel_bo == NULL) {
    cl_command_queue_delete_gpgpu(gpu);
    return CL_OUT_OF_RESOURCES;
  }
  /* Upload the bitcode */
  drm_intel_bo_subdata(gpu->kernel_bo, 0, kernel_gen->kern_base.exec_code_sz,
                       kernel_gen->kern_base.exec_code);

  ret = gen_gpgpu_setup_curbe(kernel, kernel_gen, gpu, work_dim, global_wk_off, global_wk_sz,
                              local_wk_sz_use, local_wk_sz, (uint64_t)gpu_ctx->device_enqueue_helper_ptr);
  if (ret != CL_SUCCESS) {
    cl_command_queue_delete_gpgpu(gpu);
    return ret;
  }

  gpu->thread.num_cs_entries = 64;
  gpu->thread.size_cs_entry = gpu->thread.curbe_size / 32;

  ret = gen_gpgpu_setup_aux(gpu);
  if (ret != CL_SUCCESS) {
    cl_command_queue_delete_gpgpu(gpu);
    return ret;
  }

  /* Copy the aux setting of the parent kernel except curbe */
  dri_bo_map(parent_gpu->aux.aux_bo, 1);
  memcpy(gpu->aux.aux_bo->virtual + gpu->aux.surface_heap_offset,
         parent_gpu->aux.aux_bo->virtual + parent_gpu->aux.surface_heap_offset,
         sizeof(surface_heap_t));
  memcpy(gpu->aux.aux_bo->virtual + gpu->aux.sampler_state_offset,
         parent_gpu->aux.aux_bo->virtual + parent_gpu->aux.sampler_state_offset,
         MAX(GEN_MAX_SAMPLERS * sizeof(gen6_sampler_state_t), GEN_MAX_VME_STATES * sizeof(gen7_vme_state_t)));
  memcpy(gpu->aux.aux_bo->virtual + gpu->aux.sampler_border_color_state_offset,
         parent_gpu->aux.aux_bo->virtual + parent_gpu->aux.sampler_border_color_state_offset,
         GEN_MAX_SAMPLERS * sizeof(gen7_sampler_border_color_t));
  dri_bo_unmap(parent_gpu->aux.aux_bo);

  /* Setup the kernel arg. First one must be SVM and SLM later */
  for (i = 0; i < kernel->arg_n; i++) {
    if (i == 0) {
      assert(kernel->args[i].arg_type == ArgTypePointer);
      assert(kernel->args[i].arg_addrspace == AddressSpaceGlobal);
      /* No need to bind BTI, already in parent's BTI table */
      *(uint64_t *)(gpu->thread.curbe + kernel_gen->arg_extra_info[i].arg_offset) =
        (uint64_t)gpu_ctx->device_enqueue_helper_ptr;
      continue;
    }

    assert(0); // TODO: SLM setting
  }

  ret = gen_gpgpu_setup_scratch(gpu);
  if (ret != CL_SUCCESS) {
    cl_command_queue_delete_gpgpu(gpu);
    return ret;
  }
  /* Bind a stack if needed */
  ret = gen_gpgu_bind_stack(gpu, kernel, kernel_gen);
  if (ret != CL_SUCCESS) {
    cl_command_queue_delete_gpgpu(gpu);
    return ret;
  }

  /* Must be a 2.0 OpenCL */
  ret = gen_setup_constant_buffer_for_20(kernel, kernel_gen, prog_gen, gpu);
  if (ret != CL_SUCCESS) {
    cl_command_queue_delete_gpgpu(gpu);
    return ret;
  }

  gen_gpgpu_build_idrt(gpu);
  gen_gpgpu_upload_final_curbe(kernel, kernel_gen, gpu, local_wk_sz_use);
  gen_gpgpu_finish_aux(gpu);

  /* Start a new batch buffer */
  gpu->batch = intel_batchbuffer_create(ctx_gen->drv, gen_gpu_compute_batch_sz(kernel));
  if (gpu->batch == NULL) {
    cl_command_queue_delete_gpgpu(gpu);
    return CL_OUT_OF_RESOURCES;
  }

  gen_gpgpu_batch_start(gpu);
  gen_gpgpu_walker(gpu, gpu->simd_size, gpu->thread.thread_num,
                   global_wk_off, global_dim_off, global_wk_sz_use, local_wk_sz_use);
  gen_gpgpu_batch_end(gpu, 0);

  if (intel_batchbuffer_flush(gpu->batch) < 0) {
    cl_command_queue_delete_gpgpu(gpu);
    return CL_INVALID_VALUE;
  }

  intel_batchbuffer_finish(gpu->batch);
  cl_command_queue_delete_gpgpu(gpu);
  return CL_SUCCESS;
}

static cl_int
cl_command_queue_gen_device_enqueue(cl_command_queue queue, cl_kernel kernel, drm_intel_bufmgr *bufmgr,
                                    const uint32_t work_dim, const size_t *global_wk_off,
                                    const size_t *global_wk_sz, const size_t *local_wk_sz,
                                    gen_gpgpu_exec_ctx *gpu_ctx)
{
  cl_int err = CL_SUCCESS;
  int i, j, k;
  const size_t global_wk_sz_div[3] = {
    global_wk_sz[0] / local_wk_sz[0] * local_wk_sz[0],
    global_wk_sz[1] / local_wk_sz[1] * local_wk_sz[1],
    global_wk_sz[2] / local_wk_sz[2] * local_wk_sz[2]};

  const size_t global_wk_sz_rem[3] = {
    global_wk_sz[0] % local_wk_sz[0],
    global_wk_sz[1] % local_wk_sz[1],
    global_wk_sz[2] % local_wk_sz[2]};

  const size_t *global_wk_all[2] = {global_wk_sz_div, global_wk_sz_rem};
  /* Go through the at most 8 cases and euque if there is work items left */
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 2; j++) {
      for (k = 0; k < 2; k++) {
        size_t global_wk_sz_use[3] = {global_wk_all[k][0], global_wk_all[j][1], global_wk_all[i][2]};
        size_t global_dim_off[3] = {
          k * global_wk_sz_div[0] / local_wk_sz[0],
          j * global_wk_sz_div[1] / local_wk_sz[1],
          i * global_wk_sz_div[2] / local_wk_sz[2]};
        size_t local_wk_sz_use[3] = {
          k ? global_wk_sz_rem[0] : local_wk_sz[0],
          j ? global_wk_sz_rem[1] : local_wk_sz[1],
          i ? global_wk_sz_rem[2] : local_wk_sz[2]};
        if (local_wk_sz_use[0] == 0 || local_wk_sz_use[1] == 0 || local_wk_sz_use[2] == 0)
          continue;

        err = cl_command_queue_gen_device_enqueue_once(queue, kernel, bufmgr, work_dim, global_wk_off, global_dim_off,
                                                       global_wk_sz, global_wk_sz_use, local_wk_sz, local_wk_sz_use,
                                                       gpu_ctx);

        if (err != CL_SUCCESS)
          return err;
      }
      if (work_dim < 2)
        break;
    }
    if (work_dim < 3)
      break;
  }

  assert(err == CL_SUCCESS);
  return err;
}

/* If some device enqueue happen, we need to enqueue another enqueue_nd_range to imitate it */
static cl_int
cl_command_queue_gen_handle_device_enqueue(cl_command_queue queue, cl_kernel kernel, gen_gpgpu_exec_ctx *gpu_ctx)
{
  cl_program program = kernel->program;
  cl_kernel new_kernel;
  cl_program_gen program_gen;
  cl_context_gen ctx_gen;
  cl_int err = CL_SUCCESS;
  void *ptr;
  int type;
  int dim;
  char *name;
  int i;

  DEV_PRIVATE_DATA(queue->ctx, queue->device, ctx_gen);
  DEV_PRIVATE_DATA(program, queue->device, program_gen);

  assert(gpu_ctx->device_enqueue_helper_ptr);
  assert(gpu_ctx->device_enqueue_helper_bo);
  drm_intel_bo_wait_rendering(gpu_ctx->device_enqueue_helper_bo);

  int total_size = *(int *)gpu_ctx->device_enqueue_helper_ptr;
  ptr = gpu_ctx->device_enqueue_helper_ptr;
  ptr += sizeof(int);

  while (ptr - gpu_ctx->device_enqueue_helper_ptr < total_size) {
    size_t fixed_global_off[] = {0, 0, 0};
    size_t fixed_global_sz[] = {1, 1, 1};
    size_t fixed_local_sz[] = {1, 1, 1};
    ndrange_info_t *ndrange_info = (ndrange_info_t *)ptr;
    ptr += sizeof(ndrange_info_t);

    Block_literal *block = (Block_literal *)ptr;
    ptr += block->descriptor->size;

    type = ndrange_info->type;
    dim = (type & 0xf0) >> 4;
    type = type & 0xf;
    assert(dim <= 2);

    for (i = 0; i <= dim; i++) {
      fixed_global_sz[i] = ndrange_info->global_work_size[i];
      if (type > 1)
        fixed_local_sz[i] = ndrange_info->local_work_size[i];
      if (type > 2)
        fixed_global_off[i] = ndrange_info->global_work_offset[i];
    }

//    int *slm_sizes = (int *)ptr;
    int slm_size = block->descriptor->slm_size;
    ptr += slm_size;

    assert(block->index < program_gen->device_enqueue_info_num);
    name = program_gen->device_enqueue_info[block->index].kernel_name;

    new_kernel = CL_CALLOC(1, sizeof(struct _cl_kernel));
    if (new_kernel == NULL)
      return CL_OUT_OF_HOST_MEMORY;

    CL_OBJECT_INIT_BASE(new_kernel, CL_OBJECT_KERNEL_MAGIC);
    new_kernel->program = program;

    new_kernel->name = CL_CALLOC(1, strlen(name) + 1);
    if (new_kernel->name == NULL) {
      CL_FREE(new_kernel);
      return CL_OUT_OF_HOST_MEMORY;
    }
    memcpy(new_kernel->name, name, strlen(name) + 1);

    new_kernel->each_device = CL_CALLOC(program->each_device_num, sizeof(cl_kernel_for_device));
    if (new_kernel->each_device == NULL) {
      CL_FREE(new_kernel->name);
      CL_FREE(new_kernel);
      return CL_OUT_OF_HOST_MEMORY;
    }

    new_kernel->each_device_num = program->each_device_num;
    /* No need to add to program's list. */
    err = cl_kernel_create_gen(queue->device, new_kernel);
    if (err != CL_SUCCESS) {
      cl_kernel_delete_gen(queue->device, new_kernel);
      CL_FREE(new_kernel->each_device);
      CL_FREE(new_kernel->name);
      CL_FREE(new_kernel);
    }

    err = cl_command_queue_gen_device_enqueue(queue, new_kernel, ctx_gen->drv->bufmgr, dim + 1,
                                              fixed_global_off, fixed_global_sz, fixed_local_sz, gpu_ctx);

    cl_kernel_delete_gen(queue->device, new_kernel);
    CL_FREE(new_kernel->each_device);
    CL_FREE(new_kernel->name);
    CL_FREE(new_kernel);

    if (err != CL_SUCCESS)
      return err;
  }

  return CL_SUCCESS;
}

LOCAL int
cl_command_queue_finish_gpgpu(void *gpgpu)
{
  gen_gpgpu_exec_ctx *gpu_exec_ctx = gpgpu;
  gen_gpgpu *gpu;
  cl_int i;

  for (i = 0; i < gpu_exec_ctx->gpu_num; i++) {
    gpu = gpu_exec_ctx->all_gpu[i];
    assert(gpu);

    if (!gpu->batch || !gpu->batch->buffer)
      return CL_INVALID_VALUE;

    intel_batchbuffer_finish(gpu->batch);
  }

  return CL_SUCCESS;
}

LOCAL cl_int
cl_enqueue_handle_nd_range_gen(cl_event event, cl_int status)
{
  cl_int err = CL_SUCCESS;

  assert(event->exec_data.type == EnqueueNDRangeKernel);

  if (status == CL_QUEUED) {
    size_t fixed_global_off[] = {0, 0, 0};
    size_t fixed_global_sz[] = {1, 1, 1};
    size_t fixed_local_sz[] = {1, 1, 1};
    cl_command_queue queue = event->queue;
    cl_kernel kernel = event->exec_data.nd_range.kernel;
    cl_int work_dim = event->exec_data.nd_range.work_dim;
    size_t *global_wk_off = event->exec_data.nd_range.global_wk_off;
    size_t *global_wk_sz = event->exec_data.nd_range.global_wk_sz;
    size_t *local_wk_sz = event->exec_data.nd_range.local_wk_sz;
    cl_int i;

    if (local_wk_sz[0] != 0 || local_wk_sz[1] != 0 || local_wk_sz[2] != 0) {
      for (i = 0; i < work_dim; ++i) {
        fixed_local_sz[i] = local_wk_sz[i];
      }
    } else {
      uint j, maxDimSize = 64 /* from 64? */, maxGroupSize = 256; //MAX_WORK_GROUP_SIZE may too large
      size_t realGroupSize = 1;
      for (i = 0; i < work_dim; i++) {
        for (j = maxDimSize; j > 1; j--) {
          if (global_wk_sz[i] % j == 0 && j <= maxGroupSize) {
            fixed_local_sz[i] = j;
            maxGroupSize = maxGroupSize / j;
            maxDimSize = maxGroupSize > maxDimSize ? maxDimSize : maxGroupSize;
            break; //choose next work_dim
          }
        }
        realGroupSize *= fixed_local_sz[i];
      }

      //in a loop of conformance test (such as test_api repeated_setup_cleanup), in each loop:
      //create a new context, a new command queue, and uses 'globalsize[0]=1000, localsize=NULL' to enqueu kernel
      //it triggers the following message for many times.
      //to avoid too many messages, only print it for the first time of the process.
      //just use static variable since it doesn't matter to print a few times at multi-thread case.
      static int warn_no_good_localsize = 1;
      if (realGroupSize % 8 != 0 && warn_no_good_localsize) {
        warn_no_good_localsize = 0;
        CL_LOG_WARNING("unable to find good values for local_work_size[i], please provide\n"
                       " local_work_size[] explicitly, you can find good values with\n"
                       " trial-and-error method.");
      }
    }

    for (i = 0; i < work_dim; ++i)
      fixed_global_sz[i] = global_wk_sz[i];

    if (global_wk_off[0] != 0 || global_wk_off[1] != 0 || global_wk_off[2] != 0)
      for (i = 0; i < work_dim; ++i)
        fixed_global_off[i] = global_wk_off[i];

    if (kernel->compile_wg_sz[0] || kernel->compile_wg_sz[1] || kernel->compile_wg_sz[2]) {
      if (fixed_local_sz[0] != kernel->compile_wg_sz[0] ||
          fixed_local_sz[1] != kernel->compile_wg_sz[1] ||
          fixed_local_sz[2] != kernel->compile_wg_sz[2]) {
        err = CL_INVALID_WORK_GROUP_SIZE;
        return err;
      }
    }

    err = cl_command_queue_ND_range_wrap(queue, kernel, event, work_dim, fixed_global_off,
                                         fixed_global_sz, fixed_local_sz);
    return err;
  }

  if (status == CL_SUBMITTED) {
    assert(event->exec_data.exec_ctx);
    err = cl_command_queue_flush_gpgpu(event->exec_data.exec_ctx);
    return err;
  }

  if (status == CL_RUNNING) {
    /* Nothing to do */
    return CL_SUCCESS;
  }

  assert(status == CL_COMPLETE);
  assert(event->exec_data.exec_ctx);
  err = cl_command_queue_finish_gpgpu(event->exec_data.exec_ctx);

  if (err == CL_SUCCESS) {
    if (((gen_gpgpu_exec_ctx *)event->exec_data.exec_ctx)->device_enqueue_helper_ptr) {
      err = cl_command_queue_gen_handle_device_enqueue(event->queue, event->exec_data.nd_range.kernel,
                                                       event->exec_data.exec_ctx);
    }
  }

  /* If profiling, we will delay the GPU's delete to event's delete */
  if ((event->queue->props & CL_QUEUE_PROFILING_ENABLE) == 0) {
    cl_enqueue_nd_range_delete_gen(event);
    event->exec_data.exec_ctx = NULL;
  }

  return err;
}

LOCAL void
cl_enqueue_nd_range_delete_gen(cl_event event)
{
  gen_gpgpu_exec_ctx *gpu_exec_ctx = event->exec_data.exec_ctx;

  if (gpu_exec_ctx) {
    gen_gpgpu *gpu;
    cl_int i;

    if (gpu_exec_ctx->device_enqueue_helper_bo) {
      drm_intel_bo_unreference(gpu_exec_ctx->device_enqueue_helper_bo);
      gpu_exec_ctx->device_enqueue_helper_bo = NULL;
    }
    if (gpu_exec_ctx->device_enqueue_helper_ptr) {
      CL_FREE(gpu_exec_ctx->device_enqueue_helper_ptr);
      gpu_exec_ctx->device_enqueue_helper_ptr = NULL;
    }

    for (i = 0; i < gpu_exec_ctx->gpu_num; i++) {
      gpu = gpu_exec_ctx->all_gpu[i];
      assert(gpu);
      cl_command_queue_delete_gpgpu(gpu);
    }

    CL_FREE(gpu_exec_ctx);
    event->exec_data.exec_ctx = NULL;
  }
}

LOCAL cl_int
cl_command_queue_create_gen(cl_device_id device, cl_command_queue queue)
{
  return CL_SUCCESS;
}

LOCAL void
cl_command_queue_delete_gen(cl_device_id device, cl_command_queue queue)
{
}
