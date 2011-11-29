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
#include "cl_mem.h"
#include "cl_alloc.h"
#include "cl_utils.h"

#include "CL/cl.h"
#include "intel_bufmgr.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

static void
cl_arg_list_destroy(cl_arg_info_t *arg_info)
{
  cl_arg_info_t *next_arg_info = NULL;
  while (arg_info) {
    next_arg_info = arg_info->next;
    cl_free(arg_info);
    arg_info = next_arg_info;
  }
}

static void
cl_curbe_list_destroy(cl_curbe_patch_info_t *curbe_info)
{
  cl_curbe_patch_info_t *next_curbe_info = NULL;
  while (curbe_info) {
    next_curbe_info = curbe_info->next;
    cl_free(curbe_info);
    curbe_info = next_curbe_info;
  }
}

/* Header for all internal objects (cl_mem_object, cl_kernel_object, ...) */
typedef struct cl_object_header {
  uint64_t magic;
  volatile int ref_n;
} cl_object_header_t;

static void
cl_kernel_release_args(cl_kernel k)
{
  uint32_t i;
  assert(k->args);
  for (i = 0; i < k->arg_n; ++i) {
    cl_object_header_t *header = (cl_object_header_t *) k->args[i];
    if (header == NULL)
      continue;
    FATAL_IF (header->magic != CL_MAGIC_MEM_HEADER,
              "A non memory object was set as an argument");
    cl_mem_delete((cl_mem)header);
  }
}

LOCAL void
cl_kernel_delete(cl_kernel k)
{
  if (k == NULL)
    return;

  /* We are not done with the kernel */
  if (atomic_dec(&k->ref_n) > 1) return;

  /* User may have set some OCL object as arguments. As we referenced them when
   * we set them, we release all their references here
   */
  if (k->args) cl_kernel_release_args(k);

  /* Free the chain lists (may also be arrays) */
  cl_arg_list_destroy(k->arg_info);
  cl_curbe_list_destroy(k->curbe_info);

  /* Free the CURBE data */
  cl_free(k->cst_buffer);

  /* Free the argument array */
  cl_free(k->args);

  /* Free the array to track argument setting */
  cl_free(k->is_provided);

  /* Release one reference on all bos we own */
  if (k->bo)       drm_intel_bo_unreference(k->bo);
  if (k->const_bo) drm_intel_bo_unreference(k->const_bo);

  /* This will be true for kernels created by clCreateKernel */
  if (k->ref_its_program) cl_program_delete(k->program);

  cl_free(k->name);
  k->magic = CL_MAGIC_DEAD_HEADER; /* For safety */
  cl_free(k);
}

LOCAL cl_kernel
cl_kernel_new(void)
{
  cl_kernel k = NULL;
  TRY_ALLOC_NO_ERR (k, CALLOC(struct _cl_kernel));
  k->ref_n = 1;
  k->magic = CL_MAGIC_KERNEL_HEADER;

exit:
  return k;
error:
  cl_kernel_delete(k);
  k = NULL;
  goto exit;
}

LOCAL void
cl_kernel_add_ref(cl_kernel k)
{
  atomic_inc(&k->ref_n);
}

static void
cl_kernel_chain_arg(cl_kernel k, cl_arg_info_t *arg_info)
{
  cl_arg_info_t *next = k->arg_info;
  arg_info->next = next;
  k->arg_info = arg_info;
}

static void
cl_kernel_chain_curbe(cl_kernel k, cl_curbe_patch_info_t *curbe_info)
{
  cl_curbe_patch_info_t *next = k->curbe_info;
  curbe_info->next = next;
  k->curbe_info = curbe_info;
}

static INLINE cl_curbe_patch_info_t*
cl_kernel_get_curbe_info_list(cl_kernel k, uint64_t key)
{
  cl_curbe_patch_info_t *curbe_info = k->curbe_info;
  while (curbe_info) {
    if (curbe_info->key == key) break;
    curbe_info = curbe_info->next;
  }
  return curbe_info;
}

