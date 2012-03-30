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
typedef struct sim_bufmgr { volatile int buf_n; } sim_bufmgr_t;

static sim_bufmgr_t*
sim_bufmgr_new(void)
{
  return cl_calloc(1,sizeof(sim_bufmgr_t));
}

static void
sim_bufmgr_delete(sim_bufmgr_t *bufmgr)
{
  cl_free(bufmgr);
}

/* Fake low-driver */
typedef struct sim_driver {
  sim_bufmgr_t *bufmgr;
  int gen_ver;
} sim_driver_t;

static void
sim_driver_delete(sim_driver_t *driver)
{
  if (driver == NULL) return;
  sim_bufmgr_delete(driver->bufmgr);
  cl_free(driver);
}

static sim_driver_t*
sim_driver_new(void)
{
  sim_driver_t *driver = NULL;
  TRY_ALLOC_NO_ERR(driver, cl_calloc(1, sizeof(sim_driver_t)));
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
sim_driver_get_ver(sim_driver_t *driver)
{
  return driver->gen_ver;
}

static sim_bufmgr_t*
sim_driver_get_bufmgr(sim_driver_t *driver)
{
  return driver->bufmgr;
}

static int
sim_driver_get_device_id(void)
{
  return PCI_CHIP_IVYBRIDGE_GT2; // XXX get some env variable instead
}


/* Just a named buffer to mirror real drm functions */
typedef struct sim_buffer {
  void *data;           /* data in the buffer */
  size_t sz;            /* size allocated */
  volatile int ref_n;   /* number of references */
  char *name;           /* name of the buffer */
  sim_bufmgr_t *bufmgr; /* owns the buffer */
} sim_buffer_t;

static void
sim_buffer_delete(sim_buffer_t *buf)
{
  if (buf == NULL) return;
  cl_free(buf->data);
  cl_free(buf->name);
  cl_free(buf);
}

static sim_buffer_t*
sim_buffer_alloc(sim_bufmgr_t *bufmgr, const char *name, unsigned long sz, unsigned long align)
{
  sim_buffer_t *buf = NULL;
  assert(bufmgr);
  TRY_ALLOC_NO_ERR(buf, cl_calloc(1, sizeof(sim_buffer_t)));
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
sim_buffer_unreference(sim_buffer_t *buf)
{
  if (UNLIKELY(buf == NULL)) return;
  if (atomic_dec(&buf->ref_n) > 1) return;
  atomic_dec(&buf->bufmgr->buf_n);
  sim_buffer_delete(buf);
}

static void
sim_buffer_reference(sim_buffer_t *buf)
{
  if (UNLIKELY(buf == NULL)) return;
  atomic_inc(&buf->ref_n);
}

static void*
sim_buffer_get_virtual(sim_buffer_t *buf)
{
  if (UNLIKELY(buf == NULL)) return NULL;
  return buf->data;
}

static void*
sim_buffer_get_size(sim_buffer_t *buf)
{
  if (UNLIKELY(buf == NULL)) return 0;
  return buf->data;
}

static int
sim_buffer_subdata(sim_buffer_t *buf, unsigned long offset, unsigned long size, const void *data)
{
  if (data == NULL) return 0;
  if (buf == NULL) return 0;
  memcpy((char*) buf->data + offset, data, size);
  return 0;
}
static int sim_buffer_map(sim_buffer_t *buf, uint32_t write_enable) {return 0;}
static int sim_buffer_unmap(sim_buffer_t *buf) {return 0;}
static int sim_buffer_pin(sim_buffer_t *buf, uint32_t alignment) {return 0;}
static int sim_buffer_unpin(sim_buffer_t *buf) {return 0;}
static int sim_buffer_wait_rendering(sim_buffer_t *buf) {return 0;}

/* Encapsulates operations needed to run one NDrange */
typedef struct sim_gpgpu
{
  sim_driver_t *driver; // the driver the gpgpu states belongs to
} sim_gpgpu_t;

static void sim_gpgpu_delete(sim_gpgpu_t *gpgpu)
{
  cl_free(gpgpu);
}

static sim_gpgpu_t *sim_gpgpu_new(sim_driver_t *driver)
{
  sim_gpgpu_t *gpgpu = NULL;
  TRY_ALLOC_NO_ERR(gpgpu, cl_calloc(1, sizeof(sim_gpgpu_t)));

exit:
  return gpgpu;
error:
  sim_gpgpu_delete(gpgpu);
  gpgpu = NULL;
  goto exit;
}

#undef NOT_IMPLEMENTED
#define NOT_IMPLEMENTED

static void sim_gpgpu_bind_buf(sim_gpgpu_t *gpgpu, int32_t index, sim_buffer_t *buf, uint32_t cchint)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_bind_image2D(sim_gpgpu_t *gpgpu,
                            int32_t index,
                            sim_buffer_t *obj_bo,
                            uint32_t format,
                            int32_t w,
                            int32_t h,
                            int pitch,
                            cl_gpgpu_tiling tiling)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_state_init(sim_gpgpu_t *gpgpu, uint32_t max_threads, uint32_t size_cs_entry)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_set_perf_counters(sim_gpgpu_t *gpgpu, sim_buffer_t *perf)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_upload_constants(sim_gpgpu_t *gpgpu, const void* data, uint32_t size)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_states_setup(sim_gpgpu_t *gpgpu, cl_gpgpu_kernel* kernel, uint32_t ker_n)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_upload_samplers(sim_gpgpu_t *state, const void *data, uint32_t n)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_batch_reset(sim_gpgpu_t *state, size_t sz)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_batch_start(sim_gpgpu_t *state)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_batch_end(sim_gpgpu_t *state, int32_t flush_mode)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_flush(sim_gpgpu_t *state)
{ NOT_IMPLEMENTED; }
static void sim_gpgpu_walker(sim_gpgpu_t *state,
                             uint32_t simd_sz,
                             uint32_t thread_n,
                             const size_t global_wk_off[3],
                             const size_t global_wk_sz[3],
                             const size_t local_wk_sz[3])
{ NOT_IMPLEMENTED; }

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

