/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

/**
 * \file sim_vector.h
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * c++ class helper for the simulator
 */

#ifndef __GBE_SIM_VECTOR_H__
#define __GBE_SIM_VECTOR_H__

#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>
#include <stdint.h>
#include <cmath>

#define INLINE inline __attribute__((always_inline))
#define ID(X) (X)
#define PS2SI(X) _mm_castps_si128(X)
#define SI2PS(X) _mm_castsi128_ps(X)

/* Some extra SSE functions */
template<size_t i0, size_t i1, size_t i2, size_t i3>
INLINE const __m128 shuffle(const __m128& b) {
  return _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(b), _MM_SHUFFLE(i3, i2, i1, i0)));
}
template<size_t i> INLINE
__m128 expand(const __m128& b) { 
  return shuffle<i, i, i, i>(b);
}
template<size_t index_0, size_t index_1, size_t index_2, size_t index_3>
INLINE const __m128i shuffle(const __m128i& a) {
  return _mm_shuffle_epi32(a, _MM_SHUFFLE(index_3, index_2, index_1, index_0));
}
template<size_t index>
INLINE const __m128i expand(const __m128i& b) {
  return shuffle<index, index, index, index>(b);
}

/*! Base structure for scalar double word */
union scalar_dw { uint32_t u; int32_t s; float f; };

/*! Base structure for scalar mask */
union scalar_m { uint32_t u; int32_t s; float f; };

/*! Base structure for vectors 4 / 8 / 16 / 32 double words */
template <uint32_t vectorNum>
struct simd_dw {
  INLINE simd_dw(void) {}
  INLINE simd_dw(const scalar_dw &s) {
    for (uint32_t i = 0; i < vectorNum; ++i) m[i] = _mm_load1_ps(&s.f);
  }
  simd_dw &operator= (const scalar_dw &s) {
    for (uint32_t i = 0; i < vectorNum; ++i) m[i] = _mm_load1_ps(&s.f);
    return *this;
  }
  __m128 m[vectorNum];
};

/*! Base structure for 4 / 8 / 16 / 32 booleans (m stands for "mask") */
template <uint32_t vectorNum>
struct simd_m {
  INLINE simd_m(void) {}
  INLINE simd_m(scalar_m s) {
    for (uint32_t i = 0; i < vectorNum; ++i) m[i] = _mm_load1_ps(&s.f);
  }
  __m128 m[vectorNum];
};

/*! To cast through memory */
union cast_dw {
  INLINE cast_dw(uint32_t u0, uint32_t u1, uint32_t u2, uint32_t u3) {
    u[0] = u0; u[1] = u1; u[2] = u2; u[3] = u3;
  }
  INLINE cast_dw(int32_t s0, int32_t s1, int32_t s2, int32_t s3) {
    s[0] = s0; s[1] = s1; s[2] = s2; s[3] = s3;
  }
  INLINE cast_dw(float f0, float f1, float f2, float f3) {
    f[0] = f0; f[1] = f1; f[2] = f2; f[3] = f3;
  }
  INLINE cast_dw(const __m128 &v) : v(v) {}
  INLINE cast_dw(const __m128i &vi) : vi(vi) {}
  INLINE cast_dw(void) {}
  __m128 v;
  __m128i vi;
  uint32_t u[4];
  int32_t s[4];
  float f[4];
};
static const cast_dw alltrue(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);

/* Some convenient typedefs */
typedef scalar_dw  simd1dw;
typedef simd_dw<1> simd4dw;
typedef simd_dw<2> simd8dw;
typedef simd_dw<4> simd16dw;
typedef simd_dw<8> simd32dw;
typedef scalar_m   simd1m;
typedef simd_m<1>  simd4m;
typedef simd_m<2>  simd8m;
typedef simd_m<4>  simd16m;
typedef simd_m<8>  simd32m;

//////////////////////////////////////////////////////////////////////////////
// Vector instructions
//////////////////////////////////////////////////////////////////////////////
/* Simple function to get the number of element per vector */
template <uint32_t vectorNum>
INLINE uint32_t elemNum(const simd_dw<vectorNum> &x) {
  return 4 * vectorNum;
}
template <uint32_t vectorNum>
INLINE uint32_t elemNum(const simd_m<vectorNum> &x) {
  return 4 * vectorNum;
}

/* Build an integer mask from the mask vectors */
template <uint32_t vectorNum>
INLINE uint32_t mask(const simd_m<vectorNum> v) {
  uint32_t m = _mm_movemask_ps(v.m[0]);
  for (uint32_t i = 1; i < vectorNum; ++i)
    m |= (_mm_movemask_ps(v.m[i]) << (4*i));
  return m;
}

