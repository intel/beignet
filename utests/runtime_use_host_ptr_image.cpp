#include "utest_helper.hpp"
#include <string.h>

static void runtime_use_host_ptr_image(void)
{
  const size_t w = 512;
  const size_t h = 512;

  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNORM_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;

  size_t alignment = 4096;  //page size
  if (cl_check_beignet())
    alignment = 64;     //cacheline size, beignet has loose limitaiont to enable userptr

  //src image
  int ret = posix_memalign(&buf_data[0], alignment, sizeof(uint32_t) * w * h);
  OCL_ASSERT(ret == 0);
  for (size_t i = 0; i < w*h; ++i)
    ((uint32_t*)buf_data[0])[i] = i;

  OCL_CREATE_IMAGE(buf[0], CL_MEM_USE_HOST_PTR, &format, &desc, buf_data[0]);

  //dst image
  ret = posix_memalign(&buf_data[1], alignment, sizeof(uint32_t) * w * h);
  OCL_ASSERT(ret == 0);
  for (size_t i = 0; i < w*h; ++i)
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
  OCL_NDRANGE(2);

  // Check result
  size_t origin[3];
  origin[0] = 0;
  origin[1] = 0;
  origin[2] = 0;
  size_t region[3];
  region[0] = w;
  region[1] = h;
  region[2] = 1;
  size_t pitch = 0;
  void* mapptr = (int*)clEnqueueMapImage(queue, buf[1], CL_TRUE, CL_MAP_READ, origin, region, &pitch, NULL, 0, NULL, NULL, NULL);
  OCL_ASSERT(mapptr == buf_data[1]);
  for (uint32_t i = 0; i < w*h; ++i) {
    //printf("%d: src: 0x%x, dst: 0x%x\n", i, ((uint32_t*)buf_data[0])[i], ((uint32_t*)buf_data[1])[i]);
    OCL_ASSERT(((uint32_t*)buf_data[0])[i] == ((uint32_t*)buf_data[1])[i]);
  }
  clEnqueueUnmapMemObject(queue, buf[1], mapptr, 0, NULL, NULL);

  free(buf_data[0]);
  buf_data[0] = NULL;
  free(buf_data[1]);
  buf_data[1] = NULL;
}

MAKE_UTEST_FROM_FUNCTION(runtime_use_host_ptr_image);
