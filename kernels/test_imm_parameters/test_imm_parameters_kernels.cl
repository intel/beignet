__kernel void test_imm_parameters (__global int *dst,
                                   const    float a0, 
                                   const    char a1, 
                                   const    int a2,
                                   __global int *src,
                                   const    short a3,
                                   const    uint a4,
                                   const    int a5)
{
  int tid = get_global_id(0);
  dst[6*tid+0] = (int) a0 + src[tid];
  dst[6*tid+1] = (int) a1 + src[tid];
  dst[6*tid+2] = (int) a2 + src[tid];
  dst[6*tid+3] = (int) a3 + src[tid];
  dst[6*tid+4] = (int) a4 + src[tid];
  dst[6*tid+5] = (int) a5 + src[tid];
}