/* Vector instructions that use sse* */
#define VEC_OP(DST_TYPE, SRC_TYPE, NAME, INTRINSIC_NAME, FN, FN0, FN1)\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const SRC_TYPE &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i)\
    dst.m[i] = FN(INTRINSIC_NAME(FN0(v0.m[i]), FN1(v1.m[i])));\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const scalar_dw &v1) {\
  NAME(dst, v0, simd_dw<vectorNum>(v1));\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const scalar_dw &v0, const SRC_TYPE &v1) {\
  NAME(dst, simd_dw<vectorNum>(v0), v1);\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const scalar_dw &v0, const scalar_dw &v1) {\
  NAME(dst, simd_dw<vectorNum>(v0), simd_dw<vectorNum>(v1));\
}
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, ADD_F, _mm_add_ps, ID, ID, ID);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, SUB_F, _mm_sub_ps, ID, ID, ID);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, MUL_F, _mm_mul_ps, ID, ID, ID);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, DIV_F, _mm_div_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, EQ_F, _mm_cmpeq_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, NE_F, _mm_cmpneq_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, LT_F, _mm_cmplt_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, LE_F, _mm_cmple_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, GT_F, _mm_cmpgt_ps, ID, ID, ID);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, GE_F, _mm_cmpge_ps, ID, ID, ID);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, ADD_S32, _mm_add_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, SUB_S32, _mm_sub_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, EQ_S32, _mm_cmpeq_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, LT_S32, _mm_cmplt_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_m<vectorNum>,  simd_dw<vectorNum>, GT_S32, _mm_cmpgt_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, OR_S32, _mm_or_ps, ID, ID, ID);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, XOR_S32, _mm_xor_ps, ID, ID, ID);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, AND_S32, _mm_and_ps, ID, ID, ID);
#undef VEC_OP

/* Vector integer operations that we can get by switching argument order */
#define VEC_OP(DST_TYPE, SRC_TYPE, NAME, INTRINSIC_NAME, FN, FN0, FN1)\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const SRC_TYPE &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i)\
    dst.m[i] = _mm_xor_ps(FN(INTRINSIC_NAME(FN1(v0.m[i]), FN0(v1.m[i]))), alltrue.v);\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const scalar_dw &v1) {\
  NAME(dst, v0, simd_dw<vectorNum>(v1));\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const scalar_dw &v0, const SRC_TYPE &v1) {\
  NAME(dst, simd_dw<vectorNum>(v0), v1);\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const scalar_dw &v0, const scalar_dw &v1) {\
  NAME(dst, simd_dw<vectorNum>(v0), simd_dw<vectorNum>(v1));\
}
VEC_OP(simd_m<vectorNum>, simd_dw<vectorNum>, GE_S32, _mm_cmplt_epi32, SI2PS, PS2SI, PS2SI);
VEC_OP(simd_m<vectorNum>, simd_dw<vectorNum>, LE_S32, _mm_cmpgt_epi32, SI2PS, PS2SI, PS2SI);
#undef VEC_OP

/* Vector binary integer operations that require C */
#define VEC_OP(DST_TYPE, SRC_TYPE, NAME, OP, FIELD)\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const SRC_TYPE &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i) {\
    cast_dw c0(v0.m[i]), c1(v1.m[i]), d;\
    for (uint32_t j = 0; j < 4; ++j)\
      d.FIELD[j] = c0.FIELD[j] OP c1.FIELD[j];\
    dst.m[i] = d.v;\
  }\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const scalar_dw &v1) {\
  NAME(dst, v0, simd_dw<vectorNum>(v1));\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const scalar_dw &v0, const SRC_TYPE &v1) {\
  NAME(dst, simd_dw<vectorNum>(v0), v1);\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const scalar_dw &v0, const scalar_dw &v1) {\
  NAME(dst, simd_dw<vectorNum>(v0), simd_dw<vectorNum>(v1));\
}
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, MUL_S32, *, s);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, DIV_S32, /, s);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, REM_S32, %, s);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, MUL_U32, *, u);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, DIV_U32, /, u);
VEC_OP(simd_dw<vectorNum>, simd_dw<vectorNum>, REM_U32, %, u);
#undef VEC_OP

