#include "utest_helper.hpp"

static void compiler_async_stride_copy(void)
{
  const size_t n = 1024;
  const size_t local_size = 128;
  const int copiesPerWorkItem = 5;
  const int stride =3;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_async_stride_copy");
  OCL_CREATE_BUFFER(buf[0], 0, n * copiesPerWorkItem * sizeof(char) * 4 * stride, NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * copiesPerWorkItem * sizeof(char) * 4 * stride, NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, local_size*copiesPerWorkItem*sizeof(char)*4, NULL);
  OCL_SET_ARG(3, sizeof(int), &copiesPerWorkItem);
  OCL_SET_ARG(4, sizeof(int), &stride);

  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < n * copiesPerWorkItem * 4 * stride; ++i)
      ((char*)buf_data[1])[i] = rand() & 0xff;
  OCL_UNMAP_BUFFER(1);

  // Run the kernel
  globals[0] = n;
  locals[0] = local_size;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);

  // Check results
  char *dst = (char*)buf_data[0];
  char *src = (char*)buf_data[1];
  for (uint32_t i = 0; i < n * copiesPerWorkItem; i += stride * 4) {
    OCL_ASSERT(dst[i + 0] == (char)(src[i + 0] + 3));
    OCL_ASSERT(dst[i + 1] == (char)(src[i + 1] + 3));
    OCL_ASSERT(dst[i + 2] == (char)(src[i + 2] + 3));
    OCL_ASSERT(dst[i + 3] == (char)(src[i + 3] + 3));
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_async_stride_copy);
