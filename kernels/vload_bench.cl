#define VLOAD_BENCH(T, N, M) \
__kernel void \
vload_bench_##M ##T ##N(__global T* src, __global uint* dst, uint offset) \
{ \
  int id = (int)get_global_id(0); \
  uint ##N srcV = 0; \
  for(int i = 0; i < M; i++) \
  { \
    srcV += convert_uint ##N(vload ##N(id + (i & 0xFFFF), src + offset)); \
  } \
  vstore ##N(srcV, id, dst);\
  /*if (id < 16)*/ \
  /*printf("id %d %d %d\n", id, srcV.s0, srcV.s1);*/ \
}

#define VLOAD_BENCH_ALL_VECTOR(T, N_ITERATIONS) \
               VLOAD_BENCH(T, 2, N_ITERATIONS)  \
               VLOAD_BENCH(T, 3, N_ITERATIONS)  \
               VLOAD_BENCH(T, 4, N_ITERATIONS)  \
               VLOAD_BENCH(T, 8, N_ITERATIONS)  \
               VLOAD_BENCH(T, 16, N_ITERATIONS)

#define VLOAD_BENCH_ALL_TYPES(N_ITERATIONS)     \
   VLOAD_BENCH_ALL_VECTOR(uchar, N_ITERATIONS)  \
   VLOAD_BENCH_ALL_VECTOR(char, N_ITERATIONS)   \
   VLOAD_BENCH_ALL_VECTOR(ushort, N_ITERATIONS) \
   VLOAD_BENCH_ALL_VECTOR(short, N_ITERATIONS)  \
   VLOAD_BENCH_ALL_VECTOR(uint, N_ITERATIONS)   \
   VLOAD_BENCH_ALL_VECTOR(int, N_ITERATIONS)    \
   VLOAD_BENCH_ALL_VECTOR(float, N_ITERATIONS)

VLOAD_BENCH_ALL_TYPES(1)
VLOAD_BENCH_ALL_TYPES(10000)
