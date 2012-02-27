__kernel void store(__global int *dst, __local int *dst0, int x)
{
  dst[0] = 1;
}

