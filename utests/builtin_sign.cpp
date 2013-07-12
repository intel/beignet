#include <cmath>
#include "utest_helper.hpp"

void builtin_sign(void)
{
  const int n = 32;
  float src[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("builtin_sign");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  OCL_MAP_BUFFER(0);
  src[0] = ((float*)buf_data[0])[0] = nanf("");
  src[1] = ((float*)buf_data[0])[1] = INFINITY;
  src[2] = ((float*)buf_data[0])[2] = 0.f;
  src[3] = ((float*)buf_data[0])[3] = -0.f;
  for (int i = 4; i < n; ++i) {
    src[i] = ((float*)buf_data[0])[i] = (rand() & 15) * 0.1 - 0.75;
  }
  OCL_UNMAP_BUFFER(0);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(1);
  float *dst = (float*)buf_data[1];
  OCL_ASSERT(dst[0] == 0);
  OCL_ASSERT(dst[1] == 1.f);
  OCL_ASSERT(dst[2] == 0.f);
  OCL_ASSERT(dst[3] == -0.f);
  for (int i = 4; i < n; ++i) {
    if (src[i] == 0.f)
      OCL_ASSERT(dst[i] == 0.f);
    else if (src[i] == -0.f)
      OCL_ASSERT(dst[i] == -0.f);
    else
      OCL_ASSERT(dst[i] == (src[i] > 0 ? 1 : -1));
  }
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(builtin_sign);
