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
  if (threadid == (threadn - 1))
    return (get_local_size(0)*get_local_size(1)*get_local_size(2) -1) % get_max_sub_group_size() + 1;
  else
    return get_max_sub_group_size();
}

/* broadcast */
#define BROADCAST_IMPL(GEN_TYPE) \
    OVERLOADABLE GEN_TYPE __gen_ocl_sub_group_broadcast(GEN_TYPE a, uint local_id); \
    OVERLOADABLE GEN_TYPE sub_group_broadcast(GEN_TYPE a, uint local_id) { \
      return __gen_ocl_sub_group_broadcast(a, local_id); \
    }

BROADCAST_IMPL(int)
BROADCAST_IMPL(uint)
BROADCAST_IMPL(long)
BROADCAST_IMPL(ulong)
BROADCAST_IMPL(half)
BROADCAST_IMPL(float)
BROADCAST_IMPL(double)
BROADCAST_IMPL(short)
BROADCAST_IMPL(ushort)
#undef BROADCAST_IMPL

OVERLOADABLE short intel_sub_group_broadcast(short a, uint local_id) {
  return __gen_ocl_sub_group_broadcast(a, local_id);
}

OVERLOADABLE ushort intel_sub_group_broadcast(ushort a, uint local_id) {
  return __gen_ocl_sub_group_broadcast(a, local_id);
}

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
RANGE_OP(reduce, add, half, true)
RANGE_OP(reduce, add, float, true)
RANGE_OP(reduce, add, double, true)
RANGE_OP(reduce, add, short, true)
RANGE_OP(reduce, add, ushort, false)
/* reduce min */
RANGE_OP(reduce, min, int, true)
RANGE_OP(reduce, min, uint, false)
RANGE_OP(reduce, min, long, true)
RANGE_OP(reduce, min, ulong, false)
RANGE_OP(reduce, min, half, true)
RANGE_OP(reduce, min, float, true)
RANGE_OP(reduce, min, double, true)
RANGE_OP(reduce, min, short, true)
RANGE_OP(reduce, min, ushort, false)
/* reduce max */
RANGE_OP(reduce, max, int, true)
RANGE_OP(reduce, max, uint, false)
RANGE_OP(reduce, max, long, true)
RANGE_OP(reduce, max, ulong, false)
RANGE_OP(reduce, max, half, true)
RANGE_OP(reduce, max, float, true)
RANGE_OP(reduce, max, double, true)
RANGE_OP(reduce, max, short, true)
RANGE_OP(reduce, max, ushort, false)

/* scan_inclusive add */
RANGE_OP(scan_inclusive, add, int, true)
RANGE_OP(scan_inclusive, add, uint, false)
RANGE_OP(scan_inclusive, add, long, true)
RANGE_OP(scan_inclusive, add, ulong, false)
RANGE_OP(scan_inclusive, add, half, true)
RANGE_OP(scan_inclusive, add, float, true)
RANGE_OP(scan_inclusive, add, double, true)
RANGE_OP(scan_inclusive, add, short, true)
RANGE_OP(scan_inclusive, add, ushort, false)
/* scan_inclusive min */
RANGE_OP(scan_inclusive, min, int, true)
RANGE_OP(scan_inclusive, min, uint, false)
RANGE_OP(scan_inclusive, min, long, true)
RANGE_OP(scan_inclusive, min, ulong, false)
RANGE_OP(scan_inclusive, min, half, true)
RANGE_OP(scan_inclusive, min, float, true)
RANGE_OP(scan_inclusive, min, double, true)
RANGE_OP(scan_inclusive, min, short, true)
RANGE_OP(scan_inclusive, min, ushort, false)
/* scan_inclusive max */
RANGE_OP(scan_inclusive, max, int, true)
RANGE_OP(scan_inclusive, max, uint, false)
RANGE_OP(scan_inclusive, max, long, true)
RANGE_OP(scan_inclusive, max, ulong, false)
RANGE_OP(scan_inclusive, max, half, true)
RANGE_OP(scan_inclusive, max, float, true)
RANGE_OP(scan_inclusive, max, double, true)
RANGE_OP(scan_inclusive, max, short, true)
RANGE_OP(scan_inclusive, max, ushort, false)

