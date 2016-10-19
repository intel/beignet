#include "utest_helper.hpp"

void compiler_sub_group_shuffle_down_int(void)
{
  if(!cl_check_subgroups())
    return;
  const size_t n = 32;
  const int32_t buf_size = 4 * n + 1;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_sub_group_shuffle_down",
                              "compiler_sub_group_shuffle_down_int");
  OCL_CREATE_BUFFER(buf[0], 0, buf_size * sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  int c = 13;
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
    OCL_ASSERT( (index + c >= suggroupsize ? 456 : 123) == dst[4*i]);
    OCL_ASSERT( (index + c >= suggroupsize ? (round * suggroupsize + (i + c) % suggroupsize): 123) == dst[4*i+1]);
    OCL_ASSERT( (index + index + 1 >= suggroupsize ? -(round * suggroupsize + (i + index + 1) % suggroupsize) : (round * suggroupsize + (i + index + 1) % suggroupsize))  == dst[4*i+2]);
    OCL_ASSERT((round * suggroupsize + (suggroupsize - 1)) == dst[4*i+3]);
  }
  OCL_UNMAP_BUFFER(0);
}
MAKE_UTEST_FROM_FUNCTION(compiler_sub_group_shuffle_down_int);

void compiler_sub_group_shuffle_down_short(void)
{
  if(!cl_check_subgroups_short())
    return;
  const size_t n = 32;
  const int32_t buf_size = 4 * n + 1;

  // Setup kernel and buffers
  OCL_CALL(cl_kernel_init, "compiler_sub_group_shuffle_down.cl",
                           "compiler_sub_group_shuffle_down_short",
                           SOURCE, "-DSHORT");
  OCL_CREATE_BUFFER(buf[0], 0, buf_size * sizeof(short), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  int c = 13;
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
    OCL_ASSERT( (index + c >= suggroupsize ? 456 : 123) == dst[4*i]);
    OCL_ASSERT( (index + c >= suggroupsize ? (round * suggroupsize + (i + c) % suggroupsize): 123) == dst[4*i+1]);
    OCL_ASSERT( (index + index + 1 >= suggroupsize ? -(round * suggroupsize + (i + index + 1) % suggroupsize) : (round * suggroupsize + (i + index + 1) % suggroupsize))  == dst[4*i+2]);
    OCL_ASSERT((round * suggroupsize + (suggroupsize - 1)) == dst[4*i+3]);
  }
  OCL_UNMAP_BUFFER(0);
}
MAKE_UTEST_FROM_FUNCTION(compiler_sub_group_shuffle_down_short);
