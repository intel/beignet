#include <string.h>
#include "utests/utest_helper.hpp"
#include <sys/time.h>

#define BENCH_COPY_IMAGE(J, T, K, M, Q) \
double benchmark_ ##J ##_image_ ##T(void) \
{ \
  struct timeval start,stop; \
\
  const size_t w = 1920; \
  const size_t h = 1080; \
  const size_t sz = 4 * w * h; \
  cl_image_format format; \
  cl_image_desc desc; \
\
  memset(&desc, 0x0, sizeof(cl_image_desc)); \
  memset(&format, 0x0, sizeof(cl_image_format)); \
\
  OCL_CREATE_KERNEL_FROM_FILE("bench_copy_image",K); \
  buf_data[0] = (uint32_t*) malloc(sizeof(M) * sz); \
  for (uint32_t i = 0; i < sz; ++i) { \
    ((M*)buf_data[0])[i] = rand(); \
  } \
\
  format.image_channel_order = CL_RGBA; \
  format.image_channel_data_type = Q; \
  desc.image_type = CL_MEM_OBJECT_IMAGE2D; \
  desc.image_width = w; \
  desc.image_height = h; \
  desc.image_row_pitch = desc.image_width * sizeof(M) * 4; \
  OCL_CREATE_IMAGE(buf[0], CL_MEM_COPY_HOST_PTR, &format, &desc, buf_data[0]); \
\
  desc.image_row_pitch = 0; \
  OCL_CREATE_IMAGE(buf[1], 0, &format, &desc, NULL); \
\
  free(buf_data[0]); \
  buf_data[0] = NULL; \
\
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]); \
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]); \
\
  globals[0] = w; \
  globals[1] = h; \
  locals[0] = 16; \
  locals[1] = 4; \
\
  gettimeofday(&start,0); \
  for (size_t i=0; i<1000; i++) { \
    OCL_NDRANGE(2); \
  } \
  OCL_FINISH(); \
\
  OCL_MAP_BUFFER_GTT(1); \
  OCL_UNMAP_BUFFER_GTT(1); \
  gettimeofday(&stop,0); \
\
  free(buf_data[0]); \
  buf_data[0] = NULL; \
\
  double elapsed = time_subtract(&stop, &start, 0); \
\
  return (double)(1000 / (elapsed * 1e-3)); \
} \
\
MAKE_BENCHMARK_FROM_FUNCTION_KEEP_PROGRAM(benchmark_ ##J ##_image_ ##T, true, "FPS");

BENCH_COPY_IMAGE(copy,uchar, "bench_copy_image", unsigned char, CL_UNSIGNED_INT8)
BENCH_COPY_IMAGE(copy,ushort, "bench_copy_image", unsigned short, CL_UNSIGNED_INT16)
BENCH_COPY_IMAGE(copy,uint, "bench_copy_image", unsigned int,CL_UNSIGNED_INT32)
BENCH_COPY_IMAGE(filter,uchar, "bench_filter_image", unsigned char,CL_UNSIGNED_INT8)
BENCH_COPY_IMAGE(filter,ushort, "bench_filter_image", unsigned short,CL_UNSIGNED_INT16)
BENCH_COPY_IMAGE(filter,uint, "bench_filter_image", unsigned int,CL_UNSIGNED_INT32)
