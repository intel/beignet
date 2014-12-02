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
  uint32_t* mapptr = (uint32_t*)clEnqueueMapBuffer(queue, buf[0], CL_TRUE, CL_MAP_READ, 0, n*sizeof(uint32_t), 0, NULL, NULL, NULL);
  for (uint32_t i = 0; i < n; ++i)
    OCL_ASSERT(mapptr[i] == i / 2);
  clEnqueueUnmapMemObject(queue, buf[0], mapptr, 0, NULL, NULL);
}

MAKE_UTEST_FROM_FUNCTION(runtime_alloc_host_ptr_buffer);
