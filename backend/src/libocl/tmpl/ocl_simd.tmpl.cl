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

/* broadcast */
#define BROADCAST_IMPL(GEN_TYPE) \
    OVERLOADABLE GEN_TYPE __gen_ocl_sub_group_broadcast(GEN_TYPE a, size_t local_id); \
    OVERLOADABLE GEN_TYPE sub_group_broadcast(GEN_TYPE a, size_t local_id) { \
      return __gen_ocl_sub_group_broadcast(a, local_id); \
    } \
    OVERLOADABLE GEN_TYPE __gen_ocl_sub_group_broadcast(GEN_TYPE a, size_t local_id_x, size_t local_id_y); \
    OVERLOADABLE GEN_TYPE sub_group_broadcast(GEN_TYPE a, size_t local_id_x, size_t local_id_y) { \
      return __gen_ocl_sub_group_broadcast(a, local_id_x, local_id_y);  \
    } \
    OVERLOADABLE GEN_TYPE __gen_ocl_sub_group_broadcast(GEN_TYPE a, size_t local_id_x, size_t local_id_y, size_t local_id_z); \
    OVERLOADABLE GEN_TYPE sub_group_broadcast(GEN_TYPE a, size_t local_id_x, size_t local_id_y, size_t local_id_z) { \
      return __gen_ocl_sub_group_broadcast(a, local_id_x, local_id_y, local_id_z); \
    }

BROADCAST_IMPL(int)
BROADCAST_IMPL(uint)
BROADCAST_IMPL(long)
BROADCAST_IMPL(ulong)
BROADCAST_IMPL(float)
BROADCAST_IMPL(double)
#undef BROADCAST_IMPL


#define RANGE_OP(RANGE, OP, GEN_TYPE, SIGN) \
    OVERLOADABLE GEN_TYPE __gen_ocl_sub_group_##RANGE##_##OP(bool sign, GEN_TYPE x); \
    OVERLOADABLE GEN_TYPE sub_group_##RANGE##_##OP(GEN_TYPE x) { \
      return __gen_ocl_sub_group_##RANGE##_##OP(SIGN, x);  \
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
PURE CONST uint __gen_ocl_sub_group_block_read_mem(const global uint* p);
OVERLOADABLE uint intel_sub_group_block_read(const global uint* p)
{
  return __gen_ocl_sub_group_block_read_mem(p);
}
OVERLOADABLE uint2 intel_sub_group_block_read2(const global uint* p)
{
  return (uint2)(intel_sub_group_block_read(p),
                 intel_sub_group_block_read(p + get_simd_size()));
}
OVERLOADABLE uint4 intel_sub_group_block_read4(const global uint* p)
{
  return (uint4)(intel_sub_group_block_read(p),
                 intel_sub_group_block_read(p + get_simd_size()),
                 intel_sub_group_block_read(p + get_simd_size() * 2),
                 intel_sub_group_block_read(p + get_simd_size() * 3));

}
OVERLOADABLE uint8 intel_sub_group_block_read8(const global uint* p)
{
  return (uint8)(intel_sub_group_block_read(p),
                 intel_sub_group_block_read(p + get_simd_size()),
                 intel_sub_group_block_read(p + get_simd_size() * 2),
                 intel_sub_group_block_read(p + get_simd_size() * 3),
                 intel_sub_group_block_read(p + get_simd_size() * 4),
                 intel_sub_group_block_read(p + get_simd_size() * 5),
                 intel_sub_group_block_read(p + get_simd_size() * 6),
                 intel_sub_group_block_read(p + get_simd_size() * 7));
}

void __gen_ocl_sub_group_block_write_mem(const global uint* p, uint data);
OVERLOADABLE void intel_sub_group_block_write(const global uint* p, uint data)
{
  __gen_ocl_sub_group_block_write_mem(p, data);
}
OVERLOADABLE void intel_sub_group_block_write2(const global uint* p, uint2 data)
{
  intel_sub_group_block_write(p, data.s0);
  intel_sub_group_block_write(p + get_simd_size(), data.s1);
}
OVERLOADABLE void intel_sub_group_block_write4(const global uint* p,uint4 data)
{
  intel_sub_group_block_write(p, data.s0);
  intel_sub_group_block_write(p + get_simd_size(), data.s1);
  intel_sub_group_block_write(p + get_simd_size() * 2, data.s2);
  intel_sub_group_block_write(p + get_simd_size() * 3, data.s3);

}
OVERLOADABLE void intel_sub_group_block_write8(const global uint* p,uint8 data)
{
  intel_sub_group_block_write(p, data.s0);
  intel_sub_group_block_write(p + get_simd_size(), data.s1);
  intel_sub_group_block_write(p + get_simd_size() * 2, data.s2);
  intel_sub_group_block_write(p + get_simd_size() * 3, data.s3);
  intel_sub_group_block_write(p + get_simd_size() * 4, data.s4);
  intel_sub_group_block_write(p + get_simd_size() * 5, data.s5);
  intel_sub_group_block_write(p + get_simd_size() * 6, data.s6);
  intel_sub_group_block_write(p + get_simd_size() * 7, data.s7);
}

PURE CONST uint __gen_ocl_sub_group_block_read_image(image2d_t p, int x, int y);
PURE CONST uint2 __gen_ocl_sub_group_block_read_image2(image2d_t p, int x, int y);
PURE CONST uint4 __gen_ocl_sub_group_block_read_image4(image2d_t p, int x, int y);
PURE CONST uint8 __gen_ocl_sub_group_block_read_image8(image2d_t p, int x, int y);
OVERLOADABLE uint intel_sub_group_block_read(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_image(p, cord.x, cord.y);
}
OVERLOADABLE uint2 intel_sub_group_block_read2(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_image2(p, cord.x, cord.y);
}
OVERLOADABLE uint4 intel_sub_group_block_read4(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_image4(p, cord.x, cord.y);
}
OVERLOADABLE uint8 intel_sub_group_block_read8(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_image8(p, cord.x, cord.y);
}
void __gen_ocl_sub_group_block_write_image(image2d_t p, int x, int y, uint data);
void __gen_ocl_sub_group_block_write_image2(image2d_t p, int x, int y, uint2 data);
void __gen_ocl_sub_group_block_write_image4(image2d_t p, int x, int y, uint4 data);
void __gen_ocl_sub_group_block_write_image8(image2d_t p, int x, int y, uint8 data);
OVERLOADABLE void intel_sub_group_block_write(image2d_t p, int2 cord, uint data)
{
  __gen_ocl_sub_group_block_write_image(p, cord.x, cord.y, data);
}
OVERLOADABLE void intel_sub_group_block_write2(image2d_t p, int2 cord, uint2 data)
{
  __gen_ocl_sub_group_block_write_image2(p, cord.x, cord.y, data);
}
OVERLOADABLE void intel_sub_group_block_write4(image2d_t p, int2 cord, uint4 data)
{
  __gen_ocl_sub_group_block_write_image4(p, cord.x, cord.y, data);
}
OVERLOADABLE void intel_sub_group_block_write8(image2d_t p, int2 cord, uint8 data)
{
  __gen_ocl_sub_group_block_write_image8(p, cord.x, cord.y, data);
}
