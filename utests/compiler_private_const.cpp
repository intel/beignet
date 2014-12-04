#include "utest_helper.hpp"

void compiler_private_const(void)
{
  const size_t n = 16;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_private_const");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  globals[0] = n;
  locals[0] = n;


  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(0);
  for (size_t i = 0; i < n; ++i)
    OCL_ASSERT(((int32_t*)buf_data[0])[i] == (int32_t)(i * 2));
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_private_const);


