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
#include "ocl_sync.h"
#include "ocl_misc.h"

void __gen_ocl_barrier_local(void);
void __gen_ocl_barrier_global(void);
void __gen_ocl_debugwait(void);

OVERLOADABLE void mem_fence(cl_mem_fence_flags flags) {
}

OVERLOADABLE void read_mem_fence(cl_mem_fence_flags flags) {
}

OVERLOADABLE void write_mem_fence(cl_mem_fence_flags flags) {
}
cl_mem_fence_flags get_fence(void *ptr) {
  bool cond = __gen_ocl_in_local((size_t)ptr);
  return cond ? CLK_LOCAL_MEM_FENCE : CLK_GLOBAL_MEM_FENCE;
}
