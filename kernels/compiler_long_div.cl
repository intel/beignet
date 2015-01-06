kernel void compiler_long_div(__global long *srcA, __global long *srcB, __global long *dst)
{
    int tid = get_global_id(0);
    dst[tid] = srcA[tid] / srcB[tid];
}

kernel void compiler_long_rem(__global long *srcA, __global long *srcB, __global long *dst)
{
    int tid = get_global_id(0);
    dst[tid] = srcA[tid] % srcB[tid];
}

