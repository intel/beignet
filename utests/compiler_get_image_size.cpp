#include "utest_helper.hpp"

static void compiler_get_image_size(void)
{
  const size_t w = 256;
  const size_t h = 512;
  cl_image_format format;
  cl_image_desc desc;

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = 0;

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_get_image_size");

  OCL_CREATE_IMAGE(buf[0], 0, &format, &desc, NULL);
  OCL_CREATE_BUFFER(buf[1], 0, 32 * sizeof(int), NULL);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = 32;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Check result
  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < 32; i++)
    OCL_ASSERT(((uint32_t*)buf_data[1])[i] == ((w << 16) | (h)));
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_get_image_size);
