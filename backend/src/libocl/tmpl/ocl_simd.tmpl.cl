/*
 * Copyright @ 2015 Intel Corporation
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

#include "ocl_simd.h"
#include "ocl_workitem.h"

uint get_max_sub_group_size(void)
{
  uint local_sz = get_local_size(0)*get_local_size(1)*get_local_size(2);
  uint simd_sz = get_simd_size();
  return local_sz > simd_sz ? simd_sz : local_sz;
}

uint get_sub_group_size(void)
{
  uint threadn = get_num_sub_groups();
  uint threadid = get_sub_group_id();
  if((threadid == (threadn - 1)) && (threadn > 1))
    return (get_local_size(0)*get_local_size(1)*get_local_size(2)) % get_max_sub_group_size();
  else
    return get_max_sub_group_size();
}
