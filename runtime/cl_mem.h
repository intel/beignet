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

#include "cl_base_object.h"
#include "CL/cl.h"
#include <assert.h>
#include <pthread.h>
#if defined(HAS_GL_EGL)
#include "EGL/egl.h"
#endif

#ifndef CL_VERSION_1_2
#define CL_MEM_OBJECT_IMAGE1D 0x10F4
#define CL_MEM_OBJECT_IMAGE1D_ARRAY 0x10F5
#define CL_MEM_OBJECT_IMAGE1D_BUFFER 0x10F6
#define CL_MEM_OBJECT_IMAGE2D_ARRAY 0x10F3
typedef struct _cl_image_desc {
  cl_mem_object_type image_type;
  size_t image_width;
  size_t image_height;
  size_t image_depth;
  size_t image_array_size;
  size_t image_row_pitch;
  size_t image_slice_pitch;
  cl_uint num_mip_levels;
  cl_uint num_samples;
  cl_mem buffer;
} cl_image_desc;
#endif

typedef struct _cl_mem_dstr_cb {
  list_node node; /* Mem callback list node */
  void(CL_CALLBACK *pfn_notify)(cl_mem memobj, void *user_data);
  void *user_data;
} _cl_mem_dstr_cb;
typedef _cl_mem_dstr_cb *cl_mem_dstr_cb;

/* All possible memory object type for buffers and images */
typedef enum cl_mem_type {
  CL_MEM_BUFFER_TYPE,
  CL_MEM_SUBBUFFER_TYPE,
  CL_MEM_PIPE_TYPE,
  CL_MEM_SVM_TYPE,
  CL_MEM_IMAGE_TYPE,
  CL_MEM_GL_IMAGE_TYPE,
} cl_mem_type;

typedef struct _cl_mem_for_device {
  cl_device_id device; /* Point to the device it belong to */
} _cl_mem_for_device;
typedef _cl_mem_for_device *cl_mem_for_device;

typedef struct _cl_mem_map_info {
  list_node node;
  void *map_ptr; /* The ptr return by API */
  union {
    struct {
      size_t offset;
      size_t size;
    } buffer;
    struct {
      size_t origin[3]; /* mapped origin */
      size_t region[3]; /* mapped region */
      size_t row_pitch;
      size_t slice_pitch;
    } image;
  };
} _cl_mem_map_info;
typedef _cl_mem_map_info *cl_mem_map_info;

typedef struct _cl_mem {
  _cl_base_object base;
  cl_mem_type type;
  cl_mem_flags flags;             /* Flags specified at the creation time */
  size_t size;                    /* Request size when create*/
  cl_context ctx;                 /* Context it belongs to */
  cl_mem_for_device *each_device; /* Content interpreted by device */
  cl_uint each_device_num;        /* Each device number */
  list_head dstr_cb_head;         /* All destroy callbacks */
  list_head mapped_ptr_head;      /* All mapped ptr records */
  cl_int in_enqueue_use;          /* Set when the mem is using, e.g. ndrang, mapping */
  atomic_t map_ref;               /* Mapped times */
  void *host_ptr;                 /* Only valid for CL_MEM_USE_HOST_PTR */
} _cl_mem;

#define CL_OBJECT_MEM_MAGIC 0x381a27b9ee6504dfLL

typedef struct _cl_mem_buffer {
  _cl_mem base;
  list_head sub_buffers;         /* All sub buffer list */
  list_node sub_node;            /* Sub node link to its parent */
  cl_uint sub_buffer_num;        /* All sub buffer num */
  size_t sub_offset;             /* The sub start offset. */
  cl_svm_mem_flags svm_flags;    /* Flags copied from SVM object */
  struct _cl_mem_buffer *parent; /* Point to the parent buffer if is sub-buffer */
  cl_mem svm_buf;                /* Is created based on svm pointer */
  size_t svm_offset;             /* Offset from return address of svmAlloc */
} _cl_mem_buffer;
typedef _cl_mem_buffer *cl_mem_buffer;

