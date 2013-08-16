__kernel void
compiler_async_stride_copy(__global char4 *dst, __global char4 *src, __local char4 *localBuffer, int copiesPerWorkItem, int stride)
{
  event_t event;
  int copiesPerWorkgroup = copiesPerWorkItem * get_local_size(0);
  int i;
  event = async_work_group_strided_copy( (__local char4*)localBuffer, (__global const char4*)(src+copiesPerWorkgroup*stride*get_group_id(0)), (size_t)copiesPerWorkgroup, (size_t)stride, (event_t)0 );
  wait_group_events( 1, &event );

  for(i=0; i<copiesPerWorkItem; i++)
    localBuffer[ get_local_id( 0 )*copiesPerWorkItem+i ] = localBuffer[ get_local_id( 0 )*copiesPerWorkItem+i ] + (char4)(3);
  barrier(CLK_LOCAL_MEM_FENCE);

  event = async_work_group_strided_copy((__global char4*)(dst+copiesPerWorkgroup*stride*get_group_id(0)), (__local const char4*)localBuffer, (size_t)copiesPerWorkgroup, (size_t)stride, (event_t)0 );
  wait_group_events( 1, &event );
}
