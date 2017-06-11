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
 */

#include "cl_gen.h"

#define DECL_INTERNAL_KERN(NAME)          \
  extern char cl_internal_##NAME##_str[]; \
  extern size_t cl_internal_##NAME##_str_size;

DECL_INTERNAL_KERN(block_motion_estimate_intel)
DECL_INTERNAL_KERN(copy_buf_align16)
DECL_INTERNAL_KERN(copy_buf_align4)
DECL_INTERNAL_KERN(copy_buffer_to_image_2d_align16)
DECL_INTERNAL_KERN(copy_buffer_to_image_2d)
DECL_INTERNAL_KERN(copy_buffer_to_image_3d)
DECL_INTERNAL_KERN(copy_buf_rect_align4)
DECL_INTERNAL_KERN(copy_buf_rect)
DECL_INTERNAL_KERN(copy_buf_unalign_dst_offset)
DECL_INTERNAL_KERN(copy_buf_unalign_same_offset)
DECL_INTERNAL_KERN(copy_buf_unalign_src_offset)
DECL_INTERNAL_KERN(copy_image_1d_array_to_1d_array)
DECL_INTERNAL_KERN(copy_image_1d_to_1d)
DECL_INTERNAL_KERN(copy_image_2d_array_to_2d_array)
DECL_INTERNAL_KERN(copy_image_2d_array_to_2d)
DECL_INTERNAL_KERN(copy_image_2d_array_to_3d)
DECL_INTERNAL_KERN(copy_image_2d_to_2d_array)
DECL_INTERNAL_KERN(copy_image_2d_to_2d)
DECL_INTERNAL_KERN(copy_image_2d_to_3d)
DECL_INTERNAL_KERN(copy_image_2d_to_buffer_align16)
DECL_INTERNAL_KERN(copy_image_2d_to_buffer)
DECL_INTERNAL_KERN(copy_image_3d_to_2d_array)
DECL_INTERNAL_KERN(copy_image_3d_to_2d)
DECL_INTERNAL_KERN(copy_image_3d_to_3d)
DECL_INTERNAL_KERN(copy_image_3d_to_buffer)
DECL_INTERNAL_KERN(fill_buf_align128)
DECL_INTERNAL_KERN(fill_buf_align2)
DECL_INTERNAL_KERN(fill_buf_align4)
DECL_INTERNAL_KERN(fill_buf_align8)
DECL_INTERNAL_KERN(fill_buf_unalign)
DECL_INTERNAL_KERN(fill_image_1d_array)
DECL_INTERNAL_KERN(fill_image_1d)
DECL_INTERNAL_KERN(fill_image_2d_array)
DECL_INTERNAL_KERN(fill_image_2d)
DECL_INTERNAL_KERN(fill_image_3d)

#define REF_INTERNAL_KERN(NAME) (cl_internal_##NAME##_str), &(cl_internal_##NAME##_str_size)