/* scan_exclusive add */
RANGE_OP(scan_exclusive, add, int, true)
RANGE_OP(scan_exclusive, add, uint, false)
RANGE_OP(scan_exclusive, add, long, true)
RANGE_OP(scan_exclusive, add, ulong, false)
RANGE_OP(scan_exclusive, add, half, true)
RANGE_OP(scan_exclusive, add, float, true)
RANGE_OP(scan_exclusive, add, double, true)
RANGE_OP(scan_exclusive, add, short, true)
RANGE_OP(scan_exclusive, add, ushort, false)
/* scan_exclusive min */
RANGE_OP(scan_exclusive, min, int, true)
RANGE_OP(scan_exclusive, min, uint, false)
RANGE_OP(scan_exclusive, min, long, true)
RANGE_OP(scan_exclusive, min, ulong, false)
RANGE_OP(scan_exclusive, min, half, true)
RANGE_OP(scan_exclusive, min, float, true)
RANGE_OP(scan_exclusive, min, double, true)
RANGE_OP(scan_exclusive, min, short, true)
RANGE_OP(scan_exclusive, min, ushort, false)
/* scan_exclusive max */
RANGE_OP(scan_exclusive, max, int, true)
RANGE_OP(scan_exclusive, max, uint, false)
RANGE_OP(scan_exclusive, max, long, true)
RANGE_OP(scan_exclusive, max, ulong, false)
RANGE_OP(scan_exclusive, max, half, true)
RANGE_OP(scan_exclusive, max, float, true)
RANGE_OP(scan_exclusive, max, double, true)
RANGE_OP(scan_exclusive, max, short, true)
RANGE_OP(scan_exclusive, max, ushort, false)

#undef RANGE_OP

#define INTEL_RANGE_OP(RANGE, OP, GEN_TYPE, SIGN) \
    OVERLOADABLE GEN_TYPE intel_sub_group_##RANGE##_##OP(GEN_TYPE x) { \
      return __gen_ocl_sub_group_##RANGE##_##OP(SIGN, x);  \
    }

INTEL_RANGE_OP(reduce, add, short, true)
INTEL_RANGE_OP(reduce, add, ushort, false)
INTEL_RANGE_OP(reduce, min, short, true)
INTEL_RANGE_OP(reduce, min, ushort, false)
INTEL_RANGE_OP(reduce, max, short, true)
INTEL_RANGE_OP(reduce, max, ushort, false)
INTEL_RANGE_OP(scan_inclusive, add, short, true)
INTEL_RANGE_OP(scan_inclusive, add, ushort, false)
INTEL_RANGE_OP(scan_inclusive, min, short, true)
INTEL_RANGE_OP(scan_inclusive, min, ushort, false)
INTEL_RANGE_OP(scan_inclusive, max, short, true)
INTEL_RANGE_OP(scan_inclusive, max, ushort, false)
INTEL_RANGE_OP(scan_exclusive, add, short, true)
INTEL_RANGE_OP(scan_exclusive, add, ushort, false)
INTEL_RANGE_OP(scan_exclusive, min, short, true)
INTEL_RANGE_OP(scan_exclusive, min, ushort, false)
INTEL_RANGE_OP(scan_exclusive, max, short, true)
INTEL_RANGE_OP(scan_exclusive, max, ushort, false)

#undef INTEL_RANGE_OP
PURE CONST uint __gen_ocl_sub_group_block_read_ui_mem(const global uint* p);
PURE CONST uint2 __gen_ocl_sub_group_block_read_ui_mem2(const global uint* p);
PURE CONST uint4 __gen_ocl_sub_group_block_read_ui_mem4(const global uint* p);
PURE CONST uint8 __gen_ocl_sub_group_block_read_ui_mem8(const global uint* p);
OVERLOADABLE uint intel_sub_group_block_read(const global uint* p)
{
  return __gen_ocl_sub_group_block_read_ui_mem(p);
}
OVERLOADABLE uint2 intel_sub_group_block_read2(const global uint* p)
{
  return __gen_ocl_sub_group_block_read_ui_mem2(p);
}
OVERLOADABLE uint4 intel_sub_group_block_read4(const global uint* p)
{
  return __gen_ocl_sub_group_block_read_ui_mem4(p);
}
OVERLOADABLE uint8 intel_sub_group_block_read8(const global uint* p)
{
  return __gen_ocl_sub_group_block_read_ui_mem8(p);
}
OVERLOADABLE uint intel_sub_group_block_read_ui(const global uint* p)
{
  return __gen_ocl_sub_group_block_read_ui_mem(p);
}
OVERLOADABLE uint2 intel_sub_group_block_read_ui2(const global uint* p)
{
  return __gen_ocl_sub_group_block_read_ui_mem2(p);
}
OVERLOADABLE uint4 intel_sub_group_block_read_ui4(const global uint* p)
{
  return __gen_ocl_sub_group_block_read_ui_mem4(p);
}
OVERLOADABLE uint8 intel_sub_group_block_read_ui8(const global uint* p)
{
  return __gen_ocl_sub_group_block_read_ui_mem8(p);
}

