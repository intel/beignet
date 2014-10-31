#include "utest_helper.hpp"
#include <cmath>

static int w = 0;
static int h = 0;
static int sz = 0;
static const size_t chunk = 64;
static int *src = NULL, *dst = NULL;

static void compiler_box_blur()
{
  OCL_CREATE_KERNEL("compiler_box_blur");

  /* Load the picture */
  src = cl_read_bmp("sample.bmp", &w, &h);
  sz = w * h * sizeof(int);

  /* Run the kernel */
  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, sz, src);
  OCL_CREATE_BUFFER(buf[1], 0, sz, NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(int), &w);
  OCL_SET_ARG(3, sizeof(int), &h);
  OCL_SET_ARG(4, sizeof(int), &chunk);
  globals[0] = size_t(w/4);
  globals[1] = h/chunk + ((h%chunk)?1:0);
  locals[0] = 16;
  locals[1] = 1;
  free(src);
  OCL_NDRANGE(2);
  OCL_MAP_BUFFER(1);
  dst = (int*) buf_data[1];

  /* Save the image (for debug purpose) */
  cl_write_bmp(dst, w, h, "compiler_box_blur.bmp");

  /* Compare with the golden image */
  OCL_CHECK_IMAGE(dst, w, h, "compiler_box_blur_ref.bmp");
}

MAKE_UTEST_FROM_FUNCTION(compiler_box_blur);

