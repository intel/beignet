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
#ifndef __OCL_VLOAD_H__
#define __OCL_VLOAD_H__

#include "ocl_types.h"

/////////////////////////////////////////////////////////////////////////////
// Vector loads and stores
/////////////////////////////////////////////////////////////////////////////

// These loads and stores will use untyped reads and writes, so we can just
// cast to vector loads / stores. Not C99 compliant BTW due to aliasing issue.
// Well we do not care, we do not activate TBAA in the compiler
#define DECL_UNTYPED_RW_SPACE_N(TYPE, DIM, SPACE) \
OVERLOADABLE TYPE##DIM vload##DIM(size_t offset, const SPACE TYPE *p); \
OVERLOADABLE void vstore##DIM(TYPE##DIM v, size_t offset, SPACE TYPE *p);

#define DECL_UNTYPED_RD_SPACE_N(TYPE, DIM, SPACE) \
OVERLOADABLE TYPE##DIM vload##DIM(size_t offset, const SPACE TYPE *p);

#define DECL_UNTYPED_V3_SPACE(TYPE, SPACE) \
OVERLOADABLE void vstore3(TYPE##3 v, size_t offset, SPACE TYPE *p); \
OVERLOADABLE TYPE##3 vload3(size_t offset, const SPACE TYPE *p);

#define DECL_UNTYPED_RDV3_SPACE(TYPE, SPACE) \
OVERLOADABLE TYPE##3 vload3(size_t offset, const SPACE TYPE *p);

#define DECL_UNTYPED_RW_ALL_SPACE(TYPE, SPACE) \
  DECL_UNTYPED_RW_SPACE_N(TYPE, 2, SPACE) \
  DECL_UNTYPED_V3_SPACE(TYPE, SPACE) \
  DECL_UNTYPED_RW_SPACE_N(TYPE, 4, SPACE) \
  DECL_UNTYPED_RW_SPACE_N(TYPE, 8, SPACE) \
  DECL_UNTYPED_RW_SPACE_N(TYPE, 16, SPACE)

#define DECL_UNTYPED_RD_ALL_SPACE(TYPE, SPACE) \
  DECL_UNTYPED_RD_SPACE_N(TYPE, 2, SPACE) \
  DECL_UNTYPED_RDV3_SPACE(TYPE, SPACE) \
  DECL_UNTYPED_RD_SPACE_N(TYPE, 4, SPACE) \
  DECL_UNTYPED_RD_SPACE_N(TYPE, 8, SPACE) \
  DECL_UNTYPED_RD_SPACE_N(TYPE, 16, SPACE)

#define DECL_UNTYPED_RW_ALL(TYPE) \
  DECL_UNTYPED_RW_ALL_SPACE(TYPE, __global) \
  DECL_UNTYPED_RW_ALL_SPACE(TYPE, __local) \
  DECL_UNTYPED_RD_ALL_SPACE(TYPE, __constant) \
  DECL_UNTYPED_RW_ALL_SPACE(TYPE, __private)

#define DECL_BYTE_RD_SPACE(TYPE, SPACE) \
OVERLOADABLE TYPE##2 vload2(size_t offset, const SPACE TYPE *p);  \
OVERLOADABLE TYPE##3 vload3(size_t offset, const SPACE TYPE *p);  \
OVERLOADABLE TYPE##4 vload4(size_t offset, const SPACE TYPE *p);  \
OVERLOADABLE TYPE##8 vload8(size_t offset, const SPACE TYPE *p);  \
OVERLOADABLE TYPE##16 vload16(size_t offset, const SPACE TYPE *p);

#define DECL_BYTE_WR_SPACE(TYPE, SPACE) \
OVERLOADABLE void vstore2(TYPE##2 v, size_t offset, SPACE TYPE *p); \
OVERLOADABLE void vstore3(TYPE##3 v, size_t offset, SPACE TYPE *p); \
OVERLOADABLE void vstore4(TYPE##4 v, size_t offset, SPACE TYPE *p); \
OVERLOADABLE void vstore8(TYPE##8 v, size_t offset, SPACE TYPE *p); \
OVERLOADABLE void vstore16(TYPE##16 v, size_t offset, SPACE TYPE *p);

#define DECL_BYTE_RW_ALL(TYPE) \
  DECL_BYTE_RD_SPACE(TYPE, __global) \
  DECL_BYTE_RD_SPACE(TYPE, __local) \
  DECL_BYTE_RD_SPACE(TYPE, __private) \
  DECL_BYTE_RD_SPACE(TYPE, __constant) \
  DECL_BYTE_WR_SPACE(TYPE, __global) \
  DECL_BYTE_WR_SPACE(TYPE, __local) \
  DECL_BYTE_WR_SPACE(TYPE, __private)

DECL_BYTE_RW_ALL(char)
DECL_BYTE_RW_ALL(uchar)
DECL_BYTE_RW_ALL(short)
DECL_BYTE_RW_ALL(ushort)
DECL_UNTYPED_RW_ALL(int)
DECL_UNTYPED_RW_ALL(uint)
DECL_UNTYPED_RW_ALL(long)
DECL_UNTYPED_RW_ALL(ulong)
DECL_UNTYPED_RW_ALL(float)
DECL_UNTYPED_RW_ALL(double)

#undef DECL_UNTYPED_RW_ALL
#undef DECL_UNTYPED_RW_ALL_SPACE
#undef DECL_UNTYPED_RD_ALL_SPACE
#undef DECL_UNTYPED_RW_SPACE_N
#undef DECL_UNTYPED_RD_SPACE_N
#undef DECL_UNTYPED_V3_SPACE
#undef DECL_UNTYPED_RDV3_SPACE
#undef DECL_BYTE_RD_SPACE
#undef DECL_BYTE_WR_SPACE
#undef DECL_BYTE_RW_ALL


#define DECL_HALF_LD_SPACE(SPACE) \
OVERLOADABLE float vload_half(size_t offset, const SPACE half *p);  \
OVERLOADABLE float2 vload_half2(size_t offset, const SPACE half *p); \
OVERLOADABLE float3 vload_half3(size_t offset, const SPACE half *p); \
OVERLOADABLE float3 vloada_half3(size_t offset, const SPACE half *p); \
OVERLOADABLE float4 vload_half4(size_t offset, const SPACE half *p);  \
OVERLOADABLE float8 vload_half8(size_t offset, const SPACE half *p);  \
OVERLOADABLE float16 vload_half16(size_t offset, const SPACE half *p);

#define DECL_HALF_ST_SPACE_ROUND(SPACE, ROUND, FUNC) \
OVERLOADABLE void vstore_half##ROUND(float data, size_t offset, SPACE half *p);  \
OVERLOADABLE void vstorea_half##ROUND(float data, size_t offset, SPACE half *p); \
OVERLOADABLE void vstore_half2##ROUND(float2 data, size_t offset, SPACE half *p); \
OVERLOADABLE void vstorea_half2##ROUND(float2 data, size_t offset, SPACE half *p); \
OVERLOADABLE void vstore_half3##ROUND(float3 data, size_t offset, SPACE half *p);  \
OVERLOADABLE void vstorea_half3##ROUND(float3 data, size_t offset, SPACE half *p); \
OVERLOADABLE void vstore_half4##ROUND(float4 data, size_t offset, SPACE half *p); \
OVERLOADABLE void vstorea_half4##ROUND(float4 data, size_t offset, SPACE half *p); \
OVERLOADABLE void vstore_half8##ROUND(float8 data, size_t offset, SPACE half *p); \
OVERLOADABLE void vstorea_half8##ROUND(float8 data, size_t offset, SPACE half *p); \
OVERLOADABLE void vstore_half16##ROUND(float16 data, size_t offset, SPACE half *p); \
OVERLOADABLE void vstorea_half16##ROUND(float16 data, size_t offset, SPACE half *p);

#define DECL_HALF_ST_SPACE(SPACE) \
  DECL_HALF_ST_SPACE_ROUND(SPACE,  , dummy) \
  DECL_HALF_ST_SPACE_ROUND(SPACE, _rte, dummy) \
  DECL_HALF_ST_SPACE_ROUND(SPACE, _rtz, dummy) \
  DECL_HALF_ST_SPACE_ROUND(SPACE, _rtp, dummy) \
  DECL_HALF_ST_SPACE_ROUND(SPACE, _rtn, dummy) \

DECL_HALF_LD_SPACE(__global)
DECL_HALF_LD_SPACE(__local)
DECL_HALF_LD_SPACE(__constant)
DECL_HALF_LD_SPACE(__private)

DECL_HALF_ST_SPACE(__global)
DECL_HALF_ST_SPACE(__local)
DECL_HALF_ST_SPACE(__private)

//#undef DECL_UNTYPED_RW_ALL_SPACE
#undef DECL_HALF_LD_SPACE
#undef DECL_HALF_ST_SPACE
#undef DECL_HALF_ST_SPACE_ROUND

#define vloada_half vload_half
#define vloada_half2 vload_half2
#define vloada_half4 vload_half4
#define vloada_half8 vload_half8
#define vloada_half16 vload_half16

#endif  /* __OCL_VLOAD_H__ */
