#include "utest_helper.hpp"
#include <string.h>

static void runtime_use_host_ptr_large_image(void)
{
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
  for (uint32_t i = 0; i < w*h*4; ++i) {
    OCL_ASSERT(((uint32_t*)buf_data[0])[i] == ((uint32_t*)buf_data[1])[i]);
  }
  clEnqueueUnmapMemObject(queue, buf[1], mapptr, 0, NULL, NULL);

  free(buf_data[0]);
  buf_data[0] = NULL;
  free(buf_data[1]);
  buf_data[1] = NULL;
}

MAKE_UTEST_FROM_FUNCTION(runtime_use_host_ptr_large_image);

static void runtime_use_host_ptr_large_image_1(void)
{
  cl_int status;
  const size_t w = 4096;
  const size_t h = 4096;
  size_t image_row_pitch, image_slice_pitch;
  size_t origin[3] = {5, 5, 0};
  size_t region[3] = {8, 8, 1};
  uint8_t *p = NULL;
  uint8_t *q = NULL;

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

  // Use mapping mode to fill data into src image
  buf_data[1] = clEnqueueMapImage(queue, buf[0], CL_TRUE, CL_MAP_WRITE, origin, region,
    &image_row_pitch, &image_slice_pitch, 0, NULL, NULL, &status);

  OCL_ASSERT(image_slice_pitch == 0);
  for (uint32_t j = 0; j < region[1]; ++j)
    for (uint32_t i = 0; i < region[0]; i++)
      for (uint32_t k = 0; k < 4; k++)
        ((uint32_t*)buf_data[1])[(j * w + i) * 4 + k] = rand();

  clEnqueueUnmapMemObject(queue, buf[0], buf_data[1], 0, NULL, NULL);

  // Check src image
  origin[0] = 0;
  origin[1] = 0;
  origin[2] = 0;
  region[0] = w;
  region[1] = h;
  region[2] = 1;
  buf_data[1] = clEnqueueMapImage(queue, buf[0], CL_TRUE, CL_MAP_READ, origin, region,
    &image_row_pitch, &image_slice_pitch, 0, NULL, NULL, &status);

  OCL_ASSERT(image_slice_pitch == 0);

  for (uint32_t j = 0; j < h; ++j) {
    p = ((uint8_t*)buf_data[0]) + j * image_row_pitch;
    q = ((uint8_t*)buf_data[1]) + j * image_row_pitch;
    for (uint32_t i = 0; i < w; i++)
      for (uint32_t k = 0; k < 4; k++)
        OCL_ASSERT(((uint32_t*)p)[i * 4 + k] == ((uint32_t*)q)[i * 4 + k]);
  }

  clEnqueueUnmapMemObject(queue, buf[0], buf_data[1], 0, NULL, NULL);

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
  OCL_NDRANGE(2);

  // Check result
  origin[0] = 0;
  origin[1] = 0;
  origin[2] = 0;
  region[0] = w;
  region[1] = h;
  region[2] = 1;
  void* mapptr = (int*)clEnqueueMapImage(queue, buf[1], CL_TRUE, CL_MAP_READ, origin, region,
    &image_row_pitch, &image_slice_pitch, 0, NULL, NULL, NULL);
  OCL_ASSERT(mapptr == buf_data[1]);
  for (uint32_t j = 0; j < h; ++j) {
    p = ((uint8_t*)buf_data[0]) + j * image_row_pitch;
    q = ((uint8_t*)buf_data[1]) + j * image_row_pitch;
    for (uint32_t i = 0; i < w; i++)
      for (uint32_t k = 0; k < 4; k++)
        OCL_ASSERT(((uint32_t*)p)[i * 4 + k] == ((uint32_t*)q)[i * 4 + k]);
  }
  clEnqueueUnmapMemObject(queue, buf[1], mapptr, 0, NULL, NULL);

  free(buf_data[0]);
  buf_data[0] = NULL;
  free(buf_data[1]);
  buf_data[1] = NULL;
}

MAKE_UTEST_FROM_FUNCTION(runtime_use_host_ptr_large_image_1);
