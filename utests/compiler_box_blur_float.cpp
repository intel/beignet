#include "utest_helper.hpp"
#include <cmath>

static int *tmp = NULL;
static struct float4 {float x,y,z,w;} *src = NULL, *dst = NULL;
static int w = 0;
static int h = 0;
static int sz = 0;
static const size_t chunk = 64;

static void compiler_box_blur_float()
{
  OCL_CREATE_KERNEL("compiler_box_blur_float");

  /* Load the picture */
  tmp = cl_read_bmp("lenna128x128.bmp", &w, &h);
  sz = w * h * sizeof(float[4]);
  src = (float4*)malloc(sz);

  /* RGBA -> float4 conversion */
  const int n = w*h;
  for (int i = 0; i < n; ++i) {
    src[i].x = (float) (tmp[i] & 0xff);
    src[i].y = (float) ((tmp[i] >> 8) & 0xff);
    src[i].z = (float) ((tmp[i] >> 16) & 0xff);
    src[i].w = 0.f;
  }
  free(tmp);

  /* Run the kernel */
  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, sz, src);
  OCL_CREATE_BUFFER(buf[1], 0, sz, NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(int), &w);
  OCL_SET_ARG(3, sizeof(int), &h);
  OCL_SET_ARG(4, sizeof(int), &chunk);
  globals[0] = size_t(w);
  globals[1] = h/chunk + ((h%chunk)?1:0);
  locals[0] = 16;
  locals[1] = 1;
  free(src);
  OCL_NDRANGE(2);
  OCL_MAP_BUFFER(1);
  dst = (float4*) buf_data[1];

  /* Convert back to RGBA and save */
  int *tmp = (int*) malloc(n*sizeof(int));
  for (int i = 0; i < n; ++i) {
    int to = int(std::min(dst[i].x, 255.f));
    to |= int(std::min(dst[i].y, 255.f)) << 8;
    to |= int(std::min(dst[i].z, 255.f)) << 16;
    tmp[i] = to;
  }

  /* Save the image (for debug purpose) */
  cl_write_bmp(tmp, w, h, "compiler_box_blur_float.bmp");

  /* Compare with the golden image */
  OCL_CHECK_IMAGE(tmp, w, h, "compiler_box_blur_float_ref.bmp");
  free(tmp);
}

MAKE_UTEST_FROM_FUNCTION(compiler_box_blur_float);

