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
#ifndef __OCL_MEMCPY_H__
#define __OCL_MEMCPY_H__
#include "ocl_types.h"

/////////////////////////////////////////////////////////////////////////////
// memcopy functions
/////////////////////////////////////////////////////////////////////////////
void __gen_memcpy_gg_align(__global uchar* dst, __global uchar* src, size_t size);
void __gen_memcpy_gp_align(__global uchar* dst, __private uchar* src, size_t size);
void __gen_memcpy_gl_align(__global uchar* dst, __local uchar* src, size_t size);
void __gen_memcpy_gc_align(__global uchar* dst, __constant uchar* src, size_t size);
void __gen_memcpy_pg_align(__private uchar* dst, __global uchar* src, size_t size);
void __gen_memcpy_pp_align(__private uchar* dst, __private uchar* src, size_t size);
void __gen_memcpy_pl_align(__private uchar* dst, __local uchar* src, size_t size);
void __gen_memcpy_pc_align(__private uchar* dst, __constant uchar* src, size_t size);
void __gen_memcpy_lg_align(__local uchar* dst, __global uchar* src, size_t size);
void __gen_memcpy_lp_align(__local uchar* dst, __private uchar* src, size_t size);
void __gen_memcpy_ll_align(__local uchar* dst, __local uchar* src, size_t size);
void __gen_memcpy_lc_align(__local uchar* dst, __constant uchar* src, size_t size);

void __gen_memcpy_gg(__global uchar* dst, __global uchar* src, size_t size);
void __gen_memcpy_gp(__global uchar* dst, __private uchar* src, size_t size);
void __gen_memcpy_gl(__global uchar* dst, __local uchar* src, size_t size);
void __gen_memcpy_gc(__global uchar* dst, __constant uchar* src, size_t size);
void __gen_memcpy_pg(__private uchar* dst, __global uchar* src, size_t size);
void __gen_memcpy_pp(__private uchar* dst, __private uchar* src, size_t size);
void __gen_memcpy_pl(__private uchar* dst, __local uchar* src, size_t size);
void __gen_memcpy_pc(__private uchar* dst, __constant uchar* src, size_t size);
void __gen_memcpy_lg(__local uchar* dst, __global uchar* src, size_t size);
void __gen_memcpy_lp(__local uchar* dst, __private uchar* src, size_t size);
void __gen_memcpy_ll(__local uchar* dst, __local uchar* src, size_t size);
void __gen_memcpy_lc(__local uchar* dst, __constant uchar* src, size_t size);

#endif  /* __OCL_MEMCPY_H__ */
