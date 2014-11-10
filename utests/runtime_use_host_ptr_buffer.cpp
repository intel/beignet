#include "utest_helper.hpp"

static void runtime_use_host_ptr_buffer(void)
{
  const size_t n = 4096*100;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("runtime_use_host_ptr_buffer");

  int ret = posix_memalign(&buf_data[0], 4096, sizeof(uint32_t) * n);
  OCL_ASSERT(ret == 0);

  for (uint32_t i = 0; i < n; ++i) ((uint32_t*)buf_data[0])[i] = i;
  OCL_CREATE_BUFFER(buf[0], CL_MEM_USE_HOST_PTR, n * sizeof(uint32_t), buf_data[0]);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  globals[0] = n;
  locals[0] = 256;
  OCL_NDRANGE(1);

  // Check result

#ifdef HAS_USERPTR
  OCL_FINISH();
#else
  void* mapptr = (int*)clEnqueueMapBuffer(queue, buf[0], CL_TRUE, CL_MAP_READ, 0, n*sizeof(uint32_t), 0, NULL, NULL, NULL);
  OCL_ASSERT(mapptr == buf_data[0]);
  clEnqueueUnmapMemObject(queue, buf[0], mapptr, 0, NULL, NULL);
#endif

  for (uint32_t i = 0; i < n; ++i)
    OCL_ASSERT(((uint32_t*)buf_data[0])[i] == i / 2);

  free(buf_data[0]);
  buf_data[0] = NULL;
}

MAKE_UTEST_FROM_FUNCTION(runtime_use_host_ptr_buffer);
