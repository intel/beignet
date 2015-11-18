kernel void compiler_workgroup_broadcast(global uint *src, global uint *dst) {
    uint val = src[get_group_id(0)*(get_local_size(1) * get_local_size(0))
	+ get_group_id(1)*(get_local_size(1) * get_local_size(0) * get_num_groups(0))
	+ get_local_id(1)* get_local_size(0) + get_local_id(0)];
    uint bv = work_group_broadcast(val, 8, 3);
    dst[get_group_id(0)*(get_local_size(1) * get_local_size(0))
	+ get_group_id(1)*(get_local_size(1) * get_local_size(0) * get_num_groups(0))
	+ get_local_id(1)* get_local_size(0) + get_local_id(0)] = bv;
}
