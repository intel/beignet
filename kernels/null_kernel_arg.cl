__kernel void
null_kernel_arg(__global unsigned int *dst, __global unsigned int * mask_global, __constant unsigned int* mask_const)
{
  if(dst && mask_global==0 && mask_const == NULL)
  {
    uint idx = (uint)get_global_id(0);
    dst[idx] = idx;
  }
}
