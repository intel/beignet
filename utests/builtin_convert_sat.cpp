#include <cstdint>
#include "utest_helper.hpp"

typedef unsigned char uchar;
typedef unsigned short ushort;

int64_t my_rand(void) {
  int64_t x = rand() - RAND_MAX/2;
  int64_t y = rand() - RAND_MAX/2;
  return x * y;
}

#define DEF2(DST_TYPE, SRC_TYPE, DST_MIN, DST_MAX, REAL_SRC_TYPE) \
void builtin_convert_ ## SRC_TYPE ## _to_ ## DST_TYPE ## _sat(void) \
{ \
  const int n = 128; \
  OCL_CREATE_KERNEL_FROM_FILE("builtin_convert_sat", "builtin_convert_" # SRC_TYPE "_to_" # DST_TYPE "_sat"); \
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(REAL_SRC_TYPE), NULL); \
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(DST_TYPE), NULL); \
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]); \
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]); \
  globals[0] = n; \
  locals[0] = 16; \
  OCL_MAP_BUFFER(0); \
  for (int i = 0; i < n; i++) \
    ((REAL_SRC_TYPE *)buf_data[0])[i] = my_rand(); \
  OCL_UNMAP_BUFFER(0); \
  OCL_NDRANGE(1); \
  OCL_MAP_BUFFER(0); \
  OCL_MAP_BUFFER(1); \
  for (int i = 0; i < n; i++) { \
    REAL_SRC_TYPE src = ((REAL_SRC_TYPE *)buf_data[0])[i]; \
    DST_TYPE dst; \
    if ((double)src > (double)DST_MAX) \
      dst = DST_MAX; \
    else if ((double)src < (double)DST_MIN) \
      dst = DST_MIN; \
    else \
      dst = src; \
    OCL_ASSERT(((DST_TYPE *)buf_data[1])[i] == dst); \
  } \
  OCL_UNMAP_BUFFER(0); \
  OCL_UNMAP_BUFFER(1); \
} \
MAKE_UTEST_FROM_FUNCTION(builtin_convert_ ## SRC_TYPE ## _to_ ## DST_TYPE ## _sat);

#define DEF(DST_TYPE, SRC_TYPE, DST_MIN, DST_MAX) \
  DEF2(DST_TYPE, SRC_TYPE, DST_MIN, DST_MAX, SRC_TYPE)

DEF(char, uchar, -128, 127);
DEF(char, short, -128, 127);
DEF(char, ushort, -128, 127);
DEF(char, int, -128, 127);
DEF(char, uint, -128, 127);
DEF2(char, long, -128, 127, int64_t);
DEF(char, float, -128, 127);
DEF(uchar, char, 0, 255);
DEF(uchar, short, 0, 255);
DEF(uchar, ushort, 0, 255);
DEF(uchar, int, 0, 255);
DEF(uchar, uint, 0, 255);
DEF2(uchar, long, 0, 255, int64_t);
DEF(uchar, float, 0, 255);
DEF(short, ushort, -32768, 32767);
DEF(short, int, -32768, 32767);
DEF(short, uint, -32768, 32767);
DEF2(short, long, -32768, 32767, int64_t);
DEF(short, float, -32768, 32767);
DEF(ushort, short, 0, 65535);
DEF(ushort, int, 0, 65535);
DEF(ushort, uint, 0, 65535);
DEF2(ushort, long, 0, 65535, int64_t);
DEF(ushort, float, 0, 65535);
DEF(int, uint, -0x7FFFFFFF-1, 0x7FFFFFFF);
DEF2(int, long, -0x7FFFFFFF-1, 0x7FFFFFFF, int64_t);
DEF(int, float, -0x7FFFFFFF-1, 0x7FFFFFFF);
DEF(uint, int, 0, 0xffffffffu);
DEF2(uint, long, 0, 0xffffffffu, int64_t);
DEF(uint, float, 0, 0xffffffffu);
#undef DEF
