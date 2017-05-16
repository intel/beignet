#include <string.h>
#include "utest_helper.hpp"

static void compiler_copy_large_image(void)
{
  const size_t w = 4096;
  const size_t h = 4096;
  cl_image_format format;
  cl_image_desc desc;
  cl_sampler sampler;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_copy_image");
  buf_data[0] = (uint32_t*) malloc(sizeof(uint32_t) * w * h * 4);
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; i++)
      for (uint32_t k = 0; k < 4; k++)
        ((uint32_t*)buf_data[0])[(j * w + i) * 4 + k] = k;

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT32;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = w * sizeof(uint32_t) * 4;
  OCL_CREATE_IMAGE(buf[0], CL_MEM_COPY_HOST_PTR, &format, &desc, buf_data[0]);

  desc.image_row_pitch = 0;
  OCL_CREATE_IMAGE(buf[1], 0, &format, &desc, NULL);
  OCL_CREATE_SAMPLER(sampler, CL_ADDRESS_REPEAT, CL_FILTER_NEAREST);
  free(buf_data[0]);
  buf_data[0] = NULL;

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(sampler), &sampler);
  globals[0] = w;
  globals[1] = h;
  locals[0] = 16;
  locals[1] = 16;
  OCL_NDRANGE(2);

  // Check result
  OCL_MAP_BUFFER_GTT(0);
  OCL_MAP_BUFFER_GTT(1);
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; i++)
      for (uint32_t k = 0; k < 4; k++)
        OCL_ASSERT(((uint32_t*)buf_data[0])[(j * w + i) * 4 + k] ==
          ((uint32_t*)buf_data[1])[(j * w + i) * 4 + k]);
  OCL_UNMAP_BUFFER_GTT(0);
  OCL_UNMAP_BUFFER_GTT(1);

  OCL_CALL(clReleaseSampler, sampler);
}

MAKE_UTEST_FROM_FUNCTION(compiler_copy_large_image);

static void compiler_copy_large_image_1(void)
{
  const size_t w = 4096;
  const size_t h = 4096;
  const size_t origin[3] = {0, 0, 0};
  const size_t region[3] = {w, h, 1};
  cl_image_format format;
  cl_image_desc desc;
  cl_sampler sampler;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_copy_image");
  buf_data[0] = (uint32_t*) malloc(sizeof(uint32_t) * w * h * 4);
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; i++)
      for (uint32_t k = 0; k < 4; k++)
        ((uint32_t*)buf_data[0])[(j * w + i) * 4 + k] = k;

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT32;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = 0;
  OCL_CREATE_IMAGE(buf[0], 0, &format, &desc, NULL);
  OCL_CREATE_IMAGE(buf[1], 0, &format, &desc, NULL);
  OCL_CREATE_SAMPLER(sampler, CL_ADDRESS_REPEAT, CL_FILTER_NEAREST);
  OCL_WRITE_IMAGE(buf[0], origin, region, buf_data[0]);
  free(buf_data[0]);
  buf_data[0] = NULL;

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(sampler), &sampler);
  globals[0] = w;
  globals[1] = h;
  locals[0] = 16;
  locals[1] = 16;
  OCL_NDRANGE(2);

  // Check result
  OCL_MAP_BUFFER_GTT(0);
  OCL_MAP_BUFFER_GTT(1);
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; i++)
      for (uint32_t k = 0; k < 4; k++)
        OCL_ASSERT(((uint32_t*)buf_data[0])[(j * w + i) * 4 + k] ==
          ((uint32_t*)buf_data[1])[(j * w + i) * 4 + k]);
  OCL_UNMAP_BUFFER_GTT(0);
  OCL_UNMAP_BUFFER_GTT(1);

  OCL_CALL(clReleaseSampler, sampler);
}

MAKE_UTEST_FROM_FUNCTION(compiler_copy_large_image_1);