static INLINE cl_curbe_patch_info_t*
cl_kernel_new_curbe_info(cl_kernel k, cl_patch_data_parameter_buffer_t *data)
{
  cl_curbe_patch_info_t *curbe = NULL;

  TRY_ALLOC_NO_ERR (curbe, CALLOC(cl_curbe_patch_info_t));
  curbe->type       = data->type;
  curbe->arg_index  = data->index;
  curbe->offsets[0] = data->offset;
  curbe->sz         = data->data_sz;
  curbe->src_offset = data->src_offset;
  curbe->is_local   = CL_FALSE;
  curbe->last       = 0;
  cl_kernel_chain_curbe(k, curbe);

exit:
  return curbe;
error:
  cl_free(curbe);
  curbe = NULL;
  goto exit;
}

static int
cl_arg_cmp(const void *a, const void *b)
{
  const cl_arg_info_t *arg0 = (const cl_arg_info_t *) a;
  const cl_arg_info_t *arg1 = (const cl_arg_info_t *) b;
  return arg0->arg_index > arg1->arg_index;
}

static int
cl_curbe_cmp(const void *a, const void *b)
{
  const cl_curbe_patch_info_t *curbe0 = (const cl_curbe_patch_info_t *) a;
  const cl_curbe_patch_info_t *curbe1 = (const cl_curbe_patch_info_t *) b;
  return curbe0->key > curbe1->key;
}

static cl_int
cl_kernel_sort_arg_list(cl_kernel k)
{
  cl_arg_info_t *arg_info = NULL;
  cl_arg_info_t *array = NULL;
  cl_int arg_info_n = 0;
  cl_int err = CL_SUCCESS;

  /* How many arguments do we have? */
  arg_info = k->arg_info;
  while (arg_info) {
    arg_info_n++;
    arg_info = arg_info->next;
  }

  /* Now fill the array with the unsorted arguments */
  TRY_ALLOC (array, CALLOC_ARRAY(cl_arg_info_t, arg_info_n));
  arg_info = k->arg_info;
  arg_info_n = 0;
  while (arg_info) {
    array[arg_info_n++] = *arg_info;
    array->next = NULL;
    arg_info = arg_info->next;
  }

  /* Sort the argument list array */
  qsort(array, arg_info_n, sizeof(cl_arg_info_t), cl_arg_cmp);

  /* Replace the list by the array */
  cl_arg_list_destroy(k->arg_info);
  k->arg_info = array;
  k->arg_info_n = arg_info_n;

exit:
  return err;
error:
  cl_free(array);
  goto exit;
}

static cl_int
cl_kernel_sort_curbe_info_list(cl_kernel k)
{
  cl_curbe_patch_info_t *curbe_info = NULL;
  cl_curbe_patch_info_t *array = NULL;
  cl_int curbe_info_n = 0;
  cl_int err = CL_SUCCESS;

  /* How many curbe info do we have? */
  curbe_info = k->curbe_info;
  while (curbe_info) {
    curbe_info_n++;
    curbe_info = curbe_info->next;
  }

  /* Now fill the array with the unsorted curbe info */
  TRY_ALLOC (array, CALLOC_ARRAY(cl_curbe_patch_info_t, curbe_info_n));
  curbe_info = k->curbe_info;
  curbe_info_n = 0;
  while (curbe_info) {
    array[curbe_info_n++] = *curbe_info;
    array->next = NULL;
    curbe_info = curbe_info->next;
  }

  /* Sort the curbe list array */
  qsort(array, curbe_info_n, sizeof(cl_curbe_patch_info_t), cl_curbe_cmp);

  /* Replace the list by the array */
  cl_curbe_list_destroy(k->curbe_info);
  k->curbe_info = array;
  k->curbe_info_n = curbe_info_n;
  k->curbe_info->next = NULL;

exit:
  return err;
error:
  cl_free(array);
  goto exit;
}

#define ASSOC_ITEM(ENUM,TYPE,FIELD)                               \
  case JOIN(PATCH_TOKEN_, ENUM):                                  \
    info->FIELD = *(JOIN(JOIN(cl_patch_,TYPE),_t)*) patch;        \
  break;

