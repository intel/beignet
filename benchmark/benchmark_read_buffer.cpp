#include "utests/utest_helper.hpp"
#include <sys/time.h>

double benchmark_read_buffer(void)
{
  struct timeval start,stop;

  const size_t n = 1024 * 1024;
  int count = 16;
  const size_t sz = 4 * n * count;

  OCL_CREATE_BUFFER(buf[0], 0, sz * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, sz * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, sz * sizeof(float), NULL);

  OCL_CREATE_KERNEL("compiler_read_buffer");

  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (size_t i = 0; i < sz; i ++) {
    ((float *)(buf_data[0]))[i] = rand();
    ((float *)(buf_data[1]))[i] = rand();
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Setup kernel and buffers
  globals[0] = n;
  locals[0] = 256;

  gettimeofday(&start,0);
  for (size_t i=0; i<100; i++) {
    OCL_NDRANGE(1);
  }
  OCL_FINISH();
  gettimeofday(&stop,0);

  free(buf_data[0]);
  buf_data[0] = NULL;

  double elapsed = time_subtract(&stop, &start, 0);

  return BANDWIDTH(sz * sizeof(float) * 2 * 100, elapsed);
}

MAKE_BENCHMARK_FROM_FUNCTION(benchmark_read_buffer, "GB/S");
