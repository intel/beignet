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

int __gen_ocl_work_group_all(int predicate);
int work_group_all(int predicate) {
  return __gen_ocl_work_group_all(predicate);
}

int __gen_ocl_work_group_any(int predicate);
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


#define RANGE_OP(RANGE, OP, GEN_TYPE) \
    OVERLOADABLE GEN_TYPE __gen_ocl_work_group_##RANGE##_##OP(GEN_TYPE x); \
    OVERLOADABLE GEN_TYPE work_group_##RANGE##_##OP(GEN_TYPE x) { \
      return __gen_ocl_work_group_##RANGE##_##OP(x);  \
    }

/* reduce add */
RANGE_OP(reduce, add, int)
RANGE_OP(reduce, add, uint)
RANGE_OP(reduce, add, long)
RANGE_OP(reduce, add, ulong)
RANGE_OP(reduce, add, float)
RANGE_OP(reduce, add, double)
/* reduce min */
RANGE_OP(reduce, min, int)
RANGE_OP(reduce, min, uint)
RANGE_OP(reduce, min, long)
RANGE_OP(reduce, min, ulong)
RANGE_OP(reduce, min, float)
RANGE_OP(reduce, min, double)
/* reduce max */
RANGE_OP(reduce, max, int)
RANGE_OP(reduce, max, uint)
RANGE_OP(reduce, max, long)
RANGE_OP(reduce, max, ulong)
RANGE_OP(reduce, max, float)
RANGE_OP(reduce, max, double)

/* scan_inclusive add */
RANGE_OP(scan_inclusive, add, int)
RANGE_OP(scan_inclusive, add, uint)
RANGE_OP(scan_inclusive, add, long)
RANGE_OP(scan_inclusive, add, ulong)
RANGE_OP(scan_inclusive, add, float)
RANGE_OP(scan_inclusive, add, double)
/* scan_inclusive min */
RANGE_OP(scan_inclusive, min, int)
RANGE_OP(scan_inclusive, min, uint)
RANGE_OP(scan_inclusive, min, long)
RANGE_OP(scan_inclusive, min, ulong)
RANGE_OP(scan_inclusive, min, float)
RANGE_OP(scan_inclusive, min, double)
/* scan_inclusive max */
RANGE_OP(scan_inclusive, max, int)
RANGE_OP(scan_inclusive, max, uint)
RANGE_OP(scan_inclusive, max, long)
RANGE_OP(scan_inclusive, max, ulong)
RANGE_OP(scan_inclusive, max, float)
RANGE_OP(scan_inclusive, max, double)

/* scan_exclusive add */
RANGE_OP(scan_exclusive, add, int)
RANGE_OP(scan_exclusive, add, uint)
RANGE_OP(scan_exclusive, add, long)
RANGE_OP(scan_exclusive, add, ulong)
RANGE_OP(scan_exclusive, add, float)
RANGE_OP(scan_exclusive, add, double)
/* scan_exclusive min */
RANGE_OP(scan_exclusive, min, int)
RANGE_OP(scan_exclusive, min, uint)
RANGE_OP(scan_exclusive, min, long)
RANGE_OP(scan_exclusive, min, ulong)
RANGE_OP(scan_exclusive, min, float)
RANGE_OP(scan_exclusive, min, double)
/* scan_exclusive max */
RANGE_OP(scan_exclusive, max, int)
RANGE_OP(scan_exclusive, max, uint)
RANGE_OP(scan_exclusive, max, long)
RANGE_OP(scan_exclusive, max, ulong)
RANGE_OP(scan_exclusive, max, float)
RANGE_OP(scan_exclusive, max, double)

#undef RANGE_OP
