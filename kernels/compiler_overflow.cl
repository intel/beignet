#define COMPILER_OVERFLOW_ADD(TYPE, FUNC) \
    kernel void compiler_overflow_##TYPE##_##FUNC (global TYPE* src0, global TYPE* src1, global TYPE* dst)   \
{                                                \
  __global TYPE* A = &src0[get_global_id(0)];    \
  __global TYPE* B = &src1[get_global_id(0)];    \
  __global TYPE* C = &dst[get_global_id(0)];    \
  *C = *A + *B;    \
  TYPE carry = -convert_##TYPE(*C < *B); \
               \
  (*C).y += carry.x;  \
  carry.y += ((*C).y < carry.x); \
  (*C).z += carry.y;   \
    \
  carry.z += ((*C).z < carry.y); \
  (*C).w += carry.z; \
  carry.w += ((*C).w < carry.z); \
}


COMPILER_OVERFLOW_ADD(ulong4, add)
COMPILER_OVERFLOW_ADD(uint4, add)
COMPILER_OVERFLOW_ADD(ushort4, add)
COMPILER_OVERFLOW_ADD(uchar4, add)

#define COMPILER_OVERFLOW_SUB(TYPE, FUNC) \
    kernel void compiler_overflow_##TYPE##_##FUNC (global TYPE* src0, global TYPE* src1, global TYPE* dst)   \
{                                                \
  __global TYPE* A = &src0[get_global_id(0)];    \
  __global TYPE* B = &src1[get_global_id(0)];    \
  __global TYPE* C = &dst[get_global_id(0)];    \
  TYPE borrow; \
  unsigned result; \
  size_t num = sizeof(*A)/sizeof((*A)[0]); \
  for (uint i = 0; i < num; i++ ) {\
     borrow[i] = __builtin_usub_overflow((*A)[i], (*B)[i], &result); \
     (*C)[i] = result;  \
   }\
\
  for (uint i = 0; i < num-1; i++ ) {\
    borrow[i+1] += (*C)[i+1] < borrow[i];(*C)[i+1] -= borrow[i]; \
  }\
\
}

COMPILER_OVERFLOW_SUB(uint4, sub)
