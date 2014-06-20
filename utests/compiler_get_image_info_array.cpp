#include <string.h>
#include "utest_helper.hpp"

static void compiler_get_image_info_array(void)
{
  const int w = 256;
  const int h = 512;
  const int array_size1 = 10;
  const int array_size2 = 3;
  cl_image_format format;
  cl_image_desc desc;

  // Create the 1D array buffer.
  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE1D_ARRAY;
  desc.image_width = w;
  desc.image_array_size = array_size1;
  OCL_CREATE_IMAGE(buf[0], 0, &format, &desc, NULL);

  // Create the 2D array buffer.
  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D_ARRAY;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_array_size = array_size2;
  OCL_CREATE_IMAGE(buf[1], 0, &format, &desc, NULL);

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_get_image_info_array");

  OCL_CREATE_BUFFER(buf[2], 0, 32 * sizeof(int), NULL);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = 32;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Check result
  OCL_MAP_BUFFER(2);
  OCL_ASSERT(((int*)buf_data[2])[0] == w);
  OCL_ASSERT(((int*)buf_data[2])[1] == array_size1);
  OCL_ASSERT(((int*)buf_data[2])[2] == CL_UNSIGNED_INT8);
  OCL_ASSERT(((int*)buf_data[2])[3] == CL_RGBA);

  OCL_ASSERT(((int*)buf_data[2])[4] == w);
  OCL_ASSERT(((int*)buf_data[2])[5] == h);
  OCL_ASSERT(((int*)buf_data[2])[6] == array_size2);
  OCL_ASSERT(((int*)buf_data[2])[7] == CL_UNSIGNED_INT8);
  OCL_ASSERT(((int*)buf_data[2])[8] == CL_RGBA);
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_get_image_info_array);
