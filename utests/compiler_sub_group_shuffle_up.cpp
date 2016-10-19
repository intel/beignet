#include "utest_helper.hpp"

void compiler_sub_group_shuffle_up_int(void)
{
  if(!cl_check_subgroups())
    return;
  const size_t n = 32;
  const int32_t buf_size = 4 * n + 1;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_sub_group_shuffle_up",
                              "compiler_sub_group_shuffle_up_int");
  OCL_CREATE_BUFFER(buf[0], 0, buf_size * sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  int c = 3;
  OCL_SET_ARG(1, sizeof(int), &c);

  globals[0] = n;
  locals[0] = 16;

  OCL_MAP_BUFFER(0);
  for (int32_t i = 0; i < buf_size; ++i)
    ((int*)buf_data[0])[i] = -1;
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(0);
  int* dst = (int *)buf_data[0];
  int suggroupsize = dst[0];
  OCL_ASSERT(suggroupsize == 8 || suggroupsize == 16);

  dst++;
  for (int32_t i = 0; i < (int32_t) n; ++i){
    int round = i / suggroupsize;
    int index = i % suggroupsize;
    //printf("%d %d %d %d\n",dst[4*i], dst[4*i+1], dst[4*i+2], dst[4*i+3]);
    OCL_ASSERT( ((c - index) > 0 ? 123 : 456) == dst[4*i]);
    OCL_ASSERT( ((c - index) > 0 ? 123 : (i - c)) == dst[4*i+1]);
    OCL_ASSERT( ((suggroupsize - index - 1 - index) > 0 ? (i + index + 1) : -(i + index + 1 - suggroupsize)) == dst[4*i+2]);
    OCL_ASSERT((round * suggroupsize + (suggroupsize - 1)) == dst[4*i+3]);
  }
  OCL_UNMAP_BUFFER(0);
}
MAKE_UTEST_FROM_FUNCTION(compiler_sub_group_shuffle_up_int);

void compiler_sub_group_shuffle_up_short(void)
{
  if(!cl_check_subgroups_short())
    return;
  const size_t n = 32;
  const int32_t buf_size = 4 * n + 1;

  // Setup kernel and buffers
  OCL_CALL(cl_kernel_init, "compiler_sub_group_shuffle_up.cl",
                           "compiler_sub_group_shuffle_up_short",
                           SOURCE, "-DSHORT");
  OCL_CREATE_BUFFER(buf[0], 0, buf_size * sizeof(short), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  int c = 3;
  OCL_SET_ARG(1, sizeof(int), &c);

  globals[0] = n;
  locals[0] = 16;

  OCL_MAP_BUFFER(0);
  for (int32_t i = 0; i < buf_size; ++i)
    ((short*)buf_data[0])[i] = -1;
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(0);
  short* dst = (short *)buf_data[0];
  short suggroupsize = dst[0];
  OCL_ASSERT(suggroupsize == 8 || suggroupsize == 16);

  dst++;
  for (int32_t i = 0; i < (int32_t) n; ++i){
    int round = i / suggroupsize;
    int index = i % suggroupsize;
    //printf("%d %d %d %d\n",dst[4*i], dst[4*i+1], dst[4*i+2], dst[4*i+3]);
    OCL_ASSERT( ((c - index) > 0 ? 123 : 456) == dst[4*i]);
    OCL_ASSERT( ((c - index) > 0 ? 123 : (i - c)) == dst[4*i+1]);
    OCL_ASSERT( ((suggroupsize - index - 1 - index) > 0 ? (i + index + 1) : -(i + index + 1 - suggroupsize)) == dst[4*i+2]);
    OCL_ASSERT((round * suggroupsize + (suggroupsize - 1)) == dst[4*i+3]);
  }
  OCL_UNMAP_BUFFER(0);
}
MAKE_UTEST_FROM_FUNCTION(compiler_sub_group_shuffle_up_short);
