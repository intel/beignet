__kernel void
compiler_insn_selection_max(__global float* src, __global float* dst)
{
  int id = (int)get_global_id(0);
  dst[id] = max(src[id], src[0]);
}

