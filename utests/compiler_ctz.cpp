#include "utest_helper.hpp"

namespace {

template<typename T>
T get_max();

template<typename U>
void test(const char *kernel_name)
{
  const size_t n = 65;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_ctz", kernel_name);
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(U), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(U), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i) {
      ((U*)buf_data[0])[i] = 1ll << i;
      if(i == sizeof(U)*8)
        ((U*)buf_data[0])[i] = 0;
  }

  OCL_UNMAP_BUFFER(0);

  globals[0] = n;
  locals[0] = 1;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < n; ++i) {
      if(sizeof(U) == 1 && i <= 8 )
        OCL_ASSERT(((U*)buf_data[1])[i] == (U)i );
      else if(sizeof(U) == 2 && i <= 16 )
        OCL_ASSERT(((U*)buf_data[1])[i] == (U)i );
      else if(sizeof(U) == 4 && i <= 32 )
        OCL_ASSERT(((U*)buf_data[1])[i] == (U)i );
      else if(sizeof(U) == 8 && i <= 64 )
        OCL_ASSERT(((U*)buf_data[1])[i] == (U)i );
    }
  OCL_UNMAP_BUFFER(1);

}
}

#define compiler_ctz(type, kernel)\
static void compiler_ctz_ ##type(void)\
{\
  test<type>(# kernel);\
}\
MAKE_UTEST_FROM_FUNCTION(compiler_ctz_ ## type);

compiler_ctz(uint64_t, compiler_ctz_ulong)
compiler_ctz(uint32_t, compiler_ctz_uint)
compiler_ctz(uint16_t, compiler_ctz_ushort)
compiler_ctz(uint8_t, compiler_ctz_uchar)
compiler_ctz(int64_t, compiler_ctz_long)
compiler_ctz(int32_t, compiler_ctz_int)
compiler_ctz(int16_t, compiler_ctz_short)
compiler_ctz(int8_t, compiler_ctz_char)
