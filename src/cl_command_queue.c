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

#include "program.h" // for BTI_MAX_IMAGE_NUM
#include "cl_command_queue.h"
#include "cl_context.h"
#include "cl_program.h"
#include "cl_kernel.h"
#include "cl_device_id.h"
#include "cl_mem.h"
#include "cl_utils.h"
#include "cl_thread.h"
#include "cl_alloc.h"
#include "cl_driver.h"
#include "cl_khr_icd.h"
#include "cl_event.h"
#include "performance.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

LOCAL cl_command_queue
cl_command_queue_new(cl_context ctx)
{
  cl_command_queue queue = NULL;

  assert(ctx);
  TRY_ALLOC_NO_ERR (queue, CALLOC(struct _cl_command_queue));
  SET_ICD(queue->dispatch)
  queue->magic = CL_MAGIC_QUEUE_HEADER;
  queue->ref_n = 1;
  queue->ctx = ctx;
  if ((queue->thread_data = cl_thread_data_create()) == NULL) {
    goto error;
  }

  /* Append the command queue in the list */
  pthread_mutex_lock(&ctx->queue_lock);
    queue->next = ctx->queues;
    if (ctx->queues != NULL)
      ctx->queues->prev = queue;
    ctx->queues = queue;
  pthread_mutex_unlock(&ctx->queue_lock);

  /* The queue also belongs to its context */
  cl_context_add_ref(ctx);

exit:
  return queue;
error:
  cl_command_queue_delete(queue);
  queue = NULL;
  goto exit;
}

LOCAL void
cl_command_queue_delete(cl_command_queue queue)
{
  assert(queue);
  if (atomic_dec(&queue->ref_n) != 1) return;

  // If there is a valid last event, we need to give it a chance to
  // call the call-back function.
  if (queue->last_event && queue->last_event->user_cb)
    cl_event_update_status(queue->last_event, 1);
  /* Remove it from the list */
  assert(queue->ctx);
  pthread_mutex_lock(&queue->ctx->queue_lock);
    if (queue->prev)
      queue->prev->next = queue->next;
    if (queue->next)
      queue->next->prev = queue->prev;
    if (queue->ctx->queues == queue)
      queue->ctx->queues = queue->next;
  pthread_mutex_unlock(&queue->ctx->queue_lock);
  if (queue->fulsim_out != NULL) {
    cl_mem_delete(queue->fulsim_out);
    queue->fulsim_out = NULL;
  }

  cl_thread_data_destroy(queue);
  queue->thread_data = NULL;
  cl_mem_delete(queue->perf);
  cl_context_delete(queue->ctx);
  cl_free(queue->wait_events);
  queue->magic = CL_MAGIC_DEAD_HEADER; /* For safety */
  cl_free(queue);
}

LOCAL void
cl_command_queue_add_ref(cl_command_queue queue)
{
  atomic_inc(&queue->ref_n);
}

static void
set_image_info(char *curbe,
               struct ImageInfo * image_info,
               struct _cl_mem_image *image)
{
  if (image_info->wSlot >= 0)
    *(uint32_t*)(curbe + image_info->wSlot) = image->w;
  if (image_info->hSlot >= 0)
    *(uint32_t*)(curbe + image_info->hSlot) = image->h;
  if (image_info->depthSlot >= 0)
    *(uint32_t*)(curbe + image_info->depthSlot) = image->depth;
  if (image_info->channelOrderSlot >= 0)
    *(uint32_t*)(curbe + image_info->channelOrderSlot) = image->fmt.image_channel_order;
  if (image_info->dataTypeSlot >= 0)
    *(uint32_t*)(curbe + image_info->dataTypeSlot) = image->fmt.image_channel_data_type;
}

