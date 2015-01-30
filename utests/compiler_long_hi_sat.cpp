#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

static void __u64_mul_u64(uint64_t sourceA, uint64_t sourceB, uint64_t &destLow, uint64_t &destHi)
{
  uint64_t lowA, lowB;
  uint64_t highA, highB;

  lowA = sourceA & 0xffffffff;
  highA = sourceA >> 32;
  lowB = sourceB & 0xffffffff;
  highB = sourceB >> 32;

  uint64_t aHibHi = highA * highB;
  uint64_t aHibLo = highA * lowB;
  uint64_t aLobHi = lowA * highB;
  uint64_t aLobLo = lowA * lowB;

  uint64_t aLobLoHi = aLobLo >> 32;
  uint64_t aLobHiLo = aLobHi & 0xFFFFFFFFULL;
  aHibLo += aLobLoHi + aLobHiLo;

  destHi = aHibHi + (aHibLo >> 32 ) + (aLobHi >> 32);    // Cant overflow
  destLow = (aHibLo << 32) | ( aLobLo & 0xFFFFFFFFULL);
}

static void __64_mul_64(int64_t sourceA, int64_t sourceB, uint64_t &destLow, int64_t &destHi)
{
  int64_t aSign = sourceA >> 63;
  int64_t bSign = sourceB >> 63;
  int64_t resultSign = aSign ^ bSign;

  // take absolute values of the argument
  sourceA = (sourceA ^ aSign) - aSign;
  sourceB = (sourceB ^ bSign) - bSign;

  uint64_t hi;
  __u64_mul_u64( (uint64_t) sourceA, (uint64_t) sourceB, destLow, hi );

  // Fix the sign
  if( resultSign ) {
    destLow ^= resultSign;
    hi ^= resultSign;
    destLow -= resultSign;
    //carry if necessary
    if( 0 == destLow )
      hi -= resultSign;
  }

  destHi = (int64_t) hi;
}

static void __mad_sat(int64_t sourceA, int64_t sourceB, int64_t sourceC, int64_t& dst)
{
  cl_long multHi;
  cl_ulong multLo;
  __64_mul_64(sourceA, sourceB, multLo, multHi);
  cl_ulong sum = multLo + sourceC;

  // carry if overflow
  if(sourceC >= 0) {
    if(multLo > sum) {
      multHi++;
      if(CL_LONG_MIN == multHi) {
        multHi = CL_LONG_MAX;
        sum = CL_ULONG_MAX;
      }
    }
  } else {
    if( multLo < sum ) {
      multHi--;
      if( CL_LONG_MAX == multHi ) {
        multHi = CL_LONG_MIN;
        sum = 0;
      }
    }
  }

  // saturate
  if( multHi > 0 )
    sum = CL_LONG_MAX;
  else if ( multHi == 0 && sum > CL_LONG_MAX)
    sum = CL_LONG_MAX;
  else if ( multHi == -1 && sum < (cl_ulong)CL_LONG_MIN)
    sum = CL_LONG_MIN;
  else if( multHi < -1 )
    sum = CL_LONG_MIN;

  dst = (cl_long) sum;
}

void compiler_long_mul_hi(void)
{
  const size_t n = 32;
  int64_t src[n];
  int64_t num0 = 0xF00A00CED0090B0CUL;
  int64_t num1 = 0x7FABCD57FC098FC1UL;
  memset(src, 0, sizeof(int64_t) * n);

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_hi_sat", "compiler_long_mul_hi");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint64_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint64_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_long), &num0);
  OCL_SET_ARG(3, sizeof(cl_long), &num1);
  globals[0] = n;
  locals[0] = 32;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    uint64_t a = rand();
    a = a <<32 | a;
    src[i] = a;
  }

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, sizeof(uint64_t) * n);
  OCL_UNMAP_BUFFER(0);

  uint64_t res_lo;
  int64_t res_hi;

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    if (i % 2 == 0)
      __64_mul_64(src[i], num0, res_lo, res_hi);
    else
      __64_mul_64(src[i], num1, res_lo, res_hi);

    OCL_ASSERT(((int64_t *)(buf_data[1]))[i] == res_hi);
  }
  OCL_UNMAP_BUFFER(1);
}

void compiler_long_mul_sat(void)
{
  const size_t n = 32;
  int64_t src[n];
  int64_t num0 = 0xF00000CED8090B0CUL;
  int64_t num1 = 0x0000000000098FC1UL;
  memset(src, 0, sizeof(int64_t) * n);

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_hi_sat", "compiler_long_mul_sat");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint64_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint64_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_long), &num0);
  OCL_SET_ARG(3, sizeof(cl_long), &num1);
  globals[0] = n;
  locals[0] = 32;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    uint64_t a = rand();
    a = a <<32 | a;
    src[i] = a;
  }

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, sizeof(uint64_t) * n);
  OCL_UNMAP_BUFFER(0);

  int64_t res;

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    __mad_sat(src[i], num0, num1, res);

    OCL_ASSERT(((int64_t *)(buf_data[1]))[i] == res);
  }
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_long_mul_hi);
MAKE_UTEST_FROM_FUNCTION(compiler_long_mul_sat);
