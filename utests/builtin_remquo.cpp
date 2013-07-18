#include <cmath>
#include <cstring>
#include "utest_helper.hpp"

void builtin_remquo(void)
{
  const int n = 16;
  float src1[n], src2[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("builtin_remquo");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[3], 0, n * sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);
  globals[0] = n;
  locals[0] = 16;

  src1[0] = 1,         src2[0] = 0;
  src1[1] = 1,         src2[1] = -0.f;
  src1[2] = INFINITY,  src2[2] = 1;
  src1[3] = -INFINITY, src2[3] = 1;
  src1[4] = nanf(""),  src2[4] = nanf("");
  src1[5] = 1.625f,    src2[5] = 1;
  src1[6] = -1.625f,   src2[6] = 1;
  src1[7] = 1.625f,    src2[7] = -1;
  src1[8] = -1.625f,   src2[8] = -1;
  src1[9] = 5,         src2[9] = 2;
  src1[10] = 3,        src2[10] = 2;
  src1[11] = -0.f,     src2[11] = 1;

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], src1, n * sizeof(float));
  memcpy(buf_data[1], src2, n * sizeof(float));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(2);
  OCL_MAP_BUFFER(3);
  float *dest = (float *)buf_data[2];
  int *quo = (int *)buf_data[3];
  OCL_ASSERT(isnanf(dest[0]));
  OCL_ASSERT(isnanf(dest[1]));
  OCL_ASSERT(isnanf(dest[2]));
  OCL_ASSERT(isnanf(dest[3]));
  OCL_ASSERT(isnanf(dest[4]));
  OCL_ASSERT(dest[5] == -0.375f && quo[5] ==  2);
  OCL_ASSERT(dest[6] ==  0.375f && quo[6] == -2);
  OCL_ASSERT(dest[7] == -0.375f && quo[7] == -2);
  OCL_ASSERT(dest[8] ==  0.375f && quo[8] ==  2);
  OCL_ASSERT(dest[9] == 1       && quo[9] ==  2);
  OCL_ASSERT(dest[10] == -1     && quo[10] == 2);
  OCL_ASSERT(dest[11] == -0.f   && quo[11] == 0);
  OCL_UNMAP_BUFFER(2);
  OCL_UNMAP_BUFFER(3);
}

MAKE_UTEST_FROM_FUNCTION(builtin_remquo);
