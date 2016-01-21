#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

void compiler_workgroup_reduce_min_uniform(void)
{
  const size_t n = 17;
  uint32_t src = 253;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_workgroup_reduce", "compiler_workgroup_reduce_min_uniform");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(uint32_t), &src);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[0]);
  globals[0] = n;
  locals[0] = n;

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(0);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%u ", ((uint32_t *)buf_data[0])[i]);
    OCL_ASSERT(((uint32_t *)buf_data[0])[i] == 253);
  }
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_workgroup_reduce_min_uniform);

static uint32_t test_array_uint[64] = {23, 34, 16, 91, 25, 133, 7787, 134, 987, 9853, 33, 21, 865, 1441, 9083, 812,
                                  10, 43435, 63, 445, 253, 65, 24, 30, 76, 989, 120 ,113 ,133, 41, 18, 91,
                                  8321, 6712, 881, 911, 5, 788, 8991, 88, 19, 1110, 1231, 1341, 1983, 1983, 91, 212,
                                  712, 31, 881, 963, 6801, 651, 9810, 77, 98, 5, 16, 1888, 141, 1613, 1771, 16};

void compiler_workgroup_reduce_min_uint(void)
{
  const size_t n = 60;
  uint32_t* src = test_array_uint;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_workgroup_reduce", "compiler_workgroup_reduce_min_uint");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = n;

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, n * sizeof(uint32_t));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%u ", ((uint32_t *)buf_data[1])[i]);
    OCL_ASSERT(((uint32_t *)buf_data[1])[i] == 5);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_workgroup_reduce_min_uint);

void compiler_workgroup_reduce_max_uint(void)
{
  const size_t n = 60;
  uint32_t* src = test_array_uint;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_workgroup_reduce", "compiler_workgroup_reduce_max_uint");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = n;

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, n * sizeof(uint32_t));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%u ", ((uint32_t *)buf_data[1])[i]);
    OCL_ASSERT(((uint32_t *)buf_data[1])[i] == 43435);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_workgroup_reduce_max_uint);

void compiler_workgroup_reduce_add_uint(void)
{
  const size_t n = 50;
  uint32_t* src = test_array_uint;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_workgroup_reduce", "compiler_workgroup_reduce_add_uint");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = n;

  uint32_t cpu_res = 0;
  for (size_t i = 0; i < n; i++)
    cpu_res += src[i];

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, n * sizeof(uint32_t));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%u ", ((uint32_t *)buf_data[1])[i]);
    OCL_ASSERT(((uint32_t *)buf_data[1])[i] == cpu_res);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_workgroup_reduce_add_uint);

static float test_array_float[64] =
  {1.0234f, 0.34e32f, -13441.4334f, 1893.21f, -9999.0f, -88.00f, 1.3f, 1.0f,
   2.33f, 134.44f, 263.0f, 1.0f, 0.0f, 344.900043f, 0.1e30f, 1.0e10f,

   10.0f, 43.435f, 6.3f, 44.545f, 0.253f, 6.5f, 0.24f, 10.30f,
   1312.76f, -0.00989f, 124213.120f, 1.13f, 1.33f, 4.1f, 1.8f, 3234.91f,

   3.21e38f, 6.712f, 0.881f, 12.91f, 5.0f, 7.88f, 128991.0f, 8.8f,
   0.0019f, -0.1110f, 12.0e31f, -3.3E38f, 1.983f, 1.983f, 10091.0f, 2.12f,

   0.88712, 1e31f, -881.0f, -196e3f, 68.01f, -651.121f, 9.810f, -0.77f,
   100.98f, 50.0f, 1000.16f, -18e18f, 0.141f, 1613.0f, 1.771f, -16.13f};

void compiler_workgroup_reduce_min_float(void)
{
  const size_t n = 60;
  float* src = test_array_float;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_workgroup_reduce", "compiler_workgroup_reduce_min_float");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = n;

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, n * sizeof(float));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%f ", ((float *)buf_data[1])[i]);
    OCL_ASSERT(((float *)buf_data[1])[i] == -3.3E38f);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_workgroup_reduce_min_float);

void compiler_workgroup_reduce_max_float(void)
{
  const size_t n = 60;
  float* src = test_array_float;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_workgroup_reduce", "compiler_workgroup_reduce_max_float");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = n;

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, n * sizeof(float));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%f ", ((float *)buf_data[1])[i]);
    OCL_ASSERT(((float *)buf_data[1])[i] == 3.21e38f);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_workgroup_reduce_max_float);

void compiler_workgroup_reduce_add_float(void)
{
  const size_t n = 42;
  float* src = test_array_float;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_workgroup_reduce", "compiler_workgroup_reduce_add_float");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = n;

  float cpu_res = 0;
  for (size_t i = 0; i < n; i++)
    cpu_res += src[i];

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, n * sizeof(float));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%f ", ((float *)buf_data[1])[i]);
    OCL_ASSERT(((float *)buf_data[1])[i] == cpu_res);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_workgroup_reduce_add_float);
