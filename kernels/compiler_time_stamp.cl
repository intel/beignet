__kernel void
compiler_time_stamp(__global int *src, __global int *dst)
{
  int i;
  int final[16];
  struct time_stamp t1, t2, t3;
  t1 = __gen_ocl_get_timestamp();
  for (i = 0; i < 16; ++i) {
    int array[16], j;
    for (j = 0; j < 16; ++j)
      array[j] = get_global_id(0);
    for (j = 0; j < src[0]; ++j)
      array[j] = 1+src[j];
    final[i] = array[i];
    if(i == 7)
      t2 = __gen_ocl_get_timestamp();
  }
  t3 = __gen_ocl_get_timestamp();
  // currently printf does not support long type.
  // printf("tmEvt %d %d %d  tmDiff %lu %lu\n", t3-t1, t2-t1);

  // time_stamp.event maybe not zero, then the time diff is not accurate,
  // because a time event occurs before the time stamp.
  printf("tmEvt %d %d %d  tmDiff %u %u\n", t1.event, t2.event, t3.event,
        (uint)(t3.tick-t1.tick), (uint)(t2.tick-t1.tick));

  dst[get_global_id(0)] = final[get_global_id(0)];
}
