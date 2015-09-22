#include "utest_helper.hpp"
#include "string.h"

#define cpu_htons(A)     ((((uint16_t)(A) & 0xff00) >> 8) | \
    (((uint16_t)(A) & 0x00ff) << 8))
#define cpu_htonl(A)     ((((uint32_t)(A) & 0xff000000) >> 24) | \
    (((uint32_t)(A) & 0x00ff0000) >> 8) | \
    (((uint32_t)(A) & 0x0000ff00) << 8) | \
    (((uint32_t)(A) & 0x000000ff) << 24))
#define cpu_htonll(A)     ((((uint64_t)(A) & 0xff00000000000000) >> 56) | \
    (((uint64_t)(A) & 0x00ff000000000000) >> 40) | \
    (((uint64_t)(A) & 0x0000ff0000000000) >> 24) | \
    (((uint64_t)(A) & 0x000000ff00000000) >> 8) |  \
    (((uint64_t)(A) & 0x00000000ff000000) << 8) |  \
    (((uint64_t)(A) & 0x0000000000ff0000) << 24) | \
    (((uint64_t)(A) & 0x000000000000ff00) << 40) | \
    (((uint64_t)(A) & 0x00000000000000ff) << 56) )


template <typename T> static void gen_rand_val(T & val)
{
  val = static_cast<T>(rand());//(0xAABBCCDD);//
}

template <typename T> static void cpu(int global_id, T *src, T *dst)
{
  T f = src[global_id];
  T g = 0;
  if (sizeof(T) == sizeof(int16_t))
    g = cpu_htons(f);
  else if (sizeof(T) == sizeof(int32_t))
    g = cpu_htonl(f);
  else if (sizeof(T) == sizeof(int64_t))
    g = cpu_htonll(f);
  dst[global_id] = g;
}

template <typename T> static void cpu(int global_id, T src, T *dst)
{
  T f = src;
  T g = 0;
  if (sizeof(T) == sizeof(int16_t))
    g = cpu_htons(f);
  else if (sizeof(T) == sizeof(int32_t))
    g = cpu_htonl(f);
  else if (sizeof(T) == sizeof(int64_t))
    g = cpu_htonll(f);
  dst[global_id] = g;
}

template <typename T> inline static void print_data(T& val)
{
  if(sizeof(T) == sizeof(uint16_t))
    printf(" 0x%hx", (uint16_t)val);
  else if(sizeof(T) == sizeof(uint32_t))
    printf(" 0x%x", (uint32_t)val);
  else if(sizeof(T) == sizeof(uint64_t))
    printf(" 0x%lx", (uint64_t)val);
}

template <typename T> static void dump_data(T* raw, T* cpu, T* gpu, int n)
{
  printf("\nRaw: \n");
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    print_data(raw[i]);
  }

  printf("\nCPU: \n");
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    print_data(cpu[i]);
  }
  printf("\nGPU: \n");
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    print_data(gpu[i]);
  }
}

template <typename T> static void dump_data(T raw, T* cpu, T* gpu, int n)
{
  printf("\nRaw: \n");
  print_data(raw);

  printf("\nCPU: \n");
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    print_data(cpu[i]);
  }
  printf("\nGPU: \n");
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    print_data(gpu[i]);
  }
}

