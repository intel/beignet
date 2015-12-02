kernel void builtin_global_linear_id( __global int *ret) {
  int id = get_global_linear_id();
  ret[id] = id;
}
