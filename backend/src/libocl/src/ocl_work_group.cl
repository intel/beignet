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
#include "ocl_work_group.h"

int __gen_ocl_work_group_all(int);
int work_group_all(int predicate) {
  return __gen_ocl_work_group_all(predicate);
}

int __gen_ocl_work_group_any(int);
int work_group_any(int predicate) {
  return __gen_ocl_work_group_any(predicate);
}

/* broadcast */
#define BROADCAST_IMPL(GEN_TYPE) \
    OVERLOADABLE GEN_TYPE __gen_ocl_work_group_broadcast(GEN_TYPE a, size_t local_id); \
    OVERLOADABLE GEN_TYPE work_group_broadcast(GEN_TYPE a, size_t local_id) { \
      return __gen_ocl_work_group_broadcast(a, local_id); \
    } \
    OVERLOADABLE GEN_TYPE __gen_ocl_work_group_broadcast(GEN_TYPE a, size_t local_id_x, size_t local_id_y); \
    OVERLOADABLE GEN_TYPE work_group_broadcast(GEN_TYPE a, size_t local_id_x, size_t local_id_y) { \
      return __gen_ocl_work_group_broadcast(a, local_id_x, local_id_y);  \
    } \
    OVERLOADABLE GEN_TYPE __gen_ocl_work_group_broadcast(GEN_TYPE a, size_t local_id_x, size_t local_id_y, size_t local_id_z); \
    OVERLOADABLE GEN_TYPE work_group_broadcast(GEN_TYPE a, size_t local_id_x, size_t local_id_y, size_t local_id_z) { \
      return __gen_ocl_work_group_broadcast(a, local_id_x, local_id_y, local_id_z); \
    }

BROADCAST_IMPL(int)
BROADCAST_IMPL(uint)
BROADCAST_IMPL(long)
BROADCAST_IMPL(ulong)
BROADCAST_IMPL(float)
BROADCAST_IMPL(double)
#undef BROADCAST_IMPL


#define RANGE_OP(RANGE, OP, GEN_TYPE, SIGN) \
    OVERLOADABLE GEN_TYPE __gen_ocl_work_group_##RANGE##_##OP(bool sign, GEN_TYPE x); \
    OVERLOADABLE GEN_TYPE work_group_##RANGE##_##OP(GEN_TYPE x) { \
      return __gen_ocl_work_group_##RANGE##_##OP(SIGN, x);  \
    }

/* reduce add */
RANGE_OP(reduce, add, int, true)
RANGE_OP(reduce, add, uint, false)
RANGE_OP(reduce, add, long, true)
RANGE_OP(reduce, add, ulong, false)
RANGE_OP(reduce, add, float, true)
RANGE_OP(reduce, add, double, true)
/* reduce min */
RANGE_OP(reduce, min, int, true)
RANGE_OP(reduce, min, uint, false)
RANGE_OP(reduce, min, long, true)
RANGE_OP(reduce, min, ulong, false)
RANGE_OP(reduce, min, float, true)
RANGE_OP(reduce, min, double, true)
/* reduce max */
RANGE_OP(reduce, max, int, true)
RANGE_OP(reduce, max, uint, false)
RANGE_OP(reduce, max, long, true)
RANGE_OP(reduce, max, ulong, false)
RANGE_OP(reduce, max, float, true)
RANGE_OP(reduce, max, double, true)

/* scan_inclusive add */
RANGE_OP(scan_inclusive, add, int, true)
RANGE_OP(scan_inclusive, add, uint, false)
RANGE_OP(scan_inclusive, add, long, true)
RANGE_OP(scan_inclusive, add, ulong, false)
RANGE_OP(scan_inclusive, add, float, true)
RANGE_OP(scan_inclusive, add, double, true)
/* scan_inclusive min */
RANGE_OP(scan_inclusive, min, int, true)
RANGE_OP(scan_inclusive, min, uint, false)
RANGE_OP(scan_inclusive, min, long, true)
RANGE_OP(scan_inclusive, min, ulong, false)
RANGE_OP(scan_inclusive, min, float, true)
RANGE_OP(scan_inclusive, min, double, true)
/* scan_inclusive max */
RANGE_OP(scan_inclusive, max, int, true)
RANGE_OP(scan_inclusive, max, uint, false)
RANGE_OP(scan_inclusive, max, long, true)
RANGE_OP(scan_inclusive, max, ulong, false)
RANGE_OP(scan_inclusive, max, float, true)
RANGE_OP(scan_inclusive, max, double, true)

/* scan_exclusive add */
RANGE_OP(scan_exclusive, add, int, true)
RANGE_OP(scan_exclusive, add, uint, false)
RANGE_OP(scan_exclusive, add, long, true)
RANGE_OP(scan_exclusive, add, ulong, false)
RANGE_OP(scan_exclusive, add, float, true)
RANGE_OP(scan_exclusive, add, double, true)
/* scan_exclusive min */
RANGE_OP(scan_exclusive, min, int, true)
RANGE_OP(scan_exclusive, min, uint, false)
RANGE_OP(scan_exclusive, min, long, true)
RANGE_OP(scan_exclusive, min, ulong, false)
RANGE_OP(scan_exclusive, min, float, true)
RANGE_OP(scan_exclusive, min, double, true)
/* scan_exclusive max */
RANGE_OP(scan_exclusive, max, int, true)
RANGE_OP(scan_exclusive, max, uint, false)
RANGE_OP(scan_exclusive, max, long, true)
RANGE_OP(scan_exclusive, max, ulong, false)
RANGE_OP(scan_exclusive, max, float, true)
RANGE_OP(scan_exclusive, max, double, true)

#undef RANGE_OP
