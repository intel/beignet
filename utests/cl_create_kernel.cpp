#include "utest_helper.hpp"

static void test_create_kernel(void)
{
  cl_ulong max_mem_size;
  cl_int status;

  OCL_CALL(clGetDeviceInfo, device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_mem_size), &max_mem_size, NULL);
  OCL_ASSERT(max_mem_size < (cl_ulong)-1);
  // increment the size so that following clCreateBuffer() would fail.
  ++max_mem_size;
  buf[0] = clCreateBuffer(ctx, 0, max_mem_size, NULL, &status);
  OCL_ASSERT(status == CL_INVALID_BUFFER_SIZE);
}

MAKE_UTEST_FROM_FUNCTION(test_create_kernel);
