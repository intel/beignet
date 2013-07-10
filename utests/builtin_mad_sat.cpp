#include "utest_helper.hpp"

void builtin_mad_sat(void)
{
  const int n = 32;
  short src1[n], src2[n], src3[n];
srand(0);
  // Setup kernel and buffers
  OCL_CREATE_KERNEL("builtin_mad_sat");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(short), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(short), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(short), NULL);
  OCL_CREATE_BUFFER(buf[3], 0, n * sizeof(short), NULL);
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
    src1[i] = ((short*)buf_data[0])[i] = rand();
    src2[i] = ((short*)buf_data[1])[i] = rand();
    src3[i] = ((short*)buf_data[2])[i] = rand();
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(3);
  for (int i = 0; i < n; ++i) {
    int a = (int)src1[i] * (int)src2[i] + (int)src3[i];
    a = a > 0x7FFF ? 0x7FFF : (a < -0x8000 ? -0x8000 : a);
    OCL_ASSERT(((short*)buf_data[3])[i] == (short)a);
  }
  OCL_UNMAP_BUFFER(3);
}

MAKE_UTEST_FROM_FUNCTION(builtin_mad_sat);
