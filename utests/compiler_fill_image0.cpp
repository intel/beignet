#include <string.h>
#include "utest_helper.hpp"

static void compiler_fill_image0(void)
{
  const size_t w = 512;
  const size_t h = 512;
  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = 0;

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_fill_image0");

  OCL_CREATE_IMAGE(buf[0], 0, &format, &desc, NULL);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  globals[0] = w;
  globals[1] = h;
  locals[0] = 16;
  locals[1] = 16;
  OCL_NDRANGE(2);

  // Check result
  OCL_MAP_BUFFER_GTT(0);
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; i++)
      OCL_ASSERT(((uint32_t*)buf_data[0])[j * w + i] == (i << 16 | j));
  OCL_UNMAP_BUFFER_GTT(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_fill_image0);
