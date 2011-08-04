__kernel void
test_2d_copy(__global int* dst0,
             __global int* dst1,
             __global int* src,
             int w)
{
    const int x = (int)get_global_id(0);
    const int y = (int)get_global_id(1);
    const int index = x + y * w;
    dst0[index] = src[index];
    dst1[index] = x + y;
}