static cl_int
cl_kernel_allocate_inline_buffer(cl_kernel k,
                                 cl_patch_alloc_surf_with_init_t *init,
                                 const char **patch,
                                 size_t *read)
{
  drm_intel_bufmgr *bufmgr = NULL;
  const size_t sz = init->sz;
  cl_int err = CL_SUCCESS;

  FATAL_IF (init->offset % SURFACE_SZ, "Bad alignment for inline buffer offset");
  FATAL_IF (k->const_bo != NULL, "inline buffer already declared");
  assert(k->program && k->program->ctx);
  bufmgr = cl_context_get_intel_bufmgr(k->program->ctx);
  TRY_ALLOC (k->const_bo, drm_intel_bo_alloc(bufmgr,
                                             "Inline buffer",
                                             sz,
                                             64));
  drm_intel_bo_subdata(k->const_bo, 0, sz, &init->data);
  k->const_bo_index = init->offset / SURFACE_SZ;
  *read += sz;
  *patch += sz;

error:
  return err;
}

static cl_int
cl_kernel_setup_patch_list(cl_kernel k, const char *patch, size_t sz)
{
  cl_kernel_patch_info_t *info = &k->patch;
  cl_curbe_patch_info_t *curbe_info = NULL;
  cl_arg_info_t *arg_info = NULL;
  uint64_t curbe_key;
  size_t read = 0;
  cl_int err = CL_SUCCESS;

  while (read < sz) {
    const cl_patch_item_header_t *item = (const cl_patch_item_header_t *) patch;
    switch (item->token) {
      case PATCH_TOKEN_MEDIA_VFE_STATE: break;
      ASSOC_ITEM (MEDIA_INTERFACE_DESCRIPTOR_LOAD, interface_desc_load, idrt);
      ASSOC_ITEM (STATE_SIP, sip, sip);
      ASSOC_ITEM (MEDIA_CURBE_LOAD, curbe_load, curbe);
      ASSOC_ITEM (SAMPLER_STATE_ARRAY, sampler_state_array, sampler_state);
      ASSOC_ITEM (INTERFACE_DESCRIPTOR_DATA, interface_desc_data, surf_desc);
      ASSOC_ITEM (BINDING_TABLE_STATE, binding_table_state, binding_table);
      ASSOC_ITEM (ALLOCATE_SCRATCH_SURFACE, alloc_scratch_surf, scratch);
      ASSOC_ITEM (ALLOCATE_PRIVATE_MEMORY, alloc_private_memory_surf, private_surf);
      ASSOC_ITEM (ALLOCATE_LOCAL_SURFACE, alloc_local_surf, local_surf);
      ASSOC_ITEM (EXECUTION_ENVIRONMENT, exec_env, exec_env);
      ASSOC_ITEM (THREAD_PAYLOAD, thread_payload, thread_payload);

      case PATCH_TOKEN_DATA_PARAMETER_STREAM:
        info->curbe.sz = *(uint32_t *) (patch + sizeof(cl_patch_item_header_t));
        info->curbe.offset = 0;
      break;
      case PATCH_TOKEN_IMAGE_MEMORY_KERNEL_ARGUMENT:
      case PATCH_TOKEN_CONSTANT_MEMORY_KERNEL_ARGUMENT:
      case PATCH_TOKEN_GLOBAL_MEMORY_KERNEL_ARGUMENT:
      {
        cl_global_memory_object_arg_t *from = (cl_global_memory_object_arg_t *) patch;

        TRY_ALLOC (arg_info, CALLOC(cl_arg_info_t));
        arg_info->arg_index = from->index;
        arg_info->offset = from->offset;
        if (item->token == PATCH_TOKEN_GLOBAL_MEMORY_KERNEL_ARGUMENT)
          arg_info->type = OCLRT_ARG_TYPE_BUFFER;
        else if (item->token == PATCH_TOKEN_CONSTANT_MEMORY_KERNEL_ARGUMENT)
          arg_info->type = OCLRT_ARG_TYPE_CONST;
        else if (item->token == PATCH_TOKEN_IMAGE_MEMORY_KERNEL_ARGUMENT)
          arg_info->type = OCLRT_ARG_TYPE_IMAGE;
        else
          assert(0);

        arg_info->sz = sizeof(cl_mem);
        arg_info->is_patched = CL_FALSE;

        /* Chain the argument to the next arguments */
        cl_kernel_chain_arg(k, arg_info);
        k->arg_n = MAX(k->arg_n, arg_info->arg_index);
        k->arg_info_n++;
      }
      break;

      case PATCH_TOKEN_ALLOCATE_SURFACE_WITH_INITIALIZATION:
      {
        cl_patch_alloc_surf_with_init_t *from = (cl_patch_alloc_surf_with_init_t *) patch;
        TRY (cl_kernel_allocate_inline_buffer, k, from, &patch, &read);
      }
      break;

      case PATCH_TOKEN_DATA_PARAMETER_BUFFER:
      {
        cl_patch_data_parameter_buffer_t *data = (cl_patch_data_parameter_buffer_t *) patch;
        switch (data->type)
        {
          case DATA_PARAMETER_KERNEL_ARGUMENT:
          case DATA_PARAMETER_SUM_OF_LOCAL_MEMORY_ARGUMENT_SIZES:
          case DATA_PARAMETER_LOCAL_WORK_SIZE:
          case DATA_PARAMETER_GLOBAL_WORK_SIZE:
          case DATA_PARAMETER_GLOBAL_WORK_OFFSET:
          case DATA_PARAMETER_NUM_WORK_GROUPS:
          case DATA_PARAMETER_WORK_DIMENSIONS:
          case DATA_PARAMETER_IMAGE_WIDTH:
          case DATA_PARAMETER_IMAGE_HEIGHT:
          case DATA_PARAMETER_IMAGE_DEPTH:
          case DATA_PARAMETER_IMAGE_CHANNEL_DATA_TYPE:
          case DATA_PARAMETER_IMAGE_CHANNEL_ORDER:
          case DATA_PARAMETER_NUM_HARDWARE_THREADS:
          {
            curbe_key = cl_curbe_key(data->type, data->index, data->src_offset);
            curbe_info = cl_kernel_get_curbe_info_list(k, curbe_key);
            if (curbe_info != NULL)
              curbe_info->offsets[++curbe_info->last] = data->offset;
            else
              TRY_ALLOC (curbe_info, cl_kernel_new_curbe_info(k, data));
            curbe_info->key = curbe_key;
            curbe_info->is_patched = CL_FALSE;
            curbe_info = NULL;
            k->arg_n = MAX(k->arg_n, data->index);

            /* We will need to allocate a local surface */
            if (data->type == DATA_PARAMETER_SUM_OF_LOCAL_MEMORY_ARGUMENT_SIZES)
              k->has_local_buffer = CL_TRUE;
            break;
          }
          default: NOT_IMPLEMENTED;
        }
      }
      break;
      default:
        FATAL("Undefined item in patch list");
      break;
    }
    patch += item->size;
    read += item->size;
  }

  if (k->patch.local_surf.sz != 0)
    k->has_local_buffer = CL_TRUE;

  /* k->arg_n was the offset of the last argument. Turn it into an argument
   * number
   */
  k->arg_n++;

  /* Transform the argument and the curbe info lists into sorted arrays */
  if (k->arg_info)
    TRY (cl_kernel_sort_arg_list, k);
  if (k->curbe_info)
    TRY (cl_kernel_sort_curbe_info_list, k);

error:
  return err;
}

