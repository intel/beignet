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

#ifndef __CL_MEM_H__
#define __CL_MEM_H__

#include "cl_internals.h"
#include "cl_driver_type.h"
#include "CL/cl.h"
#include "cl_base_object.h"
#include <assert.h>
#include <pthread.h>
#if defined(HAS_GL_EGL)
#include "EGL/egl.h"
#endif

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

typedef struct _cl_mapped_ptr {
  void * ptr;
  void * v_ptr;
  size_t size;
  size_t origin[3];  /* mapped origin */
  size_t region[3];  /* mapped region */
}cl_mapped_ptr;

typedef struct _cl_mem_dstr_cb {
  list_node node; /* Mem callback list node */
  void(CL_CALLBACK *pfn_notify)(cl_mem memobj, void *user_data);
  void *user_data;
} _cl_mem_dstr_cb;
typedef _cl_mem_dstr_cb* cl_mem_dstr_cb;

/* Used for buffers and images */
enum cl_mem_type {
  CL_MEM_BUFFER_TYPE,
  CL_MEM_SUBBUFFER_TYPE,
  CL_MEM_PIPE_TYPE,
  CL_MEM_SVM_TYPE,
  CL_MEM_IMAGE_TYPE,
  CL_MEM_GL_IMAGE_TYPE,
  CL_MEM_BUFFER1D_IMAGE_TYPE
};
#define IS_IMAGE(mem) (mem->type >= CL_MEM_IMAGE_TYPE)
#define IS_GL_IMAGE(mem) (mem->type == CL_MEM_GL_IMAGE_TYPE)

typedef  struct _cl_mem {
  _cl_base_object base;
  enum cl_mem_type type;
  cl_buffer bo;             /* Data in GPU memory */
  size_t size;              /* original request size, not alignment size, used in constant buffer */
  cl_context ctx;           /* Context it belongs to */
  cl_mem_flags flags;       /* Flags specified at the creation time */
  void * host_ptr;          /* Pointer of the host mem specified by CL_MEM_ALLOC_HOST_PTR, CL_MEM_USE_HOST_PTR */
  cl_mapped_ptr* mapped_ptr;/* Store the mapped addresses and size by caller. */
  int mapped_ptr_sz;        /* The array size of mapped_ptr. */
  int map_ref;              /* The mapped count. */
  uint8_t mapped_gtt;       /* This object has mapped gtt, for unmap. */
  list_head dstr_cb_head;   /* All destroy callbacks. */
  uint8_t is_userptr;       /* CL_MEM_USE_HOST_PTR is enabled */
  cl_bool is_svm;           /* This object  is svm */
  size_t offset;            /* offset of host_ptr to the page beginning, only for CL_MEM_USE_HOST_PTR*/

  uint8_t cmrt_mem_type;    /* CmBuffer, CmSurface2D, ... */
  void* cmrt_mem;
} _cl_mem;

#define CL_OBJECT_MEM_MAGIC 0x381a27b9ee6504dfLL
#define CL_OBJECT_IS_MEM(obj) ((obj &&                           \
         ((cl_base_object)obj)->magic == CL_OBJECT_MEM_MAGIC &&  \
         CL_OBJECT_GET_REF(obj) >= 1))
#define CL_OBJECT_IS_IMAGE(mem) ((mem &&                           \
         ((cl_base_object)mem)->magic == CL_OBJECT_MEM_MAGIC &&    \
         CL_OBJECT_GET_REF(mem) >= 1 &&                            \
         mem->type >= CL_MEM_IMAGE_TYPE))
#define CL_OBJECT_IS_BUFFER(mem) ((mem &&                          \
         ((cl_base_object)mem)->magic == CL_OBJECT_MEM_MAGIC &&    \
         CL_OBJECT_GET_REF(mem) >= 1 &&                            \
         mem->type < CL_MEM_IMAGE_TYPE))

typedef struct _cl_mem_pipe {
  _cl_mem base;
  cl_svm_mem_flags flags;                 /* Flags specified at the creation time */
  uint32_t packet_size;
  uint32_t max_packets;
} _cl_mem_pipe;

typedef struct _cl_mem_svm {
  _cl_mem base;
  cl_svm_mem_flags flags;                 /* Flags specified at the creation time */
} _cl_mem_svm;

