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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#if USE_OLD_COMPILER
static const int icbe_ver = 1001;
#else
static const int icbe_ver = 1002;
#endif

#define DECL_LOAD_HEADER(GEN)                                           \
static const char*                                                      \
JOIN(cl_kernel_load_header,GEN)(cl_kernel ker,                          \
                                const char *header,                     \
                                size_t *name_sz,                        \
                                size_t *ker_sz)                         \
{                                                                       \
  const JOIN(JOIN(cl_kernel_header,GEN),_t) *h =                        \
    (const JOIN(JOIN(cl_kernel_header,GEN),_t) *) header;               \
  *ker_sz  = *name_sz = h->header.kernel_name_sz;                       \
  *ker_sz += ker->patch_list_sz   = h->header.patch_list_sz;            \
  *ker_sz += ker->kernel_heap_sz  = h->kernel_heap_sz;                  \
  *ker_sz += ker->general_heap_sz = h->general_state_heap_sz;           \
  *ker_sz += ker->surface_heap_sz = h->surface_state_heap_sz;           \
  *ker_sz += ker->dynamic_heap_sz = h->dynamic_state_heap_sz;           \
  return header + sizeof(JOIN(JOIN(cl_kernel_header,GEN),_t));          \
}

DECL_LOAD_HEADER(6)
DECL_LOAD_HEADER(7)
DECL_LOAD_HEADER(75)

#undef DECL_LOAD_HEADER

static int
cl_program_decode(cl_program p)
{
  cl_program_header_t *header = (cl_program_header_t *) p->bin;
  const char *ker = NULL, *bin = NULL;
  size_t ker_sz = 0, name_sz = 0;
  int i, err = 0;

  /* Check binary consistency */
  assert(p->ctx && p->ctx->device);
  FATAL_IF (header->magic != 0x494e5443, "Bad file format for the program\n");
  FATAL_IF (header->device != p->ctx->device->gfx_id, "File not compiled for this device\n");
  FATAL_IF (header->version != icbe_ver, "Uncompatible compiler\n");
  FATAL_IF ((p->ker_n = header->ker_n) == 0, "No kernel found in the program\n");

  /* Allocate the kernel array */
  TRY_ALLOC (p->ker, CALLOC_ARRAY(cl_kernel, p->ker_n));

  /* Load all kernels */
  ker = bin = p->bin + sizeof(cl_program_header_t);
  for (i = 0; i < header->ker_n; ++i) {

    /* Format changes from generation to generation */
    TRY_ALLOC (p->ker[i], cl_kernel_new());
    switch (header->device) {
      case IGFX_GEN7_5_CORE:
        ker = cl_kernel_load_header75(p->ker[i], ker, &name_sz, &ker_sz);
      break;
      case IGFX_GEN7_CORE:
        ker = cl_kernel_load_header7(p->ker[i], ker, &name_sz, &ker_sz);
      break;
      case IGFX_GEN6_CORE:
        ker = cl_kernel_load_header6(p->ker[i], ker, &name_sz, &ker_sz);
      break;
      default:
        FATAL ("Unsupported platform");
      break;
    }

    /* Set the kernel name */
    TRY_ALLOC (p->ker[i]->name, CALLOC_ARRAY(char, name_sz));
    memcpy(p->ker[i]->name, ker, name_sz);
    name_sz = ALIGN(name_sz, 4);

    /* Points to the kernel code */
    ker += name_sz; 

    /* Initialize the kernel */
    p->ker[i]->program = p;
    TRY (cl_kernel_setup, p->ker[i], ker);

    /* Pointer to the next kernel to setup */
    ker += (ker_sz - name_sz);
  }

exit:
  return err;
error:
  goto exit;
}

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

  /* Decode the binary blob */
  TRY_NO_ERR (cl_program_decode, p);

  /* Append the command queue in the list */
  pthread_mutex_lock(&ctx->program_lock);
    p->next = ctx->programs;
    if (ctx->programs != NULL)
      ctx->programs->prev = p;
    ctx->programs = p;
  pthread_mutex_unlock(&ctx->program_lock);
  cl_context_add_ref(ctx);

exit:
  return p;
error:
  cl_program_delete(p);
  p = NULL;
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

  TRY_ALLOC (program, cl_program_new(ctx, (const char *) binaries[0], lengths[0]));

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
  cl_kernel from = NULL, to = NULL;
  cl_int err = CL_SUCCESS;
  uint32_t i = 0;

  if (UNLIKELY(name == NULL)) {
    err = CL_INVALID_KERNEL_NAME;
    goto error;
  }

  /* Find the program first */
  for (i = 0; i < p->ker_n; ++i) {
    assert(p->ker[i] && p->ker[i]->name);
    if (strcmp(p->ker[i]->name, name) == 0) {
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

