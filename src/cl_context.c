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

#include "cl_platform_id.h"
#include "cl_device_id.h"
#include "cl_genx_driver.h"
#include "cl_context.h"
#include "cl_command_queue.h"
#include "cl_mem.h"
#include "cl_alloc.h"
#include "cl_utils.h"

#include "CL/cl.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

static cl_int
cl_context_properties_is_ok(const cl_context_properties *properties)
{
  const cl_context_properties *prop = properties;
  size_t prop_n = 0;
  cl_int err = CL_SUCCESS;

  if (properties == NULL)
    goto exit;
  while (*prop) {
    prop += 2;
    prop_n++;
  }

  /* XXX */
  FATAL_IF (prop_n > 1, "Only one property is supported now");
  INVALID_VALUE_IF (*properties != CL_CONTEXT_PLATFORM);
  if (UNLIKELY((cl_platform_id) properties[1] != intel_platform)) {
    err = CL_INVALID_PLATFORM;
    goto error;
  }

exit:
error:
  return err;
}

static cl_int
cl_device_id_is_ok(const cl_device_id device)
{
  return device != cl_get_gt_device() ? CL_FALSE : CL_TRUE;
}

LOCAL cl_context
cl_create_context(const cl_context_properties *  properties,
                  cl_uint                        num_devices,
                  const cl_device_id *           devices,
                  void (CL_CALLBACK * pfn_notify) (const char*, const void*, size_t, void*),
                  void *                         user_data,
                  cl_int *                       errcode_ret)
{
  cl_platform_id platform = NULL;
  cl_context ctx = NULL;
  cl_int err = CL_SUCCESS;

  /* Assert parameters correctness */
  INVALID_VALUE_IF (devices == NULL);
  INVALID_VALUE_IF (num_devices == 0);
  INVALID_VALUE_IF (pfn_notify == NULL && user_data != NULL);

  /* XXX */
  FATAL_IF (pfn_notify != NULL || user_data != NULL, "Unsupported call back");
  FATAL_IF (num_devices != 1, "Only one device is supported");

  /* Check that we are getting the right platform */
  if (UNLIKELY((err = cl_context_properties_is_ok(properties)) != CL_SUCCESS))
    goto error;
  platform = intel_platform;

  /* Now check if the user is asking for the right device */
  if (UNLIKELY(cl_device_id_is_ok(*devices) == CL_FALSE)) {
    err = CL_INVALID_DEVICE;
    goto error;
  }

  /* We are good */
  if (UNLIKELY((ctx = cl_context_new()) == NULL)) {
    err = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }

  /* Attach the device to the context */
  ctx->device = *devices;

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
cl_context_new(void)
{
  cl_context ctx = NULL;
  int err = 0;

  TRY_ALLOC (ctx, CALLOC(struct _cl_context));
  TRY_ALLOC (ctx->intel_drv, cl_genx_driver_new());
  ctx->magic = CL_MAGIC_CONTEXT_HEADER;
  ctx->ref_n = 1;
  pthread_mutex_init(&ctx->program_lock, NULL);
  pthread_mutex_init(&ctx->queue_lock, NULL);
  pthread_mutex_init(&ctx->buffer_lock, NULL);

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
  if (UNLIKELY(ctx == NULL))
    return;

  /* We are not done yet */
  if (atomic_dec(&ctx->ref_n) > 1)
    return;

  /* All object lists should have been freed. Otherwise, the reference counter
   * of the context cannot be 0
   */
  assert(ctx->queues == NULL);
  assert(ctx->programs == NULL);
  assert(ctx->buffers == NULL);
  assert(ctx->intel_drv);
  cl_genx_driver_delete(ctx->intel_drv);
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

  if (UNLIKELY(device != ctx->device)) {
    err = CL_INVALID_DEVICE;
    goto error;
  }

  /* We create the command queue and store it in the context list of queues */
  TRY_ALLOC (queue, cl_command_queue_new(ctx));

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return queue;
error:
  cl_command_queue_delete(queue);
  goto exit;
}

struct intel_driver;
extern struct _drm_intel_bufmgr* intel_driver_get_buf(struct intel_driver*);

struct _drm_intel_bufmgr*
cl_context_get_intel_bufmgr(cl_context ctx)
{
  return intel_driver_get_buf((struct intel_driver*) ctx->intel_drv);
}

