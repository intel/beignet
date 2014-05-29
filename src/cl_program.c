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
#include "cl_khr_icd.h"
#include "cl_gbe_loader.h"
#include "CL/cl.h"
#include "CL/cl_intel.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

static void
cl_program_release_sources(cl_program p)
{
  if (p->source) {
    cl_free(p->source);
    p->source = NULL;
  }
}

static void
cl_program_release_binary(cl_program p)
{
  if (p->binary) {
    cl_free(p->binary);
    p->binary = NULL;
  }
}

LOCAL void
cl_program_delete(cl_program p)
{
  uint32_t ref, i;

  if (p == NULL)
    return;

  /* We are not done with it yet */
  if ((ref = atomic_dec(&p->ref_n)) > 1) return;

  /* Destroy the sources and binary if still allocated */
  cl_program_release_sources(p);
  cl_program_release_binary(p);

  /* Release the build options. */
  if (p->build_opts) {
    cl_free(p->build_opts);
    p->build_opts = NULL;
  }

  /* Remove it from the list */
  assert(p->ctx);
  pthread_mutex_lock(&p->ctx->program_lock);
    if (p->prev)
      p->prev->next = p->next;
    if (p->next)
      p->next->prev = p->prev;
    if (p->ctx->programs == p)
      p->ctx->programs = p->next;
  pthread_mutex_unlock(&p->ctx->program_lock);

  cl_free(p->bin);               /* Free the blob */
  for (i = 0; i < p->ker_n; ++i) /* Free the kernels */
    cl_kernel_delete(p->ker[i]);
  cl_free(p->ker);

  /* Program belongs to their parent context */
  cl_context_delete(p->ctx);

  /* Free the program as allocated by the compiler */
  if (p->opaque) gbe_program_delete(p->opaque);

  p->magic = CL_MAGIC_DEAD_HEADER; /* For safety */
  cl_free(p);
}

