#include "utest_helper.hpp"
#include <stdint.h>

typedef unsigned char uchar;
typedef unsigned short ushort;

#define DEF(TYPE, KER_TYPE, VEC_SIZE) \
static void compiler_async_copy_##KER_TYPE##VEC_SIZE(void) \
{ \
  const size_t n = 1024; \
  const size_t local_size = 32; \
  const int copiesPerWorkItem = 5; \
\
  /* Setup kernel and buffers */\
  OCL_CREATE_KERNEL_FROM_FILE("compiler_async_copy", "compiler_async_copy_" # KER_TYPE # VEC_SIZE); \
  OCL_CREATE_BUFFER(buf[0], 0, n * copiesPerWorkItem * sizeof(TYPE) * VEC_SIZE, NULL); \
  OCL_CREATE_BUFFER(buf[1], 0, n * copiesPerWorkItem * sizeof(TYPE) * VEC_SIZE, NULL); \
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]); \
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]); \
  OCL_SET_ARG(2, local_size*copiesPerWorkItem*sizeof(TYPE)*VEC_SIZE, NULL); \
  OCL_SET_ARG(3, sizeof(int), &copiesPerWorkItem); \
\
  OCL_MAP_BUFFER(1); \
  for (uint32_t i = 0; i < n * copiesPerWorkItem * VEC_SIZE; ++i) \
      ((TYPE*)buf_data[1])[i] = rand(); \
  OCL_UNMAP_BUFFER(1); \
\
  /* Run the kernel */\
  globals[0] = n; \
  locals[0] = local_size; \
  OCL_NDRANGE(1); \
  OCL_MAP_BUFFER(0); \
  OCL_MAP_BUFFER(1); \
\
  /* Check results */\
  TYPE *dst = (TYPE*)buf_data[0]; \
  TYPE *src = (TYPE*)buf_data[1]; \
  for (uint32_t i = 0; i < n * copiesPerWorkItem * VEC_SIZE; i++) \
    OCL_ASSERT(dst[i] == src[i]); \
  OCL_UNMAP_BUFFER(0); \
  OCL_UNMAP_BUFFER(1); \
} \
\
MAKE_UTEST_FROM_FUNCTION(compiler_async_copy_##KER_TYPE##VEC_SIZE);

DEF(char, char, 2);
DEF(uchar, uchar, 2);
DEF(short, short, 2);
DEF(ushort, ushort, 2);
DEF(int, int, 2);
DEF(uint, uint, 2);
DEF(int64_t, long, 2);
DEF(uint64_t, ulong, 2);
DEF(float, float, 2);
//DEF(double, double, 2);
