#include "utest_helper.hpp"

namespace {

typedef struct {
  uint32_t x;
  uint32_t y;
  uint32_t z;
  uint32_t w;
} uint4;

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t z;
  uint16_t w;
} ushort4;

typedef struct {
  uint8_t x;
  uint8_t y;
  uint8_t z;
  uint8_t w;
} uchar4;

template<typename T>
void test(const char *kernel_name, int s_type)
{
  const size_t n = 16;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_overflow", kernel_name);
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(T), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(T), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i) {
    ((T*)buf_data[0])[i].x = s_type?CL_INT_MAX:CL_UINT_MAX;
    ((T*)buf_data[0])[i].y = s_type?CL_INT_MAX:CL_UINT_MAX;
    ((T*)buf_data[0])[i].z = s_type?CL_INT_MAX:CL_UINT_MAX;
    ((T*)buf_data[0])[i].w = i;
  }
  OCL_UNMAP_BUFFER(0);

  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < 16; ++i) {
    OCL_ASSERT(((T*)buf_data[1])[i].x == 0);
    OCL_ASSERT(((T*)buf_data[1])[i].y == 1);
    OCL_ASSERT(((T*)buf_data[1])[i].z == 1);
    OCL_ASSERT(((T*)buf_data[1])[i].w == i+2);
  }
  OCL_UNMAP_BUFFER(1);
}

}

#define compiler_overflow(type, kernel, s_type) \
static void compiler_overflow_ ##type(void)\
{\
  test<type>(# kernel, s_type);\
}\
MAKE_UTEST_FROM_FUNCTION(compiler_overflow_ ## type);

compiler_overflow(uint4, compiler_overflow_uint4, 0)
compiler_overflow(ushort4, compiler_overflow_ushort4, 0)
compiler_overflow(uchar4, compiler_overflow_uchar4, 0)
