#include "utests/utest_helper.hpp"
#include <sys/time.h>

double benchmark_copy_buf(void)
{
  size_t i;
  const size_t sz = 127 *1023 * 1023;
  const size_t cb = sz;
  size_t src_off =0, dst_off = 0;
  struct timeval start,stop;

  cl_char* buf0;

  OCL_CREATE_BUFFER(buf[0], 0, sz * sizeof(char), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, sz * sizeof(char), NULL);

  buf0 = (cl_char *)clEnqueueMapBuffer(queue, buf[0], CL_TRUE, CL_MAP_WRITE, 0, sizeof(char), 0, NULL, NULL, NULL);

  OCL_ASSERT(buf0 != NULL);

  for (i=0; i < sz; i++) {
    buf0[i]=(rand() & 0xFF);
  }

  clEnqueueUnmapMemObject(queue, buf[0], buf0, 0, NULL, NULL);

  if (src_off + cb > sz || dst_off + cb > sz) {
    /* Expect Error. */
    OCL_ASSERT(clEnqueueCopyBuffer(queue, buf[0], buf[1],
          src_off, dst_off, cb*sizeof(char), 0, NULL, NULL));
  }

  /* Internal kernel will be built for the first time of calling
   * clEnqueueCopyBuffer, so the first execution time of clEnqueueCopyBuffer
   * will be much longer. It should not be added to benchmark time. */
  OCL_ASSERT(CL_SUCCESS == clEnqueueCopyBuffer(queue, buf[0], buf[1],
        src_off, dst_off, cb*sizeof(char), 0, NULL, NULL));
  OCL_FINISH();
  gettimeofday(&start,0);

  for (i=0; i<100; i++) {
    OCL_ASSERT(CL_SUCCESS == clEnqueueCopyBuffer(queue, buf[0], buf[1],
          src_off, dst_off, cb*sizeof(char), 0, NULL, NULL));
  }
  OCL_FINISH();

  gettimeofday(&stop,0);
  double elapsed = time_subtract(&stop, &start, 0);

  return BANDWIDTH(sz * sizeof(char) * 100, elapsed);
}

MAKE_BENCHMARK_FROM_FUNCTION(benchmark_copy_buf, "GB/S");
