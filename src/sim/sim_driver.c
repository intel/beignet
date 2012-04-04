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

#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_device_data.h"
#include "sim/sim_driver.h"
#include "CL/cl.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "cl_driver.h"

/* Fake buffer manager that just counts allocations */
struct _sim_bufmgr { volatile int buf_n; };
typedef struct _sim_bufmgr *sim_bufmgr;

static sim_bufmgr
sim_bufmgr_new(void)
{
  return cl_calloc(1,sizeof(struct _sim_bufmgr));
}

static void
sim_bufmgr_delete(sim_bufmgr bufmgr)
{
  cl_free(bufmgr);
}

/* Fake low-level driver */
struct _sim_driver {
  sim_bufmgr bufmgr;
  int gen_ver;
};

typedef struct _sim_driver *sim_driver;

static void
sim_driver_delete(sim_driver driver)
{
  if (driver == NULL) return;
  sim_bufmgr_delete(driver->bufmgr);
  cl_free(driver);
}

static sim_driver
sim_driver_new(void)
{
  sim_driver driver = NULL;
  TRY_ALLOC_NO_ERR(driver, cl_calloc(1, sizeof(struct _sim_driver)));
  TRY_ALLOC_NO_ERR(driver->bufmgr, sim_bufmgr_new());
  driver->gen_ver = 7; // XXX make it flexible
exit:
  return driver;
error:
  sim_driver_delete(driver);
  driver = NULL;
  goto exit;
}

static int
sim_driver_get_ver(sim_driver driver)
{
  return driver->gen_ver;
}

static sim_bufmgr
sim_driver_get_bufmgr(sim_driver driver)
{
  return driver->bufmgr;
}

static int
sim_driver_get_device_id(void)
{
  return PCI_CHIP_IVYBRIDGE_GT2; // XXX get some env variable instead
}

/* Just a named buffer to mirror real drm functions */
struct _sim_buffer {
  void *data;         /* data in the buffer */
  size_t sz;          /* size allocated */
  volatile int ref_n; /* number of references */
  char *name;         /* name of the buffer */
  sim_bufmgr bufmgr;  /* owns the buffer */
};
typedef struct _sim_buffer *sim_buffer;

static void
sim_buffer_delete(sim_buffer buf)
{
  if (buf == NULL) return;
  cl_free(buf->data);
  cl_free(buf->name);
  cl_free(buf);
}

static sim_buffer
sim_buffer_alloc(sim_bufmgr bufmgr, const char *name, unsigned long sz, unsigned long align)
{
  sim_buffer buf = NULL;
  assert(bufmgr);
  TRY_ALLOC_NO_ERR(buf, cl_calloc(1, sizeof(struct _sim_buffer)));
  if (sz) buf->data = cl_aligned_malloc(sz, align);
  if (name) {
    const size_t len = strlen(name);
    TRY_ALLOC_NO_ERR(buf->name, cl_malloc(len+1));
    memcpy(buf->name, name, len);
    buf->name[len] = 0;
  }
  buf->ref_n = 1;
  buf->bufmgr = bufmgr;
  buf->sz = sz;
  atomic_inc(&buf->bufmgr->buf_n);

exit:
  return buf;
error:
  sim_buffer_delete(buf);
  buf = NULL;
  goto exit;
}

static void
sim_buffer_unreference(sim_buffer buf)
{
  if (UNLIKELY(buf == NULL)) return;
  if (atomic_dec(&buf->ref_n) > 1) return;
  atomic_dec(&buf->bufmgr->buf_n);
  sim_buffer_delete(buf);
}

static void
sim_buffer_reference(sim_buffer buf)
{
  if (UNLIKELY(buf == NULL)) return;
  atomic_inc(&buf->ref_n);
}

static void*
sim_buffer_get_virtual(sim_buffer buf)
{
  if (UNLIKELY(buf == NULL)) return NULL;
  return buf->data;
}

static void*
sim_buffer_get_size(sim_buffer buf)
{
  if (UNLIKELY(buf == NULL)) return 0;
  return buf->data;
}

static int
sim_buffer_subdata(sim_buffer buf, unsigned long offset, unsigned long size, const void *data)
{
  if (data == NULL) return 0;
  if (buf == NULL) return 0;
  memcpy((char*) buf->data + offset, data, size);
  return 0;
}
static int sim_buffer_map(sim_buffer buf, uint32_t write_enable) {return 0;}
static int sim_buffer_unmap(sim_buffer buf) {return 0;}
static int sim_buffer_pin(sim_buffer buf, uint32_t alignment) {return 0;}
static int sim_buffer_unpin(sim_buffer buf) {return 0;}
static int sim_buffer_wait_rendering(sim_buffer buf) {return 0;}

/* Function to call for each HW thread we simulate */
typedef void (sim_kernel_cb)(void);

/* Encapsulates operations needed to run one NDrange */
struct _sim_gpgpu
{
  sim_driver driver;     /* the driver the gpgpu states belongs to */
  sim_kernel_cb *kernel; /* call it for each HW thread */
  uint32_t max_threads;  /* HW threads running */
  uint32_t cst_sz;       /* size of the constant buffer */
};
typedef struct _sim_gpgpu *sim_gpgpu;

static void sim_gpgpu_delete(sim_gpgpu gpgpu)
{
  cl_free(gpgpu);
}

static sim_gpgpu sim_gpgpu_new(sim_driver driver)
{
  sim_gpgpu gpgpu = NULL;
  TRY_ALLOC_NO_ERR(gpgpu, cl_calloc(1, sizeof(struct _sim_gpgpu)));

exit:
  return gpgpu;
error:
  sim_gpgpu_delete(gpgpu);
  gpgpu = NULL;
  goto exit;
}

#undef NOT_IMPLEMENTED
#define NOT_IMPLEMENTED

