#include <string.h>
#include "utest_helper.hpp"

static void compare_image_2d_and_1d_array(void)
{
  const int w = 64;
  const int h = 32;
  cl_image_format format;
  cl_image_desc desc;
  cl_sampler sampler;

  // Create the 1D array buffer.
  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  uint32_t* image_data1 = (uint32_t *)malloc(w * h * sizeof(uint32_t));
  uint32_t* image_data2 = (uint32_t *)malloc(w * h * sizeof(uint32_t));
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      char a = 0;
      if (j % 2 == 0)
        a = (j + 3) & 0x3f;

      image_data2[w * j + i] = image_data1[w * j + i] = a << 24 | a << 16 | a << 8 | a;
    }
  }

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = w * sizeof(uint32_t);
  OCL_CREATE_IMAGE(buf[0], CL_MEM_COPY_HOST_PTR, &format, &desc, image_data1);

  // Create the 2D array buffer.
  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE1D_ARRAY;
  desc.image_width = w;
  desc.image_array_size = h;
  desc.image_row_pitch = w * sizeof(uint32_t);
  OCL_CREATE_IMAGE(buf[1], CL_MEM_COPY_HOST_PTR, &format, &desc, image_data2);

  OCL_CREATE_SAMPLER(sampler, CL_ADDRESS_REPEAT, CL_FILTER_LINEAR);

  // Setup kernel and images
  OCL_CREATE_KERNEL("compare_image_2d_and_1d_array");

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_sampler), &sampler);
  globals[0] = 32;
  globals[1] = 16;
  locals[0] = 32;
  locals[1] = 16;
  OCL_NDRANGE(2);

  OCL_MAP_BUFFER_GTT(0);
  OCL_MAP_BUFFER_GTT(1);
  for (int j = 0; j < h; ++j) {
    for (int i = 0; i < w; i++) {
      // Because the array index will not join the sample caculation, the result should
      // be different between the 2D and 1D_array.
      if (j % 2 == 0)
        OCL_ASSERT(((uint32_t*)buf_data[0])[j * w + i] == ((uint32_t*)buf_data[1])[j * w + i]);
    }
  }
  OCL_UNMAP_BUFFER_GTT(0);
  OCL_UNMAP_BUFFER_GTT(1);

  OCL_CALL(clReleaseSampler, sampler);
}

MAKE_UTEST_FROM_FUNCTION(compare_image_2d_and_1d_array);
