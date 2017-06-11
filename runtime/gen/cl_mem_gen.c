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
 */

#include "cl_gen.h"
#include <unistd.h>

/* All drm list for buffer, image and SVM usage, for debug */
static list_head gen_drm_bo_list = {{&(gen_drm_bo_list.head_node), &(gen_drm_bo_list.head_node)}};
static pthread_mutex_t gen_drm_bo_list_mutex = PTHREAD_MUTEX_INITIALIZER;

LOCAL cl_mem_drm_bo
cl_mem_gen_create_drm_bo(dri_bufmgr *bufmgr, size_t size, size_t alignment,
                         cl_image_gen_tiling tiling, size_t stride, void *orig_data)
{
  cl_mem_drm_bo drm_bo = CL_CALLOC(1, sizeof(_cl_mem_drm_bo));
  if (drm_bo == NULL)
    return NULL;

  /* HSW: Byte scattered Read/Write has limitation that
     the buffer size must be a multiple of 4 bytes. */
  size = ALIGN(size, 4);

  drm_bo->bo = drm_intel_bo_alloc(bufmgr, "CL memory object", size, alignment);
  if (drm_bo->bo == NULL) {
    CL_FREE(drm_bo);
    return NULL;
  }

  CL_OBJECT_INIT_BASE(drm_bo, CL_OBJECT_DRM_BO_MAGIC);
  drm_bo->gpu_size = size;
  drm_bo->tiling = tiling;
  drm_bo->stride = stride;
  drm_bo->host_coherent = CL_FALSE;
  intel_buffer_set_tiling(drm_bo->bo, drm_bo->tiling, drm_bo->stride);

  if (orig_data)
    drm_intel_bo_subdata(drm_bo->bo, 0, size, orig_data);

  pthread_mutex_lock(&gen_drm_bo_list_mutex);
  list_add_tail(&gen_drm_bo_list, &drm_bo->base.node);
  pthread_mutex_unlock(&gen_drm_bo_list_mutex);

  return drm_bo;
}

LOCAL cl_mem_drm_bo
cl_mem_gen_create_drm_bo_from_hostptr(dri_bufmgr *bufmgr, cl_bool svm,
                                      size_t size, cl_uint cacheline_size, void *host_ptr)
{
#ifdef HAS_USERPTR
  int page_size = getpagesize();

  if ((ALIGN((unsigned long)host_ptr, cacheline_size) != (unsigned long)host_ptr) ||
      (ALIGN((unsigned long)size, cacheline_size) != (unsigned long)size)) {
    /* Must Align to a cache line size, or GPU will overwrite the data when cache flush */
    return NULL;
  }

  cl_mem_drm_bo drm_bo = CL_CALLOC(1, sizeof(_cl_mem_drm_bo));
  if (drm_bo == NULL)
    return NULL;

  CL_OBJECT_INIT_BASE(drm_bo, CL_OBJECT_DRM_BO_MAGIC);
  drm_bo->host_coherent = CL_TRUE;
  drm_bo->mapped_ptr = (void *)(((unsigned long)host_ptr) & (~(page_size - 1)));
  drm_bo->in_page_offset = host_ptr - drm_bo->mapped_ptr;
  drm_bo->gpu_size = ALIGN((drm_bo->in_page_offset + size), page_size);
  drm_bo->bo = intel_buffer_alloc_userptr(bufmgr, "CL userptr memory object",
                                          drm_bo->mapped_ptr, drm_bo->gpu_size, 0);
  if (drm_bo->bo == NULL) {
    CL_FREE(drm_bo);
    return NULL;
  }

  if (svm) {
    drm_intel_bo_set_softpin_offset(drm_bo->bo, (size_t)drm_bo->mapped_ptr);
    drm_intel_bo_use_48b_address_range(drm_bo->bo, 1);
    drm_bo->svm = CL_TRUE;
  }

  pthread_mutex_lock(&gen_drm_bo_list_mutex);
  list_add_tail(&gen_drm_bo_list, &drm_bo->base.node);
  pthread_mutex_unlock(&gen_drm_bo_list_mutex);

  return drm_bo;
#else
  return NULL;
#endif
}

LOCAL void
cl_mem_gen_drm_bo_ref(cl_mem_drm_bo drm_bo)
{
  assert(CL_OBJECT_IS_DRM_BO(drm_bo));
  assert(drm_bo->bo);
  CL_OBJECT_INC_REF(drm_bo);
}

LOCAL void
cl_mem_gen_drm_bo_delete(cl_mem_drm_bo drm_bo)
{
  assert(CL_OBJECT_IS_DRM_BO(drm_bo));
  assert(drm_bo->bo);

  if (CL_OBJECT_DEC_REF(drm_bo) > 1)
    return;

  pthread_mutex_lock(&gen_drm_bo_list_mutex);
  list_node_del(&drm_bo->base.node);
  pthread_mutex_unlock(&gen_drm_bo_list_mutex);

  if (drm_bo->drm_map_ref > 0) {
    CL_LOG_WARNING("Pay Attention: the drm object: %p is destroying but still hole %d map references",
                   drm_bo->bo, drm_bo->drm_map_ref);
  }
  drm_intel_bo_unreference(drm_bo->bo);
  CL_OBJECT_DESTROY_BASE(drm_bo);
  CL_FREE(drm_bo);
}

LOCAL void *
cl_mem_gen_drm_bo_map(cl_mem_drm_bo drm_bo, cl_bool unsync)
{
  cl_bool already_sync = CL_FALSE;
  void *ret_ptr = NULL;

  assert(CL_OBJECT_IS_DRM_BO(drm_bo));
  assert(drm_bo->bo);

  CL_OBJECT_TAKE_OWNERSHIP(drm_bo, 1);
  if (drm_bo->drm_map_ref != 0) {
    assert(drm_bo->mapped_ptr != NULL);
    assert(drm_bo->mapped_ptr == drm_bo->bo->virtual);
  } else {
    if (drm_bo->host_coherent == CL_TRUE) {
      /* Host ptr never need call drm_map api */
      assert(drm_bo->tiling == CL_NO_TILE);
      assert(drm_bo->mapped_ptr == drm_bo->bo->virtual);
    } else if (drm_bo->tiling != CL_NO_TILE || unsync) {
      drm_intel_gem_bo_map_unsynchronized(drm_bo->bo);
      drm_bo->mapped_ptr = drm_bo->bo->virtual;
    } else {
      drm_intel_bo_map(drm_bo->bo, 1); // Always mapped write
      already_sync = CL_TRUE;
      drm_bo->mapped_ptr = drm_bo->bo->virtual;
    }
    assert(drm_bo->mapped_ptr != NULL);
  }

  drm_bo->drm_map_ref++;
  if (drm_bo->host_coherent == CL_TRUE) {
    ret_ptr = drm_bo->mapped_ptr + drm_bo->in_page_offset;
  } else {
    ret_ptr = drm_bo->mapped_ptr;
  }

  CL_OBJECT_RELEASE_OWNERSHIP(drm_bo);

  if (unsync == CL_FALSE && already_sync == CL_FALSE) {
    drm_intel_bo_wait_rendering(drm_bo->bo);
  }

  assert(ret_ptr);
  return ret_ptr;
}

