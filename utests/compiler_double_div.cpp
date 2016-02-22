#include <cmath>
#include "utest_helper.hpp"

void compiler_double_div(void)
{
  const size_t n = 16;
  double cpu_src0[n], cpu_src1[n];

  if (!cl_check_double())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_double_div");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(double), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    cpu_src0[i] = ((double*)buf_data[0])[i] = ((double)(((i - 5)*1334) * 11105));
    cpu_src1[i] = ((double*)buf_data[1])[i] = 499.13542123*(i + 132.43 + 142.32*i);
    ((double*)buf_data[2])[i] = 0.0;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    if (i % 3 != 0)
      OCL_ASSERT(fabs(((double*)buf_data[2])[i] - cpu_src0[i]/cpu_src1[i]) < 1e-32);
    else
      OCL_ASSERT(((double*)buf_data[2])[i] == 0.0);

    //printf("%d :  %f        ref value: %f\n", i, ((double*)buf_data[2])[i], cpu_src0[i]/cpu_src1[i]);
  }
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_double_div);

void compiler_double_div_uniform(void)
{
  double src0 = 13234.1438786319;
  double src1 = 0.000134123;
  double tmp = 25.128;

  if (!cl_check_double())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_double_div", "compiler_double_div_uniform");
  OCL_CREATE_BUFFER(buf[0], 0, sizeof(double), NULL);
  OCL_SET_ARG(0, sizeof(double), &src0);
  OCL_SET_ARG(1, sizeof(double), &src1);
  OCL_SET_ARG(2, sizeof(double), &tmp);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[0]);
  globals[0] = 16;
  locals[0] = 16;

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(0);
  OCL_ASSERT(fabs(((double*)buf_data[0])[0] - src0/src1) < 1e-32);
  //printf("%f        ref value: %f\n", ((double*)buf_data[0])[0], src0/src1);
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_double_div_uniform);
