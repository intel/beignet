#include <string.h>
#include "utest_helper.hpp"

static void compiler_fill_image_2d_array(void)
{
  const size_t w = 64;
  const size_t h = 16;
  const size_t array = 8;
  cl_image_format format;
  cl_image_desc desc;
  size_t origin[3] = { };
  size_t region[3];
  uint32_t* dst;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D_ARRAY;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = 0;//w * sizeof(uint32_t);
  desc.image_array_size = array;

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_fill_image_2d_array");

  OCL_CREATE_IMAGE(buf[0], 0, &format, &desc, NULL);

  OCL_MAP_BUFFER_GTT(0);
  memset(buf_data[0], 0, sizeof(uint32_t) * w * h * array);
  OCL_UNMAP_BUFFER_GTT(0);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  globals[0] = w/2;
  locals[0] = 16;
  globals[1] = h;
  locals[1] = 8;
  globals[2] = array;
  locals[2] = 8;
  OCL_NDRANGE(3);

  // Check result
  region[0] = w;
  region[1] = h;
  region[2] = array;
  dst = (uint32_t*)malloc(w*h*array*sizeof(uint32_t));
  OCL_READ_IMAGE(buf[0], origin, region, dst);

#if 0
  printf("------ The image result is: -------\n");
  for (uint32_t k = 0; k < array; k++) {
    for (uint32_t j = 0; j < h; j++) {
      for (uint32_t i = 0; i < w; i++) {
        printf(" %2x", dst[k*h*w + j*w + i]);
      }
      printf("\n");
    }
    printf("\n");
  }
#endif

  for (uint32_t k = 0; k < array - 1; k++) {
    for (uint32_t j = 0; j < h; j++) {
      for (uint32_t i = 0; i < w/2; i++) {
        OCL_ASSERT(dst[k*w*h + j*w + i] == 0x03020100);
      }
      for (uint32_t i = w/2; i < w; i++) {
        OCL_ASSERT(dst[k*w*h + j*w + i] == 0);
      }
    }
  }

  for (uint32_t j = 0; j < h; j++) {
    for (uint32_t i = 0; i < w; i++) {
      OCL_ASSERT(dst[(array - 1)*w*h + j*w + i] == 0x0);
    }
  }
  free(dst);
}

MAKE_UTEST_FROM_FUNCTION(compiler_fill_image_2d_array);
