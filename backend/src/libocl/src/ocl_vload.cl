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
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#include "ocl_vload.h"
#include "ocl_relational.h"

// These loads and stores will use untyped reads and writes, so we can just
// cast to vector loads / stores. Not C99 compliant BTW due to aliasing issue.
// Well we do not care, we do not activate TBAA in the compiler
#define DECL_UNTYPED_RW_SPACE_N(TYPE, DIM, SPACE) \
OVERLOADABLE TYPE##DIM vload##DIM(size_t offset, const SPACE TYPE *p) { \
  return *(SPACE TYPE##DIM *) (p + DIM * offset); \
} \
OVERLOADABLE void vstore##DIM(TYPE##DIM v, size_t offset, SPACE TYPE *p) { \
  *(SPACE TYPE##DIM *) (p + DIM * offset) = v; \
}

#define DECL_UNTYPED_RD_SPACE_N(TYPE, DIM, SPACE) \
OVERLOADABLE TYPE##DIM vload##DIM(size_t offset, const SPACE TYPE *p) { \
  return *(SPACE TYPE##DIM *) (p + DIM * offset); \
}

#define DECL_UNTYPED_V3_SPACE(TYPE, SPACE) \
OVERLOADABLE void vstore3(TYPE##3 v, size_t offset, SPACE TYPE *p) {\
  *(p + 3 * offset) = v.s0; \
  *(p + 3 * offset + 1) = v.s1; \
  *(p + 3 * offset + 2) = v.s2; \
} \
OVERLOADABLE TYPE##3 vload3(size_t offset, const SPACE TYPE *p) { \
  return (TYPE##3)(*(p + 3 * offset), *(p+ 3 * offset + 1), *(p + 3 * offset + 2));\
}

#define DECL_UNTYPED_RDV3_SPACE(TYPE, SPACE) \
OVERLOADABLE TYPE##3 vload3(size_t offset, const SPACE TYPE *p) { \
  return (TYPE##3)(*(p + 3 * offset), *(p+ 3 * offset + 1), *(p + 3 * offset + 2));\
}

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
OVERLOADABLE TYPE##2 vload2(size_t offset, const SPACE TYPE *p) { \
  return (TYPE##2)(*(p+2*offset), *(p+2*offset+1)); \
} \
OVERLOADABLE TYPE##3 vload3(size_t offset, const SPACE TYPE *p) { \
  return (TYPE##3)(*(p+3*offset), *(p+3*offset+1), *(p+3*offset+2)); \
} \
OVERLOADABLE TYPE##4 vload4(size_t offset, const SPACE TYPE *p) { \
  return (TYPE##4)(vload2(2*offset, p), vload2(2*offset, p+2)); \
} \
OVERLOADABLE TYPE##8 vload8(size_t offset, const SPACE TYPE *p) { \
  return (TYPE##8)(vload4(2*offset, p), vload4(2*offset, p+4)); \
} \
OVERLOADABLE TYPE##16 vload16(size_t offset, const SPACE TYPE *p) { \
  return (TYPE##16)(vload8(2*offset, p), vload8(2*offset, p+8)); \
}

#define DECL_BYTE_WR_SPACE(TYPE, SPACE) \
OVERLOADABLE void vstore2(TYPE##2 v, size_t offset, SPACE TYPE *p) {\
  *(p + 2 * offset) = v.s0; \
  *(p + 2 * offset + 1) = v.s1; \
} \
OVERLOADABLE void vstore3(TYPE##3 v, size_t offset, SPACE TYPE *p) {\
  *(p + 3 * offset) = v.s0; \
  *(p + 3 * offset + 1) = v.s1; \
  *(p + 3 * offset + 2) = v.s2; \
} \
OVERLOADABLE void vstore4(TYPE##4 v, size_t offset, SPACE TYPE *p) { \
  vstore2(v.lo, 2*offset, p); \
  vstore2(v.hi, 2*offset, p+2); \
} \
OVERLOADABLE void vstore8(TYPE##8 v, size_t offset, SPACE TYPE *p) { \
  vstore4(v.lo, 2*offset, p); \
  vstore4(v.hi, 2*offset, p+4); \
} \
OVERLOADABLE void vstore16(TYPE##16 v, size_t offset, SPACE TYPE *p) { \
  vstore8(v.lo, 2*offset, p); \
  vstore8(v.hi, 2*offset, p+8); \
}

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

PURE CONST float __gen_ocl_f16to32(short h);
PURE CONST short __gen_ocl_f32to16(float f);

OVERLOADABLE short f32to16_rtp(float f) {
  short s = __gen_ocl_f32to16(f);
  float con = __gen_ocl_f16to32(s);
  //if(isinf(con)) return s;
  if (f > con)
    return s - signbit(f) * 2 + 1;
  else
    return s;
}

OVERLOADABLE short f32to16_rtn(float f) {
  short s = __gen_ocl_f32to16(f);
  float con = __gen_ocl_f16to32(s);
  //if(isinf(con)) return s;
  if (con > f)
    return s + signbit(f) * 2 - 1;
  else
    return s;
}