LOCAL void
cl_mem_gen_drm_bo_unmap(cl_mem_drm_bo drm_bo)
{
  assert(CL_OBJECT_IS_DRM_BO(drm_bo));
  assert(drm_bo->bo);

  CL_OBJECT_TAKE_OWNERSHIP(drm_bo, 1);
  drm_bo->drm_map_ref--;
  assert(drm_bo->bo->virtual != NULL);
  assert(drm_bo->mapped_ptr == drm_bo->bo->virtual);
  assert(drm_bo->drm_map_ref >= 0);

  if (drm_bo->drm_map_ref == 0) {
    if (drm_bo->host_coherent == CL_FALSE) {
      drm_intel_bo_unmap(drm_bo->bo);
      assert(drm_bo->bo->virtual == NULL);
      drm_bo->mapped_ptr = NULL;
    }
  }
  CL_OBJECT_RELEASE_OWNERSHIP(drm_bo);
}

LOCAL void
cl_mem_gen_drm_bo_sync(cl_mem_drm_bo drm_bo)
{
  assert(CL_OBJECT_IS_DRM_BO(drm_bo));
  assert(drm_bo->bo);
  drm_intel_bo_wait_rendering(drm_bo->bo);
}

LOCAL cl_bool
cl_mem_gen_drm_bo_expand(cl_mem_drm_bo drm_bo, size_t new_size, size_t alignment)
{
  drm_intel_bo *new_bo;

  assert(CL_OBJECT_IS_DRM_BO(drm_bo));
  assert(drm_bo->bo);

  CL_OBJECT_TAKE_OWNERSHIP(drm_bo, 1);
  if (drm_bo->drm_map_ref > 0) { // Someone still mapping it, can not do this
    CL_OBJECT_RELEASE_OWNERSHIP(drm_bo);
    return CL_FALSE;
  }

  if (drm_bo->tiling != CL_NO_TILE) { /* Only support no tile mode */
    CL_OBJECT_RELEASE_OWNERSHIP(drm_bo);
    return CL_FALSE;
  }

  if (drm_bo->host_coherent == CL_TRUE) { /* If use host conherent ptr, can not expand */
    CL_OBJECT_RELEASE_OWNERSHIP(drm_bo);
    return CL_FALSE;
  }

  new_bo = drm_intel_bo_alloc(drm_bo->bo->bufmgr, "CL memory object", new_size, alignment);
  if (new_bo == NULL) {
    CL_OBJECT_RELEASE_OWNERSHIP(drm_bo);
    return CL_FALSE;
  }

  drm_intel_bo_wait_rendering(drm_bo->bo);

  drm_intel_bo_map(new_bo, 1);
  void *dst = new_bo->virtual;
  void *src = NULL;
  if (drm_bo->host_coherent) {
    src = drm_bo->mapped_ptr;
  } else {
    drm_intel_bo_map(drm_bo->bo, 1);
    src = drm_bo->bo->virtual;
  }
  assert(src);
  memset(dst, 0, new_size);
  memcpy(dst, src, drm_bo->gpu_size);

  drm_intel_bo_unmap(new_bo);
  if (drm_bo->host_coherent == CL_FALSE) {
    drm_intel_bo_unmap(drm_bo->bo);
  }

  /* Reset all field */
  drm_intel_bo_unreference(drm_bo->bo);
  assert(drm_bo->drm_map_ref == 0);
  drm_bo->bo = new_bo;
  drm_bo->gpu_size = new_size;
  drm_bo->host_coherent = CL_FALSE;
  drm_bo->mapped_ptr = NULL;
  drm_bo->in_page_offset = 0;
  drm_bo->tiling = CL_NO_TILE;
  drm_bo->stride = 0;

  CL_OBJECT_RELEASE_OWNERSHIP(drm_bo);
  return CL_TRUE;
}

LOCAL cl_bool
cl_mem_gen_drm_bo_upload_data(cl_mem_drm_bo drm_bo, size_t offset, void *data, size_t size)
{
  int err = 0;
  assert(CL_OBJECT_IS_DRM_BO(drm_bo));
  assert(drm_bo->bo);
  drm_intel_bo_wait_rendering(drm_bo->bo);

  CL_OBJECT_TAKE_OWNERSHIP(drm_bo, 1);

  if (drm_bo->host_coherent) {
    assert(drm_bo->mapped_ptr);
    assert(drm_bo->gpu_size >= offset + size);
    if (drm_bo->mapped_ptr + drm_bo->in_page_offset + offset != data)
      memcpy(drm_bo->mapped_ptr + drm_bo->in_page_offset + offset, data, size);
  } else {
    err = drm_intel_bo_subdata(drm_bo->bo, offset, size, data);
  }

  CL_OBJECT_RELEASE_OWNERSHIP(drm_bo);
  return (err == 0);
}

#define LOCAL_SZ_0 16
#define LOCAL_SZ_1 4
#define LOCAL_SZ_2 4

