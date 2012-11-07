__kernel void
compiler_insn_selection_masked_min_max(__global float* src, __global float* dst)
{
  int id = (int)get_global_id(0);
  if (get_local_id(0) > 5)
    dst[id] = max(src[id], src[7]);
  else
    dst[id] = min(src[id], src[10]);
}