typedef struct _cl_mem_image {
  _cl_mem base;
  cl_image_format fmt;           /* only for images */
  size_t bpp;                    /* number of bytes per pixel */
  cl_mem_object_type image_type; /* only for images 1D/2D...*/
  size_t w, h, depth;            /* only for images (depth is only for 3D images) */
  size_t row_pitch, slice_pitch;
  cl_mem mem_from; /* We created from this buffer */
  cl_bool is_nv12; /* Is from nv12 format image */
} _cl_mem_image;
typedef _cl_mem_image *cl_mem_image;

typedef struct _cl_mem_gl_image {
  struct _cl_mem_image base;
  int fd;
#if defined(HAS_GL_EGL)
  EGLImage egl_image;
#endif
} _cl_mem_gl_image;
typedef _cl_mem_gl_image *cl_mem_gl_image;

typedef struct _cl_mem_svm {
  _cl_mem base;
  cl_svm_mem_flags flags; /* Flags specified at the creation time */
  size_t real_size;
} _cl_mem_svm;
typedef _cl_mem_svm *cl_mem_svm;

typedef struct _cl_mem_pipe {
  _cl_mem base;
  cl_uint packet_size;
  cl_uint max_packets;
} _cl_mem_pipe;
typedef _cl_mem_pipe *cl_mem_pipe;

#define CL_OBJECT_IS_MEM(obj)                              \
  ((obj &&                                                 \
    ((cl_base_object)obj)->magic == CL_OBJECT_MEM_MAGIC && \
    CL_OBJECT_GET_REF(obj) >= 1))
#define CL_OBJECT_IS_IMAGE(mem) \
  (CL_OBJECT_IS_MEM(mem) && ((cl_mem)mem)->type >= CL_MEM_IMAGE_TYPE)
#define CL_OBJECT_IS_GL_IMAGE(mem) \
  (CL_OBJECT_IS_MEM(mem) && ((cl_mem)mem)->type == CL_MEM_GL_IMAGE_TYPE)
#define CL_OBJECT_IS_BUFFER(mem) \
  (CL_OBJECT_IS_MEM(mem) &&      \
   (((cl_mem)mem)->type == CL_MEM_BUFFER_TYPE || ((cl_mem)mem)->type == CL_MEM_SUBBUFFER_TYPE))
#define CL_OBJECT_IS_SUB_BUFFER(mem) \
  (CL_OBJECT_IS_MEM(mem) && ((cl_mem)mem)->type == CL_MEM_SUBBUFFER_TYPE)
#define CL_OBJECT_IS_BUFFER_SVM(mem) \
  (CL_OBJECT_IS_BUFFER(mem) && (((cl_mem)mem)->flag & CL_MEM_USES_SVM_POINTER))
#define CL_OBJECT_IS_SVM(mem) \
  (CL_OBJECT_IS_MEM(mem) && ((cl_mem)mem)->type == CL_MEM_SVM_TYPE)
#define CL_OBJECT_IS_PIPE(mem) \
  (CL_OBJECT_IS_MEM(mem) && ((cl_mem)mem)->type == CL_MEM_PIPE_TYPE)

#define CL_OBJECT_IS_1D_IMAGE(image)                                      \
  (CL_OBJECT_IS_IMAGE(image) &&                                           \
   (((cl_mem_image)(image))->image_type == CL_MEM_OBJECT_IMAGE1D ||       \
    ((cl_mem_image)(image))->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY || \
    ((cl_mem_image)(image))->image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER))
#define CL_OBJECT_IS_2D_IMAGE(image)                                \
  (CL_OBJECT_IS_IMAGE(image) &&                                     \
   (((cl_mem_image)(image))->image_type == CL_MEM_OBJECT_IMAGE2D || \
    ((cl_mem_image)(image))->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY))
#define CL_OBJECT_IS_3D_IMAGE(image) \
  (CL_OBJECT_IS_IMAGE(image) &&      \
   (((cl_mem_image)(image))->image_type == CL_MEM_OBJECT_IMAGE3D))
#define CL_OBJECT_IS_IMAGE_ARRAY(image)                                   \
  (CL_OBJECT_IS_IMAGE(image) &&                                           \
   (((cl_mem_image)(image))->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY || \
    ((cl_mem_image)(image))->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY))