#undef ASSOC_ITEM

LOCAL int
cl_kernel_setup(cl_kernel k, const char *ker)
{
  drm_intel_bufmgr *bufmgr = NULL;
  int err = 0;

  /* Kernel instruction */
  FATAL_IF (k->kernel_heap_sz == 0, "No instruction found for this kernel");
  k->kernel_heap = ker;
  ker += k->kernel_heap_sz;

  /* No general heap */
  FATAL_IF (k->general_heap_sz, "General heap unsupported");

  /* Dynamic heap */
  if (k->dynamic_heap_sz) {
    k->dynamic_heap = ker;
    ker += k->dynamic_heap_sz;
  }

  /* Surface state heap */
  if (k->surface_heap_sz) {
    k->surface_heap = ker;
    ker += k->surface_heap_sz;
  }

  /* Patch list */
  if (k->patch_list_sz) {
    k->patch_list = ker;
    ker += k->patch_list_sz;
  }

  /* Read all the patch elements */
  TRY (cl_kernel_setup_patch_list, k, k->patch_list, k->patch_list_sz);

  /* Create the kernel in GPU memory */
  assert(k->program && k->program->ctx);
  bufmgr = cl_context_get_intel_bufmgr(k->program->ctx);
  assert(bufmgr);
  TRY_ALLOC (k->bo, drm_intel_bo_alloc(bufmgr,
                                       "OCL kernel",
                                       k->kernel_heap_sz,
                                       64));
  drm_intel_bo_subdata(k->bo, 0, k->kernel_heap_sz, k->kernel_heap);

  /* We have some restrictions on the compiled binary */
  FATAL_IF (k->patch.exec_env.largest_compiled_simd_sz != 16, "Unsupported SIMD size");
  FATAL_IF (k->patch.exec_env.compiled_simd16 == 0, "Unsupported SIMD size");

error:
  return err;
}

