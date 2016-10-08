/* 
 * Copyright © 2012 Intel Corporation
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

#include "cl_utils.h"
#include <string.h>

LOCAL cl_int
cl_get_info_helper(void *src, size_t src_size, void *dst, size_t dst_size, size_t *ret_size)
{
  if (dst && dst_size < src_size)
    return CL_INVALID_VALUE;

  if (dst_size) {
    memcpy(dst, src, src_size);
  }

  if (ret_size)
    *ret_size = src_size;
  return CL_SUCCESS;
}
