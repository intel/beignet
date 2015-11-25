#include "utest_helper.hpp"
#include <cmath>
void compiler_mix(void)
{
  const float MAXERR = 1e-3f;
  const int n = 1024;
  float src1[n], src2[n], src3[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_mix");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[3], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);
  globals[0] = n;
  locals[0] = 16;

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  for (int i = 0; i < n; ++i) {
    src1[i] = ((float*)buf_data[0])[i] = (float)rand();
    src2[i] = ((float*)buf_data[1])[i] = (float)rand();
    src3[i] = ((float*)buf_data[2])[i] = (float)rand()/(float)RAND_MAX;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(3);
  float res, err;
  float max_err = 0.0f;
  for (int i = 0; i < n; ++i)
  {
    res = src1[i] + ((src2[i] - src1[i]) * src3[i]);
    err = fabsf((((float*)buf_data[3])[i] - res)/ res);
    max_err = err > max_err? err: max_err;
  }
  OCL_UNMAP_BUFFER(3);
  printf("\tmix max err is %g\n",max_err);
  OCL_ASSERT(max_err < MAXERR);
}

MAKE_UTEST_FROM_FUNCTION(compiler_mix);
