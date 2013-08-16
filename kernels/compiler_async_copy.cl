__kernel void
compiler_async_copy(__global int2 *dst, __global int2 *src, __local int2 *localBuffer, int copiesPerWorkItem)
{
  event_t event;
  int copiesPerWorkgroup = copiesPerWorkItem * get_local_size(0);
  int i;
  event = async_work_group_copy((__local int2*)localBuffer, (__global const int2*)(src+copiesPerWorkgroup*get_group_id(0)), (size_t)copiesPerWorkgroup, (event_t)0 );
  wait_group_events( 1, &event );

  for(i=0; i<copiesPerWorkItem; i++)
    localBuffer[ get_local_id( 0 )*copiesPerWorkItem+i ] = localBuffer[ get_local_id( 0 )*copiesPerWorkItem+i ] + (int2)(3, 3);
  barrier(CLK_LOCAL_MEM_FENCE);

  event = async_work_group_copy((__global int2*)(dst+copiesPerWorkgroup*get_group_id(0)), (__local const int2*)localBuffer, (size_t)copiesPerWorkgroup, (event_t)0 );
  wait_group_events( 1, &event );
}
