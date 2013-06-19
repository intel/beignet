#include "utest_helper.hpp"

static void compiler_get_image_info(void)
{
  const size_t w = 256;
  const size_t h = 512;
  const size_t depth = 3;
  cl_image_format format;

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT8;

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_get_image_info");

  OCL_CREATE_IMAGE3D(buf[0], 0, &format, w, h, depth, 0, 0, NULL);
  OCL_CREATE_BUFFER(buf[1], 0, 32 * sizeof(int), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, 32 * sizeof(int), NULL);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = 32;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Check result
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  for (uint32_t i = 0; i < 32; i++)
  {
    OCL_ASSERT(((uint32_t*)buf_data[1])[i] == ((w << 20) | (h << 8) | depth));
    OCL_ASSERT(((uint32_t*)buf_data[2])[i] == ((CL_UNSIGNED_INT8 << 16) | CL_RGBA));
  }
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_get_image_info);
