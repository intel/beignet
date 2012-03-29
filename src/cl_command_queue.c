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
#include "intel_bufmgr.h"
#include "intel/intel_gpgpu.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

LOCAL cl_command_queue
cl_command_queue_new(cl_context ctx)
{
  cl_command_queue queue = NULL;

  assert(ctx);
  TRY_ALLOC_NO_ERR (queue, CALLOC(struct _cl_command_queue));
  queue->magic = CL_MAGIC_QUEUE_HEADER;
  queue->ref_n = 1;
  queue->ctx = ctx;
  TRY_ALLOC_NO_ERR (queue->gpgpu,
                    intel_gpgpu_new((struct intel_driver*) ctx->intel_drv));

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
  if (atomic_dec(&queue->ref_n) != 1)
    return;

  /* Remove it from the list */
  assert(queue->ctx);
  pthread_mutex_lock(&queue->ctx->queue_lock);
    if (queue->prev)
      queue->prev->next = queue->next;
    if (queue->next)
      queue->next->prev = queue->prev;
    if (queue->next == NULL && queue->prev == NULL)
      queue->ctx->queues = NULL;
  pthread_mutex_unlock(&queue->ctx->queue_lock);
  if (queue->fulsim_out != NULL) {
    cl_mem_delete(queue->fulsim_out);
    queue->fulsim_out = NULL;
  }
  cl_mem_delete(queue->perf);
  cl_context_delete(queue->ctx);
  intel_gpgpu_delete(queue->gpgpu);
  queue->magic = CL_MAGIC_DEAD_HEADER; /* For safety */
  cl_free(queue);
}

LOCAL void
cl_command_queue_add_ref(cl_command_queue queue)
{
  atomic_inc(&queue->ref_n);
}

  LOCAL cl_int
