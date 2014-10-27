#define COMPILER_OVERFLOW(TYPE) \
    kernel void compiler_overflow_##TYPE (global TYPE* src, global TYPE* dst)   \
{                                                \
  __global TYPE* A = &src[get_global_id(0)];    \
  TYPE B = 1; \
  *A += B;    \
  TYPE carry = -convert_##TYPE((*A) < B); \
               \
  (*A).y += carry.x;  \
  carry.y += ((*A).y < carry.x); \
  (*A).z += carry.y;   \
    \
  carry.z += ((*A).z < carry.y); \
  (*A).w += carry.z; \
   dst[get_global_id(0)] = src[get_global_id(0)]; \
}

COMPILER_OVERFLOW(uint4)
COMPILER_OVERFLOW(ushort4)
COMPILER_OVERFLOW(uchar4)
