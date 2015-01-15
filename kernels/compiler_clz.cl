#define COMPILER_CLZ(TYPE) \
    kernel void compiler_clz_##TYPE(global TYPE* src, global TYPE* dst)   \
{                                                \
  __global TYPE* A = &src[get_global_id(0)];    \
  __global TYPE* B = &dst[get_global_id(0)];    \
  *B =  __builtin_clz(*A);   \
}

COMPILER_CLZ(uint)
COMPILER_CLZ(ulong)
COMPILER_CLZ(ushort)
COMPILER_CLZ(uchar)