static cl_int
cl_mem_copy_buffer_gen(cl_command_queue queue, cl_event event, cl_mem src_buf,
                       cl_mem dst_buf, size_t src_offset, size_t dst_offset, size_t cb)
{
  cl_int ret = CL_SUCCESS;
  cl_kernel ker = NULL;
  size_t global_off[] = {0, 0, 0};
  size_t global_sz[] = {1, 1, 1};
  size_t local_sz[] = {1, 1, 1};
  const unsigned int masks[4] = {0xffffffff, 0x0ff, 0x0ffff, 0x0ffffff};
  int aligned = 0;
  int dw_src_offset = src_offset / 4;
  int dw_dst_offset = dst_offset / 4;

  /* We use one kernel to copy the data. The kernel is lazily created. */
  assert(src_buf->ctx == dst_buf->ctx);

  /* All 16 bytes aligned, fast and easy one. */
  if ((cb % 16 == 0) && (src_offset % 16 == 0) && (dst_offset % 16 == 0)) {
    ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                            CL_ENQUEUE_COPY_BUFFER_ALIGN16);
    cb = cb / 16;
    aligned = 1;
  } else if ((cb % 4 == 0) && (src_offset % 4 == 0) && (dst_offset % 4 == 0)) { /* all Dword aligned.*/
    ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                            CL_ENQUEUE_COPY_BUFFER_ALIGN4);
    cb = cb / 4;
    aligned = 1;
  }

  if (aligned) {
    assert(ker);

    if (cb < LOCAL_SZ_0) {
      local_sz[0] = 1;
    } else {
      local_sz[0] = LOCAL_SZ_0;
    }
    global_sz[0] = ((cb + LOCAL_SZ_0 - 1) / LOCAL_SZ_0) * LOCAL_SZ_0;
    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_buf);
    cl_kernel_set_arg(ker, 1, sizeof(int), &dw_src_offset);
    cl_kernel_set_arg(ker, 2, sizeof(cl_mem), &dst_buf);
    cl_kernel_set_arg(ker, 3, sizeof(int), &dw_dst_offset);
    cl_kernel_set_arg(ker, 4, sizeof(int), &cb);
    ret = cl_command_queue_ND_range_wrap(queue, ker, event, 1, global_off, global_sz, local_sz);
    return ret;
  }

  /* Now handle the unaligned cases. */
  int dw_num = ((dst_offset % 4 + cb) + 3) / 4;
  unsigned int first_mask = dst_offset % 4 == 0 ? 0x0 : masks[dst_offset % 4];
  unsigned int last_mask = masks[(dst_offset + cb) % 4];
  /* handle the very small range copy. */
  if (cb < 4 && dw_num == 1) {
    first_mask = first_mask | ~last_mask;
  }

  if (cb < LOCAL_SZ_0) {
    local_sz[0] = 1;
  } else {
    local_sz[0] = LOCAL_SZ_0;
  }
  global_sz[0] = ((dw_num + LOCAL_SZ_0 - 1) / LOCAL_SZ_0) * LOCAL_SZ_0;

  if (src_offset % 4 == dst_offset % 4) {
    /* Src and dst has the same unaligned offset, just handle the
       header and tail. */
    ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                            CL_ENQUEUE_COPY_BUFFER_UNALIGN_SAME_OFFSET);

    assert(ker);

    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_buf);
    cl_kernel_set_arg(ker, 1, sizeof(int), &dw_src_offset);
    cl_kernel_set_arg(ker, 2, sizeof(cl_mem), &dst_buf);
    cl_kernel_set_arg(ker, 3, sizeof(int), &dw_dst_offset);
    cl_kernel_set_arg(ker, 4, sizeof(int), &dw_num);
    cl_kernel_set_arg(ker, 5, sizeof(int), &first_mask);
    cl_kernel_set_arg(ker, 6, sizeof(int), &last_mask);
    ret = cl_command_queue_ND_range_wrap(queue, ker, event, 1, global_off, global_sz, local_sz);
    return ret;
  }

  /* Dst's offset < Src's offset, so one dst dword need two sequential src dwords to fill it. */
  if (dst_offset % 4 < src_offset % 4) {
    int align_diff = src_offset % 4 - dst_offset % 4;
    unsigned int dw_mask = masks[align_diff];
    int shift = align_diff * 8;

    ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                            CL_ENQUEUE_COPY_BUFFER_UNALIGN_DST_OFFSET);
    assert(ker);

    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_buf);
    cl_kernel_set_arg(ker, 1, sizeof(int), &dw_src_offset);
    cl_kernel_set_arg(ker, 2, sizeof(cl_mem), &dst_buf);
    cl_kernel_set_arg(ker, 3, sizeof(int), &dw_dst_offset);
    cl_kernel_set_arg(ker, 4, sizeof(int), &dw_num);
    cl_kernel_set_arg(ker, 5, sizeof(int), &first_mask);
    cl_kernel_set_arg(ker, 6, sizeof(int), &last_mask);
    cl_kernel_set_arg(ker, 7, sizeof(int), &shift);
    cl_kernel_set_arg(ker, 8, sizeof(int), &dw_mask);
    ret = cl_command_queue_ND_range_wrap(queue, ker, event, 1, global_off, global_sz, local_sz);
    return ret;
  }

  /* Dst's offset > Src's offset, so one dst dword need two sequential src - and src to fill it. */
  if (dst_offset % 4 > src_offset % 4) {
    int align_diff = dst_offset % 4 - src_offset % 4;
    unsigned int dw_mask = masks[4 - align_diff];
    int shift = align_diff * 8;
    int src_less = !(src_offset % 4) && !((src_offset + cb) % 4);

    ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                            CL_ENQUEUE_COPY_BUFFER_UNALIGN_SRC_OFFSET);
    assert(ker);

    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_buf);
    cl_kernel_set_arg(ker, 1, sizeof(int), &dw_src_offset);
    cl_kernel_set_arg(ker, 2, sizeof(cl_mem), &dst_buf);
    cl_kernel_set_arg(ker, 3, sizeof(int), &dw_dst_offset);
    cl_kernel_set_arg(ker, 4, sizeof(int), &dw_num);
    cl_kernel_set_arg(ker, 5, sizeof(int), &first_mask);
    cl_kernel_set_arg(ker, 6, sizeof(int), &last_mask);
    cl_kernel_set_arg(ker, 7, sizeof(int), &shift);
    cl_kernel_set_arg(ker, 8, sizeof(int), &dw_mask);
    cl_kernel_set_arg(ker, 9, sizeof(int), &src_less);
    ret = cl_command_queue_ND_range_wrap(queue, ker, event, 1, global_off, global_sz, local_sz);
    return ret;
  }

  /* no case can hanldle? */
  assert(0);
}

LOCAL cl_int
cl_mem_enqueue_copy_buffer_gen(cl_event event, cl_int status)
{
  cl_int ret = CL_SUCCESS;
  assert(event->exec_data.type == EnqueueCopyBuffer);

  if (event->exec_data.copy_buffer.cb == 0) // no need to do anything
    return CL_SUCCESS;

  if (status == CL_QUEUED) {
    ret = cl_mem_copy_buffer_gen(event->queue, event, event->exec_data.copy_buffer.src,
                                 event->exec_data.copy_buffer.dst,
                                 event->exec_data.copy_buffer.src_offset,
                                 event->exec_data.copy_buffer.dst_offset,
                                 event->exec_data.copy_buffer.cb);
    return ret;
  }

  if (status == CL_SUBMITTED) {
    assert(event->exec_data.exec_ctx);
    ret = cl_command_queue_flush_gpgpu(event->exec_data.exec_ctx);
    return ret;
  }

  if (status == CL_RUNNING) {
    /* Nothing to do */
    return CL_SUCCESS;
  }

  assert(status == CL_COMPLETE);
  assert(event->exec_data.exec_ctx);
  ret = cl_command_queue_finish_gpgpu(event->exec_data.exec_ctx);
  return ret;
}