OVERLOADABLE short f32to16_rtz(float f) {
  short s = __gen_ocl_f32to16(f);
  float con = __gen_ocl_f16to32(s);
  //if(isinf(con)) return s;
  if (((con > f) && !signbit(f)) ||
      ((con < f) && signbit(f)))
    return s - 1;
  else
    return s;
}

#define DECL_HALF_LD_SPACE(SPACE) \
OVERLOADABLE float vload_half(size_t offset, const SPACE half *p) { \
  return __gen_ocl_f16to32(*(SPACE short *)(p + offset)); \
} \
OVERLOADABLE float2 vload_half2(size_t offset, const SPACE half *p) { \
  return (float2)(vload_half(offset*2, p), \
                  vload_half(offset*2 + 1, p)); \
} \
OVERLOADABLE float3 vload_half3(size_t offset, const SPACE half *p) { \
  return (float3)(vload_half(offset*3, p), \
                  vload_half(offset*3 + 1, p), \
                  vload_half(offset*3 + 2, p)); \
} \
OVERLOADABLE float3 vloada_half3(size_t offset, const SPACE half *p) { \
  return (float3)(vload_half(offset*4, p), \
                  vload_half(offset*4 + 1, p), \
                  vload_half(offset*4 + 2, p)); \
} \
OVERLOADABLE float4 vload_half4(size_t offset, const SPACE half *p) { \
  return (float4)(vload_half2(offset*2, p), \
                  vload_half2(offset*2 + 1, p)); \
} \
OVERLOADABLE float8 vload_half8(size_t offset, const SPACE half *p) { \
  return (float8)(vload_half4(offset*2, p), \
                  vload_half4(offset*2 + 1, p)); \
} \
OVERLOADABLE float16 vload_half16(size_t offset, const SPACE half *p) { \
  return (float16)(vload_half8(offset*2, p), \
                   vload_half8(offset*2 + 1, p)); \
}

#define DECL_HALF_ST_SPACE_ROUND(SPACE, ROUND, FUNC) \
OVERLOADABLE void vstore_half##ROUND(float data, size_t offset, SPACE half *p) { \
  *(SPACE short *)(p + offset) = FUNC(data); \
} \
OVERLOADABLE void vstorea_half##ROUND(float data, size_t offset, SPACE half *p) { \
  vstore_half##ROUND(data, offset, p); \
} \
OVERLOADABLE void vstore_half2##ROUND(float2 data, size_t offset, SPACE half *p) { \
  vstore_half##ROUND(data.lo, offset*2, p); \
  vstore_half##ROUND(data.hi, offset*2 + 1, p); \
} \
OVERLOADABLE void vstorea_half2##ROUND(float2 data, size_t offset, SPACE half *p) { \
  vstore_half2##ROUND(data, offset, p); \
} \
OVERLOADABLE void vstore_half3##ROUND(float3 data, size_t offset, SPACE half *p) { \
  vstore_half##ROUND(data.s0, offset*3, p); \
  vstore_half##ROUND(data.s1, offset*3 + 1, p); \
  vstore_half##ROUND(data.s2, offset*3 + 2, p); \
} \
OVERLOADABLE void vstorea_half3##ROUND(float3 data, size_t offset, SPACE half *p) { \
  vstore_half##ROUND(data.s0, offset*4, p); \
  vstore_half##ROUND(data.s1, offset*4 + 1, p); \
  vstore_half##ROUND(data.s2, offset*4 + 2, p); \
} \
OVERLOADABLE void vstore_half4##ROUND(float4 data, size_t offset, SPACE half *p) { \
  vstore_half2##ROUND(data.lo, offset*2, p); \
  vstore_half2##ROUND(data.hi, offset*2 + 1, p); \
} \
OVERLOADABLE void vstorea_half4##ROUND(float4 data, size_t offset, SPACE half *p) { \
  vstore_half4##ROUND(data, offset, p); \
} \
OVERLOADABLE void vstore_half8##ROUND(float8 data, size_t offset, SPACE half *p) { \
  vstore_half4##ROUND(data.lo, offset*2, p); \
  vstore_half4##ROUND(data.hi, offset*2 + 1, p); \
} \
OVERLOADABLE void vstorea_half8##ROUND(float8 data, size_t offset, SPACE half *p) { \
  vstore_half8##ROUND(data, offset, p); \
} \
OVERLOADABLE void vstore_half16##ROUND(float16 data, size_t offset, SPACE half *p) { \
  vstore_half8##ROUND(data.lo, offset*2, p); \
  vstore_half8##ROUND(data.hi, offset*2 + 1, p); \
} \
OVERLOADABLE void vstorea_half16##ROUND(float16 data, size_t offset, SPACE half *p) { \
  vstore_half16##ROUND(data, offset, p); \
}

#define DECL_HALF_ST_SPACE(SPACE) \
  DECL_HALF_ST_SPACE_ROUND(SPACE,  , __gen_ocl_f32to16) \
  DECL_HALF_ST_SPACE_ROUND(SPACE, _rte, __gen_ocl_f32to16) \
  DECL_HALF_ST_SPACE_ROUND(SPACE, _rtz, f32to16_rtz) \
  DECL_HALF_ST_SPACE_ROUND(SPACE, _rtp, f32to16_rtp) \
  DECL_HALF_ST_SPACE_ROUND(SPACE, _rtn, f32to16_rtn) \

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
