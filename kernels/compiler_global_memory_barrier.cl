__kernel void compiler_global_memory_barrier(__global int *dst, __global int *src) {
  src[get_local_size(0) * (2 * get_group_id(0)) + get_local_id(0)] = get_local_id(0);
  src[get_local_size(0) * (2 * get_group_id(0) + 1) + get_local_id(0)] = get_local_id(0);
  barrier(CLK_GLOBAL_MEM_FENCE);
  dst[get_local_size(0) * (2 * get_group_id(0)) + get_local_id(0)] = src[get_local_size(0) * 2 * get_group_id(0) + get_local_size(0) - (get_local_id(0) + 1)];
  dst[get_local_size(0) * (2 * get_group_id(0) + 1) + get_local_id(0)] = src[get_local_size(0) * (2 * get_group_id(0) + 1) + get_local_size(0) - (get_local_id(0) + 1)];
}
