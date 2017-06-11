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
#include "cl_device_id.h"
#include "cl_alloc.h"

static uint32_t
sampler_cl_to_clk(cl_bool normalized_coords, cl_addressing_mode address, cl_filter_mode filter)
{
  int clk_address = CLK_ADDRESS_NONE;
  int clk_filter = CLK_FILTER_NEAREST;
  switch (address) {
  case CL_ADDRESS_NONE:
    clk_address = CLK_ADDRESS_NONE;
    break;
  case CL_ADDRESS_CLAMP:
    clk_address = CLK_ADDRESS_CLAMP;
    break;
  case CL_ADDRESS_CLAMP_TO_EDGE:
    clk_address = CLK_ADDRESS_CLAMP_TO_EDGE;
    break;
  case CL_ADDRESS_REPEAT:
    clk_address = CLK_ADDRESS_REPEAT;
    break;
  case CL_ADDRESS_MIRRORED_REPEAT:
    clk_address = CLK_ADDRESS_MIRRORED_REPEAT;
    break;
  default:
    assert(0);
  }
  switch (filter) {
  case CL_FILTER_NEAREST:
    clk_filter = CLK_FILTER_NEAREST;
    break;
  case CL_FILTER_LINEAR:
    clk_filter = CLK_FILTER_LINEAR;
    break;
  default:
    assert(0);
  }
  return (clk_address << __CLK_ADDRESS_BASE) | (normalized_coords << __CLK_NORMALIZED_BASE) | (clk_filter);
}

LOCAL cl_sampler
cl_create_sampler(cl_context ctx, cl_bool normalized_coords, cl_addressing_mode address,
                  cl_filter_mode filter, cl_int *errcode_ret)
{
  cl_sampler sampler = NULL;
  cl_int err = CL_SUCCESS;
  cl_int i;

  /* Allocate and inialize the structure itself */
  sampler = CL_CALLOC(1, sizeof(_cl_sampler));
  if (sampler == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }

  sampler->each_device = CL_CALLOC(ctx->device_num, sizeof(cl_sampler_for_device));
  if (sampler->each_device == NULL) {
    CL_FREE(sampler);
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }

  CL_OBJECT_INIT_BASE(sampler, CL_OBJECT_SAMPLER_MAGIC);
  sampler->normalized_coords = normalized_coords;
  sampler->address = address;
  sampler->filter = filter;

  sampler->clkSamplerValue = sampler_cl_to_clk(normalized_coords, address, filter);
  /* Append the sampler in the context sampler list */
  cl_context_add_sampler(ctx, sampler);

  for (i = 0; i < ctx->device_num; i++) {
    err = (ctx->devices[i]->api.sampler_create)(ctx->devices[i], sampler);
    if (err != CL_SUCCESS) {
      *errcode_ret = err;
      cl_sampler_delete(sampler);
      return NULL;
    }
  }

  *errcode_ret = CL_SUCCESS;
  return sampler;
}

LOCAL void
cl_sampler_delete(cl_sampler sampler)
{
  cl_int i;

  if (UNLIKELY(sampler == NULL))
    return;
  if (CL_OBJECT_DEC_REF(sampler) > 1)
    return;

  for (i = 0; i < sampler->each_device_num; i++) {
    if (sampler->each_device[i])
      (sampler->each_device[i]->device->api.sampler_delete)(sampler->each_device[i]->device, sampler);
  }
  CL_FREE(sampler->each_device);

  cl_context_remove_sampler(sampler->ctx, sampler);

  CL_OBJECT_DESTROY_BASE(sampler);
  CL_FREE(sampler);
}

LOCAL void
cl_sampler_add_ref(cl_sampler sampler)
{
  assert(sampler);
  CL_OBJECT_INC_REF(sampler);
}
