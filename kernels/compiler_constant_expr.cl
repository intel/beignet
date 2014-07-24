float3 foo_pow3(float3 src0, float3 src1)
{
  union {
    float3 f3;
    float   farray[4];
  } s0, s1, dst;
  s0.f3 = src0;
  s1.f3 = src1;
  int i;
  for(i = 0; i < 3; i++)
    dst.farray[i] = pow(s0.farray[i], s1.farray[i]);
  return dst.f3;
}

__kernel void
compiler_constant_expr(__global float* src, __global float *dst)
{
  int gid = get_global_id(0);
  float3 f3 = vload3(gid, src);
  float3 cf3 = (float3)(1.f, 2.f, 3.f);
  float3 result = foo_pow3(f3, cf3);
  vstore3(result, gid, dst); 
} 
