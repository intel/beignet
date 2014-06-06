#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void
double_precision_check(__global float* src, __global float* dst)
{
  int id = (int)get_global_id(0);
  double d0 = 0.12345678912345678 + src[1];
  double d1 = 0.12355678922345678 + src[0];
  float rem = d1 - d0;
  dst[id] = rem;
}
