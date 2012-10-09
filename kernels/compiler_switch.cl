__kernel void compiler_switch(__global int *dst, __global int *src)
{
  switch (get_global_id(0)) {
    case 0: dst[get_global_id(0)] = src[get_global_id(0) + 4]; break;
    case 1: dst[get_global_id(0)] = src[get_global_id(0) + 14]; break;
    case 2: dst[get_global_id(0)] = src[get_global_id(0) + 13]; break;
    case 6: dst[get_global_id(0)] = src[get_global_id(0) + 11]; break;
    case 7: dst[get_global_id(0)] = src[get_global_id(0) + 10]; break;
    case 10: dst[get_global_id(0)] = src[get_global_id(0) + 9]; break;
    case 12: dst[get_global_id(0)] = src[get_global_id(0) + 6]; break;
    default: dst[get_global_id(0)] = src[get_global_id(0) + 8]; break;
  }
}

