#include "utest_helper.hpp"

struct hop{
  int  offset;
  int  threshold0;
  int  threshold1;
};

void compiler_argument_structure_select(void)
{
  const size_t n = 2048;
  hop h;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_argument_structure_select");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  h.offset = 2;
  h.threshold0 = 5;
  h.threshold1 = 7;
  OCL_SET_ARG(1, sizeof(hop), &h);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);

  // Check results
  OCL_ASSERT(((uint32_t*)buf_data[0])[0] == 5);
  for (uint32_t i = 1; i < n; ++i ) {
    OCL_ASSERT(((uint32_t*)buf_data[0])[i] == 7);
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_argument_structure_select);

