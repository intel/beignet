__kernel void load_store(__local int *dst, __local int *src)
{
  dst[0] = src[0];
}


