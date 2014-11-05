#include "utest_helper.hpp"
#include "string.h"

namespace {
#define cpu_htons(A)     ((((uint16_t)(A) & 0xff00) >> 8) | \
    (((uint16_t)(A) & 0x00ff) << 8))
#define cpu_htonl(A)     ((((uint32_t)(A) & 0xff000000) >> 24) | \
    (((uint32_t)(A) & 0x00ff0000) >> 8) | \
    (((uint32_t)(A) & 0x0000ff00) << 8) | \
    (((uint32_t)(A) & 0x000000ff) << 24))

template <typename T> static void cpu(int global_id, T *src, T *dst)
{
    T f = src[global_id];
    T g = 0;
    if(sizeof(T) == sizeof(int16_t))
      g = cpu_htons(f);
    else if(sizeof(T) == sizeof(int32_t))
      g = cpu_htonl(f);
    dst[global_id] = g;
}

template <typename T> static void gen_rand_val (T & val)
{
    val = static_cast<T>(rand() );
}

template <typename T>
inline static void print_data (T& val)
{
    if(sizeof(T) == sizeof(uint16_t))
        printf(" %hx", val);
    else
        printf(" %x", val);
}

template <typename T> static void dump_data (T* src, T* dst, int n)
{
    printf("\nRaw: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((T *)buf_data[0])[i]);
    }

    printf("\nCPU: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(dst[i]);
    }
    printf("\nGPU: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((T *)buf_data[1])[i]);
    }
}

template<typename T>
void test(const char *kernel_name)
{
  const size_t n = 64;
  T cpu_dst[n];
  T cpu_src[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_bswap", kernel_name);
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(T), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(T), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  OCL_MAP_BUFFER(0);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    gen_rand_val(cpu_src[i]);
  }

  memcpy(buf_data[0], cpu_src, sizeof(T) * n);

  /* Clear the dst buffer to avoid random data. */
  OCL_MAP_BUFFER(1);
  memset(buf_data[1], 0, sizeof(T) * n);
  OCL_UNMAP_BUFFER(1);

  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i)
    cpu(i, cpu_src, cpu_dst);

  OCL_MAP_BUFFER(1);
 // dump_data(cpu_src, cpu_dst, n);

  OCL_ASSERT(!memcmp(buf_data[1], cpu_dst, sizeof(T) * n));

  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(0);
}

}

#define compiler_bswap(type, kernel) \
static void compiler_bswap_ ##type(void)\
{\
  test<type>(# kernel);\
}\
MAKE_UTEST_FROM_FUNCTION(compiler_bswap_ ## type);

compiler_bswap(int16_t, compiler_bswap_short)
compiler_bswap(uint16_t, compiler_bswap_ushort)
compiler_bswap(int32_t, compiler_bswap_int)
compiler_bswap(uint32_t, compiler_bswap_uint)
