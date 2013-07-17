#include <cmath>
#include <cstring>
#include "utest_helper.hpp"

void builtin_modf(void)
{
  const int n = 32;
  float src[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("builtin_modf");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = n;
  locals[0] = 16;

  src[0] = INFINITY;
  src[1] = -INFINITY;
  src[2] = nanf("");
  src[3] = 0;
  src[4] = 1.5f;
  src[5] = 2.5f;
  src[6] = -2.5f;
  src[7] = 20;
  src[8] = 21;
  src[9] = 89.5f;

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, n * sizeof(float));
  OCL_UNMAP_BUFFER(0);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  float *dst = (float *)buf_data[1];
  float *it = (float *)buf_data[2];
  OCL_ASSERT(dst[0] == 0 && it[0] == INFINITY);
  OCL_ASSERT(dst[1] == -0.f && it[1] == -INFINITY);
  OCL_ASSERT(isnanf(dst[2]) && isnanf(it[2]));
  OCL_ASSERT(dst[3] == 0 && it[3] == 0);
  OCL_ASSERT(dst[4] == 0.5f && it[4] == 1);
  OCL_ASSERT(dst[5] == 0.5f && it[5] == 2);
  OCL_ASSERT(dst[6] == -0.5f && it[6] == -2);
  OCL_ASSERT(dst[7] == 0 && it[7] == 20);
  OCL_ASSERT(dst[8] == 0 && it[8] == 21);
  OCL_ASSERT(dst[9] == 0.5f && it[9] == 89);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(builtin_modf);
