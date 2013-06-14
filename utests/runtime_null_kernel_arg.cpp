#include "utest_helper.hpp"

void runtime_null_kernel_arg(void)
{
  const size_t n = 32;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("null_kernel_arg");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), NULL);
  OCL_SET_ARG(2, sizeof(cl_mem), NULL);

    // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);

  // Check results
  for (uint32_t i = 0; i < n; ++i)
    OCL_ASSERT(((uint32_t*)buf_data[0])[i] == i);
  OCL_UNMAP_BUFFER(0);
}


MAKE_UTEST_FROM_FUNCTION(runtime_null_kernel_arg);
