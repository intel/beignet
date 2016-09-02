#include <string.h>
#include "utest_helper.hpp"

void image_1D_buffer(void)
{
  size_t buffer_sz = 8192 * 2 + 32;
  int *buf_content = (int *)malloc(buffer_sz * sizeof(int));
  int error;
  cl_image_desc image_desc;
  cl_image_format image_format;
  cl_mem ret_mem = NULL;

  OCL_CREATE_KERNEL("image_1D_buffer");

  for (int32_t i = 0; i < (int32_t)buffer_sz; ++i)
    buf_content[i] = rand();

  OCL_CREATE_BUFFER(buf[0], CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, buffer_sz * sizeof(int), buf_content);
  OCL_CREATE_BUFFER(buf[1], CL_MEM_READ_WRITE, buffer_sz * sizeof(int), NULL);

  memset(&image_desc, 0x0, sizeof(cl_image_desc));
  memset(&image_format, 0x0, sizeof(cl_image_format));

  image_desc.image_type = CL_MEM_OBJECT_IMAGE1D_BUFFER;
  image_desc.image_row_pitch = buffer_sz * sizeof(int);
  image_desc.image_width = buffer_sz; //assume r32
  image_desc.buffer = buf[0];

  image_format.image_channel_order = CL_R;
  image_format.image_channel_data_type = CL_UNSIGNED_INT32;

  // Create the source image1d_buffer.
  OCL_CREATE_IMAGE(buf[2], CL_MEM_READ_ONLY, &image_format, &image_desc, NULL);
  error = clGetImageInfo(buf[2], CL_IMAGE_BUFFER, sizeof(ret_mem), &ret_mem, NULL);
  OCL_ASSERT(error == CL_SUCCESS);
  OCL_ASSERT(ret_mem == buf[0]);

  // Create the destination image1d_buffer.
  image_desc.buffer = buf[1];
  OCL_CREATE_IMAGE(buf[3], CL_MEM_READ_ONLY, &image_format, &image_desc, NULL);
  error = clGetImageInfo(buf[3], CL_IMAGE_BUFFER, sizeof(ret_mem), &ret_mem, NULL);
  OCL_ASSERT(error == CL_SUCCESS);
  OCL_ASSERT(ret_mem == buf[1]);

  OCL_SET_ARG(0, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[3]);

  globals[0] = buffer_sz;
  locals[0] = 16;

  OCL_NDRANGE(1);

  /* Now check the result. */
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < buffer_sz; i++) {
    if (((uint32_t*)buf_data[1])[i] != ((uint32_t*)buf_data[0])[i])
      printf("i %d expected %x got %x", i, ((uint32_t*)buf_data[0])[i], ((uint32_t*)buf_data[1])[i]);
    OCL_ASSERT(((uint32_t*)buf_data[1])[i] == ((uint32_t*)buf_data[0])[i]);
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  free(buf_content);
}

MAKE_UTEST_FROM_FUNCTION(image_1D_buffer);
