#define GENERIC_KERNEL(T)                                              \
kernel void compiler_generic_pointer_##T(global T *src, global T *dst) \
{                                                                      \
  size_t gid = get_global_id(0);                                       \
  size_t lid = get_local_id(0);                                        \
  private T pdata[16];                                                 \
  local T ldata[16];                                                   \
  generic T * p1 = &pdata[lid];                                        \
  generic T * p2 = &ldata[lid];                                        \
  generic T *p = (gid & 1) ? p1 : p2;                                  \
  /* below expression is not supported by clang now   */               \
  /* generic T *p = (gid & 1) ? p1 : (T *)&ldata[lid]; */              \
  *p = src[gid];                                                       \
  /* fill other data */                                                \
  if(gid&1) {                                                          \
    ldata[lid] = 20;                                                   \
  } else {                                                             \
    for (int i = 0; i < 16; i++) {                                     \
      pdata[i] = src[lid];;                                            \
    }                                                                  \
  }                                                                    \
  barrier(CLK_LOCAL_MEM_FENCE);                                        \
                                                                       \
  generic T * q1 = &pdata[lid];                                        \
  generic T * q2 = &ldata[lid];                                        \
  generic T *q = (gid & 1) ? q1 : q2;                                  \
  dst[gid] = *q + pdata[lid];                                          \
}

GENERIC_KERNEL(int)
GENERIC_KERNEL(char)
GENERIC_KERNEL(short)
GENERIC_KERNEL(long)
