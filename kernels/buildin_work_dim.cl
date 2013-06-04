kernel void buildin_work_dim( __global int *ret ) {
  *ret = get_work_dim();
}
