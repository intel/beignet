__kernel void
compiler_if_else(__global int *src, __global int *dst)
{
  int id = (int)get_global_id(0);
  dst[id] = src[id];
  if (dst[id] >= 0) {
    dst[id] = src[id+1];
    src[id] = 1;
  } else {
    dst[id]--;
    src[id] = 2;
  }
}

