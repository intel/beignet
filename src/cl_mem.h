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

#ifndef __CL_MEM_H__
#define __CL_MEM_H__

#include "cl_internals.h"
#include "CL/cl.h"

/* Store the object in video memory */
struct _drm_intel_bo;

/* Used for buffers and images */
struct _cl_mem {
  uint64_t magic;           /* To identify it as a memory object */
  volatile int ref_n;       /* This object is reference counted */
  struct _drm_intel_bo *bo; /* Data in GPU memory */
  cl_mem prev, next;        /* We chain the memory buffers together */
  cl_context ctx;           /* Context it belongs to */
  cl_mem_flags flags;       /* Flags specified at the creation time */
  uint32_t is_image;        /* Indicate if this is an image or not */
  cl_image_format fmt;      /* only for images */
  size_t w,h,depth,pitch;   /* only for images (depth is only for 3d images) */
};

/* Create a new memory object and initialize it with possible user data */
extern cl_mem cl_mem_new(cl_context, cl_mem_flags, size_t, void*, cl_int*);

/* Idem but this is an image */
extern cl_mem cl_mem_new_image2D(cl_context,
                                 cl_mem_flags,
                                 const cl_image_format*,
                                 size_t w,
                                 size_t h,
                                 size_t pitch,
                                 void *,
                                 cl_int *);

/* Unref the object and delete it if no more reference */
extern void cl_mem_delete(cl_mem);

/* Add one more reference to this object */
extern void cl_mem_add_ref(cl_mem);

/* Directly map a memory object (just use drm_intel_bo_map) */
extern void *cl_mem_map(cl_mem);

/* Unmap a memory object (just use drm_intel_bo_unmap) */
extern cl_int cl_mem_unmap(cl_mem);

/* Pin/unpin the buffer in memory (must be root) */
extern cl_int cl_mem_pin(cl_mem);
extern cl_int cl_mem_unpin(cl_mem);

#endif /* __CL_MEM_H__ */

