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
#ifndef __OCL_ATOM20_H__
#define __OCL_ATOM20_H__
#include "ocl_types.h"

/////////////////////////////////////////////////////////////////////////////
// Atomic functions
/////////////////////////////////////////////////////////////////////////////

OVERLOADABLE uint atomic_add(volatile __global uint *p, uint val);
OVERLOADABLE uint atomic_add(volatile __local uint *p, uint val);
OVERLOADABLE int atomic_add(volatile __global int *p, int val);
OVERLOADABLE int atomic_add(volatile __local int *p, int val);

OVERLOADABLE uint atomic_sub(volatile __global uint *p, uint val);
OVERLOADABLE uint atomic_sub(volatile __local uint *p, uint val);
OVERLOADABLE int atomic_sub(volatile __global int *p, int val);
OVERLOADABLE int atomic_sub(volatile __local int *p, int val);

OVERLOADABLE uint atomic_and(volatile __global uint *p, uint val);
OVERLOADABLE uint atomic_and(volatile __local uint *p, uint val);
OVERLOADABLE int atomic_and(volatile __global int *p, int val);
OVERLOADABLE int atomic_and(volatile __local int *p, int val);

OVERLOADABLE uint atomic_or(volatile __global uint *p, uint val);
OVERLOADABLE uint atomic_or(volatile __local uint *p, uint val);
OVERLOADABLE int atomic_or(volatile __global int *p, int val);
OVERLOADABLE int atomic_or(volatile __local int *p, int val);

OVERLOADABLE uint atomic_xor(volatile __global uint *p, uint val);
OVERLOADABLE uint atomic_xor(volatile __local uint *p, uint val);
OVERLOADABLE int atomic_xor(volatile __global int *p, int val);
OVERLOADABLE int atomic_xor(volatile __local int *p, int val);

OVERLOADABLE uint atomic_xchg(volatile __global uint *p, uint val);
OVERLOADABLE uint atomic_xchg(volatile __local uint *p, uint val);
OVERLOADABLE int atomic_xchg(volatile __global int *p, int val);
OVERLOADABLE int atomic_xchg(volatile __local int *p, int val);

OVERLOADABLE int atomic_min(volatile __global int *p, int val);
OVERLOADABLE int atomic_min(volatile __local int *p, int val);

OVERLOADABLE int atomic_max(volatile __global int *p, int val);
OVERLOADABLE int atomic_max(volatile __local int *p, int val);

OVERLOADABLE uint atomic_min(volatile __global uint *p, uint val);
OVERLOADABLE uint atomic_min(volatile __local uint *p, uint val);

OVERLOADABLE uint atomic_max(volatile __global uint *p, uint val);
OVERLOADABLE uint atomic_max(volatile __local uint *p, uint val);

OVERLOADABLE float atomic_xchg (volatile __global float *p, float val);
OVERLOADABLE float atomic_xchg (volatile __local float *p, float val);

OVERLOADABLE uint atomic_inc (volatile __global uint *p);
OVERLOADABLE uint atomic_inc (volatile __local uint *p);
OVERLOADABLE int atomic_inc (volatile __global int *p);
OVERLOADABLE int atomic_inc (volatile __local int *p);

OVERLOADABLE uint atomic_dec (volatile __global uint *p);
OVERLOADABLE uint atomic_dec (volatile __local uint *p);
OVERLOADABLE int atomic_dec (volatile __global int *p);
OVERLOADABLE int atomic_dec (volatile __local int *p);

OVERLOADABLE uint atomic_cmpxchg (volatile __global uint *p, uint cmp, uint val);
OVERLOADABLE uint atomic_cmpxchg (volatile __local uint *p, uint cmp, uint val);
OVERLOADABLE int atomic_cmpxchg (volatile __global int *p, int cmp, int val);
OVERLOADABLE int atomic_cmpxchg (volatile __local int *p, int cmp, int val);


// XXX for conformance test
// The following atom_xxx api is on OpenCL spec 1.0.
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

