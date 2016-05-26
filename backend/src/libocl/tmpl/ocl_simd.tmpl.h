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

uint get_simd_size(void);

uint get_sub_group_size(void);
uint get_max_sub_group_size(void);
uint get_num_sub_groups(void);
uint get_sub_group_id(void);
uint get_sub_group_local_id(void);

/* broadcast */
OVERLOADABLE int sub_group_broadcast(int a, size_t local_id);
OVERLOADABLE uint sub_group_broadcast(uint a, size_t local_id);
OVERLOADABLE long sub_group_broadcast(long a, size_t local_id);
OVERLOADABLE ulong sub_group_broadcast(ulong a, size_t local_id);
OVERLOADABLE float sub_group_broadcast(float a, size_t local_id);
OVERLOADABLE double sub_group_broadcast(double a, size_t local_id);

OVERLOADABLE int sub_group_broadcast(int a, size_t local_id_x, size_t local_id_y);
OVERLOADABLE uint sub_group_broadcast(uint a, size_t local_id_x, size_t local_id_y);
OVERLOADABLE long sub_group_broadcast(long a, size_t local_id_x, size_t local_id_y);
OVERLOADABLE ulong sub_group_broadcast(ulong a, size_t local_id_x, size_t local_id_y);
OVERLOADABLE float sub_group_broadcast(float a, size_t local_id_x, size_t local_id_y);
OVERLOADABLE double sub_group_broadcast(double a, size_t local_id_x, size_t local_id_y);

OVERLOADABLE int sub_group_broadcast(int a, size_t local_id_x, size_t local_id_y, size_t local_id_z);
OVERLOADABLE uint sub_group_broadcast(uint a, size_t local_id_x, size_t local_id_y, size_t local_id_z);
OVERLOADABLE long sub_group_broadcast(long a, size_t local_id_x, size_t local_id_y, size_t local_id_z);
OVERLOADABLE ulong sub_group_broadcast(ulong a, size_t local_id_x, size_t local_id_y, size_t local_id_z);
OVERLOADABLE float sub_group_broadcast(float a, size_t local_id_x, size_t local_id_y, size_t local_id_z);
OVERLOADABLE double sub_group_broadcast(double a, size_t local_id_x, size_t local_id_y, size_t local_id_z);

/* reduce add */
OVERLOADABLE int sub_group_reduce_add(int x);
OVERLOADABLE uint sub_group_reduce_add(uint x);
OVERLOADABLE long sub_group_reduce_add(long x);
OVERLOADABLE ulong sub_group_reduce_add(ulong x);
OVERLOADABLE float sub_group_reduce_add(float x);
OVERLOADABLE double sub_group_reduce_add(double x);

/* reduce min */
OVERLOADABLE int sub_group_reduce_min(int x);
OVERLOADABLE uint sub_group_reduce_min(uint x);
OVERLOADABLE long sub_group_reduce_min(long x);
OVERLOADABLE ulong sub_group_reduce_min(ulong x);
OVERLOADABLE float sub_group_reduce_min(float x);
OVERLOADABLE double sub_group_reduce_min(double x);

/* reduce max */
OVERLOADABLE int sub_group_reduce_max(int x);
OVERLOADABLE uint sub_group_reduce_max(uint x);
OVERLOADABLE long sub_group_reduce_max(long x);
OVERLOADABLE ulong sub_group_reduce_max(ulong x);
OVERLOADABLE float sub_group_reduce_max(float x);
OVERLOADABLE double sub_group_reduce_max(double x);

/* scan_inclusive add */
OVERLOADABLE int sub_group_scan_inclusive_add(int x);
OVERLOADABLE uint sub_group_scan_inclusive_add(uint x);
OVERLOADABLE long sub_group_scan_inclusive_add(long x);
OVERLOADABLE ulong sub_group_scan_inclusive_add(ulong x);
OVERLOADABLE float sub_group_scan_inclusive_add(float x);
OVERLOADABLE double sub_group_scan_inclusive_add(double x);

