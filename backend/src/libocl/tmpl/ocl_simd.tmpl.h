/*
 * Copyright © 2015 Intel Corporation
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
#ifndef __OCL_SIMD_H__
#define __OCL_SIMD_H__

#include "ocl_types.h"

/////////////////////////////////////////////////////////////////////////////
// SIMD level function
/////////////////////////////////////////////////////////////////////////////
int sub_group_any(int);
int sub_group_all(int);

uint get_max_sub_group_size(void);
uint get_sub_group_id(void);

OVERLOADABLE float intel_sub_group_shuffle(float x, uint c);
OVERLOADABLE int intel_sub_group_shuffle(int x, uint c);
OVERLOADABLE uint intel_sub_group_shuffle(uint x, uint c);
