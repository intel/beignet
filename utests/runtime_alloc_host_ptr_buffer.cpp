#include "utest_helper.hpp"

static void runtime_alloc_host_ptr_buffer(void)
{
  const size_t n = 4096*100;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("runtime_alloc_host_ptr_buffer");

  OCL_CREATE_BUFFER(buf[0], CL_MEM_ALLOC_HOST_PTR, n * sizeof(uint32_t), NULL);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  globals[0] = n;
  locals[0] = 256;
  OCL_NDRANGE(1);

  // Check result
  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i)
    OCL_ASSERT(((int*)buf_data[0])[i] == (int)i / 2);
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(runtime_alloc_host_ptr_buffer);