LOCAL cl_int
cl_command_queue_bind_image(cl_command_queue queue, cl_kernel k)
{
  uint32_t i;
  GET_QUEUE_THREAD_GPGPU(queue);

  for (i = 0; i < k->image_sz; i++) {
    int id = k->images[i].arg_idx;
    struct _cl_mem_image *image;
    assert(interp_kernel_get_arg_type(k->opaque, id) == GBE_ARG_IMAGE);
    image = cl_mem_image(k->args[id].mem);
    set_image_info(k->curbe, &k->images[i], image);
    cl_gpgpu_bind_image(gpgpu, k->images[i].idx, image->base.bo, image->offset,
                        image->intel_fmt, image->image_type,
                        image->w, image->h, image->depth,
                        image->row_pitch, (cl_gpgpu_tiling)image->tiling);
    // TODO, this workaround is for GEN7/GEN75 only, we may need to do it in the driver layer
    // on demand.
    if (image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
      cl_gpgpu_bind_image(gpgpu, k->images[i].idx + BTI_MAX_IMAGE_NUM, image->base.bo, image->offset,
                          image->intel_fmt, image->image_type,
                          image->w, image->h, image->depth,
                          image->row_pitch, (cl_gpgpu_tiling)image->tiling);
  }
  return CL_SUCCESS;
}

LOCAL cl_int
cl_command_queue_bind_surface(cl_command_queue queue, cl_kernel k)
{
  GET_QUEUE_THREAD_GPGPU(queue);

  /* Bind all user buffers (given by clSetKernelArg) */
  uint32_t i;
  enum gbe_arg_type arg_type; /* kind of argument */
  for (i = 0; i < k->arg_n; ++i) {
    uint32_t offset; // location of the address in the curbe
    arg_type = interp_kernel_get_arg_type(k->opaque, i);
    if (arg_type != GBE_ARG_GLOBAL_PTR || !k->args[i].mem)
      continue;
    offset = interp_kernel_get_curbe_offset(k->opaque, GBE_CURBE_KERNEL_ARGUMENT, i);
    if (k->args[i].mem->type == CL_MEM_SUBBUFFER_TYPE) {
      struct _cl_mem_buffer* buffer = (struct _cl_mem_buffer*)k->args[i].mem;
      cl_gpgpu_bind_buf(gpgpu, k->args[i].mem->bo, offset, buffer->sub_offset, k->args[i].mem->size, interp_kernel_get_arg_bti(k->opaque, i));
    } else {
      cl_gpgpu_bind_buf(gpgpu, k->args[i].mem->bo, offset, 0, k->args[i].mem->size, interp_kernel_get_arg_bti(k->opaque, i));
    }
  }

  return CL_SUCCESS;
}


#if USE_FULSIM
extern void drm_intel_bufmgr_gem_stop_aubfile(cl_buffer_mgr);
extern void drm_intel_bufmgr_gem_set_aubfile(cl_buffer_mgr, FILE*);
extern void aub_exec_dump_raw_file(cl_buffer, size_t offset, size_t sz);

static void
cl_run_fulsim(void)
{
  const char *run_it = getenv("OCL_SIMULATOR");
  const char *debug_mode = getenv("OCL_FULSIM_DEBUG_MODE");
  if (run_it == NULL || strcmp(run_it, "1")) return;

#if EMULATE_GEN == 7 /* IVB */
  if (debug_mode == NULL || strcmp(debug_mode, "1"))
    system("wine AubLoad.exe dump.aub -device ivbB0");
  else
    system("wine AubLoad.exe dump.aub -device ivbB0 -debug");
#elif EMULATE_GEN == 75 /* HSW */
  if (debug_mode == NULL || strcmp(debug_mode, "1"))
    system("wine AubLoad.exe dump.aub -device hsw.h.a0");
  else
    system("wine AubLoad.exe dump.aub -device hsw.h.a0 -debug");
#else
#error "Unknown device"
#endif
}

/* Each buffer is dump using several chunks of this size */
static const size_t chunk_sz = 8192u;

static cl_int
cl_fulsim_dump_all_surfaces(cl_command_queue queue, cl_kernel k)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  int i;
  size_t j;

  /* Bind user defined surface */
  for (i = 0; i < k->arg_n; ++i) {
    size_t chunk_n, chunk_remainder;
    if (interp_kernel_get_arg_type(k->opaque, i) != GBE_ARG_GLOBAL_PTR)
      continue;
    mem = (cl_mem) k->args[i].mem;
    CHECK_MEM(mem);
    chunk_n = cl_buffer_get_size(mem->bo) / chunk_sz;
    chunk_remainder = cl_buffer_get_size(mem->bo) % chunk_sz;
    for (j = 0; j < chunk_n; ++j)
      aub_exec_dump_raw_file(mem->bo, j * chunk_sz, chunk_sz);
    if (chunk_remainder)
      aub_exec_dump_raw_file(mem->bo, chunk_n * chunk_sz, chunk_remainder);
  }
error:
  return err;
}

