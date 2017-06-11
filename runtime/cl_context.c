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

#include "cl_platform_id.h"
#include "cl_device_id.h"
#include "cl_context.h"
#include "cl_command_queue.h"
#include "cl_sampler.h"
#include "cl_event.h"
#include "cl_alloc.h"
#include "cl_utils.h"
#include "cl_mem.h"
#include "cl_khr_icd.h"
#include "cl_program.h"
#include "CL/cl_gl.h"
#include <string.h>

LOCAL void
cl_context_add_queue(cl_context ctx, cl_command_queue queue)
{
  assert(queue->ctx == NULL);
  cl_context_add_ref(ctx);

  CL_OBJECT_LOCK(ctx);
  while (ctx->queue_modify_disable) {
    CL_OBJECT_WAIT_ON_COND(ctx);
  }
  list_add_tail(&ctx->queues, &queue->base.node);
  ctx->queue_num++;
  CL_OBJECT_UNLOCK(ctx);

  queue->ctx = ctx;
}

LOCAL void
cl_context_remove_queue(cl_context ctx, cl_command_queue queue)
{
  assert(queue->ctx == ctx);

  CL_OBJECT_LOCK(ctx);
  while (ctx->queue_modify_disable) {
    CL_OBJECT_WAIT_ON_COND(ctx);
  }
  list_node_del(&queue->base.node);
  ctx->queue_num--;
  CL_OBJECT_UNLOCK(ctx);

  cl_context_delete(ctx);
  queue->ctx = NULL;
}

LOCAL void
cl_context_add_mem(cl_context ctx, cl_mem mem)
{
  assert(mem->ctx == NULL);
  cl_context_add_ref(ctx);

  CL_OBJECT_LOCK(ctx);
  list_add_tail(&ctx->mem_objects, &mem->base.node);
  ctx->mem_object_num++;
  CL_OBJECT_UNLOCK(ctx);

  mem->ctx = ctx;
}

LOCAL void
cl_context_remove_mem(cl_context ctx, cl_mem mem)
{
  assert(mem->ctx == ctx);
  CL_OBJECT_LOCK(ctx);
  list_node_del(&mem->base.node);
  ctx->mem_object_num--;
  CL_OBJECT_UNLOCK(ctx);

  cl_context_delete(ctx);
  mem->ctx = NULL;
}

LOCAL void
cl_context_add_sampler(cl_context ctx, cl_sampler sampler)
{
  assert(sampler->ctx == NULL);
  cl_context_add_ref(ctx);

  CL_OBJECT_LOCK(ctx);
  list_add_tail(&ctx->samplers, &sampler->base.node);
  ctx->sampler_num++;
  CL_OBJECT_UNLOCK(ctx);

  sampler->ctx = ctx;
}

LOCAL void
cl_context_remove_sampler(cl_context ctx, cl_sampler sampler)
{
  assert(sampler->ctx == ctx);
  CL_OBJECT_LOCK(ctx);
  list_node_del(&sampler->base.node);
  ctx->sampler_num--;
  CL_OBJECT_UNLOCK(ctx);

  cl_context_delete(ctx);
  sampler->ctx = NULL;
}

LOCAL void
cl_context_add_event(cl_context ctx, cl_event event)
{
  assert(event->ctx == NULL);
  cl_context_add_ref(ctx);

  CL_OBJECT_LOCK(ctx);
  list_add_tail(&ctx->events, &event->base.node);
  ctx->event_num++;
  CL_OBJECT_UNLOCK(ctx);

  event->ctx = ctx;
}

LOCAL void
cl_context_remove_event(cl_context ctx, cl_event event)
{
  assert(event->ctx == ctx);
  CL_OBJECT_LOCK(ctx);
  list_node_del(&event->base.node);
  ctx->event_num--;
  CL_OBJECT_UNLOCK(ctx);

  cl_context_delete(ctx);
  event->ctx = NULL;
}

LOCAL void
cl_context_add_program(cl_context ctx, cl_program program)
{
  assert(program->ctx == NULL);
  cl_context_add_ref(ctx);

  CL_OBJECT_LOCK(ctx);
  list_add_tail(&ctx->programs, &program->base.node);
  ctx->program_num++;
  CL_OBJECT_UNLOCK(ctx);

  program->ctx = ctx;
}