LOCAL cl_int
cl_mem_enqueue_fill_buffer_gen(cl_event event, cl_int status)
{
  cl_int ret = CL_SUCCESS;
  assert(event->exec_data.type == EnqueueFillBuffer);

  if (event->exec_data.fill_buffer.size == 0) // no need to do anything
    return CL_SUCCESS;

  if (status == CL_QUEUED) {
    cl_command_queue queue = event->queue;
    const void *pattern = event->exec_data.fill_buffer.pattern;
    size_t pattern_size = event->exec_data.fill_buffer.pattern_size;
    cl_mem buffer = event->exec_data.fill_buffer.buffer;
    size_t offset = event->exec_data.fill_buffer.offset;
    size_t size = event->exec_data.fill_buffer.size;
    cl_kernel ker = NULL;
    size_t global_off[] = {0, 0, 0};
    size_t global_sz[] = {1, 1, 1};
    size_t local_sz[] = {1, 1, 1};
    char pattern_comb[4];
    int is_128 = 0;
    const void *pattern1 = NULL;

    assert(offset % pattern_size == 0);
    assert(size % pattern_size == 0);

    if (!size)
      return ret;

    if (pattern_size == 128) {
      /* 128 is according to pattern of double16, but double works not very
       well on some platform. We use two float16 to handle this. */
      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                              CL_ENQUEUE_FILL_BUFFER_ALIGN128);
      is_128 = 1;
      pattern_size = pattern_size / 2;
      pattern1 = pattern + pattern_size;
      size = size / 2;
    } else if (pattern_size % 8 == 0) { /* Handle the 8 16 32 64 cases here. */
      int order = ffs(pattern_size / 8) - 1;

      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                              CL_ENQUEUE_FILL_BUFFER_ALIGN8_8 + order);
    } else if (pattern_size == 4) {
      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                              CL_ENQUEUE_FILL_BUFFER_ALIGN4);
    } else if (size >= 4 && size % 4 == 0 && offset % 4 == 0) {
      /* The unaligned case. But if copy size and offset are aligned to 4, we can fake
       the pattern with the pattern duplication fill in. */
      assert(pattern_size == 1 || pattern_size == 2);

      if (pattern_size == 2) {
        memcpy(pattern_comb, pattern, sizeof(char) * 2);
        memcpy(pattern_comb + 2, pattern, sizeof(char) * 2);
      } else {
        pattern_comb[0] = pattern_comb[1] = pattern_comb[2] = pattern_comb[3] = *(char *)pattern;
      }

      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                              CL_ENQUEUE_FILL_BUFFER_ALIGN4);
      pattern_size = 4;
      pattern = pattern_comb;
    }
    //TODO: Unaligned cases, we may need to optimize it as cl_mem_copy, using mask in kernel
    //functions. This depend on the usage but now we just use aligned 1 and 2.
    else if (pattern_size == 2) {
      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                              CL_ENQUEUE_FILL_BUFFER_ALIGN2);
    } else if (pattern_size == 1) {
      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                              CL_ENQUEUE_FILL_BUFFER_UNALIGN);
    } else
      assert(0);

    assert(ker);

    size = size / pattern_size;
    offset = offset / pattern_size;

    if (size < LOCAL_SZ_0) {
      local_sz[0] = 1;
    } else {
      local_sz[0] = LOCAL_SZ_0;
    }
    global_sz[0] = ((size + LOCAL_SZ_0 - 1) / LOCAL_SZ_0) * LOCAL_SZ_0;
    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &buffer);
    cl_kernel_set_arg(ker, 1, pattern_size, pattern);
    cl_kernel_set_arg(ker, 2, sizeof(cl_uint), &offset);
    cl_kernel_set_arg(ker, 3, sizeof(cl_uint), &size);
    if (is_128)
      cl_kernel_set_arg(ker, 4, pattern_size, pattern1);

    ret = cl_command_queue_ND_range_wrap(queue, ker, event, 1, global_off, global_sz, local_sz);
    return ret;
  }

  if (status == CL_SUBMITTED) {
    assert(event->exec_data.exec_ctx);
    ret = cl_command_queue_flush_gpgpu(event->exec_data.exec_ctx);
    return ret;
  }

  if (status == CL_RUNNING) {
    /* Nothing to do */
    return CL_SUCCESS;
  }

  assert(status == CL_COMPLETE);
  assert(event->exec_data.exec_ctx);
  ret = cl_command_queue_finish_gpgpu(event->exec_data.exec_ctx);
  return ret;
}

LOCAL cl_int
cl_mem_enqueue_copy_buffer_rect_gen(cl_event event, cl_int status)
{
  cl_int ret = CL_SUCCESS;
  assert(event->exec_data.type == EnqueueCopyBufferRect);

  if (status == CL_QUEUED) {
    cl_command_queue queue = event->queue;
    cl_mem src_buf = event->exec_data.copy_buffer_rect.src_buf;
    cl_mem dst_buf = event->exec_data.copy_buffer_rect.dst_buf;
    const size_t *src_origin = event->exec_data.copy_buffer_rect.src_origin;
    const size_t *dst_origin = event->exec_data.copy_buffer_rect.dst_origin;
    const size_t *region = event->exec_data.copy_buffer_rect.region;
    size_t src_row_pitch = event->exec_data.copy_buffer_rect.src_row_pitch;
    size_t src_slice_pitch = event->exec_data.copy_buffer_rect.src_slice_pitch;
    size_t dst_row_pitch = event->exec_data.copy_buffer_rect.dst_row_pitch;
    size_t dst_slice_pitch = event->exec_data.copy_buffer_rect.dst_slice_pitch;
    cl_kernel ker = NULL;
    size_t global_off[] = {0, 0, 0};
    size_t global_sz[] = {1, 1, 1};
    size_t local_sz[] = {LOCAL_SZ_0, LOCAL_SZ_1, LOCAL_SZ_1};

    // the src and dst mem rect is continuous, the copy is degraded to buf copy
    if ((region[0] == dst_row_pitch) && (region[0] == src_row_pitch) &&
        (region[1] * src_row_pitch == src_slice_pitch) &&
        (region[1] * dst_row_pitch == dst_slice_pitch)) {
      cl_int src_offset = src_origin[2] * src_slice_pitch +
                          src_origin[1] * src_row_pitch + src_origin[0];
      cl_int dst_offset = dst_origin[2] * dst_slice_pitch +
                          dst_origin[1] * dst_row_pitch + dst_origin[0];
      cl_int size = region[0] * region[1] * region[2];
      ret = cl_mem_copy_buffer_gen(queue, event, src_buf, dst_buf, src_offset, dst_offset, size);
      return ret;
    }

    if (region[1] == 1)
      local_sz[1] = 1;
    if (region[2] == 1)
      local_sz[2] = 1;
    global_sz[0] = ((region[0] + local_sz[0] - 1) / local_sz[0]) * local_sz[0];
    global_sz[1] = ((region[1] + local_sz[1] - 1) / local_sz[1]) * local_sz[1];
    global_sz[2] = ((region[2] + local_sz[2] - 1) / local_sz[2]) * local_sz[2];
    cl_int src_offset = src_origin[2] * src_slice_pitch + src_origin[1] * src_row_pitch + src_origin[0];
    cl_int dst_offset = dst_origin[2] * dst_slice_pitch + dst_origin[1] * dst_row_pitch + dst_origin[0];

    /* We use one kernel to copy the data. The kernel is lazily created. */
    assert(src_buf->ctx == dst_buf->ctx);

    /* setup the kernel and run. */
    size_t region0 = region[0];
    if ((src_offset % 4 == 0) && (dst_offset % 4 == 0) &&
        (src_row_pitch % 4 == 0) && (dst_row_pitch % 4 == 0) &&
        (src_slice_pitch % 4 == 0) && (dst_slice_pitch % 4 == 0) && (region0 % 4 == 0)) {
      region0 /= 4;
      src_offset /= 4;
      dst_offset /= 4;
      src_row_pitch /= 4;
      dst_row_pitch /= 4;
      src_slice_pitch /= 4;
      dst_slice_pitch /= 4;
      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_COPY_BUFFER_RECT_ALIGN4);
    } else {
      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_COPY_BUFFER_RECT);
    }

    assert(ker);

    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_buf);
    cl_kernel_set_arg(ker, 1, sizeof(cl_mem), &dst_buf);
    cl_kernel_set_arg(ker, 2, sizeof(cl_int), &region0);
    cl_kernel_set_arg(ker, 3, sizeof(cl_int), &region[1]);
    cl_kernel_set_arg(ker, 4, sizeof(cl_int), &region[2]);
    cl_kernel_set_arg(ker, 5, sizeof(cl_int), &src_offset);
    cl_kernel_set_arg(ker, 6, sizeof(cl_int), &dst_offset);
    cl_kernel_set_arg(ker, 7, sizeof(cl_int), &src_row_pitch);
    cl_kernel_set_arg(ker, 8, sizeof(cl_int), &src_slice_pitch);
    cl_kernel_set_arg(ker, 9, sizeof(cl_int), &dst_row_pitch);
    cl_kernel_set_arg(ker, 10, sizeof(cl_int), &dst_slice_pitch);

    ret = cl_command_queue_ND_range_wrap(queue, ker, event, 1, global_off, global_sz, local_sz);
    return ret;
  }

  if (status == CL_SUBMITTED) {
    assert(event->exec_data.exec_ctx);
    ret = cl_command_queue_flush_gpgpu(event->exec_data.exec_ctx);
    return ret;
  }

  if (status == CL_RUNNING) {
    /* Nothing to do */
    return CL_SUCCESS;
  }

  assert(status == CL_COMPLETE);
  assert(event->exec_data.exec_ctx);
  ret = cl_command_queue_finish_gpgpu(event->exec_data.exec_ctx);
  return ret;
}