struct _cl_mem_image {
  _cl_mem base;
  cl_image_format fmt;            /* only for images */
  uint32_t intel_fmt;             /* format to provide in the surface state */
  size_t bpp;                     /* number of bytes per pixel */
  cl_mem_object_type image_type;  /* only for images 1D/2D...*/
  size_t w, h, depth;             /* only for images (depth is only for 3D images) */
  size_t row_pitch, slice_pitch;
  size_t host_row_pitch, host_slice_pitch;
  cl_image_tiling_t tiling;       /* only IVB+ supports TILE_[X,Y] (image only) */
  size_t tile_x, tile_y;          /* tile offset, used for mipmap images.  */
  size_t offset;                  /* offset for dri_bo, used when it's reloc. */
  cl_mem buffer_1d;               /* if the image is created from buffer, it point to the buffer.*/
  uint8_t is_image_from_buffer;       /* IMAGE from Buffer*/
};

struct _cl_mem_gl_image {
  struct _cl_mem_image base;
  int fd;
#if defined(HAS_GL_EGL)
  EGLImage egl_image;
#endif
};

struct _cl_mem_buffer1d_image {
  struct _cl_mem_image base;
  uint32_t size;
  _cl_mem * descbuffer;
};

#define IS_1D_IMAGE(image) (image->image_type == CL_MEM_OBJECT_IMAGE1D ||        \
                                image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY ||  \
                                image->image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER)

#define IS_2D_IMAGE(image) (image->image_type == CL_MEM_OBJECT_IMAGE2D ||        \
                                image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)

#define IS_3D_IMAGE(image) (image->image_type == CL_MEM_OBJECT_IMAGE3D)

#define IS_IMAGE_ARRAY(image) (image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY || \
                                   image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)

inline static void
cl_mem_image_init(struct _cl_mem_image *image, size_t w, size_t h,
                  cl_mem_object_type image_type,
                  size_t depth, cl_image_format fmt,
                  uint32_t intel_fmt, uint32_t bpp,
                  size_t row_pitch, size_t slice_pitch,
                  cl_image_tiling_t tiling,
                  size_t tile_x, size_t tile_y,
                  size_t offset)
{
  image->w = w;
  image->h = h;
  image->image_type = image_type;
  image->depth = depth;
  image->fmt = fmt;
  image->intel_fmt = intel_fmt;
  image->bpp = bpp;
  image->row_pitch = row_pitch;
  image->slice_pitch = slice_pitch;
  image->tiling = tiling;
  image->tile_x = tile_x;
  image->tile_y = tile_y;
  image->offset = offset;
}

struct _cl_mem_buffer {
  _cl_mem base;
  struct _cl_mem_buffer* subs;         /* Sub buf objects. */
  size_t sub_offset;                   /* The sub start offset. */
  struct _cl_mem_buffer* sub_prev, *sub_next;/* We chain the sub memory buffers together */
  pthread_mutex_t sub_lock;            /* Sub buffers list lock*/
  struct _cl_mem_buffer* parent;       /* Point to the parent buffer if is sub-buffer */
};

inline static struct _cl_mem_image *
cl_mem_image(cl_mem mem)
{
  assert(IS_IMAGE(mem));
  return (struct _cl_mem_image *)mem;
}

inline static struct _cl_mem_gl_image *
cl_mem_gl_image(cl_mem mem)
{
  assert(IS_GL_IMAGE(mem));
  return (struct _cl_mem_gl_image*)mem;
}

inline static struct _cl_mem_pipe *
cl_mem_pipe(cl_mem mem)
{
  assert(mem->type == CL_MEM_PIPE_TYPE);
  return (struct _cl_mem_pipe *)mem;
}

/* Query information about a memory object */
extern cl_mem_object_type cl_get_mem_object_type(cl_mem mem);

/* Query whether mem is in buffers */
extern cl_int cl_mem_is_valid(cl_mem mem, cl_context ctx);

/* Create a new memory object and initialize it with possible user data */
extern cl_mem cl_mem_new_buffer(cl_context, cl_mem_flags, size_t, void*, cl_int*);

/* Create a new sub memory object */
extern cl_mem cl_mem_new_sub_buffer(cl_mem, cl_mem_flags, cl_buffer_create_type, const void *, cl_int *);

extern cl_mem cl_mem_new_pipe(cl_context, cl_mem_flags, cl_uint, cl_uint, cl_int *);
/* Query information about a pipe object */
extern cl_int cl_get_pipe_info(cl_mem, cl_mem_info, size_t, void *, size_t *);

void* cl_mem_svm_allocate(cl_context, cl_svm_mem_flags, size_t, unsigned int);
void cl_mem_svm_delete(cl_context, void *svm_pointer);

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
extern void cl_mem_gl_delete(struct _cl_mem_gl_image *);

/* Add one more reference to this object */
extern void cl_mem_add_ref(cl_mem);

