#include "utests/utest_helper.hpp"
#include <sys/time.h>

void test_copy_buf(size_t sz, size_t src_off, size_t dst_off, size_t cb)
{
  unsigned int i;
  cl_char* buf0;

  OCL_CREATE_BUFFER(buf[0], 0, sz * sizeof(char), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, sz * sizeof(char), NULL);

  buf0 = (cl_char *)clEnqueueMapBuffer(queue, buf[0], CL_TRUE, CL_MAP_WRITE, 0, sizeof(char), 0, NULL, NULL, NULL);

  for (i=0; i < sz; i++) {
    buf0[i]=(rand() & 0xFF);
  }

  clEnqueueUnmapMemObject(queue, buf[0], buf0, 0, NULL, NULL);

  if (src_off + cb > sz || dst_off + cb > sz) {
  /* Expect Error. */
    OCL_ASSERT(clEnqueueCopyBuffer(queue, buf[0], buf[1],
                 src_off, dst_off, cb*sizeof(char), 0, NULL, NULL));
    return;
  }

  OCL_ASSERT(CL_SUCCESS == clEnqueueCopyBuffer(queue, buf[0], buf[1],
    src_off, dst_off, cb*sizeof(char), 0, NULL, NULL));
}

int tim_subtract(struct timeval *y, struct timeval *x, struct timeval *result){
  if ( x->tv_sec > y->tv_sec )
    return   -1;

  if ((x->tv_sec == y->tv_sec) && (x->tv_usec > y->tv_usec))
    return   -1;

  if ( result != NULL){
    result->tv_sec = ( y->tv_sec - x->tv_sec );
    result->tv_usec = ( y->tv_usec - x->tv_usec );

    if (result->tv_usec < 0){
      result->tv_sec --;
      result->tv_usec += 1000000;
    }
  }

  int msec = 1000.0*(y->tv_sec - x->tv_sec) + (y->tv_usec - x->tv_usec)/1000.0;
  return msec;
}


int enqueue_copy_buf(void)
{
  size_t i;
  const size_t sz = 127 *1023 * 1023;
  struct timeval start,stop;

  gettimeofday(&start,0);

  for (i=0; i<10; i++) {
    test_copy_buf(sz, 0, 0, sz);
  }

  gettimeofday(&stop,0);
  return tim_subtract(&stop, &start, 0);
}

MAKE_BENCHMARK_FROM_FUNCTION(enqueue_copy_buf);
