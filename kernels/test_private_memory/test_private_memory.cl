__kernel void test_private_memory(__global int *dst)
{
  int local0[32];
  int tid = get_global_id(0);
  int i, res = 0;

  for (i = 0; i < 32; ++i) {
    local0[i] = i + tid;
  }
  for (i = 0; i < 32; ++i) {
    res += local0[i];
  }

  dst[tid] = res;
}