LOCAL void
cl_context_remove_program(cl_context ctx, cl_program program)
{
  assert(program->ctx == ctx);
  CL_OBJECT_LOCK(ctx);
  list_node_del(&program->base.node);
  ctx->program_num--;
  CL_OBJECT_UNLOCK(ctx);

  cl_context_delete(ctx);
  program->ctx = NULL;
}

static cl_int
cl_context_properties_process(const cl_context_properties *prop,
                              struct _cl_context_prop *cl_props, cl_uint *prop_len)
{
#define CHECK(var)              \
  if (var)                      \
    return CL_INVALID_PROPERTY; \
  else                          \
    var = 1;

  int set_cl_context_platform = 0,
      set_cl_gl_context_khr = 0,
      set_cl_egl_display_khr = 0,
      set_cl_glx_display_khr = 0,
      set_cl_wgl_hdc_khr = 0,
      set_cl_cgl_sharegroup_khr = 0;
  cl_int err = CL_SUCCESS;

  cl_props->gl_type = CL_GL_NOSHARE;
  cl_props->platform_id = 0;

  if (prop == NULL)
    goto error;

  while (*prop) {
    switch (*prop) {
    case CL_CONTEXT_PLATFORM:
      CHECK(set_cl_context_platform);
      cl_props->platform_id = *(prop + 1);
      if (UNLIKELY((cl_platform_id)cl_props->platform_id != cl_get_platform_default())) {
        err = CL_INVALID_PLATFORM;
        goto error;
      }
      break;
    case CL_GL_CONTEXT_KHR:
      CHECK(set_cl_gl_context_khr);
      cl_props->gl_context = *(prop + 1);
      break;
    case CL_EGL_DISPLAY_KHR:
      CHECK(set_cl_egl_display_khr);
      cl_props->gl_type = CL_GL_EGL_DISPLAY;
      cl_props->egl_display = *(prop + 1);
      break;
    case CL_GLX_DISPLAY_KHR:
      CHECK(set_cl_glx_display_khr);
      cl_props->gl_type = CL_GL_GLX_DISPLAY;
      cl_props->glx_display = *(prop + 1);
      break;
    case CL_WGL_HDC_KHR:
      CHECK(set_cl_wgl_hdc_khr);
      cl_props->gl_type = CL_GL_WGL_HDC;
      cl_props->wgl_hdc = *(prop + 1);
      break;
    case CL_CGL_SHAREGROUP_KHR:
      CHECK(set_cl_cgl_sharegroup_khr);
      cl_props->gl_type = CL_GL_CGL_SHAREGROUP;
      cl_props->cgl_sharegroup = *(prop + 1);
      break;
    default:
      err = CL_INVALID_PROPERTY;
      goto error;
    }
    prop += 2;
    *prop_len += 2;
  }
  (*prop_len)++;

error:
  return err;

#undef CHECK
}

static cl_context
cl_context_new(struct _cl_context_prop *props, cl_uint dev_num, cl_device_id *all_dev)
{
  cl_context ctx = NULL;

  ctx = CL_CALLOC(1, sizeof(_cl_context));
  if (ctx == NULL)
    return NULL;

  CL_OBJECT_INIT_BASE(ctx, CL_OBJECT_CONTEXT_MAGIC);
  ctx->device_num = dev_num;
  ctx->devices = CL_MALLOC(dev_num * sizeof(cl_device_id));
  if (ctx->devices == NULL) {
    CL_FREE(ctx);
    return NULL;
  }
  memcpy(ctx->devices, all_dev, dev_num * sizeof(cl_device_id));

  ctx->props = *props;
  list_init(&ctx->queues);
  list_init(&ctx->mem_objects);
  list_init(&ctx->samplers);
  list_init(&ctx->events);
  list_init(&ctx->programs);
  ctx->queue_modify_disable = 0;

  ctx->each_device = CL_CALLOC(ctx->device_num, sizeof(cl_context_for_device));
  if (ctx->each_device == NULL) {
    CL_FREE(ctx);
    return NULL;
  }

  ctx->each_device_num = ctx->device_num;
  return ctx;
}

