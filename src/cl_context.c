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

#include "cl_platform_id.h"
#include "cl_device_id.h"
#include "cl_context.h"
#include "cl_command_queue.h"
#include "cl_mem.h"
#include "cl_sampler.h"
#include "cl_event.h"
#include "cl_alloc.h"
#include "cl_utils.h"
#include "cl_driver.h"
#include "cl_khr_icd.h"
#include "cl_kernel.h"
#include "cl_program.h"

#include "CL/cl.h"
#include "CL/cl_gl.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

LOCAL void
cl_context_add_queue(cl_context ctx, cl_command_queue queue) {
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
cl_context_remove_queue(cl_context ctx, cl_command_queue queue) {
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
cl_context_add_mem(cl_context ctx, cl_mem mem) {
  assert(mem->ctx == NULL);
  cl_context_add_ref(ctx);

  CL_OBJECT_LOCK(ctx);
  list_add_tail(&ctx->mem_objects, &mem->base.node);
  ctx->mem_object_num++;
  CL_OBJECT_UNLOCK(ctx);

  mem->ctx = ctx;
}

LOCAL void
cl_context_remove_mem(cl_context ctx, cl_mem mem) {
  assert(mem->ctx == ctx);
  CL_OBJECT_LOCK(ctx);
  list_node_del(&mem->base.node);
  ctx->mem_object_num--;
  CL_OBJECT_UNLOCK(ctx);

  cl_context_delete(ctx);
  mem->ctx = NULL;
}

LOCAL void
cl_context_add_sampler(cl_context ctx, cl_sampler sampler) {
  assert(sampler->ctx == NULL);
  cl_context_add_ref(ctx);

  CL_OBJECT_LOCK(ctx);
  list_add_tail(&ctx->samplers, &sampler->base.node);
  ctx->sampler_num++;
  CL_OBJECT_UNLOCK(ctx);

  sampler->ctx = ctx;
}

LOCAL void
cl_context_remove_sampler(cl_context ctx, cl_sampler sampler) {
  assert(sampler->ctx == ctx);
  CL_OBJECT_LOCK(ctx);
  list_node_del(&sampler->base.node);
  ctx->sampler_num--;
  CL_OBJECT_UNLOCK(ctx);

  cl_context_delete(ctx);
  sampler->ctx = NULL;
}

LOCAL void
cl_context_add_event(cl_context ctx, cl_event event) {
  assert(event->ctx == NULL);
  cl_context_add_ref(ctx);

  CL_OBJECT_LOCK(ctx);
  list_add_tail(&ctx->events, &event->base.node);
  ctx->event_num++;
  CL_OBJECT_UNLOCK(ctx);

  event->ctx = ctx;
}

LOCAL void
cl_context_remove_event(cl_context ctx, cl_event event) {
  assert(event->ctx == ctx);
  CL_OBJECT_LOCK(ctx);
  list_node_del(&event->base.node);
  ctx->event_num--;
  CL_OBJECT_UNLOCK(ctx);

  cl_context_delete(ctx);
  event->ctx = NULL;
}

LOCAL void
cl_context_add_program(cl_context ctx, cl_program program) {
  assert(program->ctx == NULL);
  cl_context_add_ref(ctx);

  CL_OBJECT_LOCK(ctx);
  list_add_tail(&ctx->programs, &program->base.node);
  ctx->program_num++;
  CL_OBJECT_UNLOCK(ctx);

  program->ctx = ctx;
}

LOCAL void
cl_context_remove_program(cl_context ctx, cl_program program) {
  assert(program->ctx == ctx);
  CL_OBJECT_LOCK(ctx);
  list_node_del(&program->base.node);
  ctx->program_num--;
  CL_OBJECT_UNLOCK(ctx);

  cl_context_delete(ctx);
  program->ctx = NULL;
}


#define CHECK(var) \
  if (var) \
    return CL_INVALID_PROPERTY; \
  else \
    var = 1;

static cl_int
cl_context_properties_process(const cl_context_properties *prop,
                              struct _cl_context_prop *cl_props, cl_uint * prop_len)
{
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
    goto exit;


  while(*prop) {
    switch (*prop) {
    case CL_CONTEXT_PLATFORM:
      CHECK (set_cl_context_platform);
      cl_props->platform_id = *(prop + 1);
      if (UNLIKELY((cl_platform_id) cl_props->platform_id != cl_get_platform_default())) {
        err = CL_INVALID_PLATFORM;
        goto error;
      }
      break;
    case CL_GL_CONTEXT_KHR:
      CHECK (set_cl_gl_context_khr);
      cl_props->gl_context = *(prop + 1);
      break;
    case CL_EGL_DISPLAY_KHR:
      CHECK (set_cl_egl_display_khr);
      cl_props->gl_type = CL_GL_EGL_DISPLAY;
      cl_props->egl_display = *(prop + 1);
      break;
    case CL_GLX_DISPLAY_KHR:
      CHECK (set_cl_glx_display_khr);
      cl_props->gl_type = CL_GL_GLX_DISPLAY;
      cl_props->glx_display = *(prop + 1);
      break;
    case CL_WGL_HDC_KHR:
      CHECK (set_cl_wgl_hdc_khr);
      cl_props->gl_type = CL_GL_WGL_HDC;
      cl_props->wgl_hdc = *(prop + 1);
      break;
    case CL_CGL_SHAREGROUP_KHR:
      CHECK (set_cl_cgl_sharegroup_khr);
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
exit:
error:
  return err;
}



LOCAL cl_context
cl_create_context(const cl_context_properties *  properties,
                  cl_uint                        num_devices,
                  const cl_device_id *           devices,
                  void (CL_CALLBACK * pfn_notify) (const char*, const void*, size_t, void*),
                  void *                         user_data,
                  cl_int *                       errcode_ret)
{
  /* cl_platform_id platform = NULL; */
  struct _cl_context_prop props;
  cl_context ctx = NULL;
  cl_int err = CL_SUCCESS;
  cl_uint prop_len = 0;
  cl_uint dev_num = 0;
  cl_device_id* all_dev = NULL;
  cl_uint i, j;

  /* XXX */
  FATAL_IF (num_devices != 1, "Only one device is supported");

  /* Check that we are getting the right platform */
  if (UNLIKELY(((err = cl_context_properties_process(properties, &props, &prop_len)) != CL_SUCCESS)))
    goto error;

  /* Filter out repeated device. */
  assert(num_devices > 0);
  all_dev = cl_calloc(num_devices, sizeof(cl_device_id));
  if (all_dev == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }
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
  assert(dev_num == 1); // TODO: multi devices later.

  /* We are good */
  if (UNLIKELY((ctx = cl_context_new(&props, dev_num, all_dev)) == NULL)) {
    cl_free(all_dev);
    err = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }

  if(properties != NULL && prop_len > 0) {
    TRY_ALLOC (ctx->prop_user, CALLOC_ARRAY(cl_context_properties, prop_len));
    memcpy(ctx->prop_user, properties, sizeof(cl_context_properties)*prop_len);
  }
  ctx->prop_len = prop_len;
  /* cl_context_new will use all_dev. */
  all_dev = NULL;

  /* Save the user callback and user data*/
  ctx->pfn_notify = pfn_notify;
  ctx->user_data = user_data;
  cl_driver_set_atomic_flag(ctx->drv, ctx->devices[0]->atomic_test_result);

exit:
  if (errcode_ret != NULL)
    *errcode_ret = err;
  return ctx;
error:
  cl_context_delete(ctx);
  ctx = NULL;
  goto exit;
}

LOCAL cl_context
cl_context_new(struct _cl_context_prop *props, cl_uint dev_num, cl_device_id* all_dev)
{
  cl_context ctx = NULL;

  TRY_ALLOC_NO_ERR (ctx, CALLOC(struct _cl_context));
  CL_OBJECT_INIT_BASE(ctx, CL_OBJECT_CONTEXT_MAGIC);
  ctx->devices = all_dev;
  ctx->device_num = dev_num;
  list_init(&ctx->queues);
  list_init(&ctx->mem_objects);
  list_init(&ctx->samplers);
  list_init(&ctx->events);
  list_init(&ctx->programs);
  ctx->queue_modify_disable = CL_FALSE;
  TRY_ALLOC_NO_ERR (ctx->drv, cl_driver_new(props));
  ctx->props = *props;
  ctx->ver = cl_driver_get_ver(ctx->drv);

exit:
  return ctx;
error:
  cl_context_delete(ctx);
  ctx = NULL;
  goto exit;
}

LOCAL void
cl_context_delete(cl_context ctx)
{
  int i = 0;
  if (UNLIKELY(ctx == NULL))
    return;

  /* We are not done yet */
  if (CL_OBJECT_DEC_REF(ctx) > 1)
    return;

  /* delete the internal programs. */
  for (i = CL_INTERNAL_KERNEL_MIN; i < CL_INTERNAL_KERNEL_MAX; i++) {
    if (ctx->internal_kernels[i]) {
      cl_kernel_delete(ctx->internal_kernels[i]);
      ctx->internal_kernels[i] = NULL;

      assert(ctx->internal_prgs[i]);
      cl_program_delete(ctx->internal_prgs[i]);
      ctx->internal_prgs[i] = NULL;
    }

    if (ctx->internal_kernels[i]) {
      cl_kernel_delete(ctx->built_in_kernels[i]);
      ctx->built_in_kernels[i] = NULL;
    }
  }

  cl_program_delete(ctx->built_in_prgs);
  ctx->built_in_prgs = NULL;

  cl_free(ctx->prop_user);
  cl_driver_delete(ctx->drv);
  CL_OBJECT_DESTROY_BASE(ctx);
  cl_free(ctx);
}

LOCAL void
cl_context_add_ref(cl_context ctx)
{
  assert(ctx);
  CL_OBJECT_INC_REF(ctx);
}

cl_buffer_mgr
cl_context_get_bufmgr(cl_context ctx)
{
  return cl_driver_get_bufmgr(ctx->drv);
}

cl_kernel
cl_context_get_static_kernel_from_bin(cl_context ctx, cl_int index,
                  const char * str_kernel, size_t size, const char * str_option)
{
  cl_int ret;
  cl_int binary_status = CL_SUCCESS;
  cl_kernel ker;

  CL_OBJECT_TAKE_OWNERSHIP(ctx, 1);
  if (ctx->internal_prgs[index] == NULL) {
    ctx->internal_prgs[index] = cl_program_create_from_binary(ctx, 1, &ctx->devices[0],
      &size, (const unsigned char **)&str_kernel, &binary_status, &ret);

    if (!ctx->internal_prgs[index]) {
      ker = NULL;
      goto unlock;
    }
    ret = cl_program_build(ctx->internal_prgs[index], str_option);
    if (ret != CL_SUCCESS) {
      ker = NULL;
      goto unlock;
    }

    ctx->internal_prgs[index]->is_built = 1;

    /* All CL_ENQUEUE_FILL_BUFFER_ALIGN16_xxx use the same program, different kernel. */
    if (index >= CL_ENQUEUE_FILL_BUFFER_ALIGN8_8 && index <= CL_ENQUEUE_FILL_BUFFER_ALIGN8_64) {
      int i = CL_ENQUEUE_FILL_BUFFER_ALIGN8_8;
      for (; i <= CL_ENQUEUE_FILL_BUFFER_ALIGN8_64; i++) {
        if (index != i) {
          assert(ctx->internal_prgs[i] == NULL);
          assert(ctx->internal_kernels[i] == NULL);
          cl_program_add_ref(ctx->internal_prgs[index]);
          ctx->internal_prgs[i] = ctx->internal_prgs[index];
        }

        if (i == CL_ENQUEUE_FILL_BUFFER_ALIGN8_8) {
          ctx->internal_kernels[i] = cl_program_create_kernel(ctx->internal_prgs[index],
                                                              "__cl_fill_region_align8_2", NULL);
        } else if (i == CL_ENQUEUE_FILL_BUFFER_ALIGN8_16) {
          ctx->internal_kernels[i] = cl_program_create_kernel(ctx->internal_prgs[index],
                                                              "__cl_fill_region_align8_4", NULL);
        } else if (i == CL_ENQUEUE_FILL_BUFFER_ALIGN8_32) {
          ctx->internal_kernels[i] = cl_program_create_kernel(ctx->internal_prgs[index],
                                                              "__cl_fill_region_align8_8", NULL);
        } else if (i == CL_ENQUEUE_FILL_BUFFER_ALIGN8_64) {
          ctx->internal_kernels[i] = cl_program_create_kernel(ctx->internal_prgs[index],
                                                              "__cl_fill_region_align8_16", NULL);
        } else
          assert(0);
      }
    } else {
      ctx->internal_kernels[index] = cl_kernel_dup(ctx->internal_prgs[index]->ker[0]);
    }
  }
  ker = ctx->internal_kernels[index];

unlock:
  CL_OBJECT_RELEASE_OWNERSHIP(ctx);
  return cl_kernel_dup(ker);
}


cl_mem
cl_context_get_svm_from_ptr(cl_context ctx, const void * p)
{
  struct list_node *pos;
  cl_mem buf;

  list_for_each (pos, (&ctx->mem_objects)) {
    buf = (cl_mem)list_entry(pos, _cl_base_object, node);
    if(buf->host_ptr == NULL) continue;
    if(buf->is_svm == 0) continue;
    if(buf->type != CL_MEM_SVM_TYPE) continue;
    if((size_t)buf->host_ptr <= (size_t)p &&
       (size_t)p < ((size_t)buf->host_ptr + buf->size))
      return buf;
  }
  return NULL;
}

cl_mem
cl_context_get_mem_from_ptr(cl_context ctx, const void * p)
{
  struct list_node *pos;
  cl_mem buf;

  list_for_each (pos, (&ctx->mem_objects)) {
    buf = (cl_mem)list_entry(pos, _cl_base_object, node);
    if(buf->host_ptr == NULL) continue;
    if((size_t)buf->host_ptr <= (size_t)p &&
       (size_t)p < ((size_t)buf->host_ptr + buf->size))
      return buf;
  }
  return NULL;
}
