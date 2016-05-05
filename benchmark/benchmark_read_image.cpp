#include <string.h>
#include "utests/utest_helper.hpp"
#include <sys/time.h>

double benchmark_read_image(void)
{
  struct timeval start,stop;

  const size_t x_count = 4;
  const size_t y_count = 4;
  const size_t w = 1024;
  const size_t h = 1024;
  const size_t sz = 4 * x_count * y_count * w * h;
  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  // Setup kernel and images
  OCL_CREATE_KERNEL("compiler_read_image");
  buf_data[0] = (uint32_t*) malloc(sizeof(float) * sz);
  buf_data[1] = (uint32_t*) malloc(sizeof(float) * sz);
  for (uint32_t i = 0; i < sz; ++i) {
    ((float*)buf_data[0])[i] = rand();
    ((float*)buf_data[1])[i] = rand();
  }

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_FLOAT;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w * x_count;
  desc.image_height = h * y_count;
  desc.image_row_pitch = desc.image_width * sizeof(float) * 4;
  OCL_CREATE_IMAGE(buf[0], CL_MEM_COPY_HOST_PTR, &format, &desc, buf_data[0]);
  OCL_CREATE_IMAGE(buf[1], CL_MEM_COPY_HOST_PTR, &format, &desc, buf_data[1]);
  OCL_CREATE_BUFFER(buf[2], 0, sz * sizeof(float), NULL);

  free(buf_data[0]);
  buf_data[0] = NULL;
  free(buf_data[1]);
  buf_data[1] = NULL;

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = w;
  globals[1] = h;
  locals[0] = 16;
  locals[1] = 16;

  gettimeofday(&start,0);
  for (size_t i=0; i<100; i++) {
    OCL_NDRANGE(2);
  }
  OCL_FINISH();
  gettimeofday(&stop,0);

  free(buf_data[0]);
  buf_data[0] = NULL;

  double elapsed = time_subtract(&stop, &start, 0);

  return BANDWIDTH(sz * sizeof(float) * 2 * 100, elapsed);
}

MAKE_BENCHMARK_FROM_FUNCTION(benchmark_read_image, "GB/S");