static cl_int
cl_mem_allocate_pipe_gen(cl_device_id device, cl_mem mem)
{
  cl_context_gen ctx_gen;
  cl_mem_gen mem_gen;
  size_t alignment = 64;
  size_t total_sz;
  cl_uint *ptr = NULL;
  cl_mem_pipe pipe = NULL;

  assert(mem->size != 0);
  DEV_PRIVATE_DATA(mem->ctx, device, ctx_gen);
  assert(ctx_gen);

  mem_gen = CL_CALLOC(1, sizeof(_cl_mem_gen));
  if (mem_gen == NULL)
    return CL_OUT_OF_HOST_MEMORY;

  mem_gen->mem_base.device = device;
  mem->each_device[0] = (cl_mem_for_device)mem_gen;

  total_sz = mem->size;
  /* HSW: Byte scattered Read/Write has limitation that
	   the buffer size must be a multiple of 4 bytes. */
  total_sz = ALIGN(total_sz, 4);
  //The head of pipe is for data struct, and alignment to 128 byte for max data type double16
  total_sz += 128;

  mem_gen->drm_bo = cl_mem_gen_create_drm_bo(ctx_gen->drv->bufmgr, total_sz, alignment,
                                             CL_NO_TILE, 0, NULL);
  assert(mem_gen->drm_bo);

  pipe = cl_mem_to_pipe(mem);

  ptr = cl_mem_gen_drm_bo_map(mem_gen->drm_bo, CL_FALSE);
  assert(ptr);
  ptr[0] = pipe->max_packets;
  ptr[1] = pipe->packet_size;
  ptr[2] = 0; //write ptr
  ptr[3] = 0; //read ptr
  ptr[4] = 0; //reservation read ptr
  ptr[5] = 0; //reservation write ptr
  ptr[6] = 0; //packet num
  cl_mem_gen_drm_bo_unmap(mem_gen->drm_bo);

  return CL_SUCCESS;
}

static cl_int
cl_mem_allocate_buffer_gen(cl_device_id device, cl_mem mem)
{
  cl_context_gen ctx_gen;
  cl_mem_gen mem_gen;
  size_t alignment = 64;

  assert(mem->size != 0);
  DEV_PRIVATE_DATA(mem->ctx, device, ctx_gen);
  assert(ctx_gen);

  mem_gen = CL_CALLOC(1, sizeof(_cl_mem_gen));
  if (mem_gen == NULL)
    return CL_OUT_OF_HOST_MEMORY;

  mem_gen->mem_base.device = device;
  mem->each_device[0] = (cl_mem_for_device)mem_gen;

  /* Pinning will require stricter alignment rules */
  if (mem->flags & CL_MEM_PINNABLE)
    alignment = 4096;

  if (mem->flags & CL_MEM_USE_HOST_PTR && device->host_unified_memory) {
    if (cl_mem_to_buffer(mem)->svm_buf) {
      cl_mem svm = cl_mem_to_buffer(mem)->svm_buf;
      cl_mem_gen svm_gen;
      assert(CL_OBJECT_IS_SVM(svm));
      DEV_PRIVATE_DATA(svm, device, svm_gen);
      mem_gen->drm_bo = svm_gen->drm_bo;
      cl_mem_gen_drm_bo_ref(mem_gen->drm_bo);
    } else {
      mem_gen->drm_bo = cl_mem_gen_create_drm_bo_from_hostptr(
        ctx_gen->drv->bufmgr, CL_FALSE, mem->size, device->global_mem_cache_line_size, mem->host_ptr);
      if (mem_gen->drm_bo == NULL)
        mem_gen->drm_bo = cl_mem_gen_create_drm_bo(ctx_gen->drv->bufmgr, mem->size, alignment,
                                                   CL_NO_TILE, 0, mem->host_ptr);
    }
  } else {
    mem_gen->drm_bo = cl_mem_gen_create_drm_bo(ctx_gen->drv->bufmgr, mem->size, alignment,
                                               CL_NO_TILE, 0, mem->host_ptr);
  }
  assert(mem_gen->drm_bo);

  if (mem->flags & CL_MEM_COPY_HOST_PTR) {
    assert(mem->host_ptr);
    mem->host_ptr = NULL;
  }

  return CL_SUCCESS;
}

