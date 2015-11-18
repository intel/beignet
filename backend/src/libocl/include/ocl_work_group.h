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
#ifndef __OCL_WORK_GROUP_H__
#define __OCL_WORK_GROUP_H__
#include "ocl_types.h"

int work_group_all(int predicate);
int work_group_any(int predicate);

/* broadcast */
OVERLOADABLE int work_group_broadcast(int a, size_t local_id);
OVERLOADABLE uint work_group_broadcast(uint a, size_t local_id);
OVERLOADABLE long work_group_broadcast(long a, size_t local_id);
OVERLOADABLE ulong work_group_broadcast(ulong a, size_t local_id);
OVERLOADABLE float work_group_broadcast(float a, size_t local_id);
OVERLOADABLE double work_group_broadcast(double a, size_t local_id);

OVERLOADABLE int work_group_broadcast(int a, size_t local_id_x, size_t local_id_y);
OVERLOADABLE uint work_group_broadcast(uint a, size_t local_id_x, size_t local_id_y);
OVERLOADABLE long work_group_broadcast(long a, size_t local_id_x, size_t local_id_y);
OVERLOADABLE ulong work_group_broadcast(ulong a, size_t local_id_x, size_t local_id_y);
OVERLOADABLE float work_group_broadcast(float a, size_t local_id_x, size_t local_id_y);
OVERLOADABLE double work_group_broadcast(double a, size_t local_id_x, size_t local_id_y);

OVERLOADABLE int work_group_broadcast(int a, size_t local_id_x, size_t local_id_y, size_t local_id_z);
OVERLOADABLE uint work_group_broadcast(uint a, size_t local_id_x, size_t local_id_y, size_t local_id_z);
OVERLOADABLE long work_group_broadcast(long a, size_t local_id_x, size_t local_id_y, size_t local_id_z);
OVERLOADABLE ulong work_group_broadcast(ulong a, size_t local_id_x, size_t local_id_y, size_t local_id_z);
OVERLOADABLE float work_group_broadcast(float a, size_t local_id_x, size_t local_id_y, size_t local_id_z);
OVERLOADABLE double work_group_broadcast(double a, size_t local_id_x, size_t local_id_y, size_t local_id_z);

/* reduce add */
OVERLOADABLE int work_group_reduce_add(int x);
OVERLOADABLE uint work_group_reduce_add(uint x);
OVERLOADABLE long work_group_reduce_add(long x);
OVERLOADABLE ulong work_group_reduce_add(ulong x);
OVERLOADABLE float work_group_reduce_add(float x);
OVERLOADABLE double work_group_reduce_add(double x);

/* reduce min */
OVERLOADABLE int work_group_reduce_min(int x);
OVERLOADABLE uint work_group_reduce_min(uint x);
OVERLOADABLE long work_group_reduce_min(long x);
OVERLOADABLE ulong work_group_reduce_min(ulong x);
OVERLOADABLE float work_group_reduce_min(float x);
OVERLOADABLE double work_group_reduce_min(double x);

/* reduce max */
OVERLOADABLE int work_group_reduce_max(int x);
OVERLOADABLE uint work_group_reduce_max(uint x);
OVERLOADABLE long work_group_reduce_max(long x);
OVERLOADABLE ulong work_group_reduce_max(ulong x);
OVERLOADABLE float work_group_reduce_max(float x);
OVERLOADABLE double work_group_reduce_max(double x);

/* scan_inclusive add */
OVERLOADABLE int work_group_scan_inclusive_add(int x);
OVERLOADABLE uint work_group_scan_inclusive_add(uint x);
OVERLOADABLE long work_group_scan_inclusive_add(long x);
OVERLOADABLE ulong work_group_scan_inclusive_add(ulong x);
OVERLOADABLE float work_group_scan_inclusive_add(float x);
OVERLOADABLE double work_group_scan_inclusive_add(double x);

/* scan_inclusive min */
OVERLOADABLE int work_group_scan_inclusive_min(int x);
OVERLOADABLE uint work_group_scan_inclusive_min(uint x);
OVERLOADABLE long work_group_scan_inclusive_min(long x);
OVERLOADABLE ulong work_group_scan_inclusive_min(ulong x);
OVERLOADABLE float work_group_scan_inclusive_min(float x);
OVERLOADABLE double work_group_scan_inclusive_min(double x);

/* scan_inclusive max */
OVERLOADABLE int work_group_scan_inclusive_max(int x);
OVERLOADABLE uint work_group_scan_inclusive_max(uint x);
OVERLOADABLE long work_group_scan_inclusive_max(long x);
OVERLOADABLE ulong work_group_scan_inclusive_max(ulong x);
OVERLOADABLE float work_group_scan_inclusive_max(float x);
OVERLOADABLE double work_group_scan_inclusive_max(double x);

/* scan_exclusive add */
OVERLOADABLE int work_group_scan_exclusive_add(int x);
OVERLOADABLE uint work_group_scan_exclusive_add(uint x);
OVERLOADABLE long work_group_scan_exclusive_add(long x);
OVERLOADABLE ulong work_group_scan_exclusive_add(ulong x);
OVERLOADABLE float work_group_scan_exclusive_add(float x);
OVERLOADABLE double work_group_scan_exclusive_add(double x);

/* scan_exclusive min */
OVERLOADABLE int work_group_scan_exclusive_min(int x);
OVERLOADABLE uint work_group_scan_exclusive_min(uint x);
OVERLOADABLE long work_group_scan_exclusive_min(long x);
OVERLOADABLE ulong work_group_scan_exclusive_min(ulong x);
OVERLOADABLE float work_group_scan_exclusive_min(float x);
OVERLOADABLE double work_group_scan_exclusive_min(double x);

/* scan_exclusive max */
OVERLOADABLE int work_group_scan_exclusive_max(int x);
OVERLOADABLE uint work_group_scan_exclusive_max(uint x);
OVERLOADABLE long work_group_scan_exclusive_max(long x);
OVERLOADABLE ulong work_group_scan_exclusive_max(ulong x);
OVERLOADABLE float work_group_scan_exclusive_max(float x);
OVERLOADABLE double work_group_scan_exclusive_max(double x);
#endif  /* __OCL_WORK_GROUP_H__ */
