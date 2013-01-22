#include "utest_helper.hpp"

static void cpu(int global_id, int *src, int *dst, int x) {
  dst[global_id] = src[global_id] % x;
}

void compiler_integer_remainder(void)
{
  const size_t n = 16;
  int cpu_dst[16], cpu_src[16];
  const int x = 7;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_integer_remainder");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(x), &x);
  globals[0] = 16;
  locals[0] = 16;

  // Run random tests
  for (uint32_t pass = 0; pass < 8; ++pass) {
    OCL_MAP_BUFFER(0);
    for (int32_t i = 0; i < (int32_t) n; ++i)
      cpu_src[i] = ((int32_t*)buf_data[0])[i] = rand() % 16;
    OCL_UNMAP_BUFFER(0);

    // Run the kernel on GPU
    OCL_NDRANGE(1);

    // Run on CPU
    for (int32_t i = 0; i <(int32_t) n; ++i) cpu(i, cpu_src, cpu_dst, x);

    // Compare
    OCL_MAP_BUFFER(1);
    for (int32_t i = 0; i < 11; ++i)
      OCL_ASSERT(((int32_t*)buf_data[1])[i] == cpu_dst[i]);
    OCL_UNMAP_BUFFER(1);
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_integer_remainder);
