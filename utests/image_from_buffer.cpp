#include <string.h>
#include "utest_helper.hpp"
#include <malloc.h>
#include <cstring>

static void image_from_buffer(void)
{
  size_t param_value_size;
  std::string extensionStr;
  OCL_CALL (clGetPlatformInfo, platform, CL_PLATFORM_EXTENSIONS, 0, 0, &param_value_size);
  std::vector<char> param_value(param_value_size);
  OCL_CALL (clGetPlatformInfo, platform, CL_PLATFORM_EXTENSIONS, param_value_size, param_value.empty() ? NULL : &param_value.front(), &param_value_size);
  if (!param_value.empty())
    extensionStr = std::string(&param_value.front(), param_value_size-1);

  if (!std::strstr(extensionStr.c_str(), "cl_khr_image2d_from_buffer")) {
    return;
  }

  size_t base_address_alignment = 0;
  OCL_CALL (clGetDeviceInfo, device, CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT, sizeof(base_address_alignment), &base_address_alignment, NULL);
  const size_t w = 512;
  const size_t h = 512;
  cl_image_format format;
  cl_image_desc desc;
  int error;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  OCL_CREATE_KERNEL("image_from_buffer");

  // Setup kernel and images
  size_t buffer_sz = sizeof(uint32_t) * w * h;
  uint32_t* src_data;
  src_data = (uint32_t*)memalign(base_address_alignment, buffer_sz);
  if(!src_data) {
    fprintf(stderr, "run out of memory\n");
    return;
  }

  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; i++)
      src_data[j * w + i] = j * w + i;

  cl_mem buff = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, buffer_sz, src_data, &error);

  OCL_ASSERT(error == CL_SUCCESS);
  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = w * sizeof(uint32_t);

  desc.buffer = 0;
  OCL_CREATE_IMAGE(buf[0], CL_MEM_COPY_HOST_PTR, &format, &desc, src_data);

  desc.buffer = buff;
  OCL_CREATE_IMAGE(buf[1], 0, &format, &desc, NULL);

  desc.buffer = 0;
  desc.image_row_pitch = 0;
  OCL_CREATE_IMAGE(buf[2], CL_MEM_WRITE_ONLY, &format, &desc, NULL);

  OCL_SET_ARG(0, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[2]);

  globals[0] = w;
  globals[1] = h;
  locals[0] = 16;
  locals[1] = 4;

  OCL_NDRANGE(2);

  // Check result
  OCL_MAP_BUFFER_GTT(0);
  OCL_MAP_BUFFER_GTT(1);
  OCL_MAP_BUFFER_GTT(2);
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; i++)
    {
      //printf("%d,%d\n", ((uint32_t*)buf_data[0])[j * w + i], ((uint32_t*)buf_data[1])[j * w + i]);
      //printf("%d,%d,%d,%d\n", j, i, ((uint32_t*)buf_data[0])[j * w + i], ((uint32_t*)buf_data[2])[j * w + i]);
      OCL_ASSERT(((uint32_t*)buf_data[0])[j * w + i] == ((uint32_t*)buf_data[1])[j * w + i]);
      OCL_ASSERT(((uint32_t*)buf_data[0])[j * w + i] == ((uint32_t*)buf_data[2])[j * w + i]);
    }
  OCL_UNMAP_BUFFER_GTT(0);
  OCL_UNMAP_BUFFER_GTT(1);
  OCL_UNMAP_BUFFER_GTT(2);

  free(src_data);

  //spec didn't tell the sequence of release buffer of image. so release either buffer or image first is ok here.
  //we follow the rule of destroy the bo at the last release, then the access of buffer after release image is legal
  //and vice verse.
#if 1
  clReleaseMemObject(buf[1]);
  clReleaseMemObject(buff);
#else
  clReleaseMemObject(buff);
  clReleaseMemObject(buf[1]);
#endif
  clReleaseMemObject(buf[2]);
  buf[1] = NULL;
  buf[2] = NULL;
}

MAKE_UTEST_FROM_FUNCTION(image_from_buffer);