struct bmphdr {
  /* 2 bytes of magic here, "BM", total header size is 54 bytes! */
  int filesize;      /*  4 total file size incl header */
  short as0, as1;    /*  8 app specific */
  int bmpoffset;     /* 12 ofset of bmp data  */
  int headerbytes;   /* 16 bytes in header from this point (40 actually) */
  int width;         /* 20  */
  int height;        /* 24  */
  short nplanes;     /* 26 no of color planes */
  short bpp;         /* 28 bits/pixel */
  int compression;   /* 32 BI_RGB = 0 = no compression */
  int sizeraw;       /* 36 size of raw bmp file, excluding header, incl padding */
  int hres;          /* 40 horz resolutions pixels/meter */
  int vres;          /* 44 */
  int npalcolors;    /* 48 No of colors in palette */
  int nimportant;    /* 52 No of important colors */
  /* raw b, g, r data here, dword aligned per scan line */
};

static int*
cl_read_bmp(const char *filename, int *width, int *height)
{
  int n;
  struct bmphdr hdr;

  FILE *fp = fopen(filename, "rb");
  assert(fp);

  char magic[2];
  n = fread(&magic[0], 1, 2, fp);
  assert(n == 2 && magic[0] == 'B' && magic[1] == 'M');

  n = fread(&hdr, 1, sizeof(hdr), fp);
  assert(n == sizeof(hdr));

  assert(hdr.width > 0 &&
         hdr.height > 0 &&
         hdr.nplanes == 1
         && hdr.compression == 0);

  int *rgb32 = (int *) cl_malloc(hdr.width * hdr.height * sizeof(int));
  assert(rgb32);
  int x, y;

  int *dst = rgb32;
  for (y = 0; y < hdr.height; y++) {
    for (x = 0; x < hdr.width; x++) {
      assert(!feof(fp));
      int b = (getc(fp) & 0x0ff);
      int g = (getc(fp) & 0x0ff);
      int r = (getc(fp) & 0x0ff);
      *dst++ = (r | (g << 8) | (b << 16) | 0xff000000);	/* abgr */
    }
    while (x & 3) {
      getc(fp);
      x++;
    }
  }
  fclose(fp);
  *width = hdr.width;
  *height = hdr.height;
  return rgb32;
}

static char*
cl_read_dump(const char *name, size_t *size)
{
  char *raw = NULL, *dump = NULL;
  size_t i, sz;
  int w, h;
  if ((raw = (char*) cl_read_bmp(name, &w, &h)) == NULL)
    return NULL;
  sz = w * h;
  dump = (char*) cl_malloc(sz);
  assert(dump);
  for (i = 0; i < sz; ++i)
    dump[i] = raw[4*i];
  cl_free(raw);
  if (size)
    *size = sz;
  return dump;
}

static cl_int
cl_fulsim_read_all_surfaces(cl_command_queue queue, cl_kernel k)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  char *from = NULL, *to = NULL;
  size_t size, j, chunk_n, chunk_remainder;
  int i, curr = 0;
  /* Bind user defined surface */
  for (i = 0; i < k->arg_n; ++i) {
    if (interp_kernel_get_arg_type(k->opaque, i) != GBE_ARG_GLOBAL_PTR)
      continue;
    mem = (cl_mem) k->args[i].mem;
    CHECK_MEM(mem);
    assert(mem->bo);
    chunk_n = cl_buffer_get_size(mem->bo) / chunk_sz;
    chunk_remainder = cl_buffer_get_size(mem->bo) % chunk_sz;
    to = cl_mem_map(mem, 1);
    for (j = 0; j < chunk_n; ++j) {
      char name[256];
      sprintf(name, "dump%03i.bmp", curr);
#ifdef NDEBUG
      from = cl_read_dump(name, NULL);
#else
      from = cl_read_dump(name, &size);
      assert(size == chunk_sz);
#endif /* NDEBUG */
      memcpy(to + j*chunk_sz, from, chunk_sz);
      cl_free(from);
      curr++;
    }
    if (chunk_remainder) {
      char name[256];
      sprintf(name, "dump%03i.bmp", curr);
#ifdef NDEBUG
      from = cl_read_dump(name, NULL);
#else
      from = cl_read_dump(name, &size);
      assert(size == chunk_remainder);
#endif /* NDEBUG */
      memcpy(to + chunk_n*chunk_sz, from, chunk_remainder);
      cl_free(from);
      curr++;
    }
    cl_mem_unmap(mem);
  }
error:
  return err;
}
#endif

