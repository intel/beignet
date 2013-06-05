__kernel void
test_copy_buffer(__global float* src, __global float* dst)
{
  int id = (int)get_global_id(0);
  dst[id] = src[id];
}
