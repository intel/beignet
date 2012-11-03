#include "utest_helper.hpp"

void compiler_insert_to_constant(void)
{
  const size_t n = 32;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_insert_to_constant");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t[4]), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);

  // Check results
  uint32_t *data = (uint32_t*) buf_data[0];
  for (uint32_t i = 0; i < n; ++i) {
    OCL_ASSERT(data[4*i+0] == 0);
    OCL_ASSERT(data[4*i+1] == 1);
    OCL_ASSERT(data[4*i+2] == i);
    OCL_ASSERT(data[4*i+3] == 3);
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_insert_to_constant);