LOCAL cl_kernel
cl_kernel_dup(cl_kernel from)
{
  cl_kernel to = NULL;
  size_t name_sz = 0;
  size_t cst_buffer_sz = 0;

  assert(from);
  TRY_ALLOC_NO_ERR (to, CALLOC(struct _cl_kernel));
  *to = *from; /* most fields do not belong to the kernel but the program */
  to->ref_n = 1;
  name_sz = strlen(from->name) + 1; /* zero terminated */
  TRY_ALLOC_NO_ERR (to->name, CALLOC_ARRAY(char, name_sz));
  memcpy(to->name, from->name, name_sz);

  /* Duplicate the argument info list */
  if (from->arg_info != NULL) {
    assert(from->arg_info_n != 0);
    assert(from->arg_info->next == NULL);
    TRY_ALLOC_NO_ERR (to->arg_info, CALLOC_ARRAY(cl_arg_info_t, from->arg_info_n));
    memcpy(to->arg_info, from->arg_info, sizeof(cl_arg_info_t) * from->arg_info_n);
  }

  /* Duplicate the curbe info */
  if (from->curbe_info != NULL) {
    assert(from->curbe_info_n != 0);
    assert(from->curbe_info->next == NULL);
    TRY_ALLOC_NO_ERR (to->curbe_info,
                      CALLOC_ARRAY(cl_curbe_patch_info_t, from->curbe_info_n));
    memcpy(to->curbe_info,
           from->curbe_info,
           sizeof(cl_curbe_patch_info_t) * from->curbe_info_n);
  }

  /* This kernel (used outside the internal code) will need to see its CURBE
   * updated when setting arguments
   */
  cst_buffer_sz = ALIGN(to->patch.curbe.sz, 32);
  if (cst_buffer_sz)
    TRY_ALLOC_NO_ERR (to->cst_buffer, cl_malloc(cst_buffer_sz));

  /* We store for each argument the buffer currently set */
  TRY_ALLOC_NO_ERR (to->args, CALLOC_ARRAY(void*, to->arg_n));

  /* We track here that all arguments are provided by the user */
  TRY_ALLOC_NO_ERR (to->is_provided, CALLOC_ARRAY(uint8_t, to->arg_n));

  /* Retain the bos */
  if (from->bo)       drm_intel_bo_reference(from->bo);
  if (from->const_bo) drm_intel_bo_reference(from->const_bo);

  /* We retain the program destruction since this kernel (user allocated)
   * depends on the program for some of its pointers
   */
  assert(from->program);
  cl_program_add_ref(from->program);
  to->ref_its_program = CL_TRUE;

exit:
  return to;
error:
  cl_free(to->arg_info);
  cl_free(to->curbe_info);
  to->arg_info = NULL;
  to->curbe_info = NULL;
  cl_kernel_delete(to);
  to = NULL;
  goto exit;
}

/* arg_info / curbe_info are sorted. Just use a dichotomic search */
#define DECL_DICHO_SEARCH(FN, TYPE, KEY_TYPE, FIELD, SUB_FIELD)     \
LOCAL TYPE*                                                         \
JOIN(cl_kernel_get_,FN)(cl_kernel k, KEY_TYPE index)                \
{                                                                   \
  uint32_t begin = 0, end = k->JOIN(FN,_n);                         \
                                                                    \
  while (end > begin) {                                             \
    uint32_t mid = (begin + end) / 2;                               \
    if (k->FIELD[mid].SUB_FIELD == index)                           \
      return k->FIELD + mid;                                        \
    else if (k->FIELD[mid].SUB_FIELD > index)                       \
      end = mid;                                                    \
    else                                                            \
      begin = mid + 1;                                              \
  }                                                                 \
                                                                    \
  return NULL;                                                      \
}

