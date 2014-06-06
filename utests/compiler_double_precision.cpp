#include "utest_helper.hpp"
#include <math.h>

static void double_precision_check(void)
{
  const size_t n = 16; //8192 * 4;

  double d0 = 0.12345678912345678;
  double d1 = 0.12355678922345678;
  float cpu_result = d1 - d0;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("double_precision_check");
  //OCL_CREATE_KERNEL("compiler_array");
  buf_data[0] = (uint32_t*) malloc(sizeof(uint32_t) * n);
  for (uint32_t i = 0; i < n; ++i) ((float*)buf_data[0])[i] = 0;
  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, n * sizeof(uint32_t), buf_data[0]);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  free(buf_data[0]);
  buf_data[0] = NULL;

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Check result
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  bool precisionOK = true;
  for (uint32_t i = 0; i < n; ++i) {
    float error = ((float*)buf_data[1])[i] - cpu_result;
    if (error != 0)
      precisionOK = false;
    OCL_ASSERT((fabs(error) < 1e-4));
  }
  if (!precisionOK)
    printf("\n  - WARN: GPU doesn't have correct double precision. Got %.7G, expected %.7G\n", ((float*)buf_data[1])[0], cpu_result);
}

MAKE_UTEST_FROM_FUNCTION(double_precision_check);
