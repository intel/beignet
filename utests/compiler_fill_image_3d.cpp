#include <string.h>
#include "utest_helper.hpp"

static void compiler_fill_image_3d(void)
{
  const size_t w = 512;
  const size_t h = 512;
  const size_t depth = 5;
  uint32_t color = 0x12345678;
  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE3D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_depth = depth;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_fill_image_3d");

  OCL_CREATE_IMAGE(buf[0], 0, &format, &desc, NULL);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(color), &color);
  globals[0] = w;
  globals[1] = h;
  globals[2] = depth;
  locals[0] = 16;
  locals[1] = 16;
  locals[2] = 1;
  OCL_NDRANGE(3);

  // Check result
  OCL_MAP_BUFFER(0);
  for (uint32_t k = 0; k < depth; k++)
    for (uint32_t j = 0; j < h; ++j)
      for (uint32_t i = 0; i < w; i++)
        OCL_ASSERT(((uint32_t*)buf_data[0])[k*w*h + j*w + i] == 0x78563412);
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_fill_image_3d);
