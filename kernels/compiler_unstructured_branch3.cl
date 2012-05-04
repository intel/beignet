__kernel void
compiler_unstructured_branch3(__global int *src, __global int *dst)
{
  int id = (int)get_global_id(0);
  dst[id] = src[id];
  if (dst[id] >= 2) goto label1;
  dst[id] = 1;
  if (src[id] < 2) goto label2;
  dst[id]--;
  label1:
  dst[id] -= 2;
  label2:
  dst[id] += 2;
}


