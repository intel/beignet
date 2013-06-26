#define DECL_KERNEL_SUB(type)\
__kernel void \
compiler_sub_##type(__global type *src0, __global type *src1, __global type *dst) \
{ \
  int id = (int)get_global_id(0); \
  dst[id] = src0[id] - src1[id]; \
}

#define DECL_KERNEL_ADD(type)\
__kernel void \
compiler_add_##type(__global type *src0, __global type *src1, __global type *dst) \
{ \
  int id = (int)get_global_id(0); \
  dst[id] = src0[id] + src1[id]; \
}

#define DECL_KERNEL_MUL(type)\
__kernel void \
compiler_mul_##type(__global type *src0, __global type *src1, __global type *dst) \
{ \
  int id = (int)get_global_id(0); \
  dst[id] = src0[id] * src1[id]; \
}

#define DECL_KERNEL_DIV(type)\
__kernel void \
compiler_div_##type(__global type *src0, __global type *src1, __global type *dst) \
{ \
  int id = (int)get_global_id(0); \
  dst[id] = src0[id] / src1[id]; \
}

#define DECL_KERNEL_REM(type)\
__kernel void \
compiler_rem_##type(__global type *src0, __global type *src1, __global type *dst) \
{ \
  int id = (int)get_global_id(0); \
  dst[id] = src0[id] % src1[id]; \
}

#define DECL_KERNEL_FOR_ALL_TYPE(op) \
DECL_KERNEL_##op(char)               \
DECL_KERNEL_##op(uchar)              \
DECL_KERNEL_##op(short)              \
DECL_KERNEL_##op(ushort)             \
DECL_KERNEL_##op(int)                \
DECL_KERNEL_##op(uint)

DECL_KERNEL_FOR_ALL_TYPE(SUB)
DECL_KERNEL_FOR_ALL_TYPE(ADD)
DECL_KERNEL_FOR_ALL_TYPE(MUL)
DECL_KERNEL_FOR_ALL_TYPE(DIV)
DECL_KERNEL_FOR_ALL_TYPE(REM)
