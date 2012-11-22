/* test OpenCL 1.1 Atomic Functions (section 6.11.1, 9.4) */
__kernel void compiler_atomic_functions(global int *a, global int *b) {
  atomic_add(a, *b);
  atomic_sub(a, *b);
  atomic_xchg(a, *b);
  atomic_inc(a);
  atomic_dec(a);
  atomic_cmpxchg(a, b, 100);
  atomic_min(a, *b);
  atomic_max(a, *b);
  atomic_and(a, *b);
  atomic_or(a, *b);
  atomic_xor(a, *b);
}
