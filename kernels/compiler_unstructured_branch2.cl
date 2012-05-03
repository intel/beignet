__kernel void
compiler_unstructured_branch2(__global int *src, __global int *dst)
{
  int id = (int)get_global_id(0);
  dst[id] = src[id];
  if (dst[id] < 0) goto label1;
  dst[id] = 1;
  if (dst[id] > src[id]) goto label3;
  dst[id]++;
  if (src[id] <= 2) goto label2;
  label1:
  dst[id] -= 2;
  label2:
  dst[id] += 2;
  label3:
  dst[id] *= 3;
}

