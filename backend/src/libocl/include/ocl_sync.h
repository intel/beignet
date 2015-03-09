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
#ifndef __OCL_SYNC_H__
#define __OCL_SYNC_H__

#include "ocl_types.h"

/////////////////////////////////////////////////////////////////////////////
// Synchronization functions
/////////////////////////////////////////////////////////////////////////////
#define CLK_LOCAL_MEM_FENCE  (1 << 0)
#define CLK_GLOBAL_MEM_FENCE (1 << 1)

typedef uint cl_mem_fence_flags;
OVERLOADABLE void barrier(cl_mem_fence_flags flags);
void mem_fence(cl_mem_fence_flags flags);
void read_mem_fence(cl_mem_fence_flags flags);
void write_mem_fence(cl_mem_fence_flags flags);

#endif  /* __OCL_SYNC_H__ */