static struct {
  cl_int index;
  void *program_binary;
  size_t *size;
  char *kernel_name;
} gen_internals_kernels[] = {
  {CL_ENQUEUE_COPY_BUFFER_ALIGN4, REF_INTERNAL_KERN(copy_buf_align4), "__cl_copy_region_align4"},
  {CL_ENQUEUE_COPY_BUFFER_ALIGN16, REF_INTERNAL_KERN(copy_buf_align16), "__cl_copy_region_align16"},
  {CL_ENQUEUE_COPY_BUFFER_UNALIGN_SAME_OFFSET, REF_INTERNAL_KERN(copy_buf_unalign_same_offset), "__cl_copy_region_unalign_same_offset"},
  {CL_ENQUEUE_COPY_BUFFER_UNALIGN_DST_OFFSET, REF_INTERNAL_KERN(copy_buf_unalign_dst_offset), "__cl_copy_region_unalign_dst_offset"},
  {CL_ENQUEUE_COPY_BUFFER_UNALIGN_SRC_OFFSET, REF_INTERNAL_KERN(copy_buf_unalign_src_offset), "__cl_copy_region_unalign_src_offset"},
  {CL_ENQUEUE_COPY_BUFFER_RECT, REF_INTERNAL_KERN(copy_buf_rect), "__cl_copy_buffer_rect"},
  {CL_ENQUEUE_COPY_BUFFER_RECT_ALIGN4, REF_INTERNAL_KERN(copy_buf_rect_align4), "__cl_copy_buffer_rect_align4"},
  {CL_ENQUEUE_COPY_IMAGE_1D_TO_1D, REF_INTERNAL_KERN(copy_image_1d_to_1d), "__cl_copy_image_1d_to_1d"},
  {CL_ENQUEUE_COPY_IMAGE_2D_TO_2D, REF_INTERNAL_KERN(copy_image_2d_to_2d), "__cl_copy_image_2d_to_2d"},
  {CL_ENQUEUE_COPY_IMAGE_3D_TO_2D, REF_INTERNAL_KERN(copy_image_3d_to_2d), "__cl_copy_image_3d_to_2d"},
  {CL_ENQUEUE_COPY_IMAGE_2D_TO_3D, REF_INTERNAL_KERN(copy_image_2d_to_3d), "__cl_copy_image_2d_to_3d"},
  {CL_ENQUEUE_COPY_IMAGE_3D_TO_3D, REF_INTERNAL_KERN(copy_image_3d_to_3d), "__cl_copy_image_3d_to_3d"},
  {CL_ENQUEUE_COPY_IMAGE_2D_TO_2D_ARRAY, REF_INTERNAL_KERN(copy_image_2d_to_2d_array), "__cl_copy_image_2d_to_2d_array"},
  {CL_ENQUEUE_COPY_IMAGE_1D_ARRAY_TO_1D_ARRAY, REF_INTERNAL_KERN(copy_image_1d_array_to_1d_array), "__cl_copy_image_1d_array_to_1d_array"},
  {CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_2D_ARRAY, REF_INTERNAL_KERN(copy_image_2d_array_to_2d_array), "__cl_copy_image_2d_array_to_2d_array"},
  {CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_2D, REF_INTERNAL_KERN(copy_image_2d_array_to_2d), "__cl_copy_image_2d_array_to_2d"},
  {CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_3D, REF_INTERNAL_KERN(copy_image_2d_array_to_3d), "__cl_copy_image_2d_array_to_3d"},
  {CL_ENQUEUE_COPY_IMAGE_3D_TO_2D_ARRAY, REF_INTERNAL_KERN(copy_image_3d_to_2d_array), "__cl_copy_image_3d_to_2d_array"},
  {CL_ENQUEUE_COPY_IMAGE_2D_TO_BUFFER, REF_INTERNAL_KERN(copy_image_2d_to_buffer), "__cl_copy_image_2d_to_buffer"},
  {CL_ENQUEUE_COPY_IMAGE_2D_TO_BUFFER_ALIGN16, REF_INTERNAL_KERN(copy_image_2d_to_buffer_align16), "__cl_copy_image_2d_to_buffer_align16"},
  {CL_ENQUEUE_COPY_IMAGE_3D_TO_BUFFER, REF_INTERNAL_KERN(copy_image_3d_to_buffer), "__cl_copy_image_3d_to_buffer"},
  {CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_2D, REF_INTERNAL_KERN(copy_buffer_to_image_2d), "__cl_copy_buffer_to_image_2d"},
  {CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_2D_ALIGN16, REF_INTERNAL_KERN(copy_buffer_to_image_2d_align16), "__cl_copy_buffer_to_image_2d_align16"},
  {CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_3D, REF_INTERNAL_KERN(copy_buffer_to_image_3d), "__cl_copy_buffer_to_image_3d"},
  {CL_ENQUEUE_FILL_BUFFER_UNALIGN, REF_INTERNAL_KERN(fill_buf_unalign), "__cl_fill_region_unalign"},
  {CL_ENQUEUE_FILL_BUFFER_ALIGN2, REF_INTERNAL_KERN(fill_buf_align2), "__cl_fill_region_align2"},
  {CL_ENQUEUE_FILL_BUFFER_ALIGN4, REF_INTERNAL_KERN(fill_buf_align4), "__cl_fill_region_align4"},
  {CL_ENQUEUE_FILL_BUFFER_ALIGN8_8, REF_INTERNAL_KERN(fill_buf_align8), "__cl_fill_region_align8_2"},
  {CL_ENQUEUE_FILL_BUFFER_ALIGN8_16, REF_INTERNAL_KERN(fill_buf_align8), "__cl_fill_region_align8_4"},
  {CL_ENQUEUE_FILL_BUFFER_ALIGN8_32, REF_INTERNAL_KERN(fill_buf_align8), "__cl_fill_region_align8_8"},
  {CL_ENQUEUE_FILL_BUFFER_ALIGN8_64, REF_INTERNAL_KERN(fill_buf_align8), "__cl_fill_region_align8_16"},
  {CL_ENQUEUE_FILL_BUFFER_ALIGN128, REF_INTERNAL_KERN(fill_buf_align128), "__cl_fill_region_align128"},
  {CL_ENQUEUE_FILL_IMAGE_1D, REF_INTERNAL_KERN(fill_image_1d), "__cl_fill_image_1d"},
  {CL_ENQUEUE_FILL_IMAGE_1D_ARRAY, REF_INTERNAL_KERN(fill_image_1d_array), "__cl_fill_image_1d_array"},
  {CL_ENQUEUE_FILL_IMAGE_2D, REF_INTERNAL_KERN(fill_image_2d), "__cl_fill_image_2d"},
  {CL_ENQUEUE_FILL_IMAGE_2D_ARRAY, REF_INTERNAL_KERN(fill_image_2d_array), "__cl_fill_image_2d_array"},
  {CL_ENQUEUE_FILL_IMAGE_3D, REF_INTERNAL_KERN(fill_image_3d), "__cl_fill_image_3d"},
};

