kernel void builtin_local_linear_id( __global int *ret) {
  int id = get_local_linear_id() + (get_group_id(0) + \
           get_group_id(1) * 2 + get_group_id(2) * 6) * \
           get_local_size(0) * get_local_size(1) * get_local_size(2);
  ret[id] = id;
}