void __gen_ocl_sub_group_block_write_ui_mem(global uint* p, uint data);
void __gen_ocl_sub_group_block_write_ui_mem2(global uint* p, uint2 data);
void __gen_ocl_sub_group_block_write_ui_mem4(global uint* p, uint4 data);
void __gen_ocl_sub_group_block_write_ui_mem8(global uint* p, uint8 data);
OVERLOADABLE void intel_sub_group_block_write(global uint* p, uint data)
{
  __gen_ocl_sub_group_block_write_ui_mem(p, data);
}
OVERLOADABLE void intel_sub_group_block_write2(global uint* p, uint2 data)
{
  __gen_ocl_sub_group_block_write_ui_mem2(p, data);
}
OVERLOADABLE void intel_sub_group_block_write4(global uint* p,uint4 data)
{
  __gen_ocl_sub_group_block_write_ui_mem4(p, data);
}
OVERLOADABLE void intel_sub_group_block_write8(global uint* p,uint8 data)
{
  __gen_ocl_sub_group_block_write_ui_mem8(p, data);
}
OVERLOADABLE void intel_sub_group_block_write_ui(global uint* p, uint data)
{
  __gen_ocl_sub_group_block_write_ui_mem(p, data);
}
OVERLOADABLE void intel_sub_group_block_write_ui2(global uint* p, uint2 data)
{
  __gen_ocl_sub_group_block_write_ui_mem2(p, data);
}
OVERLOADABLE void intel_sub_group_block_write_ui4(global uint* p,uint4 data)
{
  __gen_ocl_sub_group_block_write_ui_mem4(p, data);
}
OVERLOADABLE void intel_sub_group_block_write_ui8(global uint* p,uint8 data)
{
  __gen_ocl_sub_group_block_write_ui_mem8(p, data);
}

PURE CONST uint __gen_ocl_sub_group_block_read_ui_image(image2d_t p, int x, int y);
PURE CONST uint2 __gen_ocl_sub_group_block_read_ui_image2(image2d_t p, int x, int y);
PURE CONST uint4 __gen_ocl_sub_group_block_read_ui_image4(image2d_t p, int x, int y);
PURE CONST uint8 __gen_ocl_sub_group_block_read_ui_image8(image2d_t p, int x, int y);
OVERLOADABLE uint intel_sub_group_block_read(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_ui_image(p, cord.x, cord.y);
}
OVERLOADABLE uint2 intel_sub_group_block_read2(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_ui_image2(p, cord.x, cord.y);
}
OVERLOADABLE uint4 intel_sub_group_block_read4(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_ui_image4(p, cord.x, cord.y);
}
OVERLOADABLE uint8 intel_sub_group_block_read8(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_ui_image8(p, cord.x, cord.y);
}
OVERLOADABLE uint intel_sub_group_block_read_ui(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_ui_image(p, cord.x, cord.y);
}
OVERLOADABLE uint2 intel_sub_group_block_read_ui2(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_ui_image2(p, cord.x, cord.y);
}
OVERLOADABLE uint4 intel_sub_group_block_read_ui4(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_ui_image4(p, cord.x, cord.y);
}
OVERLOADABLE uint8 intel_sub_group_block_read_ui8(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_ui_image8(p, cord.x, cord.y);
}