DECL_DICHO_SEARCH(arg_info, cl_arg_info_t, uint32_t, arg_info, arg_index)
DECL_DICHO_SEARCH(curbe_info, cl_curbe_patch_info_t, uint64_t, curbe_info, key)

#undef DECL_DICHO_SEARCH

/* Set the given value (typically a function parameter)
 * in the constant buffer
 */
static cl_int
cl_kernel_set_curbe_entry(cl_kernel k,
                          uint32_t index,
                          size_t sz,
                          const void *value)
{
  cl_curbe_patch_info_t *info = NULL;
  uint64_t key;
  cl_int err = CL_SUCCESS;
  uint32_t i;

  /* Case 1: regular kernel argument (int, float ...) */
  key = cl_curbe_key(DATA_PARAMETER_KERNEL_ARGUMENT, index, 0);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL) {

    /* User must give a value for these arguments */
    if (value == NULL) {
      err = CL_INVALID_ARG_VALUE;
      goto error;
    }

    /* Sizes must match */
    if (UNLIKELY(sz > info->sz)) {
      err = CL_INVALID_ARG_SIZE;
      goto error;
    }

    /* Patch all locations */
    assert(k->cst_buffer);
    for (i = 0; i <= info->last; ++i) {
      assert(sz + info->offsets[i] <= k->patch.curbe.sz);
      memcpy(k->cst_buffer + info->offsets[i], value, sz);
    }

    /* We are done */
    goto exit;
  }

  /* Case 2: Local buffer size */
  key = cl_curbe_key(DATA_PARAMETER_SUM_OF_LOCAL_MEMORY_ARGUMENT_SIZES, index, 0);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL) {
    info->sz = sz;
    goto exit;
  }

  /* Problem. We were not able to find anything */
  err = CL_INVALID_ARG_INDEX;

exit:
error:
  return err;
}

LOCAL cl_int
cl_kernel_set_arg(cl_kernel k, cl_uint index, size_t sz, const void *value)
{
  const cl_arg_info_t *arg_info = NULL;
  cl_mem *mem = NULL;
  cl_int err = CL_SUCCESS;

  /* Not a valid argument if exce*/
  assert(k);
  if (UNLIKELY(index >= k->arg_n)) {
    err = CL_INVALID_ARG_VALUE;
    goto error;
  }

  /* Is it a buffer / image / sampler to set */
  if ((arg_info = cl_kernel_get_arg_info(k, index)) != NULL) {
    switch (arg_info->type) {
      case OCLRT_ARG_TYPE_CONST:
      case OCLRT_ARG_TYPE_IMAGE:
      case OCLRT_ARG_TYPE_BUFFER:
      {
        /* Check the buffer consistency */
        FATAL_IF(value == NULL, "Unsupported NULL value for buffer (TBD)");
        if (UNLIKELY(sz != sizeof(void*))) {
          err = CL_INVALID_ARG_SIZE;
          goto error;
        }
        mem = (cl_mem*) value;
        FATAL_IF (mem == NULL, "Buffer cannot be NULL");
        CHECK_MEM((*mem));

        /* The kernel holds a reference on it now */
        cl_mem_add_ref(*mem);
        cl_mem_delete(k->args[index]);
        k->args[index] = *mem;
      }
      k->is_provided[index] = CL_TRUE;
      goto exit;
      default: NOT_IMPLEMENTED;
    }
  }

  TRY (cl_kernel_set_curbe_entry, k, index, sz, value);
  k->is_provided[index] = CL_TRUE;

exit:
error:
  return err;
}

static INLINE int32_t
cl_kernel_get_first_local(cl_kernel k)
{
  int32_t i;
  for (i = 0; i < (int32_t) k->curbe_info_n; ++i)
    if (k->curbe_info[i].type == DATA_PARAMETER_SUM_OF_LOCAL_MEMORY_ARGUMENT_SIZES)
      return i;
  return k->curbe_info_n;
}

