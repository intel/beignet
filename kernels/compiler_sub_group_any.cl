__kernel void compiler_sub_group_any(global int *src, global int *dst)
{
  int i = get_global_id(0);

  if (i % 2 == 1) {
    if (sub_group_any(src[i] == 5) || sub_group_any(src[i] == 9))
      dst[i] = 1;
    else if (sub_group_any(src[i] == 6))
      dst[i] = 0;
    else
      dst[i] = 2;
  }
  else
    dst[i] = 3;
}
