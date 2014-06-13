#include <string.h>
#include "utest_helper.hpp"

static void compiler_fill_image_1d(void)
{
  const size_t w = 2048;
  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE1D;
  desc.image_width = w;
  desc.image_row_pitch = 0;

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_fill_image_1d");

  OCL_CREATE_IMAGE(buf[0], 0, &format, &desc, NULL);

  OCL_MAP_BUFFER_GTT(0);
  for (uint32_t i = 0; i < w; i++) {
      ((uint32_t*)buf_data[0])[i] = 0;
  }
  OCL_UNMAP_BUFFER_GTT(0);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  globals[0] = w/2;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Check result
  OCL_MAP_BUFFER_GTT(0);
  //printf("------ The image result is: -------\n");
  for (uint32_t i = 0; i < w/2; i++) {
      //printf(" %2x", ((uint32_t *)buf_data[0])[i]);
      OCL_ASSERT(((uint32_t*)buf_data[0])[i] == 0x03020100);
  }
  for (uint32_t i = w/2; i < w; i++) {
      //printf(" %2x", ((uint32_t *)buf_data[0])[i]);
      OCL_ASSERT(((uint32_t*)buf_data[0])[i] == 0);
  }
  OCL_UNMAP_BUFFER_GTT(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_fill_image_1d);
