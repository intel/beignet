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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

/* Common fields for both SKL devices */
.max_parameter_size = 1024,
.global_mem_cache_line_size = 64, /* XXX */
.global_mem_cache_size = 8 << 10, /* XXX */
.local_mem_type = CL_LOCAL,
.local_mem_size = 64 << 10,
.scratch_mem_size = 2 << 20,
.max_mem_alloc_size = 4 * 1024 * 1024 * 1024ul,
.global_mem_size = 4 * 1024 * 1024 * 1024ul,

#define GEN9_DEVICE 1
#include "cl_gt_device.h"
#undef GEN9_DEVICE

