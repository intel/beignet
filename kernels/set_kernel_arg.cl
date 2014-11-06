__kernel void
set_kernel_arg(__global unsigned int *dst, float3 src)
{
  size_t gid = get_global_id(0);

  switch (gid%3)
  {
    case 0:
        dst[gid] = src.x;
      break;
    case 1:
        dst[gid] = src.y;
      break;
    case 2:
        dst[gid] = src.z;
      break;
    default:
      break;
  }
}
