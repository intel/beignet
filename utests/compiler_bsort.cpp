#include "utest_helper.hpp"
/*
 * This test is for non-constant extractelement scalarize
 * this bitonic sort test will use this path in
 *
 *  comp = input < shuffle(input, mask1) ^ dir;                    \
 *  input = shuffle(input, as_uint4(comp + add1));                 \
 *
 * The origin buff is
 * {3.0 5.0 4.0 6.0 0.0 7.0 2.0 1.0}
 * and the expected result is
 * {0.0 1.0 2.0 3.0 4.0 5.0 6.0 7.0}
 */
void compiler_bsort(void)
{
  const int n = 8;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_bsort");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  globals[0] = 1;
  locals[0] = 1;

  OCL_MAP_BUFFER(0);
  ((float *)(buf_data[0]))[0] = 3.0f;
  ((float *)(buf_data[0]))[1] = 5.0f;
  ((float *)(buf_data[0]))[2] = 4.0f;
  ((float *)(buf_data[0]))[3] = 6.0f;
  ((float *)(buf_data[0]))[4] = 0.0f;
  ((float *)(buf_data[0]))[5] = 7.0f;
  ((float *)(buf_data[0]))[6] = 2.0f;
  ((float *)(buf_data[0]))[7] = 1.0f;
  OCL_UNMAP_BUFFER(0);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(0);
  for (int i = 0; i < n; i ++) {
    OCL_ASSERT(((float *)(buf_data[0]))[i] == (float)i);
  }
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_bsort);
