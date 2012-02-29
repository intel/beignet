__attribute__((pure)) unsigned int __gen_get_global_id0(void);
__attribute__((pure)) unsigned int __gen_get_global_id1(void);
__attribute__((pure)) unsigned int __gen_get_global_id2(void);

inline unsigned get_global_id(unsigned int dim) {
  if (dim == 0) return __gen_get_global_id0();
  else if (dim == 1) return __gen_get_global_id1();
  else if (dim == 2) return __gen_get_global_id2();
  else return 0;
}

__kernel void test_global_id(__global int *dst)
{
  short hop = get_global_id(0);
  dst[get_global_id(0)] = hop;
}

