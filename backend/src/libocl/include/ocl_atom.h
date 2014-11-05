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
#ifndef __OCL_ATOM_H__
#define __OCL_ATOM_H__
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


#endif  /* __OCL_ATOM_H__ */
