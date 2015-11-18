#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

void compiler_workgroup_broadcast(void)
{
  const size_t n0 = 32;
  const size_t n1 = 16;
  const size_t n = n0 * n1;
  uint32_t src[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_workgroup_broadcast");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n0;
  globals[1] = n1;
  locals[0] = 16;
  locals[1] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src[i] = i;
  }
  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, sizeof(src));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(2);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n/2; ++i) {
//    printf("%u ", ((uint32_t *)buf_data[1])[i]);
    OCL_ASSERT(((uint32_t *)buf_data[1])[i] == 56);
  }
  for (int32_t i = n/2; i < (int32_t) n; ++i) {
    //	printf("%u ", ((uint32_t *)buf_data[1])[i]);
    OCL_ASSERT(((uint32_t *)buf_data[1])[i] == 312);
  }
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_workgroup_broadcast);