/* We hold mem->ownership when call this */
LOCAL cl_int
cl_mem_allocate_gen(cl_device_id device, cl_mem mem)
{
  cl_int err = CL_SUCCESS;
  cl_mem_gen mem_gen = NULL;

  assert(!CL_OBJECT_IS_SVM(mem));

  if (mem->each_device[0]) // Already allocate
    return CL_SUCCESS;

  if (CL_OBJECT_IS_SUB_BUFFER(mem)) {
    /* Parent must have already allocated */
    assert(cl_mem_to_buffer(mem)->parent->base.each_device[0]);
    if (cl_mem_to_buffer(mem)->parent->base.each_device[0]->device != device) {
      /* Parent and sub buffer can not belong to different device */
      return CL_MEM_OBJECT_ALLOCATION_FAILURE;
    }

    /* Just point to parent's private data */
    mem_gen = CL_CALLOC(1, sizeof(_cl_mem_gen));
    if (mem_gen == NULL)
      return CL_OUT_OF_HOST_MEMORY;

    mem_gen->mem_base.device = device;
    mem_gen->drm_bo = ((cl_mem_gen)(cl_mem_to_buffer(mem)->parent->base.each_device[0]))->drm_bo;
    cl_mem_gen_drm_bo_ref(mem_gen->drm_bo);
    mem->each_device[0] = (cl_mem_for_device)mem_gen;
  } else if (CL_OBJECT_IS_BUFFER(mem)) {
    err = cl_mem_allocate_buffer_gen(device, mem);
  } else if (CL_OBJECT_IS_IMAGE(mem)) {
    err = cl_mem_allocate_image_gen(device, mem);
  } else if (CL_OBJECT_IS_PIPE(mem)) {
    err = cl_mem_allocate_pipe_gen(device, mem);
  } else {
    assert(0);
  }

  return err;
}

LOCAL void
cl_mem_deallocate_gen(cl_device_id device, cl_mem mem)
{
  cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];
  assert(!CL_OBJECT_IS_SVM(mem));

  if (mem_gen == NULL)
    return;

  assert(mem_gen->drm_bo);
  cl_mem_gen_drm_bo_delete(mem_gen->drm_bo);
  mem_gen->drm_bo = NULL;
  CL_FREE(mem_gen);
  mem->each_device[0] = NULL;
}

LOCAL cl_int
cl_svm_create_gen(cl_device_id device, cl_mem svm_mem)
{
  cl_mem_gen mem_gen = NULL;
  cl_context_gen ctx_gen;
  int page_size;
  page_size = getpagesize();
  cl_mem_svm svm = cl_mem_to_svm(svm_mem);

  DEV_PRIVATE_DATA(svm_mem->ctx, device, ctx_gen);
  assert(ctx_gen);

  if (svm->real_size == 0 || ALIGN(svm->real_size, page_size) != svm->real_size)
    return CL_DEVICE_MEM_BASE_ADDR_ALIGN;

  if (svm_mem->host_ptr == NULL ||
      ALIGN((size_t)svm_mem->host_ptr, page_size) != (size_t)svm_mem->host_ptr)
    return CL_DEVICE_MEM_BASE_ADDR_ALIGN;

  mem_gen = CL_CALLOC(1, sizeof(_cl_mem_gen));
  if (mem_gen == NULL)
    return CL_OUT_OF_HOST_MEMORY;

  mem_gen->mem_base.device = device;
  mem_gen->drm_bo = cl_mem_gen_create_drm_bo_from_hostptr(
    ctx_gen->drv->bufmgr, CL_TRUE, svm->real_size,
    device->global_mem_cache_line_size, svm_mem->host_ptr);

  if (mem_gen->drm_bo == NULL) {
    CL_FREE(mem_gen);
    return CL_OUT_OF_RESOURCES;
  }

  ASSIGN_DEV_PRIVATE_DATA(svm_mem, device, (cl_mem_for_device)mem_gen);
  return CL_SUCCESS;
}

LOCAL void
cl_svm_delete_gen(cl_device_id device, cl_mem svm_mem)
{
  cl_mem_gen mem_gen = NULL;
  DEV_PRIVATE_DATA(svm_mem, device, mem_gen);
  if (mem_gen == NULL)
    return;

  assert(mem_gen->drm_bo);
  cl_mem_gen_drm_bo_delete(mem_gen->drm_bo);
  mem_gen->drm_bo = NULL;
  CL_FREE(mem_gen);
}

static cl_int
cl_enqueue_handle_map_buffer_gen(cl_event event, cl_int status)
{
  cl_mem mem = event->exec_data.map_buffer.mem_obj;
  cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];
  void *ptr = NULL;
  assert(mem_gen);
  assert(mem_gen->drm_bo);
  assert(event->exec_data.map_buffer.size <= mem->size);

  if (status == CL_SUBMITTED || status == CL_RUNNING)
    return CL_SUCCESS;

  if (status == CL_QUEUED) {
    ptr = cl_mem_gen_drm_bo_map(mem_gen->drm_bo, event->exec_data.map_buffer.unsync_map);
    assert(ptr);
    if (CL_OBJECT_IS_SUB_BUFFER(mem)) {
      ptr += cl_mem_to_buffer(mem)->sub_offset;
    }

    ptr += event->exec_data.map_buffer.offset;
    if (mem->flags & CL_MEM_USE_HOST_PTR) {
      assert(mem->host_ptr);
      event->exec_data.map_buffer.ptr = mem->host_ptr + event->exec_data.map_buffer.offset;
    } else {
      event->exec_data.map_buffer.ptr = ptr;
    }

    event->exec_data.exec_ctx = ptr; // Find a place to store the mapped ptr temp
    if (cl_mem_to_buffer(mem)->svm_buf) {
      /* If from a svm, we never need to copy, always host coherent. */
      assert(mem->flags & CL_MEM_USE_HOST_PTR);
      event->exec_data.exec_ctx = event->exec_data.map_buffer.ptr;
    }

    return CL_SUCCESS;
  }

  assert(status == CL_COMPLETE);

  if (event->exec_data.map_buffer.unsync_map)
    cl_mem_gen_drm_bo_sync(mem_gen->drm_bo);

  ptr = event->exec_data.exec_ctx;
  assert(ptr);

  /* Sync back the data to host if fake USE_HOST_PTR */
  if ((mem->flags & CL_MEM_USE_HOST_PTR) && ptr != event->exec_data.map_buffer.ptr) {
    /* Should never overlap with the real buffer mapped address */
    assert((ptr + event->exec_data.map_buffer.size <= event->exec_data.map_buffer.ptr) ||
           (event->exec_data.map_buffer.ptr + event->exec_data.map_buffer.size <= ptr));
    memcpy(event->exec_data.map_buffer.ptr, ptr, event->exec_data.map_buffer.size);
  }

  return CL_SUCCESS;
}

LOCAL cl_int
cl_enqueue_map_mem_gen(cl_event event, cl_int status)
{
  if (event->exec_data.type == EnqueueMapBuffer)
    return cl_enqueue_handle_map_buffer_gen(event, status);

  if (event->exec_data.type == EnqueueMapImage)
    return cl_enqueue_handle_map_image_gen(event, status);

  assert(0);
  return CL_INVALID_VALUE;
}

