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

#include "cl_context.h"
#include "cl_sampler.h"
#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_khr_icd.h"
#include "cl_kernel.h"

#include <assert.h>

uint32_t cl_to_clk(cl_bool normalized_coords,
                   cl_addressing_mode address,
                   cl_filter_mode filter)
{
  int clk_address = CLK_ADDRESS_NONE;
  int clk_filter = CLK_FILTER_NEAREST;
  switch (address) {
  case CL_ADDRESS_NONE: clk_address = CLK_ADDRESS_NONE; break;
  case CL_ADDRESS_CLAMP: clk_address = CLK_ADDRESS_CLAMP; break;
  case CL_ADDRESS_CLAMP_TO_EDGE: clk_address = CLK_ADDRESS_CLAMP_TO_EDGE; break;
  case CL_ADDRESS_REPEAT: clk_address = CLK_ADDRESS_REPEAT; break;
  case CL_ADDRESS_MIRRORED_REPEAT: clk_address = CLK_ADDRESS_MIRRORED_REPEAT; break;
  default:
    assert(0);
  }
  switch(filter) {
  case CL_FILTER_NEAREST: clk_filter = CLK_FILTER_NEAREST; break;
  case CL_FILTER_LINEAR: clk_filter = CLK_FILTER_LINEAR; break;
  default:
    assert(0);
  }
  return (clk_address << __CLK_ADDRESS_BASE)
         | (normalized_coords << __CLK_NORMALIZED_BASE)
         | (clk_filter);
}

#define IS_SAMPLER_ARG(v) (v & __CLK_SAMPLER_ARG_KEY_BIT)
#define SAMPLER_ARG_ID(v) ((v & __CLK_SAMPLER_ARG_MASK) >> __CLK_SAMPLER_ARG_BASE)
int cl_set_sampler_arg_slot(cl_kernel k, int index, cl_sampler sampler)
{
  int slot_id;
  for(slot_id = 0; slot_id < k->sampler_sz; slot_id++)
  {
    if (IS_SAMPLER_ARG(k->samplers[slot_id])) {
     if (SAMPLER_ARG_ID(k->samplers[slot_id]) == index) {
       k->samplers[slot_id] = (k->samplers[slot_id] & (~__CLK_SAMPLER_MASK))
                              | sampler->clkSamplerValue;
       return slot_id;
     }
    }
  }
  return -1;
}

LOCAL cl_sampler
cl_sampler_new(cl_context ctx,
               cl_bool normalized_coords,
               cl_addressing_mode address,
               cl_filter_mode filter,
               cl_int *errcode_ret)
{
  cl_sampler sampler = NULL;
  cl_int err = CL_SUCCESS;

  /* Allocate and inialize the structure itself */
  TRY_ALLOC (sampler, CALLOC(struct _cl_sampler));
  SET_ICD(sampler->dispatch)
  sampler->ref_n = 1;
  sampler->magic = CL_MAGIC_SAMPLER_HEADER;
  sampler->normalized_coords = normalized_coords;
  sampler->address = address;
  sampler->filter = filter;

  /* Append the sampler in the context sampler list */
  pthread_mutex_lock(&ctx->sampler_lock);
    sampler->next = ctx->samplers;
    if (ctx->samplers != NULL)
      ctx->samplers->prev = sampler;
    ctx->samplers = sampler;
  pthread_mutex_unlock(&ctx->sampler_lock);
  sampler->ctx = ctx;
  cl_context_add_ref(ctx);

  sampler->clkSamplerValue = cl_to_clk(normalized_coords, address, filter);

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return sampler;
error:
  cl_sampler_delete(sampler);
  sampler = NULL;
  goto exit;
}

LOCAL void
cl_sampler_delete(cl_sampler sampler)
{
  if (UNLIKELY(sampler == NULL))
    return;
  if (atomic_dec(&sampler->ref_n) > 1)
    return;

  assert(sampler->ctx);
  pthread_mutex_lock(&sampler->ctx->sampler_lock);
    if (sampler->prev)
      sampler->prev->next = sampler->next;
    if (sampler->next)
      sampler->next->prev = sampler->prev;
    if (sampler->ctx->samplers == sampler)
      sampler->ctx->samplers = sampler->next;
  pthread_mutex_unlock(&sampler->ctx->sampler_lock);
  cl_context_delete(sampler->ctx);

  cl_free(sampler);
}

LOCAL void
cl_sampler_add_ref(cl_sampler sampler)
{
  assert(sampler);
  atomic_inc(&sampler->ref_n);
}

