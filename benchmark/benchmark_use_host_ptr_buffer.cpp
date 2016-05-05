#include "utests/utest_helper.hpp"
#include <sys/time.h>

double benchmark_use_host_ptr_buffer(void)
{
  struct timeval start,stop;

  const size_t n = 4096*4096;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("runtime_use_host_ptr_buffer");

  int ret = posix_memalign(&buf_data[0], 64, sizeof(uint32_t) * n);
  OCL_ASSERT(ret == 0);

  for (uint32_t i = 0; i < n; ++i) ((uint32_t*)buf_data[0])[i] = i;
  OCL_CREATE_BUFFER(buf[0], CL_MEM_USE_HOST_PTR, n * sizeof(uint32_t), buf_data[0]);

  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  globals[0] = n;
  locals[0] = 256;

  gettimeofday(&start,0);
  for (size_t i=0; i<100; i++) {
    OCL_NDRANGE(1);
    void* mapptr = (int*)clEnqueueMapBuffer(queue, buf[0], CL_TRUE, CL_MAP_READ, 0, n*sizeof(uint32_t), 0, NULL, NULL, NULL);
    clEnqueueUnmapMemObject(queue, buf[0], mapptr, 0, NULL, NULL);
  }
  gettimeofday(&stop,0);

  free(buf_data[0]);
  buf_data[0] = NULL;

  double elapsed = time_subtract(&stop, &start, 0);

  return BANDWIDTH(n*sizeof(uint32_t)*100*2, elapsed);
}

MAKE_BENCHMARK_FROM_FUNCTION(benchmark_use_host_ptr_buffer, "GB/S");