/* Vector compare vectors that require C */
#define VEC_OP(DST_TYPE, SRC_TYPE, NAME, OP, FIELD)\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const SRC_TYPE &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i) {\
    cast_dw c0(v0.m[i]), c1(v1.m[i]), d;\
    for (uint32_t j = 0; j < 4; ++j)\
      d.u[j] = (c0.FIELD[j] OP c1.FIELD[j]) ? ~0u : 0u;\
    dst.m[i] = d.v;\
  }\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const SRC_TYPE &v0, const scalar_dw &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i) {\
    cast_dw c0(v0.m[i]), d;\
    for (uint32_t j = 0; j < 4; ++j)\
      d.u[j] = (c0.FIELD[j] OP v1.FIELD) ? ~0u : 0u;\
    dst.m[i] = d.v;\
  }\
}\
template <uint32_t vectorNum>\
INLINE void NAME(DST_TYPE &dst, const scalar_dw &v0, const SRC_TYPE &v1) {\
  for (uint32_t i = 0; i < vectorNum; ++i) {\
    cast_dw c1(v1.m[i]), d;\
    for (uint32_t j = 0; j < 4; ++j)\
      d.u[j] = (v0.FIELD OP c1.FIELD[j]) ? ~0u : 0u;\
    dst.m[i] = d.v;\
  }\
}
VEC_OP(simd_m<vectorNum>, simd_dw<vectorNum>, LE_U32, <=, u);
VEC_OP(simd_m<vectorNum>, simd_dw<vectorNum>, LT_U32, <, u);
VEC_OP(simd_m<vectorNum>, simd_dw<vectorNum>, GE_U32, >=, u);
VEC_OP(simd_m<vectorNum>, simd_dw<vectorNum>, GT_U32, >, u);
#undef VEC_OP

template <uint32_t vectorNum>
INLINE void NE_S32(simd_m<vectorNum> &dst,
                   const simd_dw<vectorNum> &v0,
                   const simd_dw<vectorNum> &v1)
{
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_xor_ps(alltrue.v, SI2PS(_mm_cmpeq_epi32(PS2SI(v0.m[i]), PS2SI(v1.m[i]))));
}
template <uint32_t vectorNum>
INLINE void NE_S32(simd_m<vectorNum> &dst,
                   const simd_dw<vectorNum> &v0,
                   const scalar_dw &v1)
{
  NE_S32(dst, v0, simd_dw<vectorNum>(v1));
}
template <uint32_t vectorNum>
INLINE void NE_S32(simd_m<vectorNum> &dst,
                   const scalar_dw &v0,
                   const simd_dw<vectorNum> &v1)
{
  NE_S32(dst, simd_dw<vectorNum>(v0), v1);
}

/* Load from contiguous double words */
template <uint32_t vectorNum>
INLINE void LOAD(simd_dw<vectorNum> &dst, const char *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_loadu_ps((const float*) ptr + 4*i);
}

/* Store to contiguous double words */
template <uint32_t vectorNum>
INLINE void STORE(const simd_dw<vectorNum> &src, char *ptr) {
  for (uint32_t i = 0; i < vectorNum; ++i)
    _mm_storeu_ps((float*) ptr + 4*i, src.m[i]);
}

/* Load immediates */
template <uint32_t vectorNum>
INLINE void LOADI(simd_dw<vectorNum> &dst, uint32_t u) {
  union { uint32_t u; float f; } cast;
  cast.u = u;
  for (uint32_t i = 0; i < vectorNum; ++i)
    dst.m[i] = _mm_load1_ps(&cast.f);
}

#include <cstdio>
/* Scatter */
template <uint32_t vectorNum>
INLINE void SCATTER(const simd_dw<vectorNum> &offset,
                    const simd_dw<vectorNum> &value,
                    char *base_address) {
  for (uint32_t i = 0; i < vectorNum; ++i) {
    const int v0 = _mm_extract_epi32(PS2SI(value.m[i]), 0);
    const int v1 = _mm_extract_epi32(PS2SI(value.m[i]), 1);
    const int v2 = _mm_extract_epi32(PS2SI(value.m[i]), 2);
    const int v3 = _mm_extract_epi32(PS2SI(value.m[i]), 3);
    const int o0 = _mm_extract_epi32(PS2SI(offset.m[i]), 0);
    const int o1 = _mm_extract_epi32(PS2SI(offset.m[i]), 1);
    const int o2 = _mm_extract_epi32(PS2SI(offset.m[i]), 2);
    const int o3 = _mm_extract_epi32(PS2SI(offset.m[i]), 3);
    *(int*)(base_address + o0) = v0;
    *(int*)(base_address + o1) = v1;
    *(int*)(base_address + o2) = v2;
    *(int*)(base_address + o3) = v3;
  }
}
template <uint32_t vectorNum>
INLINE void SCATTER(const simd_dw<vectorNum> &offset,
                    const scalar_dw &value,
                    char *base_address) {
  SCATTER(simd_dw<vectorNum>(value), offset, base_address);
}
template <uint32_t vectorNum>
INLINE void SCATTER(const scalar_dw &offset,
                    const simd_dw<vectorNum> &value,
                    char *base_address) {
  SCATTER(value, simd_dw<vectorNum>(offset), base_address);
}
#include <cstdio>
/* Gather */
template <uint32_t vectorNum>
INLINE void GATHER(simd_dw<vectorNum> &dst,
                   const simd_dw<vectorNum> &offset,
                   const char *base_address) {
  for (uint32_t i = 0; i < vectorNum; ++i) {
    const int o0 = _mm_extract_epi32(offset.m[i], 0);
    const int o1 = _mm_extract_epi32(offset.m[i], 1);
    const int o2 = _mm_extract_epi32(offset.m[i], 2);
    const int o3 = _mm_extract_epi32(offset.m[i], 3);
    const int v0 = *(const int*)(base_address + o0);
    const int v1 = *(const int*)(base_address + o1);
    const int v2 = *(const int*)(base_address + o2);
    const int v3 = *(const int*)(base_address + o3);
    dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v0, 0));
    dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v1, 1));
    dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v2, 2));
    dst.m[i] = SI2PS(_mm_insert_epi32(PS2SI(dst.m[i]), v3, 3));
  }
}
template <uint32_t vectorNum>
INLINE void GATHER(simd_dw<vectorNum> &dst,
                   const scalar_dw &offset,
                   const char *base_address) {
  GATHER(dst, simd_dw<vectorNum>(offset), base_address);
}

