#include <string.h>
#include "utest_helper.hpp"

static void compiler_fill_large_image(void)
{
  const size_t w = 4096;
  const size_t h = 4096;
  const size_t origin[3] = {0, 0, 0};
  const size_t region[3] = {w, h, 1};
  uint32_t color = 0x12345678;
  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT32;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = 0;

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_fill_image");

  OCL_CREATE_IMAGE(buf[0], 0, &format, &desc, NULL);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(color), &color);
  globals[0] = w;
  globals[1] = h;
  locals[0] = 16;
  locals[1] = 16;
  OCL_NDRANGE(2);

  // Check result
  uint32_t *data = (uint32_t*)malloc(sizeof(uint32_t) * w * h * 4);
  OCL_READ_IMAGE(buf[0], origin, region, data);
  for (uint32_t j = 0; j < h; ++j) {
    for (uint32_t i = 0; i < w; i++) {
      uint32_t k = (j * w + i) * 4;
      OCL_ASSERT(data[k] == 0x12);
      OCL_ASSERT(data[k + 1] == 0x34);
      OCL_ASSERT(data[k + 2] == 0x56);
      OCL_ASSERT(data[k + 3] == 0x78);
    }
  }
  free(data);

  OCL_MAP_BUFFER_GTT(0);
  for (uint32_t j = 0; j < h; ++j) {
    for (uint32_t i = 0; i < w; i++) {
      uint32_t k = (j * w + i) * 4;
      OCL_ASSERT(((uint32_t*)buf_data[0])[k] == 0x12);
      OCL_ASSERT(((uint32_t*)buf_data[0])[k + 1] == 0x34);
      OCL_ASSERT(((uint32_t*)buf_data[0])[k + 2] == 0x56);
      OCL_ASSERT(((uint32_t*)buf_data[0])[k + 3] == 0x78);
    }
  }
  OCL_UNMAP_BUFFER_GTT(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_fill_large_image);

static void compiler_fill_large_image_1(void)
{
  const size_t w = 4096;
  const size_t h = 4096;
  const size_t origin[3] = {0, 0, 0};
  const size_t region[3] = {w, h, 1};
  uint32_t color[4] = {0x12, 0x34, 0x56, 0x78};
  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT32;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = 0;

  // Setup kernel and images
  OCL_CREATE_IMAGE(buf[0], 0, &format, &desc, NULL);

  // Fill Image
  clEnqueueFillImage(queue, buf[0], color, origin, region, 0, NULL, NULL);

  // Check result
  uint32_t *data = (uint32_t*)malloc(sizeof(uint32_t) * w * h * 4);
  OCL_READ_IMAGE(buf[0], origin, region, data);
  for (uint32_t j = 0; j < h; ++j) {
    for (uint32_t i = 0; i < w; i++) {
      int k = (j * w + i) * 4;
      OCL_ASSERT(data[k] == 0x12);
      OCL_ASSERT(data[k + 1] == 0x34);
      OCL_ASSERT(data[k + 2] == 0x56);
      OCL_ASSERT(data[k + 3] == 0x78);
    }
  }
  free(data);

  OCL_MAP_BUFFER_GTT(0);
  for (uint32_t j = 0; j < h; ++j) {
    for (uint32_t i = 0; i < w; i++) {
      int k = (j * w + i) * 4;
      OCL_ASSERT(((uint32_t*)buf_data[0])[k] == 0x12);
      OCL_ASSERT(((uint32_t*)buf_data[0])[k + 1] == 0x34);
      OCL_ASSERT(((uint32_t*)buf_data[0])[k + 2] == 0x56);
      OCL_ASSERT(((uint32_t*)buf_data[0])[k + 3] == 0x78);
    }
  }
  OCL_UNMAP_BUFFER_GTT(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_fill_large_image_1);
