#include "utest_helper.hpp"

void compiler_clz_short(void)
{
  const size_t n = 16;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_clz_short");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(short), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(short), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  OCL_MAP_BUFFER(0);
  ((short*)buf_data[0])[0] = 0;
  for (int32_t i = 1; i < (int32_t) n; ++i)
    ((short*)buf_data[0])[i] = 0xffffu >> i;
  OCL_UNMAP_BUFFER(0);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(1);
  OCL_ASSERT(((short*)buf_data[1])[0] == 16);
  for (unsigned i = 1; i < (unsigned) n; ++i)
    OCL_ASSERT(((short*)buf_data[1])[i] == (short)i);
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_clz_short);
