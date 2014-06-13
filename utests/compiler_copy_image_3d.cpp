#include "utest_helper.hpp"
#include "string.h"

static void compiler_copy_image_3d(void)
{
  const size_t w = 512;
  const size_t h = 512;
  const size_t depth = 4;
  cl_image_format format;
  cl_image_desc desc;
  cl_sampler sampler;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_copy_image_3d");
  buf_data[0] = (uint32_t*) malloc(sizeof(uint32_t) * w * h * depth);
  for (uint32_t k = 0; k < depth; k++)
    for (uint32_t j = 0; j < h; j++)
      for (uint32_t i = 0; i < w; i++)
        ((float*)buf_data[0])[k*w*h + j*w + i] = (k << 10) + (j << 10) + i;

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNORM_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE3D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_depth = depth;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;

  OCL_CREATE_IMAGE(buf[0], CL_MEM_COPY_HOST_PTR, &format, &desc, buf_data[0]);
  OCL_CREATE_IMAGE(buf[1], 0, &format, &desc, NULL);
  memset(&desc, 0, sizeof(desc));
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_depth = 1;
  for(uint32_t i = 0; i < depth; i++)
   OCL_CREATE_IMAGE(buf[2 + i], 0, &format, &desc, NULL);

  OCL_CREATE_SAMPLER(sampler, CL_ADDRESS_REPEAT, CL_FILTER_NEAREST);
  free(buf_data[0]);
  buf_data[0] = NULL;

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(sampler), &sampler);
  for(uint32_t i = 0; i < depth; i++)
    OCL_SET_ARG(3 + i, sizeof(cl_mem), &buf[2 + i]);
  globals[0] = w;
  globals[1] = h;
  globals[2] = depth;
  locals[0] = 64;
  locals[1] = 1;
  locals[2] = 1;
  OCL_NDRANGE(3);

  // Check result
  for(uint32_t i = 0; i < depth + 2; i++)
    OCL_MAP_BUFFER_GTT(i);
  for (uint32_t k = 0; k < depth; k++)
    for (uint32_t j = 0; j < h; ++j)
      for (uint32_t i = 0; i < w; i++) {
        OCL_ASSERT(((float*)buf_data[0])[k*w*((h+1)&-2LL) + j*w + i] == ((float*)buf_data[1])[k*w*((h+1)&-2LL) + j*w + i]);
        OCL_ASSERT(((float*)buf_data[0])[k*w*((h+1)&-2LL) + j*w + i] == ((float*)buf_data[k + 2])[j * w + i]);
      }

  for(uint32_t i = 0; i < depth + 2; i++)
    OCL_UNMAP_BUFFER_GTT(i);

  OCL_CALL(clReleaseSampler, sampler);
}

MAKE_UTEST_FROM_FUNCTION(compiler_copy_image_3d);