cl_command_queue_bind_surface(cl_command_queue queue,
                              cl_kernel k,
                              char *curbe,
                              drm_intel_bo **local, 
                              drm_intel_bo **priv,
                              drm_intel_bo **scratch,
                              uint32_t local_sz)
{
  cl_context ctx = queue->ctx;
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  drm_intel_bufmgr *bufmgr = cl_context_get_intel_bufmgr(ctx);
  drm_intel_bo *sync_bo = NULL;
  cl_int err = CL_SUCCESS;
#if 0
  cl_context ctx = queue->ctx;
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  drm_intel_bufmgr *bufmgr = cl_context_get_intel_bufmgr(ctx);
  cl_mem mem = NULL;
  drm_intel_bo *bo = NULL, *sync_bo = NULL;
  const size_t max_thread = ctx->device->max_compute_unit;
  cl_int err = CL_SUCCESS;
  uint32_t i, index;

  /* Bind user defined surface */
  for (i = 0; i < k->arg_info_n; ++i) {
    assert(k->arg_info[i].offset % SURFACE_SZ == 0);
    index = k->arg_info[i].offset / SURFACE_SZ;
    mem = (cl_mem) k->args[k->arg_info[i].arg_index];
    assert(index != MAX_SURFACES - 1);
    CHECK_MEM(mem);
    bo = mem->bo;
    assert(bo);
    if (mem->is_image) {
      const int32_t w = mem->w, h = mem->h, pitch = mem->pitch;
      const uint32_t fmt = mem->intel_fmt;
      gpgpu_tiling_t tiling = GPGPU_NO_TILE;
      if (mem->tiling == CL_TILE_X)
        tiling = GPGPU_TILE_X;
      else if (mem->tiling == CL_TILE_Y)
        tiling = GPGPU_TILE_Y;
      gpgpu_bind_image2D(gpgpu, index, bo, fmt, w, h, pitch, tiling);

      /* Copy the image parameters (width, height) in the constant buffer if the
       * user requests them
       */
      cl_kernel_copy_image_parameters(k, mem, index, curbe);
    } else
      gpgpu_bind_buf(gpgpu, index, bo, cc_llc_l3);
  }

  /* Allocate the constant surface (if any) */
  if (k->const_bo) {
    assert(k->const_bo_index != MAX_SURFACES - 1);
    gpgpu_bind_buf(gpgpu, k->const_bo_index,
                   k->const_bo,
                   cc_llc_l3);
  }

  /* Allocate local surface needed for SLM and bind it */
  if (local && local_sz != 0) {
    const size_t sz = 16 * local_sz; /* XXX 16 == maximum barrier number */
    assert(k->patch.local_surf.offset % SURFACE_SZ == 0);
    index = k->patch.local_surf.offset / SURFACE_SZ;
    assert(index != MAX_SURFACES - 1);
    *local = drm_intel_bo_alloc(bufmgr, "CL local surface", sz, 64);
    gpgpu_bind_buf(gpgpu, index, *local, cc_llc_l3);
  }
  else if (local)
    *local = NULL;

  /* Allocate private surface and bind it */
  if (priv && k->patch.private_surf.size != 0) {
    const size_t sz = max_thread *
                      k->patch.private_surf.size *
                      k->patch.exec_env.largest_compiled_simd_sz;
    // assert(k->patch.exec_env.largest_compiled_simd_sz == 16);
    assert(k->patch.private_surf.offset % SURFACE_SZ == 0);
    index = k->patch.private_surf.offset / SURFACE_SZ;
    assert(index != MAX_SURFACES - 1);
    *priv = drm_intel_bo_alloc(bufmgr, "CL private surface", sz, 64);
    gpgpu_bind_buf(gpgpu, index, *priv, cc_llc_l3);
  }
  else if(priv)
    *priv = NULL;

  /* Allocate scratch surface and bind it */
  if (scratch && k->patch.scratch.size != 0) {
    const size_t sz = max_thread * /* XXX is it given per lane ??? */
                      k->patch.scratch.size *
                      k->patch.exec_env.largest_compiled_simd_sz;
    // assert(k->patch.exec_env.largest_compiled_simd_sz == 16);
    assert(k->patch.scratch.offset % SURFACE_SZ == 0);
    assert(index != MAX_SURFACES - 1);
    index = k->patch.scratch.offset / SURFACE_SZ;
    *scratch = drm_intel_bo_alloc(bufmgr, "CL scratch surface", sz, 64);
    gpgpu_bind_buf(gpgpu, index, *scratch, cc_llc_l3);
  }
  else if (scratch)
    *scratch = NULL;
#endif
  /* Now bind a bo used for synchronization */
  sync_bo = drm_intel_bo_alloc(bufmgr, "sync surface", 64, 64);
  gpgpu_bind_buf(gpgpu, MAX_SURFACES-1, sync_bo, cc_llc_l3);
  if (queue->last_batch != NULL)
    drm_intel_bo_unreference(queue->last_batch);
  queue->last_batch = sync_bo;

// error:
  assert(err == CL_SUCCESS); /* Cannot fail here */
  return err;
}

#if USE_FULSIM
extern void drm_intel_bufmgr_gem_stop_aubfile(drm_intel_bufmgr*);
extern void drm_intel_bufmgr_gem_set_aubfile(drm_intel_bufmgr*, FILE*);
extern void aub_exec_dump_raw_file(drm_intel_bo*, size_t offset, size_t sz);

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
#if 0
  cl_mem mem = NULL;
  int i;
  size_t j;

  /* Bind user defined surface */
  for (i = 0; i < k->arg_info_n; ++i) {
    size_t chunk_n, chunk_remainder;
    if (k->arg_info[i].type != OCLRT_ARG_TYPE_BUFFER)
      continue;
    mem = (cl_mem) k->args[k->arg_info[i].arg_index];
    CHECK_MEM(mem);
    chunk_n = mem->bo->size / chunk_sz;
    chunk_remainder = mem->bo->size % chunk_sz;
    for (j = 0; j < chunk_n; ++j)
      aub_exec_dump_raw_file(mem->bo, j * chunk_sz, chunk_sz);
    if (chunk_remainder)
      aub_exec_dump_raw_file(mem->bo, chunk_n * chunk_sz, chunk_remainder);
  }
error:
#endif
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