static void sim_gpgpu_bind_buf(sim_gpgpu gpgpu, int32_t index, sim_buffer buf, uint32_t cchint)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_bind_image2D(sim_gpgpu gpgpu,
                                   int32_t index,
                                   sim_buffer obj_bo,
                                   uint32_t format,
                                   int32_t w,
                                   int32_t h,
                                   int pitch,
                                   cl_gpgpu_tiling tiling)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_state_init(sim_gpgpu gpgpu, uint32_t max_threads, uint32_t size_cs_entry)
{
  assert(gpgpu);
  memset(gpgpu, 0, sizeof(*gpgpu));
  gpgpu->cst_sz = size_cs_entry * 32;
  gpgpu->max_threads = max_threads;
}

static void sim_gpgpu_set_perf_counters(sim_gpgpu gpgpu, sim_buffer perf)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_upload_constants(sim_gpgpu gpgpu, const void* data, uint32_t size)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_states_setup(sim_gpgpu gpgpu, cl_gpgpu_kernel *kernel)
{
  cl_buffer_map(kernel->bo, 0);
  gpgpu->kernel = *(sim_kernel_cb **) cl_buffer_get_virtual(kernel->bo);
}

static void sim_gpgpu_upload_samplers(sim_gpgpu gpgpu, const void *data, uint32_t n)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_batch_reset(sim_gpgpu gpgpu, size_t sz)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_batch_start(sim_gpgpu gpgpu)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_batch_end(sim_gpgpu gpgpu, int32_t flush_mode)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_flush(sim_gpgpu gpgpu)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_walker(sim_gpgpu gpgpu,
                             uint32_t simd_sz,
                             uint32_t thread_n,
                             const size_t global_wk_off[3],
                             const size_t global_wk_sz[3],
                             const size_t local_wk_sz[3])
{
  uint32_t x, y, z;
  const uint32_t global_wk_dim[3] = {
    global_wk_sz[0] / local_wk_sz[0],
    global_wk_sz[1] / local_wk_sz[1],
    global_wk_sz[2] / local_wk_sz[2]
  };
  assert(simd_sz == 8 || simd_sz == 16);
  for (z = 0; z < global_wk_dim[2]; ++z)
  for (y = 0; y < global_wk_dim[1]; ++y)
  for (x = 0; x < global_wk_dim[0]; ++x)
    gpgpu->kernel();
}

LOCAL void
sim_setup_callbacks(void)
{
  cl_driver_new = (cl_driver_new_cb *) sim_driver_new;
  cl_driver_delete = (cl_driver_delete_cb *) sim_driver_delete;
  cl_driver_get_ver = (cl_driver_get_ver_cb *) sim_driver_get_ver;
  cl_driver_get_bufmgr = (cl_driver_get_bufmgr_cb *) sim_driver_get_bufmgr;
  cl_driver_get_device_id = (cl_driver_get_device_id_cb *) sim_driver_get_device_id;
  cl_buffer_alloc = (cl_buffer_alloc_cb *) sim_buffer_alloc;
  cl_buffer_reference = (cl_buffer_reference_cb *) sim_buffer_reference;
  cl_buffer_unreference = (cl_buffer_unreference_cb *) sim_buffer_unreference;
  cl_buffer_map = (cl_buffer_map_cb *) sim_buffer_map;
  cl_buffer_unmap = (cl_buffer_unmap_cb *) sim_buffer_unmap;
  cl_buffer_get_virtual = (cl_buffer_get_virtual_cb *) sim_buffer_get_virtual;
  cl_buffer_get_size = (cl_buffer_get_size_cb *) sim_buffer_get_size;
  cl_buffer_pin = (cl_buffer_pin_cb *) sim_buffer_pin;
  cl_buffer_unpin = (cl_buffer_unpin_cb *) sim_buffer_unpin;
  cl_buffer_subdata = (cl_buffer_subdata_cb *) sim_buffer_subdata;
  cl_buffer_wait_rendering = (cl_buffer_wait_rendering_cb *) sim_buffer_wait_rendering;
  cl_gpgpu_new = (cl_gpgpu_new_cb *) sim_gpgpu_new;
  cl_gpgpu_delete = (cl_gpgpu_delete_cb *) sim_gpgpu_delete;
  cl_gpgpu_bind_image2D = (cl_gpgpu_bind_image2D_cb *) sim_gpgpu_bind_image2D;
  cl_gpgpu_bind_buf = (cl_gpgpu_bind_buf_cb *) sim_gpgpu_bind_buf;
  cl_gpgpu_state_init = (cl_gpgpu_state_init_cb *) sim_gpgpu_state_init;
  cl_gpgpu_set_perf_counters = (cl_gpgpu_set_perf_counters_cb *) sim_gpgpu_set_perf_counters;
  cl_gpgpu_upload_constants = (cl_gpgpu_upload_constants_cb *) sim_gpgpu_upload_constants;
  cl_gpgpu_states_setup = (cl_gpgpu_states_setup_cb *) sim_gpgpu_states_setup;
  cl_gpgpu_upload_samplers = (cl_gpgpu_upload_samplers_cb *) sim_gpgpu_upload_samplers;
  cl_gpgpu_batch_reset = (cl_gpgpu_batch_reset_cb *) sim_gpgpu_batch_reset;
  cl_gpgpu_batch_start = (cl_gpgpu_batch_start_cb *) sim_gpgpu_batch_start;
  cl_gpgpu_batch_end = (cl_gpgpu_batch_end_cb *) sim_gpgpu_batch_end;
  cl_gpgpu_flush = (cl_gpgpu_flush_cb *) sim_gpgpu_flush;
  cl_gpgpu_walker = (cl_gpgpu_walker_cb *) sim_gpgpu_walker;
}

