__kernel void compiler_atomic_functions(__global int *dst, __local int *tmp, __global int *src) {
  int lid = get_local_id(0);
  int i = lid % 12;
  atomic_xchg(&tmp[4], -1);
  switch(i) {
    case 0: atomic_inc(&tmp[i]); break;
    case 1: atomic_dec(&tmp[i]); break;
    case 2: atomic_add(&tmp[i], src[lid]); break;
    case 3: atomic_sub(&tmp[i], src[lid]); break;
    case 4: atomic_and(&tmp[i], ~(src[lid]<<(lid>>2))); break;
    case 5: atomic_or (&tmp[i], src[lid]<<(lid>>2)); break;
    case 6: atomic_xor(&tmp[i], src[lid]); break;
    case 7: atomic_min(&tmp[i], -src[lid]); break;
    case 8: atomic_max(&tmp[i], src[lid]); break;
    case 9: atomic_min((__local unsigned int *)&tmp[i], -src[lid]); break;
    case 10: atomic_max((__local unsigned int *)&tmp[i], src[lid]); break;
    case 11: atomic_cmpxchg(&(tmp[i]), 0, src[10]); break;
    default:  break;
  }

  switch(i) {
    case 0: atomic_inc(&dst[i]); break;
    case 1: atomic_dec(&dst[i]); break;
    case 2: atomic_add(&dst[i], src[lid]); break;
    case 3: atomic_sub(&dst[i], src[lid]); break;
    case 4: atomic_and(&dst[i], ~(src[lid]<<(lid>>2))); break;
    case 5: atomic_or (&dst[i], src[lid]<<(lid>>2)); break;
    case 6: atomic_xor(&dst[i], src[lid]); break;
    case 7: atomic_min(&dst[i], -src[lid]); break;
    case 8: atomic_max(&dst[i], src[lid]); break;
    case 9: atomic_min((__global unsigned int *)&dst[i], -src[lid]); break;
    case 10: atomic_max((__global unsigned int *)&dst[i], src[lid]); break;
    case 11: atomic_cmpxchg(&dst[i], 0, src[10]); break;
    default:  break;
  }

  barrier(CLK_GLOBAL_MEM_FENCE);

  if(get_global_id(0) == 0) {
    for(i=0; i<12; i=i+1)
      atomic_add(&dst[i], tmp[i]);
  }
}
