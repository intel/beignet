kernel void builtin_global_size( __global int *ret, __global int *i_dim ) {
  *ret = get_global_size( *i_dim);
}
