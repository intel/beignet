#include "utest_helper.hpp"
#include <cmath>

void compiler_sqrt_div(void) {
  const int n = 1024;
  float src[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_sqrt_div");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * 4 * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int j = 0; j < 1024; j++) {
    OCL_MAP_BUFFER(0);
    for (int i = 0; i < n; ++i) {
      src[i] = ((float *)buf_data[0])[i] = (j * n + i + 1) * 0.001f;
    }
    OCL_UNMAP_BUFFER(0);

    OCL_NDRANGE(1);

    OCL_MAP_BUFFER(1);
    float *dst = (float *)buf_data[1];
    for (int i = 0; i < n; ++i) {
      float cpu = 1.0f / sqrt(src[i]);
      float gpu = dst[4 * i];
      if (fabsf(cpu - gpu) >= 1e-3) {
        printf("%f %f %f", src[i], cpu, gpu);
        OCL_ASSERT(0);
      }

      cpu = i / sqrt(src[i]);
      gpu = dst[4 * i + 1];
      if (fabsf(cpu - gpu) >= 1e-3) {
        printf("%f %f %f", src[i], cpu, gpu);
        OCL_ASSERT(0);
      }

      cpu = 2.0f / sqrt(src[i]);
      gpu = dst[4 * i + 2];
      if (fabsf(cpu - gpu) >= 1e-3) {
        printf("%f %f %f", src[i], cpu, gpu);
        OCL_ASSERT(0);
      }

      cpu = 1.0f / sqrt(src[i]) + sqrt(src[i]);
      gpu = dst[4 * i + 3];
      if (fabsf(cpu - gpu) >= 1e-3) {
        printf("%f %f %f", src[i], cpu, gpu);
        OCL_ASSERT(0);
      }
    }
    OCL_UNMAP_BUFFER(1);
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_sqrt_div);