#if 0
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

#if 0
  /* Dump stuff out */
  printf("   filesize = %d\n", hdr.filesize);	/* total file size incl header */
  printf("        as0 = %d\n", hdr.as0);
  printf("        as1 = %d\n", hdr.as1);
  printf("  bmpoffset = %d\n", hdr.bmpoffset);	/* ofset of bmp data  */
  printf("headerbytes = %d\n", hdr.headerbytes);	/* bytes in header from this point (40 actually) */
  printf("      width = %d\n", hdr.width);
  printf("     height = %d\n", hdr.height);
  printf("    nplanes = %d\n", hdr.nplanes);	/* no of color planes */
  printf("        bpp = %d\n", hdr.bpp);	/* bits/pixel */
  printf("compression = %d\n", hdr.compression);	/* BI_RGB = 0 = no compression */
  printf("    sizeraw = %d\n", hdr.sizeraw);	/* size of raw bmp file, excluding header, incl padding */
  printf("       hres = %d\n", hdr.hres);	/* horz resolutions pixels/meter */
  printf("       vres = %d\n", hdr.vres);
  printf(" npalcolors = %d\n", hdr.npalcolors);	/* No of colors in palette */
  printf(" nimportant = %d\n", hdr.nimportant);	/* No of important colors */
#endif
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
#endif

static cl_int
cl_fulsim_read_all_surfaces(cl_command_queue queue, cl_kernel k)
{
  cl_int err = CL_SUCCESS;
#if 0
  cl_mem mem = NULL;
  char *from = NULL, *to = NULL;
  size_t size, j, chunk_n, chunk_remainder;
  int i, curr = 0;
  /* Bind user defined surface */
  for (i = 0; i < k->arg_info_n; ++i) {
    if (k->arg_info[i].type != OCLRT_ARG_TYPE_BUFFER)
      continue;
    mem = (cl_mem) k->args[k->arg_info[i].arg_index];
    CHECK_MEM(mem);
    assert(mem->bo);
    chunk_n = mem->bo->size / chunk_sz;
    chunk_remainder = mem->bo->size % chunk_sz;
    to = cl_mem_map(mem);
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
#endif
  return err;

}
#endif /* USE_FULSIM */

extern cl_int cl_command_queue_ND_range_gen7(cl_command_queue, cl_kernel, const size_t *, const size_t *, const size_t *);

LOCAL cl_int
cl_command_queue_ND_range(cl_command_queue queue,
                          cl_kernel k,
                          const size_t *global_wk_off,
                          const size_t *global_wk_sz,
                          const size_t *local_wk_sz)
{
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  const int32_t ver = intel_gpgpu_version(gpgpu);
  cl_int err = CL_SUCCESS;

#if USE_FULSIM
  drm_intel_bufmgr *bufmgr = NULL;
  FILE *file = fopen("dump.aub", "wb");
  FATAL_IF (file == NULL, "Unable to open file dump.aub");
  bufmgr = cl_context_get_intel_bufmgr(queue->ctx);
  drm_intel_bufmgr_gem_set_aubfile(bufmgr, file);
#endif /* USE_FULSIM */

  if (ver == 7 || ver == 75)
    TRY (cl_command_queue_ND_range_gen7, queue, k, global_wk_off, global_wk_sz, local_wk_sz);
  else
    FATAL ("Unknown Gen Device");

#if USE_FULSIM
  TRY (cl_fulsim_dump_all_surfaces, queue, k);
  drm_intel_bufmgr_gem_stop_aubfile(bufmgr);
  fclose(file);
  cl_run_fulsim();
  TRY (cl_fulsim_read_all_surfaces, queue, k);
#endif /* USE_FULSIM */

error:
  return err;
}

LOCAL cl_int
cl_command_queue_finish(cl_command_queue queue)
{
  if (queue->last_batch == NULL)
    return CL_SUCCESS;
  drm_intel_bo_wait_rendering(queue->last_batch);
  drm_intel_bo_unreference(queue->last_batch);
  queue->last_batch = NULL;
  return CL_SUCCESS;
}

