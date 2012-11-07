__kernel void
compiler_insn_selection_min(__global float* src, __global float* dst)
{
  int id = (int)get_global_id(0);
  dst[id] = min(src[id], src[0]);
}

