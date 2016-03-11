/*
 * Copyright © 2012 - 2014 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "ocl_atom_20.h"
#include "ocl_as.h"
#include "ocl_sync.h"

OVERLOADABLE uint __gen_ocl_atomic_add(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_add(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_sub(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_sub(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_and(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_and(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_or(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_or(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_xor(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_xor(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_xchg(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_xchg(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_inc(__global uint *p);
OVERLOADABLE uint __gen_ocl_atomic_inc(__local uint *p);
OVERLOADABLE uint __gen_ocl_atomic_dec(__global uint *p);
OVERLOADABLE uint __gen_ocl_atomic_dec(__local uint *p);
OVERLOADABLE uint __gen_ocl_atomic_cmpxchg(__global uint *p, uint cmp, uint val);
OVERLOADABLE uint __gen_ocl_atomic_cmpxchg(__local uint *p, uint cmp, uint val);
OVERLOADABLE uint __gen_ocl_atomic_imin(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_imin(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_imax(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_imax(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_umin(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_umin(__local uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_umax(__global uint *p, uint val);
OVERLOADABLE uint __gen_ocl_atomic_umax(__local uint *p, uint val);

#define DECL_ATOMIC_OP_SPACE(NAME, TYPE, SPACE, PREFIX)                        \
  OVERLOADABLE TYPE atomic_##NAME (volatile SPACE TYPE *p, TYPE val) { \
    return (TYPE)__gen_ocl_##PREFIX##NAME((SPACE uint *)p, val);            \
  }

#define DECL_ATOMIC_OP_TYPE(NAME, TYPE, PREFIX) \
  DECL_ATOMIC_OP_SPACE(NAME, TYPE, __global, PREFIX) \
  DECL_ATOMIC_OP_SPACE(NAME, TYPE, __local, PREFIX)

#define DECL_ATOMIC_OP(NAME) \
  DECL_ATOMIC_OP_TYPE(NAME, uint, atomic_)        \
  DECL_ATOMIC_OP_TYPE(NAME, int, atomic_)

DECL_ATOMIC_OP(add)
DECL_ATOMIC_OP(sub)
DECL_ATOMIC_OP(and)
DECL_ATOMIC_OP(or)
DECL_ATOMIC_OP(xor)
DECL_ATOMIC_OP(xchg)
DECL_ATOMIC_OP_TYPE(min, int, atomic_i)
DECL_ATOMIC_OP_TYPE(max, int, atomic_i)
DECL_ATOMIC_OP_TYPE(min, uint, atomic_u)
DECL_ATOMIC_OP_TYPE(max, uint, atomic_u)

#undef DECL_ATOMIC_OP_SPACE

#define DECL_ATOMIC_OP_SPACE(NAME, TYPE, SPACE, PREFIX)                        \
  OVERLOADABLE TYPE atomic_##NAME (volatile SPACE TYPE *p, TYPE val) { \
    return as_float(__gen_ocl_##PREFIX##NAME((SPACE uint *)p, as_uint(val))); \
  }
DECL_ATOMIC_OP_SPACE(xchg, float, __global, atomic_)
DECL_ATOMIC_OP_SPACE(xchg, float, __local, atomic_)

#undef DECL_ATOMIC_OP
#undef DECL_ATOMIC_OP_TYPE
#undef DECL_ATOMIC_OP_SPACE

#define DECL_ATOMIC_OP_SPACE(NAME, TYPE, SPACE) \
  OVERLOADABLE TYPE atomic_##NAME (volatile SPACE TYPE *p) { \
    return (TYPE)__gen_ocl_atomic_##NAME((SPACE uint *)p); \
  }

#define DECL_ATOMIC_OP_TYPE(NAME, TYPE) \
  DECL_ATOMIC_OP_SPACE(NAME, TYPE, __global) \
  DECL_ATOMIC_OP_SPACE(NAME, TYPE, __local)

#define DECL_ATOMIC_OP(NAME) \
  DECL_ATOMIC_OP_TYPE(NAME, uint) \
  DECL_ATOMIC_OP_TYPE(NAME, int)

DECL_ATOMIC_OP(inc)
DECL_ATOMIC_OP(dec)

#undef DECL_ATOMIC_OP
#undef DECL_ATOMIC_OP_TYPE
#undef DECL_ATOMIC_OP_SPACE

#define DECL_ATOMIC_OP_SPACE(NAME, TYPE, SPACE)  \
  OVERLOADABLE TYPE atomic_##NAME (volatile SPACE TYPE *p, TYPE cmp, TYPE val) { \
    return (TYPE)__gen_ocl_atomic_##NAME((SPACE uint *)p, (uint)cmp, (uint)val); \
  }

#define DECL_ATOMIC_OP_TYPE(NAME, TYPE) \
  DECL_ATOMIC_OP_SPACE(NAME, TYPE, __global) \
  DECL_ATOMIC_OP_SPACE(NAME, TYPE, __local)

#define DECL_ATOMIC_OP(NAME) \
  DECL_ATOMIC_OP_TYPE(NAME, uint) \
  DECL_ATOMIC_OP_TYPE(NAME, int)

DECL_ATOMIC_OP(cmpxchg)

#undef DECL_ATOMIC_OP
#undef DECL_ATOMIC_OP_TYPE
#undef DECL_ATOMIC_OP_SPACE

// XXX for conformance test
// The following atom_xxx api is on OpenCL spec 1.0.
// But the conformance test suite will test them anyway.
#define atom_add atomic_add
#define atom_sub atomic_sub
#define atom_and atomic_and
#define atom_or atomic_or
#define atom_xor atomic_xor
#define atom_xchg atomic_xchg
#define atom_min atomic_min
#define atom_max atomic_max
#define atom_inc atomic_inc
#define atom_dec atomic_dec
#define atom_cmpxchg atomic_cmpxchg

// OpenCL 2.0 features.
#define DECL_ATOMIC_OP_TYPE(NAME, PREFIX, ATYPE, STYPE, CTYPE)                       \
  OVERLOADABLE CTYPE atomic_##NAME (volatile ATYPE *p, CTYPE val) { \
    return (CTYPE)__gen_ocl_atomic_##PREFIX((STYPE*)p, val, memory_order_seq_cst, memory_scope_device);              \
  }

#define DECL_ATOMIC_COMPARE_EXCHANGE_TYPE(NAME, PREFIX, ATYPE, STYPE, CTYPE)                       \
  OVERLOADABLE bool atomic_##NAME (volatile ATYPE *p, CTYPE* expected, CTYPE val) { \
    CTYPE oldValue = __gen_ocl_atomic_##PREFIX((STYPE*)p, *expected, val, memory_order_seq_cst, memory_order_seq_cst, memory_scope_device);   \
    bool ret = oldValue == *expected; \
    *expected = oldValue; \
    return ret;  \
  }

#define DECL_ATOMIC_LOAD_TYPE(NAME, PREFIX, ATYPE, STYPE, CTYPE)                       \
  OVERLOADABLE CTYPE atomic_##NAME (volatile ATYPE *p) { \
    return (CTYPE)__gen_ocl_atomic_##PREFIX((STYPE*)p, 0, memory_order_seq_cst, memory_scope_device);              \
  }

#define DECL_ATOMIC_NO_RET_TYPE(NAME, PREFIX, ATYPE, STYPE, CTYPE)                       \
  OVERLOADABLE void atomic_##NAME (volatile ATYPE *p, CTYPE val) { \
    __gen_ocl_atomic_##PREFIX((STYPE*)p, val, memory_order_seq_cst, memory_scope_device);              \
  }

#define DECL_ATOMIC_OP(NAME, PREFIX) \
  DECL_ATOMIC_OP_TYPE(NAME, PREFIX##32, atomic_uint, atomic_int, uint) \
  DECL_ATOMIC_OP_TYPE(NAME, PREFIX##32, atomic_int, atomic_int, int) \
  //DECL_ATOMIC_OP_TYPE(NAME, PREFIX##64, atomic_ulong, atomic_long, ulong) \
  DECL_ATOMIC_OP_TYPE(NAME, PREFIX##64, atomic_long, atomic_long, long) \

#define DECL_ATOMIC_COMPARE_EXCHANGE_OP(NAME, PREFIX) \
  DECL_ATOMIC_COMPARE_EXCHANGE_TYPE(NAME, PREFIX##32, atomic_uint, atomic_int, uint) \
  DECL_ATOMIC_COMPARE_EXCHANGE_TYPE(NAME, PREFIX##32, atomic_int, atomic_int, int)  \
  //DECL_ATOMIC_COMPARE_EXCHANGE_TYPE(NAME, PREFIX##64, atomic_ulong, atomic_long, ulong) \
  DECL_ATOMIC_COMPARE_EXCHANGE_TYPE(NAME, PREFIX##64, atomic_long, atomic_long, long) \

#define DECL_ATOMIC_LOAD_OP(NAME, PREFIX) \
  DECL_ATOMIC_LOAD_TYPE(NAME, PREFIX##32, atomic_uint, atomic_int, uint) \
  DECL_ATOMIC_LOAD_TYPE(NAME, PREFIX##32, atomic_int, atomic_int, int)  \
  //DECL_ATOMIC_LOAD_TYPE(NAME, PREFIX##64, atomic_ulong, atomic_long, ulong) \
  DECL_ATOMIC_LOAD_TYPE(NAME, PREFIX##64, atomic_long, atomic_long, long) \

#define DECL_ATOMIC_NO_RET_OP(NAME, PREFIX) \
  DECL_ATOMIC_NO_RET_TYPE(NAME, PREFIX##32, atomic_uint, atomic_int, uint) \
  DECL_ATOMIC_NO_RET_TYPE(NAME, PREFIX##32, atomic_int, atomic_int, int)   \
  //DECL_ATOMIC_NO_RET_TYPE(NAME, PREFIX##64, atomic_ulong, atomic_long, ulong) \
  DECL_ATOMIC_NO_RET_TYPE(NAME, PREFIX##64, atomic_long, atomic_long, long) \

DECL_ATOMIC_OP(exchange,  exchange)
DECL_ATOMIC_OP(fetch_add, fetch_add)
DECL_ATOMIC_OP(fetch_sub, fetch_sub)
DECL_ATOMIC_OP(fetch_and, fetch_and)
DECL_ATOMIC_OP(fetch_or,  fetch_or)
DECL_ATOMIC_OP(fetch_xor, fetch_xor)
DECL_ATOMIC_LOAD_OP(load, fetch_add)
DECL_ATOMIC_NO_RET_OP(init, exchange)
DECL_ATOMIC_NO_RET_OP(store, exchange)
DECL_ATOMIC_COMPARE_EXCHANGE_OP(compare_exchange_strong, compare_exchange_strong)
DECL_ATOMIC_COMPARE_EXCHANGE_OP(compare_exchange_weak, compare_exchange_weak)
DECL_ATOMIC_OP_TYPE(fetch_min, fetch_imin32, atomic_int, atomic_int, int)
DECL_ATOMIC_OP_TYPE(fetch_min, fetch_umin32, atomic_uint, atomic_int, uint)
DECL_ATOMIC_OP_TYPE(fetch_max, fetch_imax32, atomic_int, atomic_int, int)
DECL_ATOMIC_OP_TYPE(fetch_max, fetch_umax32, atomic_uint, atomic_int, uint)
#ifndef DISABLE_ATOMIC_INT64
DECL_ATOMIC_OP_TYPE(fetch_min, fetch_imin64, atomic_long, atomic_long, long)
DECL_ATOMIC_OP_TYPE(fetch_min, fetch_umin64, atomic_ulong, atomic_long, ulong)
DECL_ATOMIC_OP_TYPE(fetch_max, fetch_imax64, atomic_long, atomic_long, long)
DECL_ATOMIC_OP_TYPE(fetch_max, fetch_umax64, atomic_ulong, atomic_long, ulong)
#endif
DECL_ATOMIC_OP_TYPE(exchange, exchangef, atomic_float, atomic_int, float)
DECL_ATOMIC_NO_RET_TYPE(init, exchangef, atomic_float, atomic_int, float)
DECL_ATOMIC_NO_RET_TYPE(store, exchangef, atomic_float, atomic_int, float)
DECL_ATOMIC_LOAD_TYPE(load, fetch_addf, atomic_float, atomic_int, float)

#undef DECL_ATOMIC_OP_TYPE
#undef DECL_ATOMIC_LOAD_TYPE
#undef DECL_ATOMIC_NO_RET_TYPE
#undef DECL_ATOMIC_COMPARE_EXCHANGE_TYPE

// with memory_order.

#define DECL_ATOMIC_OP_TYPE(NAME, PREFIX, ATYPE, STYPE, CTYPE)                       \
  OVERLOADABLE CTYPE atomic_##NAME (volatile ATYPE *p, CTYPE val, memory_order order) { \
    return (CTYPE)__gen_ocl_atomic_##PREFIX((STYPE*)p, val, order, memory_scope_device);              \
  }

#define DECL_ATOMIC_COMPARE_EXCHANGE_TYPE(NAME, PREFIX, ATYPE, STYPE, CTYPE)                       \
  OVERLOADABLE bool atomic_##NAME (volatile ATYPE *p, CTYPE* expected, CTYPE val, memory_order success, memory_order failure) { \
    CTYPE oldValue = __gen_ocl_atomic_##PREFIX((STYPE*)p, *expected, val, success, failure, memory_scope_device);   \
    bool ret = oldValue == *expected; \
    *expected = oldValue; \
    return ret;  \
  }

#define DECL_ATOMIC_LOAD_TYPE(NAME, PREFIX, ATYPE, STYPE, CTYPE)                       \
  OVERLOADABLE CTYPE atomic_##NAME (volatile ATYPE *p, memory_order order) { \
    return (CTYPE)__gen_ocl_atomic_##PREFIX((STYPE*)p, 0, order, memory_scope_device);              \
  }

#define DECL_ATOMIC_NO_RET_TYPE(NAME, PREFIX, ATYPE, STYPE, CTYPE)                       \
  OVERLOADABLE void atomic_##NAME (volatile ATYPE *p, CTYPE val, memory_order order) { \
    __gen_ocl_atomic_##PREFIX((STYPE*)p, val, order, memory_scope_device);              \
  }

DECL_ATOMIC_OP(exchange_explicit,  exchange)
DECL_ATOMIC_OP(fetch_add_explicit, fetch_add)
DECL_ATOMIC_OP(fetch_sub_explicit, fetch_sub)
DECL_ATOMIC_OP(fetch_and_explicit, fetch_and)
DECL_ATOMIC_OP(fetch_or_explicit,  fetch_or)
DECL_ATOMIC_OP(fetch_xor_explicit, fetch_xor)
DECL_ATOMIC_LOAD_OP(load_explicit, fetch_add)
DECL_ATOMIC_NO_RET_OP(store_explicit, exchange)
DECL_ATOMIC_COMPARE_EXCHANGE_OP(compare_exchange_strong_explicit, compare_exchange_strong)
DECL_ATOMIC_COMPARE_EXCHANGE_OP(compare_exchange_weak_explicit, compare_exchange_weak)
DECL_ATOMIC_OP_TYPE(fetch_min_explicit, fetch_imin32, atomic_int, atomic_int, int)
DECL_ATOMIC_OP_TYPE(fetch_min_explicit, fetch_umin32, atomic_uint, atomic_int, uint)
DECL_ATOMIC_OP_TYPE(fetch_max_explicit, fetch_imax32, atomic_int, atomic_int, int)
DECL_ATOMIC_OP_TYPE(fetch_max_explicit, fetch_umax32, atomic_uint, atomic_int, uint)
#ifndef DISABLE_ATOMIC_INT64
DECL_ATOMIC_OP_TYPE(fetch_min_explicit, fetch_imin64, atomic_long, atomic_long, long)
DECL_ATOMIC_OP_TYPE(fetch_min_explicit, fetch_umin64, atomic_ulong, atomic_long, ulong)
DECL_ATOMIC_OP_TYPE(fetch_max_explicit, fetch_imax64, atomic_long, atomic_long, long)
DECL_ATOMIC_OP_TYPE(fetch_max_explicit, fetch_umax64, atomic_ulong, atomic_long, ulong)
#endif
DECL_ATOMIC_OP_TYPE(exchange_explicit, exchangef, atomic_float, atomic_int, float)
DECL_ATOMIC_NO_RET_TYPE(init_explicit, exchangef, atomic_float, atomic_int, float)
DECL_ATOMIC_NO_RET_TYPE(store_explicit, exchangef, atomic_float, atomic_int, float)
DECL_ATOMIC_LOAD_TYPE(load_explicit, fetch_addf, atomic_float, atomic_int, float)

#undef DECL_ATOMIC_OP_TYPE
#undef DECL_ATOMIC_LOAD_TYPE
#undef DECL_ATOMIC_NO_RET_TYPE
#undef DECL_ATOMIC_COMPARE_EXCHANGE_TYPE

// with memory_order and memory_scope
#define DECL_ATOMIC_OP_TYPE(NAME, PREFIX, ATYPE, STYPE, CTYPE)                       \
  OVERLOADABLE CTYPE atomic_##NAME (volatile ATYPE *p, CTYPE val, memory_order order, memory_scope scope) { \
    return (CTYPE)__gen_ocl_atomic_##PREFIX((STYPE*)p, val, order, scope);              \
  }

#define DECL_ATOMIC_COMPARE_EXCHANGE_TYPE(NAME, PREFIX, ATYPE, STYPE, CTYPE)                       \
  OVERLOADABLE bool atomic_##NAME (volatile ATYPE *p, CTYPE* expected, CTYPE val, memory_order success, memory_order failure, memory_scope scope) { \
    CTYPE oldValue = __gen_ocl_atomic_##PREFIX((STYPE*)p, *expected, val, success, failure,  scope);   \
    bool ret = oldValue == *expected; \
    *expected = oldValue; \
    return ret;  \
  }

#define DECL_ATOMIC_LOAD_TYPE(NAME, PREFIX, ATYPE, STYPE, CTYPE)                       \
  OVERLOADABLE CTYPE atomic_##NAME (volatile ATYPE *p, memory_order order, memory_scope scope) { \
    return (CTYPE)__gen_ocl_atomic_##PREFIX((STYPE*)p, 0, order, scope);              \
  }

#define DECL_ATOMIC_NO_RET_TYPE(NAME, PREFIX, ATYPE, STYPE, CTYPE)                       \
  OVERLOADABLE void atomic_##NAME (volatile ATYPE *p, CTYPE val, memory_order order, memory_scope scope) { \
    __gen_ocl_atomic_##PREFIX((STYPE*)p, val, order, scope);              \
  }

DECL_ATOMIC_OP(exchange_explicit,  exchange)
DECL_ATOMIC_OP(fetch_add_explicit, fetch_add)
DECL_ATOMIC_OP(fetch_sub_explicit, fetch_sub)
DECL_ATOMIC_OP(fetch_and_explicit, fetch_and)
DECL_ATOMIC_OP(fetch_or_explicit,  fetch_or)
DECL_ATOMIC_OP(fetch_xor_explicit, fetch_xor)
DECL_ATOMIC_LOAD_OP(load_explicit, fetch_add)
DECL_ATOMIC_NO_RET_OP(store_explicit, exchange)
DECL_ATOMIC_COMPARE_EXCHANGE_OP(compare_exchange_strong_explicit, compare_exchange_strong)
DECL_ATOMIC_COMPARE_EXCHANGE_OP(compare_exchange_weak_explicit, compare_exchange_weak)
DECL_ATOMIC_OP_TYPE(fetch_min_explicit, fetch_imin32, atomic_int, atomic_int, int)
DECL_ATOMIC_OP_TYPE(fetch_min_explicit, fetch_umin32, atomic_uint, atomic_int, uint)
DECL_ATOMIC_OP_TYPE(fetch_max_explicit, fetch_imax32, atomic_int, atomic_int, int)
DECL_ATOMIC_OP_TYPE(fetch_max_explicit, fetch_umax32, atomic_uint, atomic_int, uint)
#ifndef DISABLE_ATOMIC_INT64
DECL_ATOMIC_OP_TYPE(fetch_min_explicit, fetch_imin64, atomic_long, atomic_long, long)
DECL_ATOMIC_OP_TYPE(fetch_min_explicit, fetch_umin64, atomic_ulong, atomic_long, ulong)
DECL_ATOMIC_OP_TYPE(fetch_max_explicit, fetch_imax64, atomic_long, atomic_long, long)
DECL_ATOMIC_OP_TYPE(fetch_max_explicit, fetch_umax64, atomic_ulong, atomic_long, ulong)
#endif
DECL_ATOMIC_OP_TYPE(exchange_explicit, exchangef, atomic_float, atomic_int, float)
DECL_ATOMIC_NO_RET_TYPE(init_explicit, exchangef, atomic_float, atomic_int, float)
DECL_ATOMIC_NO_RET_TYPE(store_explicit, exchangef, atomic_float, atomic_int, float)
DECL_ATOMIC_LOAD_TYPE(load_explicit, fetch_addf, atomic_float, atomic_int, float)

#undef DECL_ATOMIC_OP_TYPE
#undef DECL_ATOMIC_LOAD_TYPE
#undef DECL_ATOMIC_NO_RET_TYPE
#undef DECL_ATOMIC_COMPARE_EXCHANGE_TYPE
#undef DECL_ATOMIC_OP
#undef DECL_ATOMIC_LOAD_OP
#undef DECL_ATOMIC_NO_RET_OP
#undef DECL_ATOMIC_COMPARE_EXCHANGE_OP

OVERLOADABLE bool atomic_flag_test_and_set(volatile atomic_flag *object) {
  atomic_int * temp = (atomic_int*)object;
  int expected = 0;
  int new_value = 1;
  int oldValue = __gen_ocl_atomic_compare_exchange_strong32(temp, expected, new_value, memory_order_seq_cst, memory_order_seq_cst, memory_scope_device);
  if(oldValue == new_value)
    return true;
  else
    return false;
}

OVERLOADABLE bool atomic_flag_test_and_set_explicit(volatile atomic_flag *object, memory_order order) {
  atomic_int * temp = (atomic_int*)object;
  int expected = 0;
  int new_value = 1;
  int oldValue = __gen_ocl_atomic_compare_exchange_strong32(temp, expected, new_value, memory_order_seq_cst, memory_order_seq_cst, memory_scope_device);
  if(oldValue == new_value)
    return true;
  else
    return false;
}

OVERLOADABLE bool atomic_flag_test_and_set_explicit(volatile atomic_flag *object, memory_order order, memory_scope scope){
  atomic_int * temp = (atomic_int*)object;
  int expected = 0;
  int new_value = 1;
  int oldValue = __gen_ocl_atomic_compare_exchange_strong32(temp, expected, new_value, memory_order_seq_cst, memory_order_seq_cst, memory_scope_device);
  if(oldValue == new_value)
    return true;
  else
    return false;
}

OVERLOADABLE void atomic_flag_clear(volatile atomic_flag *object){
  atomic_int * temp = (atomic_int*)object;
  __gen_ocl_atomic_exchange32(temp, 0, memory_order_seq_cst, memory_scope_device);
}

OVERLOADABLE void atomic_flag_clear_explicit(volatile atomic_flag *object, memory_order order){
  atomic_int * temp = (atomic_int*)object;
  __gen_ocl_atomic_exchange32(temp, 0, memory_order_seq_cst, memory_scope_device);
}

OVERLOADABLE void atomic_flag_clear_explicit(volatile atomic_flag *object, memory_order order, memory_scope scope){
  atomic_int * temp = (atomic_int*)object;
  __gen_ocl_atomic_exchange32(temp, 0, memory_order_seq_cst, memory_scope_device);
}

OVERLOADABLE void atomic_work_item_fence(cl_mem_fence_flags flags, memory_order order, memory_scope scope){
}
