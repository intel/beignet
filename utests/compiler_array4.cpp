#include "utest_helper.hpp"

static void cpu(int global_id, int *src, int *dst) {
  dst[global_id * 4] = src[global_id * 4];
}

void compiler_array4(void)
{
  const size_t n = 16;
  int cpu_dst[64], cpu_src[64];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_array4");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t) * 4, NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t) * 4, NULL);
  uint32_t offset = 1;
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(offset), &offset);
  globals[0] = 16;
  locals[0] = 16;

  // Run random tests
  for (uint32_t pass = 0; pass < 8; ++pass) {
    OCL_MAP_BUFFER(0);
    for (int32_t i = 0; i < (int32_t) n; ++i)
      cpu_src[i * 4] = ((int32_t*)buf_data[0])[i * 4] = rand() % 16;
    OCL_UNMAP_BUFFER(0);

    // Run the kernel on GPU
    OCL_NDRANGE(1);

    // Run on CPU
    for (int32_t i = 0; i <(int32_t) n; ++i) cpu(i, cpu_src, cpu_dst);

    // Compare
    OCL_MAP_BUFFER(1);
    for (int32_t i = 0; i < 11; ++i) {
      OCL_ASSERT(((int32_t*)buf_data[1])[i * 4] == cpu_dst[i * 4]);
    }
    OCL_UNMAP_BUFFER(1);
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_array4);
