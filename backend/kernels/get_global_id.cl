__attribute__((pure,const)) unsigned int __gen_ocl_get_global_id0(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_global_id1(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_global_id2(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_local_id0(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_local_id1(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_local_id2(void);

inline unsigned get_global_id(unsigned int dim) {
  if (dim == 0) return __gen_ocl_get_global_id0();
  else if (dim == 1) return __gen_ocl_get_global_id1();
  else if (dim == 2) return __gen_ocl_get_global_id2();
  else return 0;
}

inline unsigned get_local_id(unsigned int dim) {
  if (dim == 0) return __gen_ocl_get_local_id0();
  else if (dim == 1) return __gen_ocl_get_local_id1();
  else if (dim == 2) return __gen_ocl_get_local_id2();
  else return 0;
}

__kernel void test_global_id(__global int *dst, __global int *p)
{
  short hop = get_local_id(0);
  dst[get_global_id(0)] = hop;
  p[get_global_id(0)] = get_local_id(0);
}