#define CL_OBJECT_IS_IMAGE_FROM_BUFFER(image) \
  (CL_OBJECT_IS_IMAGE(image) && (((cl_mem_image)(image))->buffer_from != NULL))

inline static cl_mem_image cl_mem_to_image(cl_mem mem)
{
  assert(CL_OBJECT_IS_IMAGE(mem));
  return (struct _cl_mem_image *)mem;
}
inline static cl_mem_gl_image cl_mem_to_gl_image(cl_mem mem)
{
  assert(CL_OBJECT_IS_GL_IMAGE(mem));
  return (struct _cl_mem_gl_image *)mem;
}
inline static cl_mem_buffer cl_mem_to_buffer(cl_mem mem)
{
  assert(CL_OBJECT_IS_BUFFER(mem));
  return (struct _cl_mem_buffer *)mem;
}
inline static cl_mem_svm cl_mem_to_svm(cl_mem mem)
{
  assert(CL_OBJECT_IS_SVM(mem));
  return (struct _cl_mem_svm *)mem;
}
inline static cl_mem_pipe cl_mem_to_pipe(cl_mem mem)
{
  assert(CL_OBJECT_IS_PIPE(mem));
  return (struct _cl_mem_pipe *)mem;
}

extern cl_mem_object_type cl_mem_get_object_type(cl_mem mem);
/* Query whether mem is in buffers */
extern cl_int cl_mem_is_valid(cl_mem mem, cl_context ctx);
extern cl_mem cl_mem_create_buffer(cl_context, cl_mem_flags, size_t, void *, cl_int *);
extern cl_mem cl_mem_create_sub_buffer(cl_mem, cl_mem_flags, cl_buffer_create_type, const void *, cl_int *);
extern cl_mem cl_mem_create_image(cl_context context, cl_mem_flags flags, const cl_image_format *image_format,
                                  const cl_image_desc *image_desc, void *host_ptr, cl_int *errcode_ret);
extern cl_mem cl_mem_create_pipe(cl_context ctx, cl_mem_flags flags, cl_uint pipe_packet_size,
                                 cl_uint pipe_max_packets, cl_int *errcode_ret);
/* Add one more reference to this object */
extern void cl_mem_add_ref(cl_mem);
/* Unref the object and delete it if no more reference */
extern void cl_mem_delete(cl_mem);
extern cl_int cl_mem_set_destructor_callback(cl_mem memobj,
                                             void(CL_CALLBACK *pfn_notify)(cl_mem, void *), void *user_data);
extern cl_int cl_mem_assure_allocated(cl_device_id device, cl_mem mem);
/* Destroy egl image. */
extern void cl_mem_gl_delete(struct _cl_mem_gl_image *);
extern void *cl_mem_svm_allocate(cl_context ctx, cl_svm_mem_flags flags, size_t size, unsigned int alignment);
extern void cl_mem_svm_delete(cl_context ctx, cl_mem mem_svm);
extern void cl_svm_free_delete_func(cl_event event);
extern cl_int cl_enqueue_handle_map_mem(cl_event event, cl_int status);
extern cl_int cl_enqueue_handle_unmap_mem(cl_event event, cl_int status);
extern cl_int cl_enqueue_handle_read_write_mem(cl_event event, cl_int status);
extern cl_int cl_enqueue_handle_copy_mem(cl_event event, cl_int status);
extern cl_int cl_enqueue_handle_fill_mem(cl_event event, cl_int status);
extern cl_int cl_enqueue_handle_svm_free(cl_event event, cl_int status);
extern cl_int cl_enqueue_handle_svm_map_unmap(cl_event event, cl_int status);
extern cl_int cl_enqueue_handle_svm_copy(cl_event event, cl_int status);
extern cl_int cl_enqueue_handle_svm_fill(cl_event event, cl_int status);
extern void cl_mem_copy_image_region_helper(const size_t *origin, const size_t *region,
                                            void *dst, size_t dst_row_pitch, size_t dst_slice_pitch,
                                            const void *src, size_t src_row_pitch, size_t src_slice_pitch,
                                            size_t bpp, size_t image_w, size_t image_h,
                                            cl_bool offset_dst, cl_bool offset_src);
#endif /* __CL_MEM_H__ */
