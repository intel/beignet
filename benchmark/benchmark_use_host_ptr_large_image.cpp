#include "utests/utest_helper.hpp"
#include <sys/time.h>
#include <string.h>

double benchmark_use_host_ptr_large_image(void)
{
  struct timeval start,stop;

  const size_t w = 4096;
  const size_t h = 4096;

  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT32;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;

  size_t alignment = 4096;  //page size
  if (cl_check_beignet())
    alignment = 64;     //cacheline size, beignet has loose limitaiont to enable userptr

  //src image
  int ret = posix_memalign(&buf_data[0], alignment, sizeof(uint32_t) * w * h * 4);
  OCL_ASSERT(ret == 0);
  for (size_t i = 0; i < w*h*4; ++i)
    ((uint32_t*)buf_data[0])[i] = i;

  OCL_CREATE_IMAGE(buf[0], CL_MEM_USE_HOST_PTR, &format, &desc, buf_data[0]);

  //dst image
  ret = posix_memalign(&buf_data[1], alignment, sizeof(uint32_t) * w * h * 4);
  OCL_ASSERT(ret == 0);
  for (size_t i = 0; i < w*h*4; ++i)
    ((uint32_t*)buf_data[1])[i] = 0;

  OCL_CREATE_IMAGE(buf[1], CL_MEM_USE_HOST_PTR, &format, &desc, buf_data[1]);

  OCL_CREATE_KERNEL("runtime_use_host_ptr_image");

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = w;
  globals[1] = h;
  locals[0] = 16;
  locals[1] = 16;

  size_t origin[3];
  origin[0] = 0;
  origin[1] = 0;
  origin[2] = 0;
  size_t region[3];
  region[0] = w;
  region[1] = h;
  region[2] = 1;
  size_t pitch = 0;

  gettimeofday(&start,0);
  for (size_t i=0; i<100; i++) {
    OCL_NDRANGE(2);
    void* mapptr = (int*)clEnqueueMapImage(queue, buf[1], CL_TRUE, CL_MAP_READ, origin,
      region, &pitch, NULL, 0, NULL, NULL, NULL);
    OCL_ASSERT(mapptr == buf_data[1]);
    clEnqueueUnmapMemObject(queue, buf[1], mapptr, 0, NULL, NULL);
  }
  gettimeofday(&stop,0);

  free(buf_data[0]);
  buf_data[0] = NULL;
  free(buf_data[1]);
  buf_data[1] = NULL;

  double elapsed = time_subtract(&stop, &start, 0);

  return BANDWIDTH(w*h*sizeof(uint32_t)*4*100*2, elapsed);
}

MAKE_BENCHMARK_FROM_FUNCTION(benchmark_use_host_ptr_large_image, "GB/S");
