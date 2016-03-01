#define COMPILER_CTZ(TYPE) \
    kernel void compiler_ctz_##TYPE(global TYPE* src, global TYPE* dst)   \
{                                                \
  __global TYPE* A = &src[get_global_id(0)];    \
  __global TYPE* B = &dst[get_global_id(0)];    \
  *B =  ctz(*A);   \
}

COMPILER_CTZ(ulong)
COMPILER_CTZ(uint)
COMPILER_CTZ(ushort)
COMPILER_CTZ(uchar)
COMPILER_CTZ(long)
COMPILER_CTZ(int)
COMPILER_CTZ(short)
COMPILER_CTZ(char)
