#include "utest_helper.hpp"

static void compiler_copy_image_3d(void)
{
  const size_t w = 512;
  const size_t h = 512;
  const size_t depth = 1;
  cl_image_format format;
  cl_sampler sampler;

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_copy_image_3d");
  buf_data[0] = (uint32_t*) malloc(sizeof(uint32_t) * w * h * depth);
  for (uint32_t k = 0; k < depth; k++)
    for (uint32_t j = 0; j < h; j++)
      for (uint32_t i = 0; i < w; i++)
        ((uint32_t*)buf_data[0])[k*w*h + j*w + i] = k*w*h + j*w + i;

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT8;
  OCL_CREATE_IMAGE3D(buf[0], CL_MEM_COPY_HOST_PTR, &format, w, h, depth, 0, 0, buf_data[0]);
  OCL_CREATE_IMAGE3D(buf[1], 0, &format, w, h, depth, 0, 0, NULL);
  OCL_CREATE_SAMPLER(sampler, CL_ADDRESS_REPEAT, CL_FILTER_NEAREST);
  free(buf_data[0]);
  buf_data[0] = NULL;

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(sampler), &sampler);
  globals[0] = w;
  globals[1] = h;
  locals[0] = 16;
  locals[1] = 16;
  OCL_NDRANGE(2);

  // Check result
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (uint32_t k = 0; k < depth; k++)
    for (uint32_t j = 0; j < h; ++j)
      for (uint32_t i = 0; i < w; i++)
        OCL_ASSERT(((uint32_t*)buf_data[0])[k*w*h + j*w + i] == ((uint32_t*)buf_data[1])[k*w*h + j*w + i]);
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_copy_image_3d);
