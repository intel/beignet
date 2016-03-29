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
#include "ocl_workitem.h"

PURE CONST uint __gen_ocl_get_work_dim(void);
OVERLOADABLE uint get_work_dim(void)
{
  return __gen_ocl_get_work_dim();
}


#define DECL_INTERNAL_WORK_ITEM_FN(NAME) \
PURE CONST unsigned int __gen_ocl_##NAME##0(void); \
PURE CONST unsigned int __gen_ocl_##NAME##1(void); \
PURE CONST unsigned int __gen_ocl_##NAME##2(void);
DECL_INTERNAL_WORK_ITEM_FN(get_group_id)
DECL_INTERNAL_WORK_ITEM_FN(get_local_id)
DECL_INTERNAL_WORK_ITEM_FN(get_enqueued_local_size)
DECL_INTERNAL_WORK_ITEM_FN(get_local_size)
DECL_INTERNAL_WORK_ITEM_FN(get_global_size)
DECL_INTERNAL_WORK_ITEM_FN(get_global_offset)
DECL_INTERNAL_WORK_ITEM_FN(get_num_groups)
#undef DECL_INTERNAL_WORK_ITEM_FN

#define DECL_PUBLIC_WORK_ITEM_FN(NAME, OTHER_RET)    \
OVERLOADABLE size_t NAME(unsigned int dim) {             \
  if (dim == 0) return __gen_ocl_##NAME##0();        \
  else if (dim == 1) return __gen_ocl_##NAME##1();   \
  else if (dim == 2) return __gen_ocl_##NAME##2();   \
  else return OTHER_RET;                             \
}

DECL_PUBLIC_WORK_ITEM_FN(get_group_id, 0)
DECL_PUBLIC_WORK_ITEM_FN(get_local_id, 0)
DECL_PUBLIC_WORK_ITEM_FN(get_enqueued_local_size, 1)
DECL_PUBLIC_WORK_ITEM_FN(get_local_size, 1)
DECL_PUBLIC_WORK_ITEM_FN(get_global_size, 1)
DECL_PUBLIC_WORK_ITEM_FN(get_global_offset, 0)
DECL_PUBLIC_WORK_ITEM_FN(get_num_groups, 1)
#undef DECL_PUBLIC_WORK_ITEM_FN

OVERLOADABLE size_t get_global_id(uint dim) {
  return get_local_id(dim) + get_enqueued_local_size(dim) * get_group_id(dim) + get_global_offset(dim);
}

OVERLOADABLE size_t get_global_linear_id(void)
{
  uint dim = __gen_ocl_get_work_dim();
  if (dim == 1) return get_global_id(0) - get_global_offset(0);
  else if (dim == 2) return (get_global_id(1) - get_global_offset(1)) * get_global_size(0) +
                             get_global_id(0) -get_global_offset(0);
  else if (dim == 3) return ((get_global_id(2) - get_global_offset(2)) *
                              get_global_size(1) * get_global_size(0)) +
                            ((get_global_id(1) - get_global_offset(1)) * get_global_size (0)) +
                             (get_global_id(0) - get_global_offset(0));
  else return 0;
}

OVERLOADABLE size_t get_local_linear_id(void)
{
  uint dim = __gen_ocl_get_work_dim();
  if (dim == 1) return get_local_id(0);
  else if (dim == 2) return get_local_id(1) * get_enqueued_local_size(0) + get_local_id(0);
  else if (dim == 3) return (get_local_id(2) * get_enqueued_local_size(1) * get_local_size(0)) +
                            (get_local_id(1) * get_enqueued_local_size(0)) + get_local_id(0);
  else return 0;
}
