#include <cmath>
#include "utest_helper.hpp"

void compiler_double_4(void)
{
  const size_t n = 16;
  double cpu_src1[n], cpu_src2[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_double_4");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(double), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    cpu_src1[i] = ((double*)buf_data[0])[i] = rand() * 1e-2;
    cpu_src2[i] = ((double*)buf_data[1])[i] = rand() * 1e-2;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i)
    OCL_ASSERT(fabs(((double*)buf_data[2])[i] - cpu_src1[i] - cpu_src2[i]) < 1e-4);
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_double_4);