LOCAL uint32_t
cl_kernel_local_memory_sz(cl_kernel k)
{
  int32_t i;
  uint32_t local_mem_sz = 0;

  if (k->has_local_buffer) {

    /* Look for all local surfaces offset to set */
    i = cl_kernel_get_first_local(k);

    /* Now, set the offsets for all local surfaces */
    for (; i < (int32_t) k->curbe_info_n; ++i) {
      cl_curbe_patch_info_t *info = k->curbe_info + i;
      const size_t offset = local_mem_sz;
      if (info->type != DATA_PARAMETER_SUM_OF_LOCAL_MEMORY_ARGUMENT_SIZES)
        break;
      assert(info->last == 0);
      assert(sizeof(int32_t) + info->offsets[0] <= k->patch.curbe.sz);
      memcpy(k->cst_buffer + info->offsets[0], &offset, sizeof(int32_t));
      local_mem_sz += info->sz;
    }
    local_mem_sz += k->patch.local_surf.sz;
  }
  return local_mem_sz;
}

LOCAL char*
cl_kernel_create_cst_buffer(cl_kernel k,
                            const size_t *global_wk_off,
                            const size_t *global_wk_sz,
                            const size_t *local_wk_sz,
                            cl_uint wk_dim,
                            cl_uint thread_n)
{
  cl_curbe_patch_info_t *info = NULL;
  const size_t sz = k->patch.curbe.sz;
  uint64_t key = 0;
  char *data = NULL;

  TRY_ALLOC_NO_ERR (data, (char *) cl_calloc(sz, 1));
  memcpy(data, k->cst_buffer, sz);

  /* Global work group offset */
  key = cl_curbe_key(DATA_PARAMETER_GLOBAL_WORK_OFFSET, 0, 0);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], global_wk_off,   sizeof(uint32_t));
  key = cl_curbe_key(DATA_PARAMETER_GLOBAL_WORK_OFFSET, 0, 4);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], global_wk_off+1, sizeof(uint32_t));
  key = cl_curbe_key(DATA_PARAMETER_GLOBAL_WORK_OFFSET, 0, 8);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], global_wk_off+2, sizeof(uint32_t));

  /* Global work group size */
  key = cl_curbe_key(DATA_PARAMETER_GLOBAL_WORK_SIZE, 0, 0);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], global_wk_sz,   sizeof(uint32_t));
  key = cl_curbe_key(DATA_PARAMETER_GLOBAL_WORK_SIZE, 0, 4);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], global_wk_sz+1, sizeof(uint32_t));
  key = cl_curbe_key(DATA_PARAMETER_GLOBAL_WORK_SIZE, 0, 8);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], global_wk_sz+2, sizeof(uint32_t));

  /* Local work group size */
  key = cl_curbe_key(DATA_PARAMETER_LOCAL_WORK_SIZE, 0, 0);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], local_wk_sz,   sizeof(uint32_t));
  key = cl_curbe_key(DATA_PARAMETER_LOCAL_WORK_SIZE, 0, 4);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], local_wk_sz+1, sizeof(uint32_t));
  key = cl_curbe_key(DATA_PARAMETER_LOCAL_WORK_SIZE, 0, 8);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], local_wk_sz+2, sizeof(uint32_t));

  /* HW thread number (Gen7+) */
  key = cl_curbe_key(DATA_PARAMETER_NUM_HARDWARE_THREADS, 0, 0);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], &thread_n, sizeof(uint32_t));

exit:
  return data;
error:
  cl_free(data);
  data = NULL;
  goto exit;
}

LOCAL cl_int
cl_kernel_work_group_sz(cl_kernel ker,
                        const size_t *local_wk_sz,
                        uint32_t wk_dim,
                        size_t *wk_grp_sz)
{
  cl_int err = CL_SUCCESS;
  size_t sz = 0;
  cl_uint i;

  for (i = 0; i < wk_dim; ++i)
    if ((&ker->patch.exec_env.required_wgr_sz_x)[i] &&
        (&ker->patch.exec_env.required_wgr_sz_x)[i] != local_wk_sz[i]) {
      err = CL_INVALID_WORK_ITEM_SIZE;
      goto error;
    }
  sz = local_wk_sz[0];
  for (i = 1; i < wk_dim; ++i)
    sz *= local_wk_sz[i];
  FATAL_IF (sz % 16, "Work group size must be a multiple of 16");
  if (sz > ker->program->ctx->device->max_work_group_size) {
    err = CL_INVALID_WORK_ITEM_SIZE;
    goto error;
  }

error:
  if (wk_grp_sz)
    *wk_grp_sz = sz;
  return err;
}