//////////////////////////////////////////////////////////////////////////////
// Scalar instructions
//////////////////////////////////////////////////////////////////////////////
INLINE uint32_t elemNum(const scalar_dw &x) { return 1; }
INLINE uint32_t elemNum(const scalar_m &x) { return 1; }
INLINE uint32_t mask(const scalar_m &v) { return v.u ? 1 : 0; }
INLINE void ADD_F(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.f = v0.f + v1.f; }
INLINE void SUB_F(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.f = v0.f - v1.f; }
INLINE void MUL_F(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.f = v0.f * v1.f; }
INLINE void DIV_F(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.f = v0.f / v1.f; }
INLINE void EQ_F(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.f == v1.f ? ~0 : 0); }
INLINE void NE_F(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.f != v1.f ? ~0 : 0); }
INLINE void LE_F(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.f <= v1.f ? ~0 : 0); }
INLINE void LT_F(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.f < v1.f ? ~0 : 0); }
INLINE void GE_F(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.f >= v1.f ? ~0 : 0); }
INLINE void GT_F(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.f > v1.f ? ~0 : 0); }
INLINE void ADD_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s + v1.s; }
INLINE void SUB_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s - v1.s; }
INLINE void MUL_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s * v1.s; }
INLINE void DIV_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s / v1.s; }
INLINE void REM_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s % v1.s; }
INLINE void MUL_U32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.u = v0.u * v1.u; }
INLINE void DIV_U32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.u = v0.u / v1.u; }
INLINE void REM_U32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.u = v0.u % v1.u; }
INLINE void EQ_S32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.s == v1.s ? ~0 : 0); }
INLINE void NE_S32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.s != v1.s ? ~0 : 0); }
INLINE void LE_S32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.s <= v1.s ? ~0 : 0); }
INLINE void LT_S32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.s < v1.s ? ~0 : 0); }
INLINE void GE_S32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.s >= v1.s ? ~0 : 0); }
INLINE void GT_S32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.s > v1.s ? ~0 : 0); }
INLINE void XOR_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s ^ v1.s; }
INLINE void OR_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s | v1.s; }
INLINE void AND_S32(scalar_dw &dst, scalar_dw v0, scalar_dw v1) { dst.s = v0.s & v1.s; }
INLINE void LE_U32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.u <= v1.u ? ~0 : 0); }
INLINE void LT_U32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.u < v1.u ? ~0 : 0); }
INLINE void GE_U32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.u >= v1.u ? ~0 : 0); }
INLINE void GT_U32(scalar_m &dst, scalar_dw v0, scalar_dw v1) { dst.u = (v0.u > v1.u ? ~0 : 0); }
INLINE void LOAD(scalar_dw &dst, const char *ptr) { dst.u = *(const uint32_t *) ptr; }
INLINE void STORE(scalar_dw src, char *ptr) { *(uint32_t *) ptr = src.u; }
INLINE void LOADI(scalar_dw &dst, uint32_t u) { dst.u = u; }
INLINE void SCATTER(scalar_dw value, scalar_dw offset, char *base) { *(uint32_t*)(base + offset.u) = value.u; }
INLINE void GATHER(scalar_dw &dst, scalar_dw offset, const char *base) { dst.u = *(const uint32_t*)(base + offset.u); }

//////////////////////////////////////////////////////////////////////////////
// Identical instructions are forwarded
//////////////////////////////////////////////////////////////////////////////

#define ADD_U32 ADD_S32
#define SUB_U32 SUB_S32
#define XOR_U32 XOR_S32
#define OR_U32 OR_S32
#define AND_U32 AND_S32
#define EQ_U32 EQ_S32
#define NE_U32 NE_S32

#undef PS2SI
#undef SI2PS
#undef ID
#undef INLINE

#endif /* __GBE_SIM_VECTOR_H__ */

