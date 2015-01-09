#define COUNT 16

__kernel void
compiler_read_buffer(__global float4* src0, __global float4* src1, __global float4* dst)
{
  float4 sum = 0;
  int offset = 0, i = 0;
  int id = (int)get_global_id(0);
  int sz = (int)get_global_size(0);
  for(i=0; i<COUNT; i++) {
    sum = sum + src0[offset + id] + src1[offset + id];
    offset += sz;
  }
  dst[id] = sum;
}
