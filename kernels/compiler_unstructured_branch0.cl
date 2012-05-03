__kernel void
compiler_unstructured_branch0(__global int *src, __global int *dst)
{
  int id = (int)get_global_id(0);
  dst[id] = src[id];
  if (dst[id] >= 0) goto label;

  do {
    dst[id] = 1;
  label:
    id += get_local_size(0);
  } while (id < 32);
}

