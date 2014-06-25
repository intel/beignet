#include "utest_helper.hpp"
#include "string.h"

static void compiler_movforphi_undef(void)
{
  const size_t w = 16;
  const size_t h = 16;
  cl_sampler sampler;
  cl_image_format format;
  cl_image_desc desc;

  // Setup kernel and images
  OCL_CREATE_KERNEL("test_movforphi_undef");
  buf_data[0] = (uint32_t*) malloc(sizeof(uint32_t) * w * h);
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; i++)
      ((uint32_t*)buf_data[0])[j * w + i] = j * w + i;

  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNSIGNED_INT8;
  memset(&desc, 0, sizeof(desc));
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = w * sizeof(uint32_t);
  OCL_CREATE_IMAGE(buf[0], CL_MEM_COPY_HOST_PTR, &format, &desc, buf_data[0]);

  desc.image_row_pitch = 0;
  OCL_CREATE_IMAGE(buf[1], 0, &format, &desc, NULL);
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
  // Just compare the initial 2 data is enough for this case, as the initial 2 data must in the first
  // tile box and we can just get the correct coords.
  for (uint32_t j = 0; j < 1; ++j)
    for (uint32_t i = 0; i < 3; i++)
    {
      if (i == 0)
        OCL_ASSERT(((uint32_t*)buf_data[0])[j * w + i + 1] == ((uint32_t*)buf_data[1])[j * w + i]);
    }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  OCL_CALL(clReleaseSampler, sampler);
}

MAKE_UTEST_FROM_FUNCTION(compiler_movforphi_undef);
