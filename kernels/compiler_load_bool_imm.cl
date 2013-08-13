__kernel void
compiler_load_bool_imm(__global int *dst, __local int *localBuffer, int copiesPerWorkItem )
{
  int i;
  for(i=0; i<copiesPerWorkItem; i++)
    localBuffer[get_local_id(0)*copiesPerWorkItem+i] = copiesPerWorkItem;
  barrier(CLK_LOCAL_MEM_FENCE);

  for(i=0; i<copiesPerWorkItem; i++)
    dst[get_global_id(0)*copiesPerWorkItem + i] = localBuffer[get_local_id(0)*copiesPerWorkItem+i];
  barrier(CLK_LOCAL_MEM_FENCE);
}
