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

#ifndef __CL_SAMPLER_H__
#define __CL_SAMPLER_H__

#include "CL/cl.h"
#include "cl_base_object.h"
#include "../backend/src/ocl_common_defines.h"
#include <stdint.h>

/* How to access images */
typedef struct _cl_sampler {
  _cl_base_object base;
  cl_context ctx;             /* Context it belongs to */
  cl_bool normalized_coords;  /* Are coordinates normalized? */
  cl_addressing_mode address; /* CLAMP / REPEAT and so on... */
  cl_filter_mode filter;      /* LINEAR / NEAREST mostly */
  uint32_t clkSamplerValue;
} _cl_sampler;

#define CL_OBJECT_SAMPLER_MAGIC 0x686a0ecba79ce32fLL
#define CL_OBJECT_IS_SAMPLER(obj) ((obj &&                                                     \
                                    ((cl_base_object)obj)->magic == CL_OBJECT_SAMPLER_MAGIC && \
                                    CL_OBJECT_GET_REF(obj) >= 1))

/* Create a new sampler object */
extern cl_sampler cl_create_sampler(cl_context, cl_bool, cl_addressing_mode, cl_filter_mode, cl_int *err);
/* Unref the object and delete it if no more reference on it */
extern void cl_sampler_delete(cl_sampler);
/* Add one more reference to this object */
extern void cl_sampler_add_ref(cl_sampler);
/* set a sampler kernel argument */
int cl_set_sampler_arg_slot(cl_kernel k, int index, cl_sampler sampler);

#endif /* __CL_SAMPLER_H__ */