LOCAL void
cl_context_delete(cl_context ctx)
{
  cl_int i = 0;

  if (ctx == NULL)
    return;

  /* We are not done yet */
  if (CL_OBJECT_DEC_REF(ctx) > 1)
    return;

  assert(ctx->devices);
  for (i = 0; i < ctx->each_device_num; i++) {
    if (ctx->each_device[i])
      (ctx->each_device[i]->device->api.context_delete)(ctx->each_device[i]->device, ctx);
  }
  CL_FREE(ctx->each_device);
  ctx->each_device = NULL;

  if (ctx->prop_user) {
    CL_FREE(ctx->prop_user);
    ctx->prop_user = NULL;
  }

  CL_FREE(ctx->devices);
  ctx->devices = NULL;

  CL_OBJECT_DESTROY_BASE(ctx);
  CL_FREE(ctx);
}

LOCAL cl_context
cl_context_create(const cl_context_properties *properties,
                  cl_uint num_devices,
                  const cl_device_id *devices,
                  void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                  void *user_data,
                  cl_int *errcode_ret)
{
  /* cl_platform_id platform = NULL; */
  struct _cl_context_prop props;
  cl_context ctx = NULL;
  cl_int err = CL_SUCCESS;
  cl_uint prop_len = 0;
  cl_uint dev_num = 0;
  cl_device_id *all_dev = NULL;
  cl_uint i, j;

  assert(num_devices > 0);

  /* Check that we are getting the right platform */
  if ((err = cl_context_properties_process(properties, &props, &prop_len)) != CL_SUCCESS) {
    *errcode_ret = err;
    return NULL;
  }

  all_dev = CL_CALLOC(num_devices, sizeof(cl_device_id));
  if (all_dev == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }

  /* Filter out repeated device. */
  for (i = 0; i < num_devices; i++) {
    for (j = 0; j < i; j++) {
      if (devices[j] == devices[i]) {
        break;
      }
    }

    if (j != i) { // Find some duplicated one.
      continue;
    }

    all_dev[dev_num] = devices[i];
    dev_num++;
  }

  /* We are good */
  ctx = cl_context_new(&props, dev_num, all_dev);
  CL_FREE(all_dev);
  if (ctx == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }

  if (properties != NULL && prop_len > 0) {
    ctx->prop_user = CL_MALLOC(sizeof(cl_context_properties) * prop_len);
    if (ctx->prop_user == NULL) {
      cl_context_delete(ctx);
      *errcode_ret = CL_OUT_OF_HOST_MEMORY;
      return NULL;
    }
    memcpy(ctx->prop_user, properties, sizeof(cl_context_properties) * prop_len);
  }

  ctx->prop_len = prop_len;
  /* Save the user callback and user data*/
  ctx->pfn_notify = pfn_notify;
  ctx->user_data = user_data;

  for (i = 0; i < ctx->device_num; i++) {
    err = (ctx->devices[i]->api.context_create)(ctx->devices[i], ctx);
    if (err != CL_SUCCESS) {
      *errcode_ret = err;
      cl_context_delete(ctx);
      return NULL;
    }
  }

  *errcode_ret = err;
  return ctx;
}

LOCAL void
cl_context_add_ref(cl_context ctx)
{
  assert(ctx);
  CL_OBJECT_INC_REF(ctx);
}

LOCAL cl_mem
cl_context_get_svm_by_ptr(cl_context ctx, const void *p, cl_bool no_offset)
{
  struct list_node *pos;
  cl_mem buf = NULL;

  CL_OBJECT_LOCK(ctx);
  list_for_each(pos, (&ctx->mem_objects))
  {
    buf = (cl_mem)list_entry(pos, _cl_base_object, node);
    if (!CL_OBJECT_IS_SVM(buf))
      continue;

    assert(buf->host_ptr != NULL);

    if (no_offset) {
      if (p == buf->host_ptr) {
        CL_OBJECT_UNLOCK(ctx);
        return buf;
      }
    } else {
      if (p >= buf->host_ptr && p < buf->host_ptr + buf->size) {
        CL_OBJECT_UNLOCK(ctx);
        return buf;
      }
    }
  }
  CL_OBJECT_UNLOCK(ctx);

  return NULL;
}
