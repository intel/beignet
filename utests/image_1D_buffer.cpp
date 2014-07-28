#include <string.h>
#include "utest_helper.hpp"

void image_1D_buffer(void)
{
  size_t buffer_sz = 1024;
  char *buf_content = (char *)malloc(buffer_sz * sizeof(char));
  int error;
  cl_image_desc image_desc;
  cl_image_format image_format;
  cl_sampler sampler;
  cl_mem image1, image2;
  cl_mem ret_mem = NULL;

  OCL_CREATE_KERNEL("image_1D_buffer");

  for (int32_t i = 0; i < (int32_t)buffer_sz; ++i)
    buf_content[i] = (rand() & 127);

  cl_mem buff = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                      buffer_sz, buf_content, &error);
  OCL_ASSERT(error == CL_SUCCESS);

  memset(&image_desc, 0x0, sizeof(cl_image_desc));
  memset(&image_format, 0x0, sizeof(cl_image_format));

  image_desc.image_type = CL_MEM_OBJECT_IMAGE1D_BUFFER;
  image_desc.image_row_pitch = buffer_sz;
  image_desc.image_width = buffer_sz / sizeof(uint32_t); //assume rgba32
  image_desc.buffer = buff;

  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_UNSIGNED_INT8;

  image1 = clCreateImage(ctx, CL_MEM_READ_ONLY, &image_format,
                        &image_desc, NULL, &error );
  OCL_ASSERT(error == CL_SUCCESS);

  error = clGetImageInfo(image1, CL_IMAGE_BUFFER, sizeof(ret_mem), &ret_mem, NULL);
  OCL_ASSERT(error == CL_SUCCESS);
  OCL_ASSERT(ret_mem == buff);


  memset(&image_desc, 0x0, sizeof(cl_image_desc));
  image_desc.image_type = CL_MEM_OBJECT_IMAGE1D;
  image_desc.image_width = buffer_sz / sizeof(uint32_t);
  image2 = clCreateImage(ctx, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                         &image_format, &image_desc, buf_content, &error);
  OCL_ASSERT(error == CL_SUCCESS);

  // Create sampler to use
  sampler = clCreateSampler(ctx, false, CL_ADDRESS_NONE, CL_FILTER_NEAREST, &error );
  OCL_ASSERT(error == CL_SUCCESS);

  cl_mem result_buf = buf[0] = clCreateBuffer(ctx, 0, buffer_sz, NULL, &error);
  OCL_ASSERT(error == CL_SUCCESS);

  OCL_SET_ARG(0, sizeof(cl_mem), &image1);
  OCL_SET_ARG(1, sizeof(cl_mem), &image2);
  OCL_SET_ARG(2, sizeof(sampler), &sampler);
  OCL_SET_ARG(3, sizeof(cl_mem), &result_buf);

  globals[0] = buffer_sz/sizeof(int32_t);
  locals[0] = 16;

  OCL_NDRANGE(1);

  /* Now check the result. */
  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < buffer_sz/sizeof(int32_t); i++)
    OCL_ASSERT(((uint32_t*)buf_data[0])[i] == 1);
  OCL_UNMAP_BUFFER(0);

  clReleaseSampler(sampler);
  clReleaseMemObject(image1);
  clReleaseMemObject(image2);
  clReleaseMemObject(buff);
}

MAKE_UTEST_FROM_FUNCTION(image_1D_buffer);
