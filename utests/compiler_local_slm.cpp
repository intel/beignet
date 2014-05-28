#include "utest_helper.hpp"

void compiler_local_slm(void)
{
  const size_t n = 32;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_local_slm", "compiler_local_slm");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i)
    OCL_ASSERT(((uint32_t*)buf_data[0])[i] == (i%16 + 2 + 1+ i/16));
  OCL_UNMAP_BUFFER(0);
}

void compiler_local_slm1(void)
{
  const size_t n = 2;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_local_slm", "compiler_local_slm1");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint64_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  globals[0] = 1;
  locals[0] = 1;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);
  uint64_t * ptr = (uint64_t*)buf_data[0];
  OCL_ASSERT((ptr[1] -ptr[0])  == 4);
  OCL_UNMAP_BUFFER(0);
}
MAKE_UTEST_FROM_FUNCTION(compiler_local_slm);
MAKE_UTEST_FROM_FUNCTION(compiler_local_slm1);
