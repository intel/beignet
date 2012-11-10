__kernel void compiler_local_memory_two_ptr(__global int *dst,
                                            __local int *src0,
                                            __local int *src1)
{
  src0[get_local_id(0)] = get_local_id(0);
  src1[get_local_id(0)] = get_global_id(0);
  dst[get_global_id(0)] = src0[15 - get_local_id(0)] + src1[15 - get_local_id(0)];
}