void compiler_bswap(void)
{
  const size_t n = 16;
  uint32_t src0[n];
  uint16_t src1[n];
  uint32_t dst0[n];
  uint16_t dst1[n];
  int32_t src2 = static_cast<int32_t>(rand());
  int32_t dst2[n];
  int16_t src3 = static_cast<int16_t>(rand());
  int16_t dst3[n];
  uint64_t src4[n];
  uint64_t dst4[n];
  int64_t src5 = static_cast<int64_t>(rand()) << 32| static_cast<int64_t>(rand());
  int64_t dst5[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_bswap", "compiler_bswap");
  OCL_CREATE_BUFFER(buf[0], 0, sizeof(src0), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_CREATE_BUFFER(buf[1], 0, sizeof(dst0), NULL);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  OCL_CREATE_BUFFER(buf[2], 0, sizeof(src1), NULL);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_CREATE_BUFFER(buf[3], 0, sizeof(dst1), NULL);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);

  OCL_SET_ARG(4, sizeof(int32_t), &src2);
  OCL_CREATE_BUFFER(buf[4], 0, sizeof(dst2), NULL);
  OCL_SET_ARG(5, sizeof(cl_mem), &buf[4]);

  OCL_SET_ARG(6, sizeof(int16_t), &src3);
  OCL_CREATE_BUFFER(buf[5], 0, sizeof(dst3), NULL);
  OCL_SET_ARG(7, sizeof(cl_mem), &buf[5]);

  OCL_CREATE_BUFFER(buf[6], 0, sizeof(src4), NULL);
  OCL_SET_ARG(8, sizeof(cl_mem), &buf[6]);
  OCL_CREATE_BUFFER(buf[7], 0, sizeof(dst4), NULL);
  OCL_SET_ARG(9, sizeof(cl_mem), &buf[7]);

  OCL_SET_ARG(10, sizeof(int64_t), &src5);
  OCL_CREATE_BUFFER(buf[8], 0, sizeof(dst5), NULL);
  OCL_SET_ARG(11, sizeof(cl_mem), &buf[8]);

  OCL_MAP_BUFFER(0);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    gen_rand_val(src0[i]);
  }
  memcpy(buf_data[0], src0, sizeof(src0));
  OCL_UNMAP_BUFFER(0);

  /* Clear the dst buffer to avoid random data. */
  OCL_MAP_BUFFER(1);
  memset(buf_data[1], 0, sizeof(dst0));
  OCL_UNMAP_BUFFER(1);

  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    gen_rand_val(src1[i]);
  }
  memcpy(buf_data[2], src1, sizeof(src1));
  OCL_UNMAP_BUFFER(2);

  /* Clear the dst buffer to avoid random data. */
  OCL_MAP_BUFFER(3);
  memset(buf_data[3], 0, sizeof(dst1));
  OCL_UNMAP_BUFFER(3);

  /* Clear the dst buffer to avoid random data. */
  OCL_MAP_BUFFER(4);
  memset(buf_data[4], 0, sizeof(dst2));
  OCL_UNMAP_BUFFER(4);

  /* Clear the dst buffer to avoid random data. */
  OCL_MAP_BUFFER(5);
  memset(buf_data[5], 0, sizeof(dst3));
  OCL_UNMAP_BUFFER(5);

  OCL_MAP_BUFFER(6);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    uint64_t x, y;
    gen_rand_val(x);
    gen_rand_val(y);
    src4[i] = (x << 32)| y;
  }
  memcpy(buf_data[6], src4, sizeof(src4));
  OCL_UNMAP_BUFFER(6);

  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    if (i%2) {
      dst0[i] = src0[i];
      continue;
    }
    cpu(i, src0, dst0);
  }

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    cpu(i, src1, dst1);

    if (i%2) {
      dst1[i] = dst1[i] + 1;
      cpu(i, dst1, dst1);
    }
  }

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i)
    cpu(i, src2, dst2);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i)
    cpu(i, src3, dst3);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i)
    cpu(i, src4, dst4);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i)
    cpu(i, src5, dst5);

  OCL_MAP_BUFFER(1);
  //dump_data(src0, dst0, (uint32_t *)buf_data[1], n);
  OCL_ASSERT(!memcmp(buf_data[1], dst0, sizeof(dst0)));
  OCL_UNMAP_BUFFER(1);

  OCL_MAP_BUFFER(3);
  //dump_data(src1, dst1, (uint16_t *)buf_data[3], n);
  OCL_ASSERT(!memcmp(buf_data[3], dst1, sizeof(dst1)));
  OCL_UNMAP_BUFFER(3);

  OCL_MAP_BUFFER(4);
  //dump_data(src2, dst2, (int32_t *)buf_data[4], n);
  OCL_ASSERT(!memcmp(buf_data[4], dst2, sizeof(dst2)));
  OCL_UNMAP_BUFFER(4);

  OCL_MAP_BUFFER(5);
  //dump_data(src3, dst3, (int16_t *)buf_data[5], n);
  OCL_ASSERT(!memcmp(buf_data[5], dst3, sizeof(dst3)));
  OCL_UNMAP_BUFFER(5);

  OCL_MAP_BUFFER(7);
  //dump_data(src4, dst4, (uint64_t *)buf_data[7], n);
  OCL_ASSERT(!memcmp(buf_data[7], dst4, sizeof(dst4)));
  OCL_UNMAP_BUFFER(7);

  OCL_MAP_BUFFER(8);
  //dump_data(src5, dst5, (int64_t *)buf_data[8], n);
  OCL_ASSERT(!memcmp(buf_data[8], dst5, sizeof(dst5)));
  OCL_UNMAP_BUFFER(8);
}

MAKE_UTEST_FROM_FUNCTION(compiler_bswap);
