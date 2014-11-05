#include <cmath>
#include "utest_helper.hpp"
#include <string.h>

void builtin_tgamma(void)
{
  const int n = 1024;
  float src[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("builtin_tgamma");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;
  const char* env_strict = getenv("OCL_STRICT_CONFORMANCE");
  float ULPSIZE_FACTOR = 1.0;
  if (env_strict == NULL || strcmp(env_strict, "0") == 0)
    ULPSIZE_FACTOR = 10000.;

  for (int j = 0; j < 1024; j ++) {
    OCL_MAP_BUFFER(0);
    for (int i = 0; i < n; ++i) {
      src[i] = ((float*)buf_data[0])[i] = (j*n+i+1) * 0.001f;
    }
    OCL_UNMAP_BUFFER(0);

    OCL_NDRANGE(1);

    OCL_MAP_BUFFER(1);
    float *dst = (float*)buf_data[1];
    for (int i = 0; i < n; ++i) {
      float cpu = tgammaf(src[i]);
      if (isinf(cpu)) {
        OCL_ASSERT(isinf(dst[i]));
      } else if (fabsf(cpu - dst[i]) >= cl_FLT_ULP(cpu) * ULPSIZE_FACTOR) {
        printf("%f %f %f\n", src[i], cpu, dst[i]);
        OCL_ASSERT(0);
      }
    }
    OCL_UNMAP_BUFFER(1);
  }
}

MAKE_UTEST_FROM_FUNCTION(builtin_tgamma);
