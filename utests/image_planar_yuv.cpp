#include <string.h>
#include "utest_helper.hpp"
#include <malloc.h>
#include <cstring>

static void image_planar_yuv(void)
{
  size_t param_value_size;
  std::string extensionStr;
  OCL_CALL (clGetPlatformInfo, platform, CL_PLATFORM_EXTENSIONS, 0, 0, &param_value_size);
  std::vector<char> param_value(param_value_size);
  OCL_CALL (clGetPlatformInfo, platform, CL_PLATFORM_EXTENSIONS, param_value_size, param_value.empty() ? NULL : &param_value.front(), &param_value_size);
  if (!param_value.empty())
    extensionStr = std::string(&param_value.front(), param_value_size-1);

  if (!std::strstr(extensionStr.c_str(), "cl_intel_planar_yuv")) {
    return;
  }

  size_t base_address_alignment = 0;
  OCL_CALL (clGetDeviceInfo, device, CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT, sizeof(base_address_alignment), &base_address_alignment, NULL);
  const size_t w = 36;
  const size_t h = 36;
  // Setup kernel and images
  size_t buffer_sz = sizeof(uint8_t) * w * h * 2;
  size_t yplane_sz = sizeof(uint8_t) * w * h;
  size_t uvplane_sz = sizeof(uint8_t) * w/2 * h /2 * 2;
  uint8_t* src_data;
  uint8_t* yplane_data;
  uint8_t* uvplane_data;
  src_data = (uint8_t*)memalign(base_address_alignment, buffer_sz);
  yplane_data = (uint8_t*)memalign(base_address_alignment, yplane_sz);
  uvplane_data = (uint8_t*)memalign(base_address_alignment, uvplane_sz);
  if (!src_data || !yplane_data || !uvplane_data) {
    fprintf(stderr, "run out of memory\n");
    return;
  }
  memset(src_data, 0, buffer_sz);
  memset(yplane_data, 0, yplane_sz);
  memset(uvplane_data, 0, uvplane_sz);
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; i++)
      src_data[j * w + i] = uint8_t( i+5);

  for (uint32_t j = h; j < 2 * h; ++j) {
    for (uint32_t i = 0; i < w; i += 2) {
      src_data[j * w + i] = uint8_t(i);
      src_data[j * w + i + 1] = uint8_t(i + 1);
    }
  }

  cl_image_format image_format;
  image_format.image_channel_order     = CL_NV12_INTEL;
  image_format.image_channel_data_type = CL_UNORM_INT8;

  cl_image_desc image_desc;
  image_desc.image_type                = CL_MEM_OBJECT_IMAGE2D;
  image_desc.image_width               = w;
  image_desc.image_height              = h;
  image_desc.image_array_size          = 0;
  image_desc.image_row_pitch           = 0;
  image_desc.image_slice_pitch         = 0;
  image_desc.num_mip_levels            = 0;
  image_desc.num_samples               = 0;
  image_desc.mem_object                = NULL;

  int error;

  cl_mem nv12Img = clCreateImage(ctx, CL_MEM_READ_ONLY |  CL_MEM_ACCESS_FLAGS_UNRESTRICTED_INTEL,
                    &image_format, &image_desc, src_data, &error);
  OCL_ASSERT(nv12Img);
  size_t origin[] = {0,0,0};
  size_t region[] = {w, h, 1};

// as the whole yuv image write is not supported, format PLANAR_420_8
// couldn't be written by typed surface write, we disable this usage so far.
#if 0
  OCL_CREATE_KERNEL_FROM_FILE("image_planar_yuv", "image_planar_total");

  cl_mem nv12ImgOut = clCreateImage(ctx, CL_MEM_READ_WRITE | CL_MEM_ACCESS_FLAGS_UNRESTRICTED_INTEL,
                    &image_format, &image_desc, NULL, &error);

  OCL_SET_ARG(0, sizeof(cl_mem), &nv12Img);
  OCL_SET_ARG(1, sizeof(cl_mem), &nv12ImgOut);

  globals[0] = w;
  globals[1] = h;
  locals[0] = 16;
  locals[1] = 4;

  OCL_NDRANGE(2);
  clReleaseMemObject(nv12ImgOut);
