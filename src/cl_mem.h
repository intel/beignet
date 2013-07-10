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
#include "cl_driver.h"
#include "CL/cl.h"

#ifndef CL_VERSION_1_2
#define CL_MEM_OBJECT_IMAGE1D                       0x10F4
#define CL_MEM_OBJECT_IMAGE1D_ARRAY                 0x10F5
#define CL_MEM_OBJECT_IMAGE1D_BUFFER                0x10F6
#define CL_MEM_OBJECT_IMAGE2D_ARRAY                 0x10F3
typedef struct _cl_image_desc {
    cl_mem_object_type      image_type;
    size_t                  image_width;
    size_t                  image_height;
    size_t                  image_depth;
    size_t                  image_array_size;
    size_t                  image_row_pitch;
    size_t                  image_slice_pitch;
    cl_uint                 num_mip_levels;
    cl_uint                 num_samples;
    cl_mem                  buffer;
} cl_image_desc;
#endif

typedef enum cl_image_tiling {
  CL_NO_TILE = 0,
  CL_TILE_X  = 1,
  CL_TILE_Y  = 2
} cl_image_tiling_t;

/* Used for buffers and images */
struct _cl_mem {
  DEFINE_ICD(dispatch)
  uint64_t magic;           /* To identify it as a memory object */
  volatile int ref_n;       /* This object is reference counted */
  cl_buffer bo;             /* Data in GPU memory */
  void *egl_image;          /* created from external egl image*/
  size_t size;              /* original request size, not alignment size, used in constant buffer */
  cl_mem prev, next;        /* We chain the memory buffers together */
  cl_context ctx;           /* Context it belongs to */
  cl_mem_flags flags;       /* Flags specified at the creation time */
  uint32_t is_image;        /* Indicate if this is an image or not */
  cl_image_format fmt;      /* only for images */
  cl_mem_object_type type;  /* only for images 1D/2D...*/
  size_t w,h,depth;         /* only for images (depth is only for 3D images) */
  size_t row_pitch,slice_pitch;
  uint32_t intel_fmt;       /* format to provide in the surface state */
  uint32_t bpp;             /* number of bytes per pixel */
  cl_image_tiling_t tiling; /* only IVB+ supports TILE_[X,Y] (image only) */
};

/* Query information about a memory object */
extern cl_int cl_get_mem_object_info(cl_mem, cl_mem_info, size_t, void *, size_t *);

/* Query information about an image */
extern cl_int cl_get_image_info(cl_mem, cl_image_info, size_t, void *, size_t *);

/* Create a new memory object and initialize it with possible user data */
extern cl_mem cl_mem_new(cl_context, cl_mem_flags, size_t, void*, cl_int*);

/* Idem but this is an image */
extern cl_mem
cl_mem_new_image(cl_context context,
                 cl_mem_flags flags,
                 const cl_image_format *image_format,
                 const cl_image_desc *image_desc,
                 void *host_ptr,
                 cl_int *errcode_ret);

/* Unref the object and delete it if no more reference */
extern void cl_mem_delete(cl_mem);

/* Destroy egl image. */
extern void cl_mem_gl_delete(cl_mem);

/* Add one more reference to this object */
extern void cl_mem_add_ref(cl_mem);

/* Directly map a memory object */
extern void *cl_mem_map(cl_mem);

/* Unmap a memory object */
extern cl_int cl_mem_unmap(cl_mem);

/* Directly map a memory object in GTT mode */
extern void *cl_mem_map_gtt(cl_mem);

/* Unmap a memory object in GTT mode */
extern cl_int cl_mem_unmap_gtt(cl_mem);

/* Directly map a memory object - tiled images are mapped in GTT mode */
extern void *cl_mem_map_auto(cl_mem);

/* Unmap a memory object - tiled images are unmapped in GTT mode */
extern cl_int cl_mem_unmap_auto(cl_mem);

/* Pin/unpin the buffer in memory (you must be root) */
extern cl_int cl_mem_pin(cl_mem);
extern cl_int cl_mem_unpin(cl_mem);

#endif /* __CL_MEM_H__ */

