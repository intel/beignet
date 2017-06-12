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

static void compiler_fill_large_image_2(void)
{
  const size_t w = 8191;
  const size_t h = 8192;
  const size_t origin[3] = {0, 0, 0};
  const size_t region[3] = {w, h, 1};
  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_copy_image");
  buf_data[0] = (unsigned char*) malloc(sizeof(unsigned char) * 8192 * 8192 * 4);
  buf_data[1] = (unsigned char*) malloc(sizeof(unsigned char) * 8192 * 8192 * 4);
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; i++)
      for (uint32_t k = 0; k < 4; k++)
        ((unsigned char*)buf_data[0])[(j * w + i) * 4 + k] = (unsigned char)rand();

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNORM_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = 0;
  OCL_CREATE_IMAGE(buf[0], 0, &format, &desc, NULL);
  OCL_WRITE_IMAGE(buf[0], origin, region, buf_data[0]);
  OCL_READ_IMAGE(buf[0], origin, region, buf_data[1]);

  // Check result
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; i++)
      for (uint32_t k = 0; k < 4; k++)
        OCL_ASSERT(((uint8_t*)buf_data[0])[(j * w + i) * 4 + k] ==
          ((uint8_t*)buf_data[1])[(j * w + i) * 4 + k]);

  free(buf_data[0]);
  free(buf_data[1]);
  buf_data[0] = NULL;
  buf_data[1] = NULL;
}

MAKE_UTEST_FROM_FUNCTION(compiler_fill_large_image_2);

static void compiler_fill_large_image_3(void)
{
  const size_t w = 8192;
  const size_t h = 8192;
  const size_t num_of_lines = 8;
  size_t origin[3] = {0, 0, 0};
  const size_t region[3] = {w, num_of_lines, 1};
  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  // Setup kernel and images
  buf_data[0] = (uint32_t*) malloc(sizeof(uint32_t) * w * num_of_lines * 4);
  buf_data[1] = (uint32_t*) malloc(sizeof(uint32_t) * w * h * 4);

  memset(buf_data[0], 0, sizeof(uint32_t) * w * num_of_lines * 4);
  memset(buf_data[1], 0, sizeof(uint32_t) * w * h * 4);

  for (uint32_t j = 0; j < num_of_lines; ++j)
    for (uint32_t i = 0; i < w; i++)
      for (uint32_t k = 0; k < 4; k++)
        ((uint32_t*)buf_data[0])[(j * w + i) * 4 + k] = (uint32_t)rand();

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT32;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = 0;
  OCL_CREATE_IMAGE(buf[0], 0, &format, &desc, NULL);
  OCL_WRITE_IMAGE(buf[0], origin, region, buf_data[0]);
  OCL_READ_IMAGE(buf[0], origin, region, buf_data[1]);

  // Check result
  for (uint32_t j = 0; j < num_of_lines; ++j)
    for (uint32_t i = 0; i < w; i++)
      for (uint32_t k = 0; k < 4; k++)
        OCL_ASSERT(((uint32_t*)buf_data[0])[(j * w + i) * 4 + k] ==
          ((uint32_t*)buf_data[1])[(j * w + i) * 4 + k]);

  free(buf_data[0]);
  free(buf_data[1]);
  buf_data[0] = NULL;
  buf_data[1] = NULL;
}

MAKE_UTEST_FROM_FUNCTION(compiler_fill_large_image_3);