//OpenCL 2.0 features
#define ATOMIC_GEN_FUNCTIONS(ATYPE, CTYPE, POSTFIX) \
CTYPE __gen_ocl_atomic_exchange##POSTFIX(volatile ATYPE *p, CTYPE val, int order, int scope);  \
CTYPE __gen_ocl_atomic_fetch_add##POSTFIX(volatile ATYPE *p, CTYPE val, int order, int scope); \
CTYPE __gen_ocl_atomic_fetch_sub##POSTFIX(volatile ATYPE *p, CTYPE val, int order, int scope); \
CTYPE __gen_ocl_atomic_fetch_or##POSTFIX(volatile ATYPE *p,  CTYPE val, int order, int scope); \
CTYPE __gen_ocl_atomic_fetch_xor##POSTFIX(volatile ATYPE *p, CTYPE val, int order, int scope); \
CTYPE __gen_ocl_atomic_fetch_and##POSTFIX(volatile ATYPE *p, CTYPE val, int order, int scope); \
CTYPE __gen_ocl_atomic_fetch_imin##POSTFIX(volatile ATYPE *p, CTYPE val, int order, int scope); \
CTYPE __gen_ocl_atomic_fetch_umin##POSTFIX(volatile ATYPE *p, CTYPE val, int order, int scope); \
CTYPE __gen_ocl_atomic_fetch_imax##POSTFIX(volatile ATYPE *p, CTYPE val, int order, int scope); \
CTYPE __gen_ocl_atomic_fetch_umax##POSTFIX(volatile ATYPE *p, CTYPE val, int order, int scope);\
CTYPE __gen_ocl_atomic_compare_exchange_strong##POSTFIX(volatile ATYPE* object, CTYPE expected, CTYPE desired, int sucess, int failure, int scope); \
CTYPE __gen_ocl_atomic_compare_exchange_weak##POSTFIX(volatile ATYPE* object, CTYPE expected, CTYPE desired, int sucess, int failure, int scope);

ATOMIC_GEN_FUNCTIONS(atomic_int, int, 32)
#ifndef DISABLE_ATOMIC_INT64
ATOMIC_GEN_FUNCTIONS(atomic_long, long, 64)
#endif
float __gen_ocl_atomic_exchangef(volatile atomic_int *p, float val, int order, int scope);
float __gen_ocl_atomic_fetch_addf(volatile atomic_int *p, float val, int order, int scope);

#undef ATOMIC_GEN_FUNCTIONS

/* only used to initialize global address space */
//#define ATOMIC_VAR_INIT(C value)
#define ATOMIC_VAR_INIT
#define ATOMIC_FLAG_INIT 0