static cl_int
cl_enqueue_handle_unmap_buffer_gen(cl_event event, cl_int status)
{
  cl_mem mem = event->exec_data.unmap.mem_obj;
  cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];

  assert(mem_gen);
  assert(mem_gen->drm_bo);
  assert(event->exec_data.unmap.ptr);
  assert(event->exec_data.unmap.size > 0);

  if (status == CL_QUEUED || status == CL_RUNNING || status == CL_SUBMITTED)
    return CL_SUCCESS;

  /* Sync back the content if fake USE_HOST_PTR */
  if (mem->flags & CL_MEM_USE_HOST_PTR &&
      event->exec_data.unmap.ptr != mem->host_ptr + event->exec_data.unmap.offset) {
    /* SVM never comes to here */
    assert(cl_mem_to_buffer(mem)->svm_buf == NULL);
    assert(mem_gen->drm_bo->mapped_ptr);

    void *dst_ptr = mem_gen->drm_bo->mapped_ptr + event->exec_data.unmap.offset;
    if (CL_OBJECT_IS_SUB_BUFFER(mem))
      dst_ptr += cl_mem_to_buffer(mem)->sub_offset;

    /* Should never overlap with the real buffer mapped address */
    assert((event->exec_data.unmap.ptr + event->exec_data.unmap.size <= dst_ptr) ||
           (dst_ptr + event->exec_data.unmap.size <= event->exec_data.unmap.ptr));
    memcpy(dst_ptr, event->exec_data.unmap.ptr, event->exec_data.unmap.size);
  }

  cl_mem_gen_drm_bo_unmap(mem_gen->drm_bo);
  return CL_SUCCESS;
}

LOCAL cl_int
cl_enqueue_unmap_mem_gen(cl_event event, cl_int status)
{
  assert(event->exec_data.type == EnqueueUnmapMemObject);
  assert(CL_OBJECT_IS_MEM(event->exec_data.unmap.mem_obj));

  if (CL_OBJECT_IS_BUFFER(event->exec_data.unmap.mem_obj))
    return cl_enqueue_handle_unmap_buffer_gen(event, status);

  if (CL_OBJECT_IS_IMAGE(event->exec_data.unmap.mem_obj))
    return cl_enqueue_handle_unmap_image_gen(event, status);

  assert(0);
  return CL_INVALID_VALUE;
}

LOCAL cl_int
cl_enqueue_read_buffer_gen(cl_event event, cl_int status)
{
  cl_mem_gen mem_gen = NULL;
  void *data_ptr = NULL;
  cl_mem mem = NULL;

  if (event->exec_data.type == EnqueueReadBuffer) {
    mem = event->exec_data.read_write_buffer.buffer;
  } else if (event->exec_data.type == EnqueueReadBufferRect) {
    mem = event->exec_data.read_write_buffer_rect.buffer;
  } else {
    assert(0);
  }

  mem_gen = (cl_mem_gen)mem->each_device[0];
  assert(mem_gen);
  assert(mem_gen->drm_bo);
  assert(CL_OBJECT_IS_BUFFER(mem));

  if (status == CL_QUEUED || status == CL_RUNNING || status == CL_SUBMITTED)
    return CL_SUCCESS;

  data_ptr = cl_mem_gen_drm_bo_map(mem_gen->drm_bo, CL_FALSE);
  if (data_ptr == NULL)
    return CL_OUT_OF_RESOURCES;

  if (CL_OBJECT_IS_SUB_BUFFER(mem)) {
    data_ptr += cl_mem_to_buffer(mem)->sub_offset;
  }

  if (event->exec_data.type == EnqueueReadBuffer) {
    /* sometimes, application invokes read buffer, instead of map buffer, even if userptr is enabled
       memcpy is not necessary for this case */
    if (event->exec_data.read_write_buffer.ptr != (char *)data_ptr + event->exec_data.read_write_buffer.offset) {
      memcpy(event->exec_data.read_write_buffer.ptr,
             (char *)data_ptr + event->exec_data.read_write_buffer.offset, event->exec_data.read_write_buffer.size);
    }
  } else if (event->exec_data.type == EnqueueReadBufferRect) {
    void *dst_ptr = NULL;
    size_t *origin = event->exec_data.read_write_buffer_rect.origin;
    size_t *host_origin = event->exec_data.read_write_buffer_rect.host_origin;
    size_t *region = event->exec_data.read_write_buffer_rect.region;
    size_t host_row_pitch = event->exec_data.read_write_buffer_rect.host_row_pitch;
    size_t host_slice_pitch = event->exec_data.read_write_buffer_rect.host_slice_pitch;
    size_t row_pitch = event->exec_data.read_write_buffer_rect.row_pitch;
    size_t slice_pitch = event->exec_data.read_write_buffer_rect.slice_pitch;
    size_t offset = origin[0] + row_pitch * origin[1] + slice_pitch * origin[2];
    data_ptr = (char *)data_ptr + offset;
    offset = host_origin[0] + host_row_pitch * host_origin[1] + host_slice_pitch * host_origin[2];
    dst_ptr = (char *)event->exec_data.read_write_buffer_rect.ptr + offset;

    if (row_pitch == region[0] && row_pitch == host_row_pitch &&
        (region[2] == 1 || (slice_pitch == region[0] * region[1] && slice_pitch == host_slice_pitch))) {
      memcpy(dst_ptr, data_ptr, region[2] == 1 ? row_pitch * region[1] : slice_pitch * region[2]);
    } else {
      cl_uint y, z;
      for (z = 0; z < region[2]; z++) {
        const char *src = data_ptr;
        char *dst = dst_ptr;
        for (y = 0; y < region[1]; y++) {
          memcpy(dst, src, region[0]);
          src += row_pitch;
          dst += host_row_pitch;
        }
        data_ptr = (char *)data_ptr + slice_pitch;
        dst_ptr = (char *)dst_ptr + host_slice_pitch;
      }
    }
  }

  cl_mem_gen_drm_bo_unmap(mem_gen->drm_bo);
  return CL_SUCCESS;
}

/* Write use subdata, no map need */
LOCAL cl_int
cl_enqueue_write_buffer_gen(cl_event event, cl_int status)
{
  cl_mem mem = event->exec_data.read_write_buffer.buffer;
  cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];

  assert(mem_gen);
  assert(mem_gen->drm_bo);
  assert(CL_OBJECT_IS_BUFFER(mem));
  assert(event->exec_data.type == EnqueueWriteBuffer);

  if (status == CL_QUEUED || status == CL_RUNNING || status == CL_SUBMITTED)
    return CL_SUCCESS;

  size_t offset = event->exec_data.read_write_buffer.offset;
  if (CL_OBJECT_IS_SUB_BUFFER(mem)) {
    offset += cl_mem_to_buffer(mem)->sub_offset;
  }

  if (cl_mem_gen_drm_bo_upload_data(mem_gen->drm_bo, offset, event->exec_data.read_write_buffer.ptr,
                                    event->exec_data.read_write_buffer.size) == CL_TRUE) {
    return CL_SUCCESS;
  }

  return CL_OUT_OF_RESOURCES;
}

