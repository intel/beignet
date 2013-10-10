struct Test{
  char t0;
  int t1;
};

constant int two= 2;

__kernel void compiler_local_slm(__global int *dst) {
  __local int hop[16];
  __local char a;
  __local struct Test c;

  c.t1 = get_group_id(0);
  a = two;// seems clang currently has a bug if I write 'a=2;' so currently workaroud it.
  hop[get_local_id(0)] = get_local_id(0);
  barrier(CLK_LOCAL_MEM_FENCE);
  dst[get_global_id(0)] = hop[get_local_id(0)] + (int)a + hop[1] + c.t1;
}

__kernel void compiler_local_slm1(__global ulong *dst) {
  __local int hop[16];
  dst[1] = (ulong)&hop[1];
  dst[0] = (ulong)&hop[0];
}
