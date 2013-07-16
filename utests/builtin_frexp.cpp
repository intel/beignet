#include <cmath>
#include "utest_helper.hpp"

void builtin_frexp(void)
{
  const int n = 32;
  float src[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("builtin_frexp");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = n;
  locals[0] = 16;

  OCL_MAP_BUFFER(0);
  src[0] = ((float*)buf_data[0])[0] = 0.f;
  src[1] = ((float*)buf_data[0])[1] = -0.f;
  src[2] = ((float*)buf_data[0])[2] = nanf("");
  src[3] = ((float*)buf_data[0])[3] = INFINITY;
  src[4] = ((float*)buf_data[0])[4] = -INFINITY;
  for (int i = 5; i < n; ++i)
    src[i] = ((float*)buf_data[0])[i] = (rand() & 255) * 0.1f - 12.8f;
  OCL_UNMAP_BUFFER(0);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  float *dst = (float*)buf_data[1];
  int *exp = (int*)buf_data[2];
  int w;
  OCL_ASSERT(dst[0] == 0.f && exp[0] == 0);
  OCL_ASSERT(dst[1] == -0.f && exp[1] == 0);
  OCL_ASSERT(isnanf(dst[2]));
  OCL_ASSERT(dst[3] == INFINITY);
  OCL_ASSERT(dst[4] == -INFINITY);
  for (int i = 5; i < n; ++i) {
    OCL_ASSERT(fabsf(dst[i] - frexpf(src[i], &w)) < 1e-5);
    OCL_ASSERT(exp[i] == w);
  }
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(builtin_frexp);
