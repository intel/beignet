kernel void compiler_workgroup_reduce_min_uniform(uint src, global uint *dst) {
   uint min_val = work_group_reduce_min(src);
   dst[get_local_id(0)] = min_val;
}

kernel void compiler_workgroup_reduce_min_uint(global uint *src, global uint *dst) {
   uint val = src[get_local_id(0)];
   uint min_val = work_group_reduce_min(val);
   dst[get_local_id(0)] = min_val;
}

kernel void compiler_workgroup_reduce_max_uint(global uint *src, global uint *dst) {
   uint val = src[get_local_id(0)];
   uint max_val = work_group_reduce_max(val);
   dst[get_local_id(0)] = max_val;
}

kernel void compiler_workgroup_reduce_min_float(global float *src, global float *dst) {
   float val = src[get_local_id(0)];
   float min_val = work_group_reduce_min(val);
   dst[get_local_id(0)] = min_val;
}

kernel void compiler_workgroup_reduce_max_float(global float *src, global float *dst) {
   float val = src[get_local_id(0)];
   float max_val = work_group_reduce_max(val);
   dst[get_local_id(0)] = max_val;
}

kernel void compiler_workgroup_reduce_add_uint(global uint *src, global uint *dst) {
   uint val = src[get_local_id(0)];
   uint sum = work_group_reduce_add(val);
   dst[get_local_id(0)] = sum;
}

kernel void compiler_workgroup_reduce_add_float(global float *src, global float *dst) {
   float val = src[get_local_id(0)];
   float sum = work_group_reduce_add(val);
   dst[get_local_id(0)] = sum;
}
