#define COMPILER_CLZ(TYPE) \
    kernel void compiler_clz_##TYPE(global TYPE* src, global TYPE* dst)   \
{                                                \
  __global TYPE* A = &src[get_global_id(0)];    \
  __global TYPE* B = &dst[get_global_id(0)];    \
  *B =  clz(*A);   \
}

COMPILER_CLZ(ulong)
COMPILER_CLZ(uint)
COMPILER_CLZ(ushort)
COMPILER_CLZ(uchar)
COMPILER_CLZ(long)
COMPILER_CLZ(int)
COMPILER_CLZ(short)
COMPILER_CLZ(char)
