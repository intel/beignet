#include <cmath>
#include <cstring>
#include "utest_helper.hpp"

static int as_int(float f) {
  void *p = &f;
  return *(int *)p;
}

void builtin_nextafter(void)
{
  const int n = 16;
  float src1[n], src2[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("builtin_nextafter");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = n;
  locals[0] = 16;

  src1[0] = nanf(""), src2[0] = 1.1f;
  src1[1] = 2.2f,     src2[1] = nanf("");
  src1[2] = nanf(""), src2[2] = nanf("");
  src1[3] = 123.4f,   src2[3] = 123.4f;
  src1[4] = 0.f,      src2[4] = 1.f;
  src1[5] = -0.f,     src2[5] = -1.f;
  for (int i = 6; i < n; ++i) {
    src1[i] = (rand() & 255) * 0.1f - 12.8f;
    src2[i] = (rand() & 255) * 0.1f - 12.8f;
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], src1, n * sizeof(float));
  memcpy(buf_data[1], src2, n * sizeof(float));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(2);
  float *dest = (float *)buf_data[2];
  if (0)
    for (int i = 0; i < n; ++i)
      printf("%d %x %x %x %x\n", i, as_int(src1[i]), as_int(src2[i]),
             as_int(dest[i]), as_int(nextafterf(src1[i], src2[i])));
  OCL_ASSERT(isnanf(dest[0]));
  OCL_ASSERT(isnanf(dest[1]));
  OCL_ASSERT(isnanf(dest[2]));
  for (int i = 3; i < n; ++i)
    OCL_ASSERT(dest[i] == nextafterf(src1[i], src2[i]));
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(builtin_nextafter);
