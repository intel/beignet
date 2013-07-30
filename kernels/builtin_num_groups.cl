kernel void builtin_num_groups( __global int *ret, __global int *i_dim ) {
  *ret = get_num_groups( *i_dim);
}
