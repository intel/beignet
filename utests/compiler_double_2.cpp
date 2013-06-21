#include <cmath>
#include "utest_helper.hpp"

static void cpu(int global_id, float *src, double *dst) {
  float f = src[global_id];
  float d = 1.234567890123456789;
  dst[global_id] = global_id < 14 ? d * (d + f) : 14;
}

void compiler_double_2(void)
{
  const size_t n = 16;
  float cpu_src[n];
  double cpu_dst[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_double_2");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(double), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  for (uint32_t pass = 0; pass < 1; ++pass) {
    OCL_MAP_BUFFER(0);
    for (int32_t i = 0; i < (int32_t) n; ++i)
      cpu_src[i] = ((float*)buf_data[0])[i] = .1f * (rand() & 15) - .75f;
    OCL_UNMAP_BUFFER(0);

    // Run the kernel on GPU
    OCL_NDRANGE(1);

    // Run on CPU
    for (int32_t i = 0; i < (int32_t) n; ++i)
      cpu(i, cpu_src, cpu_dst);

    // Compare
    OCL_MAP_BUFFER(1);
    for (int32_t i = 0; i < (int32_t) n; ++i)
      OCL_ASSERT(fabs(((double*)buf_data[1])[i] - cpu_dst[i]) < 1e-4);
    OCL_UNMAP_BUFFER(1);
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_double_2);
