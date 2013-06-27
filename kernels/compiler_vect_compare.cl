__kernel void
compiler_vect_compare(__global int4 *src, __global int4 *dst)
{
  int4 test = (int4)(0,0,0,0);

  dst[get_global_id(0)] = test < src[get_global_id(0)];
}
