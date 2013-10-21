#include "utest_helper.hpp"

#define VECSIZE 8
void compiler_function_argument2(void)
{
  char arg0[8] = { 0 };
  unsigned char arg1[8] = { 0 };
  short arg2[8] = { 0 };
  unsigned short arg3[8] = { 0 };
  int arg4[8] = { 0 };
  unsigned int arg5[8] = { 0 };
  float arg6[8] = { 0 };

  for (uint32_t i = 0; i < 8; ++i) {
      arg0[i] = rand();
      arg1[i] = rand();
      arg2[i] = rand();
      arg3[i] = rand();
      arg4[i] = rand();
      arg5[i] = rand();
      arg6[i] = rand();
  }

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_function_argument2");
  OCL_CREATE_BUFFER(buf[0], 0, sizeof(float) * 8 * 8, NULL);
  OCL_SET_ARG(0, sizeof(arg0), arg0);
  OCL_SET_ARG(1, sizeof(arg1), arg1);
  OCL_SET_ARG(2, sizeof(arg2), arg2);
  OCL_SET_ARG(3, sizeof(arg3), arg3);
  OCL_SET_ARG(4, sizeof(arg4), arg4);
  OCL_SET_ARG(5, sizeof(arg5), arg5);
  OCL_SET_ARG(6, sizeof(arg6), arg6);
  OCL_SET_ARG(7, sizeof(cl_mem), &buf[0]);

  // Run the kernel
  globals[0] = 1;
  locals[0] = 1;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);

  /* Check results */
  float *dst = (float*)buf_data[0];

  for (uint32_t i = 0; i < 8; ++i) {
      OCL_ASSERT((float)arg0[i] == dst[0*8 + i]);
      OCL_ASSERT((float)arg1[i] == dst[1*8 + i]);
      OCL_ASSERT((float)arg2[i] == dst[2*8 + i]);
      OCL_ASSERT((float)arg3[i] == dst[3*8 + i]);
      OCL_ASSERT((float)arg4[i] == dst[4*8 + i]);
      OCL_ASSERT((float)arg5[i] == dst[5*8 + i]);
      OCL_ASSERT((float)arg6[i] == dst[6*8 + i]);
  }
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_function_argument2);