void __gen_ocl_sub_group_block_write_ui_image(image2d_t p, int x, int y, uint data);
void __gen_ocl_sub_group_block_write_ui_image2(image2d_t p, int x, int y, uint2 data);
void __gen_ocl_sub_group_block_write_ui_image4(image2d_t p, int x, int y, uint4 data);
void __gen_ocl_sub_group_block_write_ui_image8(image2d_t p, int x, int y, uint8 data);
OVERLOADABLE void intel_sub_group_block_write(image2d_t p, int2 cord, uint data)
{
  __gen_ocl_sub_group_block_write_ui_image(p, cord.x, cord.y, data);
}
OVERLOADABLE void intel_sub_group_block_write2(image2d_t p, int2 cord, uint2 data)
{
  __gen_ocl_sub_group_block_write_ui_image2(p, cord.x, cord.y, data);
}
OVERLOADABLE void intel_sub_group_block_write4(image2d_t p, int2 cord, uint4 data)
{
  __gen_ocl_sub_group_block_write_ui_image4(p, cord.x, cord.y, data);
}
OVERLOADABLE void intel_sub_group_block_write8(image2d_t p, int2 cord, uint8 data)
{
  __gen_ocl_sub_group_block_write_ui_image8(p, cord.x, cord.y, data);
}
OVERLOADABLE void intel_sub_group_block_write_ui(image2d_t p, int2 cord, uint data)
{
  __gen_ocl_sub_group_block_write_ui_image(p, cord.x, cord.y, data);
}
OVERLOADABLE void intel_sub_group_block_write_ui2(image2d_t p, int2 cord, uint2 data)
{
  __gen_ocl_sub_group_block_write_ui_image2(p, cord.x, cord.y, data);
}
OVERLOADABLE void intel_sub_group_block_write_ui4(image2d_t p, int2 cord, uint4 data)
{
  __gen_ocl_sub_group_block_write_ui_image4(p, cord.x, cord.y, data);
}
OVERLOADABLE void intel_sub_group_block_write_ui8(image2d_t p, int2 cord, uint8 data)
{
  __gen_ocl_sub_group_block_write_ui_image8(p, cord.x, cord.y, data);
}

PURE CONST ushort __gen_ocl_sub_group_block_read_us_mem(const global ushort* p);
PURE CONST ushort2 __gen_ocl_sub_group_block_read_us_mem2(const global ushort* p);
PURE CONST ushort4 __gen_ocl_sub_group_block_read_us_mem4(const global ushort* p);
PURE CONST ushort8 __gen_ocl_sub_group_block_read_us_mem8(const global ushort* p);
OVERLOADABLE ushort intel_sub_group_block_read_us(const global ushort* p)
{
  return __gen_ocl_sub_group_block_read_us_mem(p);
}
OVERLOADABLE ushort2 intel_sub_group_block_read_us2(const global ushort* p)
{
  return __gen_ocl_sub_group_block_read_us_mem2(p);
}
OVERLOADABLE ushort4 intel_sub_group_block_read_us4(const global ushort* p)
{
  return __gen_ocl_sub_group_block_read_us_mem4(p);
}
OVERLOADABLE ushort8 intel_sub_group_block_read_us8(const global ushort* p)
{
  return __gen_ocl_sub_group_block_read_us_mem8(p);
}

void __gen_ocl_sub_group_block_write_us_mem(global ushort* p, ushort data);
void __gen_ocl_sub_group_block_write_us_mem2(global ushort* p, ushort2 data);
void __gen_ocl_sub_group_block_write_us_mem4(global ushort* p, ushort4 data);
void __gen_ocl_sub_group_block_write_us_mem8(global ushort* p, ushort8 data);
OVERLOADABLE void intel_sub_group_block_write_us(global ushort* p, ushort data)
{
  __gen_ocl_sub_group_block_write_us_mem(p, data);
}
OVERLOADABLE void intel_sub_group_block_write_us2(global ushort* p, ushort2 data)
{
  __gen_ocl_sub_group_block_write_us_mem2(p, data);
}
OVERLOADABLE void intel_sub_group_block_write_us4(global ushort* p,ushort4 data)
{
  __gen_ocl_sub_group_block_write_us_mem4(p, data);
}
OVERLOADABLE void intel_sub_group_block_write_us8(global ushort* p,ushort8 data)
{
  __gen_ocl_sub_group_block_write_us_mem8(p, data);
}

