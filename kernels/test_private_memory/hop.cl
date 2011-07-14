__kernel void test_private_memory(__global int *dst)
{
  int tid = get_global_id(0);
  dst[tid] = tid;
}

