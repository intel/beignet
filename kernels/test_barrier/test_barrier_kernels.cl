__kernel void test_barrier  ( __global int *dst,
                              const    float a0, 
                              const    char a1, 
                              const    int a2,
                              __global int *src,
                              const    short a3,
                              const    uint a4,
                              const    int a5)
{
  int tid = get_global_id(0);
  dst[tid] = (int)a0 + (int)a1 + (int)a2 + (int)a3 + (int)a4 + (int)a5 + src[tid];
  barrier(CLK_LOCAL_MEM_FENCE);
}