/* api clEnqueueCopyBuffer help function */
extern cl_int cl_mem_copy(cl_command_queue queue, cl_event event, cl_mem src_buf, cl_mem dst_buf,
              size_t src_offset, size_t dst_offset, size_t cb);

extern cl_int cl_mem_fill(cl_command_queue queue, cl_event e, const void * pattern, size_t pattern_size,
              cl_mem buffer, size_t offset, size_t size);

extern cl_int cl_image_fill(cl_command_queue queue, cl_event e, const void * pattern, struct _cl_mem_image*,
                                    const size_t *, const size_t *);

/* api clEnqueueCopyBufferRect help function */
extern cl_int cl_mem_copy_buffer_rect(cl_command_queue, cl_event event, cl_mem, cl_mem,
                                     const size_t *, const size_t *, const size_t *,
                                     size_t, size_t, size_t, size_t);

/* api clEnqueueCopyImage help function */
extern cl_int cl_mem_kernel_copy_image(cl_command_queue, cl_event event, struct _cl_mem_image*,
                                       struct _cl_mem_image*, const size_t *, const size_t *, const size_t *);

/* api clEnqueueCopyImageToBuffer help function */
extern cl_int cl_mem_copy_image_to_buffer(cl_command_queue, cl_event, struct _cl_mem_image*, cl_mem,
                                          const size_t *, const size_t, const size_t *);

/* api clEnqueueCopyBufferToImage help function */
extern cl_int cl_mem_copy_buffer_to_image(cl_command_queue, cl_event, cl_mem, struct _cl_mem_image*,
                                          const size_t, const size_t *, const size_t *);

/* Directly map a memory object */
extern void *cl_mem_map(cl_mem, int);

/* Unmap a memory object */
extern cl_int cl_mem_unmap(cl_mem);

/* Directly map a memory object in GTT mode */
extern void *cl_mem_map_gtt(cl_mem);

/* Directly map a memory object in GTT mode, with out waiting gpu idle */
extern void *cl_mem_map_gtt_unsync(cl_mem);

/* Unmap a memory object in GTT mode */
extern cl_int cl_mem_unmap_gtt(cl_mem);

/* Directly map a memory object - tiled images are mapped in GTT mode */
extern void *cl_mem_map_auto(cl_mem, int);

/* Unmap a memory object - tiled images are unmapped in GTT mode */
extern cl_int cl_mem_unmap_auto(cl_mem);

/* Pin/unpin the buffer in memory (you must be root) */
extern cl_int cl_mem_pin(cl_mem);
extern cl_int cl_mem_unpin(cl_mem);

extern cl_mem
cl_mem_allocate(enum cl_mem_type type,
                cl_context ctx,
                cl_mem_flags flags,
                size_t sz,
                cl_int is_tiled,
                void *host_ptr,
                cl_mem buffer,
                cl_int *errcode);

void
cl_mem_copy_image_region(const size_t *origin, const size_t *region,
                         void *dst, size_t dst_row_pitch, size_t dst_slice_pitch,
                         const void *src, size_t src_row_pitch, size_t src_slice_pitch,
                         const struct _cl_mem_image *image, cl_bool offset_dst, cl_bool offset_src);

void
cl_mem_copy_image_to_image(const size_t *dst_origin,const size_t *src_origin, const size_t *region,
                           const struct _cl_mem_image *dst_image, const struct _cl_mem_image *src_image);

extern cl_mem cl_mem_new_libva_buffer(cl_context ctx,
                                      unsigned int bo_name,
                                      cl_int *errcode);

extern cl_mem cl_mem_new_libva_image(cl_context ctx,
                                     unsigned int bo_name, size_t offset,
                                     size_t width, size_t height,
                                     cl_image_format fmt,
                                     size_t row_pitch,
                                     cl_int *errcode);

extern cl_int cl_mem_get_fd(cl_mem mem, int* fd);

extern cl_mem cl_mem_new_buffer_from_fd(cl_context ctx,
                                        int fd,
                                        int buffer_sz,
                                        cl_int* errcode);

extern cl_mem cl_mem_new_image_from_fd(cl_context ctx,
                                       int fd, int image_sz,
                                       size_t offset,
                                       size_t width, size_t height,
                                       cl_image_format fmt,
                                       size_t row_pitch,
                                       cl_int *errcode);

extern cl_int cl_mem_record_map_mem(cl_mem mem, void *ptr, void **mem_ptr, size_t offset,
                      size_t size, const size_t *origin, const size_t *region);

extern cl_int cl_mem_set_destructor_callback(cl_mem memobj,
                      void(CL_CALLBACK *pfn_notify)(cl_mem, void *), void *user_data);
#endif /* __CL_MEM_H__ */

