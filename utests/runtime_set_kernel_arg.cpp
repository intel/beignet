#include "utest_helper.hpp"

void runtime_set_kernel_arg(void)
{
  const size_t n = 16;

  cl_float3 src;
  src.s[0] = 1; src.s[1] =2; src.s[2] = 3;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("set_kernel_arg");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_float3), &src);

    // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);

  // Check results
  for (uint32_t i = 0; i < n; ++i) {
//    printf("%d %d\n",i, ((uint32_t*)buf_data[0])[i]);
    OCL_ASSERT(((uint32_t*)buf_data[0])[i] == src.s[i%3]);
  }
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(runtime_set_kernel_arg);