#endif

  OCL_CREATE_KERNEL_FROM_FILE("image_planar_yuv", "image_planar_seperate");

  size_t image_row_pitch = w;
  size_t image_slice_pitch = 0;

  // set mem_object to the full NV12 image
  image_desc.mem_object = nv12Img;
  
  // image_width & image_height are ignored for plane extraction
  image_desc.image_width = 0;
  image_desc.image_height = 0;

  // get access to the Y plane (CL_R)
  image_desc.image_depth = 0;

  //  set proper image_format for the Y plane
  image_format.image_channel_order = CL_R;
  image_format.image_channel_data_type = CL_UNORM_INT8;

  cl_mem nv12YplaneImg = clCreateImage(ctx, CL_MEM_READ_WRITE, &image_format,
                                       &image_desc, NULL, &error);
  OCL_ASSERT(nv12YplaneImg);
  cl_mem nv12YplaneImgOut = clCreateImage(ctx, CL_MEM_READ_WRITE, &image_format,
                                       &image_desc, NULL, &error);
  OCL_ASSERT(nv12YplaneImgOut);

  // get access to the UV plane (CL_RG)
  image_desc.image_depth = 1;

  //  set proper image_format for the UV plane
  image_format.image_channel_order = CL_RG;
  image_format.image_channel_data_type = CL_UNORM_INT8;

  cl_mem nv12UVplaneImg = clCreateImage(ctx, CL_MEM_READ_WRITE, &image_format,
                                        &image_desc, NULL, &error);
  OCL_ASSERT(nv12UVplaneImg);
  cl_mem nv12UVplaneImgOut = clCreateImage(ctx, CL_MEM_READ_WRITE, &image_format,
                                        &image_desc, NULL, &error);
  OCL_ASSERT(nv12UVplaneImgOut);

  size_t region_uv[] = {w/2, h/2, 1};

  OCL_SET_ARG(0, sizeof(cl_mem), &nv12YplaneImg);
  OCL_SET_ARG(1, sizeof(cl_mem), &nv12UVplaneImg);
  OCL_SET_ARG(2, sizeof(cl_mem), &nv12YplaneImgOut);
  OCL_SET_ARG(3, sizeof(cl_mem), &nv12UVplaneImgOut);

  globals[0] = w;
  globals[1] = h;
  locals[0] = 4;
  locals[1] = 4;

  OCL_NDRANGE(2);

  // read Y plane of NV12 image
  error = clEnqueueReadImage(queue, nv12YplaneImg, true, origin, region,
                              image_row_pitch, image_slice_pitch, yplane_data, 0, NULL, NULL);

  // read UV plane of NV12 image
  error = clEnqueueReadImage(queue, nv12UVplaneImg, true, origin, region_uv,
                              image_row_pitch, image_slice_pitch, uvplane_data, 0, NULL, NULL);

  for (uint32_t j = 0; j < h; ++j) {
    for (uint32_t i = 0; i < w; i++)
      OCL_ASSERT(yplane_data[j * w + i] == i+6);
  }
  for (uint32_t j = 0; j < h/2; ++j) {
    for (uint32_t i = 0; i < w; i++)
      OCL_ASSERT(uvplane_data[j * w + i] == i+1);
  }

  free(src_data);
  free(yplane_data);
  free(uvplane_data);
  clReleaseMemObject(nv12YplaneImg);
  clReleaseMemObject(nv12UVplaneImg);
  clReleaseMemObject(nv12YplaneImgOut);
  clReleaseMemObject(nv12UVplaneImgOut);
  clReleaseMemObject(nv12Img);
}

MAKE_UTEST_FROM_FUNCTION(image_planar_yuv);

