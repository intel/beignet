#include "utest_helper.hpp"

namespace {

typedef struct {
  unsigned long x;
  unsigned long y;
  unsigned long z;
  unsigned long w;
}ulong4;

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

template <typename U>
U get_max()
{
  int shift_bit = sizeof(U)*8;
  U u_max = 0;
  for (int i = 0; i < shift_bit; i++)
    u_max |= 1<<(shift_bit-i-1);
  return u_max;
}

template<typename T, typename U>
void test(const char *kernel_name, int func_type)
{
  const size_t n = 16;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_overflow", kernel_name);
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(T), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(T), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(T), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);

  U max = get_max<U>();

  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i) {
    if(func_type == 0) {
      ((T*)buf_data[0])[i].x = max;
      ((T*)buf_data[0])[i].y = max;
      ((T*)buf_data[0])[i].z = max;
      ((T*)buf_data[0])[i].w = i;
    }else if(func_type == 1) {
      ((T*)buf_data[0])[i].x = 0;
      ((T*)buf_data[0])[i].y = 0;
      ((T*)buf_data[0])[i].z = 0;
      ((T*)buf_data[0])[i].w = n+2-i;
    }else
      OCL_ASSERT(0);
  }
  OCL_UNMAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < n; ++i) {
      ((T*)buf_data[1])[i].x = 1;
      ((T*)buf_data[1])[i].y = 1;
      ((T*)buf_data[1])[i].z = 1;
      ((T*)buf_data[1])[i].w = 1;
  }
  OCL_UNMAP_BUFFER(1);

  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(2);
  for (uint32_t i = 0; i < 16; ++i) {
   // printf("%u,%u,%u,%u\n", ((T*)buf_data[2])[i].x,((T*)buf_data[2])[i].y, ((T*)buf_data[2])[i].z, ((T*)buf_data[2])[i].w  );
    if(func_type == 0) {
      OCL_ASSERT(((T*)buf_data[2])[i].x == 0);
      OCL_ASSERT(((T*)buf_data[2])[i].y == 1);
      OCL_ASSERT(((T*)buf_data[2])[i].z == 1);
      OCL_ASSERT(((T*)buf_data[2])[i].w == i+2);
    }else if(func_type == 1) {
      OCL_ASSERT(((T*)buf_data[2])[i].x == max);
      OCL_ASSERT(((T*)buf_data[2])[i].y == max-1);
      OCL_ASSERT(((T*)buf_data[2])[i].z == max-1);
      OCL_ASSERT(((T*)buf_data[2])[i].w == n-i);
    }else
      OCL_ASSERT(0);
  }
  OCL_UNMAP_BUFFER(2);
}

}

#define compiler_overflow_add(type, subtype, kernel, func_type) \
static void compiler_overflow_add_ ##type(void)\
{\
  test<type, subtype>(# kernel, func_type);\
}\
MAKE_UTEST_FROM_FUNCTION(compiler_overflow_add_ ## type);

#define compiler_overflow_sub(type, subtype, kernel, func_type) \
static void compiler_overflow_sub_ ##type(void)\
{\
  test<type, subtype>(# kernel, func_type);\
}\
MAKE_UTEST_FROM_FUNCTION(compiler_overflow_sub_ ## type);

compiler_overflow_add(ulong4, unsigned long, compiler_overflow_ulong4_add, 0)
compiler_overflow_add(uint4, uint32_t, compiler_overflow_uint4_add, 0)
compiler_overflow_add(ushort4, uint16_t, compiler_overflow_ushort4_add, 0)
compiler_overflow_add(uchar4, uint8_t, compiler_overflow_uchar4_add, 0)

// as llvm intrincs function doesn't support byte/short overflow,
// we just test uint overflow here.
compiler_overflow_sub(uint4, uint32_t, compiler_overflow_uint4_sub, 1)
