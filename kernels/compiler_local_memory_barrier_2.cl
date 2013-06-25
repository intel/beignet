__kernel void compiler_local_memory_barrier_2(__global int *dst, __local int *src) {
  src[get_local_id(0)] = get_local_id(0);
  src[get_local_size(0) + get_local_id(0)] = get_local_id(0);
  barrier(CLK_LOCAL_MEM_FENCE);
  dst[get_local_size(0) * (2 * get_group_id(0)) + get_local_id(0)] = src[get_local_size(0) - (get_local_id(0) + 1)];
  dst[get_local_size(0) * (2 * get_group_id(0) + 1) + get_local_id(0)] = src[get_local_size(0) + get_local_size(0) - (get_local_id(0) + 1)];
}
