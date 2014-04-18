#include "utest_helper.hpp"

void compiler_simd_all(void)
{
  const size_t n = 40;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_simd_all");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  globals[0] = n;
  locals[0] = 10;

  OCL_MAP_BUFFER(0);
  for (int32_t i = 0; i < (int32_t) n; ++i)
    ((int*)buf_data[0])[i] = i;
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Run on CPU

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%d %d\n", i, ((int *)buf_data[1])[i]);
    if (i % 2 == 1) {
      if (i < (int32_t)locals[0])
        OCL_ASSERT(((int *)buf_data[1])[i] == 1);
      else
        OCL_ASSERT(((int *)buf_data[1])[i] == 2);
    }
    else
      OCL_ASSERT(((int *)buf_data[1])[i] == 3);
  }
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_simd_all);
