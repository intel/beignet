#include "utest_helper.hpp"

namespace {

template<typename T>
T get_max();

#define DEF_TEMPLATE_MAX(TYPE, NAME)                                \
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

DEF_TEMPLATE_MAX(int8_t, CHAR)
DEF_TEMPLATE_MAX(int16_t, SHRT)
DEF_TEMPLATE_MAX(int32_t, INT)
DEF_TEMPLATE_MAX(int64_t, LONG)

template<typename T>
T get_min();

#define DEF_TEMPLATE_MIN(TYPE, NAME)                                \
template <>                                                         \
TYPE get_min<TYPE>()                                                \
{                                                                   \
  static TYPE min = CL_##NAME##_MIN;                                \
  return min;                                                       \
}                                                                   \
                                                                    \
template <>                                                         \
u##TYPE get_min<u##TYPE>()                                          \
{                                                                   \
  static u##TYPE min = 0;                                           \
  return min;                                                       \
}

DEF_TEMPLATE_MIN(int8_t, CHAR)
DEF_TEMPLATE_MIN(int16_t, SHRT)
DEF_TEMPLATE_MIN(int32_t, INT)
DEF_TEMPLATE_MIN(int64_t, LONG)

template<typename U>
void test(const char *kernel_name, int s_type)
{
  const size_t n = 64;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_clz", kernel_name);
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(U), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(U), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  U max = get_max<U>();
  U min = get_min<U>();

  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i) {
      ((U*)buf_data[0])[i] = max >> i;
      if(i == sizeof(U)*8)
        ((U*)buf_data[0])[i] = min;
  }

  OCL_UNMAP_BUFFER(0);

  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(1);
  // for unsigned type.
  if(s_type == 0)
  {
    for (uint32_t i = 0; i < n; ++i) {
      if(sizeof(U) == 1 && i < 8 )
        OCL_ASSERT(((U*)buf_data[1])[i] == (U)i );
      else if(sizeof(U) == 2 && i < 16 )
        OCL_ASSERT(((U*)buf_data[1])[i] == (U)i );
      else if(sizeof(U) == 4 && i < 32 )
        OCL_ASSERT(((U*)buf_data[1])[i] == (U)i );
      else if(sizeof(U) == 8 && i < 64 )
        OCL_ASSERT(((U*)buf_data[1])[i] == (U)i );
    }
  }
  else  // signed type
  {
    for (uint32_t i = 0; i < n; ++i) {
      if(sizeof(U) == 1)
      {
        if( i < 8 )
          OCL_ASSERT(((U*)buf_data[1])[i] == (U)i+1 );
        else if( i == 8 )
          OCL_ASSERT(((U*)buf_data[1])[i] == 0 );
      }
      else if(sizeof(U) == 2)
      {
        if( i < 16 )
          OCL_ASSERT(((U*)buf_data[1])[i] == (U)i+1 );
        else if( i == 16 )
          OCL_ASSERT(((U*)buf_data[1])[i] == 0 );
      }
      else if(sizeof(U) == 4)
      {
        if( i < 32 )
          OCL_ASSERT(((U*)buf_data[1])[i] == (U)i+1 );
        else if( i == 32 )
          OCL_ASSERT(((U*)buf_data[1])[i] == 0 );
      }
      else if(sizeof(U) == 8)
      {
        if( i < 63 )
          OCL_ASSERT(((U*)buf_data[1])[i] == (U)i+1 );
      }
    }
  }
  OCL_UNMAP_BUFFER(1);

}

}

#define compiler_clz(type, kernel, s_type)\
static void compiler_clz_ ##type(void)\
{\
  test<type>(# kernel, s_type);\
}\
MAKE_UTEST_FROM_FUNCTION(compiler_clz_ ## type);

compiler_clz(uint64_t, compiler_clz_ulong, 0)
compiler_clz(uint32_t, compiler_clz_uint, 0)
compiler_clz(uint16_t, compiler_clz_ushort, 0)
compiler_clz(uint8_t, compiler_clz_uchar, 0)
compiler_clz(int64_t, compiler_clz_long, 1)
compiler_clz(int32_t, compiler_clz_int, 1)
compiler_clz(int16_t, compiler_clz_short, 1)
compiler_clz(int8_t, compiler_clz_char, 1)