LOCAL cl_program
cl_program_new(cl_context ctx)
{
  cl_program p = NULL;

  /* Allocate the structure */
  TRY_ALLOC_NO_ERR (p, CALLOC(struct _cl_program));
  SET_ICD(p->dispatch)
  p->ref_n = 1;
  p->magic = CL_MAGIC_PROGRAM_HEADER;
  p->ctx = ctx;
  p->build_log = calloc(1000, sizeof(char));
  if (p->build_log)
    p->build_log_max_sz = 1000;
  /* The queue also belongs to its context */
  cl_context_add_ref(ctx);

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

static cl_int
cl_program_load_gen_program(cl_program p)
{
  cl_int err = CL_SUCCESS;
  uint32_t i;

  assert(p->opaque != NULL);
  p->ker_n = gbe_program_get_kernel_num(p->opaque);

  /* Allocate the kernel array */
  TRY_ALLOC (p->ker, CALLOC_ARRAY(cl_kernel, p->ker_n));

  for (i = 0; i < p->ker_n; ++i) {
    const gbe_kernel opaque = gbe_program_get_kernel(p->opaque, i);
    assert(opaque != NULL);
    TRY_ALLOC (p->ker[i], cl_kernel_new(p));
    cl_kernel_setup(p->ker[i], opaque);
  }

error:
  return err;
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

  program = cl_program_new(ctx);

  // TODO:  Need to check the binary format here to return CL_INVALID_BINARY.
  TRY_ALLOC(program->binary, cl_calloc(lengths[0], sizeof(char)));
  memcpy(program->binary, binaries[0], lengths[0]);
  program->binary_sz = lengths[0];
  program->source_type = FROM_BINARY;

  if (binary_status)
    binary_status[0] = CL_SUCCESS;

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return program;
error:
  cl_program_delete(program);
  program = NULL;
  goto exit;

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

  program = cl_program_new(ctx);
  program->opaque = compiler_program_new_from_llvm(ctx->device->vendor_id, file_name, program->build_log_max_sz, program->build_log, &program->build_log_sz, 1);
  if (UNLIKELY(program->opaque == NULL)) {
    err = CL_INVALID_PROGRAM;
    goto error;
  }

  /* Create all the kernels */
  TRY (cl_program_load_gen_program, program);
  program->source_type = FROM_LLVM;

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return program;
error:
  cl_program_delete(program);
  program = NULL;
  goto exit;
}

LOCAL cl_program
cl_program_create_from_source(cl_context ctx,
                              cl_uint count,
                              const char **strings,
                              const size_t *lengths,
                              cl_int *errcode_ret)

{
  cl_program program = NULL;
  cl_int err = CL_SUCCESS;
  cl_uint i;
  int32_t * lens = NULL;
  int32_t len_total = 0;
  assert(ctx);
  char * p = NULL;
  // the real compilation step will be done at build time since we do not have
  // yet the compilation options
  program = cl_program_new(ctx);
  TRY_ALLOC (lens, cl_calloc(count, sizeof(int32_t)));
  for (i = 0; i < (int) count; ++i) {
    size_t len;
    if (lengths == NULL || lengths[i] == 0)
      len = strlen(strings[i]);
    else
      len = lengths[i];
    lens[i] = len;
    len_total += len;
  }
  TRY_ALLOC(program->source, cl_calloc(len_total+1, sizeof(char)));
  p = program->source;
  for (i = 0; i < (int) count; ++i) {
    memcpy(p, strings[i], lens[i]);
    p += lens[i];
  }
  *p = '\0';

  program->source_type = FROM_SOURCE;

exit:
  cl_free(lens);
  lens = NULL;
  if (errcode_ret)
    *errcode_ret = err;
  return program;
error:
  cl_program_delete(program);
  program = NULL;
  goto exit;
}

LOCAL cl_int
cl_program_build(cl_program p, const char *options)
{
  cl_int err = CL_SUCCESS;
  int i = 0;
  int copyed = 0;

  if (p->ref_n > 1)
    return CL_INVALID_OPERATION;

  if (options) {
    if(p->build_opts == NULL || strcmp(options, p->build_opts) != 0) {
      if(p->build_opts) {
        cl_free(p->build_opts);
        p->build_opts = NULL;
      }
      TRY_ALLOC (p->build_opts, cl_calloc(strlen(options) + 1, sizeof(char)));
      memcpy(p->build_opts, options, strlen(options));

      p->source_type = p->source ? FROM_SOURCE : p->binary ? FROM_BINARY : FROM_LLVM;
    }
  }

  if (options == NULL && p->build_opts) {
    p->source_type = p->source ? FROM_SOURCE : p->binary ? FROM_BINARY : FROM_LLVM;

    cl_free(p->build_opts);
    p->build_opts = NULL;
  }

  if (p->source_type == FROM_SOURCE) {
    if (!CompilerSupported()) {
      err = CL_COMPILER_NOT_AVAILABLE;
      goto error;
    }

    p->opaque = compiler_program_new_from_source(p->ctx->device->vendor_id, p->source, p->build_log_max_sz, options, p->build_log, &p->build_log_sz);
    if (UNLIKELY(p->opaque == NULL)) {
      if (p->build_log_sz > 0 && strstr(p->build_log, "error: error reading 'options'"))
        err = CL_INVALID_BUILD_OPTIONS;
      else
        err = CL_BUILD_PROGRAM_FAILURE;
      goto error;
    }

    /* Create all the kernels */
    TRY (cl_program_load_gen_program, p);
    p->source_type = FROM_LLVM;
  } else if (p->source_type == FROM_BINARY) {
    p->opaque = gbe_program_new_from_binary(p->ctx->device->vendor_id, p->binary, p->binary_sz);
    if (UNLIKELY(p->opaque == NULL)) {
      err = CL_BUILD_PROGRAM_FAILURE;
      goto error;
    }

    /* Create all the kernels */
    TRY (cl_program_load_gen_program, p);
    p->source_type = FROM_LLVM;
  }

  for (i = 0; i < p->ker_n; i ++) {
    const gbe_kernel opaque = gbe_program_get_kernel(p->opaque, i);
    p->bin_sz += gbe_kernel_get_code_size(opaque);
  }

  TRY_ALLOC (p->bin, cl_calloc(p->bin_sz, sizeof(char)));
  for (i = 0; i < p->ker_n; i ++) {
    const gbe_kernel opaque = gbe_program_get_kernel(p->opaque, i);
    size_t sz = gbe_kernel_get_code_size(opaque);

    memcpy(p->bin + copyed, gbe_kernel_get_code(opaque), sz);
    copyed += sz;
  }

error:
  p->is_built = 1;
  return err;
}

LOCAL cl_kernel
cl_program_create_kernel(cl_program p, const char *name, cl_int *errcode_ret)
{
  cl_kernel from = NULL, to = NULL;
  cl_int err = CL_SUCCESS;
  uint32_t i = 0;

  /* Find the program first */
  for (i = 0; i < p->ker_n; ++i) {
    assert(p->ker[i]);
    const char *ker_name = cl_kernel_get_name(p->ker[i]);
    if (strcmp(ker_name, name) == 0) {
      from = p->ker[i];
      break;
    }
  }

  /* We were not able to find this named kernel */
  if (UNLIKELY(from == NULL)) {
    err = CL_INVALID_KERNEL_NAME;
    goto error;
  }

  TRY_ALLOC(to, cl_kernel_dup(from));

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return to;
error:
  cl_kernel_delete(to);
  to = NULL;
  goto exit;
}

LOCAL cl_int
cl_program_create_kernels_in_program(cl_program p, cl_kernel* ker)
{
  int i = 0;

  if(ker == NULL)
    return CL_SUCCESS;

  for (i = 0; i < p->ker_n; ++i) {
    TRY_ALLOC_NO_ERR(ker[i], cl_kernel_dup(p->ker[i]));
  }

  return CL_SUCCESS;

error:
  do {
    cl_kernel_delete(ker[i]);
    ker[i--] = NULL;
  } while(i > 0);

  return CL_OUT_OF_HOST_MEMORY;
}
