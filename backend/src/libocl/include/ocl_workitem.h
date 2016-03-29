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
#ifndef __OCL_WORKITEM_H__
#define __OCL_WORKITEM_H__

#include "ocl_types.h"

OVERLOADABLE uint get_work_dim(void);
OVERLOADABLE size_t get_global_size(uint dimindx);
OVERLOADABLE size_t get_global_id(uint dimindx);
OVERLOADABLE size_t get_local_size(uint dimindx);
OVERLOADABLE size_t get_enqueued_local_size(uint dimindx);
OVERLOADABLE size_t get_local_id(uint dimindx);
OVERLOADABLE size_t get_num_groups(uint dimindx);
OVERLOADABLE size_t get_group_id(uint dimindx);
OVERLOADABLE size_t get_global_offset(uint dimindx);
OVERLOADABLE size_t get_global_linear_id(void);
OVERLOADABLE size_t get_local_linear_id(void);

#endif  /* __OCL_WORKITEM_H__ */
