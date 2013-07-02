#include "utest_helper.hpp"
#include <cmath>
#include <algorithm>
#include <string.h>

#define GROUP_NUM 16
#define LOCAL_SIZE 256
static void cpu_compiler_atomic(int *dst, int *src)
{
  dst[4] = 0xffffffff;
  int tmp[16] = { 0 };
  tmp[4] = -1;
  for(int j=0; j<LOCAL_SIZE; j++) {
    int i = j % 12;

    switch(i) {
      case 0: tmp[i] += 1; break;
      case 1: tmp[i] -= 1; break;
      case 2: tmp[i] += src[j]; break;
      case 3: tmp[i] -= src[j]; break;
      case 4: tmp[i] &= ~(src[j]<<(j>>4)); break;
      case 5: tmp[i] |= src[j]<<(j>>4); break;
      case 6: tmp[i] ^= src[j]; break;
      case 7: tmp[i] = tmp[i] < -src[j] ? tmp[i] : -src[j]; break;
      case 8: tmp[i] = tmp[i] > src[j] ? tmp[i] : src[j]; break;
      case 9: tmp[i] = (unsigned int)tmp[i] < (unsigned int)(-src[j]) ? tmp[i] : -src[j]; break;
      case 10: tmp[i] = (unsigned int)tmp[i] > (unsigned int)(src[j]) ? tmp[i] : src[j]; break;
      case 11:  tmp[i] = src[10]; break;
      default:  break;
    }
  }

  for(int k=0; k<GROUP_NUM; k++) {
    for(int j=0; j<LOCAL_SIZE; j++) {
      int i = j % 12;

      switch(i) {
        case 0: dst[i] += 1; break;
        case 1: dst[i] -= 1; break;
        case 2: dst[i] += src[j]; break;
        case 3: dst[i] -= src[j]; break;
        case 4: dst[i] &= ~(src[j]<<(j>>4)); break;
        case 5: dst[i] |= src[j]<<(j>>4); break;
        case 6: dst[i] ^= src[j]; break;
        case 7: dst[i] = dst[i] < -src[j] ? dst[i] : -src[j]; break;
        case 8: dst[i] = dst[i] > src[j] ? dst[i] : src[j]; break;
        case 9: dst[i] = (unsigned int)dst[i] < (unsigned int)(-src[j]) ? dst[i] : -src[j]; break;
        case 10: dst[i] = (unsigned int)dst[i] > (unsigned int)(src[j]) ? dst[i] : src[j]; break;
        case 11:  dst[i] = src[10]; break;
        default:  break;
      }
    }
  }

  for(int i=0; i<12; i++)
    dst[i+12] = tmp[i];
}

static void compiler_atomic_functions(void)
{
  const size_t n = GROUP_NUM * LOCAL_SIZE;
  int cpu_dst[24] = {0}, cpu_src[256];

  globals[0] = n;
  locals[0] = LOCAL_SIZE;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_atomic_functions");
  OCL_CREATE_BUFFER(buf[0], 0, 24 * sizeof(int), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, locals[0] * sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, 16 * sizeof(int), NULL);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[1]);

  OCL_MAP_BUFFER(0);
  memset(buf_data[0], 0, 24 * sizeof(int));
  ((int *)buf_data[0])[4] = -1;
  OCL_UNMAP_BUFFER(0);

  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < locals[0]; ++i)
      cpu_src[i] = ((int*)buf_data[1])[i] = rand() & 0xff;
  cpu_compiler_atomic(cpu_dst, cpu_src);
  OCL_UNMAP_BUFFER(1);
  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(0);

  // Check results
  for(int i=0; i<24; i++) {
    //printf("The dst(%d) gpu(0x%x) cpu(0x%x)\n", i, ((uint32_t *)buf_data[0])[i], cpu_dst[i]);
    OCL_ASSERT(((int *)buf_data[0])[i] == cpu_dst[i]);
  }
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_atomic_functions)
