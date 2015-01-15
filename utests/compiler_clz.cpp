#include "utest_helper.hpp"

namespace {

template <typename U>
U get_max()
{
  int shift_bit = sizeof(U)*8;
  U u_max = 0;
  for (int i = 0; i < shift_bit; i++)
    u_max |= 1<<(shift_bit-i-1);
  return u_max;
}

template<typename U>
void test(const char *kernel_name)
{
  const size_t n = 64;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_clz", kernel_name);
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(U), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(U), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  U max = get_max<U>();

  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i) {
      ((U*)buf_data[0])[i] = max >> i;
  }
  OCL_UNMAP_BUFFER(0);

  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < n; ++i) {
    if(sizeof(U) == 1 && i < 8 )
      OCL_ASSERT(((U*)buf_data[1])[i] == (i+24) );
    else if(sizeof(U) == 2 && i < 16 )
      OCL_ASSERT(((U*)buf_data[1])[i] == (i+16) );
    else if(sizeof(U) == 4 && i < 32 )
      OCL_ASSERT(((U*)buf_data[1])[i] == i );
    else if(sizeof(U) == 8 && i < 32 )
      OCL_ASSERT(((U*)buf_data[1])[i] == 0 );
    else if(sizeof(U) == 8 && i > 31)
      OCL_ASSERT(((U*)buf_data[1])[i] == (i-32) );
  }
  OCL_UNMAP_BUFFER(1);

}

}

#define compiler_clz(type, kernel) \
static void compiler_clz_ ##type(void)\
{\
  test<type>(# kernel);\
}\
MAKE_UTEST_FROM_FUNCTION(compiler_clz_ ## type);

compiler_clz(uint64_t, compiler_clz_ulong)
compiler_clz(uint32_t, compiler_clz_uint)
compiler_clz(uint16_t, compiler_clz_ushort)
compiler_clz(uint8_t, compiler_clz_uchar)