extern cl_int cl_command_queue_ND_range_gen7(cl_command_queue, cl_kernel, uint32_t, const size_t *, const size_t *, const size_t *);

static cl_int
cl_kernel_check_args(cl_kernel k)
{
  uint32_t i;
  for (i = 0; i < k->arg_n; ++i)
    if (k->args[i].is_set == CL_FALSE)
      return CL_INVALID_KERNEL_ARGS;
  return CL_SUCCESS;
}

LOCAL cl_int
cl_command_queue_ND_range(cl_command_queue queue,
                          cl_kernel k,
                          const uint32_t work_dim,
                          const size_t *global_wk_off,
                          const size_t *global_wk_sz,
                          const size_t *local_wk_sz)
{
  if(b_output_kernel_perf)
    time_start(queue->ctx, cl_kernel_get_name(k), queue);
  const int32_t ver = cl_driver_get_ver(queue->ctx->drv);
  cl_int err = CL_SUCCESS;

  /* Check that the user did not forget any argument */
  TRY (cl_kernel_check_args, k);

#if USE_FULSIM
  cl_buffer_mgr bufmgr = NULL;
  FILE *file = NULL;
  const char *run_it = getenv("OCL_SIMULATOR");
  if (run_it != NULL && strcmp(run_it, "1") == 0) {
    file = fopen("dump.aub", "wb");
    FATAL_IF (file == NULL, "Unable to open file dump.aub");
    bufmgr = cl_context_get_bufmgr(queue->ctx);
    drm_intel_bufmgr_gem_set_aubfile(bufmgr, file);
  }
#endif /* USE_FULSIM */

  if (ver == 7 || ver == 75 || ver == 8)
    TRY (cl_command_queue_ND_range_gen7, queue, k, work_dim, global_wk_off, global_wk_sz, local_wk_sz);
  else
    FATAL ("Unknown Gen Device");

#if USE_FULSIM
  if (run_it != NULL && strcmp(run_it, "1") == 0) {
    TRY (cl_fulsim_dump_all_surfaces, queue, k);
    drm_intel_bufmgr_gem_stop_aubfile(bufmgr);
    fclose(file);
    cl_run_fulsim();
    TRY (cl_fulsim_read_all_surfaces, queue, k);
  }
#endif /* USE_FULSIM */

error:
  return err;
}

LOCAL void
cl_command_queue_flush_gpgpu(cl_command_queue queue, cl_gpgpu gpgpu)
{
  size_t global_wk_sz[3];
  void* printf_info = cl_gpgpu_get_printf_info(gpgpu, global_wk_sz);

  cl_gpgpu_flush(gpgpu);

  if (printf_info && interp_get_printf_num(printf_info)) {
    void *index_addr = cl_gpgpu_map_printf_buffer(gpgpu, 0);
    void *buf_addr = NULL;
    if (interp_get_printf_sizeof_size(printf_info))
      buf_addr = cl_gpgpu_map_printf_buffer(gpgpu, 1);

    interp_output_printf(printf_info, index_addr, buf_addr, global_wk_sz[0],
                      global_wk_sz[1], global_wk_sz[2]);

    cl_gpgpu_unmap_printf_buffer(gpgpu, 0);
    if (interp_get_printf_sizeof_size(printf_info))
      cl_gpgpu_unmap_printf_buffer(gpgpu, 1);
  }

  if (printf_info) {
    interp_release_printf_info(printf_info);
    global_wk_sz[0] = global_wk_sz[1] = global_wk_sz[2] = 0;
    cl_gpgpu_set_printf_info(gpgpu, NULL, global_wk_sz);
  }
}

LOCAL cl_int
cl_command_queue_flush(cl_command_queue queue)
{
  GET_QUEUE_THREAD_GPGPU(queue);
  cl_command_queue_flush_gpgpu(queue, gpgpu);
  // As we don't have a deadicate timer thread to take care the possible
  // event which has a call back function registerred and the event will
  // be released at the call back function, no other function will access
  // the event any more. If we don't do this here, we will leak that event
  // and all the corresponding buffers which is really bad.
  if (queue->last_event && queue->last_event->user_cb)
    cl_event_update_status(queue->last_event, 1);
  if (queue->current_event)
    cl_event_flush(queue->current_event);
  cl_invalid_thread_gpgpu(queue);
  return CL_SUCCESS;
}

LOCAL cl_int
cl_command_queue_finish(cl_command_queue queue)
{
  cl_gpgpu_sync(cl_get_thread_batch_buf(queue));
  return CL_SUCCESS;
}

