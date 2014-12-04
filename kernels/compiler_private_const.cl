constant int x[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
__kernel void
compiler_private_const( __global int *dst)
{
  const int array0[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

  dst[get_global_id(0)] = array0[get_global_id(0)] + x[get_global_id(0)];
}

