#include "utest_helper.hpp"

namespace {

template<typename T>
T get_max();

#define DEF_TEMPLATE(TYPE, NAME)                                    \
template <>                                                         \
TYPE get_max<TYPE>()                                                \
{                                                                   \
  static TYPE max = CL_##NAME##_MAX;                                \
  return max;                                                       \
}                                                                   \
                                                                    \
template <>                                                         \
u##TYPE get_max<u##TYPE>()                                          \
{                                                                   \
  static u##TYPE max = CL_U##NAME##_MAX;                            \
  return max;                                                       \
}

DEF_TEMPLATE(int8_t, CHAR)
DEF_TEMPLATE(int16_t, SHRT)
DEF_TEMPLATE(int32_t, INT)
DEF_TEMPLATE(int64_t, LONG)

template<typename T>
void test(const char *kernel_name, int s_type)
{
  const int n = sizeof(T) * 8;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_popcount", kernel_name);
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(T), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(T), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = n;

  OCL_MAP_BUFFER(0);
  ((T*)buf_data[0])[0] = 0;
  for (int32_t i = 1; i < (int32_t) n; ++i){
    ((T*)buf_data[0])[i] = get_max<T>() >> i;
  }
  OCL_UNMAP_BUFFER(0);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(1);
  OCL_ASSERT(((T*)buf_data[1])[0] == 0);
  for (int i = 1; i < n; ++i){
    OCL_ASSERT(((T*)buf_data[1])[i] == n-i-s_type);
  }
  OCL_UNMAP_BUFFER(1);
}

}

#define compiler_popcount(type, kernel, s_type) \
static void compiler_popcount_ ##type(void)\
{\
  test<type>(# kernel, s_type);\
}\
MAKE_UTEST_FROM_FUNCTION(compiler_popcount_ ## type);

compiler_popcount(int8_t, test_char, 1)
compiler_popcount(uint8_t, test_uchar, 0)
compiler_popcount(int16_t, test_short, 1)
compiler_popcount(uint16_t, test_ushort, 0)
compiler_popcount(int32_t, test_int, 1)
compiler_popcount(uint32_t, test_uint, 0)
compiler_popcount(int64_t, test_long, 1)
compiler_popcount(uint64_t, test_ulong, 0)
