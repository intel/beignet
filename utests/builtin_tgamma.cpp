#include <cmath>
#include "utest_helper.hpp"
#include <string.h>

void builtin_tgamma(void)
{
  const int n = 1024;
  float src[n];
  float ULPSIZE_NO_FAST_MATH = 16.0;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("builtin_tgamma");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;
  float ULPSIZE_FACTOR = select_ulpsize(ULPSIZE_FAST_MATH,ULPSIZE_NO_FAST_MATH);

  cl_device_fp_config fp_config;
  clGetDeviceInfo(device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(cl_device_fp_config), &fp_config, 0);
  bool denormals_supported = fp_config & CL_FP_DENORM;
  float max_ulp = 0, max_ulp_at = 0;

  for (int j = 0; j < 128; j ++) {
    OCL_MAP_BUFFER(0);
    for (int i = 0; i < n; ++i) {
      src[i] = ((float*)buf_data[0])[i] = j - 64 + i*0.001f;
    }
    OCL_UNMAP_BUFFER(0);

    OCL_NDRANGE(1);

    OCL_MAP_BUFFER(1);
    float *dst = (float*)buf_data[1];
    for (int i = 0; i < n; ++i) {
      float cpu = tgamma(src[i]);
      if (!denormals_supported && std::fpclassify(cpu)==FP_SUBNORMAL && dst[i]==0) {
        cpu = 0;
      }
      if (fabsf(cpu - dst[i]) > cl_FLT_ULP(cpu) * max_ulp) {
        max_ulp = fabsf(cpu - dst[i]) / cl_FLT_ULP(cpu);
        max_ulp_at = src[i];
      }
      if (std::isinf(cpu)) {
        OCL_ASSERT(std::isinf(dst[i]));
      } else if (fabsf(cpu - dst[i]) >= cl_FLT_ULP(cpu) * ULPSIZE_FACTOR) {
        printf("%f %f %f", src[i], cpu, dst[i]);
        OCL_ASSERT(0);
      }
    }
    OCL_UNMAP_BUFFER(1);
  }
  printf("max error=%f ulp at x=%f ", max_ulp, max_ulp_at);
}

MAKE_UTEST_FROM_FUNCTION(builtin_tgamma);
