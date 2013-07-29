kernel void builtin_local_size( __global int *ret, __global int *i_dim ) {
  *ret = get_local_size( *i_dim);
}
