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
#ifndef __OCL_MEMSET_H__
#define __OCL_MEMSET_H__
#include "ocl_types.h"

/////////////////////////////////////////////////////////////////////////////
// memcopy functions
/////////////////////////////////////////////////////////////////////////////
void __gen_memset_g_align(__global uchar* dst, uchar val, size_t size);
void __gen_memset_p_align(__private uchar* dst, uchar val, size_t size);
void __gen_memset_l_align(__local uchar* dst, uchar val, size_t size);

void __gen_memset_g(__global uchar* dst, uchar val, size_t size);
void __gen_memset_p(__private uchar* dst, uchar val, size_t size);
void __gen_memset_l(__local uchar* dst, uchar val, size_t size);

#endif  /* __OCL_MEMSET_H__ */
