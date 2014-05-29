#define DEF(TYPE) \
kernel void \
compiler_async_copy_##TYPE(__global TYPE *dst, __global TYPE *src, __local TYPE *localBuffer, int copiesPerWorkItem) \
{ \
  event_t event; \
  int copiesPerWorkgroup = copiesPerWorkItem * get_local_size(0); \
  int i; \
  event = async_work_group_copy((__local TYPE*)localBuffer, (__global const TYPE*)(src+copiesPerWorkgroup*get_group_id(0)), (size_t)copiesPerWorkgroup, (event_t)0 ); \
  wait_group_events( 1, &event ); \
\
  event = async_work_group_copy((__global TYPE*)(dst+copiesPerWorkgroup*get_group_id(0)), (__local const TYPE*)localBuffer, (size_t)copiesPerWorkgroup, (event_t)0 ); \
  wait_group_events( 1, &event ); \
}

DEF(char2);
DEF(uchar2);
DEF(short2);
DEF(ushort2);
DEF(int2);
DEF(uint2);
DEF(long2);
DEF(ulong2);
DEF(float2);
//DEF(double2);
