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
#include "sim/sim_buffer.h"
#include "CL/cl.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "cl_buffer.h"

/* Just to count allocations */
typedef struct sim_bufmgr { volatile int buf_n; } sim_bufmgr_t;

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

static void*
sim_buffer_map(sim_buffer_t *buf)
{
  assert(buf);
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

static int
sim_buffer_emit_reloc(sim_buffer_t *buf, 
                      uint32_t offset,
                      sim_buffer_t *target_buf,
                      uint32_t target_offset,
                      uint32_t read_domains,
                      uint32_t write_domain)
{
  return 1;
}
static int sim_buffer_unmap(sim_buffer_t *buf) {return 0;}
static int sim_buffer_pin(sim_buffer_t *buf) {return 0;}
static int sim_buffer_unpin(sim_buffer_t *buf) {return 0;}

LOCAL void
sim_setup_callbacks(void)
{
  cl_buffer_alloc = (cl_buffer_alloc_cb *) sim_buffer_alloc;
  cl_buffer_unreference = (cl_buffer_unreference_cb *) sim_buffer_unreference;
  cl_buffer_map = (cl_buffer_map_cb *) sim_buffer_map;
  cl_buffer_unmap = (cl_buffer_unmap_cb *) sim_buffer_unmap;
  cl_buffer_pin = (cl_buffer_pin_cb *) sim_buffer_pin;
  cl_buffer_unpin = (cl_buffer_unpin_cb *) sim_buffer_unpin;
  cl_buffer_subdata = (cl_buffer_subdata_cb *) sim_buffer_subdata;
  cl_buffer_emit_reloc = (cl_buffer_emit_reloc_cb *) sim_buffer_emit_reloc;
}