//store
#define ATOMIC_FUNCTIONS(ATYPE, CTYPE, MTYPE1, MTYPE2) \
OVERLOADABLE void atomic_init(volatile ATYPE *object, CTYPE desired);  \
OVERLOADABLE void atomic_store(volatile ATYPE *object, CTYPE desired);  \
OVERLOADABLE void atomic_store_explicit(volatile ATYPE *object, CTYPE desired, memory_order order);  \
OVERLOADABLE void atomic_store_explicit(volatile ATYPE *object, CTYPE desired, memory_order order, memory_scope scope);  \
OVERLOADABLE CTYPE  atomic_load(volatile ATYPE *object);  \
OVERLOADABLE CTYPE  atomic_load_explicit(volatile ATYPE *object, memory_order order);  \
OVERLOADABLE CTYPE  atomic_load_explicit(volatile ATYPE *object, memory_order order, memory_scope scope);  \
OVERLOADABLE CTYPE  atomic_exchange(volatile ATYPE *object, CTYPE desired);  \
OVERLOADABLE CTYPE  atomic_exchange_explicit(volatile ATYPE *object, CTYPE desired, memory_order order);  \
OVERLOADABLE CTYPE  atomic_exchange_explicit(volatile ATYPE *object, CTYPE desired, memory_order order, memory_scope scope);  \
OVERLOADABLE bool atomic_compare_exchange_strong(volatile ATYPE *object, CTYPE *expected, CTYPE desired);  \
OVERLOADABLE bool atomic_compare_exchange_strong_explicit(volatile ATYPE *object, CTYPE *expected, CTYPE desired, memory_order success, memory_order failure);  \
OVERLOADABLE bool atomic_compare_exchange_strong_explicit(volatile ATYPE *object, CTYPE *expected, CTYPE desired, memory_order success, memory_order failure, memory_scope scope);  \
OVERLOADABLE bool atomic_compare_exchange_weak(volatile ATYPE *object, CTYPE *expected, CTYPE desired);  \
OVERLOADABLE bool atomic_compare_exchange_weak_explicit(volatile ATYPE *object, CTYPE *expected, CTYPE desired, memory_order success, memory_order failure);  \
OVERLOADABLE bool atomic_compare_exchange_weak_explicit(volatile ATYPE *object, CTYPE *expected, CTYPE desired, memory_order success, memory_order failure, memory_scope scope);  \
OVERLOADABLE CTYPE  atomic_fetch_add(volatile ATYPE *object, MTYPE1 desired);  \
OVERLOADABLE CTYPE  atomic_fetch_add_explicit(volatile ATYPE *object, MTYPE1 desired, memory_order order);  \
OVERLOADABLE CTYPE  atomic_fetch_add_explicit(volatile ATYPE *object, MTYPE1 desired, memory_order order, memory_scope scope);  \
OVERLOADABLE CTYPE  atomic_fetch_sub(volatile ATYPE *object, MTYPE1 desired);  \
OVERLOADABLE CTYPE  atomic_fetch_sub_explicit(volatile ATYPE *object, MTYPE1 desired, memory_order order);  \
OVERLOADABLE CTYPE  atomic_fetch_sub_explicit(volatile ATYPE *object, MTYPE1 desired, memory_order order, memory_scope scope);  \
OVERLOADABLE CTYPE  atomic_fetch_or(volatile ATYPE *object, MTYPE2 desired);  \
OVERLOADABLE CTYPE  atomic_fetch_or_explicit(volatile ATYPE *object, MTYPE2 desired, memory_order order);  \
OVERLOADABLE CTYPE  atomic_fetch_or_explicit(volatile ATYPE *object, MTYPE2 desired, memory_order order, memory_scope scope);  \
OVERLOADABLE CTYPE  atomic_fetch_xor(volatile ATYPE *object, MTYPE2 desired);  \
OVERLOADABLE CTYPE  atomic_fetch_xor_explicit(volatile ATYPE *object, MTYPE2 desired, memory_order order);  \
OVERLOADABLE CTYPE  atomic_fetch_xor_explicit(volatile ATYPE *object, MTYPE2 desired, memory_order order, memory_scope scope);  \
OVERLOADABLE CTYPE  atomic_fetch_and(volatile ATYPE *object, MTYPE2 desired);  \
OVERLOADABLE CTYPE  atomic_fetch_and_explicit(volatile ATYPE *object, MTYPE2 desired, memory_order order);  \
OVERLOADABLE CTYPE  atomic_fetch_and_explicit(volatile ATYPE *object, MTYPE2 desired, memory_order order, memory_scope scope);  \
OVERLOADABLE CTYPE  atomic_fetch_min(volatile ATYPE *object, MTYPE2 desired);  \
OVERLOADABLE CTYPE  atomic_fetch_min_explicit(volatile ATYPE *object, MTYPE2 desired, memory_order order);  \
OVERLOADABLE CTYPE  atomic_fetch_min_explicit(volatile ATYPE *object, MTYPE2 desired, memory_order order, memory_scope scope);  \
OVERLOADABLE CTYPE  atomic_fetch_max(volatile ATYPE *object, MTYPE2 desired);  \
OVERLOADABLE CTYPE  atomic_fetch_max_explicit(volatile ATYPE *object, MTYPE2 desired, memory_order order);  \
OVERLOADABLE CTYPE  atomic_fetch_max_explicit(volatile ATYPE *object, MTYPE2 desired, memory_order order, memory_scope scope);

ATOMIC_FUNCTIONS(atomic_int, int, int, int)
ATOMIC_FUNCTIONS(atomic_uint, uint, uint, uint)
#ifndef DISABLE_ATOMIC_INT64
ATOMIC_FUNCTIONS(atomic_long, long, long, long)
ATOMIC_FUNCTIONS(atomic_ulong, ulong, ulong, ulong)
#endif
ATOMIC_FUNCTIONS(atomic_float, float, float, float)
#undef ATOMIC_FUNCTIONS


OVERLOADABLE bool atomic_flag_test_and_set(volatile atomic_flag *object);
OVERLOADABLE bool atomic_flag_test_and_set_explicit(volatile atomic_flag *object, memory_order order);
OVERLOADABLE bool atomic_flag_test_and_set_explicit(volatile atomic_flag *object, memory_order order, memory_scope scope);
OVERLOADABLE void atomic_flag_clear(volatile atomic_flag *object);
OVERLOADABLE void atomic_flag_clear_explicit(volatile atomic_flag *object, memory_order order);
OVERLOADABLE void atomic_flag_clear_explicit(volatile atomic_flag *object, memory_order order, memory_scope scope);

OVERLOADABLE void atomic_work_item_fence(cl_mem_fence_flags flags, memory_order order, memory_scope scope);
#endif  /* __OCL_ATOM20_H__ */
