__constant int hop[] = {8};

__kernel void test_constant_memory(__global int *dst,
                                   __local int *local0,
                                   __constant int *constants)
{
  __local int local1[32];
  int id = get_local_id(0);
  int tid = get_global_id(0);
  local0[id] = local1[id] = id;
  barrier(CLK_LOCAL_MEM_FENCE);
  dst[tid] = local0[32-id-1] + local1[32-id-1] + constants[id] + hop[0];
}