/* scan_inclusive min */
OVERLOADABLE int sub_group_scan_inclusive_min(int x);
OVERLOADABLE uint sub_group_scan_inclusive_min(uint x);
OVERLOADABLE long sub_group_scan_inclusive_min(long x);
OVERLOADABLE ulong sub_group_scan_inclusive_min(ulong x);
OVERLOADABLE float sub_group_scan_inclusive_min(float x);
OVERLOADABLE double sub_group_scan_inclusive_min(double x);

/* scan_inclusive max */
OVERLOADABLE int sub_group_scan_inclusive_max(int x);
OVERLOADABLE uint sub_group_scan_inclusive_max(uint x);
OVERLOADABLE long sub_group_scan_inclusive_max(long x);
OVERLOADABLE ulong sub_group_scan_inclusive_max(ulong x);
OVERLOADABLE float sub_group_scan_inclusive_max(float x);
OVERLOADABLE double sub_group_scan_inclusive_max(double x);

/* scan_exclusive add */
OVERLOADABLE int sub_group_scan_exclusive_add(int x);
OVERLOADABLE uint sub_group_scan_exclusive_add(uint x);
OVERLOADABLE long sub_group_scan_exclusive_add(long x);
OVERLOADABLE ulong sub_group_scan_exclusive_add(ulong x);
OVERLOADABLE float sub_group_scan_exclusive_add(float x);
OVERLOADABLE double sub_group_scan_exclusive_add(double x);

/* scan_exclusive min */
OVERLOADABLE int sub_group_scan_exclusive_min(int x);
OVERLOADABLE uint sub_group_scan_exclusive_min(uint x);
OVERLOADABLE long sub_group_scan_exclusive_min(long x);
OVERLOADABLE ulong sub_group_scan_exclusive_min(ulong x);
OVERLOADABLE float sub_group_scan_exclusive_min(float x);
OVERLOADABLE double sub_group_scan_exclusive_min(double x);

/* scan_exclusive max */
OVERLOADABLE int sub_group_scan_exclusive_max(int x);
OVERLOADABLE uint sub_group_scan_exclusive_max(uint x);
OVERLOADABLE long sub_group_scan_exclusive_max(long x);
OVERLOADABLE ulong sub_group_scan_exclusive_max(ulong x);
OVERLOADABLE float sub_group_scan_exclusive_max(float x);
OVERLOADABLE double sub_group_scan_exclusive_max(double x);

/* shuffle */
OVERLOADABLE float intel_sub_group_shuffle(float x, uint c);
OVERLOADABLE int intel_sub_group_shuffle(int x, uint c);
OVERLOADABLE uint intel_sub_group_shuffle(uint x, uint c);

/* blocak read/write */
OVERLOADABLE uint intel_sub_group_block_read(const global uint* p);
OVERLOADABLE uint2 intel_sub_group_block_read2(const global uint* p);
OVERLOADABLE uint4 intel_sub_group_block_read4(const global uint* p);
OVERLOADABLE uint8 intel_sub_group_block_read8(const global uint* p);

OVERLOADABLE void intel_sub_group_block_write(const __global uint* p, uint data);
OVERLOADABLE void intel_sub_group_block_write2(const __global uint* p, uint2 data);
OVERLOADABLE void intel_sub_group_block_write4(const __global uint* p, uint4 data);
OVERLOADABLE void intel_sub_group_block_write8(const __global uint* p, uint8 data);

OVERLOADABLE uint intel_sub_group_block_read(image2d_t image, int2 byte_coord);
OVERLOADABLE uint2 intel_sub_group_block_read2(image2d_t image, int2 byte_coord);
OVERLOADABLE uint4 intel_sub_group_block_read4(image2d_t image, int2 byte_coord);
OVERLOADABLE uint8 intel_sub_group_block_read8(image2d_t image, int2 byte_coord);

OVERLOADABLE void intel_sub_group_block_write(image2d_t image, int2 byte_coord, uint data);
OVERLOADABLE void intel_sub_group_block_write2(image2d_t image, int2 byte_coord, uint2 data);
OVERLOADABLE void intel_sub_group_block_write4(image2d_t image, int2 byte_coord, uint4 data);
OVERLOADABLE void intel_sub_group_block_write8(image2d_t image, int2 byte_coord, uint8 data);
