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
      if (UNLIKELY((cl_platform_id) cl_props->platform_id != intel_platform)) {
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
  /* XXX */
  FATAL_IF (num_devices != 1, "Only one device is supported");

  /* Check that we are getting the right platform */
  if (UNLIKELY(((err = cl_context_properties_process(properties, &props, &prop_len)) != CL_SUCCESS)))
    goto error;

  /* We are good */
  if (UNLIKELY((ctx = cl_context_new(&props)) == NULL)) {
    err = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }

  if(properties != NULL && prop_len > 0) {
    TRY_ALLOC (ctx->prop_user, CALLOC_ARRAY(cl_context_properties, prop_len));
    memcpy(ctx->prop_user, properties, sizeof(cl_context_properties)*prop_len);
  }
  ctx->prop_len = prop_len;
  /* Attach the device to the context */
  ctx->device = *devices;

  /* Save the user callback and user data*/
  ctx->pfn_notify = pfn_notify;
  ctx->user_data = user_data;

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
cl_context_new(struct _cl_context_prop *props)
{
  cl_context ctx = NULL;

  TRY_ALLOC_NO_ERR (ctx, CALLOC(struct _cl_context));
  TRY_ALLOC_NO_ERR (ctx->drv, cl_driver_new(props));
  SET_ICD(ctx->dispatch)
  ctx->props = *props;
  ctx->magic = CL_MAGIC_CONTEXT_HEADER;
  ctx->ref_n = 1;
  ctx->ver = cl_driver_get_ver(ctx->drv);
  pthread_mutex_init(&ctx->program_lock, NULL);
  pthread_mutex_init(&ctx->queue_lock, NULL);
  pthread_mutex_init(&ctx->buffer_lock, NULL);
  pthread_mutex_init(&ctx->sampler_lock, NULL);

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
  if (atomic_dec(&ctx->ref_n) > 1)
    return;

  /* delete the internal programs. */
  for (i = CL_INTERNAL_KERNEL_MIN; i < CL_INTERNAL_KERNEL_MAX; i++) {
    if (ctx->internel_kernels[i]) {
      cl_kernel_delete(ctx->internel_kernels[i]);
      ctx->internel_kernels[i] = NULL;

      assert(ctx->internal_prgs[i]);
      cl_program_delete(ctx->internal_prgs[i]);
      ctx->internal_prgs[i] = NULL;
    }

    if (ctx->internel_kernels[i]) {
      cl_kernel_delete(ctx->built_in_kernels[i]);
      ctx->built_in_kernels[i] = NULL;
    }
  }

  cl_program_delete(ctx->built_in_prgs);
  ctx->built_in_prgs = NULL;

  /* All object lists should have been freed. Otherwise, the reference counter
   * of the context cannot be 0
   */
  assert(ctx->queues == NULL);
  assert(ctx->programs == NULL);
  assert(ctx->buffers == NULL);
  assert(ctx->drv);
  cl_free(ctx->prop_user);
  cl_driver_delete(ctx->drv);
  ctx->magic = CL_MAGIC_DEAD_HEADER; /* For safety */
  cl_free(ctx);
}

LOCAL void
cl_context_add_ref(cl_context ctx)
{
  assert(ctx);
  atomic_inc(&ctx->ref_n);
}

LOCAL cl_command_queue
cl_context_create_queue(cl_context ctx,
                        cl_device_id device,
                        cl_command_queue_properties properties, /* XXX */
                        cl_int *errcode_ret)
{
  cl_command_queue queue = NULL;
  cl_int err = CL_SUCCESS;



  /* We create the command queue and store it in the context list of queues */
  TRY_ALLOC (queue, cl_command_queue_new(ctx));
  queue->props = properties;

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return queue;
error:
  cl_command_queue_delete(queue);
  queue = NULL;
  goto exit;
}

cl_buffer_mgr
cl_context_get_bufmgr(cl_context ctx)
{
  return cl_driver_get_bufmgr(ctx->drv);
}

cl_kernel
cl_context_get_static_kernel(cl_context ctx, cl_int index, const char * str_kernel, const char * str_option)
{
  cl_int ret;
  if (!ctx->internal_prgs[index]) {
    size_t length = strlen(str_kernel) + 1;
    ctx->internal_prgs[index] = cl_program_create_from_source(ctx, 1, &str_kernel, &length, NULL);

    if (!ctx->internal_prgs[index])
      return NULL;

    ret = cl_program_build(ctx->internal_prgs[index], str_option);
    if (ret != CL_SUCCESS)
      return NULL;

    ctx->internal_prgs[index]->is_built = 1;

    /* All CL_ENQUEUE_FILL_BUFFER_ALIGN16_xxx use the same program, different kernel. */
    if (index >= CL_ENQUEUE_FILL_BUFFER_ALIGN8_8 && index <= CL_ENQUEUE_FILL_BUFFER_ALIGN8_64) {
      int i = CL_ENQUEUE_FILL_BUFFER_ALIGN8_8;
      for (; i <= CL_ENQUEUE_FILL_BUFFER_ALIGN8_64; i++) {
        if (index != i) {
          assert(ctx->internal_prgs[i] == NULL);
          assert(ctx->internel_kernels[i] == NULL);
          cl_program_add_ref(ctx->internal_prgs[index]);
          ctx->internal_prgs[i] = ctx->internal_prgs[index];
        }

        if (i == CL_ENQUEUE_FILL_BUFFER_ALIGN8_8) {
          ctx->internel_kernels[i] = cl_program_create_kernel(ctx->internal_prgs[index],
                                                              "__cl_fill_region_align8_2", NULL);
        } else if (i == CL_ENQUEUE_FILL_BUFFER_ALIGN8_16) {
          ctx->internel_kernels[i] = cl_program_create_kernel(ctx->internal_prgs[index],
                                                              "__cl_fill_region_align8_4", NULL);
        } else if (i == CL_ENQUEUE_FILL_BUFFER_ALIGN8_32) {
          ctx->internel_kernels[i] = cl_program_create_kernel(ctx->internal_prgs[index],
                                                              "__cl_fill_region_align8_8", NULL);
        } else if (i == CL_ENQUEUE_FILL_BUFFER_ALIGN8_64) {
          ctx->internel_kernels[i] = cl_program_create_kernel(ctx->internal_prgs[index],
                                                              "__cl_fill_region_align8_16", NULL);
        } else
          assert(0);
      }
    } else {
      ctx->internel_kernels[index] = cl_kernel_dup(ctx->internal_prgs[index]->ker[0]);
    }
  }

  return ctx->internel_kernels[index];
}

cl_kernel
cl_context_get_static_kernel_from_bin(cl_context ctx, cl_int index,
                  const char * str_kernel, size_t size, const char * str_option)
{
  cl_int ret;
  cl_int binary_status = CL_SUCCESS;
  if (!ctx->internal_prgs[index]) {
    ctx->internal_prgs[index] = cl_program_create_from_binary(ctx, 1, &ctx->device,
      &size, (const unsigned char **)&str_kernel, &binary_status, &ret);

    if (!ctx->internal_prgs[index])
      return NULL;

    ret = cl_program_build(ctx->internal_prgs[index], str_option);
    if (ret != CL_SUCCESS)
      return NULL;

    ctx->internal_prgs[index]->is_built = 1;

    /* All CL_ENQUEUE_FILL_BUFFER_ALIGN16_xxx use the same program, different kernel. */
    if (index >= CL_ENQUEUE_FILL_BUFFER_ALIGN8_8 && index <= CL_ENQUEUE_FILL_BUFFER_ALIGN8_64) {
      int i = CL_ENQUEUE_FILL_BUFFER_ALIGN8_8;
      for (; i <= CL_ENQUEUE_FILL_BUFFER_ALIGN8_64; i++) {
        if (index != i) {
          assert(ctx->internal_prgs[i] == NULL);
          assert(ctx->internel_kernels[i] == NULL);
          cl_program_add_ref(ctx->internal_prgs[index]);
          ctx->internal_prgs[i] = ctx->internal_prgs[index];
        }

        if (i == CL_ENQUEUE_FILL_BUFFER_ALIGN8_8) {
          ctx->internel_kernels[i] = cl_program_create_kernel(ctx->internal_prgs[index],
                                                              "__cl_fill_region_align8_2", NULL);
        } else if (i == CL_ENQUEUE_FILL_BUFFER_ALIGN8_16) {
          ctx->internel_kernels[i] = cl_program_create_kernel(ctx->internal_prgs[index],
                                                              "__cl_fill_region_align8_4", NULL);
        } else if (i == CL_ENQUEUE_FILL_BUFFER_ALIGN8_32) {
          ctx->internel_kernels[i] = cl_program_create_kernel(ctx->internal_prgs[index],
                                                              "__cl_fill_region_align8_8", NULL);
        } else if (i == CL_ENQUEUE_FILL_BUFFER_ALIGN8_64) {
          ctx->internel_kernels[i] = cl_program_create_kernel(ctx->internal_prgs[index],
                                                              "__cl_fill_region_align8_16", NULL);
        } else
          assert(0);
      }
    } else {
      ctx->internel_kernels[index] = cl_kernel_dup(ctx->internal_prgs[index]->ker[0]);
    }
  }

  return ctx->internel_kernels[index];
}
