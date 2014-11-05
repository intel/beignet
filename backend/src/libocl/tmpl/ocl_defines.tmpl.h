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
#ifndef __OCL_COMMON_DEF_H__
#define __OCL_COMMON_DEF_H__

#define __OPENCL_VERSION__ 120
#define __CL_VERSION_1_0__ 100
#define __CL_VERSION_1_1__ 110
#define __CL_VERSION_1_2__ 120
#define __ENDIAN_LITTLE__ 1
#define __IMAGE_SUPPORT__ 1
#define __kernel_exec(X, TYPE) __kernel __attribute__((work_group_size_hint(X,1,1))) \
                                        __attribute__((vec_type_hint(TYPE)))
#define kernel_exec(X, TYPE) __kernel_exec(X, TYPE)
#define cl_khr_global_int32_base_atomics
#define cl_khr_global_int32_extended_atomics
#define cl_khr_local_int32_base_atomics
#define cl_khr_local_int32_extended_atomics
#define cl_khr_byte_addressable_store
#define cl_khr_icd
#define cl_khr_gl_sharing

#endif /* end of __OCL_COMMON_DEF_H__ */
