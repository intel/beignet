kernel void builtin_local_id( __global int *ret) {
  int id = get_local_id(0) +  get_group_id(0) * 2 + \
           get_local_id(1) * 4 + get_group_id(1) * 12 +\
           get_local_id(2) * 36 + get_group_id(2) * 144;
  ret[id] = id;
}