LOCAL cl_int
cl_context_create_gen(cl_device_id device, cl_context ctx)
{
  cl_context_gen ctx_gen = CL_CALLOC(1, sizeof(_cl_context_gen));
  if (ctx_gen == NULL)
    return CL_OUT_OF_HOST_MEMORY;

  ctx_gen->ctx_base.device = device;
  ctx_gen->drv = intel_driver_create(&ctx->props);
  if (ctx_gen->drv == NULL) {
    CL_FREE(ctx_gen);
    return CL_OUT_OF_RESOURCES;
  }

  ctx_gen->ver = ctx_gen->drv->gen_ver;
  intel_driver_set_atomic_flag(ctx_gen->drv, device->atomic_test_result);
  ASSIGN_DEV_PRIVATE_DATA(ctx, device, (cl_context_for_device)ctx_gen);

  return CL_SUCCESS;
}

LOCAL void
cl_context_delete_gen(cl_device_id device, cl_context ctx)
{
  cl_context_gen ctx_gen = NULL;
  DEV_PRIVATE_DATA(ctx, device, ctx_gen);

  intel_driver_delete(ctx_gen->drv);
  ctx_gen->drv = NULL;
  CL_FREE(ctx_gen);
}

LOCAL cl_kernel
cl_context_get_builtin_kernel_gen(cl_context ctx, cl_device_id device, cl_int index)
{
  cl_device_id_gen dev_gen = (cl_device_id_gen)device;
  cl_int binary_status = CL_SUCCESS;
  cl_int err = CL_SUCCESS;
  cl_program prog = NULL;
  cl_kernel ker = NULL;

  assert(index >= 0 && index < CL_INTERNAL_KERNEL_MAX);

  CL_OBJECT_LOCK(device);
  if (dev_gen->internal_program[index] == NULL) {
    assert(dev_gen->internal_kernels[index] == NULL);
  } else {
    prog = dev_gen->internal_program[index];
    ker = dev_gen->internal_kernels[index];
    assert(ker);
  }
  CL_OBJECT_UNLOCK(device);

  if (ker)
    return ker;

  prog = cl_program_create_from_binary(ctx, 1, &device, gen_internals_kernels[index].size,
                                       (const unsigned char **)&gen_internals_kernels[index].program_binary,
                                       &binary_status, &err);
  assert(err == CL_SUCCESS);
  err = cl_program_build(prog, NULL, 1, &device);
  assert(err == CL_SUCCESS);
  cl_program_take_out_of_context(prog);
  ker = cl_kernel_create(prog, gen_internals_kernels[index].kernel_name, &err);
  assert(err == CL_SUCCESS);

  /* Cache the build result to device */
  CL_OBJECT_LOCK(device);
  if (dev_gen->internal_program[index] == NULL) {
    dev_gen->internal_program[index] = prog;
    dev_gen->internal_kernels[index] = ker;
  } else { // Someone already do it ?
    cl_kernel_delete(ker);
    cl_program_delete(prog);
    ker = dev_gen->internal_kernels[index];
    assert(ker);
  }
  CL_OBJECT_UNLOCK(device);

  return ker;
}