LOCAL cl_int
cl_enqueue_write_buffer_rect_gen(cl_event event, cl_int status)
{
  cl_mem_gen mem_gen = NULL;
  void *data_ptr = NULL;
  cl_mem mem = event->exec_data.read_write_buffer_rect.buffer;

  mem_gen = (cl_mem_gen)mem->each_device[0];
  assert(mem_gen);
  assert(mem_gen->drm_bo);
  assert(CL_OBJECT_IS_BUFFER(mem));
  assert(event->exec_data.type == EnqueueWriteBufferRect);

  if (status == CL_QUEUED || status == CL_RUNNING || status == CL_SUBMITTED)
    return CL_SUCCESS;

  void *src_ptr = NULL;
  size_t *origin = event->exec_data.read_write_buffer_rect.origin;
  size_t *host_origin = event->exec_data.read_write_buffer_rect.host_origin;
  size_t *region = event->exec_data.read_write_buffer_rect.region;
  size_t host_row_pitch = event->exec_data.read_write_buffer_rect.host_row_pitch;
  size_t host_slice_pitch = event->exec_data.read_write_buffer_rect.host_slice_pitch;
  size_t row_pitch = event->exec_data.read_write_buffer_rect.row_pitch;
  size_t slice_pitch = event->exec_data.read_write_buffer_rect.slice_pitch;

  data_ptr = cl_mem_gen_drm_bo_map(mem_gen->drm_bo, CL_FALSE);
  if (data_ptr == NULL)
    return CL_OUT_OF_RESOURCES;

  if (CL_OBJECT_IS_SUB_BUFFER(mem)) {
    data_ptr += cl_mem_to_buffer(mem)->sub_offset;
  }

  size_t offset = origin[0] + row_pitch * origin[1] + slice_pitch * origin[2];
  data_ptr = (char *)data_ptr + offset;

  offset = host_origin[0] + host_row_pitch * host_origin[1] + host_slice_pitch * host_origin[2];
  src_ptr = (char *)event->exec_data.read_write_buffer_rect.ptr + offset;

  if (row_pitch == region[0] && row_pitch == host_row_pitch &&
      (region[2] == 1 || (slice_pitch == region[0] * region[1] && slice_pitch == host_slice_pitch))) {
    memcpy(data_ptr, src_ptr, region[2] == 1 ? row_pitch * region[1] : slice_pitch * region[2]);
  } else {
    cl_uint y, z;
    for (z = 0; z < region[2]; z++) {
      const char *src = src_ptr;
      char *dst = data_ptr;
      for (y = 0; y < region[1]; y++) {
        memcpy(dst, src, region[0]);
        src += host_row_pitch;
        dst += row_pitch;
      }
      src_ptr = (char *)src_ptr + host_slice_pitch;
      data_ptr = (char *)data_ptr + slice_pitch;
    }
  }

  cl_mem_gen_drm_bo_unmap(mem_gen->drm_bo);
  return CL_SUCCESS;
}

LOCAL cl_int
cl_enqueue_svm_map_gen(cl_event event, cl_int status)
{
  cl_command_queue queue = event->queue;
  cl_mem_gen mem_gen = NULL;
  cl_mem mem = event->exec_data.svm_map.svm;

  if (status == CL_QUEUED || status == CL_RUNNING || status == CL_SUBMITTED)
    return CL_SUCCESS;

  DEV_PRIVATE_DATA(mem, queue->device, mem_gen);
  assert(mem_gen->drm_bo);
  assert(mem_gen->drm_bo->svm);
  assert(mem_gen->drm_bo->gpu_size >= event->exec_data.svm_map.size);

  cl_mem_gen_drm_bo_sync(mem_gen->drm_bo);
  return CL_SUCCESS;
}

LOCAL cl_int
cl_enqueue_svm_unmap_gen(cl_event event, cl_int status)
{
  cl_command_queue queue = event->queue;
  cl_mem_gen mem_gen = NULL;
  cl_mem mem = event->exec_data.svm_unmap.svm;

  if (status == CL_QUEUED || status == CL_RUNNING || status == CL_SUBMITTED)
    return CL_SUCCESS;

  DEV_PRIVATE_DATA(mem, queue->device, mem_gen);
  assert(mem_gen->drm_bo);
  assert(mem_gen->drm_bo->svm);

  cl_mem_gen_drm_bo_sync(mem_gen->drm_bo);
  return CL_SUCCESS;
}

LOCAL cl_int
cl_enqueue_svm_fill_gen(cl_event event, cl_int status)
{
  cl_command_queue queue = event->queue;
  cl_mem_gen mem_gen = NULL;
  cl_mem mem = event->exec_data.svm_fill.svm;
  size_t i, j;

  if (status == CL_QUEUED || status == CL_RUNNING || status == CL_SUBMITTED)
    return CL_SUCCESS;

  DEV_PRIVATE_DATA(mem, queue->device, mem_gen);
  assert(mem_gen->drm_bo);
  assert(mem_gen->drm_bo->svm);
  assert(event->exec_data.svm_fill.ptr >= mem->host_ptr);

  cl_mem_gen_drm_bo_sync(mem_gen->drm_bo);

  for (i = 0; i < event->exec_data.svm_fill.size;) {
    for (j = 0; j < event->exec_data.svm_fill.pattern_size; j++) {
      ((char *)event->exec_data.svm_fill.ptr)[i++] =
        ((char *)event->exec_data.svm_fill.pattern)[j];
    }
  }

  return CL_SUCCESS;
}

LOCAL cl_int
cl_enqueue_svm_copy_gen(cl_event event, cl_int status)
{
  cl_command_queue queue = event->queue;
  cl_mem_gen src_mem_gen = NULL;
  cl_mem_gen dst_mem_gen = NULL;
  cl_mem src_mem = event->exec_data.svm_copy.src;
  cl_mem dst_mem = event->exec_data.svm_copy.dst;

  if (status == CL_QUEUED || status == CL_RUNNING || status == CL_SUBMITTED)
    return CL_SUCCESS;

  DEV_PRIVATE_DATA(src_mem, queue->device, src_mem_gen);
  DEV_PRIVATE_DATA(dst_mem, queue->device, dst_mem_gen);
  assert(event->exec_data.svm_copy.src_ptr >= src_mem->host_ptr);
  assert(event->exec_data.svm_copy.dst_ptr >= dst_mem->host_ptr);

  cl_mem_gen_drm_bo_sync(src_mem_gen->drm_bo);
  cl_mem_gen_drm_bo_sync(dst_mem_gen->drm_bo);

  memcpy(event->exec_data.svm_copy.dst_ptr,
         event->exec_data.svm_copy.src_ptr, event->exec_data.svm_copy.size);
  return CL_SUCCESS;
}
