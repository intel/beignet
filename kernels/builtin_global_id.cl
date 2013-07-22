kernel void builtin_global_id( __global int *ret) {
  int id = get_global_id(0) + get_global_id(1)*3 + get_global_id(2)*3*4;
  ret[id] = id;
}
