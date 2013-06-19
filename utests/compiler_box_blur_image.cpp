#include "utest_helper.hpp"

static void compiler_box_blur_image()
{
  int w, h;
  cl_image_format format = { };
  size_t origin[3] = { };
  size_t region[3];
  int *src, *dst;

  OCL_CREATE_KERNEL("compiler_box_blur_image");

  /* Load the picture */
  src = cl_read_bmp("lenna128x128.bmp", &w, &h);

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNORM_INT8;

  /* Run the kernel */
  OCL_CREATE_IMAGE2D(buf[0], CL_MEM_COPY_HOST_PTR, &format, w, h, w*sizeof(uint32_t), src);
  free(src);
  OCL_CREATE_IMAGE2D(buf[1], 0, &format, w, h, 0, NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = w;
  globals[1] = h;
  locals[0] = 16;
  locals[1] = 16;
  OCL_NDRANGE(2);
  dst = (int*)malloc(w*h*sizeof(uint32_t));
  region[0] = w;
  region[1] = h;
  region[2] = 1;
  OCL_READ_IMAGE(buf[1], origin, region, dst);

  /* Save the image (for debug purpose) */
  cl_write_bmp(dst, w, h, "compiler_box_blur_image.bmp");

  /* Compare with the golden image */
  OCL_CHECK_IMAGE(dst, w, h, "compiler_box_blur_ref.bmp");

  free(dst);
}

MAKE_UTEST_FROM_FUNCTION(compiler_box_blur_image);