#define DEFAULT_WAIT_EVENTS_SIZE  16
LOCAL void
cl_command_queue_insert_event(cl_command_queue queue, cl_event event)
{
  cl_int i=0;
  cl_event *new_list;

  assert(queue != NULL);
  if(queue->wait_events == NULL) {
    queue->wait_events_size = DEFAULT_WAIT_EVENTS_SIZE;
    TRY_ALLOC_NO_ERR (queue->wait_events, CALLOC_ARRAY(cl_event, queue->wait_events_size));
  }

  for(i=0; i<queue->wait_events_num; i++) {
    if(queue->wait_events[i] == event)
      return;   //is in the wait_events, need to insert
  }

  if(queue->wait_events_num < queue->wait_events_size) {
    queue->wait_events[queue->wait_events_num++] = event;
    return;
  }

  //wait_events_num == wait_events_size, array is full
  queue->wait_events_size *= 2;
  TRY_ALLOC_NO_ERR (new_list, CALLOC_ARRAY(cl_event, queue->wait_events_size));
  memcpy(new_list, queue->wait_events, sizeof(cl_event)*queue->wait_events_num);
  cl_free(queue->wait_events);
  queue->wait_events = new_list;
  queue->wait_events[queue->wait_events_num++] = event;
  return;

exit:
  return;
error:
  if(queue->wait_events)
    cl_free(queue->wait_events);
  queue->wait_events = NULL;
  queue->wait_events_size = 0;
  queue->wait_events_num = 0;
  goto exit;

}

LOCAL void
cl_command_queue_remove_event(cl_command_queue queue, cl_event event)
{
  cl_int i=0;

  assert(queue->wait_events);
  for(i=0; i<queue->wait_events_num; i++) {
    if(queue->wait_events[i] == event)
      break;
  }

  if(i == queue->wait_events_num)
    return;

  if(i == queue->wait_events_num - 1) {
    queue->wait_events[i] = NULL;
  } else {
    for(; i<queue->wait_events_num-1; i++) {
      queue->wait_events[i] = queue->wait_events[i+1];
    }
  }
  queue->wait_events_num -= 1;
}

#define DEFAULT_WAIT_EVENTS_SIZE  16
LOCAL void
cl_command_queue_insert_barrier_event(cl_command_queue queue, cl_event event)
{
  cl_int i=0;
  cl_event *new_list;

  assert(queue != NULL);
  if(queue->barrier_events == NULL) {
    queue->barrier_events_size = DEFAULT_WAIT_EVENTS_SIZE;
    TRY_ALLOC_NO_ERR (queue->barrier_events, CALLOC_ARRAY(cl_event, queue->barrier_events_size));
  }

  for(i=0; i<queue->barrier_events_num; i++) {
    if(queue->barrier_events[i] == event)
      return;   //is in the barrier_events, need to insert
  }

  if(queue->barrier_events_num < queue->barrier_events_size) {
    queue->barrier_events[queue->barrier_events_num++] = event;
    return;
  }

  //barrier_events_num == barrier_events_size, array is full
  queue->barrier_events_size *= 2;
  TRY_ALLOC_NO_ERR (new_list, CALLOC_ARRAY(cl_event, queue->barrier_events_size));
  memcpy(new_list, queue->barrier_events, sizeof(cl_event)*queue->barrier_events_num);
  cl_free(queue->barrier_events);
  queue->barrier_events = new_list;
  queue->barrier_events[queue->barrier_events_num++] = event;
  return;

exit:
  return;
error:
  if(queue->barrier_events)
    cl_free(queue->barrier_events);
  queue->barrier_events = NULL;
  queue->barrier_events_size = 0;
  queue->barrier_events_num = 0;
  goto exit;

}

LOCAL void
cl_command_queue_remove_barrier_event(cl_command_queue queue, cl_event event)
{
  cl_int i=0;

  if(queue->barrier_events_num == 0)
    return;

  for(i=0; i<queue->barrier_events_num; i++) {
    if(queue->barrier_events[i] == event)
      break;
  }

  if(i == queue->barrier_events_num)
    return;

  if(i == queue->barrier_events_num - 1) {
    queue->barrier_events[i] = NULL;
  } else {
    for(; i<queue->barrier_events_num-1; i++) {
      queue->barrier_events[i] = queue->barrier_events[i+1];
    }
  }
  queue->barrier_events_num -= 1;
}
