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

#include "cl_kernel.h"
#include "cl_program.h"
#include "cl_device_id.h"
#include "cl_context.h"
#include "cl_alloc.h"
#include "cl_utils.h"
#include "CL/cl.h"
#include "gen/program.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

LOCAL void
cl_program_delete(cl_program p)
{
  uint32_t ref, i;

  if (p == NULL)
    return;

  /* We are not done with it yet */
  if ((ref = atomic_dec(&p->ref_n)) > 1)
    return;

  /* Remove it from the list */
  assert(p->ctx);
  pthread_mutex_lock(&p->ctx->program_lock);
    if (p->prev)
      p->prev->next = p->next;
    if (p->next)
      p->next->prev = p->prev;
    if (p->prev == NULL && p->next == NULL)
      p->ctx->programs = NULL;
  pthread_mutex_unlock(&p->ctx->program_lock);

  cl_free(p->bin);               /* Free the blob */
  for (i = 0; i < p->ker_n; ++i) /* Free the kernels */
    cl_kernel_delete(p->ker[i]);
  cl_free(p->ker);

  /* Program belongs to their parent context */
  cl_context_delete(p->ctx);

  /* Free the program as allocated by the compiler */
  if (p->gen_program) GenProgramDelete(p->gen_program);

  p->magic = CL_MAGIC_DEAD_HEADER; /* For safety */
  cl_free(p);
}

LOCAL cl_program
cl_program_new(cl_context ctx, const char *data, size_t sz)
{
  cl_program p = NULL;

  /* Allocate the structure */
  TRY_ALLOC_NO_ERR (p, CALLOC(struct _cl_program));
  TRY_ALLOC_NO_ERR (p->bin, CALLOC_ARRAY(char, sz));
  memcpy(p->bin, data, sz);
  p->bin_sz = sz;
  p->ref_n = 1;
  p->magic = CL_MAGIC_PROGRAM_HEADER;
  p->ctx = ctx;

exit:
  return p;
error:
  cl_program_delete(p);
  goto exit;
}

LOCAL void
cl_program_add_ref(cl_program p)
{
  assert(p);
  atomic_inc(&p->ref_n);
}

LOCAL cl_program
cl_program_create_from_binary(cl_context             ctx,
                              cl_uint                num_devices,
                              const cl_device_id *   devices,
                              const size_t *         lengths,
                              const unsigned char ** binaries,
                              cl_int *               binary_status,
                              cl_int *               errcode_ret)
{
#if 0
  cl_program program = NULL;
  cl_int err = CL_SUCCESS;

  assert(ctx);
  INVALID_DEVICE_IF (num_devices != 1);
  INVALID_DEVICE_IF (devices == NULL);
  INVALID_DEVICE_IF (devices[0] != ctx->device);
  INVALID_VALUE_IF (binaries == NULL);
  INVALID_VALUE_IF (lengths == NULL);

  if (binaries[0] == NULL) {
    err = CL_INVALID_VALUE;
    if (binary_status)
      binary_status[0] = CL_INVALID_VALUE;
    goto error;
  }

  if (lengths[0] == 0) {
    err = CL_INVALID_VALUE;
    if (binary_status)
      binary_status[0] = CL_INVALID_VALUE;
    goto error;
  }

  // TRY_ALLOC (program, cl_program_new(ctx, (const char *) binaries[0], lengths[0]));

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return program;
error:
  cl_program_delete(program);
  program = NULL;
  goto exit;
#endif
  NOT_IMPLEMENTED;
  return CL_SUCCESS;
}

LOCAL cl_program
cl_program_create_from_llvm(cl_context ctx,
                            cl_uint num_devices,
                            const cl_device_id *devices,
                            const char *file_name,
                            cl_int *errcode_ret)
{
  cl_program program = NULL;
  cl_int err = CL_SUCCESS;

  assert(ctx);
  INVALID_DEVICE_IF (num_devices != 1);
  INVALID_DEVICE_IF (devices == NULL);
  INVALID_DEVICE_IF (devices[0] != ctx->device);
  INVALID_VALUE_IF (file_name == NULL);

  program->gen_program = GenProgramNewFromLLVM(file_name, 0, NULL, NULL);
  if (program->gen_program == NULL) {
    err = CL_INVALID_PROGRAM;
    goto error;
  }

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return program;
error:
  cl_program_delete(program);
  program = NULL;
  goto exit;
}

LOCAL cl_kernel
cl_program_create_kernel(cl_program p, const char *name, cl_int *errcode_ret)
{
  return NULL;
}