PURE CONST ushort __gen_ocl_sub_group_block_read_us_image(image2d_t p, int x, int y);
PURE CONST ushort2 __gen_ocl_sub_group_block_read_us_image2(image2d_t p, int x, int y);
PURE CONST ushort4 __gen_ocl_sub_group_block_read_us_image4(image2d_t p, int x, int y);
PURE CONST ushort8 __gen_ocl_sub_group_block_read_us_image8(image2d_t p, int x, int y);
OVERLOADABLE ushort intel_sub_group_block_read_us(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_us_image(p, cord.x, cord.y);
}
OVERLOADABLE ushort2 intel_sub_group_block_read_us2(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_us_image2(p, cord.x, cord.y);
}
OVERLOADABLE ushort4 intel_sub_group_block_read_us4(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_us_image4(p, cord.x, cord.y);
}
OVERLOADABLE ushort8 intel_sub_group_block_read_us8(image2d_t p, int2 cord)
{
  return __gen_ocl_sub_group_block_read_us_image8(p, cord.x, cord.y);
}

void __gen_ocl_sub_group_block_write_us_image(image2d_t p, int x, int y, ushort data);
void __gen_ocl_sub_group_block_write_us_image2(image2d_t p, int x, int y, ushort2 data);
void __gen_ocl_sub_group_block_write_us_image4(image2d_t p, int x, int y, ushort4 data);
void __gen_ocl_sub_group_block_write_us_image8(image2d_t p, int x, int y, ushort8 data);
OVERLOADABLE void intel_sub_group_block_write_us(image2d_t p, int2 cord, ushort data)
{
  __gen_ocl_sub_group_block_write_us_image(p, cord.x, cord.y, data);
}
OVERLOADABLE void intel_sub_group_block_write_us2(image2d_t p, int2 cord, ushort2 data)
{
  __gen_ocl_sub_group_block_write_us_image2(p, cord.x, cord.y, data);
}
OVERLOADABLE void intel_sub_group_block_write_us4(image2d_t p, int2 cord, ushort4 data)
{
  __gen_ocl_sub_group_block_write_us_image4(p, cord.x, cord.y, data);
}
OVERLOADABLE void intel_sub_group_block_write_us8(image2d_t p, int2 cord, ushort8 data)
{
  __gen_ocl_sub_group_block_write_us_image8(p, cord.x, cord.y, data);
}
#define SHUFFLE_DOWN(TYPE) \
OVERLOADABLE TYPE intel_sub_group_shuffle_down(TYPE x, TYPE y, uint c) { \
  TYPE res0, res1; \
  res0 = intel_sub_group_shuffle(x, (get_sub_group_local_id() + c)%get_max_sub_group_size()); \
  res1 = intel_sub_group_shuffle(y, (get_sub_group_local_id() + c)%get_max_sub_group_size()); \
  bool inRange = ((int)c + (int)get_sub_group_local_id() > 0) && (((int)c + (int)get_sub_group_local_id() < (int) get_max_sub_group_size())); \
  return inRange ? res0 : res1; \
}
SHUFFLE_DOWN(float)
SHUFFLE_DOWN(int)
SHUFFLE_DOWN(uint)
SHUFFLE_DOWN(short)
SHUFFLE_DOWN(ushort)
#undef SHUFFLE_DOWN

#define SHUFFLE_UP(TYPE) \
OVERLOADABLE TYPE intel_sub_group_shuffle_up(TYPE x, TYPE y, uint c) { \
  TYPE res0, res1; \
  res0 = intel_sub_group_shuffle(x, (get_max_sub_group_size() + get_sub_group_local_id() - c)%get_max_sub_group_size()); \
  res1 = intel_sub_group_shuffle(y, (get_max_sub_group_size() + get_sub_group_local_id() - c)%get_max_sub_group_size()); \
  bool inRange = ((int)c - (int)get_sub_group_local_id() > 0) && (((int)c - (int)get_sub_group_local_id() < (int) get_max_sub_group_size())); \
  return inRange ? res0 : res1; \
}
SHUFFLE_UP(float)
SHUFFLE_UP(int)
SHUFFLE_UP(uint)
SHUFFLE_UP(short)
SHUFFLE_UP(ushort)
#undef SHUFFLE_UP
#define SHUFFLE_XOR(TYPE) \
OVERLOADABLE TYPE intel_sub_group_shuffle_xor(TYPE x, uint c) { \
  return intel_sub_group_shuffle(x, (get_sub_group_local_id() ^ c) % get_max_sub_group_size()); \
}
SHUFFLE_XOR(float)
SHUFFLE_XOR(int)
SHUFFLE_XOR(uint)
SHUFFLE_XOR(short)
SHUFFLE_XOR(ushort)
#undef SHUFFLE_XOR
