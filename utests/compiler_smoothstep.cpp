#include <cmath>
#include "utest_helper.hpp"

float cpu(float e0, float e1, float x)
{
  x = (x - e0) / (e1 - e0);
  if (x >= 1)
    x = 1.f;
  if (x <= 0)
    x = 0.f;
  return x * x * (3 - 2 * x);
}

void compiler_smoothstep(void)
{
  const int n = 32;
  float src1[n], src2[n], src3[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_smoothstep");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[3], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);
  globals[0] = n;
  locals[0] = 16;

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  for (int i = 0; i < n; ++i) {
    float a = 0.1f * (rand() & 15) - 0.75f;
    float b = a + 0.1f * (rand() & 15) + 0.1f;
    float c = 0.1f * (rand() & 15) - 0.75f;
    src1[i] = ((float*)buf_data[0])[i] = a;
    src2[i] = ((float*)buf_data[1])[i] = b;
    src3[i] = ((float*)buf_data[2])[i] = c;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(3);
  for (int i = 0; i < n; ++i) {
    float a = ((float*)buf_data[3])[i];
    float b = cpu(src1[i], src2[i], src3[i]);
    OCL_ASSERT(fabsf(a - b) < 1e-4f);
  }
  OCL_UNMAP_BUFFER(3);
}

MAKE_UTEST_FROM_FUNCTION(compiler_smoothstep);
